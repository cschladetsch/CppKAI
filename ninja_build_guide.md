# KAI Project Ninja Build Guide

## Overview

This guide explains the updated build system for the KAI project, which now uses Ninja by default and supports C++23 features.

## Key Features

1. **Ninja Build System**
   - Faster builds with better parallelization
   - Default generator unless `-DBUILD_GCC=ON` is specified
   - CMake will automatically use Ninja as the generator

2. **C++23 Support**
   - The project now uses C++23 features
   - Modern language features like `std::format` are available
   - Enhanced type safety and cleaner code

3. **Centralized Logging**
   - All logs now write to the `Logs` directory in the project root
   - Defined through CMake using `KAI_LOG_DIR`
   - The directory is automatically created at build time

## How to Build

### Using Ninja (Default)

```bash
# Create a build directory
mkdir -p ninja_build && cd ninja_build

# Configure with CMake (uses Ninja by default)
cmake ..

# Build with Ninja
ninja -j$(nproc)
```

### Using Make (Legacy)

```bash
# Create a build directory
mkdir -p build && cd build

# Configure with CMake and specify Make
cmake -DBUILD_GCC=ON ..

# Build with Make
make -j$(nproc)
```

## Build Output Structure

All build outputs now go to the `/Bin` directory in the project root, including:
- Executables
- Libraries
- Test binaries

This simplifies the build structure and makes it easier to find build outputs.

## Logging System

The project uses a centralized logging system with the following features:

1. **Log Directory**: All logs go to `${CMAKE_CURRENT_SOURCE_DIR}/Logs`
2. **Log Files**:
   - `kai.log`: Main log file with all messages
   - `errors.log`: Contains only error and fatal messages
   - `kai_startup.log`: Logs system initialization
   - `[module].log`: Module-specific logs

## Running Tests

Tests can be run directly from the `/Bin` directory:

```bash
# Run all tests
./Bin/Test/KaiTest

# Run specific test suites
./Bin/Test/TestRho --gtest_filter="MinimalRho.*"
```

## C++23 Features Used

The project leverages several C++23 features:

1. **std::format**: Type-safe formatted output
2. **Improved time handling**: Modern time libraries
3. **constexpr improvements**: Enhanced compile-time evaluation
4. **Concepts**: Improved template constraints with `fconcepts-diagnostics-depth=3` 

## Compiler Flags

The following compiler flags have been added to handle legacy code with modern standards:

```cmake
# Disable specific warnings that are too noisy in legacy code
-Wno-deprecated      # Suppress warnings about deprecated features
-Wno-switch          # Suppress warnings about incomplete switch statements
-Wno-comment         # Suppress warnings about nested comments
-Wno-reorder         # Suppress warnings about field reordering in classes

# Disable pedantic warnings for legacy code
-Wno-unused-parameter         # Suppress warnings about unused parameters
-Wno-missing-field-initializers # Suppress warnings about missing initializers
-Wno-unknown-pragmas          # Suppress warnings about Visual Studio pragmas in Linux
-Wno-unused-value             # Suppress warnings about unused values in macros
-Wno-unused-but-set-variable  # Suppress warnings about variables set but not used

# GCC-specific flags for C++23
-fconcepts-diagnostics-depth=3  # Better concept error messages

# Format compatibility define
-DKAI_FORMAT_COMPATIBLE       # Enable std::format compatibility across compilers
```

## Easy Build Script

For convenience, a build script has been added to simplify the build process:

```bash
# Make the script executable if needed
chmod +x build_with_ninja.sh

# Run the script to build with Ninja
./build_with_ninja.sh
```

This script will create the build directory, configure CMake with Ninja, and build the project using all available CPU cores.

## Known Issues

- Some tests may still fail due to legacy code
- Warnings about deprecated C++ features may appear even with warning suppressions
- The C++23 standard is still evolving, so some features may require workarounds