# KAI Project Test Summary

Generated: 2026-10-02, from a full build of `master` on Linux (Clang, Release,
`KAI_NETWORKING=ON`) with the CppKaiCore and CppKaiConsoleLib fixes of the
same date. `py run_tests.py` registers **2,202 CTest entries; all pass, none
disabled.**

## Overall Status

Counts are gtest cases per binary (`--gtest_list_tests`). The language and
core suites register one CTest entry per case; `TestConsole` and
`TestNetwork` register one entry each.

| Suite | Tests | Status | Covers |
|-------|-------|--------|--------|
| TestCore | 178 | all pass | Registry, GC, BinaryStream, Array, Map |
| TestPi | 683 | all pass | Pi language, continuations, control flow |
| TestRho | 864 | all pass | Rho language, iteration, functions, early returns |
| TestSigma | 129 | all pass | Sigma type checker, generated Rho, example scripts, continuation operators |
| TestTau | 519 | all pass | Tau parser/codegen, futures, proxies |
| TestConsole | 70 | all pass | Console commands, history, languages |
| TestNetwork | 332 | all pass | Networking and Tau-over-network |
| Others | | all pass | LogTest, PerformanceTests, Test_ProxyGeneration, ContinuationMobilityDemoTests, KshUnitTests |

## Test Suite Details

### Core Tests (178)
- Registry, type system, memory management, garbage collection
- BinaryStream serialization, Array, Map containers

### Pi Language Tests (683)
- Stack operations, control flow, continuations, functions
- Arithmetic, stack manipulation, control flow, and interpreter coverage

### Rho Language Tests (864)
- Expressions, control flow, functions, recursion, closures, iteration, and translation coverage
- `RhoEarlyReturnInLoop`: early `return` inside an `if`, both in a function called from a loop and where the `if` calls a function first

### Sigma Language Tests (129)
- Type checking and `line:col` errors, generated Rho, sessions, native C++ types
- `SigmaScriptTests`: one test per program in `Test/Language/TestSigma/Scripts`
- `SigmaContinuationTests`: `f(x)&` and `f(x)!`, and every rejected use

### Tau Language Tests (519)
- Namespace/class/interface parsing, struct and enum handling
- Proxy/agent code generation, `Future<T>` parsing, strict-mode validation

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
./Bin/Test/TestSigma
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

- **2026-10-02:** every count above was re-run. Two executor fixes in
  CppKaiCore made early returns work in Rho and Sigma (a `return` inside an
  `if`, in a function called from a loop, and in an `if` block that calls a
  function first); `SigmaTests.EarlyReturnInFunctionCalledFromLoop` is no
  longer disabled. `RhoAllIterationMethodsTest.Mixed_ContinueInForEach`,
  listed as failing in older notes, passes.
- Historical documents that mention partial Rho failures describe older
  baselines and should not be treated as current.
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
