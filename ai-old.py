import os
import sys
import argparse
from datasets import Dataset
from transformers import GPT2Tokenizer, GPT2LMHeadModel, Trainer, TrainingArguments
import torch

# Function to determine and print the device being used (CPU or GPU)
def get_device():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    if device.type == "cuda":
        print(">>> Using GPU (CUDA)")
    else:
        print(">>> Using CPU")
    return device

# Step 1: Load and preprocess the C++ code base

def load_cpp_codebase(root_dir="."):
    cpp_files = []
    print(f"Looking for C++ files in: {root_dir}")
    
    for subdir, _, files in os.walk(root_dir):
        # Filter only the .cpp and .h files
        cpp_files_in_dir = [file for file in files if file.endswith((".cpp", ".h"))]
        
        # If no C++ files found in this directory, skip it
        if not cpp_files_in_dir:
            continue
        
        print(f"Found {len(cpp_files_in_dir)} C++ files in directory: {subdir}")
        
        for file in cpp_files_in_dir:
            file_path = os.path.join(subdir, file)
            print(f"Loading C++ file: {file_path}")  # Debugging: Print the path of the files being loaded
            with open(file_path, "r", encoding="utf-8") as f:
                cpp_files.append(f.read())

    print(f"Total C++ files loaded: {len(cpp_files)}")  # Print the number of loaded files
    return cpp_files

# Step 2: Create a dataset from the C++ code

def create_dataset(cpp_code_files):
    dataset = Dataset.from_dict({"text": cpp_code_files})
    print(f"Number of C++ files loaded into dataset: {len(cpp_code_files)}")  # Debugging: check number of files loaded
    dataset = dataset.train_test_split(test_size=0.1)  # Split into training and testing sets
    return dataset

# Step 3: Tokenize the dataset with padding, truncation, and labels

def tokenize_dataset(dataset, tokenizer):
    # Set the padding token if it's not already set
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token
    
    def tokenize_function(examples):
        # Tokenize the text with padding and truncation
        tokens = tokenizer(examples["text"], padding="max_length", truncation=True, max_length=512)
        tokens["labels"] = tokens["input_ids"].copy()  # Use input_ids as labels for training
        return tokens
    
    return dataset.map(tokenize_function, batched=True)

# Step 4: Fine-tune the model

def fine_tune_model(tokenized_datasets, model):
    training_args = TrainingArguments(
        output_dir="./results",
        evaluation_strategy="epoch",
        learning_rate=2e-5,
        per_device_train_batch_size=2,
        per_device_eval_batch_size=2,
        num_train_epochs=3,
        weight_decay=0.01,
        remove_unused_columns=False,
    )

    # Print dataset sizes for debugging
    print(f"Training dataset size: {len(tokenized_datasets['train'])}")
    print(f"Validation dataset size: {len(tokenized_datasets['test'])}")

    # Check if the training dataset is empty
    if len(tokenized_datasets['train']) == 0:
        raise ValueError("Training dataset is empty after tokenization.")

    trainer = Trainer(
        model=model,
        args=training_args,
        train_dataset=tokenized_datasets["train"],
        eval_dataset=tokenized_datasets["test"],
    )

    trainer.train()
    return trainer

# Step 5: Save the fine-tuned model and tokenizer

def save_model_and_tokenizer(trainer, tokenizer, model_dir):
    trainer.save_model(model_dir)  # Save the model using the trainer
    tokenizer.save_pretrained(model_dir)

# Step 6: Load the fine-tuned model and tokenizer

def load_model_and_tokenizer(model_dir):
    model = GPT2LMHeadModel.from_pretrained(model_dir)
    tokenizer = GPT2Tokenizer.from_pretrained(model_dir)
    return model, tokenizer

# Step 7: Interactive query session

def query_codebase(model, tokenizer, device):
    model.to(device)  # Ensure model is on the correct device

    while True:
        question = input("\nEnter your question about the codebase (or type 'exit' to quit): ")
        if question.lower() == 'exit':
            break
        inputs = tokenizer.encode(question, return_tensors="pt").to(device)  # Move inputs to the correct device (CUDA or CPU)
        outputs = model.generate(inputs, max_length=200)
        response = tokenizer.decode(outputs[0], skip_special_tokens=True)
        print("\nResponse:", response)

# Main function

def main(retrain):
    device = get_device()  # Get the device (CPU or GPU) and print the banner

    model_dir = "./fine-tuned-gpt2-cpp"

    if retrain or not os.path.exists(model_dir):
        print("Training the model on the C++ codebase...")
        cpp_code_files = load_cpp_codebase(".")  # Use the current directory
        
        if not cpp_code_files:
            print("No C++ files found in the specified directory and its subdirectories.")
            return
        
        dataset = create_dataset(cpp_code_files)
        tokenizer = GPT2Tokenizer.from_pretrained("gpt2")
        tokenized_datasets = tokenize_dataset(dataset, tokenizer)
        model = GPT2LMHeadModel.from_pretrained("gpt2")
        trainer = fine_tune_model(tokenized_datasets, model)
        save_model_and_tokenizer(trainer, tokenizer, model_dir)
    else:
        print("Loading the cached fine-tuned model...")
        model, tokenizer = load_model_and_tokenizer(model_dir)

    print("\nThe model is ready. You can now ask questions about your C++ codebase.")
    query_codebase(model, tokenizer, device)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train or load a local LLM for querying a C++ codebase.")
    parser.add_argument('--retrain', action='store_true', help="Retrain the model instead of loading the cached version.")
    args = parser.parse_args()

    main(args.retrain)


