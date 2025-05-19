#!/bin/bash

# Build and run the Tau connection tests

# Navigate to build directory
cd "$(dirname "$0")/.."
BUILD_DIR=./build

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

# Build the Tau tests
echo "Building Tau connection tests..."
make TestTau -j$(nproc)

# Run the Tau connection tests
echo "Running Tau connection tests..."
./bin/Test/TestTau --gtest_filter="TestNetworkConnection.*"

# Return the exit code
exit $?