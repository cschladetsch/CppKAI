#!/bin/bash

# Script to build and run the Rho language demo

# Set terminal colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Building KAI system and Rho demo...${NC}"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Generate build files
echo -e "${YELLOW}Generating build files...${NC}"
cmake ..

# Build the Rho test suite
echo -e "${YELLOW}Building tests...${NC}"
make TestRho -j4

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed! Unable to run demo.${NC}"
    exit 1
fi

# Run just the demo test
echo -e "${YELLOW}Running Rho language demo...${NC}"
./bin/Test/TestRho --gtest_filter=RhoDemo.RunDemo

if [ $? -ne 0 ]; then
    echo -e "${RED}Demo execution failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Demo completed successfully!${NC}"
echo -e "${YELLOW}You can find the demo script at: Test/Language/TestRho/Scripts/Demo.rho${NC}"