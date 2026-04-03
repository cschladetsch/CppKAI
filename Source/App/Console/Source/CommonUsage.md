# Common Console Usage Patterns

## Quick Interactive Session

```console
$ ./KAI/Bin/Console
KAI Console v1.0.0

Pi λ # Basic arithmetic
Pi λ 2 3 +
[0]: 5

Pi λ 4 5 *
[0]: 20

Pi λ # Oops, what was that first calculation?
Pi λ !-2
=> 2 3 +
[0]: 5

Pi λ # Let's use both results
Pi λ !! + 
=> 5 + 
[0]: 25

Pi λ # Define a simple function
Pi λ fun double { 2 * }

Pi λ 7 double
[0]: 14

Pi λ # Test with different values quickly
Pi λ 9 !-1:1
=> 9 double
[0]: 18

Pi λ 100 !-1:1
=> 100 double  
[0]: 200

Pi λ # Working with lists
Pi λ [1 2 3 4 5] { double } map
[0]: [2 4 6 8 10]

Pi λ # Apply a different operation to the same list
Pi λ !-1:0 { 3 + } map
=> [1 2 3 4 5] { 3 + } map
[0]: [4 5 6 7 8]

Pi λ # Quick shell integration
Pi λ `date`
Tue Jun  4 17:15:30 PST 2024

Pi λ print "Started at: !!"
[0]: "Started at: Tue Jun  4 17:15:30 PST 2024"

Pi λ # Disable expansion when needed
Pi λ $print "Use !! for history"
print "Use !! for history"
[0]: "Use !! for history"

Pi λ # Search history by prefix
Pi λ !fun
=> fun double { 2 * }

Pi λ # Extract parts of commands
Pi λ calculate 10 20 30 40
Error: 'calculate' not defined

Pi λ # Grab just the numbers
Pi λ !!:1*
=> 10 20 30 40
[0]: 10
[1]: 20
[2]: 30
[3]: 40

Pi λ + + +
[0]: 100

Pi λ exit
```

## Most Used Features

### 1. Quick Repetition
```
Pi λ !!           # Repeat last command
Pi λ !-1          # Same as !!
Pi λ !-3          # Three commands ago
```

### 2. Search and Reuse
```
Pi λ !print       # Last command starting with 'print'
Pi λ !fun         # Last function definition
```

### 3. Modify Previous Commands
```
Pi λ 5 + 3
Pi λ 10 !-1:1*    # Reuse '+ 3' with new first argument
```

### 4. Extract Command Parts
```
Pi λ !!:0         # Just the command/function
Pi λ !!:*         # All arguments
Pi λ !!:$         # Last argument
```

### 5. Shell Integration
```
Pi λ `ls *.txt`   # Run shell command
Pi λ print "`pwd`"  # Embed in strings
```

This represents how most users interact with the Console in daily use!