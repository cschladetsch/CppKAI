# Rho Language

## Overview

Rho is an infix-notation language in the KAI system, designed to provide a familiar syntax for developers accustomed to languages like Python, JavaScript, or C++. While offering a comfortable programming experience, Rho maintains full access to KAI's powerful capabilities through its seamless integration with Pi and its native support for continuations.

Rho serves as the primary application language in the KAI system, complementing Pi (the foundation language) and Tau (the interface definition language).

## Getting Started

To get started with Rho:

1. **Build the KAI System**: Use the [out-of-source build approach](./OUT_OF_SOURCE_BUILD.md)
   ```bash
   # Create build directory and build the project
   mkdir -p build && cd build
   cmake ..
   cmake --build .
   ```
   
2. **Read the Documentation**: Start with the [Rho Tutorial](RhoTutorial.md) for a comprehensive introduction

3. **Explore Examples**: Look through the [example scripts](../Test/Language/TestRho/Scripts/) for working code samples

4. **Run the Demo**: Execute `./Scripts/run_rho_demo.sh` to see Rho in action with a comprehensive demo

5. **Try It Yourself**: Use the KAI Console application to write and execute your own Rho code
   ```bash
   # After building, run the console
   ./bin/Console
   ```

## Key Features

Rho offers a rich set of features for application development:

- **Familiar Syntax**: Infix notation with Python-like syntax
- **Control Structures**: If/else conditions, for/while/do-while loops
- **Functions**: First-class functions with parameters and recursion
- **Binary Operations**: Full set of arithmetic, logical, comparison, and bitwise operators
- **Pi Integration**: Embed Pi code blocks directly within Rho code
- **Continuations**: Native support for advanced control flow
- **Strong Typing**: Type safety with automatic conversions where appropriate
- **Variable Scoping**: Proper lexical scoping with nested environments

## Recent Improvements (May 2025)

The Rho language implementation has undergone major fixes that resolved critical issues:

- **Fixed Type Mismatch Issues**: Resolved fundamental problems where the translator was generating incorrect types
- **Removed Direct Evaluation**: Eliminated approximately 1000 lines of code that was evaluating expressions at translation time
- **Fixed Store Operation**: Corrected operand ordering for variable assignments
- **All Tests Passing**: All 120 Rho language tests now pass successfully

### Technical Details

The root cause was that RhoTranslator was attempting to evaluate expressions during translation and appending primitive values (like Signed32) where the executor expected Operation or Continuation objects. The fix involved:

1. Removing all direct evaluation logic from TranslateBinaryOp
2. Eliminating the massive direct evaluation code in the PiSequence case
3. Ensuring proper operation generation for runtime execution
4. Fixing Store operation to handle operands in the correct order

For detailed information on recent fixes, see the [Rho Fix Documentation](Rho-Fix-Documentation.md) and [Todo-Rho.md](../Test/Language/TestRho/Todo-Rho.md).

## Examples

### Basic Syntax

```rho
// Variable assignment
x = 42
name = "Alice"
is_valid = true

// Arithmetic operations
sum = 5 + 3         // 8
product = 7 * 6      // 42
complex = (4 + 2) * 3 - 5  // 13

// Logical operations
result1 = true && false     // false
result2 = true || false     // true
result3 = !true             // false
```

### Control Structures

```rho
// If-else statement
if (score >= 60) {
    grade = "Pass"
} else {
    grade = "Fail"
}

// For loop
sum = 0
for (i = 0; i < 10; i = i + 1) {
    sum = sum + i
}

// While loop
counter = 0
while (counter < 5) {
    counter = counter + 1
}

// Do-while loop
i = 0
do {
    i = i + 1
} while (i < 5)
```

### Functions

```rho
// Function definition
function add(a, b) {
    return a + b
}

// Recursive function
function factorial(n) {
    if (n <= 1) {
        return 1
    } else {
        return n * factorial(n - 1)
    }
}

// Function call
result = factorial(5)    // 120
```

### Pi Integration

```rho
// Using Pi within a Rho expression
result = 5 + pi{ 2 3 + }    // 5 + 5 = 10

// Dedicated Pi block
pi{
    1 2 3        // Push values onto the stack
    + +          // Add them together
    'sum #       // Store in variable 'sum'
}

// Access Pi variables from Rho
total = sum @    // Retrieves the value stored in 'sum'
```

### Event System

```rho
// Create an event
onScoreChanged = CreateEvent()

// Add event handlers
onScoreChanged.addHandler(function(newScore) {
    console.log("Score changed to: " + newScore)
})

// Add another handler
onScoreChanged.addHandler(function(newScore) {
    if (newScore > highScore) {
        highScore = newScore
    }
})

// Trigger the event
score = 100
onScoreChanged(score)  // Calls all registered handlers
```

## Demo Script

A comprehensive demo script is available at `Test/Language/TestRho/Scripts/Demo.rho`. This script showcases all the major features of Rho and can be run using the provided `Scripts/run_rho_demo.sh` script.

## Advanced Topics

For more advanced usage of Rho, refer to these specialized documents:

- [Continuations in Rho](ContinuationControl.md): Advanced control flow
- [Rho-Pi Integration](CommonLanguageSystem.md): Working across language boundaries
- [Type System](TypeSystem.md): Understanding type handling in Rho
- [Event System](EventSystem.md): Using the modern C++23 event system from Rho

## Contributing

If you'd like to contribute to Rho language development:

1. Explore the [test suite](../Test/Language/TestRho/) to understand the implementation
2. Add new test cases to cover additional features or edge cases
3. Update documentation with new examples and clarifications
4. Submit your changes according to the project's contribution guidelines

## Resources

- [KAI Language Guide](LanguageGuide.md): Overview of all KAI languages
- [Pi Tutorial](PiTutorial.md): Understanding the foundation language
- [Tau Tutorial](TauTutorial.md): Interface definition language information