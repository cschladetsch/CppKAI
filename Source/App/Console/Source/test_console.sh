#!/bin/bash

# Test Console zsh features
echo "Testing Console zsh features..."

# Test basic commands and history
cat << 'EOF' | ../../../../build/Bin/Console
2 3 +
!!
!1
exit
EOF

echo -e "\n\nTesting shell integration..."
# Test shell integration
cat << 'EOF' | ../../../../build/Bin/Console
$ pwd
$ ls -la | head -3
sh
pwd
ls *.md
exit
exit
EOF

echo -e "\n\nTesting quick substitution..."
# Test quick substitution
cat << 'EOF' | ../../../../build/Bin/Console
print "Hello World"
^Hello^Greetings
exit
EOF

echo -e "\n\nTest complete!"