# Future Work

This document tracks known gaps and planned improvements across the KAI codebase.

## Language

### Rho — Inline Functions in `for x in container` Loops

**Status**: Active

Functions defined with `fun name(args) { body }` work correctly in most contexts but return 0 when called inside Python-style foreach loops.

```rho
fun double(x) { x * 2 }
for x in arr           // this works
    total = total + 1  // OK

for x in arr
    total = total + double(x)  // BUG: double(x) returns 0
```

Root cause: Call nodes generated inside a `ForEach` continuation body are not dispatched correctly by the executor. The fix is in `RhoTranslator.cpp` ForEach generation and/or `Executor.cpp` ForEach dispatch.

**Affected files**:
- `Source/Library/Language/Rho/Source/RhoTranslator.cpp`
- `Source/Library/Executor/Source/ExecutorPerform.cpp`

### Tau — Struct Serialization

Tau IDL supports `struct` declarations and generates plain C++ structs, but there is no automatic binary serialization for user-defined structs. Passing a struct across the network requires manual `BinaryStream` read/write code.

**Needed**: `GenerateStruct` should emit `operator<<(BinaryStream&)` and `operator>>(BinaryStream&)` methods.

### Pi — Missing Operations

| Operation | Status |
|-----------|--------|
| `+!` (increment in place) | Not implemented |
| `begin`/`until` | Not implemented |
| `sin`, `cos`, `pow`, `sqrt`, `abs` | Not implemented |
| `at`, `slice`, `toint`, `tofloat` | Not implemented |
| Code blocks `{ }` as continuations | Partial |

## Networking

### IPv6

The current `IpAddress` and ENet binding only support IPv4. Add an `IpAddress("::1")` constructor and pass `ENET_HOST_ANY6` to `enet_host_create`.

### TLS / Authenticated Connections

All wire traffic is currently plaintext. Options:
- DTLS wrapper over ENet sockets
- Shared-secret HMAC per packet
- Peer certificate exchange on connect

### Peer Discovery

Nodes must be connected explicitly by IP and port. A LAN discovery mechanism (mDNS or UDP broadcast) would allow nodes to find each other automatically.

### Streaming Large Objects

Packets are currently limited to single ENet frames. Objects larger than ~1 MB need fragmentation/reassembly at the application layer.

## Core

### Garbage Collection Cycles

`Registry.cpp` contains a `HACK` comment around the cycle-breaking logic. A proper tri-colour incremental GC with cycle detection should replace it.

### Map Serialization

`BinaryStream` serialization of `Map` objects requires a `Registry` reference for type lookup, but `BinaryStream` is not tied to a `Registry`. The serialization protocol needs a design decision: embed type IDs in the stream, or require the caller to supply a `Registry`.

## Testing

### Console Tests

Console tests are at ~25% pass rate. The missing features are:
- Tab completion
- History expansion (`!!`, `!N`)
- Zsh-style interactive prompts

### Shell (Backtick) Tests

Shell syntax is disabled by default (`-DENABLE_SHELL_SYNTAX=ON` to enable). All backtick tests are skipped in the default build.

## Documentation

- [ ] `Doc/TauTutorial.md` — add a complete worked example from `.tau` → generated code → running test
- [ ] `Doc/PiTutorial.md` — document `Suspend`/`Resume`/`Replace` continuation operators
- [ ] `Doc/RhoTutorial.md` — document `for x in container` and `break`/`continue`
