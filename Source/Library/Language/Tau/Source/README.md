# Tau Source Implementation

The Tau language implementation is divided into two main components:

## Tau Language

This section contains the lexer and parser for the Tau Interface Definition Language (IDL). It processes `.tau` source files and generates an Abstract Syntax Tree (AST) that represents the structure of the interface definitions.

Key components:
- **TauLexer**: Tokenizes Tau source code into a stream of tokens
- **TauToken**: Defines the token types used in the Tau language
- **TauParser**: Parses token streams into an AST
- **TauAstNode**: Represents nodes in the Tau AST

Recent improvements:
- Support for C++17 nested namespace syntax (`namespace A::B::C`)
- Enhanced error reporting during parsing
- Better handling of complex type hierarchies and inheritance
- Improved support for default values and parameters

## Tau Generator Library

This component uses the Tau AST to generate network Proxy and Agent C++ code. It's used by the NetworkGenerate command-line utility to produce working C++ classes from Tau interface definitions.

Key components:
- **GenerateProcess**: Abstract base class for all code generators
- **GenerateProxy**: Generates client-side proxy classes that forward method calls to remote agents
- **GenerateAgent**: Generates server-side agent classes that receive and handle remote calls

The generated code includes:
1. Method definitions that match the Tau interfaces
2. Event handling with registration and callback mechanisms
3. Parameter serialization/deserialization
4. Network communication plumbing

## Testing

The Tau implementation includes comprehensive test suites:
- Basic lexer and parser tests
- Advanced type system tests
- Code generation tests for proxies and agents
- Integration tests with the networking infrastructure

For practical examples and documentation, see:
1. [Tau Tutorial](../../../../../Doc/TauTutorial.md)
2. [NetworkGenerate application](../../../../../Source/App/NetworkGenerate)
3. [Test suites](../../../../../Test/Language/TestTau)