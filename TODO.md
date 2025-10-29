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
| ForEachLoopTest               | 8    | 10    | 80%  |
| RhoBreakContinueTests         | 9    | 12    | 75%  |
| RhoAllIterationMethodsTest    | 43   | 65    | 66%  |

### ❌ Blocking Issues to Fix

#### 1. Array/Map Indexing (PRIORITY 1 - Blocks 4-6 tests)
**Problem**: Array indexing causes hangs or failures. Root cause: Pi's `at` operation (GetChild) not implemented in executor.

**Failing tests**:
- `RangeStyle_WithArray`, `RangeStyle_ForLoop`
- `Mixed_WhileWithArray`, `Mixed_DoWhileWithArray`
- `ForEachLoopTest.ForEachMap`

**Fix**:
```cpp
// In Source/Library/Executor/Source/ExecutorPerform.cpp, add:
case Operation::GetChild: {
    auto key = Pop();
    auto container = Pop();
    if (container.IsType<Array>()) {
        int index = ConstDeref<int>(key);
        Push(Deref<Array>(container).At(index));
    } else if (container.IsType<Map>()) {
        Push(Deref<Map>(container).Get(key));
    }
    break;
}
```

```cpp
// In Source/Library/Language/Rho/Source/RhoTranslate.cpp:631, change:
return container + " " + index + " at";  // Not: "[index]"
```

#### 2. Function Calls in Loops (PRIORITY 2 - Blocks 10-12 tests)
**Problem**: Function calls inside loops don't work.

**Failing tests**: All Function_* tests (MapPattern, FilterPattern, ReducePattern, etc.)

**TODO**:
- [ ] Test: `fun f(x) return x * 2; for i in [1,2,3] sum = sum + f(i)`
- [ ] Check if issue is scope, continuation, or function lookup
- [ ] Verify function call transpilation in loop context

#### 3. Conditionals in ForEach (PRIORITY 3 - Blocks 4-6 tests)
**Problem**: Some foreach loops with `if` statements fail. Loop body doesn't execute or executes incorrectly.

**Example failure**: `ForEach_MaxValue` expects 9, gets 0
```rho
arr = [3, 7, 2, 9, 4]
max = 0
for x in arr
    if x > max
        max = x
max  # Expected: 9, Got: 0
```

**Failing tests**:
- `ForEach_FilteringPattern`, `ForEach_MaxValue`, `ForEach_MinValue`
- `Mixed_ContinueInForEach`, `Mixed_NestedForEachInWhile`

**TODO**:
- [ ] Debug why nested `if` in `for x in arr` sometimes fails
- [ ] Check indentation parsing for nested blocks
- [ ] Verify Pi code generation for conditionals inside foreach

### 🎯 Next Steps
1. Fix array indexing (1-2 hours) → Should fix 4-6 tests
2. Debug function calls in loops (2-4 hours) → Should fix 10-12 tests
3. Fix conditional logic in foreach (1-2 hours) → Should fix 4-6 tests
4. Target: 90%+ tests passing

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