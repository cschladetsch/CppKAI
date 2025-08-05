# Tau

Tau is KAI's Interface Definition Language (IDL), designed for describing networked objects, interfaces, and services within the KAI distributed object model. It provides a way to define how components communicate across the network.

**[Complete Tau Architecture Diagrams](../../../../Doc/TauArchitectureDiagrams.md)** - Complete visual documentation of Tau's IDL processing pipeline, multi-target code generation, and network integration architecture with detailed Mermaid diagrams.

### Tau Interface Definition Language Pipeline

```mermaid
graph TB
    subgraph "Tau Source Code"
        SRC["Tau IDL<br/>interface Calculator {<br/>  float add(float, float);<br/>}"]
    end
    
    subgraph "Lexical Analysis"
        LEX["Tau Lexer<br/>Tokenization"]
        TOKENS["Token Stream<br/>INTERFACE, IDENTIFIER<br/>FLOAT, IDENTIFIER"]
    end
    
    subgraph "Syntax Analysis"
        PAR["Tau Parser<br/>AST Construction"]
        AST["Tau AST Nodes<br/>Interface nodes<br/>Method nodes<br/>Type nodes"]
    end
    
    subgraph "Code Generation"
        GEN["Tau Generator<br/>Multi-target generation"]
        PROXY["Proxy Generation<br/>Client-side stubs"]
        AGENT["Agent Generation<br/>Server-side handlers"]
        STRUCT["Struct Generation<br/>Data structures"]
    end
    
    subgraph "Generated Output"
        CPP_PROXY["C++ Proxy Classes<br/>Network client code"]
        CPP_AGENT["C++ Agent Classes<br/>Network server code"]  
        CPP_STRUCT["C++ Struct Definitions<br/>Data transfer objects"]
    end
    
    SRC --> LEX
    LEX --> TOKENS
    TOKENS --> PAR
    PAR --> AST
    AST --> GEN
    GEN --> PROXY
    GEN --> AGENT
    GEN --> STRUCT
    PROXY --> CPP_PROXY
    AGENT --> CPP_AGENT
    STRUCT --> CPP_STRUCT
    
    style SRC fill:#e1bee7
    style GEN fill:#ff9800
    style CPP_PROXY fill:#4caf50
    style CPP_AGENT fill:#2196f3
    style CPP_STRUCT fill:#9c27b0
```

## Key Features

- **Interface-focused**: Defines the public interfaces of networked objects
- **Network-oriented**: Designed for cross-network communication
- **Type-safe**: Ensures type compatibility across network boundaries
- **Declarative**: Focuses on what rather than how

## Syntax Overview

Tau uses the same lexing and parsing systems as the interpreted languages Pi and Rho. **Unlike** those systems, Tau is an Interface Definition Language. The input is `.tau` files; the output is C++ files for:

1. Proxies - Client-side interfaces that forward calls to remote agents
2. Agents - Server-side implementations that receive and process remote calls

### Example

```tau
namespace Trading {
    // Define an interface
    interface ITrader {
        // Method definitions
        bool PlaceOrder(string symbol, int quantity, float price);
        void CancelOrder(string orderId);
        
        // Define an event
        event OrderPlaced(string symbol, int quantity, float price);
    }
}
```

### Tau Proxy Generation Pattern

```mermaid
sequenceDiagram
    participant Client as Client Code
    participant Proxy as Generated Proxy
    participant Network as Network Layer
    participant Agent as Remote Agent
    participant Server as Server Implementation
    
    Client->>Proxy: calculator.add(5.0, 3.0)
    Proxy->>Proxy: Serialize parameters
    Proxy->>Network: Send method call request
    Network->>Agent: Receive method call
    Agent->>Agent: Deserialize parameters
    Agent->>Server: Call actual add(5.0, 3.0)
    Server->>Agent: Return result: 8.0
    Agent->>Agent: Serialize result
    Agent->>Network: Send response
    Network->>Proxy: Receive response
    Proxy->>Proxy: Deserialize result
    Proxy->>Client: Return 8.0
```

## Recent Enhancements

- Support for C++17 nested namespace syntax (`namespace A::B::C`)
- Support for interface inheritance hierarchies
- Improved event handling with callback registration
- Enhanced type system with enums, structs, and complex types
- Better error reporting during parsing and code generation

## Usage

To generate code from Tau definitions, use the NetworkGenerate application:

```
NetworkGenerate --input=MyInterface.tau --output=GeneratedCode --proxy --agent
```

For detailed usage instructions and syntax guide, see:
1. [Tau Tutorial](../../../Doc/TauTutorial.md) for comprehensive documentation
2. [App/NetworkGenerate](../../../Source/App/NetworkGenerate) for the code generation tool

## Testing

Extensive test suites are available to demonstrate Tau's capabilities:
- Basic syntax and parsing tests
- Advanced type system tests
- Code generation tests for proxies and agents