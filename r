#!/bin/bash

# Define the Ninja-specific build directory
BUILD_DIR="build_ninja"

# Clean and recreate the build directory
echo "Creating clean build directory ($BUILD_DIR)..."
rm -rf "$BUILD_DIR" && mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

# Run CMake with Ninja generator
echo "Configuring with CMake and Ninja generator..."
cmake -G Ninja ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

bin_path="/"

# Build with the appropriate tool based on OS
if [[ "$OSTYPE" == "msys"* ]]; then
    echo "Building with MSBuild on Windows..."
    msbuild.exe *.sln
    bin_path="/Debug"
else
    echo "Building with Ninja..."
    ninja
    if [ $? -ne 0 ]; then
        echo "Ninja build failed!"
        cd ..
        exit 1
    fi
fi

cd ..

echo "Running tests..."
./run_tests 
test_result=$?

RED='\033[0;35m'
WHITE='\033[0;39m'

echo OSTYPE=$OSTYPE

echo 
echo -e $RED ---- Running Kai Console ----
echo -e $WHITE

if [ -x "./build_ninja/Bin/Console" ]; then
    # Run from the build directory if it exists there
    ./build_ninja/Bin/Console
elif [ -x "./Bin${bin_path}/Console" ]; then
    # Fallback to the traditional location
    ./Bin${bin_path}/Console
else
    echo "Error: Could not find Console executable!"
    exit 1
fi

if [ $test_result -ne 0 ]; then
    echo "Warning: Tests failed with exit code $test_result"
fi

