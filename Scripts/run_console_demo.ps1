#!/bin/bash

# Simple script to build and test MinimalServer and MinimalClient

# Navigate to project root directory
cd "$(dirname "$0")/.."
ROOT_DIR=$(pwd)
BUILD_DIR="$ROOT_DIR/build"

# Make sure the build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
else
    cd "$BUILD_DIR"
fi

# Clean build of MinimalServer and MinimalClient
echo "Building MinimalServer and MinimalClient..."
cmake --build . --target MinimalServer MinimalClient -- -j$(nproc)

# Check for successful build
if [ ! -f "$BUILD_DIR/Bin/MinimalServer" ] || [ ! -f "$BUILD_DIR/Bin/MinimalClient" ]; then
    echo "ERROR: Failed to build MinimalServer or MinimalClient"
    echo "Check CMake configuration and build errors"
    exit 1
fi

echo "Build successful!"
echo "MinimalServer and MinimalClient are ready to test"
echo

# Print instructions for testing
echo "=================== TESTING INSTRUCTIONS ==================="
echo "1. In one terminal run:"
echo "   $BUILD_DIR/Bin/MinimalServer 14591"
echo
echo "2. In another terminal run:"
echo "   $BUILD_DIR/Bin/MinimalClient 127.0.0.1 14591"
echo
echo "3. In the client terminal, type '1+2' and press enter"
echo
echo "Note: The server will echo back the message with 'Server echoed:' prefix"
echo "      This demonstrates successful connection and message exchange"
echo "==========================================================="