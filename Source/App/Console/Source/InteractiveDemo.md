# Interactive Console Demo - Step by Step

## Starting the Console

```bash
$ cd /home/xian/local/KAI
$ ./Bin/Console
```

## Demo Session: Learn by Doing

### Part 1: Basic Operations

```console
KAI Console v1.0.0
Type help for assistance, exit to quit

Pi λ # Let's start with basic arithmetic
Pi λ 2 3 +
[0]: 5

Pi λ # Pi uses stack-based operations
Pi λ 10 20 30
[0]: 10
[1]: 20
[2]: 30

Pi λ + +
[0]: 60

Pi λ # Let's clear the stack and try something else
Pi λ clear
Stack is empty

Pi λ # Define a simple function
Pi λ fun double { 2 * }

Pi λ 7 double
[0]: 14

Pi λ # Oops! Let's run that again using history
Pi λ !!
=> 7 double
[0]: 14
```

### Part 2: History Features - The Basics

```console
Pi λ # Let's see what we did before
Pi λ !fun
=> fun double { 2 * }

Pi λ # Now let's create more functions
Pi λ fun square { dup * }
Pi λ fun cube { dup dup * * }

Pi λ 5 square
[0]: 25

Pi λ 5 cube
[0]: 125

Pi λ # Use history to compare results
Pi λ !-2
=> 5 square
[0]: 25

Pi λ !-2
=> 5 cube
[0]: 125

Pi λ # Quick fix for typos
Pi λ print "Helllo World"
[0]: "Helllo World"

Pi λ ^lll^ll
=> print "Hello World"
[0]: "Hello World"
```

### Part 3: Working with Data

```console
Pi λ # Create some data
Pi λ data = [10 20 30 40 50]

Pi λ # Process it
Pi λ data { square } map
[0]: [100 400 900 1600 2500]

Pi λ # Let's sum the squares
Pi λ !! sum
=> [100 400 900 1600 2500] sum
[0]: 5500

Pi λ # Extract just the function from our previous map operation
Pi λ !map:2
=> { square }

Pi λ # Apply to a single number
Pi λ 12 !!
=> 12 { square }
[0]: 144
```

### Part 4: Shell Integration

```console
Pi λ # Check current directory
Pi λ `pwd`
/home/xian/local/KAI

Pi λ # List Pi script files
Pi λ `ls *.pi 2>/dev/null | head -5`
examples.pi
fibonacci.pi
test.pi

Pi λ # Create a timestamp
Pi λ timestamp = "`date +%Y%m%d_%H%M%S`"
Pi λ print timestamp
[0]: "20240604_143022"

Pi λ # Use it in a filename
Pi λ filename = "backup_" timestamp + ".txt" +
Pi λ print filename
[0]: "backup_20240604_143022.txt"
```

### Part 5: Advanced History - Word Designators

```console
Pi λ # Complex command with multiple parts
Pi λ process input.csv output.json --verbose --threads 4

Pi λ # Extract different parts
Pi λ !!:0
=> process

Pi λ !!:1
=> input.csv

Pi λ !!:$
=> 4

Pi λ !!:*
=> input.csv output.json --verbose --threads 4

Pi λ # Get a range
Pi λ !!:2-3
=> output.json --verbose

Pi λ # From word 2 to end
Pi λ !!:2*
=> output.json --verbose --threads 4
```

### Part 6: Path Manipulation with Modifiers

```console
Pi λ # Work with file paths
Pi λ filepath = "/home/user/projects/KAI/src/main.cpp"

Pi λ print filepath
[0]: "/home/user/projects/KAI/src/main.cpp"

Pi λ # Get directory
Pi λ !!:$:h
=> /home/user/projects/KAI/src

Pi λ # Get filename only
Pi λ !print:$:t
=> main.cpp

Pi λ # Get filename without extension
Pi λ !-2:$:t:r
=> main

Pi λ # Get extension only
Pi λ !-3:$:e
=> cpp

Pi λ # Build new path
Pi λ output = !-4:$:h + "/build/" + !-4:$:t:r + ".o"
Pi λ print output
[0]: "/home/user/projects/KAI/src/build/main.o"
```

### Part 7: Text Transformations

