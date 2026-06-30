#!/bin/bash

# Simple script to test Tau network interfaces

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"

# Make sure the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Create a temporary standalone test file
TEST_DIR="$ROOT_DIR/Test/Standalone"
mkdir -p "$TEST_DIR"

# Create a simple CMakeLists.txt for the standalone test
cat > "$TEST_DIR/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(TauNetworkTest)

# Find required packages
find_package(GTest REQUIRED)

# Source files
set(SOURCES
    TauNetworkTest.cpp
)

# Copy the relevant files from the main project
file(COPY ${CMAKE_SOURCE_DIR}/../Test/Language/TestTau/TauNetworkTests.cpp 
     DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/../Test/Language/TestLangCommon.cpp
     DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/../Test/Language/TestLangCommon.h
     DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/../Test/Language/TestTau/Scripts/Connection
     DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Scripts)

# Rename the file to match our expected name
file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/TauNetworkTests.cpp 
      ${CMAKE_CURRENT_SOURCE_DIR}/TauNetworkTest.cpp)

# Add the executable
add_executable(TauNetworkTest ${SOURCES})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${GTEST_INCLUDE_DIRS})

# Link libraries
target_link_libraries(TauNetworkTest 
    ${CMAKE_SOURCE_DIR}/../build/Bin/libTauLang.a
    ${CMAKE_SOURCE_DIR}/../build/Bin/libCore.a
    ${CMAKE_SOURCE_DIR}/../build/Bin/libCommonLang.a
    ${GTEST_LIBRARIES}
    pthread
)

# Enable testing
enable_testing()
add_test(NAME TauNetworkTest COMMAND TauNetworkTest)
EOF

# Go to standalone test directory
cd "$TEST_DIR"

# Configure and build
cmake .
make

# Run the test
./TauNetworkTest