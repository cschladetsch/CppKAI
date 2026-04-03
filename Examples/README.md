# KAI Examples

This directory contains example code demonstrating various features of the KAI language system.

## Available Examples

### ModernCppDemo.cpp
Demonstrates modern C++ integration with KAI's reflection system, showing how to expose C++ classes and methods to the scripting runtime.

### Pi Language Examples

- **foreach_examples.pi** - Demonstrates foreach loop constructs in Pi
- **test_foreach.pi** - Test cases for Pi foreach functionality

## Building Examples

Examples are built as part of the main KAI build process:

```bash
mkdir build && cd build
cmake ..
make
```

## Running Examples

After building, examples can be found in:
```
build/Source/App/Console/
```

Run the console application to execute Pi and Rho scripts:
```bash
./console script.pi
./console -l rho script.rho
```

## Related Documentation

- [Pi Language Tutorial](../Doc/PiTutorial.md)
- [Rho Language Tutorial](../Doc/RhoTutorial.md)
- [Console Guide](../Source/App/Console/README.md)
- [Language Examples](../Test/Language/)

For more comprehensive examples, see the test directories:
- `Test/Language/TestPi/Scripts/` - Pi language examples
- `Test/Language/TestRho/Scripts/` - Rho language examples