# Tau Code Generation

This module contains the code generation components for the Tau Interface Definition Language (IDL). It transforms Tau AST (Abstract Syntax Tree) into C++ code for distributed networking.

## Code Generation Components

The Tau generator produces C++ code for:

1. **Proxies** - Client-side interfaces that forward method calls to remote agents
   - Methods that match interfaces defined in Tau
   - Event registration/unregistration handlers
   - Serialization of parameters and return values

2. **Agents** - Server-side implementations that receive and handle remote calls
   - Method handlers that receive remote calls from proxies
   - Event triggering mechanisms
   - Deserialization of parameters and serialization of return values

3. **Process** - Core generation functionality shared between proxies and agents
   - AST traversal and transformation
   - Common code template rendering
   - Error checking and validation

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