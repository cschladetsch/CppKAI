# Ninja Build System Integration

This documentation describes the changes made to support the Ninja build system for the KAI project.

## Changes Made

1. Modified `./r` script to use Ninja instead of Make
   - Now uses a separate `build_ninja` directory to avoid conflicts
   - Improved error handling and status reporting
   - Updated path resolution for finding the Console executable

2. Created a new `./n` script for incremental Ninja builds
   - Preserves the build directory between runs
   - Supports passing arguments directly to Ninja
   - Provides better status reporting

3. Created a new `./nt` script for running specific tests
   - Allows building and running a specific test (e.g., `./nt TestPi`)
   - Shows colorized test results
   - Lists available tests when no arguments are provided

4. Fixed several compilation errors:
   - Changed `SetProperty` calls to `SetPropertyValue` with proper Label objects
   - Fixed scope brackets in switch-case statements to prevent jumping over variable initialization
   - Temporarily disabled parent-child navigation in the AST (needs proper implementation)
   - Updated Console.cpp to use simplified language detection

## Usage

### Full Clean Build

To perform a full clean build of the project:

```bash
./r
```

This will:
1. Remove and recreate the `build_ninja` directory
2. Run CMake with the Ninja generator
3. Build the project with Ninja
4. Run the tests
5. Launch the Console application

### Incremental Build

For faster incremental builds during development:

```bash
./n
```

This will:
1. Use the existing `build_ninja` directory (creating if it doesn't exist)
2. Build only what has changed since the last build
3. Optionally run the tests if the build succeeds

You can also pass specific targets to build:

```bash
./n Console
./n TestPi
```

### Running Specific Tests

To build and run just one test:

```bash
./nt TestPi
```

This will:
1. Build only the specified test
2. Run the test and show results
3. Return the test exit code

## Remaining Issues

Several compilation issues still need to be addressed:

1. TestForLoopSemicolons.cpp and other backup test files have include path issues:
   ```
   fatal error: ../../Include/TestLangCommon.h: No such file or directory
   ```

2. BinaryStream operator errors in OpCodes.h:
   ```
   error: no match for 'operator>>' (operand types are 'kai::BinaryStream' and 'kai::Operation')
   ```

3. Remaining issues with parent-child navigation in RhoTranslator.cpp:
   ```
   class kai::AstNodeBase<...> has no member named 'GetParent'
   ```

4. Format string warnings in PiTranslator.cpp:
   ```
   warning: '%s' directive output may be truncated writing up to 4095 bytes
   ```

## Benefits of Ninja

1. Faster build times compared to Make
2. Better parallelization of compilation tasks
3. More efficient dependency tracking
4. Consistent build system across platforms

## Future Work

1. Add proper parent-child navigation to the AST nodes to support "pi { ... }" blocks
2. Fix compilation issues with test files in the backups directory
3. Implement proper property access for Continuation objects
4. Fix format string truncation warnings
5. Consider adding a CMake option to specify the build system
6. Add support for build profiles (Debug/Release)
7. Fix the AST node navigation system