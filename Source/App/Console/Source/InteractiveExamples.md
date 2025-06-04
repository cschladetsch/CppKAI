# Interactive Console Examples

## Example 1: Mathematical Exploration

```console
Pi λ # Exploring Fibonacci sequence
Pi λ fib_pair = { dup rot + }

Pi λ 0 1 fib_pair
[0]: 1
[1]: 1

Pi λ !!
=> 0 1 fib_pair
[0]: 1
[1]: 1

Pi λ !! !! !! !! !!
=> 0 1 fib_pair fib_pair fib_pair fib_pair fib_pair
[0]: 8
[1]: 13

Pi λ # Let's create a better Fibonacci function
Pi λ fun fib { 0 1 rot { fib_pair } times drop }

Pi λ 10 fib
[0]: 55

Pi λ 15 fib
[0]: 610

Pi λ # Compare the 10th and 15th Fibonacci numbers
Pi λ !-2 !-1 /
=> 10 fib 15 fib /
[0]: 0.0901639
```

## Example 2: String Processing

```console
Pi λ names = ["Alice" "Bob" "Charlie" "Diana"]

Pi λ names { "Hello, " swap + "!" + } map
[0]: ["Hello, Alice!" "Hello, Bob!" "Hello, Charlie!" "Hello, Diana!"]

Pi λ # Oops, let's make it more formal
Pi λ names { "Dear " swap + "," + } map
[0]: ["Dear Alice," "Dear Bob," "Dear Charlie," "Dear Diana,"]

Pi λ # Extract just the transformation function
Pi λ !-1:2
=> { "Dear " swap + "," + }

Pi λ # Apply to a single name
Pi λ "Professor Smith" !!
=> "Professor Smith" { "Dear " swap + "," + }
[0]: "Dear Professor Smith,"
```

## Example 3: System Integration

```console
Pi λ # Check system resources
Pi λ `df -h | grep -E "^/dev" | head -3`
/dev/sda1        20G  8.5G   11G  45% /
/dev/sda2       100G   45G   50G  48% /home
/dev/sda3        50G   15G   33G  32% /var

Pi λ # Parse disk usage
Pi λ print "Home partition is at `df -h | grep "/home" | awk '{print $5}'` capacity"
[0]: "Home partition is at 48% capacity"

Pi λ # Create a monitoring function
Pi λ fun disk_check { `df -h | grep "$1" | awk '{print $5}'` }

Pi λ "/home" disk_check
[0]: "48%"

Pi λ "/" disk_check
[0]: "45%"

Pi λ # Save this useful function
Pi λ !fun
=> fun disk_check { `df -h | grep "$1" | awk '{print $5}'` }

Pi λ `echo '!!' > disk_monitor.kai`
```

## Example 4: Data Analysis

```console
Pi λ data = [23 45 12 67 34 89 56 78 90 23 45 67]

Pi λ data sum data size / 
[0]: 52.4167

Pi λ mean =

Pi λ # Calculate variance
Pi λ data { mean - dup * } map sum data size /
[0]: 625.243

Pi λ variance =

Pi λ variance sqrt
[0]: 25.0049

Pi λ # Let's redo this with different data using history
Pi λ [10 20 30 40 50] dup sum swap size /
[0]: 30

Pi λ mean =

Pi λ !-5:0 !-5:2 !-5:3 !-5:4 !-5:5 !-5:6 !-5:7 !-5:8
=> [10 20 30 40 50] { mean - dup * } map sum [10 20 30 40 50] size /
[0]: 200

Pi λ sqrt
[0]: 14.1421
```

## Example 5: Working with Complex History

```console
Pi λ # Define multiple related functions
Pi λ fun add { + }
Pi λ fun multiply { * }
Pi λ fun square { dup multiply }
Pi λ fun cube { dup dup multiply multiply }

Pi λ 5 square
[0]: 25

Pi λ 5 cube
[0]: 125

Pi λ # Create a power function using history
Pi λ fun power { !cube:2 !square:2 }
Error: History expansion in function definition

Pi λ # Instead, let's extract the operations
Pi λ !square:2
=> { dup multiply }

Pi λ !cube:2  
=> { dup dup multiply multiply }

Pi λ # Now we can study the patterns
Pi λ print "Square uses: " !!:* + 
[0]: "Square uses: { dup dup multiply multiply }"
```

## Example 6: Debugging with History

```console
Pi λ # Complex calculation that might have an error
Pi λ [1 2 3 4 5] { dup 2 * 1 + } map { 3 > } filter sum
[0]: 32

Pi λ # Let's break it down using history
Pi λ !-1:0
=> [1 2 3 4 5]
[0]: [1 2 3 4 5]

Pi λ !!:0 !-2:2
=> [1 2 3 4 5] { dup 2 * 1 + }
[0]: { dup 2 * 1 + }

Pi λ !-3:0 !! map
=> [1 2 3 4 5] { dup 2 * 1 + } map
[0]: [3 5 7 9 11]

Pi λ # Now we can see the intermediate result
Pi λ !! !-4:4 !-4:5
=> [3 5 7 9 11] { 3 > } filter
[0]: [5 7 9 11]

Pi λ !! sum
=> [5 7 9 11] sum
[0]: 32

Pi λ # Verified: (5+7+9+11) = 32 ✓
```

## Example 7: Building Complex Commands

```console
Pi λ # Start with simple pieces
Pi λ base_url = "https://api.example.com"

Pi λ endpoint = "/users"

Pi λ # Build up a curl command
Pi λ `echo "curl -s !-1:* !-2:*"`
curl -s "/users" "https://api.example.com"

Pi λ # That's not quite right, let's fix it
Pi λ print base_url endpoint +
[0]: "https://api.example.com/users"

Pi λ api_url = 

Pi λ `curl -s "!!" | jq '.users | length'`
=> `curl -s "https://api.example.com/users" | jq '.users | length'`
42

Pi λ # Create a reusable API function
Pi λ fun api_count { endpoint = base_url swap + `curl -s "!!" | jq '. | length'` }

Pi λ "/posts" api_count
[0]: 128
```

These examples show how zsh-like features make the Console incredibly powerful for:
- Iterative development
- Data exploration
- System integration  
- Debugging complex expressions
- Building up commands incrementally
- Reusing previous work efficiently