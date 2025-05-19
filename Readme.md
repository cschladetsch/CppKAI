# KAI - Distributed Object Model for C++

[![Build status](https://ci.appveyor.com/api/projects/status/github/cschladetsch/kai?svg=true)](https://ci.appveyor.com/project/cschladetsch/kai)
[![CodeFactor](https://www.codefactor.io/repository/github/cschladetsch/kai/badge)](https://www.codefactor.io/repository/github/cschladetsch/kai)
[![License](https://img.shields.io/github/license/cschladetsch/flow.svg?label=License&maxAge=86400)](./LICENSE)

_KAI_ is a network distributed **Object Model** for C++ with full runtime reflection, persistence, and incremental garbage collection. No macros are needed to expose fields or methods to the scripting runtime, including external code from other libraries.

## Key Features

- **Zero-Macro Reflection**: Expose C++ types and methods to scripting without macros or source modifications
- **Distributed Computing**: Share both data and computation across networked nodes
- **Multiple Languages**: Use Pi (stack-based), Rho (infix), or Tau (IDL) as needed
- **Type Safety**: Full type checking across network boundaries
- **Incremental GC**: Smooth, constant-time garbage collection with no spikes
- **Cross-Platform**: Works on Windows, Linux, macOS, and Unity3D
- **Network Transparency**: Access remote objects as if they were local
- **Dynamic Load Balancing**: Automatically distribute workload across network nodes

## Recent Updates

- **May 2025**: Added comprehensive fixes for the Rho language implementation
- **Fixed binary operations** - Full support for arithmetic, logical, comparison, and bitwise operations
- **Enhanced control structures** - Properly working if/else, for, while, and do-while loops
- **Improved function handling** - Support for recursion, nested functions, and proper scoping
- **Added a comprehensive demo** - Run `./Scripts/run_rho_demo.sh` to see all Rho features in action
- **Expanded documentation** - New [Rho Language](Doc/RhoLanguage.md) and [Rho Fix Documentation](Doc/Rho-Fix-Documentation.md)
- **Colored console output** for better readability with support for green INFO, yellow WARNING, and red ERROR messages
- **Fixed test suite** to pass all tests with proper color formatting
- **Direct test implementations** for Rho language to handle continuation issues
- **Custom test runner** to run tests in a controlled manner
- **Enhanced binary operation handling** and continuation unwrapping for Pi/Rho languages
- **Fixed type preservation** in binary operations (resolving issues with "20 20 +" pattern)
- **Improved error handling** in PerformBinaryOp method for better robustness
- **Improved build system** with proper out-of-source builds and updated build scripts

## System Components

### Core Components

- **Registry**: Type-safe object factory for creating, managing, and reflecting C++ objects
- **Domain**: A collection of registries across network nodes
- **Executor**: Stack-based virtual machine for executing code
- **Memory Management**: Incremental tri-color garbage collector

### Languages

- **Pi**: Stack-based RPN language inspired by Forth
- **Rho**: Python-like infix language that compiles to Pi
- **Tau**: Interface Definition Language (IDL) for network components

## Example Code

### Pi (Stack-based)
```pi
{ dup * } 'square #  // Define a function that squares its input
5 square @           // Retrieve the function
&                    // Execute the function
```

### Rho (Infix)
```rho
fun square(x) {
    return x * x
}
result = square(5)  // result is 25
```

### Distributed Computing
```rho
// Create a network node
node = createNetworkNode()
node.listen(14589)
node.connect("192.168.1.10", 14589)

// Create data and define a function
data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
fun square(x) { return x * x }

// Process the data using distributed execution
result = acrossAllNodes(node, data, square)
print(result)  // [1, 4, 9, 16, 25, 36, 49, 64, 81, 100]
```

## Getting Started

### Prerequisites

- Modern C++ compiler (C++23 compatible)
- CMake (3.28+)
- Boost libraries (filesystem, system, program_options, date-time, regex)
- Ninja (optional but recommended for faster builds)

### Building

Clone the repository with submodules:
```bash
git clone https://github.com/cschladetsch/KAI.git
cd KAI
git submodule init
git submodule update
```

#### Using the Build Scripts

We now provide convenient build scripts that follow best practices for out-of-source builds:

```bash
# Clean any build artifacts from source tree and set up build directory
./Scripts/clean_build.sh  # Linux/macOS
.\clean_build.bat # Windows

# Build the project using the build script
./Scripts/build.sh        # Linux/macOS
.\build.bat       # Windows
```

#### Manual Build (Out-of-Source)

For a manual build, always use the `build/` directory:

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .   # Cross-platform
# or
make              # Unix systems
# or
ninja             # If using Ninja generator
```

For more detailed build instructions, see [BUILD.md](Doc/BUILD.md).

## Applications

### Console
The basic console supports both Pi and Rho as a REPL shell:

```
$ ./Console
KAI Console v1.0
Language: Pi> 2 3 +
5
Language: Pi> :rho
Language: Rho> x = 10
Language: Rho> y = 20
Language: Rho> x + y
30
```

### GUI Application
The GUI application provides:
- Syntax highlighted code editing
- Visual stack inspection
- Network monitoring
- Object browser
- Performance metrics

## Documentation

### Main Documentation Entry Points
- [Documentation Guide](Doc/Documentation.md) - Central entry point to all documentation
- [Building Guide](Doc/OUT_OF_SOURCE_BUILD.md) - How to build the project correctly
- [Installation Guide](Doc/Install.md) - Installation instructions

### Language Documentation
- [Language System Overview](Doc/Languages.md) - Brief overview of the language system
- [Language Guide](Doc/LanguageGuide.md) - Comprehensive language system guide
- [Common Language System](Doc/CommonLanguageSystem.md) - Shared architecture

#### Individual Languages
- [Pi Tutorial](Doc/PiTutorial.md) - Stack-based foundation language
- [Rho Language](Doc/RhoLanguage.md) - Application-level language with infix notation
- [Rho Tutorial](Doc/RhoTutorial.md) - Detailed Rho language tutorial
- [Tau Tutorial](Doc/TauTutorial.md) - Interface definition language

### Technical Documentation
- [Rho Fix Documentation](Doc/Rho-Fix-Documentation.md) - Details of recent Rho language fixes
- [Networking](Doc/Networking.md) - Distributed computing capabilities
- [Test Documentation](Doc/Test.md) - How to run and write tests
- [Colored Output](Doc/ColorOutput.md) - Console color formatting

### Demo and Examples
- Run `./Scripts/run_rho_demo.sh` for a comprehensive demo of Rho language features
- Example scripts in `Test/Language/*/Scripts` directories

## Project Structure

- **Bin**: Executable output files
- **build**: Build directory (for all build artifacts)
- **CMake**: Auxiliary CMake modules
- **Doc**: Documentation and tutorials
- **Ext**: External dependencies (git submodules)
- **Include**: Global include path
- **Lib**: Library files
- **Logs**: System logs
- **Source**: Project source code
- **Test**: Unit tests

## Platforms

- Windows 10/11 (VS 2017-22)
- Linux (Ubuntu, Debian)
- macOS (Sierra and newer)
- Unity3D (2017+)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.