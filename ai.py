import os
import subprocess
import sys
from collections import defaultdict
import llvmlite.binding as llvm
import hashlib
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.metrics.pairwise import cosine_similarity
import concurrent.futures
import pickle
import argparse
import re
import shutil

USE_CLANG = False
try:
    import clang.cindex
    clang.cindex.Config.set_library_file('/usr/lib/llvm-14/lib/libclang.so.1')
    index = clang.cindex.Index.create()
    USE_CLANG = True
except Exception as e:
    print(f"Error initializing Clang: {e}")
    print("Falling back to simple analysis method.")

class AutomatedCodeAnalyzer:
    def __init__(self, root_dir=".", exclude_dirs=None, verbose=False):
        if verbose:
            print(f"Initializing AutomatedCodeAnalyzer with root_dir: {root_dir}")
        self.root_dir = root_dir
        self.exclude_dirs = exclude_dirs or ['build']
        self.files = []
        self.llvm_ir = {}
        self.analysis_results = defaultdict(int)
        self.file_contents = {}
        self.function_info = defaultdict(list)
        self.file_hashes = {}
        self.compile_errors = defaultdict(list)
        self.vectorizer = TfidfVectorizer(stop_words='english')
        self.verbose = verbose
        self.operator_usage = defaultdict(list)
        self.variable_declarations = defaultdict(list)
        self.variable_references = defaultdict(list)

        llvm.initialize()
        llvm.initialize_native_target()
        llvm.initialize_native_asmprinter()

    def load_files(self):
        if self.verbose:
            print("Loading files...")
        self.files = []
        for root, dirs, files in os.walk(self.root_dir):
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs]
            for file in files:
                if file.endswith(('.cpp', '.hpp', '.h', '.cc')):
                    file_path = os.path.join(root, file)
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    self.files.append(file_path)
                    self.file_contents[file_path] = content
                    self.file_hashes[file_path] = hashlib.md5(content.encode('utf-8')).hexdigest()
        if self.verbose:
            print(f"Loaded {len(self.files)} C++ files.")
        self.analyze_operator_usage()
        if USE_CLANG:
            self.analyze_variables_clang()
        else:
            self.analyze_variables_simple()

    def generate_llvm_ir(self):
        if self.verbose:
            print("Generating LLVM IR...")
        with concurrent.futures.ThreadPoolExecutor() as executor:
            futures = {executor.submit(self._compile_to_ir, file_path): file_path for file_path in self.files}
            for future in concurrent.futures.as_completed(futures):
                file_path = futures[future]
                try:
                    ir = future.result()
                    if ir:
                        self.llvm_ir[file_path] = ir
                except Exception as e:
                    if self.verbose:
                        print(f"Error generating LLVM IR for {file_path}: {e}")
        if self.verbose:
            print(f"Generated LLVM IR for {len(self.llvm_ir)} files.")

    def _get_include_paths(self):
        include_paths = ['-I' + self.root_dir]
        for root, dirs, _ in os.walk(self.root_dir):
            dirs[:] = [d for d in dirs if d not in self.exclude_dirs]
            for dir in dirs:
                include_paths.append('-I' + os.path.join(root, dir))
        
        system_includes = self._find_cpp_system_headers()
        for path in system_includes:
            include_paths.extend(['-isystem', path])
        
        return include_paths

    def _find_cpp_system_headers(self):
        if self.verbose:
            print("Attempting to find C++ system headers...")
        try:
            result = subprocess.run(['clang++', '-E', '-x', 'c++', '-std=c++17', '-v', '/dev/null'],
                                    stderr=subprocess.PIPE, stdout=subprocess.PIPE, text=True, check=True)
            
            lines = result.stderr.split('\n')
            include_paths = []
            record = False
            for line in lines:
                if line.strip() == "#include <...> search starts here:":
                    record = True
                elif line.strip() == "End of search list.":
                    break
                elif record:
                    include_paths.append(line.strip())
            
            if self.verbose:
                print(f"Found {len(include_paths)} system include paths.")
            return include_paths
        except subprocess.CalledProcessError as e:
            if self.verbose:
                print(f"Error finding C++ system headers: {e}")
            return []

    def _get_cxx_flags(self):
        return [
            '-std=c++17',
            '-fno-exceptions',
            '-fno-rtti',
            '-D__STDC_CONSTANT_MACROS',
            '-D__STDC_FORMAT_MACROS',
            '-D__STDC_LIMIT_MACROS',
        ]

    def _compile_to_ir(self, file_path):
        if self.verbose:
            print(f"Compiling to IR: {file_path}")
        include_paths = self._get_include_paths()
        cxx_flags = self._get_cxx_flags()
        cmd = ['clang++', '-S', '-emit-llvm', '-o', '-'] + cxx_flags + include_paths + [file_path]
        
        try:
            result = subprocess.run(cmd, capture_output=True, check=True, timeout=60)
            ir_output = result.stdout.decode('utf-8', errors='ignore')
            return ir_output
        except subprocess.CalledProcessError as e:
            error_msg = e.stderr.decode('utf-8', errors='ignore')
            self.compile_errors[file_path].append(error_msg)
            if self.verbose:
                print(f"Compilation error for {file_path}:")
                print(error_msg)
                print("Command used:")
                print(' '.join(cmd))
        except subprocess.TimeoutExpired:
            if self.verbose:
                print(f"Compilation timeout for {file_path}")
        except Exception as e:
            if self.verbose:
                print(f"Unexpected error compiling {file_path}: {e}")
        return None

    def analyze_operator_usage(self):
        if self.verbose:
            print("Analyzing C++ operator usage...")
        operators = ['new', 'delete', 'delete\\[\\]', 'new\\[\\]']
        for file_path, content in self.file_contents.items():
            for operator in operators:
                matches = re.finditer(r'\b' + operator + r'\b', content)
                for match in matches:
                    line_num = content[:match.start()].count('\n') + 1
                    self.operator_usage[operator].append((file_path, line_num))

    def analyze_variables_clang(self):
        if self.verbose:
            print("Analyzing variable declarations and references using Clang...")
        for file_path, content in self.file_contents.items():
            try:
                tu = index.parse(file_path, args=['-std=c++17'] + self._get_include_paths())
                self.traverse_ast(tu.cursor, file_path)
            except Exception as e:
                if self.verbose:
                    print(f"Error parsing {file_path}: {e}")

    def traverse_ast(self, cursor, file_path):
        if cursor.kind == clang.cindex.CursorKind.VAR_DECL:
            self.variable_declarations[cursor.spelling].append((file_path, cursor.location.line))
        elif cursor.kind == clang.cindex.CursorKind.DECL_REF_EXPR:
            self.variable_references[cursor.spelling].append((file_path, cursor.location.line))

        for child in cursor.get_children():
            self.traverse_ast(child, file_path)

    def analyze_variables_simple(self):
        if self.verbose:
            print("Analyzing variable declarations and references (simple method)...")
        var_decl_pattern = re.compile(r'\b(?:int|float|double|char|bool|auto)\s+(\w+)')
        var_ref_pattern = re.compile(r'\b(\w+)\b')
        
        for file_path, content in self.file_contents.items():
            declarations = var_decl_pattern.findall(content)
            for var in declarations:
                self.variable_declarations[var].append((file_path, content.count('\n', 0, content.index(var)) + 1))
            
            words = var_ref_pattern.findall(content)
            for word in words:
                if word in self.variable_declarations:
                    self.variable_references[word].append((file_path, content.count('\n', 0, content.index(word)) + 1))

    def analyze_files(self):
        if self.verbose:
            print("Analyzing files...")
        for file_path, ir in self.llvm_ir.items():
            if ir.strip():
                try:
                    self.analyze_ir(file_path, ir)
                except Exception as e:
                    if self.verbose:
                        print(f"Error analyzing IR for {file_path}: {e}")
            else:
                if self.verbose:
                    print(f"Skipping {file_path} due to empty or invalid LLVM IR.")
        if self.verbose:
            print("File analysis complete.")

    def analyze_ir(self, file_path, ir):
        try:
            module = llvm.parse_assembly(ir)
            for func in module.functions:
                self.analysis_results['functions'] += 1
                self.function_info[file_path].append(func.name)
                for block in func.blocks:
                    for instr in block.instructions:
                        self._analyze_instruction(instr)
        except Exception as e:
            if self.verbose:
                print(f"Error analyzing IR for {file_path}: {e}")

    def _analyze_instruction(self, instr):
        op_name = instr.opcode.name
        if op_name == 'call':
            called_func = instr.called_function
            if called_func:
                func_name = called_func.name
                if 'std::' in func_name:
                    self.analysis_results['std_library_calls'] += 1
                if 'thread' in func_name:
                    self.analysis_results['threading'] += 1
                if 'mutex' in func_name or 'lock' in func_name:
                    self.analysis_results['synchronization'] += 1
                if 'malloc' in func_name or 'free' in func_name or 'new' in func_name or 'delete' in func_name:
                    self.analysis_results['dynamic_memory'] += 1
        elif op_name == 'alloca':
            self.analysis_results['local_variables'] += 1
        elif op_name in ('load', 'store'):
            self.analysis_results['memory_operations'] += 1
        elif op_name.startswith('cmp'):
            self.analysis_results['comparisons'] += 1
        elif op_name.startswith('br'):
            self.analysis_results['branches'] += 1

    def generate_report(self):
        if self.verbose:
            print("Generating report...")
        report = "C++ Code Analysis Report (LLVM-based)\n"
        report += "=====================================\n\n"
        report += f"Total files analyzed: {len(self.llvm_ir)}\n"
        report += f"Files with compilation errors: {len(self.compile_errors)}\n\n"
        report += "Feature usage:\n"
        for feature, count in sorted(self.analysis_results.items()):
            report += f"- {feature}: {count}\n"
        return report

    def answer_question(self, question):
        if self.verbose:
            print(f"Answering question: {question}")
        lower_question = question.lower()
        if 'where' in lower_question and any(op in lower_question for op in ['new', 'delete']):
            return self.answer_operator_usage_question(question)
        elif 'where' in lower_question and 'variable' in lower_question:
            return self.answer_variable_question(question)
        
        corpus = list(self.file_contents.values())
        corpus.append(question)
        tfidf_matrix = self.vectorizer.fit_transform(corpus)
        cosine_similarities = cosine_similarity(tfidf_matrix[-1], tfidf_matrix[:-1]).flatten()
        top_indices = cosine_similarities.argsort()[-3:][::-1]
        response = f"Based on the analysis of your codebase, here's what I found:\n\n"
        for idx in top_indices:
            file_path = list(self.file_contents.keys())[idx]
            response += f"Relevant file: {file_path}\n"
            response += f"Functions in this file: {', '.join(self.function_info[file_path])}\n\n"
        if "function" in question.lower():
            response += f"Total functions in the codebase: {self.analysis_results['functions']}\n"
        if "thread" in question.lower():
            response += f"Threading-related calls: {self.analysis_results['threading']}\n"
        if "memory" in question.lower():
            response += f"Dynamic memory operations: {self.analysis_results['dynamic_memory']}\n"
        if "standard library" in question.lower():
            response += f"Standard library calls: {self.analysis_results['std_library_calls']}\n"
        most_similar_file = list(self.file_contents.keys())[top_indices[0]]
        file_content = self.file_contents[most_similar_file]
        lines = file_content.split('\n')
        snippet = '\n'.join(lines[:15])
        response += f"\nHere's a snippet from the most relevant file ({most_similar_file}):\n\n{snippet}\n..."
        return response

    def answer_operator_usage_question(self, question):
        operators = {
            'new': 'new',
            'new[]': 'new[]',
            'delete': 'delete',
            'delete[]': 'delete[]'
        }
        
        operator = next((op for op in operators if op in question.lower()), None)
        if operator:
            usages = self.operator_usage[operators[operator]]
            if usages:
                response = f"The '{operator}' operator is used in the following locations:\n\n"
                for file_path, line_num in usages:
                    response += f"- {file_path}, line {line_num}\n"
            else:
                response = f"The '{operator}' operator was not found in the analyzed codebase."
        else:
            response = "Please specify a C++ operator (new, new[], delete, or delete[]) in your question."
        
        return response

    def answer_
