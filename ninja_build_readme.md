# KAI Project Ninja Build System

## Overview

This document describes the ninja build system configuration for the KAI project, which has been updated to use C++23 features and improve logging.

## Build Configuration

### C++23 Standard
The project now uses the C++23 standard by default, enabling modern C++ features:
- `std::format` for type-safe string formatting
- Enhanced template metaprogramming with improved concepts support
- More robust and safer coding patterns
- Better compiler error diagnostics for templates

### Ninja Build System
We use the Ninja build system for faster compilation. You can use the provided build script:
```bash
# Make the script executable if needed
chmod +x build_with_ninja.sh

# Run the script to build with Ninja
./build_with_ninja.sh
```

Or manually run the build commands:
```bash
mkdir -p ninja_build
cd ninja_build
cmake ..  # Ninja is now the default generator
ninja -j$(nproc)
```

### Log Directory Structure
All logs are written to the `Logs` directory in the project root:
- `kai.log` - Main application log
- `errors.log` - Error-level messages 
- Module-specific logs (e.g., `rho.log`)

### Output Directory
All build outputs (executables, libraries) are placed in the `Bin` directory in the project root for easier access.

## Building Without Ninja

If you want to use the default build system instead of Ninja:
```bash
mkdir -p build
cd build
cmake -DBUILD_GCC=ON ..
make
```

## Compiler Flags

The build system includes special compiler flags to handle legacy code:
```
# Basic warning suppressions
-Wno-deprecated -Wno-switch -Wno-comment -Wno-reorder -Wno-unused-parameter -Wno-missing-field-initializers

# Additional suppressions for smooth C++23 builds
-Wno-unknown-pragmas -Wno-unused-value -Wno-unused-but-set-variable
```

GCC-specific flags are also included for better C++23 support:
```
-fconcepts-diagnostics-depth=3  # Better template error messages
```

We've also added a compatibility define for format support:
```
-DKAI_FORMAT_COMPATIBLE  # Enable std::format compatibility
```

## Best Practices

When developing with this configuration:

1. **Log Usage**: 
   - Use the Logger class for all logging
   - Log errors with appropriate severity
   - Check the `Logs` directory for runtime information

2. **Modern C++ Usage**:
   - Leverage C++23 features like `std::format` and standardized utilities
   - Replace string concatenation with `std::format`
   - Use structured bindings and other modern features
   - Take advantage of improved concepts and template metaprogramming

3. **Building**:
   - Prefer the Ninja build system for faster compile times
   - Run builds from the `ninja_build` directory
   - Check compiler output carefully despite warning suppressions

For more detailed information, see the accompanying `ninja_build_guide.md` file.