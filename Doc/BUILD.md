# Building KAI

KAI uses CMake as its build system and follows modern out-of-source build practices. All build artifacts should be kept in a `build/` directory, separate from source code.

## Prerequisites

- CMake 3.28 or higher
- C++23 compatible compiler (GCC 13+, Clang 16+, or MSVC 2022+)
- Boost 1.72 (newer versions may work but haven't been tested)
  - Required components: system, filesystem, program_options, date_time, regex

### Installing Boost

For all platforms:

```bash
./bootstrap.sh  # or .\bootstrap.bat on Windows
./b2 install debug --date-time --build=complete --with-chrono --with-filesystem --with-system --with-program_options
```

## Building the Project

### Standard Out-of-Source Build

Always build from a separate `build` directory to keep your source tree clean:

```bash
# Create build directory
mkdir -p build
cd build

# Generate build files
cmake ..

# Build the project
cmake --build .  # Use this on all platforms
# or
make             # On Unix-like systems
```

### Build Configuration Options

KAI provides several build options that can be configured with CMake:

```bash
# Use GCC instead of Ninja (default)
cmake .. -DBUILD_GCC=ON

# Configure build types
cmake .. -DCMAKE_BUILD_TYPE=Debug   # Default
cmake .. -DCMAKE_BUILD_TYPE=Release

# Control which components to build
cmake .. -DKAI_BUILD_TEST_ALL=ON          # Build all tests (default)
cmake .. -DKAI_BUILD_CORE_TEST=ON         # Build core unit tests (default)
cmake .. -DKAI_BUILD_TEST_LANG=ON         # Build language tests (default)
cmake .. -DKAI_BUILD_TEST_NETWORK=OFF     # Build networking tests (default: OFF)
cmake .. -DKAI_BUILD_RAKNET=OFF           # Build with RakNet (default: OFF)
```

### Platform-Specific Instructions

#### Linux/macOS

```bash
mkdir -p build && cd build
cmake ..
make
```

#### Windows

```bash
mkdir build
cd build
cmake ..
cmake --build .   # For command-line builds
# or
start *.sln       # To open in Visual Studio
```

## Output Directories

All build outputs are organized in the following directories:

- **KAI/Bin** - Executables and test binaries
- **KAI/Lib** - Static and shared libraries

## Running Tests

From the build directory:

```bash
ctest              # Run all tests
./Bin/KaiTest      # Run core tests
./Bin/TestRho      # Run Rho language tests
./Bin/TestPi       # Run Pi language tests
```

## Common Issues

- If you see CMake errors about missing Boost components, make sure you've installed Boost with all required components.
- If compilation fails with C++23 features not being recognized, ensure you're using a recent enough compiler version.
- Always clean your build directory if you encounter strange build issues:
  ```bash
  rm -rf build/*  # On Unix-like systems
  # or
  rmdir /S /Q build  # On Windows
  mkdir build
  ```

## Development Best Practices

1. **Always use out-of-source builds** - Never run CMake directly in the source directory
2. **Commit only source files** - Never commit build artifacts to the repository
3. **Use a clean build directory** - If you encounter build issues, try with a fresh build directory