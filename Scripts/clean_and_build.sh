#!/bin/bash
# Script to clean build artifacts and build with proper separation

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if a build directory is specified
BUILD_DIR="build"
if [ "$1" != "" ]; then
    BUILD_DIR="$1"
    shift
fi

echo -e "${YELLOW}Using build directory: ${BUILD_DIR}${NC}"

# Clean previous build artifacts
echo -e "${YELLOW}Cleaning previous build directory...${NC}"
rm -rf "${BUILD_DIR}"
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake

# Create clean build directory
echo -e "${YELLOW}Creating fresh build directory...${NC}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}" || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Configure with CMake
echo -e "${YELLOW}Running CMake configuration...${NC}"
cmake .. "$@" || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . || { echo -e "${RED}Build failed${NC}"; exit 1; }

# Print success message
echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Executables can be found in: ${BUILD_DIR}/Bin${NC}"