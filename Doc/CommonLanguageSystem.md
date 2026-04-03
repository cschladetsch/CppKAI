# KAI Common Language System Architecture

This document explains the underlying architecture shared by all languages in the KAI system. Understanding this common foundation helps developers better work with Pi, Rho, and Tau, and explains how these languages seamlessly interoperate.

## Architecture Overview

All KAI languages (Pi, Rho, and Tau) share a common architecture with three main components:

1. **Lexer**: Transforms raw text into tokens
2. **Parser**: Converts tokens into an Abstract Syntax Tree (AST)
3. **Translator**: Transforms the AST into executable code

This architecture follows a classic compiler design with some KAI-specific enhancements for distributed computing and interoperability.

```
┌────────┐     ┌────────┐     ┌────────┐     ┌────────────┐
│  Text  │ --> │ Lexer  │ --> │ Parser │ --> │ Translator │ --> Executable
└────────┘     └────────┘     └────────┘     └────────────┘        Code
```

## The Common Type System

At the heart of KAI's language architecture is a unified type system that ensures consistency across all languages.

### Core Types

Every KAI language understands and can work with these fundamental types:

| Type | Description | Example |
|------|-------------|---------|
| `Void` | Absence of a value | |
| `Bool` | Boolean value | `true`, `false` |
| `Number` | Numeric values | `42`, `3.14` |
| `String` | Text | `"hello"` |
| `Label` | Identifier | `'variable'` |
| `Array` | Ordered collection | `[1, 2, 3]` |
| `Map` | Key-value pairs | `{key: value}` |
| `Continuation` | Executable code block | `{ code }` |
| `Object` | Base for all user types | |

### Type Registry

The `Registry` is central to KAI's type system:

- Acts as a factory for creating objects
- Maintains type information and relationships
- Ensures type safety across network boundaries
- Allows runtime reflection and introspection
- Enables serialization and deserialization

```cpp
// Example of the Registry in C++
Registry registry;
registry.AddClass<Vector3>("Vector3");

// Creating an object through the registry
Object vector = registry.New("Vector3", 1.0f, 2.0f, 3.0f);
```

## Common Execution Model: Executor

All KAI languages ultimately execute through the `Executor`, which:

1. Maintains the **data stack** for values
2. Maintains the **context stack** for continuations
3. Processes **operations** on these stacks
4. Manages **scopes** for variable lookup
5. Handles **exceptions** and error conditions

```
┌─────────────────────────────────────┐
│              Executor               │
├─────────────┬───────────────────────┤
│ Data Stack  │ [value1, value2, ...] │
├─────────────┼───────────────────────┤
│ Context     │ [continuation1, ...]  │
│ Stack       │                       │
├─────────────┼───────────────────────┤
│ Operations  │ Plus, Minus, Store... │
├─────────────┼───────────────────────┤
│ Scopes      │ Global, Local, ...    │
└─────────────┴───────────────────────┘
```

### Operations

The Executor understands a fixed set of operations that form the basis of all language functionality:

- **Stack operations**: `Dup`, `Swap`, `Drop`, `Rot`, `Clear`, etc.
- **Arithmetic**: `Plus`, `Minus`, `Multiply`, `Divide`, etc.
- **Logic**: `LogicalAnd`, `LogicalOr`, `LogicalNot`, etc.
- **Control flow**: `IfThenSuspend`, `WhileLoop`, `DoLoop`, etc.
- **Memory**: `Store`, `Retreive`, `Remove`, etc.
- **Type manipulation**: `New`, `GetProperty`, `SetProperty`, etc.
- **Context**: `Suspend`, `Resume`, `Return`, etc.

All language-specific features ultimately translate into combinations of these basic operations.

## Translation Layer

The translation layer is what allows different syntaxes (Pi, Rho, Tau) to work with the same underlying execution model:

```
┌───────┐     ┌─────────────┐     ┌─────────────┐
│  Rho  │ --> │ Rho         │ --> │             │
└───────┘     │ Translator  │     │             │
              └─────────────┘     │             │
┌───────┐     ┌─────────────┐     │  Executor   │
│  Pi   │ --> │ Pi          │ --> │             │
└───────┘     │ Translator  │     │             │
              └─────────────┘     │             │
┌───────┐     ┌─────────────┐     │             │
│  Tau  │ --> │ Tau         │ --> │             │
└───────┘     │ Translator  │     └─────────────┘
              └─────────────┘
```

Each translator converts its language's specific syntax into:

