#!/bin/bash

# Simple script to run only the fixed and working tests

echo -e "\033[1m=============================================\033[0m"
echo -e "\033[1mRunning KAI fixed tests\033[0m"
echo -e "\033[1m=============================================\033[0m"

# Find bin directory
BIN_DIR="/home/xian/local/KAI/Bin/Test"
if [ ! -d "$BIN_DIR" ]; then
    echo "Error: $BIN_DIR directory not found"
    exit 1
fi

# Run specifically the Rho tests that we've fixed
if [ -f "$BIN_DIR/TestRho" ]; then
    echo -e "\033[36mRunning Rho Language Tests with our fixes\033[0m"
    "$BIN_DIR/TestRho" --gtest_filter="LanguageComparison.*:TestRho.*:RhoPiBasic.*:DirectBinaryOp.*:RhoPiTests.*:RhoBinaryOps.*:AdvancedBinaryOps.*:DoWhile.*:RhoLinearStream.*:RhoScriptTest.*"
else
    echo "Error: TestRho executable not found in $BIN_DIR"
    echo "Did you build the tests? Try: mkdir -p Build && cd Build && cmake .. && make"
    exit 1
fi

# Removed the separate run for DirectBinaryOp tests since they are disabled

echo -e "\033[1m=============================================\033[0m"
echo -e "\033[1mTest run complete\033[0m"
echo -e "\033[1m=============================================\033[0m"