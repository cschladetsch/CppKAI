# Console Zsh Feature Tests

This directory contains tests for the zsh-like history expansion features implemented in the KAI Console.

## Full Documentation

For complete documentation of all features, see:
- [Console Zsh Features Documentation](/Source/App/Console/Source/Readme.md)
- [Advanced Features Guide](/Source/App/Console/Source/AdvancedZshFeatures.md)
- [Quick Reference](/Source/App/Console/Source/ZshQuickReference.md)

## Test Files

1. **TestConsoleZshFeatures.cpp** - C++ unit tests using Google Test framework
2. **RunConsoleTests.sh** - Bash script for automated testing
3. **TestConsoleZsh.py** - Python script for comprehensive automated testing
4. **InteractiveTests.txt** - Manual test cases for interactive testing

## Features Tested

### Basic History Expansion
- `!!` - Repeat last command
- `!n` - Execute nth command from history (1-based)
- `!-n` - Execute nth command from end
- `!string` - Execute last command starting with string

### Word Designators
- `:0` - The command word
- `:^` - First argument (same as `:1`)
- `:$` - Last argument
- `:*` - All arguments (words 1 through end)
- `:n` - The nth word (0-based)
- `:n-m` - Words n through m
- `:n*` - Words n through end

### Complex Features
- `!-3:4*` - Words 4 through end from 3 commands ago
- History expansion within commands: `!! * 2`
- Multiple history references: `!-2 + !-1`

### Shell Commands
- Backtick execution: `` `echo hello` ``
- Embedded shell commands: `print "`date`"`

### Special Behaviors
- `$` prefix disables all zsh expansion
- Quoted strings are preserved in word splitting
- Error handling for non-existent history

## Running Tests

### C++ Unit Tests
```bash
# Build the tests
cd /home/xian/local/KAI
make TestConsole

# Run the tests
./Bin/Test/TestConsole
```

### Automated Shell Tests
```bash
cd /home/xian/local/KAI/Test/Console
./RunConsoleTests.sh
```

### Python Tests
```bash
cd /home/xian/local/KAI/Test/Console
./TestConsoleZsh.py
```

### Interactive Testing
```bash
# Start the console
/home/xian/local/KAI/Bin/Console

# Copy and paste commands from InteractiveTests.txt
```

## Test Coverage

The test suite covers:
1. All basic history operators (!!, !n, !-n, !string)
2. All word designators (:0, :^, :$, :*, :n, :n-m, :n*)
3. Complex multi-part expressions
4. Shell command integration
5. Edge cases and error conditions
6. Integration with Pi and Rho languages
7. Window App compatibility

## Expected Behavior

### Success Cases
- History expansions show the expanded command with "=> " prefix
- Word designators extract the correct portions of commands
- Shell commands execute and return output
- $ prefix prevents any expansion

### Error Cases
- Non-existent history references show "No matching command in history"
- Invalid word designators return empty strings
- Shell command failures show error messages

## Adding New Tests

To add new tests:

1. **C++ Tests**: Add new TEST_F cases to TestConsoleZshFeatures.cpp
2. **Shell Tests**: Add new run_test calls to RunConsoleTests.sh
3. **Python Tests**: Add new self.test() calls to TestConsoleZsh.py
4. **Interactive Tests**: Add new test cases to InteractiveTests.txt

## Known Limitations

1. History modifiers (:h, :t, :r, :e, :p, :s/old/new/) are not yet implemented
2. Command history is not persisted between sessions
3. Some complex nested expansions may not work as expected