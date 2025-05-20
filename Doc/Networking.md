# KAI Networking System

The KAI networking system provides peer-to-peer communication capabilities, allowing for fast and efficient distributed computation, object synchronization, and remote command execution across networked nodes.

## Network Documentation Structure

KAI's network documentation is organized into these main documents:

* **Networking.md** (this document): Overview of the networking system
* **[NetworkArchitecture.md](NetworkArchitecture.md)**: Detailed architecture of the networking system
* **[PeerToPeerSummary.md](PeerToPeerSummary.md)**: Concise summary of the peer-to-peer implementation
* **[PeerToPeerNetworking.md](PeerToPeerNetworking.md)**: Detailed documentation of the peer-to-peer system
* **[NetworkPerformance.md](NetworkPerformance.md)**: Performance considerations and optimization
* **[NetworkSecurity.md](NetworkSecurity.md)**: Security features and best practices
* **[ConnectionTesting.md](ConnectionTesting.md)**: Procedures for testing network connections
* **[NetworkIteration.md](NetworkIteration.md)**: Distributed iteration using across-node operations
* **[NetworkTauInterfaces.md](NetworkTauInterfaces.md)**: Tau language enhancements for network interfaces

## Core Concepts

* **Object → Registry (local)**: Objects belong to a local Registry
* **NetObject → Domain (shared)**: Network objects exist within a shared Domain
* **Domain → System (shared across multiple Domains)**: Domains form a distributed System

A **Registry** is a local set of unique well-known **Objects**. A **Domain** is a set of unique well-known objects within a set of network **Nodes**. 

A **System** is a set of well-known Domains - The top-level System network does not generally deal with specific NetObjects, although it can.

Rather, a network System is a collection of distributed Domains, each such unique Domain with a unique shared Registry.

In other words, a Registry can exist locally within a single Console application on a single machine. A collection of Registries is a Domain, and a collection of Domains is the overall System.

## Architecture Overview

KAI's networking architecture is designed around these key principles:

1. **Peer-to-Peer Communication**: Nodes communicate directly without requiring a central server.
2. **Distributed Computation**: Work can be shared across multiple nodes for parallel processing.
3. **Object Synchronization**: Objects can be synchronized across nodes with automatic state propagation.
4. **Code Mobility**: Functions and code can be transmitted between nodes and executed remotely.

## Distributed Computation with AcrossAllNodes

One of the most powerful features of KAI's networking capabilities is the `AcrossAllNodes` operation, which enables distributed parallel processing across connected nodes. This feature allows you to:

- Distribute computation across multiple machines
- Process large collections in parallel
- Balance workloads automatically based on node capacity
- Combine results seamlessly

### Example: Distributed Array Processing

```rho
// Create and connect nodes
node = createNetworkNode()
node.listen(14589)
node.connect("192.168.1.10", 14589)

// Define a computationally intensive function
fun process(x) {
    // Complex computation
    return x * x * x + Math.sin(x)
}

// Create a large dataset
data = array(10000)
for i = 0; i < 10000; i = i + 1
    data[i] = i / 10.0
end

// Process the data in parallel across all connected nodes
result = acrossAllNodes(node, data, process)

// The result contains the processed values, computed in parallel
// across all available network nodes
```

For more details on the AcrossAllNodes operation, see the [Network Iteration documentation](NetworkIteration.md).

## Background and Development

It has taken many years to build all this all up from the ground, starting with scripting languages and parsers, distributed garbage collection systems, and various fuzzy synchronisation models.

These are all coming along, slowly over the years. This specific project was started in 2008, and even more informally back in 2001.

The end goal is to have a server-less, truly peer-to-peer system that yet maintains state coherency and is scalable to the tens of thousands of connected nodes. That doesn't come cheaply and work continues.

One of the first things I recognised is that it's cheap and easy to send code rather than send state. So, I started with a very brief language I called Pi.

Pi is very fast for a computer to parse and execute, and has other benefits. However it's hard to read for a human so I created Rho which compiles to Pi.

Then I needed to generate Proxy code for Agents in the network, so why not leverage all the existing work I did on Pi and Rho for that too?

So then I refactored all the language systems to use one generic language system that can be used to generate a Lexer, Parser, AST Walker and Translator for any language you want. Overkill, but fun and practically useful. Fixing one bug fixes many languages.

But it's all for a simple and single purpose: to allow for natural and fluid networking without a client/server model. I make progress over time, and so it continues. Here's an overview of the networking capabilities:

## Network Features

### Remote Object Creation
Create a remote object, and set its value:

```rho
remote = new Peer("192.168.0.13", 6666)   # connect to remote machine
proxy = remote.NewProxy(Vector3)          # make a vector3 over there
proxy.Set(new Vector3(1,2,3))             # set its (remote) value
```

