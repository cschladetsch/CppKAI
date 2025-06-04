# Console Quick Start Guide

## Starting the Console

```bash
$ cd /path/to/KAI
$ ./Bin/Console
```

## Basic Commands

```console
Pi λ # Basic arithmetic - Pi uses stack-based operations
Pi λ 2 3 +
[0]: 5

Pi λ 10 20 *
[0]: 200

Pi λ # Clear the stack
Pi λ clear
```

## Shell Commands - Two Ways!

```console
Pi λ # Method 1: Use backticks for shell commands
Pi λ `ls`
(shows directory listing)

Pi λ `pwd`
/home/user/KAI

Pi λ `date`
Tue Jun 4 21:02:15 PST 2024

Pi λ # Method 2: Use $ prefix for shell commands
Pi λ $ ls
(shows directory listing)

Pi λ $ pwd
/home/user/KAI

Pi λ $ echo "Hello from shell"
Hello from shell

Pi λ # WRONG: This won't work
Pi λ ls      # This looks for a Pi function called 'ls'
```

## History Features

```console
Pi λ # Execute a command
Pi λ 5 5 +
[0]: 10

Pi λ # Repeat last command
Pi λ !!
=> 5 5 +
[0]: 10

Pi λ # Fix typos quickly
Pi λ print "Helo"
[0]: "Helo"

Pi λ ^elo^ello
=> print "Hello"
[0]: "Hello"
```

## The $ Prefix - Quick Shell Commands

```console
Pi λ # Execute shell commands quickly with $
Pi λ $ date
Tue Jun  4 21:15:30 PST 2024

Pi λ $ ls *.txt
file1.txt
file2.txt
readme.txt

Pi λ $ grep -c "function" *.cpp
main.cpp:5
utils.cpp:12

Pi λ # Compare with backtick method
Pi λ `date`
Tue Jun  4 21:15:45 PST 2024

Pi λ # Both methods work the same!
```

## Common Mistakes and Solutions

### Mistake 1: Trying to use shell commands directly
```console
WRONG:  Pi λ ls
RIGHT:  Pi λ `ls`
```

### Mistake 2: Mixing Pi and shell syntax
```console
WRONG:  Pi λ print $ ls     # Can't mix Pi and shell
RIGHT:  Pi λ print "`ls`"   # Use backticks to embed
RIGHT:  Pi λ $ ls           # Or just shell command
```

### Mistake 3: Forgetting Pi is stack-based
```console
WRONG:  Pi λ 2 + 3      # Error: not enough on stack
RIGHT:  Pi λ 2 3 +      # Pushes 2, then 3, then adds
```

## Essential Commands

1. **Exit**: `exit` or Ctrl+D
2. **Clear stack**: `clear`
3. **Show help**: `help`
4. **Shell command (method 1)**: `` `command` `` (in backticks)
5. **Shell command (method 2)**: `$ command` (with $ prefix)
6. **Repeat last**: `!!`
7. **Fix typo**: `^old^new`

## Quick Examples

```console
Pi λ # Define a function
Pi λ fun double { 2 * }

Pi λ # Use it
Pi λ 5 double
[0]: 10

Pi λ # Work with lists
Pi λ [1 2 3 4 5] { double } map
[0]: [2 4 6 8 10]

Pi λ # Shell integration
Pi λ files = "`ls *.txt | wc -l`"
Pi λ print "Found " files + " text files" +
[0]: "Found 5 text files"
```

Remember: Pi is a stack-based language, use backticks for shell commands!