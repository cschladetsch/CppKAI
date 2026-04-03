# Test Systems

These are test systems based on GoogleTest of the core systems and languages created by KAI:

* Core
* Language
* Network

These share a `Common` test library.

Tests are built in the `Bin/Test/` directory.

## Test Documentation

- **[Console Networking Tests](Console/README.md)** - Console-to-console communication testing
- **[Connection Testing Guide](../Doc/ConnectionTesting.md)** - Network connection validation
- **[Test Status Summary](../TEST_SUMMARY.md)** - Current test suite status
- **[Main Test Guide](../Doc/Test.md)** - Comprehensive testing documentation

## Folders

* **Common**. Common to all tests
* **Include**. Common to all tests
* **Language**. Tests for each language.
* **Network**. Network tests.
* **Source**. General KAI Tests

## Test Programs

After building the project, you'll find these test executables in the `Bin/Test/` directory:

* **TestCore**: Tests for core functionality, containers, and memory management
* **TestPi**: Tests for the Pi stack-based language
* **TestRho**: Tests for the Rho expression-based language
* **TestTau**: Tests for the Tau network description language

## Selecting Tests

To focus only on some tests, use the `--gtest_filter` flag:

```bash
# Run only tests in the PiBinaryOpTests test suite
./Bin/Test/TestPi --gtest_filter=PiBinaryOpTests.*

# Run a specific test
./Bin/Test/TestPi --gtest_filter=PiBinaryOpTests.IntegerAddition
```

## Colored Output

All test programs feature colored output by default to improve readability:

* **Green**: INFO log messages
* **Yellow**: WARNING log messages
* **Red**: ERROR log messages
* **Grey**: Console metadata

### Color-Related Command-Line Options

* `--debug-color` or `--color`: Explicitly enable colored output (redundant as color is on by default)
* `--no-color`: Disable colored output

Example:
```bash
# Run with colored output disabled
./Bin/Test/TestPi --no-color --gtest_filter=PiBinaryOpTests.IntegerAddition
```

For more details, see [ColorOutput.md](/Doc/ColorOutput.md).
