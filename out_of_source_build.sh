#!/bin/bash
# Script to build KAI using proper out-of-source build

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for clean build request
if [ "$1" == "clean" ]; then
    echo -e "${YELLOW}Removing previous build directory...${NC}"
    rm -rf build
    shift
fi

# Create build directory if it doesn't exist
mkdir -p build
cd build || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Configure with CMake 
echo -e "${YELLOW}Configuring with CMake...${NC}"
# Set explicit binary directory to ensure out-of-source build
cmake -DCMAKE_BINARY_DIR="$(pwd)" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DBUILD_GCC=ON \
      -DBIN_HOME="$(pwd)/Bin" \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$(pwd)/Bin" \
      .. "$@" || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . || { echo -e "${RED}Build failed${NC}"; exit 1; }

echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Executables can be found in: $(pwd)/Bin${NC}"