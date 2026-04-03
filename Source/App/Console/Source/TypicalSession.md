# Typical Interactive Console Session

```console
$ ./KAI/Bin/Console
KAI Console v1.0.0 - Interactive REPL for Pi, Rho, and Tau languages
Type 'help' for assistance

Pi λ 42
[0]: 42

Pi λ print "Welcome to KAI Console!"
[0]: "Welcome to KAI Console!"

Pi λ 3.14159 2 * 
[0]: 6.28318

Pi λ # Let's define a function
Pi λ fun circle_area { dup * 3.14159 * }

Pi λ 5 circle_area
[0]: 78.5398

Pi λ # Oops, let's check what we just calculated
Pi λ !!
=> 5 circle_area
[0]: 78.5398

Pi λ # Let's create another function
Pi λ fun circle_circumference { 2 * 3.14159 * }

Pi λ 5 circle_circumference
[0]: 31.4159

Pi λ # Now let's do some calculations with previous results
Pi λ print "Radius 5: Area = " 
[0]: "Radius 5: Area = "

Pi λ !-3
=> 5 circle_area
[0]: 78.5398

Pi λ + " Circumference = " +
[0]: "Radius 5: Area = 78.5398 Circumference = "

Pi λ !-3
=> 5 circle_circumference  
[0]: 31.4159

Pi λ +
[0]: "Radius 5: Area = 78.5398 Circumference = 31.4159"

Pi λ # Let's work with some data
Pi λ data = [1 2 3 4 5]

Pi λ data { dup * } map
[0]: [1 4 9 16 25]

Pi λ # Save this squared data
Pi λ squared_data =

Pi λ # Let's also cube the original data
Pi λ data { dup dup * * } map
[0]: [1 8 27 64 125]

Pi λ cubed_data =

Pi λ # Now let's use shell commands to save our results
Pi λ print "Original: " data + "\nSquared: " + squared_data + "\nCubed: " + cubed_data +
[0]: "Original: [1 2 3 4 5]\nSquared: [1 4 9 16 25]\nCubed: [1 8 27 64 125]"

Pi λ `echo "Results saved at: $(date)" > results.txt`

Pi λ !! 
=> `echo "Results saved at: $(date)" > results.txt`

Pi λ `cat results.txt`
Results saved at: Tue Jun  4 16:45:23 PST 2024

Pi λ # Let's explore the history with word designators
Pi λ fun process_list { { 2 * } map { 10 > } filter sum }

Pi λ [3 6 9 12 15] process_list
[0]: 54

Pi λ # Extract just the list from the previous command
Pi λ !!:0
=> [3 6 9 12 15]
[0]: [3 6 9 12 15]

Pi λ # Or just the function name
Pi λ !-2:1
=> process_list

Pi λ # Let's do something more complex
Pi λ report = { "Processing " swap + " items: " + swap process_list "Total: " swap + }

Pi λ [5 10 15 20 25] dup size report
[0]: "Processing 5 items: Total: 150"

Pi λ # Reuse parts of previous commands
Pi λ [1 2 3 4 5] !!:2 !!:3 !!:4
=> [1 2 3 4 5] dup size report
[0]: "Processing 5 items: Total: 30"

Pi λ # Check system information
Pi λ print "User: `whoami` | Directory: `pwd` | Time: `date +%H:%M`"
[0]: "User: xian | Directory: /home/xian/KAI | Time: 16:47"

Pi λ # Let's disable expansion to show literal commands
Pi λ $print "This shows literal !! and !commands"
print "This shows literal !! and !commands"
[0]: "This shows literal !! and !commands"

Pi λ # Search for specific commands in history
Pi λ !fun
=> fun process_list { { 2 * } map { 10 > } filter sum }

Pi λ # Complex example: reuse multiple parts
Pi λ calculate = { "Input: " swap + "\nDoubled: " + swap dup 2 * + "\nSquared: " + swap dup * + }

Pi λ 7 calculate
[0]: "Input: 7\nDoubled: 14\nSquared: 49"

Pi λ # Let's redo with a different number using history
Pi λ 12 !-1:1
=> 12 calculate
[0]: "Input: 12\nDoubled: 24\nSquared: 144"

Pi λ # Create a file with our functions
Pi λ `echo "# KAI Functions" > my_functions.kai`

Pi λ `echo "fun circle_area { dup * 3.14159 * }" >> my_functions.kai`

Pi λ `echo "fun circle_circumference { 2 * 3.14159 * }" >> my_functions.kai`

Pi λ !-20:*
=> fun process_list { { 2 * } map { 10 > } filter sum }

Pi λ `echo "!!" >> my_functions.kai`

Pi λ `cat my_functions.kai`
# KAI Functions
fun circle_area { dup * 3.14159 * }
fun circle_circumference { 2 * 3.14159 * }
fun process_list { { 2 * } map { 10 > } filter sum }

Pi λ # Switch to Rho language
Pi λ \rho
Rho λ # Rho language mode

Rho λ function factorial(n) {
...     if (n <= 1) {
...         return 1;
...     } else {
...         return n * factorial(n - 1);
...     }
... }

Rho λ factorial(5);
[0]: 120

Rho λ # History works across languages
Rho λ !!
=> factorial(5);
[0]: 120

Rho λ factorial(7);
[0]: 5040

Rho λ # Extract just the number from previous call
Rho λ !-1:0
=> factorial(7);

Rho λ # Back to Pi
Rho λ \pi
Pi λ # Back in Pi mode

Pi λ # Let's check our history is preserved
Pi λ !fact
=> factorial(7);
Error: 'factorial' not defined in Pi context

Pi λ # That's expected - Rho functions aren't available in Pi

Pi λ # But we can still see the command
Pi λ !`cat`
=> `cat my_functions.kai`
# KAI Functions
fun circle_area { dup * 3.14159 * }
fun circle_circumference { 2 * 3.14159 * }
fun process_list { { 2 * } map { 10 > } filter sum }

Pi λ # Clean up
Pi λ `rm results.txt my_functions.kai`

Pi λ print "Session complete!"
[0]: "Session complete!"

Pi λ exit
Goodbye!
```

## Key Takeaways from This Session:

1. **Natural Workflow**: History expansion makes iterative development natural
2. **Function Reuse**: Easy to recall and modify previous function definitions
3. **Data Processing**: Can quickly reapply operations to new data
4. **Shell Integration**: Seamlessly mix KAI commands with system operations
5. **Cross-Language**: History works across Pi/Rho language switches
6. **Word Extraction**: Precisely reuse parts of previous commands
7. **File Operations**: Easy to save and load work using shell commands
8. **Exploration**: `!!` to repeat, `!string` to search makes exploration easy

This demonstrates how zsh-like features enhance productivity in real KAI Console usage!