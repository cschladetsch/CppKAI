#!/bin/bash

# Check if --no-color flag is provided
USE_COLOR=true
for arg in "$@"; do
  if [ "$arg" == "--no-color" ]; then
    USE_COLOR=false
    break
  fi
done

# Colors for output
if [ "$USE_COLOR" = true ]; then
  RED='\033[0;31m'
  GREEN='\033[0;32m'
  YELLOW='\033[1;33m'
  BLUE='\033[0;34m'
  NC='\033[0m' # No Color
else
  RED=''
  GREEN=''
  YELLOW=''
  BLUE=''
  NC=''
fi

# Define the build directory
BUILD_DIR="build"

# Clean and recreate the build directory
echo -e "${YELLOW}Creating clean build directory ($BUILD_DIR)...${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Create bin directories
mkdir -p Bin/Test

# Run CMake with simple, direct configuration focused on fixing the path issue
echo -e "${YELLOW}Configuring with CMake...${NC}"
SRC_DIR=$(dirname $(pwd))
cmake .. \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$(pwd)/Bin" \
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$(pwd)/Bin" \
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$(pwd)/Bin" \
      -DBIN_HOME="${HOME}/Bin" \
      -DINCLUDE_HOME="${SRC_DIR}/Include/KAI"

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    cd ..
    exit 1
fi

# Build with the appropriate tool
echo -e "${YELLOW}Building project...${NC}"
cmake --build . -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    cd ..
    exit 1
fi

cd ..

echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Executables can be found in ${BUILD_DIR}/Bin${NC}"