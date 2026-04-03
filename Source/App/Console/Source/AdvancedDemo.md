# Advanced Zsh Features Demo

## Quick Substitution Demo

```console
Pi λ print "Teh quick brown fox jumps over teh lazy dog"
[0]: "Teh quick brown fox jumps over teh lazy dog"

Pi λ ^Teh^The
=> print "The quick brown fox jumps over teh lazy dog"
[0]: "The quick brown fox jumps over teh lazy dog"

Pi λ ^teh^the^
=> print "The quick brown fox jumps over the lazy dog"
[0]: "The quick brown fox jumps over the lazy dog"
```

## History Modifier Demo

```console
Pi λ process /home/user/projects/KAI/src/main.cpp

Pi λ # Get just the directory
Pi λ !!:$:h
=> /home/user/projects/KAI/src

Pi λ # Get just the filename
Pi λ !-2:$:t
=> main.cpp

Pi λ # Get filename without extension
Pi λ !-3:$:t:r
=> main

Pi λ # Get just the extension
Pi λ !-4:$:e
=> cpp

Pi λ # Build output path
Pi λ compile !-5:$ !-5:$:h:s/src/build/:+/!-5:$:t:r.o
=> compile /home/user/projects/KAI/src/main.cpp /home/user/projects/KAI/build/main.o
```

## Advanced Search Demo

```console
Pi λ calculate_fibonacci 10
[0]: 55

Pi λ print "Result stored"
[0]: "Result stored"

Pi λ calculate_factorial 5
[0]: 120

Pi λ # Find any calculation
Pi λ !?calc?
=> calculate_factorial 5
[0]: 120

Pi λ # Find the fibonacci one specifically
Pi λ !?fib?
=> calculate_fibonacci 10
[0]: 55
```

## Case Modification Demo

```console
Pi λ username = "john_doe"

Pi λ print "Welcome, " !!:$:u +
=> print "Welcome, " "JOHN_DOE" +
[0]: "Welcome, JOHN_DOE"

Pi λ SERVER_NAME = "PROD-SERVER-01"

Pi λ echo "Connecting to " !!:$:l +
=> echo "Connecting to " "prod-server-01" +
[0]: "Connecting to prod-server-01"
```

## Substitution Modifiers Demo

```console
Pi λ echo "apple banana apple cherry apple"

Pi λ !!:s/apple/orange/
=> echo "orange banana apple cherry apple"

Pi λ !-2:gs/apple/orange/
=> echo "orange banana orange cherry orange"

Pi λ config_file="/etc/myapp/config.json"

Pi λ backup !!:$ !!:$:s/config/config.backup/
=> backup /etc/myapp/config.json /etc/myapp/config.backup.json

Pi λ !!:gs/backup/archive/:p
=> backup /etc/myapp/config.json /etc/myapp/config.archive.json
(printed but not executed)
```

## Complex Real-World Examples

### Example 1: Refactoring Function Names

```console
Pi λ fun calculateTotalPrice { price tax + discount - }

Pi λ test calculateTotalPrice 100 10 5

Pi λ # Rename the function
Pi λ ^calculateTotalPrice^computeFinalCost^
=> fun computeFinalCost { price tax + discount - }

Pi λ # Update the test
Pi λ !test:gs/calculateTotalPrice/computeFinalCost/
=> test computeFinalCost 100 10 5
```

### Example 2: Building File Paths

```console
Pi λ source_dir="/project/source/components"

Pi λ compile !!:$/Button.jsx !!:$:s/source/build/:s/jsx/js/
=> compile /project/source/components/Button.jsx /project/build/components/Button.js

Pi λ # Process all components
Pi λ for component in [Button Header Footer]; do compile !!:2:h/!#:5.jsx !!:2:h:s/source/build//!#:5.js; done
```

### Example 3: API Testing

```console
Pi λ api_base="https://api.example.com/v2"

Pi λ curl !!:$/users -H "Auth: token123"

Pi λ # Test different endpoints with same auth
Pi λ !curl:s/users/products/
=> curl https://api.example.com/v2/products -H "Auth: token123"

Pi λ !-2:s/users/orders/:s/token123/token456/
=> curl https://api.example.com/v2/orders -H "Auth: token456"
```

### Example 4: Data Processing Pipeline

```console
Pi λ load_data raw_input.csv

Pi λ !!:$:r:+_processed.csv
=> raw_input_processed.csv

Pi λ transform !-2:$ !!
=> transform raw_input.csv raw_input_processed.csv

Pi λ analyze !$:s/processed/analyzed/:p
=> analyze raw_input_analyzed.csv
(preview)

Pi λ # Execute the previewed command
Pi λ !!
=> analyze raw_input_analyzed.csv
```

## Quick Reference Combinations

```console
# Last argument of previous command
!$

# First argument of previous command  
!^

# All arguments, uppercase
!!:*:u

# Second word of command 3 commands ago, lowercase
!-3:2:l

# Path without extension, quoted
!!:$:r:q

# Global substitution with preview
!!:gs/old/new/:p

# Search for command containing 'test'
!?test?

# Current command's first argument with .bak extension
!#:1.bak

# Quick typo fix
^teh^the^
```

These advanced features make the KAI Console as powerful as zsh for command-line manipulation!