### Distributed Iteration
Process collections across multiple nodes:

```rho
// Create a network node and connect to peers
node = createNetworkNode()
node.listen(14589)
node.connect("192.168.1.10", 14589)

// Create data and define a function
data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
fun square(x) { x * x }

// Process the data using distributed execution
result = acrossAllNodes(node, data, square)
print(result)  // [1, 4, 9, 16, 25, 36, 49, 64, 81, 100]
```

### Remote Object Location
Find objects across the network by name or type:

```rho
// Find all Vector3 instances across all connected nodes
vectors = node.FindAllObjectsByType("Vector3")

// Find a specific named object
playerPosition = node.FindObjectByName("player1.position")
```

### Remote Procedure Calls
Call methods on remote objects:

```rho
// Get a reference to a remote object
gameServer = node.FindObjectByName("gameServer")

// Call a method on the remote object
result = gameServer.CalculateScore(player1, player2)
```

### Remote Object Properties
Access and modify properties of remote objects:

```rho
// Get a remote player object
remotePlayer = node.FindObjectByName("player1")

// Get and set properties
currentHealth = remotePlayer.health
remotePlayer.health = currentHealth + 20
```

### Event Subscription
Subscribe to events on remote objects:

```rho
// Get a remote door object
remoteDoor = node.FindObjectByName("mainEntrance")

// Subscribe to an event
remoteDoor.OnOpen.Subscribe(fun() {
    print("Door was opened!")
})

// Later, when the door opens on the remote node,
// the event will trigger the callback function here
```

### Remote Object Reactive Programming
Create reactive bindings between local and remote objects:

```rho
// Create a binding between a local and remote property
localCounter = 0
remoteCounter = node.FindObjectByName("sharedCounter")

// Create a two-way binding
Bind(localCounter, remoteCounter)

// Now when either counter changes, the other will be updated automatically
localCounter = 5
print(remoteCounter)  // 5

// Changes from the remote side will also update the local value
```

## Network Performance Considerations

When using KAI's networking features, consider these performance tips:

1. **Batch Operations**: Group multiple operations together when possible to reduce network overhead.

2. **Use AcrossAllNodes for Large Datasets**: For large collections, distributed processing offers significant performance advantages.

3. **Local Fallback**: When network nodes aren't available, operations like AcrossAllNodes automatically fall back to local execution.

4. **Connection Management**: Monitor connection status with `node.IsConnectedTo()` before performing distributed operations.

5. **Workload Distribution**: For optimal performance, distribute work evenly across nodes with similar capabilities.

For more details on network programming with KAI, see the examples in [Test/Network](/Test/Network/) and the full [API documentation](NetworkIteration.md).

## Peer-to-Peer Networking

KAI's networking architecture is primarily peer-to-peer, with no central server-client distinction. The system allows peers to connect to each other directly, execute commands on remote peers, and share computation.

### NetworkPeer Application

The `NetworkPeer` application provides a complete peer-to-peer implementation with SSH-like command semantics for remote command execution.

#### Running NetworkPeer

```bash
# Start a peer with default configuration
./Bin/NetworkPeer

# Start a peer with specific configuration
./Bin/NetworkPeer /path/to/config.json
```

#### Configuration Files

The NetworkPeer can be configured using JSON configuration files:

```json
{
    "listenPort": 14595,
    "autoListen": true,
    "interactive": true,
    "peers": [
        {
            "host": "127.0.0.1",
            "port": 14596
        }
    ],
    "commands": {
        "add": "calc 1+2",
        "hello": "echo Hello from peer"
    }
}
```

Sample configuration files are provided in the `config/` directory:
- `peer1_config.json`: Example configuration for a listening peer
- `peer2_config.json`: Example configuration for a connecting peer
- `minimal_config.json`: Minimal configuration for basic testing

#### Remote Command Execution

From the interactive console, you can execute commands on remote peers:

```
# List connected peers
peers

# Execute a calculation on remote peer 0
@0 add

# Execute a custom command on peer 1
@1 hello
```

This will execute the defined command on the remote peer and return the result to the local console.

For detailed information about the peer-to-peer system, see the [Peer-to-Peer Networking documentation](PeerToPeerNetworking.md).

### Testing Peer-to-Peer Functionality

KAI provides several scripts to test the peer-to-peer functionality:

#### Interactive Testing

```bash
./Scripts/network/run_peers.sh
```

This script provides instructions for running two peers in separate terminals and testing the peer-to-peer communication.

#### Automated Testing

```bash
./Scripts/network/automated_demo.sh
```

This script builds the NetworkPeer and verifies it's ready for use.

You can also run the legacy test script:

```bash
./Scripts/p2p_test.sh
```

This script:
1. Starts two peer nodes
2. Connects the second peer to the first
3. Issues a command from the second peer to execute on the first
4. Verifies the calculation result (1+2=3) is correctly returned

