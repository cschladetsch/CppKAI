#!/bin/bash

# Script to run chat functionality tests

echo "=== Running Chat Functionality Tests ==="
echo

cd "$(dirname "$0")/.."

if [ -x "./Bin/Test/TestNetwork" ]; then
    TEST_BIN="./Bin/Test/TestNetwork"
elif [ -x "./Bin/TestNetwork" ]; then
    TEST_BIN="./Bin/TestNetwork"
else
    echo "Error: TestNetwork binary not found"
    exit 1
fi

echo "1. Running basic chat functionality tests..."
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