```console
Pi λ # Case conversions
Pi λ name = "john_doe"
Pi λ print name
[0]: "john_doe"

Pi λ # Convert to uppercase
Pi λ !!:$:u
=> "JOHN_DOE"

Pi λ # Mixed case handling
Pi λ title = "Introduction to Programming"
Pi λ print title
[0]: "Introduction to Programming"

Pi λ !!:$:l
=> "introduction to programming"

Pi λ # Quoting
Pi λ args = "file with spaces.txt"
Pi λ print args
[0]: "file with spaces.txt"

Pi λ !!:$:q
=> "\"file with spaces.txt\""
```

### Part 8: Substitutions

```console
Pi λ # Simple substitution
Pi λ echo "apple banana apple cherry"
[0]: "apple banana apple cherry"

Pi λ !!:s/apple/orange/
=> echo "orange banana apple cherry"
[0]: "orange banana apple cherry"

Pi λ # Global substitution
Pi λ !-2:gs/apple/orange/
=> echo "orange banana orange cherry"
[0]: "orange banana orange cherry"

Pi λ # Preview before executing
Pi λ dangerous_command --delete-all --force
Error: 'dangerous_command' not defined

Pi λ !!:s/dangerous/safe/:p
safe_command --delete-all --force
(Command printed but NOT executed)
```

### Part 9: Building Complex Commands

```console
Pi λ # Start with pieces
Pi λ base_url = "https://api.example.com"
Pi λ endpoint = "/users"
Pi λ token = "abc123"

Pi λ # Build API call
Pi λ api_call = base_url endpoint + 
Pi λ print api_call
[0]: "https://api.example.com/users"

Pi λ # Use with curl
Pi λ `curl -H "Auth: !!token" "!!api_call" 2>/dev/null || echo "Demo mode"`
Demo mode

Pi λ # Modify for different endpoint
Pi λ !api_call:s/users/products/
=> "https://api.example.com/products"
```

### Part 10: Search and Discovery

```console
Pi λ # Search for commands containing specific text
Pi λ # (After building up some history)

Pi λ !?square?
=> fun square { dup * }

Pi λ # Use shortcuts for common operations
Pi λ test arg1 arg2 arg3

Pi λ # Get last argument quickly
Pi λ print !$
=> print arg3
[0]: "arg3"

Pi λ # Get first argument quickly  
Pi λ print !^
=> print arg1
[0]: "arg1"
```

### Part 11: Combining Features

```console
Pi λ # Complex example: refactoring function names
Pi λ fun calculateTotal { sum }
Pi λ test_data = [10 20 30]
Pi λ test_data calculateTotal
[0]: 60

Pi λ # Rename the function
Pi λ ^calculateTotal^computeSum
=> fun computeSum { sum }

Pi λ # Update usage
Pi λ !test_data !?Total?:s/Total/Sum/
=> test_data computeSum
[0]: 60

Pi λ # Create variations
Pi λ !!:s/Sum/Average/:s/sum/size dup rot sum swap \//
=> fun computeAverage { size dup rot sum swap / }
```

### Part 12: Productivity Tips

```console
Pi λ # Disable expansion when needed
Pi λ $print "Use !! and !$ literally"
print "Use !! and !$ literally"
[0]: "Use !! and !$ literally"

Pi λ # Check current command construction
Pi λ echo start !#:0 middle !#:0 end
=> echo start echo middle echo end

Pi λ # Save useful commands
Pi λ useful_function = "{ dup * 3.14159 * }"
Pi λ `echo 'fun circle_area !!useful_function' > saved_functions.pi`

Pi λ # Exit when done
Pi λ exit
Goodbye!
```

## Key Takeaways

1. **Start Simple**: Basic arithmetic and stack operations
2. **Use History**: `!!` for last command, `!n` for specific commands
3. **Fix Typos**: `^old^new` for quick corrections
4. **Extract Parts**: Word designators (`:0`, `:$`, `:*`, etc.)
5. **Transform Paths**: Modifiers (`:h`, `:t`, `:r`, `:e`)
6. **Change Case**: `:u` for upper, `:l` for lower
7. **Substitute Text**: `:s/old/new/` and `:gs/old/new/`
8. **Preview Dangerous**: `:p` to see without executing
9. **Shell Integration**: Backticks for system commands
10. **Escape When Needed**: `$` prefix disables expansions

This demo shows real interactive usage - try it yourself!