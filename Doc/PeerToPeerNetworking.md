# Peer-to-Peer Networking in KAI

This document describes KAI's peer-to-peer networking capabilities, which allow direct communication between nodes with SSH-like command execution semantics.

## Overview

KAI's peer-to-peer networking system enables:

1. Direct communication between peers without a central server
2. Remote command execution using SSH-like syntax
3. JSON configuration for network setup
4. Distributed calculation and computation
5. Dynamic peer discovery and connection

## NetworkPeer Application

The `NetworkPeer` application implements a peer node in the network that can:
- Listen for incoming connections
- Connect to other peers
- Execute commands locally
- Send commands to remote peers
- Process results from remote execution

### Running a Peer

```bash
./Bin/NetworkPeer [config_file] [port]
```

Where:
- `config_file` is an optional JSON configuration file (defaults to `peer_config.json`)
- `port` is an optional listen port that overrides the configuration file

### Command Syntax

The peer console supports the following commands:

- `help` - Display available commands
- `connect <host> <port>` - Connect to another peer
- `peers` - List connected peers
- `exit` or `quit` - Exit the program
- `@<peer> <command>` - Execute a command on a remote peer
- Custom commands defined in the configuration file

Example remote command:
```
@0 calc 1+2
```

This executes the `calc 1+2` command on peer 0 and returns the result (3).

### Command Execution Flow

When a peer executes a remote command using the `@peer` syntax, here's what happens behind the scenes:

1. **Command Transmission**:
   - Peer2 sends a packet with ID_P2P_COMMAND to Peer1 containing the command string
   - Example: `@0 add` sends the command "add" to peer 0

2. **Command Processing at Remote Peer**:
   - Peer1 receives the command and looks it up in its command dictionary
   - If it's a predefined command (like "add" → "calc 1+2"), it substitutes the command
   - The command is passed to the appropriate command processor (e.g., performCalculation)
   - Example: "add" maps to "calc 1+2" which is processed by the calculation function

3. **Result Calculation**:
   - The remote peer calculates the result
   - Example: "calc 1+2" is parsed and computed to produce "3"

4. **Result Transmission**:
   - Peer1 sends back a packet with ID_P2P_RESULT containing the result string
   - Example: Peer1 sends "3" back to Peer2

5. **Result Display**:
   - The initiating peer receives and displays the result
   - Example: Peer2 displays "Result from 127.0.0.1:14595 for command 'add': 3"

This flow enables SSH-like remote command execution, where one peer can execute commands on another peer and receive the results, creating a powerful distributed computation network.

## Configuration

NetworkPeer is configured using JSON:

```json
{
    "listenPort": 14590,
    "autoListen": true,
    "interactive": true,
    "peers": [
        {
            "host": "127.0.0.1",
            "port": 14591
        }
    ],
    "commands": {
        "add": "calc 1+2",
        "status": "system status"
    }
}
```

Configuration options:
- `listenPort`: Port to listen for incoming connections
- `autoListen`: Whether to automatically start listening
- `interactive`: Run in interactive mode (command prompt)
- `peers`: List of peers to connect to at startup
- `commands`: Predefined command aliases

## Examples

### Basic Peer Setup

1. Start a peer listening on port 14590:
   ```bash
   ./Bin/NetworkPeer
   ```

2. Start another peer and connect to the first:
   ```bash
   ./Bin/NetworkPeer peer2_config.json
   ```

3. In the second peer's console, list connected peers:
   ```
   peers
   ```

4. Execute a calculation on the remote peer:
   ```
   @0 calc 1+2
   ```

5. The result (3) will be returned and displayed locally.

### Running the Test Demo

To demonstrate peer-to-peer calculation:

```bash
./Scripts/p2p_test.sh
```

This script:
1. Starts two peer nodes
2. Connects the second peer to the first
3. Issues a command from the second peer to execute on the first
4. Verifies the calculation result (1+2=3) is correctly returned

## Extending the System

The peer-to-peer system can be extended with custom command processors:

```cpp
peer.registerCommandProcessor("mycommand", [](const std::string& args) {
    // Process the command
    return "Result";
});
```

This allows for arbitrary distributed computation on the peer network.

## SSH-like Semantics

The `@peer command` syntax is inspired by SSH, allowing for:

1. Direct command execution on remote nodes
2. Result retrieval to the local console
3. Command chaining and piping (future enhancement)
4. Script execution across peers (future enhancement)

## Security Considerations

Currently, the peer-to-peer implementation does not include security features like authentication or encryption. In a production environment, you should add:

1. Peer authentication
2. TLS encryption for communications
3. Command authorization
4. Rate limiting for command execution

## Performance Considerations

For optimal performance:

1. Use non-interactive mode for high-throughput scenarios
2. Minimize the number of direct peer connections (mesh topology can be inefficient)
3. Batch commands when possible
4. Consider message size when transferring large results