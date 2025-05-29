# KAI Console

## Overview

The KAI Console is a cross-platform, colored REPL (Read-Eval-Print-Loop) interface for the KAI language system. It provides an interactive environment for executing Pi, Rho, and Tau language code with support for shell command integration and automatic stack visualization.

## Features

### Language Support
- **Pi Language**: Stack-based postfix notation (default)
- **Rho Language**: Infix notation with assignment support
- **Tau Language**: Advanced language features
- Language switching using numeric prefix (e.g., `2 rho` switches to Rho)

### Shell Command Integration

The Console supports seamless integration with shell commands through backtick syntax:

#### Full-line Shell Commands
Execute shell commands by starting a line with a backtick:
```
Pi λ `pwd
/home/user/project

Pi λ `ls
file1.txt
file2.cpp
directory/

Pi λ `echo "Hello from shell!"
Hello from shell!
```

The closing backtick is optional for full-line commands:
```
Pi λ `date
Fri May 30 00:20:09 AEST 2025
```

#### Embedded Shell Commands
Embed shell command output directly into expressions using backticks:
```
Pi λ 10 `echo 5` +
[0]: 15

Pi λ 1 `echo 2` + 3 ==
[0]: true

Pi λ `echo 100` `echo 2` /
[0]: 50
```

This feature works across all languages:
```
Rho λ result = `echo 42`
Rho λ value = 10 + `echo 5`
```

#### Error Handling
- Empty shell commands return an error message
- Failed commands show the exit code
- Command output is trimmed of leading/trailing whitespace

### Automatic Stack Display

After each command execution, the Console automatically displays the entire stack with colored output:

```
Pi λ 1 2 3
[0]: 1
[1]: 2
[2]: 3
```

Stack elements are color-coded by type:
- **Integers**: Yellow
- **Floats**: Magenta
- **Strings**: Green
- **Other types**: Gray

The stack display format is `[index]: value` where index 0 is the bottom of the stack.

### Built-in Commands

#### print
Output values to the console:
```
Pi λ "Hello World" print
Hello World

Pi λ 42 print
42
```

#### Stack Operations
- `.` - Print top of stack
- `dup` - Duplicate top of stack
- `drop` - Remove top of stack
- `swap` - Swap top two elements
- `clear` - Clear the stack

## Examples

### Basic Arithmetic with Shell Commands
```
Pi λ `echo 10` `echo 20` +
[0]: 30

Pi λ 5 `echo 3` * 2 +
[0]: 17
```

### File Operations
```
Pi λ `ls | wc -l`
[0]: 10

Pi λ `cat data.txt | head -1`
[0]: "First line of file"
```

### System Information
```
Pi λ `whoami`
[0]: "username"

Pi λ `uname -s`
[0]: "Linux"
```

### Complex Examples
```
# Calculate with dynamic values
Pi λ `echo $RANDOM` 100 % 
[0]: 42

# Process command output
Pi λ `ls -1 | wc -l` " files in directory" +
[0]: "10 files in directory"

# Conditional operations based on shell commands
Rho λ file_count = `ls | wc -l`
Rho λ if (file_count > 5) { "Many files" print }
```

## Implementation Details

### Shell Command Processing
1. Lines starting with ` are identified as shell commands
2. For embedded commands, regex pattern `` `([^`]+)` `` finds all backtick-enclosed commands
3. Commands are executed using `popen()` with read mode
4. Command output replaces the backtick expression before language processing
5. The shell expansion happens as a preprocessing step, making it language-agnostic

### Stack Display Implementation
- The `ShowColoredStack()` method is called after each command execution
- Stack elements are displayed from bottom (index 0) to top
- Type information is used to determine the appropriate color
- The rang library provides cross-platform color support

## Configuration

The Console reads configuration from various sources:
- Command-line arguments
- Configuration files (JSON format)
- Environment variables

## Security Considerations

- Shell commands are executed with the same privileges as the Console process
- No input sanitization is performed on shell commands
- Users should be cautious when executing shell commands from untrusted sources
- The `popen()` function is used, which invokes the system shell

## Future Enhancements

Potential improvements for the shell command feature:
- Command history for shell commands
- Tab completion for shell commands
- Configurable shell command prefix
- Shell command output caching
- Async shell command execution
- Pipeline support between KAI and shell