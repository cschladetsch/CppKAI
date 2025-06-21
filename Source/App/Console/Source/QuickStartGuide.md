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
Pi λ help
KAI Console Help

Available help topics:
  help basics     - Basic usage and commands
  help history    - History and command expansion  
  help shell      - Shell integration
  help languages  - Pi and Rho language features

Language-specific help:
  help pi         - Pi language reference
  help rho        - Rho language reference

Pi λ help pi       # Detailed Pi language help
Pi λ help basics   # Getting started guide
```

## Built-in Commands

```console
Pi λ help          # Show help topics
Pi λ clear         # Clear screen (or 'cls')
Pi λ history       # Show command history
Pi λ stack         # Show current stack
Pi λ pi            # Switch to Pi mode
Pi λ rho           # Switch to Rho mode
Pi λ exit          # Exit console (or 'quit')
```

## Basic Pi Operations

```console
Pi λ # Basic arithmetic - Pi uses stack-based operations
Pi λ 2 3 +
[0]: 5

Pi λ 10 20 *
[0]: 200

Pi λ # Stack operations
Pi λ dup           # Duplicate top
Pi λ swap          # Swap top two
Pi λ drop          # Remove top
Pi λ clear         # Clear stack
```

## Language Switching

```console
Pi λ rho           # Switch to Rho mode
Switched to Rho language mode

Rho λ x = 42       # Now using infix syntax
[0]: 42

Rho λ y = x * 2
[0]: 84

Rho λ pi           # Switch back to Pi
Switched to Pi language mode

Pi λ 2 3 +         # Back to postfix
[0]: 5
```

## Shell Commands (When Enabled)

*Note: Shell features require building with `-DENABLE_SHELL_SYNTAX=ON`*

```console
Pi λ # Method 1: Use $ prefix for shell commands
Pi λ $ pwd
/home/user/KAI

Pi λ $ ls *.txt
file1.txt
file2.txt

Pi λ $ echo "Hello from shell"
Hello from shell

Pi λ # Method 2: Use backticks to embed in expressions
Pi λ `echo 10` `echo 20` +
[0]: 30

Pi λ "Files: " `ls | wc -l` +
[0]: "Files: 5"
```

## Persistent History

Commands are automatically saved and restored:

```console
Pi λ 2 3 +         # Execute a command
[0]: 5

Pi λ history       # Show command history
Command History:
  1: 2 3 +

Pi λ !!            # Repeat last command
[0]: 5

Pi λ !1            # Repeat command 1
[0]: 5

# History is saved to ~/.kai_history automatically
```

## Advanced History Features

```console
Pi λ # Execute several commands
Pi λ 5 5 +
[0]: 10

Pi λ print "Hello"
Hello

Pi λ 10 20 *
[0]: 200

Pi λ # Various history expansions
Pi λ !!            # Repeat last command
Pi λ !p            # Find last command starting with 'p'
Pi λ !?Hello       # Find last command containing 'Hello'

Pi λ # Quick substitution
Pi λ print "Helo"
Hello

Pi λ ^elo^ello     # Fix typo in last command
=> print "Hello"
Hello
```

## Rho Mode Examples

```console
# Switch to Rho for infix syntax
Pi λ rho
Switched to Rho language mode

Rho λ x = 42
[0]: 42

Rho λ if (x > 40) {
...     print("Large number: " + x)
... }
Large number: 42

Rho λ fun square(n) {
...     return n * n
... }

Rho λ result = square(7)
[0]: 49

# Shell integration in Rho (when enabled)
Rho λ file_count = `ls | wc -l`
Rho λ print("Found " + file_count + " files")
```

## Color-Coded Stack Display

The stack automatically displays with color coding:

```console
Pi λ 42 "hello" 3.14
[0]: 42        # Numbers in yellow
[1]: "hello"   # Strings in green (with quotes)  
[2]: 3.14      # Floats in magenta

# Stack numbers are orange for better visibility
```

## Pro Tips

### 1. Use the help system extensively
```console
Pi λ help basics   # Learn Console basics
Pi λ help pi       # Pi language reference
Pi λ help rho      # Rho language reference
Pi λ help shell    # Shell integration
Pi λ help history  # History features
```

### 2. Leverage persistent history
- Commands save to `~/.kai_history` automatically
- Use `!!`, `!n`, `!string` for quick recalls
- History persists across sessions

### 3. Take advantage of built-in commands
```console
Pi λ stack         # Show stack anytime
Pi λ clear         # Clear screen
Pi λ history       # Review what you've done
```

### 4. Switch languages as needed
```console
Pi λ rho           # For familiar infix syntax
Rho λ pi           # For stack manipulation
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
WRONG:  Pi λ ls          # Undefined Pi function
RIGHT:  Build with -DENABLE_SHELL_SYNTAX=ON first
        Pi λ $ ls        # Then use shell commands
```

### Mistake 2: Forgetting Pi is stack-based
```console
WRONG:  Pi λ 2 + 3       # Error: not enough on stack
RIGHT:  Pi λ 2 3 +       # Pushes 2, then 3, then adds
```

### Mistake 3: Not using the help system
```console
WRONG:  Struggling with syntax
RIGHT:  Pi λ help pi     # Get comprehensive reference
```

### Mistake 4: Not leveraging history
```console
SLOW:   Retyping long commands
FAST:   Pi λ !!          # Repeat last
        Pi λ !long       # Find command starting with 'long'
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