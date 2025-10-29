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
- [x] Fixed continue in C-style for loops (infinite loop bug)
- [x] Changed C-style for loop transpilation to use Pi `for` operation
- [x] Added parser support for inline function syntax `fun name(args) { expr }`

### 📊 Current Test Status
| Test Suite                    | Pass | Total | Rate |
|-------------------------------|------|-------|------|
| ForEachLoopTest               | 9    | 10    | 90%  |
| RhoBreakContinueTests         | 12   | 12    | 100% ✅ |
| RhoAllIterationMethodsTest    | 50   | 65    | 77%  |

### ❌ Blocking Issues to Fix

#### 1. ✅ Array/Map Indexing (FIXED)
**Status**: All 5 array/map indexing tests now pass!

**Solution**: Implemented Operation::GetChild in ExecutorPerform.cpp to handle array/map indexing with the `at` keyword.

#### 2. Function Calls in Loops (PRIORITY 1 - Blocks 10 tests) ⚠️ IN PROGRESS
**Problem**: Inline functions using `fun name(args) { expr }` syntax parse correctly but return 0.

**Root Cause**:
- Parser now accepts inline function syntax (fixed "Statement expected" error)
- Functions parse and transpile but don't return correct values
- All Function_* tests get 0 instead of expected values
- Issue affects both foreach loops and regular code

**Status**: Parser implemented, execution/transpilation debugging needed

**Failing tests**: 10 Function_* tests + ForEach_WithFunction

**Working example** (old style):
```rho
fun square(n) return n * n
for (i = 1; i <= 4; i = i + 1)  # Parentheses style works!
    sum = sum + square(i)
```

**Failing example** (new style):
```rho
fun double(x) { x * 2 }
for x in arr  # Python style fails!
    sum = sum + double(x)  # "invalid continuation" error
```

**TODO**:
- [ ] Debug RhoTranslate.cpp ForEach transpilation with function calls
- [ ] Check if function calls in ForEach body generate valid Pi code
- [ ] Compare Pi output for old-style vs new-style loops with functions
- [ ] May need to fix how Call nodes are handled inside ForEach context

#### 3. ForEach Variable Access Issue (PRIORITY 2 - Blocks 4 tests) ❓ INVESTIGATING
**Problem**: Some foreach loops with conditionals get 0 instead of computed values.

**Status**: NOT a parser issue with conditional-only bodies!
- ForEach_WithConditional passes (has if with no trailing code)
- ForEach_MaxValue fails (also has if with no trailing code)
- Both use identical patterns, suggesting variable access or comparison issue

**Pattern** (this works in some tests, fails in others):
```rho
for x in arr
    if x > max     # Condition may not evaluate correctly
        max = x    # Or assignment may not work
```

**Failing tests**:
- `ForEach_MaxValue` (expects 9, gets 0)
- `ForEach_MinValue` (expects 2, gets initial value)
- `ForEach_FilteringPattern`
- `Mixed_ContinueInForEach`

**Theory**: Variable `x` might not be accessible in condition, or variables from outer scope
aren't being updated correctly. Needs transpilation/execution debugging.

### 🎯 Next Steps
1. ✅ ~~Fix array indexing~~ → DONE! Fixed 5 tests (Issue #1)
2. ✅ ~~Fix continue in C-style for loops~~ → DONE! RhoBreakContinueTests: 12/12 (100%)
3. 🔄 Debug inline function execution → In progress (Issue #2, blocks 11 tests)
4. 🔄 Debug foreach variable access → Investigating (Issue #3, blocks 4 tests)
5. Current: 50/65 tests passing (77%), RhoBreakContinueTests: 12/12 (100%)
6. Target: 65/65 tests passing (100%)

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