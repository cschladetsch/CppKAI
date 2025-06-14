# KAI - Distributed Object Model for C++ ![Image](Images/kai_logo.png)

[![Build status](https://ci.appveyor.com/api/projects/status/github/cschladetsch/kai?svg=true)](https://ci.appveyor.com/project/cschladetsch/kai)
[![CodeFactor](https://www.codefactor.io/repository/github/cschladetsch/kai/badge)](https://www.codefactor.io/repository/github/cschladetsch/kai)
[![License](https://img.shields.io/github/license/cschladetsch/flow.svg?label=License&maxAge=86400)](./LICENSE)

_KAI_ is a network distributed **Object Model** for C++ with full runtime reflection, persistence, and incremental garbage collection. No macros are needed to expose fields or methods to the scripting runtime, including external code from other libraries.

Objects and *compute* can be distributed across Nodes in a Domain.

## Demo Views

![Console](Images/BasicConsole.png)

![Window](Images/Window.png)

## Quick Links

### Documentation 

**[📚 Documentation Guide](Doc/Documentation.md)** - Start here for organized navigation of all documentation

- **Building**: [Build Guide](Doc/OUT_OF_SOURCE_BUILD.md) | [Installation](Doc/Install.md)
- **Languages**: [Pi](Doc/PiTutorial.md) | [Rho](Doc/RhoTutorial.md) | [Tau](Doc/TauTutorial.md)
- **Networking**: [Overview](Doc/Networking.md) | [Architecture](Doc/NetworkArchitecture.md)
- **Testing**: [Test Guide](Doc/Test.md) | [Connection Testing](Doc/ConnectionTesting.md)
- [Tau Code Generation](Doc/TauCodeGeneration.md) - New detailed guide for code generation

### Demo and Examples

- Run `./Scripts/run_rho_demo.sh` for a comprehensive demo of Rho language features
- Run `./Scripts/calc_test.sh` for a demonstration of network calculation
- Example scripts in `Test/Language/*/Scripts` directories

## Key Features

- **Zero-Macro Reflection**: Expose C++ types and methods to scripting without macros or source modifications
- **Distributed Computing**: Share both data and computation across networked nodes
- **Multiple Languages**: Use Pi (stack-based), Rho (infix), or Tau (IDL) as needed
- **Type Safety**: Full type checking across network boundaries
- **Incremental GC**: Smooth, constant-time garbage collection with no spikes
- **Cross-Platform**: Works on Windows, Linux, macOS, and Unity3D
- **Network Transparency**: Access remote objects as if they were local
- **Dynamic Load Balancing**: Automatically distribute workload across network nodes

## Core Components

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

Build scripts that follow best practices for out-of-source builds:

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
The interactive console supports Pi, Rho, and shell command integration:

```
$ ./Console
KAI Console v1.0
Pi λ 2 3 +
[0]: 5

Pi λ `echo 10` `echo 20` +
[0]: 30

Pi λ 1 `echo 2` + 3 ==
[0]: true

Pi λ $ ls
file1.txt  file2.cpp  directory/

Pi λ 2 rho
Rho λ x = 10
[0]: 2

Rho λ y = 20
[0]: 2

Rho λ x + y
[0]: 30
```

Features:
- **Shell Integration**: Two modes for shell commands:
  - **Embedded**: Use `` `command` `` to embed shell output in expressions
  - **Standalone**: Use `$ command` to execute shell commands directly
- **Automatic Stack Display**: Stack shown after each command with colored output
- **Language Switching**: Use `2 rho` for Rho, `1 pi` for Pi
- **Mixed Expressions**: Combine shell output with code: `1 `echo 2` + 3 == assert`

See [Console Documentation](Doc/Console.md) for full details.

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
