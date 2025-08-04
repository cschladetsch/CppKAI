# KAI Project - Claude Instructions

## Project Overview
KAI is a C++ project implementing a multi-language runtime system with networking capabilities. It includes:
- Language interpreters (Pi, Rho, Tau, Lisp, Hlsl)
- Core object system with garbage collection
- Network layer with peer-to-peer capabilities
- Console and GUI applications

## Build System
- Uses CMake as the primary build system
- Includes custom scripts in Scripts/ directory

## Command Aliases for KAI Project

### Build Commands
- "build" → run `cmake --build . --config Release`
- "clean build" → run `./Scripts/clean_build.sh`
- "build tau" → run `./Scripts/buildtau.sh`

### Test Commands  
- "test" → run `./Scripts/run_all_tests.sh`
- "rho tests" → run `./Scripts/run_rho_tests.sh`
- "tau tests" → run `./Scripts/test_tau.sh`
- "network tests" → run `./Scripts/build_and_test_network.sh`
- "connection tests" → run `./Scripts/run_connection_tests.sh`
- "console tests" → run `./Test/Console/RunConsoleTests.sh`

### Demo Commands
- "console demo" → run `./Scripts/run_console_demo.sh`
- "rho demo" → run `./Scripts/run_rho_demo.sh` 
- "p2p demo" → run `./Scripts/p2p_test.sh`

### Network Commands
- "peers" → run `./Scripts/network/run_peers.sh`
- "automated demo" → run `./Scripts/network/automated_demo.sh`

## Project Structure
- `Include/KAI/` - Header files for core system
- `Source/Library/` - Implementation of core libraries
- `Source/App/` - Application executables (Console, Window, etc.)
- `Test/` - Comprehensive test suites
- `Doc/` - Project documentation
- `Scripts/` - Build and test automation scripts

## Key Components
- **Core**: Object system, registry, memory management
- **Executor**: Stack-based execution engine  
- **Languages**: Pi (stack-based), Rho (functional), Tau (C++-like)
- **Network**: Peer-to-peer networking with RakNet
- **Console**: Interactive REPL with language switching

## Development Notes
- Follow existing C++ coding conventions in the codebase
- Use smart pointers (project has undergone smart pointer migration)
- Network functionality can be disabled via build configuration
- Tests are organized by component (Core, Language, Network)
- The project supports cross-platform builds (Windows, Linux, macOS)

## Important Files
- `CMakeLists.txt` - Main build configuration
- `Readme.md` - Project documentation
- `TODO.md` - Current development tasks
- `TEST_SUMMARY.md` - Test status overview