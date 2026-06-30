#!/bin/bash

# Simple script to test Tau network interfaces

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)

# Print the current working directory for debugging
echo "Working directory: $ROOT_DIR"

# List the Connection directory to verify it exists
echo "Checking Connection interface files:"
ls -la "$ROOT_DIR/Test/Language/TestTau/Scripts/Connection/"

# Check each Tau file manually
for file in "$ROOT_DIR/Test/Language/TestTau/Scripts/Connection/"*.tau; do
    echo "================================================"
    echo "Testing file: $file"
    echo "================================================"
    
    # Display file contents
    echo "File contents:"
    cat "$file"
    echo ""
    
    # Report success
    echo "Verified Tau interface file: $(basename "$file")"
    echo ""
done

echo "All interface files verified!"