#!/bin/bash

# Script to run chat functionality tests

echo "=== Running Chat Functionality Tests ==="
echo

# Change to build directory
cd "$(dirname "$0")/.."

# Check if tests are built
if [ ! -f "Bin/Test_Network" ]; then
    echo "Error: Tests not built. Please run 'ninja' in the build directory first."
    exit 1
fi

# Run basic chat functionality tests
echo "1. Running basic chat functionality tests..."
./Bin/Test_Network --gtest_filter="ChatFunctionalityTests.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "2. Running advanced chat tests (groups, persistence, etc.)..."
./Bin/Test_Network --gtest_filter="ChatAdvancedTests.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "3. Running Tau chat proxy generation tests..."
./Bin/Test_Network --gtest_filter="ChatProxyGenerationTest.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "4. Running ICQ-style chat tests..."
./Bin/Test_Network --gtest_filter="ICQStyleChatTest.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "=== Chat Test Summary ==="
echo "Run './Bin/Test_Network --gtest_list_tests' to see all available tests"
echo "Run './Bin/Test_Network --gtest_filter=<pattern>' to run specific tests"