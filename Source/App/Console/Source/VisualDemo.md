# Visual Demo: Console Zsh-like Features

## 1. Basic History Expansion

```console
Pi λ 1 2 +
[0]: 3

Pi λ print "Hello"
[0]: "Hello"

Pi λ !!
=> print "Hello"        # <-- Shows what command is being executed
[0]: "Hello"
```

## 2. Numbered History (!n, !-n)

```console
Pi λ !1                 # Execute first command in history
=> 1 2 +
[0]: 3

Pi λ !-1                # Execute last command
=> 1 2 +
[0]: 3

Pi λ !-2                # Execute second from last
=> print "Hello"
[0]: "Hello"
```

## 3. String Search (!string)

```console
Pi λ print "World"
[0]: "World"

Pi λ 10 20 +
[0]: 30

Pi λ !print            # Finds last command starting with 'print'
=> print "World"
[0]: "World"
```

## 4. Word Designators

```console
Pi λ echo one two three four five

Pi λ !!:0              # The command itself
=> echo

Pi λ !!:^              # First argument
=> one

Pi λ !!:$              # Last argument  
=> five

Pi λ !!:*              # All arguments
=> one two three four five

Pi λ !!:2-4            # Range of words
=> two three four

Pi λ !!:3*             # From word 3 to end
=> three four five
```

## 5. Complex Example: !-3:4*

```console
Pi λ ls -la /home/user file1.txt file2.txt file3.txt
Pi λ cd /tmp
Pi λ pwd
Pi λ !-3:4*            # Get words 4 onwards from 3 commands ago
=> file1.txt file2.txt file3.txt
```

## 6. Shell Commands

```console
Pi λ `echo "Hello from bash"`
Hello from bash

Pi λ `date +%Y-%m-%d`
2024-06-04

Pi λ print "User is `whoami`"
[0]: "User is xian"
```

## 7. History Expansion in Commands

```console
Pi λ 5
[0]: 5

Pi λ !! * 10          # Expands to: 5 * 10
=> 5 * 10
[0]: 50

Pi λ 100
[0]: 100

Pi λ 200
[0]: 200

Pi λ !-2 + !-1        # Expands to: 100 + 200
=> 100 + 200
[0]: 300
```

## 8. Disabling Expansion with $

```console
Pi λ print "test"
[0]: "test"

Pi λ $!!              # Dollar prefix prevents expansion
!!                    # Literal !! is executed (would be an error)

Pi λ $!print          # No expansion happens
!print                # Literal !print
```

## 9. Error Handling

```console
Pi λ !999
No matching command in history

Pi λ !nonexistent
No matching command in history
```

## 10. Quoted String Handling

```console
Pi λ echo "hello world" "goodbye moon" unquoted
Pi λ !!:1
=> "hello world"      # Quotes are preserved

Pi λ !!:2
=> "goodbye moon"

Pi λ !!:3
=> unquoted
```

## Key Visual Indicators

- **`=> command`** - Shows the expanded command before execution
- **`[n]: value`** - Shows stack contents after execution
- **Red text** - Error messages
- **Cyan `=> `** - Indicates history expansion occurred

## Window App Integration

The same features work in the Window app GUI:

```
┌─────────────────────────────────┐
│ Pi  Rho  Debugger               │
├─────────────────────────────────┤
│ Pi> 1 2 +                       │
│ Stack:                          │
│   3                             │
│ Pi> !!                          │
│ => 1 2 +                        │
│ Stack:                          │
│   3                             │
│ Pi> !-1:*                       │
│ => 1 2 +                        │
│ Stack:                          │
│   3                             │
└─────────────────────────────────┘
```

All zsh-like features work identically in both Console and Window apps!