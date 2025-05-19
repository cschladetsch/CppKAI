#!/bin/bash
# Script to build KAI using proper out-of-source build

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create build directory
BUILD_DIR="$(pwd)/build"
echo -e "${YELLOW}Using build directory: ${BUILD_DIR}${NC}"

# Clean build if requested
if [ "$1" == "clean" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "${BUILD_DIR}"
    shift
fi

# Create build directory if it doesn't exist
mkdir -p "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}/Bin"

# Output user information
echo -e "${BLUE}=== Building KAI with out-of-source build ===${NC}"
echo -e "${BLUE}Source directory: $(pwd)${NC}"
echo -e "${BLUE}Build directory: ${BUILD_DIR}${NC}"
echo -e "${BLUE}Binary output directory: ${BUILD_DIR}/Bin${NC}"

# Navigate to build directory
cd "${BUILD_DIR}" || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Configure with CMake
echo -e "${YELLOW}Running CMake configuration...${NC}"
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${BUILD_DIR}/Bin" \
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="${BUILD_DIR}/Bin" \
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="${BUILD_DIR}/Bin" \
      -DBIN_HOME="${BUILD_DIR}/Bin" \
      .. "$@" || { echo -e "${RED}CMake configuration failed${NC}"; exit 1; }

# Build the project
echo -e "${YELLOW}Building project...${NC}"
cmake --build . || { echo -e "${RED}Build failed${NC}"; exit 1; }

# Print success message
echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Executables and libraries can be found in:${NC}"
echo -e "${GREEN}${BUILD_DIR}/Bin${NC}"

# List built executables
echo -e "${YELLOW}Built executables:${NC}"
find "${BUILD_DIR}/Bin" -type f -executable -not -path "*/CMakeFiles/*" | sort