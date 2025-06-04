# Console Shell Mode

## Overview

The KAI Console now supports a "shell mode" where you can execute shell commands naturally without prefixes. Simply type `sh`, `bash`, or `zsh` to toggle into shell mode.

## Demo Session

```console
Pi λ # Let's do some Pi calculations first
Pi λ 10 20 +
[0]: 30

Pi λ # Now switch to shell mode
Pi λ sh
Entering shell mode. Type 'exit' to return to Pi mode.

Bash λ # Notice the prompt changed to Bash!
Bash λ pwd
/home/user/KAI

Bash λ ls -la
total 28
drwxr-xr-x  7 user user 4096 Jun  4 21:30 .
drwxr-xr-x 12 user user 4096 Jun  4 20:00 ..
-rw-r--r--  1 user user 1234 Jun  4 21:25 README.md
drwxr-xr-x  3 user user 4096 Jun  4 21:00 Source

Bash λ # History expansion still works!
Bash λ echo "Hello from shell"
Hello from shell

Bash λ !!
=> echo "Hello from shell"
Hello from shell

Bash λ # Quick substitution works too
Bash λ ^Hello^Greetings
=> echo "Greetings from shell"
Greetings from shell

Bash λ # Complex shell commands work naturally
Bash λ find . -name "*.cpp" | head -3
./Source/main.cpp
./Source/utils.cpp
./Source/parser.cpp

Bash λ # Use all shell features
Bash λ for i in 1 2 3; do echo "Number: $i"; done
Number: 1
Number: 2
Number: 3

Bash λ # Exit back to Pi mode
Bash λ exit
Exited shell mode. Back to Pi mode.

Pi λ # We're back in Pi mode
Pi λ 5 5 *
[0]: 25
```

## Features in Shell Mode

1. **Natural Shell Commands**: No need for `$` prefix or backticks
2. **Changed Prompt**: Shows "Bash λ" instead of "Pi λ"
3. **History Expansion**: All zsh-like features still work
4. **Full Shell Syntax**: Pipes, loops, redirects all work
5. **Easy Toggle**: Type `sh` to enter, `exit` to leave

## Comparison of Methods

```console
# Method 1: Backticks (from Pi mode)
Pi λ `ls -la`

# Method 2: Dollar prefix (from Pi mode)
Pi λ $ ls -la

# Method 3: Shell mode (most natural)
Pi λ sh
Bash λ ls -la
Bash λ cd /tmp
Bash λ pwd
/tmp
Bash λ exit
Pi λ
```

## Advanced Usage

### Mixing Modes Efficiently

```console
Pi λ # Calculate something in Pi
Pi λ data = [10 20 30 40 50]
Pi λ data sum
[0]: 150

Pi λ # Switch to shell to save result
Pi λ sh
Bash λ echo "Sum is 150" > result.txt
Bash λ cat result.txt
Sum is 150

Bash λ # Do more shell work
Bash λ wc -l *.txt
5 result.txt
12 data.txt
17 total

Bash λ # Back to Pi for more calculations
Bash λ exit
Pi λ 150 17 /
[0]: 8.82353
```

### History Across Modes

```console
Pi λ calculate_something complex
Pi λ sh
Bash λ # Can still reference Pi history
Bash λ echo "Last Pi command was: !-2"
=> echo "Last Pi command was: calculate_something complex"
Last Pi command was: calculate_something complex

Bash λ grep something file.txt
Bash λ exit
Pi λ # Can reference shell commands in history
Pi λ !grep
=> grep something file.txt
```

## Tips

1. **Quick Shell Tasks**: Use `sh` for extended shell work
2. **One-Liners**: Use `$ command` for single commands
3. **Embedded Results**: Use backticks to embed shell output in Pi
4. **History**: All commands across modes are saved in history

Shell mode makes the Console a complete environment for both Pi programming and system administration!