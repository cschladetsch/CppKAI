import os
import sys
import subprocess
from collections import defaultdict
import llvmlite.binding as llvm
import re
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.metrics.pairwise import cosine_similarity
import numpy as np
import concurrent.futures
import pickle
import argparse

class EnhancedLocalAICodeAnalyzer:
    def __init__(self, root_dir="."):
        self.root_dir = root_dir
        self.files = []
        self.llvm_ir = {}
        self.analysis_results = defaultdict(int)
        self.file_contents = {}
        self.function_info = defaultdict(list)

        # Initialize LLVM
        llvm.initialize()
        llvm.initialize_native_target()
        llvm.initialize_native_asmprinter()

        # Initialize TF-IDF vectorizer
        self.vectorizer = TfidfVectorizer(stop_words='english')

    def load_files(self):
        self.files = []
        for root, _, files in os.walk(self.root_dir):
            for file in files:
                if file.endswith(('.cpp', '.hpp', '.h')):  # Include header files
                    file_path = os.path.join(root, file)
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    self.files.append(file_path)
                    self.file_contents[file_path] = content
        print(f"Loaded {len(self.files)} C++ files.")

    def generate_llvm_ir(self):
        with concurrent.futures.ThreadPoolExecutor() as executor:
            futures = {executor.submit(self._compile_to_ir, file_path): file_path for file_path in self.files}
            for future in concurrent.futures.as_completed(futures):
                file_path = futures[future]
                try:
                    ir = future.result()
                    if ir:
                        self.llvm_ir[file_path] = ir
                except Exception as e:
                    print(f"Error generating LLVM IR for {file_path}: {e}")

    def _compile_to_ir(self, file_path):
        include_paths = self._get_include_paths()
        cmd = ['clang++', '-S', '-emit-llvm', '-o', '-', '-std=c++17'] + include_paths + [file_path]
        try:
            result = subprocess.run(cmd, capture_output=True, check=True, timeout=30)
            return result.stdout.decode('utf-8', errors='ignore')
        except subprocess.CalledProcessError as e:
            print(f"Compilation error for {file_path}:")
            print(e.stderr.decode('utf-8', errors='ignore'))
        except subprocess.TimeoutExpired:
            print(f"Compilation timeout for {file_path}")
        except Exception as e:
            print(f"Unexpected error compiling {file_path}: {e}")
        return None

    def _get_include_paths(self):
        include_paths = ['-I' + self.root_dir]
        for root, dirs, _ in os.walk(self.root_dir):
            for dir in dirs:
                include_paths.append('-I' + os.path.join(root, dir))
        return include_paths

    def analyze_files(self):
        for file_path, ir in self.llvm_ir.items():
            self.analyze_ir(file_path, ir)

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
        report = "C++ Code Analysis Report (LLVM-based)\n"
        report += "=====================================\n\n"
        report += f"Total files analyzed: {len(self.llvm_ir)}\n\n"
        report += "Feature usage:\n"
        for feature, count in sorted(self.analysis_results.items()):
            report += f"- {feature}: {count}\n"
        return report

    def answer_question(self, question):
        # Prepare the corpus
        corpus = list(self.file_contents.values())
        corpus.append(question)
        
        # Vectorize the corpus
        tfidf_matrix = self.vectorizer.fit_transform(corpus)
        
        # Compute similarity between the question and each file
        cosine_similarities = cosine_similarity(tfidf_matrix[-1], tfidf_matrix[:-1]).flatten()
        
        # Find the top 3 most similar files
        top_indices = cosine_similarities.argsort()[-3:][::-1]
        
        response = f"Based on the analysis of your codebase, here's what I found:\n\n"
        
        for idx in top_indices:
            file_path = list(self.file_contents.keys())[idx]
            response += f"Relevant file: {file_path}\n"
            response += f"Functions in this file: {', '.join(self.function_info[file_path])}\n\n"
        
        # Add some specific information based on the question
        if "function" in question.lower():
            response += f"Total functions in the codebase: {self.analysis_results['functions']}\n"
        if "thread" in question.lower():
            response += f"Threading-related calls: {self.analysis_results['threading']}\n"
        if "memory" in question.lower():
            response += f"Dynamic memory operations: {self.analysis_results['dynamic_memory']}\n"
        if "standard library" in question.lower():
            response += f"Standard library calls: {self.analysis_results['std_library_calls']}\n"
        
        # Add a snippet from the most similar file
        most_similar_file = list(self.file_contents.keys())[top_indices[0]]
        file_content = self.file_contents[most_similar_file]
        lines = file_content.split('\n')
        snippet = '\n'.join(lines[:15])  # First 15 lines as a snippet
        response += f"\nHere's a snippet from the most relevant file ({most_similar_file}):\n\n{snippet}\n..."
        
        return response

    def save_analysis(self, filename='analysis_results.pkl'):
        with open(filename, 'wb') as f:
            pickle.dump({
                'analysis_results': self.analysis_results,
                'function_info': self.function_info,
                'file_contents': self.file_contents
            }, f)
        print(f"Analysis results saved to {filename}")

    def load_analysis(self, filename='analysis_results.pkl'):
        if os.path.exists(filename):
            with open(filename, 'rb') as f:
                data = pickle.load(f)
                self.analysis_results = data['analysis_results']
                self.function_info = data['function_info']
                self.file_contents = data['file_contents']
            print(f"Analysis results loaded from {filename}")
            return True
        return False

def main():
    parser = argparse.ArgumentParser(description="Local AI C++ Code Analyzer")
    parser.add_argument('--dir', default='.', help='Directory to analyze')
    parser.add_argument('--load', action='store_true', help='Load previous analysis results')
    args = parser.parse_args()

    analyzer = EnhancedLocalAICodeAnalyzer(args.dir)

    if args.load and analyzer.load_analysis():
        print("Loaded previous analysis results.")
    else:
        analyzer.load_files()
        analyzer.generate_llvm_ir()
        analyzer.analyze_files()
        analyzer.save_analysis()
    
    print("\nLocal AI C++ Code Analyzer")
    print("==========================")
    print("You can now ask questions about your codebase.")
    print("Type 'exit' to quit.")
    
    while True:
        question = input("\nEnter your question: ")
        if question.lower() == 'exit':
            break
        answer = analyzer.answer_question(question)
        print("\nAnswer:")
        print(answer)

if __name__ == "__main__":
    main()
