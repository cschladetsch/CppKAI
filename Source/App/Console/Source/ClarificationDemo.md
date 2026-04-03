# Console Usage Clarification

## The $ Prefix - What It Actually Does

The `$` prefix at the start of a line **ONLY** disables history expansion. It does **NOT** execute shell commands.

### Example 1: Normal History Expansion
```console
Pi λ print "test"
[0]: "test"

Pi λ print "Previous command was: !!"
=> print "Previous command was: print "test""
Error: Syntax error (quotes don't nest properly)
```

### Example 2: Using $ to Disable Expansion
```console
Pi λ $print "Previous command was: !!"
print "Previous command was: !!"
[0]: "Previous command was: !!"
```

The `$` simply tells the Console: "Don't expand any ! history references in this line"

## How to Execute Shell Commands

### CORRECT: Use Backticks
```console
Pi λ `ls`
file1.txt  file2.txt  directory/

Pi λ `pwd`
/home/user/KAI

Pi λ `echo "Hello from shell"`
Hello from shell
```

### INCORRECT: These Don't Work
```console
Pi λ ls
Error: 'ls' not defined

Pi λ $ ls  
Error: 'ls' not defined (the $ just prevents ! expansion)

Pi λ $`ls`
Error: This also doesn't help
```

## Why Your Examples Failed

1. **`ls`** - Pi tried to find a variable/function named 'ls'
2. **`$ ls`** - Pi stripped the $ and still tried to find 'ls'
3. **`# ls`** - The # starts a comment, but Pi still tried to parse 'ls'

## Summary

- **Backticks (`)**: Execute shell commands
- **Dollar ($)**: Disable ! history expansion only
- **Hash (#)**: Comments in Pi

```console
# Correct shell command execution
Pi λ `ls -la`

# Correct use of $ prefix
Pi λ $echo "!!" "!$" "!^"   # These ! patterns are NOT expanded
```