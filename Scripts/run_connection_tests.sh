#!/bin/bash

# Build and run the connection tests

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"

# Make sure the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Enter build directory
cd "$BUILD_DIR"

# Configure with CMake if necessary
if [ ! -f "Makefile" ]; then
    echo "Configuring with CMake..."
    cmake ..
fi

echo "Building the minimal console applications..."
# Build the MinimalServer and MinimalClient applications first
cmake --build . --target MinimalServer MinimalClient

echo "Building Tau language library..."
cmake --build . --target TauLang

echo "Building network test executables..."
# First make sure any necessary dependencies are built
cmake --build . --target Test_Network

echo "Running network tests..."
if [ -f "bin/Test/Test_Network" ]; then
    ./bin/Test/Test_Network --gtest_filter="*Connection*"
else
    echo "WARNING: Test_Network executable not found."
fi

echo "Building Tau connection tests..."
# Use cmake --build instead of make for better cross-platform compatibility
cmake --build . --target TestTau

echo "Running Tau connection tests..."
if [ -f "bin/Test/TestTau" ]; then
    ./bin/Test/TestTau --gtest_filter="TestNetworkConnection.*"
else
    echo "WARNING: TestTau executable not found."
fi

# Return the exit code
exit $?