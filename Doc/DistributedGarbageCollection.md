# Distributed Garbage Collection in KAI

## Overview

The KAI system is designed to support **distributed garbage collection** across multiple networked nodes, enabling transparent remote procedure calls with the syntax:

```rho
Future<T> result = proxy->method(a, b, c);
```

This document describes the distributed GC architecture, its goals, current status, and implementation strategy.

## Core Goals

1. **Network Transparency**: Remote objects should behave like local objects
2. **Automatic Memory Management**: Distributed GC should collect unreachable objects across nodes
3. **Low Latency**: Async Future<T> pattern for non-blocking remote calls
4. **Fault Tolerance**: Handle node failures gracefully
5. **Scalability**: Support tens of thousands of connected nodes

## Architecture Components

### Local Garbage Collection

**Registry-Based GC** (Include/KAI/Core/Registry.h)
- Tri-color mark-and-sweep algorithm
- Color sets: White (condemned), Grey (uncertain), Black (reachable)
- Handle-based references (not raw pointers)
- Local object lifecycle management

**Key Operations:**
```cpp
void MarkAndSweep(Object root);  // Local GC pass
void TriColor();                  // Tri-color marking
void ReleaseWhite();              // Collect white objects
```

### Distributed Object Model

**Network Handles** (Include/KAI/Network/NetHandle.h)
- Unique identifiers for remote objects
- Format: `{nodeId, localHandle}`
- Cross-node reference tracking

**Future<T> Pattern** (Include/KAI/Network/Future.h)
```cpp
template <class T>
struct Future {
    int Id;                          // Unique operation ID
    ResponseType Response;           // Success/Failure/Timeout
    bool Complete;                   // Completion flag
    std::optional<T> Value;          // Result value
    std::string ErrorMessage;        // Error details
};
```

**Proxy/Agent Model:**
- **Proxy**: Client-side stub that forwards calls to remote nodes
- **Agent**: Server-side handler that executes calls locally
- Generated from Tau IDL definitions

### Distributed GC Strategies

#### 1. Reference Counting with Leases

**Lease-Based Management:**
- Each remote reference has a lease with expiration time
- Client must periodically renew leases to keep objects alive
- Objects with expired leases become eligible for collection

**Implementation:**
```tau
interface ILeaseManager {
    Future<int> AcquireLease(object remoteObject, int duration);
    Future<void> RenewLease(int leaseId, int additionalDuration);
    Future<void> ReleaseLease(int leaseId);
}
```

**Advantages:**
- Fault tolerance: Node crashes automatically expire leases
- Simple protocol: No need for complex distributed consensus
- Predictable behavior: Clear lifecycle boundaries

**Disadvantages:**
- Network overhead: Periodic lease renewals
- Latency-sensitive: Must renew before expiration

#### 2. Distributed Tracing

**Cross-Node Reachability:**
- Extend local tri-color marking across network boundaries
- Track inter-node references explicitly
- Coordinate GC cycles across nodes

**Reference Tracking Interface:**
```tau
interface IReferenceTracker {
    Future<void> TrackReference(int sourceNode, int targetNode, int objectId);
    Future<void> UntrackReference(int sourceNode, int targetNode, int objectId);
    Future<int[]> GetReferencingNodes(int objectId);
}
```

**Distributed Mark Phase:**
1. Start mark phase on root node
2. For each remote reference, send mark message to target node
3. Target node marks object and continues tracing
4. Aggregate reachability information
5. Sweep unreachable objects on all nodes

**Advantages:**
- Complete garbage detection (no leaks)
- No false collection (safe)
- Unified with local GC algorithm

**Disadvantages:**
- Complex coordination
- Requires distributed consensus
- Vulnerable to node failures during GC

#### 3. Hybrid Approach (Recommended)

**Combine Leases + Distributed Tracing:**
- Use leases for fault tolerance and initial approximation
- Use distributed tracing for complete collection
- Perform distributed trace periodically or on memory pressure

**Protocol:**
1. **Normal Operation**: Lease-based management
   - Proxies acquire leases on remote objects
   - Automatic renewal or explicit release
   - Fast, low-overhead

2. **Full Collection**: Triggered by memory pressure
   - Coordinate distributed mark-and-sweep
   - Collect circular garbage across nodes
   - Update lease states based on reachability

3. **Node Failure**: Lease expiration handles crashes
   - Objects with expired leases marked for collection
   - Remaining nodes continue operation
   - No distributed consensus required for failures

## Current Implementation Status

### Implemented ✅

1. **Local GC**: Tri-color mark-and-sweep in Registry (100% tests passing)
2. **Network Layer**: Node, Proxy, Agent infrastructure (Include/KAI/Network/)
3. **Future<T>**: Basic Future template with success/failure tracking
4. **Tau IDL**: Interface definition language for proxy/agent generation
5. **Code Generation**: Automatic proxy/agent generation from Tau definitions

### In Progress 🚧

1. **Distributed Reference Tracking**: Cross-node reference management
2. **Lease Management**: Automatic lease acquisition and renewal
3. **Proxy Lifecycle**: Proper cleanup when proxies go out of scope
4. **Cycle Detection**: Cross-node circular garbage detection

### Not Yet Implemented ❌

1. **Coordinated GC**: Distributed mark-and-sweep coordination
2. **Object Migration**: Move objects between nodes for load balancing
3. **Weak References**: Network-transparent weak references
4. **Finalization**: Distributed finalization queue

## Implementation Roadmap

### Phase 1: Basic Distributed References (Current)

**Goal**: Enable `Future<T> result = proxy->method(a,b,c)` syntax

**Tasks:**
- [x] Define Future<T> template
- [x] Implement Proxy/Agent base classes
- [x] Generate proxy code from Tau IDL
- [ ] Implement basic remote method invocation
- [ ] Handle serialization/deserialization of arguments and results

**Tests:** TauFutureProxyTests.cpp (50 tests)

### Phase 2: Lease-Based Object Lifetime

**Goal**: Remote objects don't leak when proxies are destroyed

**Tasks:**
- [ ] Implement ILeaseManager interface
- [ ] Automatic lease acquisition in proxy constructors
- [ ] Automatic lease release in proxy destructors
- [ ] Lease renewal background task
- [ ] Handle lease expiration on agent side

**Estimated Tests:** 30 tests for lease management

### Phase 3: Reference Tracking

**Goal**: Detect when remote objects are truly unreachable

**Tasks:**
- [ ] Implement IReferenceTracker interface
- [ ] Track outgoing references (local → remote)
- [ ] Track incoming references (remote → local)
- [ ] Store reference graph persistently
- [ ] Query reachability across nodes

**Estimated Tests:** 40 tests for reference tracking

### Phase 4: Distributed Mark-and-Sweep

**Goal**: Collect circular garbage across nodes

**Tasks:**
- [ ] Implement ICycleDetector interface
- [ ] Coordinate mark phase across nodes
- [ ] Aggregate reachability information
- [ ] Perform distributed sweep
- [ ] Handle node failures during GC

**Estimated Tests:** 50 tests for distributed GC

### Phase 5: Advanced Features

**Goal**: Production-ready distributed GC

**Tasks:**
- [ ] Object migration for load balancing
- [ ] Weak references across network
- [ ] Distributed finalization
- [ ] Performance optimization
- [ ] Monitoring and diagnostics

**Estimated Tests:** 60+ tests for advanced features

## Usage Examples

### Basic Remote Call

```rho
// Connect to remote node
node = createNetworkNode()
node.connect("192.168.1.10", 14589)

// Create proxy to remote calculator
proxy = node.CreateProxy<ICalculator>()

// Call remote method (non-blocking)
future = proxy->Add(5, 3)

// Wait for result
if future.Succeeded()
    print(future.GetValue())  // 8
else
    print("Error: " + future.ErrorMessage)
```

### Tau Interface Definition

```tau
namespace Math {
    interface ICalculator {
        Future<int> Add(int a, int b);
        Future<int> Multiply(int a, int b);
        Future<float> Divide(float a, float b);
    }
}
```

### Generated Proxy (Conceptual)

```cpp
class CalculatorProxy : public ICalculator {
    NetHandle remoteHandle_;
    Node* node_;
    LeaseManager leases_;

public:
    CalculatorProxy(Node* node, NetHandle handle)
        : node_(node), remoteHandle_(handle) {
        // Acquire lease on remote object
        leases_.AcquireLease(handle, 60000);  // 60 seconds
    }

    ~CalculatorProxy() {
        // Release lease
        leases_.ReleaseLease(remoteHandle_);
    }

    Future<int> Add(int a, int b) override {
        // Serialize call
        BinaryStream stream;
        stream << "Add" << a << b;

        // Send to remote node
        return node_->SendRequest<int>(remoteHandle_, stream);
    }
};
```

