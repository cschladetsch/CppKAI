# Console Quick Start Guide

## Starting the Console

```bash
# Basic startup
$ ./Console                    # Interactive Pi mode (default)
$ ./Console --help             # Show all options
$ ./Console --version          # Show version info

# Different modes
$ ./Console -l rho             # Start in Rho mode
$ ./Console script.pi          # Execute a Pi script
$ ./Console -t 2 -l rho        # Rho mode with trace level 2

# If installed to ~/bin (automatic during build):
$ Console --help               # Available system-wide
```

## Getting Help

The Console includes a comprehensive help system:

```console
π help
KAI Console Help

Available help topics:
  help basics     - Basic usage and commands
  help history    - History and command expansion  
  help shell      - Shell integration
  help languages  - Pi and Rho language features

Language-specific help:
  help pi         - Pi language reference
  help rho        - Rho language reference

π help pi       # Detailed Pi language help
π help basics   # Getting started guide
```

## Built-in Commands

```console
π help          # Show help topics
π clear         # Clear screen (or 'cls')
π history       # Show command history
π stack         # Show current stack
π pi            # Switch to Pi mode
π rho           # Switch to Rho mode
π exit          # Exit console (or 'quit')
```

## Basic Pi Operations

```console
π # Basic arithmetic - Pi uses stack-based operations
π 2 3 +
[0]: 5

π 10 20 *
[0]: 200

π # Stack operations
π dup           # Duplicate top
π swap          # Swap top two
π drop          # Remove top
π clear         # Clear the terminal screen (the data stack is unchanged)
```

## Language Switching

```console
π rho           # Switch to Rho mode
Switched to Rho language mode

ρ x = 42       # Now using infix syntax
[0]: 42

ρ y = x * 2
[0]: 84

ρ pi           # Switch back to Pi
Switched to Pi language mode

π 2 3 +         # Back to postfix
[0]: 5
```

## Shell Commands (When Enabled)

*Note: Shell features require building with `-DENABLE_SHELL_SYNTAX=ON`*

```console
π # Method 1: Use $ prefix for shell commands
π $ pwd
/home/user/KAI

π $ ls *.txt
file1.txt
file2.txt

π $ echo "Hello from shell"
Hello from shell

π # Method 2: Use backticks to embed in expressions
π `echo 10` `echo 20` +
[0]: 30

π "Files: " `ls | wc -l` +
[0]: "Files: 5"
```

## Persistent History

Commands are automatically saved and restored:

```console
π 2 3 +         # Execute a command
[0]: 5

π history       # Show command history
Command History:
  1: 2 3 +

π !!            # Repeat last command
[0]: 5

π !1            # Repeat command 1
[0]: 5

# History is saved per language to ~/.kai/pi.history and ~/.kai/rho.history automatically
```

## Advanced History Features

```console
π # Execute several commands
π 5 5 +
[0]: 10

π print "Hello"
Hello

π 10 20 *
[0]: 200

π # Various history expansions
π !!            # Repeat last command
π !p            # Find last command starting with 'p'
π !?Hello       # Find last command containing 'Hello'

π # Quick substitution
π print "Helo"
Hello

π ^elo^ello     # Fix typo in last command
=> print "Hello"
Hello
```

## Rho Mode Examples

```console
# Switch to Rho for infix syntax
π rho
Switched to Rho language mode

ρ x = 42
[0]: 42

ρ if (x > 40) {
...     print("Large number: " + x)
... }
Large number: 42

ρ fun square(n) {
...     return n * n
... }

ρ result = square(7)
[0]: 49

# Shell integration in Rho (when enabled)
ρ file_count = `ls | wc -l`
ρ print("Found " + file_count + " files")
```

## Color-Coded Stack Display

The stack automatically displays with color coding:

```console
π 42 "hello" 3.14
[2]: 3.14      # Floats use the neutral value color
[1]: "hello"   # Strings in green (with quotes)
[0]: 42        # The bottom value is printed last

# Stack indices are orange; [0] is the physical bottom line
```

## Pro Tips

### 1. Use the help system extensively
```console
π help basics   # Learn Console basics
π help pi       # Pi language reference
π help rho      # Rho language reference
π help shell    # Shell integration
π help history  # History features
π help stack    # Stack display and operations
```

### 2. Leverage persistent history
- Commands save per language to `~/.kai/pi.history` and `~/.kai/rho.history` automatically
- Use `!!`, `!n`, `!string` for quick recalls
- History persists across sessions

### 3. Take advantage of built-in commands
```console
π stack         # Show stack anytime
π clear         # Clear screen
π history       # Review what you've done
```

### 4. Switch languages as needed
```console
π rho           # For familiar infix syntax
ρ pi           # For stack manipulation
```

### 5. Use command-line options
```bash
$ Console -l rho                    # Start in preferred language
$ Console --verbose script.pi      # Debug script execution
$ Console -t 3 complex_script.rho  # High trace level for debugging
```

## Common Mistakes and Solutions

### Mistake 1: Trying shell commands without enabling them
```console
WRONG:  π ls          # Undefined Pi function
RIGHT:  Build with -DENABLE_SHELL_SYNTAX=ON first
        π $ ls        # Then use shell commands
```

### Mistake 2: Forgetting Pi is stack-based
```console
WRONG:  π 2 + 3       # Error: not enough on stack
RIGHT:  π 2 3 +       # Pushes 2, then 3, then adds
```

### Mistake 3: Not using the help system
```console
WRONG:  Struggling with syntax
RIGHT:  π help pi     # Get comprehensive reference
```

### Mistake 4: Not leveraging history
```console
SLOW:   Retyping long commands
FAST:   π !!          # Repeat last
        π !long       # Find command starting with 'long'
```

## Essential Commands Summary

1. **Help**: `help`, `help pi`, `help rho`, `help basics`
2. **Navigation**: `clear`, `history`, `stack`
3. **Language**: `pi`, `rho`
4. **Exit**: `exit`, `quit`, or Ctrl+D
5. **History**: `!!`, `!n`, `!string`
6. **Shell**: `$ command` (when enabled)
7. **Stack**: `dup`, `swap`, `drop`, `clear`

## Quick Reference Card

```
Stack Operations:    Help System:         Language Switch:
  2 3 +               help                 pi
  dup swap drop       help pi              rho
  clear               help rho
                      help basics
History:              
  !!                 Built-ins:           Shell (if enabled):
  !n                  clear/cls            $ command
  !string             history              `command`
  ^old^new            stack
                      exit/quit
```

Remember: Start with `help` to discover all features!
