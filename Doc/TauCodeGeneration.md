# Tau Code Generation Architecture

## Overview

Tau is the Interface Definition Language (IDL) for the KAI distributed object model. It defines service interfaces and generates the necessary proxy and agent code for network communication. This document details the code generation architecture and how to use it.

## Code Generation Classes

The Tau code generation system is built on a hierarchy of specialized generators:

### GenerateProcess (Base Class)
- **Location**: `Include/KAI/Language/Tau/Generate/GenerateProcess.h`
- **Purpose**: Base class providing common functionality for all code generators
- **Key Methods**:
  - `Generate()` - Main entry point for code generation
  - `Parse()` - Parses Tau IDL input
  - `Module()`, `Namespace()`, `Class()` - AST traversal methods
  - `StartBlock()`, `EndBlock()` - Code formatting utilities

### GenerateProxy
- **Location**: `Include/KAI/Language/Tau/Generate/GenerateProxy.h`
- **Purpose**: Generates client-side proxy classes for remote procedure calls
- **Output**: C++ proxy classes that forward method calls over the network
- **Key Features**:
  - Generates proxy methods that serialize arguments
  - Handles return values via futures
  - Supports event registration/unregistration
  - Creates type-safe RPC interfaces

### GenerateAgent  
- **Location**: `Include/KAI/Language/Tau/Generate/GenerateAgent.h`
- **Purpose**: Generates server-side agent classes for handling incoming requests
- **Output**: C++ agent classes that receive and process network requests
- **Key Features**:
  - Generates handler methods for each interface method
  - Deserializes incoming arguments
  - Invokes implementation methods
  - Sends responses for non-void methods

### GenerateStruct
- **Location**: `Include/KAI/Language/Tau/Generate/GenerateStruct.h`
- **Purpose**: Generates plain C++ struct definitions from Tau IDL
- **Output**: Simple struct definitions for data transfer objects
- **Key Features**:
  - Creates POD structures
  - Preserves field ordering
  - Supports nested structures
  - Can include method declarations

## Usage Examples

### Basic IDL Definition

```tau
namespace MyApp {
    struct UserData {
        int id;
        string name;
        string email;
    }
    
    interface IUserService {
        UserData GetUser(int id);
        void UpdateUser(UserData user);
        bool DeleteUser(int id);
        
        event OnUserChanged(int userId);
    }
}
```

### Generating Code

#### From C++ Code

```cpp
#include <KAI/Language/Tau/Generate/GenerateProxy.h>
#include <KAI/Language/Tau/Generate/GenerateAgent.h>
#include <KAI/Language/Tau/Generate/GenerateStruct.h>

// Generate proxy code
string proxyCode;
TAU_NS(Generate::GenerateProxy) proxyGen(idlSource, proxyCode);

// Generate agent code  
string agentCode;
TAU_NS(Generate::GenerateAgent) agentGen(idlSource, agentCode);

// Generate struct definitions
string structCode;
TAU_NS(Generate::GenerateStruct) structGen(idlSource, structCode);
```

#### Generated Proxy Example

```cpp
namespace MyApp {
    class IUserServiceProxy : public ProxyBase {
        using ProxyBase::StreamType;
        IUserServiceProxy(Node &node, NetHandle handle) : ProxyBase(node, handle) { }
        
        UserData GetUser(const int& id) {
            RakNet::BitStream args;
            args << id;
            auto future = _node->SendWithResponse("GetUser", args);
            return future.get();
        }
        
        void UpdateUser(const UserData& user) {
            RakNet::BitStream args;
            args << user;
            _node->Send("UpdateUser", args);
        }
        
        void RegisterOnUserChangedHandler(std::function<void(int)> handler) {
            RegisterEventHandler("OnUserChanged", handler);
        }
    };
}
```

#### Generated Agent Example

```cpp
namespace MyApp {
    class IUserServiceAgent : public AgentBase<IUserService> {
        IUserServiceAgent(Node &node, NetHandle handle) : AgentBase(node, handle) { }
        
        void Handle_GetUser(RakNet::BitStream& bs, RakNet::SystemAddress& sender) {
            int id;
            bs >> id;
            
            UserData result = _impl->GetUser(id);
            
            RakNet::BitStream response;
            response << result;
            _node->SendResponse(sender, response);
        }
        
        void Handle_UpdateUser(RakNet::BitStream& bs, RakNet::SystemAddress& sender) {
            UserData user;
            bs >> user;
            
            _impl->UpdateUser(user);
        }
    };
}
```

## Architecture Benefits

### Separation of Concerns
- **Proxy**: Handles client-side networking, serialization, and futures
- **Agent**: Handles server-side deserialization and method dispatch  
- **Struct**: Defines pure data structures without behavior

### Type Safety
- Generated code maintains full C++ type safety
- Compile-time errors for interface mismatches
- No runtime type casting required

### Network Transparency
- Clients use proxies exactly like local objects
- All networking complexity is hidden
- Automatic serialization/deserialization

## Integration with KAI Network System

The generated code integrates seamlessly with KAI's network infrastructure:

1. **Proxies** use the Node's `Send()` and `SendWithResponse()` methods
2. **Agents** register handlers with the Node's message dispatcher
3. **Structs** are serializable via RakNet BitStreams
4. **Events** use the Node's event system for multicast notifications

## Best Practices

### IDL Design
- Keep interfaces focused and cohesive
- Use structs for complex parameter types
- Prefer simple types for better interoperability
- Document your interfaces thoroughly

### Code Organization
- Generate code into separate files for each component
- Use consistent naming: `IFoo` → `FooProxy`, `FooAgent`
- Keep generated code separate from hand-written code
- Regenerate on IDL changes, don't edit generated code

### Error Handling
- Generated proxies throw on network errors
- Agents should validate inputs before processing
- Use events for asynchronous error notifications
- Implement timeouts for long-running operations

## Testing Generated Code

The KAI test suite includes comprehensive tests for code generation:

- `TauGenerateStructTests.cpp` - Tests struct generation
- `TauSeparateGenerationTests.cpp` - Tests separation of proxy/agent generation
- `TauCodeGenerationTests.cpp` - Tests various generation scenarios

Run tests with:
```bash
./build/Bin/Test/TestTau --gtest_filter="*Generate*"
```

## Future Enhancements

Planned improvements to the code generation system:

1. **Versioning Support** - Handle interface evolution
2. **Streaming Interfaces** - Support for large data transfers
3. **Custom Serializers** - Plugin architecture for type serialization
4. **Code Optimization** - Reduce generated code size
5. **Better Error Messages** - More helpful generation diagnostics

## See Also

- [Tau Tutorial](TauTutorial.md) - General Tau language tutorial
- [Network Architecture](NetworkArchitecture.md) - KAI networking overview
- [Testing Guide](Test.md) - How to test generated code