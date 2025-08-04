# Shell Command Test Suite

This test suite comprehensively tests the shell command functionality added to the KAI Console.

## Features Tested

### 1. Basic Shell Commands (Tests 1-10)
- Commands with and without closing backticks
- Basic UNIX commands: `pwd`, `echo`, `whoami`, `date`, `ls`
- Verifies both `command` and `command syntax work

### 2. Complex Shell Commands (Tests 11-20)
- Commands with pipes and redirection
- Nested command substitution using `$()`
- Environment variable expansion
- Multi-argument commands with options

### 3. Embedded Command Substitution (Tests 21-30)
- Shell commands embedded within language expressions
- Arithmetic operations using embedded command results
- Multiple embedded commands in single expressions
- Complex shell operations with pipes in embedded context

### 4. Multi-Language Support (Tests 31-38)
- Shell commands in Rho language
- Language switching while preserving shell functionality
- Variable assignment using embedded commands
- Verification that functionality works across languages

### 5. Error Handling (Tests 39-48)
- Empty shell commands
- Invalid/non-existent commands
- Commands returning non-zero exit codes
- Malformed embedded commands (missing closing backticks)
- Recovery after errors

## Test Structure

### Test Files
- `test_basic_shell_commands.txt` - Basic functionality tests
- `test_complex_shell_commands.txt` - Advanced shell features
- `test_embedded_commands.txt` - Command substitution in expressions
- `test_rho_language.txt` - Multi-language support
- `test_error_cases.txt` - Error handling and edge cases

### Test Runner
- `run_shell_tests.sh` - Automated test execution script
- Runs all test files and verification tests
- Generates detailed logs and summary reports
- Provides colored output for easy result interpretation

## Running the Tests

### Prerequisites
1. Build the KAI Console:
   ```bash
   cd /path/to/KAI/build
   ninja Console
   ```

2. Ensure shell tools are available: `pwd`, `whoami`, `echo`, `ls`, `date`, `bc`

### Execute Test Suite
```bash
cd /path/to/KAI/Test/ShellCommandTests
./run_shell_tests.sh
```

### Expected Output
- Colored terminal output showing test progress
- Final summary with pass/fail counts
- Detailed log file in `results/test_results.log`
- Individual test output files in `results/` directory

## Test Coverage

| Category | Test Count | Description |
|----------|------------|-------------|
| Basic Commands | 10 | Simple shell commands with/without closing backticks |
| Complex Commands | 10 | Advanced shell features (pipes, substitution, etc.) |
| Embedded Commands | 10 | Command substitution within expressions |
| Multi-Language | 8 | Testing across Pi and Rho languages |
| Error Cases | 10 | Error handling and edge cases |
| Verification | 3 | Automated result verification |
| **Total** | **51** | **Comprehensive coverage of all functionality** |

## Functionality Verified

### Core Features
- **Shell Commands**: Lines starting with ` execute as shell commands  
- **Flexible Syntax**: Works with or without closing backtick  
- **Embedded Substitution**: `expression with `command`` expands before processing  
- **Language Independent**: Works in Pi, Rho, and any other KAI language  
- **Error Handling**: Graceful handling of invalid commands and syntax  

### Advanced Features
- **Complex Shell Operations**: Pipes, redirection, command substitution  
- **Environment Variables**: Access to shell environment  
- **Multi-word Commands**: Commands with arguments and options  
- **Arithmetic Integration**: Shell command results used in calculations  
- **Cross-Language Persistence**: Functionality maintained across language switches  

## Expected Results

When all tests pass, you should see:
- Shell commands execute and display output correctly
- Embedded commands expand and integrate with language expressions
- Error cases fail gracefully without crashing the console
- Functionality works consistently across different languages
- Complex shell operations (pipes, substitution) work as expected

## Debugging Failed Tests

If tests fail:
1. Check `results/test_results.log` for detailed error information
2. Check individual test output files in `results/` directory
3. Ensure all prerequisite shell commands are available
4. Verify the Console binary is built correctly
5. Run individual test commands manually to isolate issues

## Adding New Tests

To add new tests:
1. Create new test cases in appropriate test file
2. Follow the naming convention: `# Test N: Description`
3. Include both positive and negative test cases
4. Update this README with new test descriptions
5. Run the full test suite to ensure no regressions