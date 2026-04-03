#!/bin/bash

echo -e "\e[1;33mRunning Rho tests one by one to isolate segmentation faults\e[0m"
cd $(dirname "$0")

# Get all the tests and test suites
TESTS=$(./Bin/Test/TestRho --gtest_list_tests)

# Current test suite
SUITE=""

# Read test names line by line
while read -r line; do
    # Test suite lines end with a dot
    if [[ $line =~ ^[A-Za-z0-9_]+\.$ ]]; then
        SUITE=$line
    else
        # It's a test case, not a suite
        TEST=$(echo "$line" | tr -d ' ')
        FULLNAME="${SUITE}${TEST}"
        
        echo -n "Running $FULLNAME... "
        
        # Run the test and capture return code
        ./Bin/Test/TestRho --gtest_filter="$FULLNAME" > /dev/null 2>&1
        RESULT=$?
        
        # Check if it failed and report
        if [ $RESULT -ne 0 ]; then
            echo -e "\e[1;31mFAILED (exit code $RESULT)\e[0m"
        else
            echo -e "\e[1;32mPASSED\e[0m"
        fi
    fi
done <<< "$TESTS"

echo -e "\n\e[1;34mRho tests completed\e[0m"