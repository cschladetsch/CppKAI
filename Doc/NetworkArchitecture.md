# KAI Network Architecture

This document provides a detailed overview of the architecture behind KAI's network functionality, explaining how the different components work together to create a robust peer-to-peer networking system.

## Core Architecture

KAI's network architecture is built around a true peer-to-peer model where every node has equal capabilities and can act as both a server and a client simultaneously. This approach enables a flexible and resilient network without central points of failure.

### Key Components

```
┌─────────────────────────────────────────────────┐
│                     Node                        │
├─────────────────────────────────────────────────┤
│                                                 │
│ ┌───────────────┐         ┌──────────────────┐  │
│ │PeerDiscovery  │         │ConnectionManager │  │
│ └───────────────┘         └──────────────────┘  │
│                                                 │
│ ┌───────────────┐         ┌──────────────────┐  │
│ │NetworkLogger  │         │ProxyAgentSystem  │  │
│ └───────────────┘         └──────────────────┘  │
│                                                 │
│               ┌──────────────┐                  │
│               │  RakNet      │                  │
│               └──────────────┘                  │
└─────────────────────────────────────────────────┘
```

#### Node

The Node class is the primary entry point to the networking system. It:
- Manages the lifecycle of network connections
- Listens for incoming connections
- Connects to remote nodes
- Processes incoming packets
- Handles object serialization and transmission
- Provides an API for peer discovery

#### RakNet Integration

KAI uses the RakNet library as its low-level networking implementation, which provides:
- Reliable UDP communication
- NAT traversal capabilities
- Automatic packet ordering and sequencing
- Ping and latency measurement
- Packet fragmentation and reassembly

#### Connection Management

The ConnectionManager maintains the state of all peer connections:
- Tracks active connections
- Monitors connection health with timeouts
- Dispatches connection events
- Manages connection IDs

#### Peer Discovery

The PeerDiscovery component provides automatic discovery of peers on the network:
- Broadcasts discovery messages
- Processes discovery responses
- Maintains a list of discovered peers
- Provides callbacks for discovery events

#### Proxies and Agents

The proxy-agent system enables remote procedure calls:
- Proxies: Client-side wrapper that forwards calls over the network
- Agents: Server-side handler that receives and executes the calls
- Both work together to provide transparent remote method invocation

## Network Protocols

KAI implements several custom protocols on top of RakNet:

### Connection Protocol

1. **Node Startup**: A node initializes RakNet and starts listening or connects to peers
2. **Connection Attempt**: Node sends a connection request to remote peer
3. **Connection Acceptance**: Remote peer accepts connection and both peers store the connection
4. **Connection Maintenance**: Periodic ping messages to detect connection loss
5. **Disconnection**: Graceful shutdown with proper cleanup

### Discovery Protocol

1. **Discovery Broadcast**: Node broadcasts a discovery ping on the local network
2. **Discovery Response**: Other nodes respond with their address information
3. **Peer Collection**: Discovery results are collected and made available to the application

### Object Transmission Protocol

1. **Object Serialization**: Objects are serialized into a binary format
2. **Packet Creation**: Serialized data is wrapped in a packet with appropriate headers
3. **Transmission**: Packet is sent to the target peer
4. **Reception**: Target peer receives and deserializes the object
5. **Object Recreation**: The object is recreated in the target peer's environment

## Command Execution Flow

The command execution flow in the peer-to-peer system works as follows:

```
┌─────────┐          ┌─────────┐
│ Peer A  │          │ Peer B  │
└─────────┘          └─────────┘
     │                    │
     │    Command @B      │
     │───────────────────>│
     │                    │
     │                    │ Process Command
     │                    │
     │     Result         │
     │<───────────────────│
     │                    │
```

1. **Command Input**: User enters a command with "@peer" prefix
2. **Command Routing**: The command is routed to the specified peer
3. **Command Translation**: Target peer translates aliases defined in configuration
4. **Command Execution**: Target peer executes the command in its environment
5. **Result Capture**: Execution result is captured and serialized
6. **Result Return**: Result is sent back to the originating peer
7. **Result Display**: Originating peer displays the result to the user

