# Window Application Tests

This directory contains comprehensive tests for the KAI Window application, covering all three tabs (Pi, Rho, and Debugger) and their interactions.

## Test Structure

### TestExecutorWindow.cpp
Core functionality tests for the ExecutorWindow class:

#### Pi Tab Tests
- **Initialization**: Verifies correct initial state
- **Basic Arithmetic**: Tests arithmetic operations (+, -, *, /)
- **Stack Operations**: Tests dup, swap, drop, clear
- **Variables**: Tests variable assignment and retrieval
- **Conditionals**: Tests if-then and if-then-else constructs
- **Loops**: Tests loop constructs and while equivalents
- **Arrays**: Tests array creation and operations
- **Strings**: Tests string literals and concatenation
- **Functions**: Tests function definition and recursion
- **Error Handling**: Tests error cases and messages
- **History**: Tests command history functionality
- **Shell Commands**: Tests $ and backtick shell execution

#### Rho Tab Tests
- **Tab Switching**: Tests switching to Rho mode
- **Basic Arithmetic**: Tests infix arithmetic expressions
- **Variables**: Tests variable declaration and usage
- **Functions**: Tests function definition with parameters
- **Control Flow**: Tests if-else statements and nesting
- **Loops**: Tests for and while loops
- **Arrays**: Tests array creation and indexing
- **Multiline Input**: Tests multiline code execution
- **Error Handling**: Tests error detection and reporting
- **Pi Block Execution**: Tests Pi blocks within Rho

#### Debugger Tab Tests
- **Initialization**: Tests debugger initial state
- **Start/Stop**: Tests debugging state transitions
- **Step Execution**: Tests step-by-step execution
- **Stack Viewing**: Tests stack display functionality
- **Watch Variables**: Tests variable watching
- **Log Messages**: Tests debug log functionality

#### Integration Tests
- **Tab Switching**: Tests seamless tab transitions
- **Language Isolation**: Tests separation of language contexts
- **Buffer Clearing**: Tests input buffer management
- **Clear All Logs**: Tests global clear functionality
- **Stack Persistence**: Tests stack state across tab switches
- **Debugger Integration**: Tests debugger with language tabs

#### Edge Cases and Stress Tests
- **Empty Commands**: Tests handling of empty input
- **Long Commands**: Tests buffer limits
- **Rapid Tab Switching**: Tests UI stability
- **Many Commands**: Tests history capacity
- **Large Stack**: Tests stack size limits

### TestWindowUI.cpp
UI-specific tests focusing on visual components and user interaction:

#### UI Component Tests
- **Input Buffers**: Tests single-line and multi-line input
- **Tab Selection**: Tests visual tab selection
- **Scroll Behavior**: Tests auto-scroll functionality
- **Log Formatting**: Tests output formatting for each language
- **Debugger Controls**: Tests debug UI elements
- **Watch Index**: Tests stack item selection

#### History Navigation Tests
- **Navigation**: Tests moving through command history
- **Isolation**: Tests separate histories per language

#### Clear Operations Tests
- **Single Language**: Tests clearing individual language logs
- **All Languages**: Tests clearing all logs simultaneously

#### Stack Display Tests
- **Empty Stack**: Tests empty stack display
- **With Items**: Tests stack display with various types

#### Error Display Tests
- **Pi Errors**: Tests Pi error message display
- **Rho Errors**: Tests Rho error message display

#### Shell Command UI Tests
- **Dollar Commands**: Tests $ prefix commands
- **Backtick Commands**: Tests backtick shell execution
- **Quick Substitution**: Tests ^old^new^ syntax

#### UI State Persistence Tests
- **Tab Switch**: Tests state preservation during switches
- **Debugger Switch**: Tests debugger state handling

#### Special Character Tests
- **Input Handling**: Tests special character input
- **Unicode Support**: Tests Unicode character handling

### TestWindowPerformance.cpp
Performance and stress tests to ensure responsive UI:

#### Command Execution Performance
- **Single Command**: Tests individual command speed
- **Batch Commands**: Tests bulk command execution
- **Complex Expressions**: Tests performance with complex code

#### Tab Switching Performance
- **Speed**: Tests rapid tab switching
- **With Content**: Tests switching with populated tabs

#### Memory Usage Tests
- **History Growth**: Tests memory usage with large history
- **Log Growth**: Tests memory with many log entries
- **Stack Growth**: Tests memory with large stacks

#### Input Buffer Performance
- **Large Input**: Tests maximum input size handling
- **Multiline Large Input**: Tests large multiline input

#### Concurrent Operations
- **Rapid Commands**: Tests rapid command entry
- **Mixed Operations**: Tests various operations together

#### Search Performance
- **Large History**: Tests searching in large histories
- **Large Logs**: Tests searching in large log files

#### Stress Tests
- **Maximum Load**: Tests system under heavy load
- **Long Running**: Tests extended session performance

#### Language-Specific Performance
- **Pi Performance**: Tests Pi-specific operations
- **Rho Performance**: Tests Rho-specific operations

#### Debugger Performance
- **Step Performance**: Tests debugging step speed
- **Large Stack Display**: Tests large stack rendering

## Running the Tests

### Build the tests:
```bash
cd /path/to/KAI/Test/Window
mkdir build
cd build
cmake ..
make
```

### Run all tests:
```bash
./TestWindow
```

### Run specific test suites:
```bash
./TestWindow --gtest_filter=ExecutorWindowTest.*
./TestWindow --gtest_filter=WindowUITest.*
./TestWindow --gtest_filter=WindowPerformanceTest.*
```

### Run specific tests:
```bash
./TestWindow --gtest_filter=ExecutorWindowTest.PiTab_BasicArithmetic
./TestWindow --gtest_filter=WindowUITest.InputBuffer_MultiLine
./TestWindow --gtest_filter=WindowPerformanceTest.Stress_MaximumLoad
```

## Test Coverage

The test suite provides comprehensive coverage of:
- All three tabs (Pi, Rho, Debugger)
- All major language features
- UI components and interactions
- Error handling and edge cases
- Performance characteristics
- Memory management
- Integration between components

## Adding New Tests

When adding new features to the Window application:
1. Add corresponding tests to the appropriate test file
2. Follow the existing naming conventions
3. Test both success and failure cases
4. Include performance tests for new features
5. Update this README with new test descriptions

## Dependencies

- Google Test framework
- KAI Core libraries
- KAI Language libraries (Pi, Rho)
- ImGui (for UI components)