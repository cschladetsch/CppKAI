# KAI Project TODO

Last updated: 2026-04-25

## Current Status

```mermaid
gantt
    title Work Status
    dateFormat YYYY-MM-DD
    section Networking
    ENet transport layer        :done,   net1, 2025-11-01, 2025-12-01
    Node/Agent/Proxy RPC        :done,   net2, 2025-12-01, 2026-01-15
    Remote property get/set     :done,   net3, 2026-01-15, 2026-02-01
    KAI_NETWORKING build flag   :done,   net4, 2026-02-01, 2026-02-15
    TauDomainPropertyTest       :done,   net5, 2026-03-01, 2026-04-01
    IPv6 support                :active, net6, 2026-04-01, 2026-06-01
    section Language
    Tau Future<T> parser fix    :done,   tau1, 2026-03-15, 2026-04-01
    Tau EOF parser fix          :done,   tau2, 2026-04-25, 2026-04-25
    Rho for-loop inline fns     :active, rho1, 2026-04-01, 2026-05-01
    section Testing
    Network E2E tests (17)      :done,   tst1, 2026-02-01, 2026-04-01
```

## Networking — Done

- [x] Replace RakNet with ENet transport layer
- [x] `Node`/`Domain`/`Agent`/`Proxy` RPC pattern over UDP
- [x] `BinaryStream`-based `Send()` with `SendReliability`/`SendRouting` enums
- [x] `BufferOffset` type replacing bare `int` channel argument
- [x] `FetchProperty<T>` / `StoreProperty<T>` — remote property get/set
- [x] `BroadcastEvent` with optional payload
- [x] `SendObject` — broadcast KAI objects to connected peers
- [x] `ID_KAI_PROPERTY_GET` / `ID_KAI_PROPERTY_SET` message IDs
- [x] `KAI_NETWORKING` CMake option — controls ENet, Network lib, Tau libraries, tests
- [x] `./Scripts/b --network` build script flag
- [x] Tau code generation available through library APIs
- [x] 17 network end-to-end tests all passing
- [x] `TauDomainPropertyTest` — Domain A agent / Domain B proxy / property fetch+set

## Networking — Still Needed

- [ ] IPv6 support (currently IPv4 only)
- [ ] TLS / authenticated connections (plaintext today)
- [ ] Peer discovery on LAN (mDNS or broadcast)
- [ ] Rate limiting / back-pressure on incoming packets
- [ ] Connection heartbeat / automatic reconnect
- [ ] Streaming large objects (currently limited by single-packet size)

## Language

### Tau
- [x] `Future<T>` return types in IDL interfaces (lexer fix — `Future<int>` is one token)
- [x] EOF-safe `Expect()` handling in shared parser utilities (fixed Tau `Next token index out of range` failures)
- [x] Inline module parser test aligned with actual AST shape (`root -> Module -> children`)
- [x] `GenerateProxy` / `GenerateAgent` produce correct class names
- [ ] Struct fields serialized over network (currently structs are code-gen only)
- [ ] Event parameters deserialized in generated agent handlers
- [ ] `async` modifier properly reflected in generated code

### Rho
- [ ] Inline function calls inside `for x in container` loops (currently return 0)
  - `fun double(x) { x * 2 }` works; `for x in arr { sum = sum + double(x) }` does not
  - Root cause: Call nodes in ForEach body don't dispatch correctly
- [ ] `continue` in `foreach` (Mixed_ContinueInForEach still failing)
- [ ] Nested break/continue in C-style for loops
- [x] Python-style `for x in container` syntax
- [x] `break` and `continue` keywords
- [x] `foreach` keyword in Pi
- [x] Array/map indexing with `at`
- [x] Pi keyword validation (rejects `max`, `min`, `swap` etc. as Rho variable names)

### Pi
- [ ] `+!` stack operation
- [ ] `begin`/`until` operations
- [ ] `sin`, `cos`, `pow`, `sqrt`, `abs` math operations
- [ ] `at`, `slice`, `toint`, `tofloat` string operations
- [ ] Code block `{ ... }` translation to continuations (PiControlFlowTests)

## Core System

- [ ] Fix garbage collection cycles (Registry.cpp — current HACK avoids cycles)
- [ ] Complete pathname resolution (Pathname.cpp)
- [ ] Map serialization with BinaryStream (requires Registry reference)
- [ ] Type traits for custom/reflected types (TestReflection.cpp)

## Build System

- [ ] Clean up duplicate library inclusions in CMake
- [ ] Resolve clang-tidy warnings in ImGui
- [x] Ignore generated bracket-indexed CTest files such as `TestTau[1]_tests.cmake`

## GUI / Window

- [ ] Fix GLFW cursor types (ResizeAll, ResizeNESW, ResizeNWSE missing)
- [ ] Display stack items from bottom up (ExecutorWindow.h)

## Code Cleanup

### Empty/stub files to implement or remove
- `Test/Source/TestStringStream.cpp`
- `Test/Source/TestDebugTrace.cpp`
- `Include/KAI/Core/Thread/*.h` — thread headers are stubs
- `Include/KAI/Core/Method.cpp0x.h` — C++0x artifact

## Notes

- Total TODO/FIXME/HACK comments in codebase: ~76
- Shell (backtick) syntax disabled by default; enable with `-DENABLE_SHELL_SYNTAX=ON`
- Network tests only built/run with `./Scripts/b --network`
- Full suite passes with networking-enabled build; some network tests are still environment-skipped when local sockets are unavailable
