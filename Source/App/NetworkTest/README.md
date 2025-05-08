# KAI RakNet Network Test

This test application demonstrates the RakNet integration with KAI. It shows how the network system facilitates distributed objects over a network.

## Building

Before building this app, you need to:

1. Make sure RakNet is properly cloned as a submodule:
   ```
   cd Ext
   git submodule update --init --recursive
   ```

2. In the main CMakeLists.txt, ensure `KAI_BUILD_RAKNET` is set to ON.

3. Build the project:
   ```
   mkdir -p build && cd build
   cmake .. -DKAI_BUILD_RAKNET=ON
   ninja
   ```

## Usage

Run as server:
```
./Bin/NetworkTest server
```

Run as client:
```
./Bin/NetworkTest client
```

The server will listen on port 14589 and broadcast its presence for peer discovery.
The client will search for servers on the network and connect automatically when found.

## Features Demonstrated

- Peer discovery: Automatically find other KAI nodes on the network
- Connection management: Track connected peers with timeout detection
- Object serialization: Transmit KAI objects over the network
- Distributed computation: Execute functions on remote nodes