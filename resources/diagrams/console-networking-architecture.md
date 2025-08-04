# KAI Console Networking Architecture

## Console-to-Console Communication Model

```mermaid
graph TB
    subgraph "Console A (Port 14600)"
        CA_UI["Console Interface<br/>Pi λ /network start 14600"]
        CA_NET["Network Manager<br/>ID: Console-1234"]
        CA_PEER[Peer Handler<br/>RakNet Interface]
        CA_MSG[Message Queue<br/>Command/Result/Broadcast]
    end
    
    subgraph "Network Transport Layer"
        TRANSPORT[RakNet P2P Transport<br/>UDP with reliability<br/>Packet priority system]
    end
    
    subgraph "Console B (Port 14601)"
        CB_UI["Console Interface<br/>Rho λ /connect localhost 14600"]
        CB_NET["Network Manager<br/>ID: Console-5678"]
        CB_PEER[Peer Handler<br/>RakNet Interface]
        CB_MSG[Message Queue<br/>Command/Result/Broadcast]
    end
    
    CA_UI <--> CA_NET
    CA_NET <--> CA_PEER
    CA_PEER <--> CA_MSG
    CA_MSG <--> TRANSPORT
    
    TRANSPORT <--> CB_MSG
    CB_MSG <--> CB_PEER
    CB_PEER <--> CB_NET
    CB_NET <--> CB_UI
    
    style CA_UI fill:#e3f2fd
    style CB_UI fill:#e8f5e8
    style TRANSPORT fill:#fff3e0
```

## Network Message Protocol

```mermaid
sequenceDiagram
    participant CA as Console A
    participant NET as Network Layer
    participant CB as Console B
    
    Note over CA,CB: Connection Establishment
    CA->>NET: /network start 14600
    CB->>NET: /network start 14601
    CB->>NET: /connect localhost 14600
    NET->>CA: Connection request from CB
    CA->>NET: Accept connection
    NET->>CB: Connection established
    
    Note over CA,CB: Command Execution
    CA->>NET: /@1 2 3 + (send to peer 1)
    NET->>CB: CONSOLE_COMMAND message
    CB->>CB: Execute: 2 3 +
    CB->>NET: CONSOLE_RESULT message
    NET->>CA: Result: [0]: 5
    
    Note over CA,CB: Broadcasting
    CA->>NET: /broadcast stack
    NET->>CB: CONSOLE_BROADCAST message
    CB->>CB: Execute: stack
    CB->>NET: CONSOLE_RESULT message
    NET->>CA: Broadcast result from CB
    
    Note over CA,CB: Language Switching
    CA->>NET: /@1 rho (switch peer to Rho)
    NET->>CB: CONSOLE_LANGUAGE_SWITCH message
    CB->>CB: Switch language to Rho
    CB->>NET: CONSOLE_RESULT message
    NET->>CA: Language switched confirmation
```

## Network Command Structure

```mermaid
graph TD
    subgraph "Network Commands"
        START["/network start [port]<br/>Enable networking"]
        STOP["/network stop<br/>Disable networking"]
        STATUS["/network status<br/>Show network info"]
        CONNECT["/connect host port<br/>Connect to peer"]
        PEERS["/peers<br/>List connected peers"]
        DIRECT["/@peer command<br/>Execute on specific peer"]
        BROADCAST["/broadcast command<br/>Execute on all peers"]
        HISTORY["/nethistory<br/>Show message history"]
    end
    
    subgraph "Message Types"
        CMD[CONSOLE_COMMAND<br/>Remote execution request]
        RESULT[CONSOLE_RESULT<br/>Execution response]
        BCAST[CONSOLE_BROADCAST<br/>Multi-peer command]
        LANG[CONSOLE_LANGUAGE_SWITCH<br/>Language change]
    end
    
    START --> CMD
    DIRECT --> CMD
    BROADCAST --> BCAST
    CMD --> RESULT
    BCAST --> RESULT
    DIRECT --> LANG
    
    style START fill:#4caf50
    style STOP fill:#f44336
    style STATUS fill:#2196f3
    style CONNECT fill:#ff9800
    style DIRECT fill:#9c27b0
    style BROADCAST fill:#e91e63
```

## Network Architecture Components

```mermaid
classDiagram
    class Console {
        +StartNetworking(port)
        +StopNetworking()
        +ConnectToPeer(host, port)
        +SendCommandToPeer(peer, command)
        +BroadcastCommand(command)
        +ProcessNetworkCommand(command)
        -peer_: RakPeerInterface*
        -connectedPeers_: vector~SystemAddress~
        -messageThread_: thread
        -networkingEnabled_: bool
    }
    
    class NetworkConsoleMessage {
        +messageType: NetworkMessageType
        +sourceId: string
        +targetId: string
        +command: string
        +result: string
        +language: Language
        +timestamp: TimeMS
    }
    
    class RakPeerInterface {
        +Startup(maxConnections, descriptors, count)
        +Connect(host, port, password, length)
        +Send(bitStream, priority, reliability, channel, address, broadcast)
        +Receive(): Packet*
        +GetSystemAddresses(): vector~SystemAddress~
    }
    
    class NetworkMessageType {
        <<enumeration>>
        CONSOLE_COMMAND
        CONSOLE_RESULT
        CONSOLE_BROADCAST
        CONSOLE_LANGUAGE_SWITCH
    }
    
    Console --> NetworkConsoleMessage : creates
    Console --> RakPeerInterface : uses
    Console --> NetworkMessageType : handles
    NetworkConsoleMessage --> NetworkMessageType : contains
```

## Peer Discovery and Connection Flow

```mermaid
stateDiagram-v2
    [*] --> Disconnected
    
    Disconnected --> Starting : /network start
    Starting --> Listening : RakNet startup success
    Starting --> Error : RakNet startup failed
    
    Listening --> Connecting : /connect command
    Listening --> Accepting : Incoming connection
    
    Connecting --> Connected : Connection success
    Connecting --> Error : Connection failed
    
    Accepting --> Connected : Accept connection
    Accepting --> Listening : Reject connection
    
    Connected --> Communicating : Send/receive messages
    Communicating --> Connected : Message processed
    
    Connected --> Disconnecting : /network stop
    Connected --> Disconnecting : Peer disconnect
    Error --> Disconnected : Reset network
    
    Disconnecting --> Disconnected : Cleanup complete
```

## Message Processing Pipeline

```mermaid
graph LR
    subgraph "Outgoing Messages"
        USER_INPUT[User Command<br/>/@1 2 3 +]
        PARSE_CMD[Parse Network Command<br/>Extract peer & command]
        CREATE_MSG[Create Message<br/>CONSOLE_COMMAND type]
        SERIALIZE[Serialize to BitStream<br/>Add message headers]
        SEND[RakNet Send<br/>HIGH_PRIORITY, RELIABLE_ORDERED]
    end
    
    subgraph "Incoming Messages" 
        RECEIVE[RakNet Receive<br/>Get packet from network]
        DESERIALIZE[Deserialize BitStream<br/>Extract message data]
        HANDLE_MSG[Handle Message Type<br/>COMMAND/RESULT/BROADCAST]
        EXECUTE[Execute Command<br/>On local executor]
        SEND_RESULT[Send Result<br/>Back to originator]
    end
    
    USER_INPUT --> PARSE_CMD --> CREATE_MSG --> SERIALIZE --> SEND
    RECEIVE --> DESERIALIZE --> HANDLE_MSG --> EXECUTE --> SEND_RESULT
    
    style USER_INPUT fill:#e3f2fd
    style EXECUTE fill:#e8f5e8
    style SEND fill:#fff3e0
    style SEND_RESULT fill:#fff3e0
```

## Thread Safety and Concurrency

```mermaid
graph TB
    subgraph "Main Thread"
        CONSOLE[Console REPL<br/>User interaction]
        COMMAND[Command Processing<br/>Network commands]
    end
    
    subgraph "Network Thread"
        MSG_LOOP[Message Processing Loop<br/>Continuous packet handling]
        PACKET_HANDLE[Packet Handler<br/>Message type dispatch]
    end
    
    subgraph "Shared Resources"
        PEERS[Connected Peers List<br/>Protected by peersMutex_]
        HISTORY[Message History<br/>Thread-safe access]
        EXECUTOR[Shared Executor<br/>Command execution]
    end
    
    CONSOLE --> COMMAND
    COMMAND --> PEERS
    COMMAND --> EXECUTOR
    
    MSG_LOOP --> PACKET_HANDLE
    PACKET_HANDLE --> PEERS
    PACKET_HANDLE --> HISTORY
    PACKET_HANDLE --> EXECUTOR
    
    style CONSOLE fill:#e3f2fd
    style MSG_LOOP fill:#fff3e0
    style PEERS fill:#ffeb3b
    style EXECUTOR fill:#4caf50
```