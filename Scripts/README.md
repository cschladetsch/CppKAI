# KAI Scripts

This directory contains utility scripts for building, testing, and demonstrating KAI functionality.

## Build Scripts

### build.sh
Main build script with options for different build types:
```bash
./build.sh              # Debug build
./build.sh release      # Release build
./build.sh clean        # Clean build
```

### clean_and_build.sh
Complete clean rebuild of the entire project.

### install-llvm.sh
Installs LLVM dependencies required for KAI compilation.

## Test Scripts

### run_all_tests.sh
Comprehensive test runner that executes all test suites:
- Core system tests
- Language tests (Pi, Rho, Tau)
- Network tests
- Console tests

### Language-Specific Test Scripts

- **run_rho_tests.sh** - Execute all Rho language tests
- **run_rho_demo.sh** - Interactive Rho language demonstration
- **test_tau.sh** - Tau language test suite
- **run_chat_tests.sh** - Chat system validation

### Network Test Scripts

- **build_and_test_network.sh** - Build and test network functionality
- **run_connection_tests.sh** - Network connection validation
- **run_tau_connection_tests.sh** - Tau network interface tests
- **p2p_test.sh** - Peer-to-peer networking tests
- **p2p_test_dynamic.sh** - Dynamic P2P configuration tests

### Console Test Scripts

- **run_console_demo.sh** - Console application demonstration
- **run_fixed_tests.sh** - Specific fixed test cases

## Network Scripts

### network/
Contains specialized networking scripts:
- **run_peers.sh** - Start multiple peer instances
- **automated_demo.sh** - Automated network demonstration

## Analysis Scripts

### analyze_complexity.py
Python script for code complexity analysis with configuration in `complexity_config.json`.

### analyze_test_history.sh
Analyzes test execution history and generates reports.

## Utility Scripts

### remove_claude_refs.sh
Utility for cleaning up AI-generated comments and references.

### calc_test.sh
Calculator functionality testing script.

## Demo Scripts

Located in project root:
- **demo_console_communication.sh** - Interactive console networking demo using tmux

## Usage Examples

### Quick Build and Test
```bash
./Scripts/clean_and_build.sh
./Scripts/run_all_tests.sh
```

### Language Development Workflow
```bash
# Test Rho language changes
./Scripts/run_rho_tests.sh
./Scripts/run_rho_demo.sh

# Test networking changes
./Scripts/build_and_test_network.sh
./Scripts/p2p_test.sh
```

### Interactive Demos
```bash
# Console networking demo
./Scripts/run_console_demo.sh

# Language feature demo
./Scripts/run_rho_demo.sh
```

## Script Requirements

Most scripts require:
- CMake 3.10+
- C++17 compatible compiler
- RakNet networking library
- tmux (for interactive demos)

Network scripts may require:
- Multiple terminal sessions
- Available network ports (14600-14699 range)
- Firewall permissions for local networking

## Adding New Scripts

When creating new scripts:
1. Make them executable: `chmod +x script_name.sh`
2. Add usage comments at the top
3. Include error handling and cleanup
4. Test on multiple platforms
5. Update this README with the new script description

## Related Documentation

- [Build Guide](../Doc/BUILD.md)
- [Test Guide](../Doc/Test.md)
- [Network Testing](../Doc/ConnectionTesting.md)
- [Console Guide](../Source/App/Console/README.md)