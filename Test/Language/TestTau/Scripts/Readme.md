# Tau Language Test Scripts

This directory contains test scripts for the Tau language parser, lexer, and code generators. Each script tests different aspects of the Tau language.

## Test Script Overview

- **Tau1.tau, Tau2.tau**: Basic syntax tests for the Tau language
- **NumberTest.tau**: Tests numeric literal parsing (integers, floats, scientific notation)
- **AssignmentTest.tau**: Tests field assignments and default parameter values
- **InheritanceTest.tau**: Tests class inheritance syntax and parsing
- **NestedNamespaces.tau**: Tests nested namespace syntax and parsing
- **ErrorTest.tau**: Contains intentional errors to test error handling
- **ComplexProxy.tau**: Tests complex proxy generation with multiple classes

## Running the Tests

The test scripts are automatically executed by the TestTau executable. To run the tests:

```
./Bin/Test/TestTau
```

## Test Features

The test scripts cover the following Tau language features:

- Basic class and interface declarations
- Property/field declarations and initialization
- Method declarations with parameters and return types
- Default parameter values
- Numeric literals (integers, floats, scientific notation)
- String literals
- Inheritance 
- Nested namespaces
- Error handling
- Proxy generation

## Output Directory

Generated code from the test scripts will be stored in the `Output` directory.