#### Peer-to-Peer Test Walkthrough

1. Start a peer in server mode:
   ```bash
   cd /path/to/KAI/build && ./Bin/NetworkPeer ../config/peer1_config.json
   ```

2. Start another peer in client mode:
   ```bash
   cd /path/to/KAI/build && ./Bin/NetworkPeer ../config/peer2_config.json
   ```

3. In the client terminal:
   - Type `peers` to list connected peers
   - Type `@0 add` to execute the "add" command on the server peer
   - You should see the result `3` returned from the calculation

## Network Components

KAI provides several components for implementing and testing network connections:

### Core Network Components

- **Node**: The primary network entity (in `/Include/KAI/Network/Node.h`)
- **ConnectionManager**: Handles connection states and timeouts (`/Include/KAI/Network/ConnectionManager.h`)
- **PeerDiscovery**: Provides automatic peer discovery on the network (`/Include/KAI/Network/PeerDiscovery.h`)
- **NetworkSerializer**: Handles object serialization (`/Include/KAI/Network/Serialization.h`)

### Network Applications

- **NetworkTest**: Demonstrates basic peer discovery and connection (`/Source/App/NetworkTest/`)
- **NetworkPeer**: Advanced peer-to-peer implementation with SSH-like command execution (`/Source/App/NetworkPeer/`)
- **MinimalServer/MinimalClient**: Simple examples showing basic connection and message passing

### RakNet Integration

The KAI network system is built on top of RakNet (available in `/Ext/RakNet/`), which provides:

- Reliable UDP communication
- Peer discovery through pings
- Automatic connection management
- Packet serialization and transmission

### Tau Network Interfaces

KAI uses the Tau Interface Definition Language (IDL) to define network interfaces in a language-neutral way. The Tau language has been enhanced to support a rich set of features needed for network communications, including:

- **Interfaces**: Define contracts between distributed components
- **Events**: Support for event-based communication with callbacks
- **Enums with dot notation**: Access enum values using `EnumType.Value` syntax
- **Structs**: Define complex data structures for network transmission
- **Default values**: Specify default values for method parameters and struct fields
- **Array types**: Define and use array data for collections

The connection-related interfaces are defined in:

- **ConnectionBasic.tau**: Basic connection interfaces and structures (enums, structs, connection management)
- **NetworkNode.tau**: Node interfaces for peer-to-peer connections (peer discovery, connection methods)
- **MessageHandling.tau**: Message passing interfaces (object serialization, RPC)
- **NetworkServices.tau**: Higher-level network services (service registry, chat interfaces)

These interfaces can be found in `/Test/Language/TestTau/Scripts/Connection/`.

### Code Generation

The Tau interfaces can be used to generate code for network communication:

- **Proxies**: Client-side code that forwards calls to the network
- **Agents**: Server-side code that receives calls and executes them locally

This allows for transparent remote procedure calls across the network. For event-based communication, the generated code includes methods for registering event handlers that get called when remote events are triggered.

Example of a network interface with events:

```tau
namespace KAI { namespace Network
{
    // Define an interface with events
    interface INode
    {
        // Network management methods
        void Connect(SystemAddress address, int port);
        void Disconnect();
        bool IsRunning();
        
        // Events for connection status changes
        event ConnectionEstablished(SystemAddress address);
        event ConnectionLost(SystemAddress address, string reason);
        event MessageReceived(SystemAddress source, Object data);
    }
}}
```

Generated proxy code will include both the method calls and event registration:

For details on connection testing and examples, see the [Connection Testing documentation](ConnectionTesting.md).

## Related Documentation

### Core Network Documentation
- [NetworkArchitecture.md](NetworkArchitecture.md): Detailed architecture of the networking components
- [PeerToPeerSummary.md](PeerToPeerSummary.md): Concise summary of the peer-to-peer implementation
- [PeerToPeerNetworking.md](PeerToPeerNetworking.md): Detailed documentation of the peer-to-peer system
- [NetworkPerformance.md](NetworkPerformance.md): Performance considerations and optimization
- [NetworkSecurity.md](NetworkSecurity.md): Security features and best practices
- [NetworkTauInterfaces.md](NetworkTauInterfaces.md): Tau language enhancements for network interfaces

### Component Documentation
- [ConnectionTesting.md](ConnectionTesting.md): Procedures for testing network connections
- [NetworkIteration.md](NetworkIteration.md): Distributed iteration using across-node operations
- [NetworkCalculationTest.md](NetworkCalculationTest.md): Examples of distributed calculations

### Implementation Documentation
- [NetworkTest README](../Source/App/NetworkTest/Readme.md): Network test application details
- [Scripts/network README](../Scripts/network/Readme.md): Network test scripts documentation

