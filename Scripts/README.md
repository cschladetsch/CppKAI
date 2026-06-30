# KAI Scripts

This directory contains utility scripts for building, testing, and demonstrating KAI functionality.

## Build Scripts

### build.ps1
Main build script with options for different build types:
```bash
./build.ps1              # Debug build
./build.ps1 release      # Release build
./build.ps1 clean        # Clean build
```

### clean_and_build.ps1
Complete clean rebuild of the entire project.

### install-llvm.ps1
Installs LLVM dependencies required for KAI compilation.

## Test Scripts

### run_all_tests.ps1
Comprehensive test runner that executes all test suites:
- Core system tests
- Language tests (Pi, Rho, Tau)
- Network tests
- Console tests

### Language-Specific Test Scripts

- **run_rho_tests.ps1** - Execute all Rho language tests
- **run_rho_demo.ps1** - Interactive Rho language demonstration
- **test_tau.ps1** - Tau language test suite
- **run_chat_tests.ps1** - Chat system validation

### Network Test Scripts

- **run_connection_tests.ps1** - Network connection validation
- **run_tau_connection_tests.ps1** - Tau network interface tests
- **p2p_test.ps1** - Peer-to-peer networking tests
- **p2p_test_dynamic.ps1** - Dynamic P2P configuration tests

### Console Test Scripts

- **run_console_demo.ps1** - Console application demonstration
- **run_fixed_tests.ps1** - Specific fixed test cases

## Network Scripts

### network/
Contains specialized networking scripts:
- **run_peers.ps1** - Start multiple peer instances
- **automated_demo.ps1** - Automated network demonstration
- **run_continuation_migration_demo.ps1** - Two-process proof that freezes a Pi
  continuation in one process, sends it to another process, thaws it, resumes
  it, and verifies the returned result
- **run_continuation_migration_tmux_demo.ps1** - tmux-friendly continuation migration demo

## Analysis Scripts

### analyze_complexity.py
Python script for code complexity analysis with configuration in `complexity_config.json`.

### analyze_test_history.ps1
Analyzes test execution history and generates reports.

## Utility Scripts

### remove_claude_refs.ps1
Utility for cleaning up AI-generated comments and references.

### calc_test.ps1
Calculator functionality testing script.

## Helper Scripts

- **b** - Root-oriented CMake build wrapper. Run it from the repository root as `./Scripts/b`.
- **be** - Build with networking enabled and run the scripted test suite.
- **run_tests** - Run the in-tree test binaries from `Bin/Test`.
- **tidy** - Apply `clang-format` to source and test C++ files.
- **r** - Repair encoding artifacts in `Test/demo_console_communication.ps1`.

## Usage Examples

### Quick Build and Test
```bash
./Scripts/clean_and_build.ps1
./Scripts/run_all_tests.ps1
```

### Language Development Workflow
```bash
# Test Rho language changes
./Scripts/run_rho_tests.ps1
./Scripts/run_rho_demo.ps1

# Test networking changes
./Scripts/p2p_test.ps1
```

### Interactive Demos
```bash
# Console networking demo
./Scripts/run_console_demo.ps1

# Language feature demo
./Scripts/run_rho_demo.ps1

# Continuation mobility proof stack
./Bin/ContinuationMobilityDemo
./Scripts/network/run_continuation_migration_demo.ps1
```

## Script Requirements

Most scripts require:
- CMake 3.10+
- C++17 compatible compiler
- ENet networking library
- tmux (for interactive demos)

Network scripts may require:
- Multiple terminal sessions
- Available network ports (14600-14699 range)
- Firewall permissions for local networking

## Adding New Scripts

When creating new scripts:
1. Make them executable: `chmod +x script_name.ps1`
2. Add usage comments at the top
3. Include error handling and cleanup
4. Test on multiple platforms
5. Update this README with the new script description

## Related Documentation

- [Build Guide](../Doc/BUILD.md)
- [Test Guide](../Doc/Test.md)
- [Network Testing](../Doc/ConnectionTesting.md)
- [Console Guide](../Source/App/Console/README.md)
