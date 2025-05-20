# Network Tau Interfaces

This document describes the Tau language enhancements for defining network interfaces in KAI.

## Overview

Tau has been enhanced to better support network interface definitions, especially for peer-to-peer communication. These enhancements include:

1. **Interface Keyword**: First-class support for declaring network interfaces
2. **Event Support**: Ability to define event-based communication channels
3. **Dot Notation for Enum Values**: Accessing enum values using `EnumType.Value` syntax
4. **Struct Support**: Explicitly defining data structures for network transmission
5. **Default Values and Initialization**: Support for field initialization and default parameters

These features, combined with KAI's existing Tau language capabilities, provide a comprehensive way to define network communications.

## Enhanced Tau Language Features

### Interfaces

Interfaces define the contract between distributed components. They can contain methods, events, and constants.

```tau
namespace KAI { namespace Network
{
    interface IConnectionManager
    {
        // Add and remove connections
        int AddConnection(SystemAddress address);
        void RemoveConnection(int id);
        void RemoveConnectionByAddress(SystemAddress address);
        
        // Connection state management
        ConnectionState GetConnectionState(int id);
        void SetConnectionState(int id, ConnectionState state);
        
        // Events
        event ConnectionStatusChanged(int id, ConnectionEvent event);
    }
}}
```

### Events

Events provide a way to define asynchronous notifications that can be raised by an object and handled by subscribers.

```tau
namespace KAI { namespace Network
{
    interface INode
    {
        // ... methods ...
        
        // Events
        event ConnectionEstablished(SystemAddress address);
        event ConnectionLost(SystemAddress address, string reason);
        event MessageReceived(SystemAddress source, Object data);
    }
}}
```

When you generate proxy code for an interface with events, the resulting class will include methods for registering event handlers:

```cpp
// Generated C++ code
void RegisterConnectionEstablishedHandler(std::function<void(SystemAddress)> handler);
void UnregisterConnectionEstablishedHandler();

void RegisterConnectionLostHandler(std::function<void(SystemAddress, std::string)> handler);
void UnregisterConnectionLostHandler();

void RegisterMessageReceivedHandler(std::function<void(SystemAddress, Object)> handler);
void UnregisterMessageReceivedHandler();
```

### Dot Notation for Enum Values

Enum values can be accessed using dot notation, which provides better readability and type safety:

```tau
namespace KAI { namespace Network
{
    enum ConnectionState 
    {
        Disconnected = 0,
        Connecting = 1,
        Connected = 2,
        Failed = 3
    }
    
    struct ConnectionInfo
    {
        SystemAddress address;
        ConnectionState state = ConnectionState.Disconnected;  // Dot notation
        int64 lastActivity = 0;
        int ping = 0;
    }
}}
```

### Struct Support

Structs allow defining data structures that can be passed across the network:

```tau
namespace KAI { namespace Network
{
    struct SystemAddress
    {
        string ip;
        int port;
    }
    
    struct ConnectionInfo
    {
        SystemAddress address;
        ConnectionState state = ConnectionState.Disconnected;
        int64 lastActivity = 0;
        int ping = 0;
    }
}}
```

### Default Values and Initialization

Fields and parameters can have default values:

```tau
namespace KAI { namespace Network
{
    interface IPeerDiscovery
    {
        void Start(int port = 14589);  // Default parameter
        void Stop();
        bool IsDiscovering();
        SystemAddress[] GetDiscoveredPeers();
        
        event PeerDiscovered(SystemAddress address);
    }
}}
```

## Network Interface Examples

KAI provides several example network interface definitions in the `/Test/Language/TestTau/Scripts/Connection/` directory:

### ConnectionBasic.tau

Defines basic connection interfaces and data structures:

```tau
namespace KAI { namespace Network
{
    // Enums for connection state and events
    enum ConnectionState { ... }
    enum ConnectionEvent { ... }
    
    // Basic data structures
    struct SystemAddress { ... }
    struct ConnectionInfo { ... }
    
    // Connection management interface
    interface IConnectionManager { ... }
}}
```

### NetworkNode.tau

Defines interfaces for peer-to-peer node management:

```tau
namespace KAI { namespace Network
{
    interface INode
    {
        // Constants
        const int DefaultPort = 14589;
        
        // Connection methods
        void Listen(int port);
        void Connect(SystemAddress address, int port);
        void Disconnect();
        
        // Network events
        event ConnectionEstablished(SystemAddress address);
        event ConnectionLost(SystemAddress address, string reason);
        event MessageReceived(SystemAddress source, Object data);
    }
    
    interface IPeerDiscovery { ... }
}}
```

### MessageHandling.tau

Defines message types and serialization:

```tau
namespace KAI { namespace Network { namespace Messaging
{
    enum MessageType { ... }
    
    struct MessageHeader { ... }
    
    interface IMessage { ... }
    
    struct ObjectMessage { ... }
    struct FunctionCallMessage { ... }
    struct EventNotificationMessage { ... }
    
    interface IMessageHandler { ... }
}}}
```

### NetworkServices.tau

Defines high-level network services:

```tau
namespace KAI { namespace Network { namespace Services
{
    interface IServiceRegistry { ... }
    interface IChatService { ... }
    interface IFileTransferService { ... }
}}}
```

## Code Generation

The Tau interfaces are used to generate code for network communication:

1. **Proxy Generation**: Creates client-side code that forwards method calls to the network
2. **Agent Generation**: Creates server-side code that receives calls and executes them locally

The improved Tau language features make this generation more robust and flexible.

### Example: Generating a Proxy from a Network Interface

```cpp
// First, load the Tau interface definition
string tauCode = ReadFile("ConnectionBasic.tau");

// Generate the proxy code
string proxyCode;
GenerateProxy generator(tauCode, proxyCode);

if (generator.Failed) {
    std::cerr << "Failed to generate proxy: " << generator.Error << std::endl;
    return;
}

// Write the proxy code to a file
WriteFile("ConnectionProxy.h", proxyCode);
```

### Using Generated Proxies

The generated proxies make remote network calls transparent to the caller:

```cpp
// Create a connection to a remote node
Node node;
node.Connect("192.168.1.100", 14589);

// Get a proxy to an IConnectionManager on the remote node
auto connManager = node.GetProxy<IConnectionManager>("connectionManager");

// Call methods on the remote object as if it were local
int connectionId = connManager->AddConnection(address);
ConnectionState state = connManager->GetConnectionState(connectionId);

// Register for events
connManager->RegisterConnectionStatusChangedHandler([](int id, ConnectionEvent event) {
    std::cout << "Connection " << id << " status changed: " << event << std::endl;
});
```

## Best Practices

When defining network interfaces with Tau:

1. **Interface Design**: Define clean, cohesive interfaces that focus on a single responsibility
2. **Event-Based Communication**: Use events for asynchronous notifications rather than polling
3. **Data Structures**: Define clear struct types for data passing to ensure type safety
4. **Error Handling**: Consider how errors will be communicated (exceptions, error codes, or events)
5. **Versioning**: Design interfaces with versioning in mind for future compatibility
6. **Documentation**: Include detailed comments within the Tau files
7. **Parameter Defaults**: Use default parameters to make interfaces more flexible
8. **Batch Operations**: Group related operations together to reduce network roundtrips

## Related Documents

- [TauTutorial.md](TauTutorial.md): General Tau language tutorial
- [Networking.md](Networking.md): Overview of KAI networking system
- [PeerToPeerNetworking.md](PeerToPeerNetworking.md): Detailed documentation of peer-to-peer system
- [ConnectionTesting.md](ConnectionTesting.md): Testing network connections and interfaces