# Tau Code Generation

This module contains the code generation components for the Tau Interface Definition Language (IDL). It transforms Tau AST (Abstract Syntax Tree) into C++ code for distributed networking.

## Code Generation Architecture

The Tau code generation system uses a clean, separated architecture:

### Base Class
- **GenerateProcess** - Base class providing common functionality for all generators
  - AST traversal and transformation
  - Code formatting utilities (StartBlock, EndBlock, etc.)
  - Common parsing and error handling

### Specialized Generators
1. **GenerateProxy** - Generates client-side proxy classes
   - Methods that forward calls over the network
   - Event registration/unregistration handlers
   - Serialization of parameters using RakNet BitStreams
   - Future-based return value handling

2. **GenerateAgent** - Generates server-side agent classes
   - Handler methods for incoming network requests
   - Deserialization of parameters from BitStreams
   - Implementation method invocation
   - Response serialization for non-void methods

3. **GenerateStruct** - Generates plain C++ struct definitions
   - Simple data structures for DTOs (Data Transfer Objects)
   - Preserves field ordering and types
   - Supports nested structures
   - Can include method declarations

## Key Design Principles

- **Separation of Concerns**: Each generator has a single responsibility
- **Type Safety**: Generated code maintains full C++ type safety
- **Network Transparency**: Proxies look and feel like local objects
- **Clean Inheritance**: All generators properly inherit from GenerateProcess

## Usage

The generation process happens in several stages:

1. Parse Tau source code into an AST
2. Process the AST to extract interfaces, methods, events, and types
3. Generate code based on the extracted information
4. Write the generated code to output files

This is handled by the NetworkGenerate application, which provides a command-line interface to the generator.

## Example

For a Tau interface:

```tau
namespace ChatApp {
    interface IChatService {
        void SendMessage(string user, string message);
        string[] GetRecentMessages(int count = 10);
        
        event MessageReceived(string user, string message, string timestamp);
    }
}
```

The generator will produce:

1. A proxy class `ChatApp::IChatServiceProxy` with:
   - `void SendMessage(string user, string message)` method
   - `string[] GetRecentMessages(int count = 10)` method
   - `RegisterMessageReceivedHandler(std::function<void(string, string, string)>)` method

2. An agent class `ChatApp::IChatServiceAgent` with:
   - `SendMessage` and `GetRecentMessages` handler implementations
   - `TriggerMessageReceived` method to raise the event

## Recent Improvements

- Support for C++17 nested namespace syntax (`namespace A::B::C`)
- Improved event handling with callback registration
- Enhanced error reporting during code generation
- Better support for complex type hierarchies and inheritance

For practical examples, see:
1. [NetworkGenerate application](../../../../Source/App/NetworkGenerate)
2. [Tau Tutorial](../../../../Doc/TauTutorial.md)
3. [Tau code generation tests](../../../../Test/Language/TestTau)