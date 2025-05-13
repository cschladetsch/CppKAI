# Testing Rho Assert Functionality

This guide outlines how to test the fixed assert functionality in the Rho language.

## Simple Test Case

For a simple test of the assert functionality, you can use the following code:

```cpp
// Create a console with Rho language
Console console;
console.SetLanguage(Language::Rho);

// Test a simple assertion
console.Process("assert(1 + 1 == 2)");
```

This should execute without errors if the assert condition is true.

## Comparison with Pi

To compare with Pi language for reference:

```cpp
// Switch to Pi language
console.SetLanguage(Language::Pi);

// Test Pi assertion
console.Process("1 1 + 2 == assert");
```

## More Complex Examples

To test more complex Rho assertions:

```cpp
// Variable assignment and assertion
console.Process("x = 10");
console.Process("y = 5");
console.Process("assert(x + y == 15)");

// Boolean logic and comparison operators
console.Process("assert(true && !false)");
console.Process("assert(10 > 5 && 5 < 10)");
console.Process("assert(5 >= 5 && 5 <= 5)");

// Combined arithmetic and comparison
console.Process("assert((10 + 5) * 2 == 30)");
```

## Testing in a Script

For a more comprehensive test, you can use the `rho_expression_test.cpp` file provided in the repository. This file tests various Rho expressions and assertions.

To compile and run this test:

```bash
g++ -I ./Include rho_expression_test.cpp -o rho_expression_test -std=c++17 -L ./Lib -lKAI
./rho_expression_test
```

Note: Due to compilation issues with the KAI codebase, you may need to resolve these issues before the test can be successfully compiled and run.

## Common Issues

If you encounter issues with the assert functionality, check the following:

1. Ensure the expression is being evaluated directly, not wrapped in a Continuation
2. Verify the Console.cpp Execute method is properly processing Rho expressions
3. Check that the RhoTranslator is using AppendDirectOperation for assert calls
4. Make sure the expression result is properly being passed to the assert operation

## Testing for Failure Cases

To test that assert properly fails on false conditions:

```cpp
// This should cause an assertion failure
console.Process("assert(1 + 1 == 3)");
```

When assert fails, it should throw an appropriate exception that can be caught and handled.

## Integration with Existing Tests

The existing test files in Test/Language/TestRho/ should also be updated to work with the new execution model. In many cases, this simply means removing workarounds that were added to compensate for the continuation wrapping issue.