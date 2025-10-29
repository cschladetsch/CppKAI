# KAI Project TODO List

## 🔄 Current Work: Python-Style Iteration Implementation (2025-10-29)

### ✅ Completed
- [x] Removed parentheses requirement from for loops
- [x] Implemented Python-style `for x in container` syntax
- [x] Added `foreach` keyword to Pi language
- [x] Added `break` and `continue` keywords to Pi
- [x] Fixed ForEach exception handling for proper control flow
- [x] Updated all Rho → Pi transpilation for loops
- [x] Updated 74 test files to use new Python-style syntax

### 📊 Current Test Status
| Test Suite                    | Pass | Total | Rate |
|-------------------------------|------|-------|------|
| ForEachLoopTest               | 9    | 10    | 90%  |
| RhoBreakContinueTests         | 9    | 12    | 75%  |
| RhoAllIterationMethodsTest    | 50   | 65    | 77%  |

### ❌ Blocking Issues to Fix

#### 1. ✅ Array/Map Indexing (FIXED)
**Status**: All 5 array/map indexing tests now pass!

**Solution**: Implemented Operation::GetChild in ExecutorPerform.cpp to handle array/map indexing with the `at` keyword.

#### 2. Function Calls in Loops (PRIORITY 1 - Blocks 10 tests)
**Problem**: Function calls inside loops don't work.

**Failing tests**: 9 Function_* tests + ForEach_WithFunction + ForEachLoopTest.ForEachWithFunction

**TODO**:
- [ ] Test: `fun f(x) return x * 2; for i in [1,2,3] sum = sum + f(i)`
- [ ] Check if issue is scope, continuation, or function lookup
- [ ] Verify function call transpilation in loop context

#### 3. Conditionals in ForEach (PRIORITY 2 - Blocks 4 tests)
**Problem**: ForEach loops with `if` that has no code after it fail. Parser issue.

**Pattern that works**:
```rho
for x in arr
    if condition
        do_something
    code_after_if  # This is required!
```

**Pattern that fails**:
```rho
for x in arr
    if condition
        do_something
    # Nothing after if - FAILS!
```

**Example**: `ForEach_MaxValue` expects 9, gets 0 because the `if` block has no code after it.

**Failing tests**:
- `ForEach_MaxValue`, `ForEach_MinValue` (if with no following code)
- `ForEach_FilteringPattern` (likely same issue)
- `Mixed_ContinueInForEach` (complex case)

**TODO**:
- [ ] Fix Rho parser to allow `if` blocks without trailing statements inside foreach
- [ ] Check RhoParser.cpp ForEach block parsing logic
- [ ] Verify Pi code generation for single-statement if blocks

### 🎯 Next Steps
1. ✅ ~~Fix array indexing~~ → DONE! Fixed 5 tests
2. Debug function calls in loops (2-4 hours) → Should fix 10 tests
3. Fix conditional logic in foreach (1-2 hours) → Should fix 4 tests
4. Current: 50/65 tests passing (77%)
5. Target with fixes: 64/65 tests passing (98%)

---

## High Priority Issues

### Build System
- [ ] Fix TestCore linking errors with Executor library
- [ ] Resolve clang-tidy warnings in ImGui library
- [ ] Clean up duplicate library inclusions in CMake

### Core System
- [ ] Fix garbage collection issues (Registry.cpp:201 - HACK to avoid cycles)
- [ ] Implement proper pathname resolution (Pathname.cpp)
- [ ] Complete String allocation optimization (String.cpp)
- [ ] Fix Map serialization with BinaryStream requiring Registry (TestMap.cpp)

### Language Implementation

#### Pi Language
- [ ] Implement code blocks { ... } translation to continuations (PiControlFlowTests.cpp)
- [ ] Fix while operation stack management issues (PiControlFlowTests.cpp)
- [ ] Fix for operation expecting 4 continuations (PiControlFlowTests.cpp)
- [ ] Implement +! stack operation (PiStackManipulationTests.cpp)
- [ ] Implement begin/until operations (PiStackManipulationTests.cpp)
- [ ] Fix nested scope resolution with & operation (TestPiLabels.cpp)
- [ ] Implement math operations: sin, cos, pow, sqrt, abs (PiMathOperationsTests.cpp)
- [ ] Implement string operations: at, slice, toint, tofloat (PiStringOperationsTests.cpp)

#### Rho Language
- [ ] Merge duplicate lexer code with PiLexer::PathnameOrKeyword (RhoLexer.cpp)

### Reflection System
- [ ] Fix type traits for custom types (TestReflection.cpp)
- [ ] Implement proper property access through reflection
- [ ] Complete type traits meta-programming support

### Network System
- [ ] Implement IPv6 support (currently only IPv4) (Peer.cpp)
- [ ] Implement object message processing (Node.cpp)
- [ ] Implement function call processing (Node.cpp)
- [ ] Implement event notification processing (Node.cpp)
- [ ] Implement connection event handling (Node.cpp)

### Window/GUI System
- [ ] Fix GLFW cursor types (missing ResizeAll, ResizeNESW, ResizeNWSE)
- [ ] Implement display items starting from bottom (ExecutorWindow.h)
- [ ] Fix unfocused window cursor changes with GLFW 3.2

### Tests
- [ ] Complete Pathname tests (TestPathname.cpp)
- [ ] Fix String Contains method or use std::string::find (TestSerialization.cpp)
- [ ] Complete Function invocation tests (TestFunction.cpp)
- [ ] Test base pointer leaking (TestFunction.cpp)

## Code Cleanup

### Empty Files to Remove or Implement
- Test/Source/TestStringStream.cpp
- Test/Source/TestDebugTrace.cpp
- Include/KAI/Core/Thread/*.h (all thread-related headers)
- Include/KAI/Platform/PC/Threads/*.h (all platform thread headers)

### Redundant/Unusual Files to Review
- Include/KAI/Core/Method.cpp0x.h (C++0x specific implementation?)
- Include/KAI/Core/Detail/CallableBase.h (marked as .notused in CallableBase.h.notused)

### Documentation
- [ ] Update architecture documentation (Doc/Architecure.md)
- [ ] Complete language guides for Pi, Rho, and Tau
- [ ] Document the reflection system
- [ ] Add examples for network programming

## Future Enhancements
- [ ] Implement full C++23 features where applicable
- [ ] Add more comprehensive error messages
- [ ] Improve memory management and reduce allocations
- [ ] Add performance benchmarks
- [ ] Implement missing mathematical operations
- [ ] Add more string manipulation functions

## Notes
- Total TODO/FIXME/HACK comments in codebase: 76
- Many Pi language operations are not yet implemented
- Reflection system needs significant work
- Network system is partially implemented