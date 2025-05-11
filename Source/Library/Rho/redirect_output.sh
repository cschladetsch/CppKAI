#!/bin/bash

# Check if an argument is provided
if [ $# -lt 1 ]; then
    echo "Error: No output file specified"
    echo "Usage: $0 'output_file' [command_to_run]"
    exit 1
fi

# Extract the output file from the first argument
# Remove single quotes if they exist
OUTPUT_FILE=$(echo "$1" | sed "s/^'//;s/'$//")

# Check if the output file path is valid
if [[ -z "$OUTPUT_FILE" ]]; then
    echo "Error: Invalid output file path"
    exit 1
fi

# Create the directory structure if it doesn't exist
mkdir -p "$(dirname "$OUTPUT_FILE")"

# If there are additional arguments, treat them as a command to run
if [ $# -gt 1 ]; then
    # Shift to remove the first argument
    shift
    
    # Execute the remaining arguments as a command and redirect output
    "$@" > "$OUTPUT_FILE" 2>&1
    
    # Store the exit code
    EXIT_CODE=$?
    
    # Print a confirmation message to stderr (not captured in the output file)
    echo "Command output has been written to: $OUTPUT_FILE" >&2
    echo "Exit code: $EXIT_CODE" >&2
    
    exit $EXIT_CODE
else
    # If no command is provided, redirect all future input to the file
    # until EOF (Ctrl+D) is received
    echo "Enter text to write to '$OUTPUT_FILE' (press Ctrl+D when finished):" >&2
    cat > "$OUTPUT_FILE"
    
    echo "Input has been written to: $OUTPUT_FILE" >&2
fi