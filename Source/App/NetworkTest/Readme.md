# KAI RakNet Network Test

This test application demonstrates the RakNet integration with KAI's networking system, showing how the framework facilitates peer discovery, connections, and distributed objects over a network.

## Overview

The NetworkTest application provides a simple client-server example to demonstrate core networking capabilities:

- The server listens on a port and broadcasts its presence
- The client discovers servers on the network and connects automatically
- Both constantly update and maintain their connections
- Connection status is displayed in real-time

This serves as a foundation for more complex peer-to-peer applications like the NetworkPeer application.

## Architecture

The NetworkTest application is built around these components:

- **Node**: Core networking entity that handles connections and discovery
- **PeerDiscovery**: Component that finds other peers on the network
- **ConnectionManager**: Manages connection states and timeouts
- **RakNet**: Underlying networking library providing reliable UDP communication

## Building

Before building this app, you need to:

1. Make sure RakNet is properly cloned as a submodule:
   ```bash
   cd Ext
   git submodule update --init --recursive
   ```

2. In the main CMakeLists.txt, ensure `KAI_BUILD_RAKNET` is set to ON.

3. Build the project:
   ```bash
   mkdir -p build && cd build
   cmake .. -DKAI_BUILD_RAKNET=ON
   cmake --build .
   ```

## Usage

Run as server:
```bash
./Bin/NetworkTest server
```

Run as client:
```bash
./Bin/NetworkTest client
```

The server will:
- Listen on port 14589
- Broadcast its presence for peer discovery
- Print connection status periodically
- Show a list of connected peers

The client will:
- Search for servers on the network via discovery
- Connect automatically when a server is found
- Fall back to connecting to localhost if no servers are found
- Display connection status

## Implementation Details

The main components are implemented in:

- `Main.cpp`: Contains the `RunServer()` and `RunClient()` functions
- `Node.h/cpp`: Provides the network node implementation
- `PeerDiscovery.h/cpp`: Handles finding peers on the network
- `ConnectionManager.h/cpp`: Manages connections and their states

## Features Demonstrated

- **Peer Discovery**: Automatically find other KAI nodes on the network using RakNet's ping system
- **Connection Management**: Track connected peers with timeout detection and automatic cleanup
- **Real-time Status Updates**: Display connection status and peer information
- **Error Handling**: Graceful handling of connection failures and recovery
- **Extensibility**: Foundation for building more complex distributed applications

## Testing

To test the network functionality, run the server and client in separate terminals. You should see:

1. The server prints "Server listening on port 14589"
2. The client discovers the server and connects to it
3. The server shows the client in its connected peers list
4. The client confirms it's connected to the server

## Related Components

- **NetworkPeer**: More advanced implementation with command execution
- **Tau Network Interfaces**: Interface definitions for network components
- **RakNet Integration**: Low-level networking implementation

For more comprehensive examples of peer-to-peer functionality, see the NetworkPeer application and the scripts in the `Scripts/network/` directory.