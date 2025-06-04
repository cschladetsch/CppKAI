#!/bin/bash

# Console Zsh Features Test Script
# This script tests all the zsh-like features implemented in the Console

CONSOLE_BIN="/home/xian/local/KAI/Bin/Console"
TEST_OUTPUT="console_test_results.txt"
TEMP_INPUT="temp_test_input.txt"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=== Testing Console Zsh-like Features ===" | tee $TEST_OUTPUT
echo "Date: $(date)" | tee -a $TEST_OUTPUT
echo "" | tee -a $TEST_OUTPUT

# Function to run a test
run_test() {
    local test_name="$1"
    local commands="$2"
    local expected_pattern="$3"
    
    echo -e "${YELLOW}Test: $test_name${NC}" | tee -a $TEST_OUTPUT
    echo "Commands:" | tee -a $TEST_OUTPUT
    echo "$commands" | tee -a $TEST_OUTPUT
    
    # Create input file
    echo "$commands" > $TEMP_INPUT
    
    # Run console with input
    local output=$(timeout 5s $CONSOLE_BIN < $TEMP_INPUT 2>&1)
    
    echo "Output:" | tee -a $TEST_OUTPUT
    echo "$output" | tee -a $TEST_OUTPUT
    
    # Check if output contains expected pattern
    if echo "$output" | grep -q "$expected_pattern"; then
        echo -e "${GREEN}✓ PASS${NC}" | tee -a $TEST_OUTPUT
        return 0
    else
        echo -e "${RED}✗ FAIL - Expected pattern not found: $expected_pattern${NC}" | tee -a $TEST_OUTPUT
        return 1
    fi
    echo "" | tee -a $TEST_OUTPUT
}

# Test counter
PASSED=0
FAILED=0

# Test 1: Basic !! (repeat last command)
if run_test "Basic !! expansion" \
"1 2 +
!!" \
"=> 1 2 +"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 2: !n (execute nth command)
if run_test "!n expansion" \
"print \"first\"
print \"second\"
print \"third\"
!2" \
"=> print \"second\""; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 3: !-n (execute nth from end)
if run_test "!-n expansion" \
"print \"one\"
print \"two\"
print \"three\"
!-2" \
"=> print \"two\""; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 4: !string (last command starting with string)
if run_test "!string expansion" \
"print \"hello\"
1 2 +
print \"world\"
!print" \
"=> print \"world\""; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 5: Word designator :0 (command word)
if run_test "Word designator :0" \
"echo one two three
!!:0" \
"=> echo"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 6: Word designator :^ (first argument)
if run_test "Word designator :^" \
"echo one two three
!!:^" \
"=> one"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 7: Word designator :$ (last argument)
if run_test "Word designator :$" \
"echo one two three
!!:$" \
"=> three"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 8: Word designator :* (all arguments)
if run_test "Word designator :*" \
"echo one two three
!!:*" \
"=> one two three"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 9: Word designator range :1-2
if run_test "Word designator range :1-2" \
"echo one two three four
!!:1-2" \
"=> one two"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 10: Word designator :2* (from word 2 to end)
if run_test "Word designator :2*" \
"echo one two three four
!!:2*" \
"=> two three four"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 11: Complex example !-3:4*
if run_test "Complex history reference !-3:4*" \
"ls -la /home/user file1.txt file2.txt file3.txt
cd /tmp
pwd
!-3:4*" \
"=> file1.txt file2.txt file3.txt"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 12: $ prefix disables expansion
if run_test "$ prefix disables expansion" \
"print \"test\"
\$!!" \
"!!"; then  # Should see literal !! in output, not expansion
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 13: Shell command with backticks
if run_test "Backtick shell command" \
"\`echo hello\`" \
"hello"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 14: Embedded shell commands
if run_test "Embedded shell commands" \
"print \"\`echo embedded\`\"" \
"embedded"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 15: History expansion in commands
if run_test "History expansion within command" \
"5
!! * 2" \
"=> 5 \* 2"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 16: Non-existent history
if run_test "Non-existent history returns error" \
"!999" \
"No matching command in history"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 17: Empty history !!
if run_test "Empty history !!" \
"!!" \
"No matching command in history"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 18: Quoted strings in word splitting
if run_test "Quoted strings in word splitting" \
"echo \"hello world\" \"goodbye moon\"
!!:1" \
"=> \"hello world\""; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 19: Multiple history references
if run_test "Multiple history references" \
"10
20
!-2 + !-1" \
"=> 10 + 20"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Test 20: Word designator on specific position
if run_test "Word designator specific position" \
"command arg1 arg2 arg3 arg4 arg5
!!:3" \
"=> arg3"; then
    ((PASSED++))
else
    ((FAILED++))
fi

# Clean up
rm -f $TEMP_INPUT

# Summary
echo "" | tee -a $TEST_OUTPUT
echo "=== Test Summary ===" | tee -a $TEST_OUTPUT
echo -e "Passed: ${GREEN}$PASSED${NC}" | tee -a $TEST_OUTPUT
echo -e "Failed: ${RED}$FAILED${NC}" | tee -a $TEST_OUTPUT
TOTAL=$((PASSED + FAILED))
PERCENTAGE=$((PASSED * 100 / TOTAL))
echo "Success Rate: $PERCENTAGE%" | tee -a $TEST_OUTPUT

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}" | tee -a $TEST_OUTPUT
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}" | tee -a $TEST_OUTPUT
    exit 1
fi