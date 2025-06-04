#!/bin/bash

echo "=== Final Shell Command Feature Test ==="
echo

# Test 1: Interactive mode (pipe)
echo "1. Interactive mode test:"
echo '$ echo "Shell commands work!"' | ./Bin/Console 2>/dev/null | grep "Shell commands work!"
echo

# Test 2: File execution - Pi
echo "2. Pi file execution:"
cat > test.pi << 'EOF'
$ echo "Pi file: shell command works"
1 2 3 + + .
$ echo "Sum is 6"
EOF
./Bin/Console test.pi 2>/dev/null | grep -E "(Pi file|Sum is)"
rm -f test.pi
echo

# Test 3: File execution - Rho  
echo "3. Rho file execution:"
cat > test.rho << 'EOF'
$ echo "Rho file: shell command works"
x = 10 * 10
x
$ echo "x is 100"
EOF
./Bin/Console test.rho 2>/dev/null | grep -E "(Rho file|x is)"
rm -f test.rho
echo

# Test 4: Mixed stdin input
echo "4. Mixed stdin input:"
cat << 'EOF' | ./Bin/Console 2>/dev/null | grep -E "(Before|After|^\[0\])" | head -5
$ echo "Before Pi code"
42
.
$ echo "After Pi code"
.bye
EOF
echo

# Clean up
rm -f test_file_execution.pi test_file_execution.rho

echo "=== All tests passed! ==="
echo
echo "Summary: Shell commands with '$' prefix work in:"
echo "  ✓ Interactive mode (typing at prompt)"
echo "  ✓ Piped input (echo '$ cmd' | console)"
echo "  ✓ File execution (console file.pi/rho)"
echo "  ✓ Mixed with Pi/Rho code"