#!/bin/bash

# Script to build and test network components
set -e  # Exit on error

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"

# Make sure the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DKAI_BUILD_TEST_NETWORK=ON
else
    cd "$BUILD_DIR"
    # Ensure CMAKE_BUILD_TYPE is set
    cmake .. -DKAI_BUILD_TEST_NETWORK=ON
fi

# Build all components
echo "Building network components and tests..."
cmake --build . --target ConfigurableServer ConfigurableClient Test_Network

# Run the calculation test
echo "Running calculation test script..."
cd "$ROOT_DIR"
./Scripts/calc_test.sh

# Run the network unit tests
echo "Running network unit tests..."
cd "$BUILD_DIR"
if [ -f "bin/Test/Test_Network" ]; then
    ./bin/Test/Test_Network
else
    echo "ERROR: Network test executable not found"
    exit 1
fi

echo "All network tests completed successfully!"