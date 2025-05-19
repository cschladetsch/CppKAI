# KAI Out-of-Source Build Guide

This document explains how to build the KAI project using proper out-of-source builds.

## What are Out-of-Source Builds?

Out-of-source builds keep build artifacts separate from the source code. This provides several benefits:

- Cleaner source directories (no build files mixed with source files)
- Easy cleanup (just delete the build directory)
- Support for multiple build configurations (Debug, Release, etc.)
- Avoids git tracking build artifacts

## Build Scripts

We provide several scripts to help with out-of-source builds:

### 1. `b` - Standard Build Script

This is the recommended build script for most users. It:

- Creates a clean build directory named `build`
- Uses Clang++ as the default compiler (can be overridden with `--gcc`)
- Uses Ninja as the default build system (can be overridden with `--no-ninja`)
- Configures CMake with correct paths to ensure proper output locations
- Builds the project using all available cores

Usage:
```bash
# Build with Clang++ (default)
./b

# Build with GCC
./b --gcc

# Build without Ninja
./b --no-ninja

# Combined options
./b --gcc --no-ninja
```

The executables will be available in `build/Bin/`.

### 2. `r` - Test Runner Script

This script is designed to run the Rho language tests one by one to help isolate any issues:

- Uses Clang++ as the default compiler (can be overridden with `--gcc`)
- Can disable colored output with `--no-color`
- Runs each test individually and reports success/failure

Usage:
```bash
# Run tests with Clang++ (default)
./r

# Run tests with GCC
./r --gcc

# Run tests without colored output
./r --no-color
```

## Manual Out-of-Source Build

If you prefer to run the commands manually:

```bash
# Create and enter build directory
mkdir -p build
cd build

# Run CMake configuration
cmake ..

# Build the project
cmake --build .
```

## Testing the Build

After building, you can run the test suite:

```bash
./run_tests
```

The `run_tests` script will automatically find test executables in either the root `Bin/Test` directory or in one of the build directories (`build/Bin/Test` or `build_ninja/Bin/Test`).

## Common Issues

### Permission Denied Error

If you see an error like `ninja: error: mkdir(/Bin): Permission denied`, it means CMake is trying to create an absolute path at the root of your filesystem.

Solution: Use the `Scripts/build.sh` script which correctly specifies all output paths with absolute paths relative to the build directory.

### CMake Cache Conflicts

If you switch between different generators (e.g., between Ninja and Unix Makefiles), you might encounter conflicts.

Solution: Always delete the build directory before switching generators, or use separate build directories for different generators.

## Best Practices

1. Always use out-of-source builds
2. Run `git clean -fxd` if build files accidentally get created in source directories
3. Add build directories to your `.gitignore` file
4. Use absolute paths for output directories to avoid permission issues