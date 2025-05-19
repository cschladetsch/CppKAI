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

## Recent Improvements

Recent enhancements to the Rho language implementation include:

- **Fixed Binary Operations**: Corrected issues with binary operations handling
- **Enhanced Control Structures**: Improved support for all control flow constructs
- **Extended Test Coverage**: Comprehensive test suite covering all language features
- **Documentation Updates**: Refreshed documentation with more examples

For detailed information on recent fixes, see the [Rho Fix Documentation](Rho-Fix-Documentation.md).

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

## Demo Script

A comprehensive demo script is available at `Test/Language/TestRho/Scripts/Demo.rho`. This script showcases all the major features of Rho and can be run using the provided `Scripts/run_rho_demo.sh` script.

## Advanced Topics

For more advanced usage of Rho, refer to these specialized documents:

- [Continuations in Rho](ContinuationControl.md): Advanced control flow
- [Rho-Pi Integration](CommonLanguageSystem.md): Working across language boundaries
- [Type System](TypeSystem.md): Understanding type handling in Rho

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