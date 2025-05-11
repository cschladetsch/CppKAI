# Rho Language Tests

This directory contains tests for the Rho language implementation in KAI.

## Current Status

The Rho language implementation currently has significant type mismatch issues that affect even basic operations. See [Todo-Rho.md](Todo-Rho.md) for a detailed analysis of the issues.

## Test Files

### Rho Language Tests (Currently Disabled)

These tests directly test Rho language functionality but are currently disabled due to type mismatch issues:

- `TestRho.cpp`: Main Rho language test suite 
- `TestDoWhile.cpp`: Tests for do-while loop functionality
- `TestForLoopSemicolons.cpp`: Tests for for-loop functionality with semicolon handling

### Pi Language Stand-in Tests

These tests use Pi language to test functionality that should eventually work in Rho:

- `SimpleRhoPiTests.cpp`: Basic Pi tests covering arithmetic, stack operations, and simple functions (all tests pass)
- `AdvancedRhoPiTests.cpp`: More advanced Pi tests (only some tests pass)

#### Working Pi Tests:
- Basic arithmetic (addition, subtraction, multiplication)
- Stack operations (dup, swap)
- Simple comparison operations (>, ==, !=)
- Logical OR operation (||)

#### Pi Tests With Issues:
- Division (/) and modulo (%) - not properly implemented in Pi
- Logical AND (&&) - has type mismatch issues
- Complex comparison operations (<=, >=) - not implemented in Pi
- Function calling with parameters - call operation not found
- Variable storage/retrieval - store/retrieve operations not found

These Pi tests serve as a reference implementation and demonstrate what functionality should work once the Rho implementation is fixed. The failing tests highlight areas where even the Pi implementation has limitations.

## Running the Tests

To run only the tests that are known to pass, use the `run_tests_passing` script in the root directory:

```bash
./run_tests_passing
```

This script will run all Core and Pi tests, as well as only the Rho tests that are known to pass (including the Pi stand-in tests).

## Adding New Tests

When adding new Rho language tests:

1. If testing basic Rho functionality that's currently broken, consider adding a Pi language test instead
2. Use the test name prefix `DISABLED_` for any test that fails due to the known Rho language issues
3. Update the `run_tests_passing` script to include your new test if it's expected to pass

## Scripts Directory

The `Scripts` directory contains various Rho language scripts used in the tests. Even though some are not currently functional, they serve as a reference for the intended Rho language syntax and features.

## References

- [KAI Languages](../../Languages.md): Overview of all languages in KAI
- [Todo-Rho.md](Todo-Rho.md): Detailed analysis of Rho language issues
- [Todo-DoWhile.md](../../../Todo-DoWhile.md): Specific issues related to do-while loops