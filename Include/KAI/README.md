# KAI Header Files

This directory contains the core header files for the KAI distributed object model system.

## Main Headers

### KAI.h
Primary include file that brings in the essential KAI functionality. Include this for basic KAI usage.

### Base.h
Foundation types and macros used throughout the KAI system.

### ClassBuilder.h
Utilities for building and registering C++ classes with the KAI reflection system.

### Language.h
Language enumeration and language-related utilities for Pi, Rho, and Tau.

## Core System Headers

### Core/
Core object model, type system, and reflection:
- **Object.h** - Base object interface and smart object wrappers
- **Registry.h** - Object factory and type registration
- **Type.h** - Type system and type information
- **Tree.h** - Object hierarchies and scoping
- **Exception.h** - KAI exception system

### Executor/
Execution engine for KAI languages:
- **Executor.h** - Stack-based virtual machine
- **Compiler.h** - Code compilation and generation
- **Continuation.h** - Continuation objects for advanced control flow

### Language/
Multi-language support system:
- **Common/** - Shared language infrastructure
- **Pi/** - Stack-based postfix language
- **Rho/** - Infix C-like language  
- **Tau/** - Interface definition language
- **Hlsl/** - High-level shading language support

### Network/
Distributed computing and networking:
- **Network.h** - Main networking interface
- **Node.h** - Network nodes and peer management
- **Agent.h** - Network agents and proxies
- **Connection.h** - Network connection management

### Console/
Interactive console system:
- **Console.h** - Main console interface with networking support
- **ConsoleColor.h** - Color and formatting utilities

### Platform/
Platform-specific adaptations:
- **Android/**, **iOS/** - Mobile platform support
- **Linux/**, **OSX/**, **Windows/** - Desktop platforms
- **Platform.h** - Platform abstraction layer

## Memory Management

### Core/Memory/
Advanced memory management system:
- **IAllocator.h** - Allocator interface
- **StandardAllocator.h** - Standard memory allocator
- **MonotonicAllocator.h** - High-performance allocator

## Object System Details

### Core/Object/
Detailed object model implementation:
- **Class.h** - Class definition and metadata
- **Method.h** - Method binding and invocation
- **Property.h** - Property access and reflection
- **Storage.h** - Object storage and lifetime management

## Usage Patterns

### Basic KAI Usage
```cpp
#include "KAI/KAI.h"
using namespace kai;

// Creates registry and basic objects
Registry registry;
auto obj = registry.New<MyClass>();
```

### Console Integration
```cpp
#include "KAI/Console/Console.h"
using namespace kai;

Console console;
console.Run();  // Interactive REPL
```

### Network Programming
```cpp
#include "KAI/Network/Network.h"
using namespace kai;

Node node;
node.Connect("localhost", 8080);
```

### Language Integration
```cpp
#include "KAI/Language/Pi/Pi.h"
#include "KAI/Language/Rho/Rho.h"

// Execute Pi code
PiTranslator pi(registry);
auto result = pi.Translate("2 3 +");

// Execute Rho code  
RhoTranslator rho(registry);
auto result2 = rho.Translate("x = 2 + 3;");
```

## Header Dependencies

The header files are organized with minimal dependencies:
- **Base.h** - No dependencies, foundation types
- **Core/** - Depends on Base.h and platform headers
- **Language/** - Depends on Core/ and Executor/
- **Network/** - Depends on Core/ and external RakNet
- **Console/** - Depends on Core/, Executor/, Language/, and Network/

## Platform Support

Headers include platform-specific code for:
- **Windows** (XP, 7, 8, 10, 11)
- **Linux** (Ubuntu, CentOS, others)
- **macOS** (10.12+)
- **Mobile** (iOS, Android)
- **Gaming** (Xbox, PlayStation - legacy support)

## Thread Safety

Most KAI objects are not thread-safe by default. For multi-threaded usage:
- Use separate Registry instances per thread
- Synchronize access to shared objects
- Network operations include internal synchronization

## Related Documentation

- [Core System](../../../Doc/Architecure.md)
- [Language System](../../../Doc/CommonLanguageSystem.md)
- [Network Architecture](../../../Doc/NetworkArchitecture.md)
- [Build Instructions](../../../Doc/BUILD.md)