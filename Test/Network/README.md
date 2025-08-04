# KAI Network Tests

This directory contains comprehensive tests for KAI's networking capabilities, including peer-to-peer communication, distributed computing, and console networking.

## Test Categories

### Core Network Tests

- **TestNode.cpp** - Tests for basic network node functionality
- **TestGenerateProxy.cpp** - Proxy generation and network object creation
- **ConsoleConnectionTest.cpp** - Console networking integration tests

### Chat System Tests

- **ChatDemo.rho** - Rho-based chat demonstration script
- **ChatFunctionalityTests.cpp** - Chat system feature validation
- **ChatAdvancedTests.cpp** - Advanced chat scenarios and edge cases
- **ChatProxyGenerationTest.cpp** - Chat proxy object generation
- **ICQStyleChatTest.cpp** - ICQ-style instant messaging tests

### Network Calculation Tests

- **CalculationTest.cpp** - Distributed calculation validation
- **IntegratedConsoleTest.cpp** - Console integration with network calculations

### Console Networking

- **TestConsoleNetworking.cpp** - Comprehensive console-to-console communication tests

## Key Features Tested

### Peer-to-Peer Communication
- Node discovery and connection establishment
- Bi-directional message passing
- Connection management and error handling

### Console Networking
- Multi-console real-time communication
- Remote command execution
- Cross-language command compatibility (Pi ↔ Rho)
- Broadcasting to multiple peers
- Network message history

### Distributed Computing
- Remote object creation and manipulation
- Proxy generation for network objects
- Load balancing across network nodes
- Failure handling and recovery

## Running Network Tests

### All Network Tests
```bash
make test
ctest -R Network
```

### Console Networking Tests
```bash
ctest -R ConsoleNetworking -V
```

### Specific Test Categories
```bash
ctest -R Chat          # Chat-related tests
ctest -R Calculation   # Network calculation tests
ctest -R Proxy         # Proxy generation tests
```

## Interactive Demos

### Console Communication Demo
Run the interactive tmux-based demo:
```bash
cd ../..
./demo_console_communication.sh
```

### Chat Demo
```bash
# Terminal 1
./console
pi> load "Test/Network/ChatDemo.rho"

# Terminal 2  
./console
pi> load "Test/Network/ChatDemo.rho"
```

## Test Configuration

Network tests use configurable ports and addresses:
- Default test port range: 14700-14799
- Localhost connections for safety
- Automatic port selection to avoid conflicts

## Related Documentation

- [Console Networking Guide](../../CONSOLE_NETWORKING.md)
- [Network Architecture](../../Doc/NetworkArchitecture.md)
- [Connection Testing](../../Doc/ConnectionTesting.md)
- [Peer-to-Peer Documentation](../../Doc/PeerToPeerNetworking.md)