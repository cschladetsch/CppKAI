# Unit Tests

See the [Unit Tests](https://github.com/cschladetsch/KAI/tree/master/Source/Test/Source) here. Locally they are just in ${KAI_HOME}/Source/Test.

There is a Readme in there too, and new Readme in each test folder.

## Running Tests

Test programs are located in the `Bin/Test/` directory after building the project. The main test programs are:

- `TestCore`: Core functionality tests
- `TestPi`: Pi language tests
- `TestRho`: Rho language tests
- `TestTau`: Tau language tests

### Command-Line Options

All test programs support standard Google Test command-line options, plus some custom options:

- `--debug-color` or `--color`: Explicitly enable colored output (redundant as color is on by default)
- `--no-color`: Disable colored output

Example:
```bash
# Run all Pi tests with colored output (default)
./Bin/Test/TestPi

# Run a specific test with colored output
./Bin/Test/TestPi --gtest_filter="PiBinaryOpTests.IntegerAddition"

# Run without colored output
./Bin/Test/TestPi --no-color
```

See [ColorOutput.md](ColorOutput.md) for more details about the colored output feature.

## Test Output

Test output includes:
- Standard Google Test result reporting
- Color-coded log messages (INFO in green, WARNING in yellow, ERROR in red)
- Grey console metadata for improved readability

## Test Organization

Tests are organized by language and functionality:
- Core tests: Basic data structures, memory management, registry operations
- Language tests: Pi, Rho, and Tau language features
