#!/bin/bash

# Script to run chat functionality tests

echo "=== Running Chat Functionality Tests ==="
echo

# Change to build directory
cd "$(dirname "$0")/.."

# Check if tests are built
if [ -f "Bin/TestNetwork" ]; then
    TEST_BIN="./Bin/TestNetwork"
else

# Run basic chat functionality tests
echo "1. Running basic chat functionality tests..."
<<<<<<< HEAD
"$TEST_BIN" --gtest_filter="ChatFunctionalityTests.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "2. Running advanced chat tests (groups, persistence, etc.)..."
"$TEST_BIN" --gtest_filter="ChatAdvancedTests.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "3. Running Tau chat proxy generation tests..."
"$TEST_BIN" --gtest_filter="ChatProxyGenerationTest.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "4. Running ICQ-style chat tests..."
"$TEST_BIN" --gtest_filter="ICQStyleChatTest.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "=== Chat Test Summary ==="
echo "Run '$TEST_BIN --gtest_list_tests' to see all available tests"
echo "Run '$TEST_BIN --gtest_filter=<pattern>' to run specific tests"

./Bin/TestNetwork --gtest_filter="ChatFunctionalityTests.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "2. Running advanced chat tests (groups, persistence, etc.)..."
./Bin/TestNetwork --gtest_filter="ChatAdvancedTests.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "3. Running Tau chat proxy generation tests..."
./Bin/TestNetwork --gtest_filter="ChatProxyGenerationTest.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "4. Running ICQ-style chat tests..."
./Bin/TestNetwork --gtest_filter="ICQStyleChatTest.*" 2>&1 | grep -E "^\[|TEST|PASSED|FAILED"

echo
echo "=== Chat Test Summary ==="
echo "Run './Bin/TestNetwork --gtest_list_tests' to see all available tests"
echo "Run './Bin/TestNetwork --gtest_filter=<pattern>' to run specific tests"
