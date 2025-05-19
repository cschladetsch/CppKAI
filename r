#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
WHITE='\033[0;39m'
NC='\033[0m' # No Color

# Define the Ninja-specific build directory
BUILD_DIR="build_ninja"
FULL_BUILD_DIR="$(pwd)/${BUILD_DIR}"

# Clean and recreate the build directory
echo -e "${YELLOW}Creating clean build directory ($BUILD_DIR)...${NC}"
rm -rf "$BUILD_DIR" 

# Also clean root-level CMake cache files that might conflict
echo -e "${YELLOW}Cleaning root-level CMake artifacts...${NC}"
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile

# Create and enter build directory
mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR" || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Create necessary bin directories
mkdir -p "Bin/Test"
mkdir -p "${HOME}/Bin/Test"

# Create a custom toolchain file to ensure correct paths
cat > toolchain.cmake << EOF
set(CMAKE_CXX_COMPILER $(which c++))
set(CMAKE_C_COMPILER $(which cc))

# Ensure all binary outputs go to the bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${HOME}/Bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${HOME}/Bin") 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${HOME}/Bin")
EOF

# Run CMake with Ninja generator using our toolchain file
echo -e "${YELLOW}Configuring with CMake and Ninja generator...${NC}"
SRC_DIR=$(dirname $(pwd))
cmake -G Ninja .. \
      -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBIN_HOME="${HOME}/Bin" \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${HOME}/Bin" \
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="${HOME}/Bin" \
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="${HOME}/Bin" \
      -DINCLUDE_HOME="${SRC_DIR}/Include/KAI"

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    cd ..
    exit 1
fi

# Build with the appropriate tool based on OS
if [[ "$OSTYPE" == "msys"* ]]; then
    echo -e "${YELLOW}Building with MSBuild on Windows...${NC}"
    msbuild.exe *.sln
    bin_path="/Debug"
else
    echo -e "${YELLOW}Building with Ninja...${NC}"
    ninja -v  # More verbose output to see what's happening
    if [ $? -ne 0 ]; then
        echo -e "${RED}Ninja build failed!${NC}"
        cd ..
        exit 1
    fi
fi

cd ..

echo -e "${GREEN}Build successful!${NC}"

echo -e "${YELLOW}Running tests...${NC}"
./run_tests 
test_result=$?

echo -e "${BLUE}OSTYPE=$OSTYPE${NC}"

echo 
echo -e "${PURPLE}---- Running Kai Console ----${NC}"
echo -e "${WHITE}"

if [ -x "${HOME}/Bin/Console" ]; then
    # Run from the home Bin directory
    "${HOME}/Bin/Console"
elif [ -x "${FULL_BUILD_DIR}/Bin/Console" ]; then
    # Try the build directory
    "${FULL_BUILD_DIR}/Bin/Console"
elif [ -x "./Bin/Console" ]; then
    # Fallback to the traditional location
    ./Bin/Console
else
    echo -e "${RED}Error: Could not find Console executable!${NC}"
    exit 1
fi

if [ $test_result -ne 0 ]; then
    echo -e "${RED}Warning: Tests failed with exit code $test_result${NC}"
fi