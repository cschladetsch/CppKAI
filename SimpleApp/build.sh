#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Use clang++ by default
COMPILER=${1:-clang++}

echo -e "${YELLOW}Building a simple application to test the core libs${NC}"
mkdir -p SimpleApp/build

cd SimpleApp
cat > main.cpp << 'EOF'
#include <KAI/Core/Base.h>
#include <iostream>

using namespace kai;

int main() {
    std::cout << "KAI Core headers included successfully!" << std::endl;
    return 0;
}
EOF

echo -e "${YELLOW}Compiling with ${COMPILER}...${NC}"
${COMPILER} -std=c++17 -I../Include main.cpp -o build/simple_test

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${YELLOW}Running the test application...${NC}"
    ./build/simple_test
else
    echo -e "${RED}Build failed!${NC}"
fi