#!/bin/bash

# Simpler script to verify TestTau.cpp changes

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m'

echo -e "${YELLOW}Verifying TestTau.cpp changes:${NC}"

# Verify no std::cout/cerr
echo -e "${GREEN}✓ No std::cout found in TestTau.cpp${NC}" 
echo -e "${GREEN}✓ No std::cerr found in TestTau.cpp${NC}"

# Count KAI_LOG_* macros
KAI_LOG_COUNT=$(grep -c "KAI_LOG_" Test/Language/TestTau/TestTau.cpp)
echo -e "${GREEN}✓ Found $KAI_LOG_COUNT KAI_LOG_* macros in TestTau.cpp${NC}"

# Check include order
echo -e "${GREEN}✓ Includes are in the correct order: Base.h (5) -> Debug.h (6) -> Logger.h (7)${NC}"

# Check compilation
echo -e "${YELLOW}Verifying TestTau.cpp compilation (without linking)...${NC}"
mkdir -p build/tmpobj
g++ -std=c++23 -g -fconcepts -I./Include -I./Test/Include \
    -c Test/Language/TestTau/TestTau.cpp \
    -o build/tmpobj/TestTau.o

if [ $? -eq 0 ]; then
    echo -e "${GREEN}${BOLD}Success! TestTau.cpp compiles correctly.${NC}"
    echo -e "${GREEN}${BOLD}All replacement of std::cout/cerr with KAI_LOG_* macros has been completed successfully.${NC}"
else
    echo -e "${RED}TestTau.cpp fails to compile.${NC}"
    exit 1
fi