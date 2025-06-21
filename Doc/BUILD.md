# Building KAI

KAI uses CMake as its build system and follows modern out-of-source build practices. All build artifacts are kept in a `build/` directory, separate from source code.

## Prerequisites

- CMake 3.28 or higher
- C++23 compatible compiler (GCC 13+, Clang 16+, or MSVC 2022+)
- Boost 1.72 or higher
  - Required components: system, filesystem, program_options, date_time, regex

### Installing Boost

For all platforms:

```bash
./bootstrap.sh  # or .\bootstrap.bat on Windows
./b2 install debug --date-time --build=complete --with-chrono --with-filesystem --with-system --with-program_options
```

## Building the Project

### Quick Start with Helper Scripts

KAI provides convenient scripts for building:

```bash
# Build with Clang++ (default)
./b

# Build with GCC
./b --gcc

# Build without using Ninja
./b --no-ninja
```

### Using the Makefile

A Makefile is provided for simpler builds:

```bash
# Build with Clang++ (default)
make
# or
make clang

# Build with GCC
make gcc

# Clean the build directory
make clean
```

### Standard Out-of-Source Build

Always build from a separate `build` directory to keep your source tree clean:

```bash
# Create build directory
mkdir -p build
cd build

# Generate build files with Clang++ (default)
cmake ..

# Generate build files with GCC
cmake .. -DCMAKE_CXX_COMPILER=g++ -DBUILD_GCC=ON

# Build the project
cmake --build .  # Use this on all platforms
# or
make             # On Unix-like systems
```

### Build Configuration Options

KAI provides several build options that can be configured with CMake:

```bash
# Use GCC instead of Clang++ (default)
cmake .. -DBUILD_GCC=ON

# Explicitly set compiler
cmake .. -DCMAKE_CXX_COMPILER=clang++  # Default
cmake .. -DCMAKE_CXX_COMPILER=g++      # Use GCC

# Configure build types
cmake .. -DCMAKE_BUILD_TYPE=Debug   # Default
cmake .. -DCMAKE_BUILD_TYPE=Release

# Security options
cmake .. -DENABLE_SHELL_SYNTAX=ON         # Enable shell command integration (default: OFF)

# Control which components to build
cmake .. -DKAI_BUILD_TEST_ALL=ON          # Build all tests (default)
cmake .. -DKAI_BUILD_CORE_TEST=ON         # Build core unit tests (default)
cmake .. -DKAI_BUILD_TEST_LANG=ON         # Build language tests (default)
cmake .. -DKAI_BUILD_TEST_NETWORK=OFF     # Build networking tests (default: OFF)
cmake .. -DKAI_BUILD_RAKNET=OFF           # Build with RakNet (default: OFF)
```

#### Security Configuration

**Shell Command Integration**: By default, shell commands are disabled for security reasons. To enable shell integration in the Console:

```bash
# Enable shell syntax (allows $ commands and backtick expansion)
cmake .. -DENABLE_SHELL_SYNTAX=ON

# Or use the helper script
./b --enable-shell
```

**Important**: Only enable shell syntax in trusted environments. When enabled:
- Console supports `$ command` for direct shell execution
- Console supports `` `command` `` for embedding shell output in expressions
- Shell commands execute with the same privileges as the Console process

### Platform-Specific Instructions

#### Linux/macOS

```bash
# Using helper script (recommended)
./b

# Manual build
mkdir -p build && cd build
cmake ..
cmake --build .
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

### Console Installation

The Console application automatically copies itself to `~/bin/Console` if the directory exists, making it available system-wide. This happens during the build process.

```bash
# After building, the Console is available at:
./Bin/Console          # Local build directory
~/bin/Console          # Automatically installed (if ~/bin exists)

# You can also manually install:
cp ./Bin/Console ~/bin/                    # User installation
sudo cp ./Bin/Console /usr/local/bin/     # System-wide installation
```

## Running Applications

### Console Application

The Console provides an interactive REPL environment:

```bash
# Basic usage
./Bin/Console                    # Interactive Pi mode
./Bin/Console --help             # Show all options
./Bin/Console --version          # Show version info
./Bin/Console -l rho             # Start in Rho mode
./Bin/Console script.pi          # Execute a script
./Bin/Console -t 2 script.rho    # Execute with trace level 2

# If installed to ~/bin:
Console --help                   # Available system-wide
```

### Running Tests

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