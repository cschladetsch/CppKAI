#!/bin/bash

# Run Window Application Tests
# This script builds and runs all Window tests with various options

set -e  # Exit on error

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}KAI Window Application Test Runner${NC}"
echo "=================================="

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake if needed
if [ ! -f "Makefile" ]; then
    echo -e "${YELLOW}Configuring with CMake...${NC}"
    cmake ..
fi

# Build the tests
echo -e "${YELLOW}Building tests...${NC}"
make -j$(nproc)

# Run tests based on command line arguments
if [ $# -eq 0 ]; then
    # No arguments - run all tests
    echo -e "${GREEN}Running all Window tests...${NC}"
    ./TestWindow
elif [ "$1" == "core" ]; then
    echo -e "${GREEN}Running core functionality tests...${NC}"
    ./TestWindow --gtest_filter=ExecutorWindowTest.*
elif [ "$1" == "ui" ]; then
    echo -e "${GREEN}Running UI tests...${NC}"
    ./TestWindow --gtest_filter=WindowUITest.*
elif [ "$1" == "perf" ]; then
    echo -e "${GREEN}Running performance tests...${NC}"
    ./TestWindow --gtest_filter=WindowPerformanceTest.*
elif [ "$1" == "pi" ]; then
    echo -e "${GREEN}Running Pi tab tests...${NC}"
    ./TestWindow --gtest_filter=*PiTab*
elif [ "$1" == "rho" ]; then
    echo -e "${GREEN}Running Rho tab tests...${NC}"
    ./TestWindow --gtest_filter=*RhoTab*
elif [ "$1" == "debug" ]; then
    echo -e "${GREEN}Running Debugger tab tests...${NC}"
    ./TestWindow --gtest_filter=*DebuggerTab*:*Debugger*
elif [ "$1" == "integration" ]; then
    echo -e "${GREEN}Running integration tests...${NC}"
    ./TestWindow --gtest_filter=*Integration*
elif [ "$1" == "stress" ]; then
    echo -e "${GREEN}Running stress tests...${NC}"
    ./TestWindow --gtest_filter=*Stress*:*Performance*
elif [ "$1" == "verbose" ]; then
    echo -e "${GREEN}Running all tests with verbose output...${NC}"
    ./TestWindow --gtest_print_time=1 --gtest_color=yes
elif [ "$1" == "help" ]; then
    echo "Usage: $0 [option]"
    echo ""
    echo "Options:"
    echo "  (none)       Run all tests"
    echo "  core         Run core functionality tests"
    echo "  ui           Run UI tests"
    echo "  perf         Run performance tests"
    echo "  pi           Run Pi tab tests"
    echo "  rho          Run Rho tab tests"
    echo "  debug        Run Debugger tab tests"
    echo "  integration  Run integration tests"
    echo "  stress       Run stress and performance tests"
    echo "  verbose      Run all tests with verbose output"
    echo "  help         Show this help message"
    echo ""
    echo "You can also pass custom gtest filters:"
    echo "  $0 \"--gtest_filter=ExecutorWindowTest.PiTab_BasicArithmetic\""
else
    # Pass through custom arguments
    echo -e "${GREEN}Running tests with custom arguments...${NC}"
    ./TestWindow "$@"
fi

# Check test results
if [ $? -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi