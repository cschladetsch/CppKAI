# Networking

The main reason for the development of this system was to allow for fast and efficient distributed computation.

* Object -> Registry (local)
* NetObject -> Domain (shared)
* Domain -> System (shared across multiple Domains)

A *Registry* is a local set of unique well-known *Objects*. A *Domain* is a set of unique well-known objects within a set of network *Nodes*. 

A *System* is a set of well-known Domains - The top-level System network does not generally deal with specific NetObjects, although it can.

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

For more details on the AcrossAllNodes operation, see the [Network Iteration documentation](Doc/NetworkIteration.md).

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

For more details on network programming with KAI, see the examples in [Test/Network](Test/Network/) and the full [API documentation](Doc/NetworkIteration.md).

