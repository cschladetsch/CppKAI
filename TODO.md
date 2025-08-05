# KAI Project TODO List

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