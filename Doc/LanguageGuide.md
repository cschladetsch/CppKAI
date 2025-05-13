# KAI Language System Guide

The KAI system includes three integrated languages that serve different roles in the distributed computing environment. This guide provides an overview of these languages and links to detailed tutorials for each.

For an in-depth look at how these languages share a common underlying architecture, see the [Common Language System Architecture](CommonLanguageSystem.md) document.

## Language Overview

| Language | Purpose | Paradigm | Notable Features |
|----------|---------|----------|-----------------|
| **Pi** | Foundation language | Stack-based, RPN | Dual stacks, continuations, efficient execution |
| **Rho** | Application language | Infix, Python-like | Native continuations, Pi embedding, familiar syntax |
| **Tau** | Interface definition | Declarative, IDL | Network proxies, distributed objects, code generation |

## Common Features

All three languages in the KAI system share several important characteristics:

1. **Type Safety**: Strong typing throughout the language system
2. **Network Awareness**: Designed for distributed computing
3. **Reflection**: Full runtime reflection capabilities
4. **Integration**: Seamless interoperability between languages
5. **Extensibility**: Easy to extend with new types and operations

## Pi: The Foundation

Pi is a stack-based language that serves as the foundation of KAI's language system. All other languages ultimately compile down to Pi operations.

Key characteristics:
- **Stack manipulation**: Operations like dup, swap, drop, and over
- **Dual stacks**: Data stack for values, context stack for control flow
- **RPN syntax**: Operations follow their operands
- **Direct execution**: Efficient runtime evaluation

[Learn more in the Pi Tutorial](PiTutorial.md)

## Rho: The Application Language

Rho is an infix language designed for writing application logic. It combines a familiar Python-like syntax with powerful features like native continuations and direct Pi embedding.

Key characteristics:
- **Familiar syntax**: Infix notation, similar to Python or JavaScript
- **Pi integration**: Can embed Pi code blocks directly
- **Native continuations**: First-class support for advanced control flow
- **Translates to Pi**: Compiles to Pi operations for execution

[Learn more in the Rho Tutorial](RhoTutorial.md)

## Tau: The Interface Definition Language

Tau is KAI's Interface Definition Language (IDL), designed for defining how components communicate across network boundaries.

Key characteristics:
- **Interface definitions**: Clear contracts between components
- **Code generation**: Creates proxies and agents for network communication
- **Type safety**: Ensures consistent type handling across the network
- **Versioning**: Supports backward compatibility

[Learn more in the Tau Tutorial](TauTutorial.md)

## Language Interoperability

One of KAI's most powerful features is the seamless interoperability between its languages:

### Rho ↔ Pi Integration

```rho
// Embedding Pi code directly in Rho
result = 10 + pi{ 3 4 + }  // result = 10 + 7 = 17

// Accessing Rho variables from Pi
x = 5
pi_result = pi{ x @ 2 * }  // pi_result = 10
```

### Tau ↔ Implementation Language Integration

Tau definitions generate code that integrates with the implementation language (typically C++):

```cpp
// Using a Tau-generated proxy
auto userService = kai::Proxy<UserService>("node2:8080");
auto user = userService->GetUserById("user123");
```

## Choosing the Right Language

When working with KAI, choose the appropriate language based on your needs:

- **Pi**: For low-level operations, stack manipulation, or when maximum efficiency is required
- **Rho**: For application logic, algorithms, or when readability and familiarity are priorities
- **Tau**: For defining interfaces between distributed components or services

## Development Workflow

A typical development workflow with KAI's language system might look like:

1. Define component interfaces using **Tau**
2. Implement application logic using **Rho**
3. Optimize performance-critical sections with **Pi**
4. Connect components across the network using Tau-generated proxies

## Tools and Environment

KAI provides several tools for working with its languages:

- **Console**: Interactive REPL for Pi and Rho
- **Code generators**: For processing Tau IDL files
- **Debuggers**: For tracing execution and viewing stack state
- **Network monitors**: For tracking distributed object communication

## Getting Started

The best way to get started with KAI's language system is to:

1. Learn basic Pi operations from the [Pi Tutorial](PiTutorial.md)
2. Become familiar with Rho syntax using the [Rho Tutorial](RhoTutorial.md)
3. Understand distributed object modeling with the [Tau Tutorial](TauTutorial.md)
4. Experiment with the Console application to try examples

## Conclusion

KAI's integrated language system provides a powerful foundation for distributed computing. By combining the efficiency of Pi, the expressiveness of Rho, and the interface clarity of Tau, developers can build robust distributed applications that scale across networks.

For more detailed information on each language, please refer to the specific tutorials linked above.