# KAI Console

## Overview

The KAI Console is a professional, cross-platform REPL (Read-Eval-Print-Loop) interface for the KAI language system. It provides a comprehensive interactive environment for executing Pi, Rho, and Tau language code with advanced features including command-line argument parsing, interactive help system, persistent history, shell integration, and enhanced user experience.

## Getting Started

### Command Line Usage

```bash
# Show help and available options
$ ./Console --help
$ ./Console --version

# Interactive modes
$ ./Console                    # Interactive Pi mode (default)
$ ./Console -l rho             # Interactive Rho mode
$ ./Console -l pi              # Interactive Pi mode (explicit)

# Script execution
$ ./Console script.pi          # Execute Pi script
$ ./Console script.rho         # Execute Rho script
$ ./Console -l rho script.txt  # Execute as Rho regardless of extension

# Advanced options
$ ./Console -t 2 script.pi     # Execute with trace level 2
$ ./Console --verbose script.rho    # Enable verbose output
$ ./Console -i script.pi       # Force interactive mode after script
$ ./Console -n script.pi       # Non-interactive mode (no REPL)
```

### Command Line Options

- `-h, --help` - Show comprehensive help message
- `-v, --version` - Show version and build information
- `-l, --language LANG` - Set default language (pi, rho)
- `-t, --trace LEVEL` - Set trace level (0-5)
- `-i, --interactive` - Force interactive mode
- `-n, --non-interactive` - Non-interactive mode
- `--verbose` - Enable verbose output

## Interactive Features

### Built-in Help System

The Console includes a comprehensive help system with contextual topics:

```
π help
KAI Console Help

Available help topics:
  help basics     - Basic usage and commands
  help history    - History and command expansion
  help stack      - Inspect and manipulate the data stack
  help shell      - Shell integration
  help languages  - Pi and Rho language features

Language-specific help:
  help pi         - Pi language reference
  help rho        - Rho language reference
```

**Help Topics:**
- `help` or `help basics` - Getting started guide with examples
- `help history` - Command history and expansion features
- `help stack` - Stack display and common Pi stack operations
- `help shell` - Shell integration documentation
- `help languages` - Overview of Pi and Rho languages
- `help pi` - Comprehensive Pi language reference
- `help rho` - Comprehensive Rho language reference

### Built-in Commands

The Console provides several built-in commands for enhanced usability:

- `help [topic]` - Show help (optionally for specific topic)
- `clear`, `cls` - Clear screen
- `exit`, `quit` - Exit console
- `pi`, `rho` - Switch language mode
- `history` - Show command history
- `stack` - Show current stack contents
- `$ <command>` - Execute shell command (when shell syntax enabled)

### Persistent Command History

Commands are automatically saved per language to `~/.kai/pi.history` and
`~/.kai/rho.history`, and persist across sessions. Prompts show only the active
language symbol; use `history` to see the stable entry numbers referenced by
`!n`:

```
π history
Command History:
  1: 2 3 +
  2: help pi
  3: x = 42; y = x * 2

π !!         # Repeat last command
π !2         # Repeat command 2
π !help      # Find last command starting with "help"
```

**History Features:**
- Automatic per-language saving to `~/.kai/pi.history` and `~/.kai/rho.history`
- Duplicate elimination
- Size limit management (1000 commands)
- History expansion: `!!`, `!n`, `!string`, `!?pattern`
- Word designators: `!^`, `!$`, `!*`

### Enhanced User Experience

- **Professional startup banner** with version and build information
- **Color-coded output** with orange-colored stack indices
- **Context-sensitive prompts** showing current language
- **User-friendly error messages** with suggestions
- **Multi-line input support** for complex structures
- **Automatic stack display** after each command

## Language Support

### Pi Language (Stack-based)
Default mode with postfix notation:
```
π 2 3 +
[0]: 5

π dup *
[0]: 25
```

### Rho Language (Infix)
C-like syntax with variables and control structures:
```
ρ x = 42
[0]: 42

ρ if (x > 40) { "Large" } else { "Small" }
[0]: "Large"
```

### Language Switching
Switch between languages using built-in commands:
```
π rho
Switched to Rho language mode

ρ pi
Switched to Pi language mode
```

## Shell Command Integration

*Note: Shell features require building with `-DENABLE_SHELL_SYNTAX=ON` for security.*

### Standalone Shell Commands with $ Prefix
Execute shell commands by starting a line with `$`:
```
π $ pwd
/home/user/project

π $ ls -la
total 48
drwxr-xr-x  12 user  user   384 Jun  5 10:00 .

π $ echo "Hello from shell!"
Hello from shell!
```

### Embedded Shell Commands with Backticks
Embed shell command output within expressions using backticks `` `command` ``:
```
π 1 `echo 2` + 3 ==
[0]: true

π `echo 10` `echo 20` +
[0]: 30

π "User: " `whoami` +
[0]: "User: username"
```

### Shell Mode
Toggle into shell mode where all commands are executed as shell commands:
```
π sh
Entering shell mode. Type 'exit' to return to Pi mode.
$ ls -la
total 48
$ exit
Exited shell mode. Back to Pi mode.
```

### Shell Integration Features
- **Two modes**: Standalone (`$`) and embedded (`` `command` ``)
- **Works in all contexts**: Interactive REPL, piped input, and file execution
- **Language agnostic**: Works in Pi, Rho, and Tau modes
- **Command history**: Shell commands are added to the command history
- **Exit code display**: Non-zero exit codes are shown for `$` commands
- **Error handling**: Empty commands and failed commands handled gracefully

## Stack Display and Visualization

### Automatic Stack Display

