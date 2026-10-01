# KAI Language Tests

This directory contains comprehensive test suites for all KAI programming languages: Pi, Rho, Sigma, Tau, and Hlsl.

## Language Test Directories

### TestPi/
Tests for the Pi stack-based language:
- **Basic Operations**: Arithmetic, stack manipulation, data types
- **Advanced Features**: Functions, continuations, control flow
- **Language Integration**: Pi-to-Rho interoperability
- **Performance Tests**: Stack operations and memory usage
- **Script Examples**: Real-world Pi programs

### TestRho/
Tests for the Rho infix language:
- **Syntax Validation**: C-like syntax parsing and execution
- **Control Structures**: If/else, loops, functions
- **Language Features**: Variables, scoping, recursion
- **Cross-Language**: Rho-to-Pi compilation verification
- **Advanced Constructs**: Closures, generators, pattern matching

### TestSigma/
Tests for the statically typed Sigma language:
- **SigmaTests**: lexing, parsing, type checking, generated Rho, and running programs
- **SigmaMoreTests**: further type rules and rejected programs, with the expected `line:col` errors
- **SigmaScriptTests**: one test per example program in `TestSigma/Scripts/*.sigma`
- One test, `SigmaTests.DISABLED_EarlyReturnInFunctionCalledFromLoop`, waits on an executor fix (see [Doc/Sigma](../../Doc/Sigma/README.md#known-issues))

### TestTau/
Tests for the Tau Interface Definition Language:
- **Code Generation**: Proxy and stub generation
- **Network Interfaces**: Remote object definitions
- **Type System**: Complex type definitions and validation
- **Namespace Support**: Hierarchical organization
- **Integration Tests**: Generated code compilation

### TestHlsl/
Tests for HLSL (High Level Shading Language) support:
- **Shader Compilation**: HLSL to intermediate representation
- **Graphics Integration**: GPU computation support
- **Syntax Parsing**: HLSL-specific language constructs

## Common Test Infrastructure

### TestLangCommon.cpp
Shared testing utilities and base classes for all language tests:
- Language setup and teardown
- Common assertion helpers
- Cross-language execution support

## Running Language Tests

### All Language Tests
```bash
make test
ctest -R Language
```

### Specific Languages
```bash
ctest -R TestPi     # Pi language tests
ctest -R TestRho    # Rho language tests  
ctest -R TestSigma  # Sigma language tests
ctest -R TestTau    # Tau IDL tests
ctest -R TestHlsl   # HLSL tests
```

### Verbose Output
```bash
ctest -R TestRho -V  # Shows detailed test output
```

## Test Categories by Feature

### Basic Language Features
- Lexical analysis and parsing
- Expression evaluation
- Variable assignment and scoping
- Function definition and calling

### Advanced Features
- Control flow (if/else, loops)
- Exception handling
- Memory management
- Garbage collection integration

### Cross-Language Integration
- Pi ↔ Rho interoperability
- Sigma → Rho → Pi compilation
- Language switching in console
- Mixed-language script execution
- Type compatibility across languages

### Performance and Stress Tests
- Large program execution
- Memory usage validation
- Stack overflow protection
- Error recovery mechanisms

## Language-Specific Examples

### Pi Examples
Located in `TestPi/Scripts/`:
- Arithmetic operations and stack manipulation
- Function definitions and recursion
- Array and string operations
- Advanced control flow

### Rho Examples  
Located in `TestRho/Scripts/`:
- C-like syntax demonstrations
- Object-oriented features
- Functional programming constructs
- Error handling patterns

### Sigma Examples
Located in `TestSigma/Scripts/`:
- Sorting, searching, a prime sieve and matrix multiplication
- Recursion (Hanoi, Fibonacci, GCD/LCM) and higher-order functions
- Lists, maps and strings with static types
- Each script must type-check, run, and end with a `true` expression

### Tau Examples
Located in `TestTau/Scripts/`:
- Interface definitions
- Network service descriptions
- Code generation templates
- Distributed object interfaces

## Related Documentation

- [Pi Language Tutorial](../../Doc/PiTutorial.md)
- [Rho Language Tutorial](../../Doc/RhoTutorial.md)
- [Sigma Language Reference](../../Doc/Sigma/README.md)
- [Tau Language Tutorial](../../Doc/TauTutorial.md)
- [Language Guide](../../Doc/LanguageGuide.md)
- [Common Language System](../../Doc/CommonLanguageSystem.md)

## Contributing

When adding new language tests:
1. Follow the existing test structure in each language directory
2. Add both positive and negative test cases
3. Include example scripts demonstrating the feature
4. Update the appropriate README.md file
5. Ensure tests pass on all supported platforms