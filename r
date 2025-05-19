#!/bin/bash

# Check if --no-color flag is provided
USE_COLOR=true
# Check if --gcc flag is provided (otherwise use clang++ by default)
USE_GCC=false
for arg in "$@"; do
  if [ "$arg" == "--no-color" ]; then
    USE_COLOR=false
  elif [ "$arg" == "--gcc" ]; then
    USE_GCC=true
  fi
done

# Colors for output
if [ "$USE_COLOR" = true ]; then
  RED='\033[0;31m'
  GREEN='\033[0;32m'
  YELLOW='\033[1;33m'
  BLUE='\033[0;34m'
  NC='\033[0m' # No Color
else
  RED=''
  GREEN=''
  YELLOW=''
  BLUE=''
  NC=''
fi

echo -e "${YELLOW}Running Rho tests one by one to isolate segmentation faults${NC}"
cd $(dirname "$0")

# Use the appropriate compiler based on the flag
COMPILER_FLAG=""
if [ "$USE_GCC" = false ]; then
  echo -e "${BLUE}Using Clang++ compiler${NC}"
  export CXX=clang++
else
  echo -e "${BLUE}Using GCC compiler${NC}"
  export CXX=g++
fi

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
            echo -e "${RED}FAILED (exit code $RESULT)${NC}"
        else
            echo -e "${GREEN}PASSED${NC}"
        fi
    fi
done <<< "$TESTS"

echo -e "\n${BLUE}Rho tests completed${NC}"