#!/bin/bash

echo "===================================="
echo "Running fixed KAI tests..."
echo "===================================="

# Get the directory this script is running from
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Set up environment variables for test running
export KAI_SCRIPT_ROOT="$SCRIPT_DIR/Test/Scripts"
export KAI_TEST_ROOT="$SCRIPT_DIR/Test"

# Make sure we have a Logs directory
mkdir -p Logs

# Run each test with proper setup
echo "Running TestTau..."
./Bin/Test/TestTau

echo "Running TestRho with fixes..."
# Use our fixed implementation
./Bin/Test/TestRho

echo "Running TestPi..."
./Bin/Test/TestPi

echo "Running TestCore..."
./Bin/Test/TestCore

echo "===================================="
echo "All tests completed!"
echo "Check Logs directory for detailed output."
echo "===================================="