After each command execution, the Console automatically displays the entire
stack. The top value is printed first and the bottom value, `[0]`, is printed on
the bottom line:

```
π 1 2 3
[2]: 3
[1]: 2
[0]: 1
```

### Color Coding
Stack elements are color-coded by type for better readability:
- **Stack numbers**: Orange/bright yellow for better visibility
- **Integers**: Yellow
- **Floats**: Neutral/default value color
- **Strings**: Green (with quotes)
- **Other types**: Gray

The stack display format is `[index]: value`; index 0 is the bottom of the stack
and therefore appears on the final stack line.

### Stack Operations
Built-in stack manipulation commands:
- `stack` - Explicitly show current stack contents
- `dup` - Duplicate top of stack
- `drop` - Remove top of stack
- `swap` - Swap top two elements
- `over` - Copy second element to top

`clear` and `cls` clear the terminal screen; they do not mutate the data stack.
The NodeGLM Debug panel's handle-targeted `clear` action clears both stacks of
the explicitly selected Executor.

## Executor Inspection and Logging

The Console can emit a private, bounded snapshot used by NodeGLM's Debug and
Tree panels. A snapshot enumerates every live `Executor` in the Registry by
handle. For each Executor it reports that Executor's own `Tree*`, root, scope,
data/context stack sizes, and up to 1000 child nodes at a maximum depth of 32.
Consumers must select an Executor explicitly; neither Tree nor Debug assumes a
single global Executor.

Debugger actions are handle-targeted and limited to `step`, `continue`, `stack`,
and `clear`. Console startup initializes KAI's native `Logger`. Tree snapshot
lifecycle, debugger attachment/actions, invalid handles, and snapshot failures
are logged through `Logger`; the snapshot records themselves remain on stdout
because they are a machine transport protocol.

## Advanced Examples

### Basic Arithmetic with Shell Commands
```
π `echo 10` `echo 20` +
[0]: 30

π 5 `echo 3` * 2 +
[0]: 17
```

### File Operations
```
π `ls | wc -l`
[0]: 10

π `cat data.txt | head -1`
[0]: "First line of file"
```

### System Information
```
π `whoami`
[0]: "username"

π `uname -s`
[0]: "Linux"
```

### Complex Rho Examples
```
# Calculate with dynamic values
ρ file_count = `ls | wc -l`
ρ if (file_count > 5) {
...     print("Many files: " + file_count)
... }
Many files: 10

# Function definitions with shell integration
ρ fun get_temp() {
...     return `cat /sys/class/thermal/thermal_zone0/temp`
... }
ρ temp = get_temp(); print("Temperature: " + temp)
```

## Configuration and Customization

### History Configuration
The Console automatically manages command history with these defaults:
- **History files**: `~/.kai/pi.history` and `~/.kai/rho.history` (one per language)
- **Maximum entries**: 1000 commands
- **Duplicate handling**: Automatic elimination
- **Persistence**: Automatic save on exit, load on startup

### Display Configuration
- **Stack display**: Enabled by default after each command
- **Color output**: Enabled by default (automatically detected)
- **Prompt format**: Shows current language with lambda symbol
- **Trace levels**: 0-5 (configurable via `-t` option)

### Build-time Configuration
```bash
# Enable shell syntax (disabled by default for security)
cmake .. -DENABLE_SHELL_SYNTAX=ON

# Build with different trace levels
cmake .. -DDEFAULT_TRACE_LEVEL=2
```

## Security Considerations

### Shell Command Security
- **Default behavior**: Shell commands are disabled by default
- **Explicit enabling**: Must be enabled at build time with `-DENABLE_SHELL_SYNTAX=ON`
- **Privilege level**: Shell commands execute with the same privileges as the Console process
- **Input validation**: No sanitization is performed on shell commands
- **Execution method**: Uses `popen()` which invokes the system shell

### Best Practices
- Only enable shell syntax in trusted environments
- Be cautious when executing shell commands from untrusted sources
- Consider running the Console in a sandboxed environment for enhanced security
- Regular users should use the Console without shell syntax enabled

## Installation

### Automatic Installation
The Console automatically copies itself to `~/bin/Console` if the directory exists, making it available system-wide.

### Manual Installation
```bash
# Copy to user bin directory
cp ./Bin/Console ~/bin/

# Or copy to system bin directory (requires sudo)
sudo cp ./Bin/Console /usr/local/bin/
```

## Implementation Details

### Architecture
- **Built on KAI Core**: Uses the Registry, Executor, and Memory systems
- **Multi-language support**: Integrates Pi, Rho, and Tau translators
- **Modular design**: Separate components for history, help, shell integration
- **Cross-platform**: Uses rang library for consistent color output

### Performance
- **Incremental garbage collection**: Smooth performance without GC pauses
- **Efficient history management**: Automatic size limiting and duplicate removal
- **Fast startup**: Optimized initialization and history loading
- **Memory efficient**: Smart pointer usage and proper resource management

### Error Handling
- **Graceful degradation**: Continues operation even if optional features fail
- **User-friendly messages**: Clear error descriptions with suggestions
- **Exception safety**: Proper exception handling throughout
- **Recovery mechanisms**: Continues operation after errors when possible

## Future Enhancements

Potential improvements being considered:
- **Tab completion**: Auto-completion for commands and functions
- **Syntax highlighting**: Real-time syntax highlighting for input
- **Configuration files**: JSON-based configuration with user customization
- **Plugin system**: Extensible architecture for custom commands
- **Remote REPL**: Network-accessible REPL for distributed debugging
- **IDE integration**: Enhanced integration with development environments
- **Performance profiling**: Built-in profiling and performance analysis tools
- **Advanced debugging**: Step-through debugging and breakpoint support
