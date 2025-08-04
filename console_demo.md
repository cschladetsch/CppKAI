# Console-to-Console Communication Demo

This demonstrates how two KAI consoles can communicate with each other over the network.

## Features Implemented

- **Network Commands**: Start/stop networking, connect to peers, send commands
- **Language Support**: Commands are executed in the sender's language context
- **Broadcast**: Send commands to all connected peers
- **Direct Messaging**: Send commands to specific peers
- **Message History**: Track all network communications
- **Real-time Updates**: See incoming commands and results in real-time

## Available Network Commands

- `/network start [port]` - Start networking (default port 14600)
- `/network stop` - Stop networking
- `/network status` - Show network status and connected peers
- `/connect <host> <port>` - Connect to a peer console
- `/peers` - List all connected peers
- `/broadcast <command>` - Send command to all connected peers
- `/@<peer> <command>` - Send command to specific peer (by IP or index)
- `/nethistory` - Show network message history
- `help network` - Show network command help

## Demo Scenario

### Console 1 (Server)
```bash
# Start the console and enable networking
$ ./console

# In console:
/network start 14600
2 3 +
stack
```

### Console 2 (Client)
```bash
# Start second console instance
$ ./console

# Connect to first console
/network start 14601
/connect localhost 14600

# Send commands to peer
/@0 5 7 *
/broadcast stack
/@localhost:14600 "Hello from Console 2" print
```

### Console 3 (Another Client)
```bash
# Start third console
$ ./console

# Connect and participate
/network start 14602
/connect localhost 14600

# Switch to Rho language and send Rho commands
rho
/@0 x = 42; print(x);
/broadcast print("Broadcasting from Rho");
```

## Example Session Output

### Console 1:
```
KAI Console v1.0
Built on Dec 04 2024 at 10:30:00
Type 'help' for available commands.

pi> /network start 14600
Starting network console on port 14600
Network console listening on port 14600 (ID: Console-1234)
Network started

pi> 2 3 +
5

pi> <- Peer connected: 127.0.0.1:14601
<- [Console-5678] 5 7 *
<- [Console-5678] Result: 35

pi> << [BROADCAST from Console-9012] stack
   Result: 5

pi> <- [Console-9012] "Hello from Console 2" print
Hello from Console 2
<- [Console-9012] Result: "Hello from Console 2"
```

### Console 2:
```
pi> /connect localhost 14600
Connecting to peer at localhost:14600
<- Connected to peer: 127.0.0.1:14600

pi> /@0 5 7 *
-> [127.0.0.1:14600] 5 7 *
<- [Console-1234] Result: 35

pi> /broadcast stack
>> [BROADCAST] stack
<- [Console-1234] Result: 5

pi> /@localhost:14600 "Hello from Console 2" print
-> [127.0.0.1:14600] "Hello from Console 2" print
<- [Console-1234] Result: "Hello from Console 2"
```

## Key Features

1. **Multi-Language Support**: Each console can run in Pi or Rho mode, and commands are executed in the sender's language context
2. **Peer Discovery**: Automatic peer connection management with real-time status updates
3. **Command Routing**: Send commands to specific peers or broadcast to all
4. **Error Handling**: Network errors and command execution errors are properly reported
5. **History Tracking**: Complete log of network communications
6. **Color-Coded Output**: Different colors for incoming/outgoing messages, broadcasts, and results

## Architecture

- **RakNet Integration**: Uses the existing RakNet adapter for reliable P2P communication
- **Console Extension**: Networking is integrated directly into the base Console class
- **Thread Safety**: Separate network message processing thread with proper synchronization
- **Protocol**: Custom message types for commands, results, broadcasts, and language switches

The implementation allows multiple KAI consoles to form a distributed computing network where commands can be executed remotely and results shared across peers.