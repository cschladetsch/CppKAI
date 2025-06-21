# Rho Test Scripts

This directory contains Rho language scripts used by the test suite. Each script tests specific language features.

## Script Organization

- `basic_*.rho` - Basic functionality tests
- `*_function*.rho` - Function definition and calling tests
- `*_loop*.rho` - Loop construct tests (for, while, do-while)
- `if_*.rho` - Conditional statement tests
- `array_*.rho` - Array operation tests
- `pi_block_*.rho` - Pi language block integration tests

## Usage

Tests load these scripts using the `LoadRhoScript()` utility function:

```cpp
std::string script = kai::test::LoadRhoScript("BasicAddFunction.rho");
```

## Adding New Scripts

When adding new test scripts:
1. Use descriptive names that indicate what feature is being tested
2. Include comments in the script explaining the test
3. Keep scripts focused on testing one feature at a time
4. Ensure the script leaves a result on the stack for verification