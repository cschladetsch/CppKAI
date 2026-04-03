# Console Zsh Features - Quick Reference

## History Expansion

| Command | Description | Example |
|---------|-------------|---------|
| `!!` | Repeat last command | `!!` → executes previous command |
| `!n` | Execute nth command (1-based) | `!3` → executes 3rd command |
| `!-n` | Execute nth from end | `!-2` → executes 2nd to last |
| `!string` | Last command starting with string | `!print` → last print command |

## Word Designators (used after history expansion)

| Designator | Description | Example with `echo a b c d` |
|------------|-------------|------------------------------|
| `:0` | The command word | `!!:0` → `echo` |
| `:n` | The nth word (0-based) | `!!:2` → `b` |
| `:^` | First argument (same as :1) | `!!:^` → `a` |
| `:$` | Last argument | `!!:$` → `d` |
| `:*` | All arguments | `!!:*` → `a b c d` |
| `:n-m` | Words n through m | `!!:1-2` → `a b` |
| `:n*` | Words n through end | `!!:2*` → `b c d` |

## Complex Examples

```bash
# Get words 4 onward from 3 commands ago
!-3:4*

# Use last command's arguments with new command
echo !!:*

# Reuse specific arguments
cp !!:2 !!:3

# Multiple history references
!-2 + !-1
```

## Shell Commands

```bash
# Execute shell command
`date`

# Embed in strings
print "Today is `date +%A`"

# Multiple embedded commands
print "`whoami` in `pwd`"
```

## Special Features

```bash
# Execute shell commands with $ prefix
$ ls        # Executes 'ls' shell command
$ pwd       # Shows current directory
$ echo hello  # Prints 'hello'

# History expansion in commands
5
!! * 10     # Becomes: 5 * 10
```

## Error Messages

- `No matching command in history` - When history reference not found
- `=> command` - Shows expanded command before execution

## Tips

1. Commands are added to history after pressing Enter
2. History is per-session (not persistent)
3. Word splitting respects quoted strings
4. Works in both Console and Window apps
5. Use `$` prefix to type literal `!` characters