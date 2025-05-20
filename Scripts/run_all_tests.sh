#!/bin/bash

# Script to run all tests in the KAI project
# This script runs all test suites and reports results

cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
SCRIPTS_DIR="$ROOT_DIR/Scripts"

# Set colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo "==============================================="
echo "        Running All KAI Tests"
echo "==============================================="

# Array of test scripts to run
TEST_SCRIPTS=(
    "run_fixed_tests.sh"
    "run_rho_tests.sh"
    "run_connection_tests.sh"
    "run_tau_connection_tests.sh"
    "p2p_test_standalone.sh"  # Using the reliable standalone version instead of p2p_test_dynamic.sh
)

# Counters for test results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Run each test script and track results
for script in "${TEST_SCRIPTS[@]}"; do
    if [ -f "$SCRIPTS_DIR/$script" ]; then
        echo -e "\n${YELLOW}Running $script...${NC}"
        chmod +x "$SCRIPTS_DIR/$script"  # Ensure the script is executable
        
        # Set a timeout for each test (10 minutes)
        timeout 600 "$SCRIPTS_DIR/$script"
        exit_code=$?
        
        # Check exit code
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}✓ $script completed successfully${NC}"
            ((PASSED_TESTS++))
        elif [ $exit_code -eq 124 ] || [ $exit_code -eq 137 ]; then
            echo -e "${RED}✗ $script timed out${NC}"
            ((FAILED_TESTS++))
        else
            echo -e "${RED}✗ $script failed with exit code $exit_code${NC}"
            ((FAILED_TESTS++))
        fi
        
        ((TOTAL_TESTS++))
    else
        echo -e "${RED}Script $script not found!${NC}"
    fi
    
    echo "-----------------------------------------------"
done

# Print summary
echo -e "\n==============================================="
echo -e "             Test Summary"
echo -e "==============================================="
echo -e "Total Tests:  $TOTAL_TESTS"
echo -e "${GREEN}Passed Tests: $PASSED_TESTS${NC}"
if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "${RED}Failed Tests: $FAILED_TESTS${NC}"
else
    echo -e "Failed Tests: $FAILED_TESTS"
fi

# Return appropriate exit code
if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi