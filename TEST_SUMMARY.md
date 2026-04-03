# KAI Project Test Summary

Generated: 2026-04-03

## Overall Status

| Suite | Passed | Total | Rate | Notes |
|-------|--------|-------|------|-------|
| KaiTest | 173 | 173 | 100% | Core object model smoke tests |
| Core | 147 | 147 | 100% | Registry, GC, BinaryStream, Array, Map |
| Pi | 330 | 330 | 100% | All Pi language tests passing |
| Tau | 308 | 308 | 100% | All Tau parser/codegen tests passing |
| Console | 11 | 11 | 100% | Console + network peer tests |
| ProxyGeneration | 1 | 1 | 100% | Tau proxy code generation test |
| Rho | 717 | 859 | 83% | 142 pre-existing failures (see below) |

## Test Suite Details

### Core Tests (KaiTest + Core)
- Registry, type system, memory management, garbage collection
- BinaryStream serialization, Array, Map containers
- `Object::Valid()` correctly detects GC-marked objects without breaking the
  deletion cascade (`GetTypeNumber()` bypasses `Valid()` to avoid a mid-delete
  false negative)

### Pi Language Tests (330/330)
- Stack operations, control flow, continuations, functions
- All 330 tests pass including previously-failing edge cases

### Rho Language Tests (717/859)
- Core features working: expressions, conditionals, while/do-while, for loops,
  functions, recursion, closures
- 142 pre-existing failures: inline function calls inside `for x in container`
  loops return 0 instead of computed value; some nested-loop break/continue
  edge cases. These are unrelated to recent changes.

### Tau Language Tests (308/308)
- Namespace/class/interface parsing, struct and enum handling
- Proxy and agent code generation (GenerateProxy, GenerateAgent)
- `Future<T>` return types parse correctly
- Strict mode: semicolons required after method declarations; return type
  validation enforced

### Console Tests (11/11)
- Interactive REPL tests, language switching, peer connection/disconnect
- `PeerDisconnectCleanup` passes reliably after ENet graceful-disconnect fix:
  peers now receive `ConnectionLost` notification when a remote host shuts down

### Proxy Generation Tests (1/1)
- Tau IDL → C++ proxy code generation round-trip
- Generated proxy uses `_node->Send()` / `_node->SendWithResponse()` with
  `NetworkException` error wrapping

## Build Commands

```bash
# Run all non-network tests (default build)
./b
./Bin/Test/KaiTest
./Bin/Test/TestCore
./Bin/Test/TestPi
./Bin/Test/TestRho
./Bin/Test/TestTau
./Bin/Test/TestConsole
./Bin/Test/Test_ProxyGeneration

# Run network tests (requires --network build)
./b --network
./Bin/Test_Network

# Filter specific tests
./Bin/Test/TestRho --gtest_filter="*ForLoop*"
./Bin/Test_Network --gtest_filter="TauDomainPropertyTest*"
```

## Network Tests (Test_Network)

Built only when `KAI_NETWORKING=ON` (use `./b --network`).

### NodeEndToEndTest (6 tests)
| Test | Description |
|------|-------------|
| `RemoteMethodCallReturnsCorrectValue` | Client calls `Add(3,4)` on server agent, receives 7 |
| `EventBroadcastReachesSubscriber` | Server broadcasts `Ping`, client subscriber fires |
| `ObjectMessageReachesSubscriber` | Server sends `int 42` object, client receives it |
| `EventPayloadDecodedCorrectly` | Server broadcasts `Score` with int payload 99 |
| `RemotePropertyGetReturnsValue` | Client fetches `Counter` property (value 55) from server |
| `RemotePropertySetUpdatesValue` | Client sets `Counter` to 77, server value updates |

### TauDomainPropertyTest (3 tests)
| Test | Description |
|------|-------------|
| `IdlGeneratesExpectedClassNames` | `.tau` IDL generates `ISensorProxy` / `ISensorAgent` |
| `DomainBProxyFetchesPropertyFromDomainA` | Domain B reads `Value=42` from Domain A agent |
| `DomainBProxySetsPropertyOnDomainA` | Domain B writes `Value=99`, Domain A servant reflects it |

### TauPiSerializationTest (1 test)
| Test | Description |
|------|-------------|
| `LocalNodeRoundTrip` | Two nodes: Tau-generated proxy calls `Add(2,3)`, expects 5 |

### TauNetworkCommunicationTest (4 tests)
- IDL parsing, proxy/agent generation, error handling

### TestGenerateProxy (3 tests)
- Proxy code generation from Tau input

## Recent Changes (2026-04-03)

- **`Object::Valid()`**: Now checks `IsMarked()` to treat GC-pending objects as
  invalid. Removed verbose defensive catch-all blocks throughout `Object.cpp`.
- **`Object::GetTypeNumber()`**: Reads `class_base` directly instead of calling
  `Valid()`, preventing a TypeMismatch during the deletion cascade.
- **`Registry::ContainsHandle()`**: New inline for safe post-GC existence checks.
- **ENet graceful disconnect**: `NetPeer::Shutdown()` now sends `enet_peer_disconnect()`
  to all peers before `enet_host_destroy()`, ensuring remote peers receive a
  `ConnectionLost` event rather than timing out.
- **Tau proxy codegen**: `GenerateProxy::MethodBody()` uses `_node->Send()` /
  `_node->SendWithResponse()` with `NetworkException` wrapping.
- **Tau agent codegen**: `GenerateAgent::AgentDecl` generates `AgentBase<ClassName>`
  with the correct template parameter.
- **TauParser strict mode**: Semicolons required after method declarations;
  return type validated against known types.
