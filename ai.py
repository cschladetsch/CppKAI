import os
import sys
import argparse
import torch
from transformers import RobertaTokenizer, RobertaForMaskedLM, RobertaConfig
from transformers import Trainer, TrainingArguments
from sklearn.metrics.pairwise import cosine_similarity
import numpy as np
import llvmlite.binding as llvm
import subprocess
import re
import pickle
import warnings
from clang.cindex import Index, CursorKind, TypeKind
import clang.cindex

class CodeAnalyzer:
    def __init__(self, root_dir="."):
        self.root_dir = root_dir
        self.files = []
        self.llvm_ir = {}
        self.ast_data = {}
        self.tokenizer = None
        self.model = None
        self.device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

        # Initialize LLVM
        llvm.initialize()
        llvm.initialize_native_target()
        llvm.initialize_native_asmprinter()

        # Set up Clang
        clang.cindex.Config.set_library_file('/usr/lib/llvm-10/lib/libclang.so.1')

    def load_files(self):
        self.files = []
        for root, _, files in os.walk(self.root_dir):
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp')):
                    file_path = os.path.join(root, file)
                    with open(file_path, 'r', encoding='utf-8') as f:
                        content = f.read()
                    self.files.append((file_path, content))
        print(f"Loaded {len(self.files)} C++ files.")

    def generate_llvm_ir(self):
        self.llvm_ir = {}
        include_paths = [
            './Include',
            './Source',
            './ThirdParty',
            './Source/Library',
            '/usr/include',
            '/usr/local/include',
        ]
        include_args = [f'-I{path}' for path in include_paths]

        for file_path, _ in self.files:
            try:
                process = subprocess.Popen(
                    ['clang++', '-std=c++17', '-S', '-emit-llvm', '-o', '-'] + include_args + [file_path],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    universal_newlines=True
                )
                stdout, stderr = process.communicate()
                if process.returncode == 0:
                    self.llvm_ir[file_path] = stdout
                else:
                    warnings.warn(f"Error generating LLVM IR for {file_path}:\n{stderr}")
                    if "file not found" in stderr:
                        missing_file = re.search(r"'(.+)' file not found", stderr)
                        if missing_file:
                            warnings.warn(f"Missing header file: {missing_file.group(1)}")
                            warnings.warn(f"Make sure the file exists and is in one of these directories: {include_paths}")
            except Exception as e:
                warnings.warn(f"Exception while processing {file_path}: {str(e)}")
        
        print(f"LLVM IR generation complete. Successfully processed {len(self.llvm_ir)} out of {len(self.files)} files.")
        if len(self.llvm_ir) < len(self.files):
            print(f"Failed to process {len(self.files) - len(self.llvm_ir)} files. Check the warnings for details.")

    def parse_ast(self):
        index = Index.create()
        for file_path, _ in self.files:
            try:
                tu = index.parse(file_path, args=['-std=c++17'])
                self.ast_data[file_path] = self.process_ast_node(tu.cursor)
            except Exception as e:
                warnings.warn(f"Error parsing AST for {file_path}: {str(e)}")
        print(f"AST parsing complete. Successfully processed {len(self.ast_data)} out of {len(self.files)} files.")

    def process_ast_node(self, node):
        result = {
            'kind': node.kind.name,
            'spelling': node.spelling,
            'type': node.type.spelling if node.type else None,
            'children': []
        }
        
        if node.kind == CursorKind.CLASS_DECL:
            result['methods'] = self.get_class_methods(node)
            result['templates'] = self.get_template_info(node)
            result['smart_pointers'] = self.get_smart_pointer_usage(node)
        elif node.kind == CursorKind.FUNCTION_DECL:
            result['is_template'] = self.is_template(node)
            result['is_overloaded'] = self.is_overloaded(node)
            result['has_move_semantics'] = self.has_move_semantics(node)
            result['is_constexpr'] = 'constexpr' in node.displayname
            result['is_consteval'] = 'consteval' in node.displayname
            result['is_variadic'] = self.is_variadic_template(node)
            result['has_noexcept'] = 'noexcept' in node.displayname
        elif node.kind == CursorKind.LAMBDA_EXPR:
            result['is_lambda'] = True
        elif node.kind == CursorKind.CXX_FOR_RANGE_STMT:
            result['is_range_based_for'] = True
        elif node.kind == CursorKind.USING_DECLARATION:
            result['is_structured_binding'] = self.is_structured_binding(node)
        elif node.kind == CursorKind.NAMESPACE:
            result['is_inline'] = 'inline' in node.displayname
        
        result['attributes'] = self.get_attributes(node)
        result['auto_usage'] = self.has_auto_type(node)
        result['stl_usage'] = self.get_stl_usage(node)
        result['algorithm_usage'] = self.get_algorithm_usage(node)
        result['threading_usage'] = self.get_threading_usage(node)
        
        result['concurrency'] = self.analyze_concurrency(node)
        result['network'] = self.analyze_network(node)
        result['performance'] = self.analyze_performance(node)
        result['rhel_specific'] = self.analyze_rhel_specific(node)
        result['build_system'] = self.analyze_build_system(node)
        result['error_handling'] = self.analyze_error_handling(node)
        result['memory_management'] = self.analyze_memory_management(node)
        result['distributed_systems'] = self.analyze_distributed_systems(node)
        
        for child in node.get_children():
            result['children'].append(self.process_ast_node(child))
        
        return result

    # ... [All other methods remain the same as in the previous version]

    def retrain(self):
        print("Retraining CodeBERT on the local codebase...")
        self.tokenizer = RobertaTokenizer.from_pretrained("microsoft/codebert-base")
        self.model = RobertaForMaskedLM.from_pretrained("microsoft/codebert-base")
        self.model.to(self.device)

        train_dataset = self.tokenizer(
            [content for _, content in self.files],
            truncation=True,
            padding=True,
            max_length=512,
            return_tensors="pt"
        )
        train_dataset = train_dataset.with_labels(train_dataset["input_ids"].clone())

        training_args = TrainingArguments(
            output_dir="./results",
            overwrite_output_dir=True,
            num_train_epochs=3,
            per_device_train_batch_size=4,
            save_steps=10_000,
            save_total_limit=2,
        )

        trainer = Trainer(
            model=self.model,
            args=training_args,
            train_dataset=train_dataset,
        )

        trainer.train()
        self.model = trainer.model
        print("Retraining complete.")

    def save_data(self, path="./analyzer_data"):
        os.makedirs(path, exist_ok=True)
        self.model.save_pretrained(os.path.join(path, "model"))
        self.tokenizer.save_pretrained(os.path.join(path, "tokenizer"))
        with open(os.path.join(path, "llvm_ir.pkl"), "wb") as f:
            pickle.dump(self.llvm_ir, f)
        with open(os.path.join(path, "ast_data.pkl"), "wb") as f:
            pickle.dump(self.ast_data, f)
        print(f"Data saved to {path}")

    def load_data(self, path="./analyzer_data"):
        if os.path.exists(path):
            self.model = RobertaForMaskedLM.from_pretrained(os.path.join(path, "model"))
            self.tokenizer = RobertaTokenizer.from_pretrained(os.path.join(path, "tokenizer"))
            self.model.to(self.device)
            with open(os.path.join(path, "llvm_ir.pkl"), "rb") as f:
                self.llvm_ir = pickle.load(f)
            with open(os.path.join(path, "ast_data.pkl"), "rb") as f:
                self.ast_data = pickle.load(f)
            print(f"Data loaded from {path}")
            return True
        return False

    def analyze_query(self, question):
        inputs = self.tokenizer(question, return_tensors="pt", truncation=True, max_length=512, padding=True)
        inputs = {k: v.to(self.device) for k, v in inputs.items()}
        with torch.no_grad():
            outputs = self.model(**inputs)
        
        question_embedding = outputs.last_hidden_state.mean(dim=1).cpu().numpy()

        file_embeddings = [self.get_file_embedding(content) for _, content in self.files]
        similarities = cosine_similarity(question_embedding, np.vstack(file_embeddings))
        relevant_files = sorted(zip(self.files, similarities[0]), key=lambda x: x[1], reverse=True)[:3]

        analysis_results = []
        for (file_path, _), _ in relevant_files:
            ir = self.llvm_ir.get(file_path, "")
            ast = self.ast_data.get(file_path, {})
            analysis_results.append(self.analyze_file(file_path, ir, ast, question))

        combined_analysis = self.combine_analyses(analysis_results, question)

        return combined_analysis

    # ... [All other methods remain the same as in the previous version]

def main():
    parser = argparse.ArgumentParser(description="Analyze a C++17 codebase using CodeBERT and LLVM.")
    parser.add_argument('--retrain', action='store_true', help="Retrain the model on the current codebase")
    args = parser.parse_args()

    analyzer = CodeAnalyzer()
    analyzer.load_files()

    if args.retrain or not analyzer.load_data():
        analyzer.generate_llvm_ir()
        analyzer.parse_ast()
        analyzer.retrain()
        analyzer.save_data()
    else:
        print("Using existing data.")

    print("Advanced C++17 RHEL Codebase Analyzer for High-Performance Distributed Systems")
    print("-------------------------------------------------------------------------------")
    print("You can now ask questions about the codebase.")

    while True:
        question = input("\nEnter your question (or 'exit' to quit): ")
        if question.lower() == 'exit':
            break
        answer = analyzer.analyze_query(question)
        print(answer)

if __name__ == "__main__":
    main()
