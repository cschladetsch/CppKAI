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

- **May 2025**: Added colored output support for test programs
- **Colored console output** for better readability with support for green INFO, yellow WARNING, and red ERROR messages
- **Fixed test suite** to pass all tests with proper color formatting
- **Direct test implementations** for Rho language to handle continuation issues
- **Custom test runner** to run tests in a controlled manner
- **Enhanced binary operation handling** and continuation unwrapping for Pi/Rho languages
- **Fixed type preservation** in binary operations (resolving issues with "20 20 +" pattern)
- **Improved error handling** in PerformBinaryOp method for better robustness

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

- Modern C++ compiler (C++14 or newer)
- CMake (3.15+)
- Boost libraries (filesystem, chrono, program_options, date-time, regex)
- Ninja (optional but recommended for faster builds)

### Building

Clone the repository with submodules:
```bash
git clone https://github.com/cschladetsch/KAI.git
cd KAI
git submodule init
git submodule update
```

Quick build with scripts:
```bash
# Full clean build with Ninja and runs tests and console
./r

# Incremental build with Ninja
./n

# Build and run a specific test
./nt TestPi

# Run only fixed tests (recommended)
./run_all_tests_fixed
```

Manual build:
```bash
# With Make
mkdir -p build && cd build && cmake .. && make

# With Ninja (recommended for faster builds)
mkdir -p build_ninja && cd build_ninja && cmake -G Ninja .. && ninja
```

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

- [Installation Guide](Install.md)
- [Language System](Languages.md)
- [Networking](Networking.md)
- [Common Language System](Doc/CommonLanguageSystem.md)
- [Language Guide](Doc/LanguageGuide.md)
- [Pi Tutorial](Doc/PiTutorial.md)
- [Rho Tutorial](Doc/RhoTutorial.md)
- [Tau Tutorial](Doc/TauTutorial.md)
- [Test Documentation](Doc/Test.md)
- [Colored Output](Doc/ColorOutput.md)
- [Test Summary](Test/Language/TestRho/TestSummary.md)
- [Test Fixes Summary](Test-Fixes-Summary.md)

## Project Structure

- **Bin**: Executable output files
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