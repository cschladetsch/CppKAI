# Future Work for KAI Tests

This document outlines the remaining work needed to fix all KAI tests, focusing on the complex cases that are still failing.

## 1. Control Flow Structure Tests

The RhoForLoop tests are still failing with error code 1 (test failure, not segmentation fault). These require:

- Complete implementation of the for loop parsing in Rho
- Proper handling of loop variables and scoping
- Proper continuation execution for nested loop bodies
- Support for loop variable updates

Affected files:
- `Source/Library/Executor/Source/Executor.cpp`
- `Source/Library/RhoLang/Source/RhoTranslator.cpp`
- `Test/Language/TestRho/TestForLoopSemicolons.cpp`

## 2. Advanced Operations Tests

The RhoAdvancedOps tests are failing with segmentation faults. These require:

- Better handling of complex expression evaluation
- Improved operator precedence handling
- Support for mixed-type operations
- Handling of edge cases like division by zero
- Support for short-circuit logic evaluation

Affected files:
- `Source/Library/Executor/Source/Executor.cpp`
- `Source/Library/Core/Source/Object.cpp`
- `Source/Library/Core/Source/Operation.cpp`

## 3. TestLangCommon Advanced Tests

The more advanced language common tests involving semicolons and complex statements need:

- Better handling of statement separation
- Proper parsing of complex expressions with semicolons
- Support for inline control structures
- Improved handling of nested expressions

Affected files:
- `Source/Library/Executor/Source/Console.cpp`
- `Source/Library/RhoLang/Source/RhoParser.cpp`
- `Test/Language/TestLangCommon.cpp`

## 4. Continuation Nesting and Unwrapping

A comprehensive solution for complex continuations needs:

- Recursive unwrapping of deeply nested continuations
- Better handling of continuation execution order
- Support for continuation merging
- Cleanup of temporary continuations

Affected files:
- `Source/Library/Executor/Source/Executor.cpp`
- `Test/Include/TestLangCommon.h`

## Implementation Strategy

The recommended approach for addressing these issues is:

1. Start with the TestLangCommon improvements since they provide the foundation
2. Address RhoAdvancedOps tests next to improve complex expression handling
3. Finally tackle the control flow structures in RhoForLoop tests

For each area:
1. Add comprehensive try-catch blocks around execution paths
2. Add null checks for all object accesses
3. Implement proper tracing to diagnose specific failure points
4. Add fallback mechanisms for graceful degradation when operations can't be completed

## Testing Strategy

For each fix:
1. Run the specific failing test in isolation with detailed tracing
2. Use GDB to track down segmentation faults
3. Add instrumentation around the failing code points
4. Verify fixes incrementally rather than trying to fix everything at once

The Scripts/run_rho_tests.sh script provides a good foundation for iterative testing and development.