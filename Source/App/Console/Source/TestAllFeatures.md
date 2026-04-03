# Console Feature Test Checklist

## Shell Command Features

### 1. Backtick Method
```console
Pi λ `pwd`
Pi λ `ls -la`
Pi λ `echo "Hello from backticks"`
Pi λ print "Directory: `pwd`"
```
✓ Should execute shell commands and return results

### 2. Dollar Prefix Method  
```console
Pi λ $ pwd
Pi λ $ ls -la
Pi λ $ echo "Hello from dollar prefix"
Pi λ $ date
```
✓ Should execute shell commands directly

### 3. Shell Mode
```console
Pi λ sh
Bash λ pwd
Bash λ ls *.txt
Bash λ for i in 1 2 3; do echo $i; done
Bash λ exit
Pi λ
```
✓ Prompt should change to "Bash λ"
✓ All commands execute as shell
✓ 'exit' returns to Pi mode

## History Expansion Features

### Basic History
```console
Pi λ 10 20 +
Pi λ !!                    # Repeat last command
Pi λ print "test"
Pi λ !-2                   # Two commands ago
Pi λ !1                    # First command
Pi λ !print                # Last command starting with 'print'
```

### Quick Substitution
```console
Pi λ print "Helo World"
Pi λ ^elo^ello             # Fix typo
Pi λ $ ls fiel.txt
Pi λ ^fiel^file            # Works in shell commands too
```

### Word Designators
```console
Pi λ process input.txt output.json --verbose
Pi λ !!:0                  # 'process'
Pi λ !!:1                  # 'input.txt'
Pi λ !!:$                  # '--verbose'
Pi λ !!:*                  # All arguments
Pi λ !!:1-2                # Range
Pi λ !!:2*                 # From 2 to end
```

### Advanced Features
```console
Pi λ !$ # Last argument of previous command
Pi λ !^ # First argument of previous command
Pi λ !?json? # Search for 'json' anywhere
Pi λ echo test !#:0        # Current command reference
```

### Path Modifiers
```console
Pi λ file=/path/to/file.txt
Pi λ print file
Pi λ !!:$:h                # /path/to
Pi λ !!:$:t                # file.txt
Pi λ !!:$:r                # /path/to/file
Pi λ !!:$:e                # txt
```

### Case and Quote Modifiers
```console
Pi λ word="hello"
Pi λ print word
Pi λ !!:$:u                # "HELLO"
Pi λ !!:$:l                # "hello"
Pi λ !!:$:q                # Quote it
```

### Substitution Modifiers
```console
Pi λ echo "apple banana apple"
Pi λ !!:s/apple/orange/    # First occurrence
Pi λ !-2:gs/apple/orange/  # All occurrences
Pi λ !!:p                  # Preview without executing
```

## Shell Mode with History
```console
Pi λ sh
Bash λ echo "test"
Bash λ !!                  # History works in shell mode
Bash λ ^test^hello         # Substitution works
Bash λ !-1:u               # Modifiers work
Bash λ exit
```

## Integration Examples
```console
Pi λ 100 25 /
[0]: 4
Pi λ $ echo "Result: 4" > result.txt
Pi λ `cat result.txt`
Result: 4
Pi λ sh
Bash λ ls *.txt
result.txt
Bash λ rm result.txt
Bash λ exit
Pi λ
```

## Error Cases
```console
Pi λ !999                  # No such history
Pi λ !xyz                  # No matching command
Pi λ ^old^new              # No 'old' in last command
```

All features should work seamlessly together!