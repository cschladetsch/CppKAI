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

- **May 2025**: Refactored Tau code generation architecture
  - **Separated proxy and agent generation** - GenerateProxy now only creates proxy classes, GenerateAgent only creates agents
  - **Added GenerateStruct class** - New dedicated class for generating plain struct definitions
  - **Improved separation of concerns** - Clear distinction between client-side proxies, server-side agents, and data structures
  - **All generators now properly inherit** from GenerateProcess base class
  - **Added comprehensive tests** for the refactored generation system
- **May 2025**: Major fixes to Rho language implementation - all tests now passing
  - **Fixed type mismatch issues** in RhoTranslator that were causing failures
  - **Removed direct evaluation** at translation time (approximately 1000 lines of code)
  - **Fixed Store operation** handling for proper variable assignments
  - **All 120 Rho tests** now pass successfully
- **May 2025**: Added comprehensive fixes and improvements to the codebase
- **Modern C++23 features**:
  - **Full C++23 codebase** - Now uses latest C++23 features throughout:
    - Spaceship operator (`<=>`) for comparisons
    - `std::string_view` for efficient string handling
    - `std::span` for safe array views
    - `std::ranges` and view adapters like `std::views::enumerate` and `std::views::repeat`
    - Template constraints with `requires` clauses and concepts
    - Modern type system with `std::optional` for error handling
    - Using `std::allocator` for memory management
  - **Enhanced error handling** - Better failure recovery and cleaner code
  - **Improved compatibility** - Graceful fallbacks for older compilers
- **Core system improvements**:
  - **Modern Event System** - Completely rewritten Event implementation using C++23 features including variadic templates, concepts, and smart pointers
  - **Enhanced type safety** - Strong type checking throughout the system
  - **Memory safety improvements** - Reduced manual memory management
- **Network improvements**:
  - **Configurable server and client** - JSON-configurable network components
  - **Remote calculation demo** - Demonstrates distributed calculation with "1+2=3"
  - **Network test cases** - Comprehensive tests for network functionality
  - **Tau network interfaces** - Interface definitions for connection handling
  - **Connection testing** - Scripts to test and verify network connectivity
- **Build system improvements**:
  - **Default to Clang++** - Now uses Clang++ by default for better C++23 support
  - **Compiler selection flags** - Easily switch between Clang++ and GCC with `--gcc` flag
  - **New Makefile** - Added top-level Makefile for simpler builds
  - **Updated scripts** - Enhanced `b` script with more options and better defaults
  - **Fixed output paths** - Corrected build paths to avoid permission issues
- **Language implementation fixes**:
  - **Fixed binary operations** - Full support for arithmetic, logical, comparison, and bitwise operations
  - **Enhanced control structures** - Properly working if/else, for, while, and do-while loops
  - **Improved function handling** - Support for recursion, nested functions, and proper scoping
- **Added a comprehensive demo** - Run `./Scripts/run_rho_demo.sh` to see all Rho features in action
- **Expanded documentation** - New [Event System](Doc/EventSystem.md), [Rho Language](Doc/RhoLanguage.md), [Rho Fix Documentation](Doc/Rho-Fix-Documentation.md), and [Networking Changes](Doc/NetworkingChanges.md)
- **Colored console output** for better readability with support for green INFO, yellow WARNING, and red ERROR messages
- **Fixed test suite** to pass all tests with proper color formatting
- **Direct test implementations** for Rho language to handle continuation issues
- **Custom test runner** to run tests in a controlled manner
- **Enhanced binary operation handling** and continuation unwrapping for Pi/Rho languages
- **Fixed type preservation** in binary operations (resolving issues with "20 20 +" pattern)
- **Code quality improvements**:
  - **Modernized allocator system** with safer memory management
  - **String handling with std::string_view** for better performance
  - **Fixed move constructor issues** in RhoTranslator
  - **Removed pessimizing moves** in LexerBase
  - **Improved code syntax** with proper parentheses in conditional statements
  - **Fixed macro issues** in Float.h type traits
  - **Eliminated unused variables** across the codebase

## System Components

### Core Components

- **Registry**: Type-safe object factory for creating, managing, and reflecting C++ objects
- **Domain**: A collection of registries across network nodes
- **Executor**: Stack-based virtual machine for executing code
- **Memory Management**: Incremental tri-color garbage collector