## Networking Layers

KAI's networking system is organized into several abstraction layers:

```
┌─────────────────────────────────────────────────┐
│              Application Layer                  │
│      (NetworkPeer, NetworkTest applications)    │
└────────────────────────┬────────────────────────┘
                         │
┌────────────────────────┴────────────────────────┐
│              Proxy/Agent Layer                  │
│    (Remote object references and methods)       │
└────────────────────────┬────────────────────────┘
                         │
┌────────────────────────┴────────────────────────┐
│              Object Layer                       │
│    (Object serialization and transmission)      │
└────────────────────────┬────────────────────────┘
                         │
┌────────────────────────┴────────────────────────┐
│              Node Layer                         │
│    (Connection management, peer discovery)      │
└────────────────────────┬────────────────────────┘
                         │
┌────────────────────────┴────────────────────────┐
│              RakNet Layer                       │
│    (Low-level networking, packet handling)      │
└─────────────────────────────────────────────────┘
```

### Application Layer
The application layer contains applications that use the networking system:
- NetworkPeer: Full peer-to-peer application with command execution
- NetworkTest: Basic peer discovery and connection testing
- Custom applications built on the KAI network framework

### Proxy/Agent Layer
This layer handles remote object references and method calls:
- Proxies represent remote objects in the local environment
- Agents handle incoming calls from remote proxies
- Both work together to provide transparent remote procedure calls

### Object Layer
The object layer handles:
- Serialization of KAI objects for network transmission
- Deserialization of received objects
- Object identity management across the network
- NetPointers for referencing objects across the network

### Node Layer
The node layer manages:
- Connections between peers
- Peer discovery
- Network event handling
- Message routing

### RakNet Layer
The lowest layer handles:
- Socket communication
- Packet fragmentation and reassembly
- Reliable delivery over UDP
- Connection establishment and maintenance

## Implementation Details

### RakNet Integration

KAI integrates with RakNet through several adapter classes:
- `RakNetStub.h`: Provides forward declarations to avoid direct inclusion of RakNet headers
- `RakNetAdapter`: Wraps RakNet functionality to keep the core KAI system independent of RakNet
- `RakNetImpl`: Implements the actual RakNet functionality

This design allows KAI to potentially switch to a different networking library without major changes to the core system.

### Serialization System

KAI uses a custom serialization system for network transmission:
- BinaryStream: Base class for serialization/deserialization
- BinaryPacket: Represents a serialized object with metadata
- Serialization methods for primitive types and complex objects
- Support for object references and cycles

### Network Logging

The NetworkLogger provides structured logging for network events:
- Connection events
- Message transmission
- Discovery events
- Errors and warnings

### Configuration System

Network configuration is managed through JSON files:
- Connection settings (ports, addresses)
- Peer discovery settings
- Command aliases
- Automatic connection behavior

## Network Security

The current implementation has limited security features, with plans for:
- Authentication: Verifying peer identity
- Authorization: Controlling what commands peers can execute
- Encryption: Protecting data during transmission
- Connection filtering: Restricting which peers can connect

## Future Architecture Enhancements

Planned improvements to the architecture include:
1. Complete security implementation with proper authentication and encryption
2. Enhanced NAT traversal capabilities
3. Improved peer discovery with DNS-based service discovery
4. Automatic mesh network formation and maintenance
5. Distributed object synchronization with conflict resolution
6. Better integration with the Rho language for network programming
7. Performance optimizations for high-throughput scenarios
8. Reliable reconnection handling with session persistence

## Related Documentation

- [Networking](Networking.md): Main networking documentation
- [PeerToPeerNetworking](PeerToPeerNetworking.md): Peer-to-peer system details
- [PeerToPeerSummary](PeerToPeerSummary.md): Concise summary of peer-to-peer functionality
- [ConnectionTesting](ConnectionTesting.md): Connection testing procedures
- [NetworkTest README](../Source/App/NetworkTest/Readme.md): Network test application details