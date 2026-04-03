#!/bin/bash

# Script to build just the Tau test with proper linking
set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Ensuring Bin/Test directory exists...${NC}"
mkdir -p Bin/Test

# First, make sure the libraries are built
echo -e "${YELLOW}Building required libraries...${NC}"
cd build
cmake --build . --target Core
cmake --build . --target CommonLang
cmake --build . --target Executor
cmake --build . --target TauLang
cd ..

# Create a direct compilation of TestTau.cpp - override CMake's approach
echo -e "${YELLOW}Compiling TestTau object files directly...${NC}"

# Create test files directory
mkdir -p build/tmpobj

# Use clang++ by default, fallback to g++
if command -v clang++ >/dev/null 2>&1; then
    CXX_COMPILER="clang++"
    COMPILER_FLAGS="-fcolor-diagnostics"
    echo -e "${GREEN}Using Clang++ compiler for direct build${NC}"
elif command -v g++ >/dev/null 2>&1; then
    CXX_COMPILER="g++"
    COMPILER_FLAGS="-fdiagnostics-color=always"
    echo -e "${YELLOW}Clang++ not found, using GCC++ for direct build${NC}"
else
    echo -e "${RED}Neither clang++ nor g++ found!${NC}"
    exit 1
fi

# Common compiler flags
COMMON_FLAGS="-std=c++20 -g -Wall -Wno-deprecated -Wno-switch -Wno-comment -Wno-reorder -Wno-unused-parameter \
    -Wno-missing-field-initializers -Wno-unknown-pragmas -Wno-unused-value -Wno-unused-but-set-variable \
    $COMPILER_FLAGS -I./Include -I./Test/Include"

# Compile each source file individually
echo -e "${YELLOW}Compiling TestTau.cpp...${NC}"
$CXX_COMPILER $COMMON_FLAGS \
    -c Test/Language/TestTau/TestTau.cpp \
    -o build/tmpobj/TestTau.o

echo -e "${YELLOW}Compiling TestLangCommon.cpp...${NC}"
$CXX_COMPILER $COMMON_FLAGS \
    -c Test/Language/TestLangCommon.cpp \
    -o build/tmpobj/TestLangCommon.o

echo -e "${YELLOW}Compiling TestCommon.cpp...${NC}"
$CXX_COMPILER $COMMON_FLAGS \
    -c Test/Common/TestCommon.cpp \
    -o build/tmpobj/TestCommon.o

echo -e "${YELLOW}Compiling MyTestStruct.cpp...${NC}"
$CXX_COMPILER $COMMON_FLAGS \
    -c Test/Common/MyTestStruct.cpp \
    -o build/tmpobj/MyTestStruct.o

echo -e "${YELLOW}Compiling Main.cpp...${NC}"
$CXX_COMPILER $COMMON_FLAGS \
    -c Test/Source/Main.cpp \
    -o build/tmpobj/Main.o

# Now link all together
echo -e "${YELLOW}Linking TestTau executable...${NC}"
$CXX_COMPILER -std=c++20 \
    build/tmpobj/TestTau.o \
    build/tmpobj/TestLangCommon.o \
    build/tmpobj/TestCommon.o \
    build/tmpobj/MyTestStruct.o \
    build/tmpobj/Main.o \
    -o Bin/Test/TestTau \
    -L./Bin -lCore -lCommonLang -lExecutor -lTauLang -lPiLang -lRhoLang -lImGui \
    -lgtest -lgtest_main -lpthread \
    -lboost_system -lboost_filesystem -lboost_program_options -lboost_date_time -lboost_regex -lboost_atomic

if [ -f Bin/Test/TestTau ]; then
    echo -e "${GREEN}Successfully built TestTau!${NC}"
    chmod +x Bin/Test/TestTau
    echo -e "${YELLOW}Running TestTau to verify it works...${NC}"
    LD_LIBRARY_PATH=./Bin Bin/Test/TestTau
else
    echo -e "${RED}Failed to build TestTau!${NC}"
    exit 1
fi