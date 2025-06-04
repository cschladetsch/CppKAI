# Console Zsh-like Features

The Console now supports zsh-like history expansion operators, unless the command line starts with '$'.

## Basic History Features:

1. **Command History**: All commands are stored in history for later reference.

2. **!! operator**: Repeats the last command
   ```
   > 1 2 +
   [0]: 3
   > !!
   => 1 2 +
   [0]: 3
   ```

3. **!n operator**: Executes the nth command from history (1-based)
   ```
   > !1
   => 1 2 +
   [0]: 3
   ```

4. **!-n operator**: Executes the nth command from the end of history
   ```
   > !-1
   => (last command)
   ```

5. **!string operator**: Executes the last command starting with 'string'
   ```
   > !print
   => print "Hello"
   ```

## Advanced Word Designators:

6. **Word selection with `:n`**: Select specific word from a command
   ```
   > echo one two three four
   > !-1:2
   => two
   ```

7. **Special word designators**:
   - `:^` - First argument (word 1)
   - `:$` - Last argument
   - `:*` - All arguments (words 1 to end)
   - `:n` - The nth word (0-based)
   - `:n-m` - Words n through m
   - `:n*` - Words n through end
   - `:-$` - All words except the command (0 through last)

8. **Complex examples like `!-3:4*`**: Get words 4 through end from 3 commands ago
   ```
   > ls -la /home/user/documents file1.txt file2.txt file3.txt
   > cd /tmp
   > pwd
   > !-3:4*
   => file1.txt file2.txt file3.txt
   ```

## Other Features:

9. **History expansion in commands**: History references can be embedded within commands
   ```
   > !! * 2
   => (last command) * 2
   ```

10. **$ prefix to disable zsh features**: Commands starting with $ are processed without zsh expansion
    ```
    > $!test
    (processes !test as literal text, not as history expansion)
    ```

## Implementation Details:

- History is stored in a vector of strings
- Commands are split into words respecting quoted strings
- When a command starting with ! is detected, it's expanded before execution
- Word designators are applied after retrieving the historical command
- The expanded command is shown with "=> " prefix for clarity
- If no matching command is found, an error message is displayed
- Commands starting with $ bypass all zsh-like processing

## TODO:
- History modifiers (`:h`, `:t`, `:r`, `:e`, `:p`, `:s/old/new/`) are not yet implemented