### Languages

- **Pi**: Stack-based RPN language inspired by Forth
- **Rho**: Python-like infix language that compiles to Pi (fully functional with all tests passing)
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
  - Clang 16+ (default, recommended)
  - GCC 13+
  - MSVC 2022+
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

We provide convenient build scripts that follow best practices for out-of-source builds:

```bash
# Quick build (using Clang++ by default)
./b

# Build with GCC
./b --gcc

# Build without Ninja
./b --no-ninja

# Using Makefile (Clang++ by default)
make

# Using Makefile with GCC
make gcc

# Clean build directory
make clean
```

#### Manual Build (Out-of-Source)

For a manual build, always use the `build/` directory:

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake (Clang++ by default)
cmake ..

# Configure with GCC
cmake .. -DBUILD_GCC=ON

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

### Network Applications
The system includes several network applications:

```bash
# Run a configurable calculation server
./build/Bin/ConfigurableServer config/server_config.json

# Run a client that sends a calculation request
./build/Bin/ConfigurableClient config/client_config.json
```

Try the calculation test demo:
```bash
./Scripts/calc_test.sh
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
- [Event System](Doc/EventSystem.md) - Modern C++23 multi-cast event implementation
- [Rho Fix Documentation](Doc/Rho-Fix-Documentation.md) - Details of recent Rho language fixes
- [Networking](Doc/Networking.md) - Distributed computing capabilities
- [Network Calculation Test](Doc/NetworkCalculationTest.md) - Details of the calculation demo
- [Connection Testing](Doc/ConnectionTesting.md) - Network connection testing
- [Networking Changes](Doc/NetworkingChanges.md) - Recent networking improvements
- [Test Documentation](Doc/Test.md) - How to run and write tests
- [Colored Output](Doc/ColorOutput.md) - Console color formatting

### Demo and Examples
- Run `./Scripts/run_rho_demo.sh` for a comprehensive demo of Rho language features
- Run `./Scripts/calc_test.sh` for a demonstration of network calculation
- Example scripts in `Test/Language/*/Scripts` directories

## Project Structure

- **Bin**: Executable output files
- **build**: Build directory (for all build artifacts)
- **CMake**: Auxiliary CMake modules
- **Doc**: Documentation and tutorials
- **Ext**: External dependencies (git submodules)
- **Include**: Global include path
- **Lib**: Library files
- **Logs**: System logs (ignored by git)
- **Source**: Project source code
- **Test**: Unit tests

### Tau Language Module Architecture

The Tau language module has a well-organized structure for code generation:

#### Code Generation Classes (`Include/KAI/Language/Tau/Generate/`)

- **GenerateProcess**: Base class for all code generators, providing common functionality
- **GenerateProxy**: Generates client-side proxy classes for remote procedure calls
- **GenerateAgent**: Generates server-side agent classes for handling incoming requests  
- **GenerateStruct**: Generates plain data structure definitions

All generators properly inherit from `GenerateProcess` and have clear separation of concerns:

1. **Proxy Generation** - Creates proxy classes that forward method calls over the network
2. **Agent Generation** - Creates agent classes that receive and process network requests
3. **Struct Generation** - Creates plain C++ struct definitions from Tau IDL

#### Source Organization

**Headers** (`Include/KAI/Language/Tau/`):
- Core components: `Tau.h`, `TauLexer.h`, `TauParser.h`, `TauToken.h`, `TauAstNode.h`
- Configuration: `Config.h`
- Code generation: `Generate/` subdirectory

**Implementation** (`Source/Library/Language/Tau/Source/`):
- `Tau/` - Core language implementation
- `Generate/` - Code generation implementations

**Tests** (`Test/Language/TestTau/`):
- Comprehensive test suite including `TauGenerateStructTests.cpp` and `TauSeparateGenerationTests.cpp`

This architecture enables clean separation between:
- Interface definitions (Tau IDL)
- Client-side proxy code (for making remote calls)
- Server-side agent code (for handling remote calls)
- Plain data structures (for data transfer objects)

## Platforms

- Windows 10/11 (VS 2017-22)
- Linux (Ubuntu, Debian)
- macOS (Sierra and newer)
- Unity3D (2017+)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.