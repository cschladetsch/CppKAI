# Executor

An **Executor** does things on other things; it changes state of objects via execution of continuations.

There is one common *Executor* for multiple languages. Think of the **Executor** as the *Virtual Machine* for the KAI system.

**KAI** supports multiple languages: Pi, Rho, and Tau. All of these languages are executed on the same type of *Executor*.

## Architecture

The Executor follows a clean separation of concerns:

1. **Console** - Handles user interaction and passes input to the Translator
2. **Translator** - Converts language-specific syntax to Continuations
3. **Executor** - Executes Continuations in a language-agnostic manner

All languages are ultimately translated into Pi operations, which are then executed by the Executor. This architecture ensures that the Executor only needs to handle Pi operations, simplifying the codebase and improving maintainability.

## Stack-Based Execution

The Executor maintains two primary stacks:
- **Data Stack** - Contains values being operated on
- **Context Stack** - Manages execution flow and continuations

Operations like `Dup`, `Swap`, `Drop`, and `Over` manipulate the Data Stack, while operations like `&`, `...`, and `!` work with the Context Stack.

## Core Operations

Core Pi operations implemented in the Executor include:
- Stack manipulation (Dup, Swap, Drop, Over)
- Arithmetic operations (Plus, Minus, Multiply, Divide)
- Logical operations (And, Or, Xor, Not)
- Comparison operations (Equiv, NotEquiv, Less, Greater)
- Variable operations (Store, Retrieve)
- Control flow (WhileLoop, ForLoop, If, IfElse)

Each operation is implemented to maintain Pi's stack-based semantics while ensuring type safety and error handling.