### Generated Agent (Conceptual)

```cpp
class CalculatorAgent : public ICalculator {
    std::unique_ptr<Calculator> impl_;

public:
    CalculatorAgent() : impl_(std::make_unique<Calculator>()) {}

    Future<int> Add(int a, int b) override {
        // Execute locally
        int result = impl_->Add(a, b);

        // Return completed future
        Future<int> future;
        future.Value = result;
        future.Complete = true;
        future.Response = ResponseType::Returned;
        return future;
    }
};
```

## Testing Strategy

### Unit Tests
- Local GC correctness (Registry tests): ✅ 147/147 passing
- Proxy/Agent generation (Tau tests): ✅ 100/109 passing
- Future<T> patterns: 🚧 TauFutureProxyTests.cpp (50 tests)

### Integration Tests
- Cross-node references
- Lease management
- Cycle detection
- Node failure scenarios

### Performance Tests
- Throughput: RPC calls/second
- Latency: Round-trip time for remote calls
- Scalability: Performance with N nodes (10, 100, 1000, 10000)
- Memory: Leak detection under sustained load

### Stress Tests
- Long-running distributed computations
- Random node failures and recoveries
- Heavy GC pressure
- Network partitions

## Debugging and Monitoring

### Trace Levels
```cpp
// Enable distributed GC tracing
node.SetGCTraceLevel(3);  // Verbose GC logging

// Trace remote calls
node.SetRPCTraceLevel(2);  // Log all RPC calls
```

### Diagnostics Interface
```tau
interface IGCDiagnostics {
    Future<MemoryStats> GetMemoryStats(int nodeId);
    Future<int> GetObjectCount(int nodeId);
    Future<int[]> GetLiveLeases();
    Future<ReferenceGraph> GetReferenceGraph();
}
```

### Metrics to Monitor
- Lease acquisition/release rate
- Lease renewal success rate
- GC pause time (local and distributed)
- Cross-node reference count
- Remote object count per node
- RPC latency percentiles (p50, p95, p99)

## Known Issues and Limitations

### Current Known Issues
1. **GC Cycles**: Registry.cpp:201 HACK to avoid local cycles
   - Affects: Local GC only
   - Workaround: Manual cycle breaking
   - Fix planned: Proper cycle detection algorithm

2. **RakNet Stub**: Network layer uses stub implementation
   - Affects: Real network testing
   - Workaround: In-memory testing only
   - Fix planned: Full RakNet integration or alternative

### Design Limitations
1. **Clock Synchronization**: Lease management assumes synchronized clocks
   - Impact: Incorrect expiration with clock skew
   - Mitigation: Use relative time offsets, NTP

2. **Network Partitions**: Current design doesn't handle splits well
   - Impact: Inconsistent state after partition heal
   - Mitigation: Partition-aware protocols, conflict resolution

3. **Memory Overhead**: Each proxy carries lease state
   - Impact: Higher memory usage with many proxies
   - Mitigation: Proxy pooling, batch lease management

## Related Documentation

- [Networking.md](Networking.md): Overview of network system
- [NetworkArchitecture.md](NetworkArchitecture.md): Detailed network architecture
- [PeerToPeerNetworking.md](PeerToPeerNetworking.md): P2P implementation
- [PROJECT_ANALYSIS.md](../PROJECT_ANALYSIS.md): Complete project analysis

## References

### Academic Papers
- "Distributed Garbage Collection" by Paul R. Wilson (1992)
- "Network Objects" by Birrell et al. (1993)
- "Java RMI and Distributed Garbage Collection" by Sun Microsystems

### Similar Systems
- Java RMI: Lease-based distributed GC
- .NET Remoting: Lease-based with sponsors
- Erlang: Per-process GC, message passing
- Orleans: Virtual actors with automatic lifecycle

## Conclusion

KAI's distributed garbage collection system aims to provide network-transparent object management with the simple syntax `Future<T> result = proxy->method(a,b,c)`. The hybrid lease + tracing approach balances fault tolerance, correctness, and performance.

Current focus (Phase 1-2) is on establishing the basic infrastructure and lease-based lifetime management. Future phases will add complete distributed tracing and advanced features for production use.

**Status**: Phase 1 in progress, with 50 new tests validating the Future<T> proxy pattern and 50 new Rho iteration tests ensuring refactoring safety.