1. **Pi**: Directly translates to Executor operations
2. **Rho**: Translates to Pi code, which then executes as operations
3. **Tau**: Generates code (C++, C#, etc.) that interfaces with the KAI system

## Abstract Syntax Tree (AST)

All KAI languages use a consistent AST structure to represent code:

```
         Program
            │
    ┌───────┴───────┐
    │               │
 Statement       Statement
    │               │
    ▼               ▼
Expression      Expression
    │
    ▼
  Tokens
```

Common node types include:
- `Program`: Root of the AST
- `Block`: Group of statements
- `Expression`: Value-producing code
- `Assignment`: Variable assignment
- `BinaryOp`: Binary operations (+, -, etc.)
- `Call`: Function/method calls
- `Conditional`: If/else structures
- `Loop`: While, for, do-while loops
- `Function`: Function definitions

## Network Distribution Layer

The common language system includes a network distribution layer that enables:

1. **Object proxying**: Transparent access to remote objects
2. **Code distribution**: Moving algorithms between nodes
3. **Event propagation**: Network-wide event handling
4. **Discovery**: Finding available services

This is implemented through:

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Node A    │     │  Network    │     │   Node B    │
├─────────────┤     │             │     ├─────────────┤
│ Objects     │<--->│ Serialized  │<--->│ Objects     │
│ Registry    │     │ Data        │     │ Registry    │
│ Executor    │     │             │     │ Executor    │
└─────────────┘     └─────────────┘     └─────────────┘
```

## Continuations: The Unifying Concept

Continuations are the key unifying concept across KAI languages:

- **Definition**: Encapsulated executable code with its own scope
- **Representation**: `Continuations` are first-class objects in the system
- **Usage**: Control flow, functions, callbacks, coroutines
- **Network**: Can be serialized and moved between nodes

All KAI languages provide syntax for working with continuations:

- **Pi**: `{ code }` creates a continuation
- **Rho**: `fun() { code }` defines a function (continuation)
- **Tau**: Defines interfaces that generate continuations at runtime

## Examples of Translation

### Pi to Operation Codes

Pi code:
```
2 3 + 4 *
```

Translates to these operations:
1. Push value `2` onto data stack
2. Push value `3` onto data stack
3. Execute `Plus` operation (consumes 2 & 3, pushes 5)
4. Push value `4` onto data stack
5. Execute `Multiply` operation (consumes 5 & 4, pushes 20)

### Rho to Pi Translation

Rho code:
```rho
x = 2 + 3 * 4
```

First translates to Pi:
```
4 3 * 2 + 'x #
```

Then executes as operations:
1. Push value `4` onto data stack
2. Push value `3` onto data stack
3. Execute `Multiply` operation (consumes 3 & 4, pushes 12)
4. Push value `2` onto data stack
5. Execute `Plus` operation (consumes 2 & 12, pushes 14)
6. Push label `'x` onto data stack
7. Execute `Store` operation (associates 14 with 'x in current scope)

### Tau to Code Generation

Tau code:
```tau
interface Calculator {
    int Add(int a, int b);
}
```

Generates:

1. **Proxy code** (client side):
```cpp
class Calculator_Proxy : public ICalculator {
public:
    virtual int Add(int a, int b) override {
        // Network serialization and call
        return NetworkCall("Add", a, b);
    }
};
```

2. **Agent code** (server side):
```cpp
class Calculator_Agent : public AgentBase {
public:
    void RegisterMethods() {
        Register("Add", &ActualCalculator::Add);
    }
    
    void DispatchCall(const String& method, Args& args) {
        if (method == "Add") {
            int a = args[0].As<int>();
            int b = args[1].As<int>();
            args.SetResult(implementation->Add(a, b));
        }
    }
};
```

## Memory Management

The common language system uses a uniform approach to memory management:

- **Object lifecycle**: Created by Registry, managed by reference counting
- **Garbage collection**: Incremental tri-color mark and sweep
- **Pooling**: Object pooling for frequent allocations
- **Cross-network**: References can span network boundaries

This ensures consistent behavior across languages and distributed components.

## Extending the System

The KAI language system is designed for extensibility:

1. **Adding types**: Register new C++ types with the Registry
2. **Custom operations**: Add new operations to the Executor
3. **Language features**: Extend translators to support new syntax
4. **Network protocols**: Add serialization for new types

## Conclusion

KAI's common language system provides a robust foundation for distributed computing:

- **Unified execution model**: All languages share the same core semantics
- **Type safety**: Consistent typing across languages and network boundaries
- **Network transparency**: Seamless distribution of objects and code
- **Language interoperability**: Different syntaxes for different purposes
- **Extensibility**: Easy to extend with new types and operations

This architecture enables the full power of KAI: running distributed applications with multiple languages that all share the same underlying model of computation.

## References

- [Pi Language Tutorial](PiTutorial.md)
- [Rho Language Tutorial](RhoTutorial.md)
- [Tau Language Tutorial](TauTutorial.md)
- [Language Guide](LanguageGuide.md)
- [KAI Architecture](Architecure.md)