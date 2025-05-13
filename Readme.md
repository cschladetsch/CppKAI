# Kai ![Logo](Doc/swords.jpg)
[![Build status](https://ci.appveyor.com/api/projects/status/github/cschladetsch/kai?svg=true)](https://ci.appveyor.com/project/cschladetsch/kai)
[![CodeFactor](https://www.codefactor.io/repository/github/cschladetsch/kai/badge)](https://www.codefactor.io/repository/github/cschladetsch/kai)
[![License](https://img.shields.io/github/license/cschladetsch/flow.svg?label=License&maxAge=86400)](./LICENSE.txt)
[![Release](https://img.shields.io/github/release/cschladetsch/flow.svg?label=Release&maxAge=60)](https://github.com/cschladetsch/kai/releases/latest)

_Kai_ is a network distributed **Object Model** for C++ with full runtime reflection, persistence, and incremental garbage collection. No Macros are needed to expose fields or methods to the scripting runtime, including external code from other libraries.

Kai provides three scripting languages - Pi, Rho and Tau. It was also the inspiration for the C# [Pyro](https://github.com/cschladetsch/Pyro) implementation, which was much easier due to .Net's reflection.

## Purpose :grey_question:
The purpose of KAI is to allow communication between networked nodes in as simple and direct way as possible. Current network implementation uses Raknet, but it is abstracted.

This is not limited to just data transfer - rather, *computation itself* can be shared between nodes, using a distributed **Executor** with a shared heartbeat.

If a node is over-loaded, it can push it's load out to other peers in the *Domain* that it shares.

This is all done via distributed object model, and a few languages.

## Key Features 🔑

- **Zero-Macro Reflection**: Expose C++ types and methods to scripting without macros or source modifications
- **Distributed Computing**: Share both data and computation across networked nodes
- **Multiple Languages**: Use Pi (stack-based), Rho (infix), or Tau (IDL) as needed
- **Type Safety**: Full type checking across network boundaries
- **Incremental GC**: Smooth, constant-time garbage collection with no spikes
- **Cross-Platform**: Works on Windows, Linux, macOS, and Unity3D
- **Network Transparent**: Access remote objects as if they were local
- **Dynamic Load Balancing**: Automatically distribute workload across network nodes

## Heart of the System :heart:
At the heart of KAI are three things: A Registry, A Domain, and some Languages.

* **Registry**: A statically-typesafe over-the-wire object factory.
* **Domain**: A Local Registry that is part of the larger Network. Objects in a Domain are in a Registry that is qualified by the Guid of that domain.
* **Languages**:
  * **Pi**: A stack-based RPN language that forms the foundation of execution.
  * **Rho**: A Python-like infix language with native continuations and Pi code embedding.
  * **Tau**: The Interface Definition Language (IDL) used to define network-distributed components.

For a comprehensive overview of KAI's language system, see the [Language Guide](Doc/LanguageGuide.md) and the [Common Language System Architecture](Doc/CommonLanguageSystem.md).

## Languages

### Pi
[Pi](Source/Library/Language/Pi) (see [Tests](Test/Language/TestPi) and [Tests Scripts](Test/Language/TestPi/Scripts)) is heavily influenced by [Forth](https://en.wikipedia.org/wiki/Forth_(programming_language)). It has two directly interactable stacks: one for data, and one for context. The data stack is used for operations (as is the context stack), but the context stack tells the machine `where to go next`. This is used to create the idea of a co-routine, which is then pushed up to Rho.

```pi
// Pi example: A function that squares its input
{ dup * } 'square #  // Define a function that squares its input
5 square @           // Retrieve the function
&                    // Execute the function
```

For a comprehensive guide on using Pi, see the [Pi Language Tutorial](Doc/PiTutorial.md).

### Rho
[Rho](Source/Library/Language/Rho) (see [Tests](Test/Language/TestRho) and [Tests Scripts](Test/Language/TestRho/Scripts)), is an infix language much like Python, LUA or Ruby, but with native continuations and the ability to inject Pi code as any factor in an expression.

```rho
// Rho example: Same functionality as the Pi example above
fun square(x) {
    return x * x
}
result = square(5)  // result is 25
```

The general idea has always been to move algorithms around the network, as well as data. In this manner, *real* load-balancing can be conducted.

For a comprehensive guide on using Rho, see the [Rho Language Tutorial](Doc/RhoTutorial.md).

### Tau
[Tau](Source/Library/Language/Tau) is the Interface Definition Language (IDL) used in KAI to define how components communicate across the network. It enables seamless cross-network communication while maintaining type safety and versioning.

```tau
// Tau example: Defining a network service interface
namespace Calculation {
    interface ICalculator {
        int Add(int a, int b);
        int Subtract(int a, int b);
        float Divide(float a, float b) throws DivideByZeroException;
    }
    
    service CalculatorService {
        implements ICalculator;
        version = "1.0";
        discoverable = true;
    }
}
```

For a comprehensive guide on using Tau, see the [Tau Language Tutorial](Doc/TauTutorial.md).

## Platforms
Currently supported platforms are:

1. **Windows 10/11** (VS 2017-22)
2. **Linux** (Ubuntu, Debian)
3. **macOS** (Sierra and newer)
4. **Unity3d** (2017+)

You can create and connect Kai nodes on different machines, swap and monitor workloads, and remote manage all nodes in the system.

## Getting Started 🚀

### Prerequisites

* A modern C++ compiler (C++14 or newer)
* [Cmake](https://cmake.org/install/) (3.15+)

```bash
$ sudo apt-get install cmake
```

* [Boost](https://www.boost.org/) libraries: `filesystem`, `chrono`, `programoptions`, `date-time` and `regex`.

```bash
$ sudo apt-get install libboost-filesystem-dev libboost-chrono-dev libboost-regex-dev libboost-program-options-dev libboost-date-time-dev
```

* [Ninja](https://ninja-build.org/) (optional but recommended for faster builds)

```bash
$ sudo apt-get install ninja-build
```

### Quick Build with Scripts

KAI provides several convenient build scripts:

* `./r` - Full clean build with Ninja and runs tests and console
* `./n` - Incremental build with Ninja (preserves the build directory)
* `./nt <TestName>` - Builds and runs a specific test (e.g., `./nt TestPi`)

### Manual Build

If you prefer to manually control the build process:

```bash
# With Make
mkdir -p build && cd build && cmake .. && make

# With Ninja (recommended for faster builds)
mkdir -p build_ninja && cd build_ninja && cmake -G Ninja .. && ninja
```

You can also specify a particular compiler:

```bash
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G Ninja && ninja
```

## Architecture Overview 🏗️

### Registry
The Registry is a type-safe object factory that:
- Manages type information for the system
- Creates and destroys objects
- Enables runtime reflection
- Provides serialization and deserialization
- Maintains object relationships

```cpp
// C++ example of using the Registry
Registry reg;
reg.AddClass<Vector3>("Vector3");
Object vec = reg.New<Vector3>(1.0f, 2.0f, 3.0f);
```

### Executor
A general-purpose stack-based virtual machine with a clean architecture. The Executor follows a separation of concerns design:

1. **Console** handles user interaction and passes input to the Translator
2. **Translator** converts language-specific syntax to Continuations
3. **Executor** executes Continuations in a language-agnostic manner

All languages (Pi, Rho, Tau) are ultimately translated to Pi operations, which the Executor handles natively. This design ensures that the Executor only needs to understand Pi's semantics, simplifying the codebase and improving maintainability.

The Executor uses two stacks (data and context) that provide capabilities not available in most languages. You can move _context_ to the data-stack, do work, then push back onto the _context_ stack. This enables powerful control flow patterns like continuations and tail recursion.

Stack operations like Dup, Swap, Drop, and Over manipulate the data stack, while operations like continuation execution (&) work with the context stack. All operations maintain proper type information and include robust error handling.

For debugging purposes, the Executor includes extensive tracing capabilities that can log each operation's effect on the stacks.

### Network System
The network system enables distributed computing by:
- Distributing objects across nodes
- Synchronizing state changes
- Transferring computation between nodes
- Managing load balancing
- Handling node discovery and connection

### Memory Management
KAI uses a tri-color incremental garbage collector that:
- Runs in small, constant-time increments
- Avoids collection spikes
- Works across network boundaries
- Preserves object relationships
- Handles cycles correctly

## Applications

### Console

The basic [Console](Source/App/Console) supports both Pi and Rho as a Repl shell. It works with color on Windows, Linux, and macOS. The following is just a basic look at the shell.

![Console Screenshot](Images/BasicConsole.png)

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

There is also a [GUI application](/Source/App/Window) based on Dear ImGui, providing:
- Syntax highlighted code editing
- Visual stack inspection
- Network monitoring
- Object browser
- Performance metrics

![GUI Screenshot](Images/Gui.jpg)

## Advanced Features

### Exposing C++ Classes

KAI allows exposing any C++ class to the runtime without modifying its source:

```cpp
// Define a regular C++ class (no base classes or macros needed)
class Vector3 {
public:
    float x, y, z;
    
    Vector3(float _x = 0, float _y = 0, float _z = 0) 
        : x(_x), y(_y), z(_z) {}
        
    float Length() const { 
        return std::sqrt(x*x + y*y + z*z); 
    }
    
    Vector3 Normalize() const {
        float len = Length();
        if (len == 0) return *this;
        return Vector3(x/len, y/len, z/len);
    }
};

// Register with KAI (typically done once at startup)
void RegisterVector3(Registry& registry) {
    ClassBuilder<Vector3> builder(registry, "Vector3");
    
    // Expose properties
    builder.Property("x", &Vector3::x);
    builder.Property("y", &Vector3::y);
    builder.Property("z", &Vector3::z);
    
    // Expose methods
    builder.Method("Length", &Vector3::Length);
    builder.Method("Normalize", &Vector3::Normalize);
    
    // Register the constructor
    builder.Constructor<float, float, float>();
}
```

Now Vector3 can be used from any KAI language:

```rho
// Create a Vector3 in Rho
v = Vector3(1, 2, 3)

// Call methods
length = v.Length()
normalized = v.Normalize()

// Access properties
v.x = 5
```

### Distributed Iteration with AcrossAllNodes

One of the most powerful features of KAI is its ability to distribute both computation and data across network nodes. The `AcrossAllNodes` operation exemplifies this capability by enabling distributed parallel iteration over collections.

#### How It Works

`AcrossAllNodes` enables the distribution of iteration tasks across multiple network nodes. It takes three arguments:

1. A network node (or null for local execution)
2. A collection to iterate over
3. A function to apply to each element

The operation then distributes the workload across connected network peers, collects the results, and returns them as a new collection.

#### Example in Rho

```rho
// Create a network node
node = createNetworkNode()
node.listen(14589)

// Connect to other nodes in the network
node.connect("192.168.1.10", 14589)
node.connect("192.168.1.11", 14589)

// Create a large array of data to process
largeArray = array(1000) 
for i = 0; i < 1000; i = i + 1
    largeArray[i] = i
end

// Define a compute-intensive function
fun computeIntensive(x) {
    result = 0
    // Simulate complex computation
    for j = 0; j < 10000; j = j + 1
        result = result + (x * x) / (j + 1)
    end
    return result
}

// Run the computation in parallel across the network
start = currentTimeMillis()
results = acrossAllNodes(node, largeArray, computeIntensive)
end = currentTimeMillis()

print("Computation completed in " + (end - start) + "ms")
print("First few results: " + results[0:5])
```

#### Supported Collection Types

`AcrossAllNodes` supports the following collection types:

- **Array**: Distributes array elements across network nodes
- **List**: Distributes list elements across network nodes
- **Map**: Distributes map entries as key-value pairs across network nodes
- **String**: Treats the string as a collection of characters to process individually

#### Advanced Features

When a network node is provided, the `AcrossAllNodes` operation:

1. Analyzes the size and complexity of the workload
2. Distributes elements to connected peers based on their available capacity
3. Handles failures and retries if a node becomes unresponsive
4. Aggregates results back into a single collection
5. Maintains type consistency between input and output collections

This enables true distributed parallel processing with minimal developer effort.

## Debugging and Monitoring

### Logging System
KAI includes a comprehensive logging system that captures debug traces, network events, and application messages:

* **Centralized Storage**: All logs are written to the `/Logs` directory for easy access and management.
* **Log Categorization**: Logs are separated by type and component for better organization:
  * Core system logs (debug, info, warning, error, fatal)
  * Network-specific logs (connections, messages, discovery events, status updates)
* **Timestamp Integration**: All log entries include timestamps for precise event tracking.
* **Console Mirroring**: Important log messages are also displayed in the console with color coding.

The logging system is used throughout KAI to track system events, diagnose issues, and monitor network activity. This is especially useful when running distributed computations across multiple nodes.

### Debugging Commands

KAI provides built-in debugging capabilities:

```rho
// Enable trace logging
System.SetTraceLevel(5)  // 0-5, where 5 is most verbose

// Print stack contents
System.PrintStack()

// Show object details
System.Inspect(myObject)

// Time an operation
start = System.CurrentTimeMillis()
// ... operations to time ...
elapsed = System.CurrentTimeMillis() - start
```

## Project Structure

### Folder Structure
* **Bin**: Where to write executable output files.
* **CMake**: Auxiliary CMake modules, primarily for finding other projects and libraries like Boost and Google Test.
* **Doc**: Documentation and tutorials.
* **Ext**: External dependencies, primarily as _git submodules_.
* **Include**: Root of the global include path. Add this to your compiler's include path.
* **Lib**: Location for built static and dynamic library files. Also add this this to your compilers library path.
* **Logs**: Directory for all system logs including debug traces, network events, and application logs.
* **Source**: The root for the source code of the project.
* **Test**: Unit tests. Output goes to *Bin/Tests*.

### Key Files and Directories
* The Test scripts for [Pi](Test/Language/TestPi/Scripts) shows the basics of pi.
* Similarly, the test scripts for [Rho](Test/Language/TestRho/Scripts) show that it's a minimalist Python-like system. Note That Rho translates to pi, and Pi transcribes very efficiently from either text or binary.
* The top-level [Include folder](Include/KAI) and [Source Folder](Source).
* [Installation](Install.md): Information on how to install and build the system.
* [Tests](Test/Readme.md): Describes the Google Tests that indicate what doesn't currently work.
* The [top level CMake file](CMakeLists.txt).

## Documentation

KAI comes with comprehensive documentation:

* **[Installation Guide](Install.md)**: Step-by-step instructions for setting up KAI
* **[Language Tutorials](Doc/)**: Detailed guides for Pi, Rho, and Tau
* **[Architecture Overview](Doc/Architecure.md)**: In-depth explanation of KAI's design
* **[Networking Guide](Networking.md)**: How to use KAI's distributed features
* **[API Reference](Include/KAI/)**: Header files with detailed comments

## Use Cases

KAI is particularly well-suited for:

1. **Game Development**: Scripting, networking, and distributed AI
2. **Distributed Systems**: Coordinating computation across multiple nodes
3. **Simulation**: Sharing computational load for complex simulations
4. **Legacy Integration**: Exposing existing C++ libraries to scripting
5. **Real-time Systems**: Low-latency networking and computation

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

Christian Schladetsch - [christian.schladetsch@gmail.com](mailto:christian.schladetsch@gmail.com)

Project Link: [https://github.com/cschladetsch/kai](https://github.com/cschladetsch/kai)

## Conclusion

KAI provides a powerful foundation for distributed computing with seamless language integration. Its ability to expose C++ types without modification, distribute computation across networks, and maintain type safety throughout makes it unique in the field of distributed systems.

This library will be useful to those that want to expose C++ types and instances to the runtime and across the network. It allows you to script C++ in a very simple way, with no macros or modifications to existing classes.

The distributed iteration feature is a prime example of how KAI enables not just data sharing, but computational sharing across networked systems with minimal code changes.