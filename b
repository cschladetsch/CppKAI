#!/bin/bash

# Build script for KAI project
# Usage: ./b [options]
# Options:
#   --clean       Clean build directory before building
#   --reconfigure Force CMake reconfiguration
#   --no-color    Disable colored output
#   --no-ninja    Use default CMake generator instead of Ninja
#   --gcc         Use GCC instead of Clang++

# Check if help is requested
for arg in "$@"; do
  if [ "$arg" == "--help" ] || [ "$arg" == "-h" ]; then
    echo "Usage: ./b [options]"
    echo "Options:"
    echo "  --clean       Clean build directory before building"
    echo "  --reconfigure Force CMake reconfiguration"
    echo "  --no-color    Disable colored output"
    echo "  --no-ninja    Use default CMake generator instead of Ninja"
    echo "  --gcc         Use GCC instead of Clang++"
    echo "  --help, -h    Show this help message"
    exit 0
  fi
done

# Check if --no-color flag is provided
USE_COLOR=true
USE_NINJA=true
USE_GCC=false

for arg in "$@"; do
  if [ "$arg" == "--no-color" ]; then
    USE_COLOR=false
  fi
  if [ "$arg" == "--no-ninja" ]; then
    USE_NINJA=false
  fi
  if [ "$arg" == "--gcc" ]; then
    USE_GCC=true
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

# Check if --clean flag is provided
CLEAN_BUILD=false
for arg in "$@"; do
  if [ "$arg" == "--clean" ]; then
    CLEAN_BUILD=true
  fi
done

# Only clean if requested
if [ "$CLEAN_BUILD" = true ]; then
  echo -e "${YELLOW}Cleaning build directory ($BUILD_DIR)...${NC}"
  rm -rf "$BUILD_DIR"
fi

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
  echo -e "${YELLOW}Creating build directory ($BUILD_DIR)...${NC}"
  mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR" || { echo -e "${RED}Failed to enter build directory${NC}"; exit 1; }

# Create bin directories
mkdir -p Bin/Test

# Check if we need to configure (CMakeCache.txt doesn't exist)
NEED_CONFIGURE=false
if [ ! -f "CMakeCache.txt" ]; then
  NEED_CONFIGURE=true
fi

# Also reconfigure if --reconfigure flag is provided
for arg in "$@"; do
  if [ "$arg" == "--reconfigure" ]; then
    NEED_CONFIGURE=true
  fi
done

SRC_DIR=$(dirname $(pwd))

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="${SRC_DIR}/Bin" \
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="${SRC_DIR}/Bin" \
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="${SRC_DIR}/Bin" \
      -DBIN_HOME="${SRC_DIR}/Bin" \
      -DINCLUDE_HOME="${SRC_DIR}/Include/KAI""

# Set compiler to clang++ by default unless --gcc flag was provided
if [ "$USE_GCC" = false ]; then
  echo -e "${BLUE}Using Clang++ compiler${NC}"
  export CXX=clang++
  export CC=clang
  CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang"
else
  echo -e "${BLUE}Using GCC compiler${NC}"
  export CXX=g++
  export CC=gcc
  CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc"
fi

if [ "$NEED_CONFIGURE" = true ]; then
  echo -e "${YELLOW}Configuring with CMake...${NC}"
  
  if [ "$USE_NINJA" = true ]; then
    which ninja > /dev/null 2>&1
    if [ $? -eq 0 ]; then
      echo -e "${BLUE}Using Ninja build system${NC}"
      cmake .. -G Ninja $CMAKE_ARGS
    else
      echo -e "${YELLOW}Ninja not found, falling back to default generator${NC}"
      cmake .. $CMAKE_ARGS
    fi
  else
    cmake .. $CMAKE_ARGS
  fi

  if [ $? -ne 0 ]; then
      echo -e "${RED}CMake configuration failed!${NC}"
      cd ..
      exit 1
  fi
else
  echo -e "${BLUE}Using existing CMake configuration (use --reconfigure to force reconfiguration)${NC}"
fi

# Build with the appropriate tool
echo -e "${YELLOW}Building project with $(nproc) parallel jobs...${NC}"
cmake --build . -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    cd ..
    exit 1
fi

cd ..

echo -e "${GREEN}Build successful!${NC}"
echo -e "${YELLOW}Executables can be found in Bin/${NC}"
echo -e "${BLUE}Main executables:${NC}"
ls -la Bin/*.exe 2>/dev/null || ls -la Bin/* 2>/dev/null | grep -E '^-..x' | awk '{print "  - " $9}'