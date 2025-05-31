# Code Cleanup Notes

## Files Removed
1. `Test/Source/TestStringStream.cpp` - Empty test file
2. `Test/Source/TestDebugTrace.cpp` - Empty test file
3. `Include/KAI/Core/Method.cpp0x.h` - Obsolete C++0x specific implementation
4. `Include/KAI/Core/Thread/` - Empty thread headers (not implemented)
5. `Include/KAI/Platform/PC/Threads/` - Empty platform thread headers (not implemented)

## Files That Need Attention
1. `Include/KAI/Core/CallableBase.h.notused` - Despite the .notused extension, this file IS used by FunctionBase and MethodBase
2. `Include/KAI/Core/Detail/CallableBase.h` - Different implementation, not compatible with the .notused version
3. Multiple "Fixed" test files in TestRho - These appear to be iterative fixes and could potentially be consolidated

## TODOs Added
- Created comprehensive `TODO.md` file with 76 identified TODO/FIXME/HACK comments categorized by:
  - Build System issues
  - Core System improvements
  - Language implementations (Pi, Rho, Tau)
  - Reflection System
  - Network System
  - Window/GUI System
  - Test improvements

## Build Status
- Window app builds successfully with GLEW
- Console app builds successfully
- Test suites have some linking issues with Executor library
- FixedCoreSuite builds independently without language dependencies