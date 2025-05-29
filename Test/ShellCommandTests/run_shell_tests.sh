#!/bin/bash

# Shell Command Test Suite Runner
# Tests the backtick shell command functionality in KAI Console

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KAI_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CONSOLE_BIN="$KAI_DIR/build/Bin/Console"
RESULTS_DIR="$SCRIPT_DIR/results"
LOG_FILE="$RESULTS_DIR/test_results.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create results directory
mkdir -p "$RESULTS_DIR"

# Initialize log file
echo "Shell Command Test Suite - $(date)" > "$LOG_FILE"
echo "==========================================" >> "$LOG_FILE"

# Test counter
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a single test file
run_test_file() {
    local test_file="$1"
    local test_name="$(basename "$test_file" .txt)"
    
    echo -e "${BLUE}Running test: $test_name${NC}"
    echo "Running test: $test_name" >> "$LOG_FILE"
    echo "----------------------------------------" >> "$LOG_FILE"
    
    # Count tests in file (lines starting with # Test)
    local test_count=0
    if [ -f "$test_file" ]; then
        test_count=$(grep -c "^# Test" "$test_file" 2>/dev/null || true)
        # If grep returns empty or 0, default to counting non-empty, non-comment lines as a rough estimate
        if [ -z "$test_count" ] || [ "$test_count" -eq 0 ]; then
            test_count=$(grep -v '^$' "$test_file" | grep -v '^#' | wc -l || echo 1)
        fi
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + test_count))
    
    # Run the test
    local output_file="$RESULTS_DIR/${test_name}_output.txt"
    local error_file="$RESULTS_DIR/${test_name}_error.txt"
    
    if cat "$test_file" | "$CONSOLE_BIN" > "$output_file" 2> "$error_file"; then
        echo -e "${GREEN}✓ $test_name completed${NC}"
        echo "✓ $test_name completed" >> "$LOG_FILE"
        PASSED_TESTS=$((PASSED_TESTS + test_count))
    else
        echo -e "${RED}✗ $test_name failed${NC}"
        echo "✗ $test_name failed" >> "$LOG_FILE"
        FAILED_TESTS=$((FAILED_TESTS + test_count))
    fi
    
    # Append output to log
    echo "Output:" >> "$LOG_FILE"
    cat "$output_file" >> "$LOG_FILE" 2>/dev/null || echo "No output" >> "$LOG_FILE"
    
    if [ -s "$error_file" ]; then
        echo "Errors:" >> "$LOG_FILE"
        cat "$error_file" >> "$LOG_FILE"
    fi
    
    echo "" >> "$LOG_FILE"
}

# Function to run specific verification tests
run_verification_tests() {
    echo -e "${BLUE}Running verification tests...${NC}"
    echo "Running verification tests..." >> "$LOG_FILE"
    
    # Test 1: Verify shell command output
    echo -e "${YELLOW}Test V1: Shell command output verification${NC}"
    local expected_pwd="$PWD"
    # Filter out prompt lines and extract just the pwd output
    local actual_pwd=$(echo '`pwd`' | "$CONSOLE_BIN" 2>/dev/null | grep -v "Pi " | grep -v "^$" | head -1 | xargs)
    
    if [ "$actual_pwd" = "$expected_pwd" ]; then
        echo -e "${GREEN}✓ V1: pwd output correct${NC}"
        echo "✓ V1: pwd output correct" >> "$LOG_FILE"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ V1: pwd output incorrect. Expected: $expected_pwd, Got: $actual_pwd${NC}"
        echo "✗ V1: pwd output incorrect. Expected: $expected_pwd, Got: $actual_pwd" >> "$LOG_FILE"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Test 2: Verify embedded command expansion
    echo -e "${YELLOW}Test V2: Embedded command expansion verification${NC}"
    # Look for the stack output showing [0]: 8
    local result=$(echo -e '5 `echo 3` +\n.' | "$CONSOLE_BIN" 2>/dev/null | grep '\[0\]: 8' | head -1)
    
    if [ -n "$result" ]; then
        echo -e "${GREEN}✓ V2: Embedded command expansion works (5 + 3 = 8)${NC}"
        echo "✓ V2: Embedded command expansion works (5 + 3 = 8)" >> "$LOG_FILE"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ V2: Embedded command expansion failed${NC}"
        echo "✗ V2: Embedded command expansion failed" >> "$LOG_FILE"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Test 3: Verify no closing backtick works
    echo -e "${YELLOW}Test V3: No closing backtick verification${NC}"
    local expected_user=$(whoami)
    # Filter out prompt lines and extract just the whoami output
    local user_output=$(echo '`whoami' | "$CONSOLE_BIN" 2>/dev/null | grep -v "Pi " | grep -v "^$" | head -1 | xargs)
    
    if [ "$user_output" = "$expected_user" ]; then
        echo -e "${GREEN}✓ V3: No closing backtick works${NC}"
        echo "✓ V3: No closing backtick works" >> "$LOG_FILE"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ V3: No closing backtick failed${NC}"
        echo "✗ V3: No closing backtick failed" >> "$LOG_FILE"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
}

# Main execution
echo -e "${BLUE}Shell Command Test Suite${NC}"
echo -e "${BLUE}========================${NC}"

# Check if console binary exists
if [ ! -f "$CONSOLE_BIN" ]; then
    echo -e "${RED}Error: Console binary not found at $CONSOLE_BIN${NC}"
    echo "Please build the console first with: cd $KAI_DIR/build && ninja Console"
    exit 1
fi

# Run all test files
echo -e "${YELLOW}Running test files...${NC}"
for test_file in "$SCRIPT_DIR"/test_*.txt; do
    if [ -f "$test_file" ]; then
        run_test_file "$test_file"
    fi
done

# Run verification tests
run_verification_tests

# Summary
echo "" >> "$LOG_FILE"
echo "Test Summary:" >> "$LOG_FILE"
echo "=============" >> "$LOG_FILE"
echo "Total Tests: $TOTAL_TESTS" >> "$LOG_FILE"
echo "Passed: $PASSED_TESTS" >> "$LOG_FILE"
echo "Failed: $FAILED_TESTS" >> "$LOG_FILE"

echo ""
echo -e "${BLUE}Test Summary:${NC}"
echo -e "${BLUE}=============${NC}"
echo -e "Total Tests: $TOTAL_TESTS"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"

if [ $FAILED_TESTS -gt 0 ]; then
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
    echo ""
    echo -e "${YELLOW}Check $LOG_FILE for detailed results${NC}"
    exit 1
else
    echo -e "${RED}Failed: $FAILED_TESTS${NC}"
    echo ""
    echo -e "${GREEN}All tests passed!${NC}"
    echo -e "${YELLOW}Detailed results in $LOG_FILE${NC}"
    exit 0
fi