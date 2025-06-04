# Advanced Zsh Features in KAI Console

## New Features Added

### 1. Quick Substitution: `^old^new^`

Replace text in the previous command quickly:

```console
Pi λ print "Hello Wrold!"
[0]: "Hello Wrold!"

Pi λ ^Wrold^World^
=> print "Hello World!"
[0]: "Hello World!"

Pi λ ls -la /hmoe/user
ls: cannot access '/hmoe/user': No such file or directory

Pi λ ^hmoe^home
=> ls -la /home/user
(file listing appears)
```

### 2. Search Anywhere: `!?string?`

Find commands containing a string anywhere (not just at the start):

```console
Pi λ calculate_area 5 circle
Pi λ print "result"
Pi λ show_circle_info 10

Pi λ !?circle?
=> show_circle_info 10
```

### 3. Quick Argument Access: `!$` and `!^`

```console
Pi λ cp important.txt backup.txt

Pi λ ls -la !$
=> ls -la backup.txt

Pi λ echo first second third

Pi λ print !^
=> print first
```

### 4. Current Command Reference: `!#`

Reference the current command line before the `!#`:

```console
Pi λ echo test !#:0
=> echo test echo

Pi λ cp file.txt !#:1.bak
=> cp file.txt file.txt.bak
```

### 5. History Modifiers

#### Path Modifiers
- `:h` - Head (remove last path component)
- `:t` - Tail (remove all but last component)
- `:r` - Root (remove extension)
- `:e` - Extension (remove all but extension)

```console
Pi λ echo /home/user/document.txt

Pi λ !!:$:h
=> /home/user

Pi λ !-2:$:t
=> document.txt

Pi λ !-3:$:r
=> /home/user/document

Pi λ !-4:$:e
=> txt
```

#### Text Modifiers
- `:u` - Uppercase
- `:l` - Lowercase
- `:q` - Quote
- `:x` - Quote each word separately

```console
Pi λ echo hello world

Pi λ !!:*:u
=> HELLO WORLD

Pi λ !-2:*:l
=> hello world

Pi λ !-3:*:q
=> "hello world"

Pi λ !-4:*:x
=> "hello" "world"
```

#### Substitution Modifiers
- `:s/old/new/` - Substitute first occurrence
- `:gs/old/new/` - Global substitution

```console
Pi λ echo foo bar foo baz

Pi λ !!:s/foo/FOO/
=> echo FOO bar foo baz

Pi λ !-2:gs/foo/FOO/
=> echo FOO bar FOO baz
```

### 6. Print Modifier: `:p`

Print the command without executing:

```console
Pi λ dangerous_command --force --delete-all

Pi λ !!:p
dangerous_command --force --delete-all
(command is printed but NOT executed)
```

## Complex Examples

### Example 1: File Processing Pipeline

```console
Pi λ process_data input.csv output.json

Pi λ backup !^:r.bak !$:r.bak
=> backup input.bak output.bak

Pi λ convert !-2:^:s/csv/tsv/ !-2:$:s/json/xml/
=> convert input.tsv output.xml
```

### Example 2: Building Commands Incrementally

```console
Pi λ base_cmd="curl -X POST"

Pi λ endpoint="https://api.example.com/users"

Pi λ !-2:$ !-1:$ -d '{"name": "!#:$:t"}'
=> curl -X POST https://api.example.com/users -d '{"name": "users"}'
```

### Example 3: Refactoring Commands

```console
Pi λ grep -r "oldFunction" src/

Pi λ ^oldFunction^newFunction^:p
=> grep -r "newFunction" src/
(printed but not executed)

Pi λ !!:gs/Function/Method/
=> grep -r "newMethod" src/
```

### Example 4: Working with Paths

```console
Pi λ compile /project/src/main.cpp /project/build/main.o

Pi λ run !$:h/!$:t:r
=> run /project/build/main

Pi λ deploy !-2:2:h:h/dist/!-2:2:t:r.exe
=> deploy /project/dist/main.exe
```

## Advanced Combinations

### Chaining Modifiers

```console
Pi λ PROCESS_FILE /Config/Settings.JSON

Pi λ !!:$:l:r
=> /config/settings

Pi λ !-2:$:t:l:s/json/yaml/
=> settings.yaml
```

### Multiple Substitutions

```console
Pi λ echo "The the quick brown fox"

Pi λ !!:gs/the/a/:s/The/A/
=> echo "A a quick brown fox"
```

### Conditional History Expansion

```console
Pi λ test -f important.conf && process important.conf

Pi λ !?conf?:gs/important/backup/:p
=> test -f backup.conf && process backup.conf
(preview without execution)
```

## Tips and Best Practices

1. **Use `:p` for dangerous commands** - Always preview before executing
2. **Combine modifiers** - Chain them for complex transformations
3. **Use `!?string?`** - When you remember part of a command but not how it started
4. **Quick fixes with `^`** - Fastest way to fix typos
5. **Path manipulation** - Use `:h`, `:t`, `:r`, `:e` for file operations
6. **Case conversion** - `:u` and `:l` for quick case changes
7. **Protect arguments** - Use `:q` or `:x` when dealing with spaces

## Comparison with Standard Zsh

All these features work identically to zsh, making KAI Console a powerful REPL that matches the capabilities of a modern shell. The only difference is that commands starting with `$` disable these expansions, giving you an escape hatch when needed.

```console
Pi λ $echo !!:$:h
!!:$:h
(literal text, no expansion)
```