# Continuations in the Rho Language

## Continuations: Only for Blocks and Pi{} Statements

In the Rho language implementation, continuations should only be used for two specific cases:

1. **Code blocks**: Sections of code that need to be treated as a unit
2. **Pi{} statements**: Embedded Pi language code within Rho

For all other operations, especially simple expressions like arithmetic and comparisons, the language should directly evaluate to primitive values (int, bool, String).

## Implementation Details

The key changes made to support this design:

1. Removed `UnwrapValue` method from `Executor.h` and `Executor.cpp`
2. Modified the `Continue` method to no longer automatically unwrap continuations
3. Implemented the `ExtractValueFromContinuation` method in `TestLangCommon.h` which preserves block continuations
4. Implemented `UnwrapStackValues` in `TestLangCommon.cpp` to properly handle stack values in tests
5. Fixed references to the removed `UnwrapValue` method in:
   - `Console.cpp`
   - `RhoTranslator.cpp`

## Test Framework Support

The test framework includes helper methods in `TestLangCommon` to extract values from any continuation objects that might be produced during testing:

1. `ExtractValueFromContinuation`: Converts continuations to primitive values, but preserves block continuations
2. `UnwrapStackValues`: Processes the data stack to handle any unexpected continuation objects
3. `AssertResult`: Test helper that automatically handles value extraction

## How to Use in Tests

```cpp
// Use AssertResult for simple expressions
AssertResult<int>("2 + 3", 5);

// For more complex tests, use UnwrapStackValues manually
console_.Execute("complex_expression");
UnwrapStackValues();
ASSERT_EQ(AtData<int>(0), expected_value);
```

## When Dealing with Blocks

Block continuations are preserved as-is by the extraction process. This allows the test framework to properly handle code that intentionally creates continuations:

```cpp
// When testing code with blocks
console_.Execute("{ 2 + 3 }");  // Block should remain a continuation
// No unwrapping needed for blocks
```

## Implementation Details

The extraction logic in `TestLangCommon::ExtractValueFromContinuation` checks for:

1. Block markers (ContinuationBegin/End operations)
2. Simple expressions that should have been directly evaluated
3. Binary operations that can be calculated from their operands