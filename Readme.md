# Kai ![Foo](Doc/swords.jpg)
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

### Heart of the System :heart:
At the heart of KAI are three things: A Registry, A Domain, and some Languages.

* Registry. A statically-typesafe over-the-wire object factory.
* Domain. A Local Registry that is part of the larger Newtork. Objects in a Domain are in a Registry that is qualified by the Guid of that domain.
* Languages:
  * Pi. Is RPN and is the base language.
  * Rho. Is like Python but has native support for continuations in its syntax. It transposes to Pi.
  * Tau. Is the Interface Definition Language (IDL) than is shared between nodes. In theory, it could produce code for any language.

## Pi
[Pi](Source/Library/Language/Pi) (see [Tests](Test/Language/TestPi) and [Tests Scripts](Test/Language/TestPi/Scripts)) is heavily influenced by [Forth](https://en.wikipedia.org/wiki/Forth_(programming_language)). It has two directly interactable stacks: one for data, and one for context. The data stack is used for operations (as is the context stack), but the context stack tells the machine `where to go next`. This is used to create the idea of a co-routine, which is then pushed up to Rho.

## Rho
[Rho](Source/Library/Language/Rho) (see [Tests](Test/Language/TestRho) and [Tests Scripts](Test/Language/TestRho/Scripts)), is an infix language much like Python, LUA or Ruby, but with native continuations and the ability to inject Pi code as any factor in an expression.

The general idea has always been to move algorithms around the network, as well as data. In this manner, *real* load-balancing can be conducted.

## Platforms
Currently supported platforms are:

1. **Windows 10** (VS 2017-19)
1. **Linux** (Ubuntu)
1. **macOS** (Sierra)
1. **Unity3d** (2017+)

You can create and connect Kai nodes on different machines, swap and monitor workloads, and remote manage all nodes in the system.

Kai comes with distributed tri-color garbage collection. It is incremental; there are no spikes in cost for the GC over time. It is smooth, and allows for a constant update loop times even with tens of thousands of objects, and with hundreds of objects being created each frame on the compute power of a gaming console from 2012.

There is also an Interface Definition Language (IDL) called ***Tau***, which is used to generate code for proxies and agents in the system.

Refer to the [Language Systems](Include/KAI/Language) and [implementation](Source/Library).

## Building :zap:

Prerequisites:

* A modern C++ compiler.
* [Cmake](https://cmake.org/install/).

```bash
$ sudo apt-get install cmake
```

* [Boost](https://www.boost.org/). The specific packages required are `filesystem`, `chrono`, `programoptions`, `date-time` and `regex`.

```bash
$ sudo apt-get install libboost-filesystem-dev libboost-chrono-dev libboost-regex-dev libboost-program-options-dev libboost-date-time-dev
```

After this do the usual:

```bash
mkdir -p build && cd build && cmake ..
```

Finally, make sure to use the same compiler/linker for C and C++ and generate Ninja:

```
$ cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -G Ninja && ninja
```

## Console

The basic [Console](Source/App/Console) supports both Pi and Rho as a Repl shell. It works and colored on Windows, Linux, and macOs. The following is just a basic look at the shell.

![Image](Images/BasicConsole.png)

### Window
There is also a [Gui](/Source/App/Window) based on imgui.

## Examples
Basic C++/runtime interaction. First, see [Sample use of non-POD structure](Test/Source/TestClassScripting.cpp). Note that to be used by KAI, the target struct or class has no conceptual or practical requirements. Specifically, it doesn't have to derive from anything and there are no macros used to expose fields or methods:

Part of the output is:

```
[----------] 1 test from TestClassScripting
[ RUN      ] TestClassScripting.Test
Info: mystruct.ToXmlString()='
<Object type='MyStruct' name=''> <!-- no name because structure is not in a dictionary -->
  <Property name='num'>42>/Property>
  <Property name='string'>Freddy</Property>
</Object>
'

Info: stream.ToString()='Handle=55, type=MyStruct '
Info: binary_stream='BinaryStream: size=32'
```

After building, you can run the the tests yourself in ```Bin/Test/KaiTest.exe```. (Or without the `.exe` on Linux of macOS of course).

Start withe the [Unit Tests](Test) then have a look at the [Applications](Source/App).

## Networking
The entire motivation for KAI was to allow for efficient, low-latency and correct networking of object state and command execution (which results in state changes!) across a group of Objects in a Registry, a group of Registries in a Domain, and across a group of Domains in a Network System.

Read more about Kai [object and compuational distribution](Networking.md).

## Executor
A general-purpose stack-based virtual machine. I wanted two stacks (one for data, one for context). These two stacks (data and context) provide some abilities that are not available on any other non-Forth based system - especially since you can readily swap between them. That is, you can move _context_ to the data-stack, do work, then push back onto the _context_ stack.

This provides for some rich computational control. This can also result in confusion, so there is a debugger and extensive (optional) tracing available.

## Logging System
KAI includes a comprehensive logging system that captures debug traces, network events, and application messages:

* **Centralized Storage**: All logs are written to the `/Logs` directory for easy access and management.
* **Log Categorization**: Logs are separated by type and component for better organization:
  * Core system logs (debug, info, warning, error, fatal)
  * Network-specific logs (connections, messages, discovery events, status updates)
* **Timestamp Integration**: All log entries include timestamps for precise event tracking.
* **Console Mirroring**: Important log messages are also displayed in the console with color coding.

The logging system is used throughout KAI to track system events, diagnose issues, and monitor network activity. This is especially useful when running distributed computations across multiple nodes.

## Folder Structure
* *Bin*. Where to write executable output files.
* *CMake*. Auxiliary CMake modules, primarily for finding other projects and libraries like Boost and Google Test.
* *Doc*. Generated documentation.
* *Ext*. External dependencies, primarily as _git submodules_.
* *Include*. Root of the global include path. Add this to your compiler's include path.
* *Lib*. Location for built static and dynamic library files. Also add this this to your compilers library path.
* *Logs*. Directory for all system logs including debug traces, network events, and application logs.
* *Source*. The root for the source code of the project.
* *Test*. Unit tests. Output goes to *Bin/Tests*.

## Interesting Files and locations
* The Test scripts for [Pi](Test/Language/TestPi/Scripts) shows the basics of pi.
* Similarly, the test scripts for [Rho](Test/Language/TestRho/Scripts) show that it's a minimalist Python-like system. Note That Rho translates to pi, and Pi transcribes very efficiently from either text or binary.
* The top-level [Include folder](Include/KAI) and [Source Folder](Source).
* [Installation](Install.md). Some information on how to install and build the system.
* [Tests](Test/Readme.md). Describes the Google Tests that indicate what doesn't currently work.
* The [top level CMake file](CMakeLists.txt).

## Installing and Running
See [Install.md](Install.md) for installation instructions.

The project will build using _CMake_ via _Visual Studio 2019_. No more mucking around with various *nix shells on Windows.

Your safest first bet is to build the *Console* app. This gives you a *Pi* Repl console.

_RakNet_ is integrated but needs some work to get back to connecting to remote nodes etc., due to recent changes in *Raknet*. This is a top priority.

_KAI_ has various dependencies, but can be built with many sub-sets. That is, if you don't want to use [ImGui](https://github.com/ocornut/imgui) you can stick with text-based (colored) [Console](Source/App/Console).

Feel free to contact [me](matilto:christian.schladetsch@gmail.com) with any questions about building or use of the system.

## Distributed Iteration with AcrossAllNodes

One of the most powerful features of KAI is its ability to distribute both computation and data across network nodes. The `AcrossAllNodes` operation exemplifies this capability by enabling distributed parallel iteration over collections.

### How It Works

`AcrossAllNodes` enables the distribution of iteration tasks across multiple network nodes. It takes three arguments:

1. A network node (or null for local execution)
2. A collection to iterate over
3. A function to apply to each element

The operation then distributes the workload across connected network peers, collects the results, and returns them as a new collection.

### Example in Rho

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

### Demo Interaction

Here's an example of what a session might look like when using the distributed iteration feature:

```
> // Create a network node and connect to peers
> node = createNetworkNode()
Node(5c7a8b4d-e9f0-4a2d-9c7b-8a5d3e6f2c1a)

> node.listen(14589)
Listening on port 14589...

> node.connect("192.168.1.10", 14589)
Connected to peer at 192.168.1.10:14589

> node.connect("192.168.1.11", 14589)
Connected to peer at 192.168.1.11:14589

> // Create test data
> data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

> // Define a function to square numbers
> fun square(x) { x * x }
<Function:square>

> // Run the calculation locally
> start = currentTimeMillis()
1683042851342

> result1 = acrossAllNodes(null, data, square)
[1, 4, 9, 16, 25, 36, 49, 64, 81, 100]

> end = currentTimeMillis()
1683042851345

> print("Local execution took " + (end - start) + "ms")
Local execution took 3ms

> // Run the calculation distributed across the network
> start = currentTimeMillis()
1683042851350

> result2 = acrossAllNodes(node, data, square)
[1, 4, 9, 16, 25, 36, 49, 64, 81, 100]

> end = currentTimeMillis()
1683042851352

> print("Network execution took " + (end - start) + "ms")
Network execution took 2ms

> // Verify results are identical
> result1 == result2
true
```

For small calculations on a simple array like this, the performance difference is minimal. However, for large datasets and computationally intensive operations, the network distribution can provide significant speedups by leveraging the computational power of multiple machines.

### Supported Collection Types

`AcrossAllNodes` supports the following collection types:

- **Array**: Distributes array elements across network nodes
- **List**: Distributes list elements across network nodes
- **Map**: Distributes map entries as key-value pairs across network nodes
- **String**: Treats the string as a collection of characters to process individually

### Advanced Features

When a network node is provided, the `AcrossAllNodes` operation:

1. Analyzes the size and complexity of the workload
2. Distributes elements to connected peers based on their available capacity
3. Handles failures and retries if a node becomes unresponsive
4. Aggregates results back into a single collection
5. Maintains type consistency between input and output collections

This enables true distributed parallel processing with minimal developer effort.

## Conclusion
This library will be useful to those that want to expose C++ types and instances to the runtime, and across the network.

It allows you to script C++ in a very simple way. Adding a new 'built-in type' to the system requires no macros, but just defining the type-traits for your class. No modifications to any class is required. After that you can script with any type and instances as you wish.

This means you can expose and script other classes as well, including those in a library that you do not have the source code to.

To be clear: you do not have to change the source code of a class in order to access and use it at runtime with KAI. Further, these exposed classes are directly accessible via the Network.

The distributed iteration feature is a prime example of how KAI enables not just data sharing, but computational sharing across networked systems with minimal code changes.

A REPL [Console](Source/App/Console) is also supplied, as well as a [Windowed](Source/App/Window) application on all platforms.


