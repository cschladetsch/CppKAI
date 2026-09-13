# KAI Project Test Summary

Generated: 2026-09-13 (Core/Rho/Tau/Network counts are a 2026-04-04 snapshot; Pi has been re-verified since - see Notes).

## Overall Status

| Suite | Passed | Total | Rate | Notes |
|-------|--------|-------|------|-------|
| Core | 147 | 147 | 100% | Registry, GC, BinaryStream, Array, Map |
| Pi | 519 | 519 | 100% | All Pi language tests passing (grew from 330 as of the 2026-04-04 snapshot - see Notes) |
| Rho | 876 | 876 | 100% | Full language suite passing |
| Tau | 308 | 308 | 100% | Tau parser/codegen and future/proxy tests passing |
| Network | 17 | 17 | 100% | Network and Tau-over-network tests passing |

## Test Suite Details

### Core Tests (147/147)
- Registry, type system, memory management, garbage collection
- BinaryStream serialization, Array, Map containers

### Pi Language Tests (519/519)
- Stack operations, control flow, continuations, functions
- Arithmetic, stack manipulation, control flow, and interpreter coverage all pass

### Rho Language Tests (876/876)
- Expressions, control flow, functions, recursion, closures, iteration, and translation coverage pass
- Historical continuation and loop regressions referenced in older reports are no longer present in the current suite snapshot

### Tau Language Tests (308/308)
- Namespace/class/interface parsing, struct and enum handling
- Proxy/agent code generation, `Future<T>` parsing, strict-mode validation
- Includes Tau future, proxy, and codegen coverage

## Build Commands

```bash
# Configure and build (Linux/macOS)
./Scripts/build.sh
# or: mkdir -p build && cd build && cmake .. && cmake --build .

# Windows (native, Clang + Ninja by default)
py build.py

# Run individual suites
./Bin/Test/TestCore
./Bin/Test/TestPi
./Bin/Test/TestRho
./Bin/Test/TestTau
./Bin/Test/TestNetwork      # Built by default (KAI_NETWORKING=ON); pass -DKAI_NETWORKING=OFF to skip

# Filter specific tests
./Bin/Test/TestRho --gtest_filter="*ForLoop*"
./Bin/Test/TestNetwork --gtest_filter="TauDomainPropertyTest*"
```

See [Doc/BUILD.md](BUILD.md) for the full option reference.

## Network Tests (TestNetwork)

Built by default (`KAI_NETWORKING=ON`); pass `-DKAI_NETWORKING=OFF` to skip.

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

## Notes

- The Core/Rho/Tau/Network counts above reflect a `develop` branch snapshot as
  of 2026-04-04. Historical documents that mention partial Rho failures
  describe older baselines still further back and should not be treated as
  current. The Pi count has since been re-verified at 519/519 (see below) and
  is more current than the rest of this table.
- **As of 2026-09-13, two Pi continuation tests that were failing are now
  fixed**: `TestPiAdvancedContinuations.TestConditionalContinuation` and
  `TestPiAdvancedControlFlow.TestContinuationConditional` (a `TypeMismatch`
  thrown from `ExecuteContinuationInline` due to a deep-comparison bug in
  `Object::operator!=` — see [`Doc/TODO.md`](TODO.md#core-system) for
  details). `PiAdvancedTests.ExtremeRecursiveSum` was a segfault caused by
  insufficient stack size, fixed by the `/STACK:16777216` linker flag added
  to the test/console targets. All three now pass, and the full `TestPi`
  suite (519 tests, up from the 330 in the 2026-04-04 snapshot as more Pi
  tests were added since) is green.
- **This snapshot is older than [`Doc/TODO.md`](TODO.md).** TODO.md's
  "Language" section lists specific, currently-tracked gaps that postdate
  this summary — including a failing test (`Mixed_ContinueInForEach`, under
  "`continue` in `foreach`") and several unimplemented Rho/Pi behaviors
  (inline function calls inside `for x in container`, and a list of missing
  Pi operations). Where the two documents disagree, treat TODO.md as current
  and this file as the last point at which the numbers above were true.
  Re-run the suites and update both documents together before relying on
  either in isolation.
