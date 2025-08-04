# KAI Console-to-Console Networking

This documents the console-to-console communication feature that allows multiple KAI console instances to communicate over a network.

## Overview

The networking functionality is integrated directly into the base `Console` class, allowing any KAI console to:
- Start a network server to accept connections
- Connect to other console instances
- Send commands to specific peers or broadcast to all
- Execute commands remotely with real-time results
- Maintain cross-language compatibility (Pi ↔ Rho)

## Quick Start

### Running the Interactive Demo

```bash
# Make the demo script executable
chmod +x demo_console_communication.sh

# Run the tmux-based demo
./demo_console_communication.sh
```

The demo will automatically:
1. Create a tmux session with two console panes
2. Set up networking on both consoles
3. Establish connection between them
4. Demonstrate various networking commands
5. Show cross-language communication
6. Provide interactive panes for manual testing

### Manual Setup

#### Console 1 (Server):
```bash
./console
```
```
pi> /network start 14600
pi> 2 3 +
pi> stack
```

#### Console 2 (Client):
```bash
./console  
```
```
pi> /network start 14601
pi> /connect localhost 14600
pi> /@0 10 *
pi> /broadcast stack
pi> /peers
```

## Network Commands

| Command | Description | Example |
|---------|-------------|---------|
| `/network start [port]` | Start networking server | `/network start 14600` |
| `/network stop` | Stop networking | `/network stop` |
| `/network status` | Show network status | `/network status` |
| `/connect <host> <port>` | Connect to peer | `/connect localhost 14600` |
| `/peers` | List connected peers | `/peers` |
| `/broadcast <command>` | Send to all peers | `/broadcast 2 3 +` |
| `/@<peer> <command>` | Send to specific peer | `/@0 stack` |
| `/nethistory` | Show message history | `/nethistory` |
| `help network` | Show network help | `help network` |

## Test Cases

### Automated Test Suite

Run the comprehensive test suite:

```bash
# Build and run all tests
make test

# Run specific networking tests
ctest -R ConsoleNetworking

# Run with verbose output
ctest -R ConsoleNetworking -V
```

### Test File Location

The main test file is located at:
```
Test/Console/TestConsoleNetworking.cpp
```

### Test Scenarios Covered

1. **BasicNetworkSetup** - Network initialization and peer connection
2. **SendCommandToPeer** - Direct command execution on remote console
3. **BroadcastCommand** - Broadcasting commands to all connected peers
4. **CrossLanguageCommunication** - Pi/Rho language interoperability
5. **NetworkErrorHandling** - Error conditions and edge cases
6. **MessageHistory** - Network message logging and retrieval
7. **NetworkStartStop** - Dynamic network enable/disable
8. **CompleteWorkflow** - Full integration test scenario

## Architecture

### Network Integration

The networking is integrated into the base `Console` class through:
- **RakNet Integration**: Uses existing RakNet adapter for reliable P2P communication
- **Thread Safety**: Separate network message processing thread
- **Message Protocol**: Custom message types for commands, results, broadcasts
- **Language Support**: Commands execute in sender's language context

### Message Types

```cpp
enum class NetworkMessageType : RakNet::MessageID {
    CONSOLE_COMMAND = RakNet::ID_USER_PACKET_ENUM + 10,
    CONSOLE_RESULT = RakNet::ID_USER_PACKET_ENUM + 11,
    CONSOLE_BROADCAST = RakNet::ID_USER_PACKET_ENUM + 12,
    CONSOLE_LANGUAGE_SWITCH = RakNet::ID_USER_PACKET_ENUM + 13
};
```

### Key Components

- **NetworkConsoleMessage**: Message structure for history tracking
- **ProcessNetworkCommand()**: Network command parser and dispatcher
- **HandleNetworkPacket()**: Incoming message handler
- **Peer Management**: Connection tracking and addressing

## Examples

### Basic Pi Operations

**Console 1:**
```
pi> /network start 14600
pi> 10 20 +
pi> stack
30
```

**Console 2:**
```  
pi> /network start 14601
pi> /connect localhost 14600
pi> /@0 5 *
<- [Console-1234] Result: 150
```

### Cross-Language Communication

**Console 1 (Pi mode):**
```
pi> /network start 14600
pi> 42
```

**Console 2 (Rho mode):**
```
pi> rho
rho> /network start 14601  
rho> /connect localhost 14600
rho> /@0 dup *
<- [Console-1234] Result: 1764
```

### Broadcasting

**Console 2:**
```
pi> /broadcast clear
>> [BROADCAST] clear
<- [Console-1234] Result: 
<- [Console-5678] Result:
```

## Implementation Files

### Headers
- `Include/KAI/Console/Console.h` - Main console class with networking

### Source  
- `Source/Library/Executor/Source/Console.cpp` - Network implementation

### Tests
- `Test/Console/TestConsoleNetworking.cpp` - Comprehensive test suite

### Demo
- `demo_console_communication.sh` - Interactive tmux demo
- `console_demo.md` - Usage documentation

## Usage Notes

1. **Port Management**: Each console needs a unique port for networking
2. **Peer Addressing**: Use IP:port or peer index (0, 1, 2...) to address peers
3. **Language Context**: Commands execute in the sender's language mode
4. **Error Handling**: Network errors are reported with colored output
5. **History Tracking**: All network messages are logged with timestamps
6. **Thread Safety**: Network operations are thread-safe with proper synchronization

## Future Enhancements

Potential improvements for the networking system:
- Peer discovery and auto-connection
- Secure authentication between peers
- File transfer capabilities
- Distributed variable/object sharing
- Load balancing for computational tasks
- Network topology visualization

---

For more information, see the comprehensive test cases and demo script included in this repository.