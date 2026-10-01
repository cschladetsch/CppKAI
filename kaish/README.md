# kaish (KAI Object Shell)

A shell that blends a bash-style command language with KAI's Pi and Rho. All
three modes share one value stack, shown HP48-style after every command
(level 1 just above the prompt).

## Build (PowerShell, from the CppKAI root)

```powershell
cmake -S . -B build
cmake --build build --config Debug --target kaish kaish_tests
.\Bin\kaish_tests.exe
.\Bin\kaish.exe
```

Inside CppKAI it links ConsoleLib, so pi and rho are the real KAI languages.
Built on its own (`cmake -S kaish -B kaish/build`) it is ps mode only.

## Modes

| Type | Effect |
|---|---|
| `ps` / `pi` / `rho` | switch mode |
| `pi 1 2 +`, `rho 1 + 2` | run one line in another mode |
| `$ ls` | run a ps line from pi or rho |
| `ps aux` | `ps` with arguments is always the program, in any mode |

In ps mode, results go onto the stack:

- `ls` pushes one entry per file (`ls -l` pushes one formatted line per file)
- any other output (`cat`, `pwd`, `echo`, external programs) is pushed as one entry
- `cd`, `mkdir`, `rm` and redirected commands push nothing; errors go to the screen
- `help`, `history`, `clear` print instead
- `term CMD` gives CMD the terminal instead of capturing it; programs listed in
  `passthrough` (editors, pagers, REPLs) always get the terminal

Stack words in ps mode: `drop [n]`, `dup`, `swap`, `clr`. In pi and rho use the
languages' own words; `clr` clears the stack in every mode.

`@N` at the start of an unquoted word is stack level N, so after `ls` you can
`cp @2 @1.bak` or `cat @1`. References copy the value (like PICK); use `drop` to
remove it. `@N` past the top of the stack is "Too few arguments" and the command
does not run. Write `'@1'` for a literal; `user@host` and `pkg@4` are untouched.

## Scripts

`kaish build.sh` runs a ps script. `kaish file.pi` and `kaish file.rho` (or
`source file.pi` interactively) run KAI scripts the way the Console does: `#`
lines are comments, `$` lines are ps commands whose results go onto the stack,
and the code between them is evaluated as a block. The stack is printed at the end.

## ~/.kaish.json

Written with defaults on first interactive run.

```json
{
  "stack_levels": 8,
  "mode": "ps",
  "show_stack": true,
  "passthrough": ["vim", "less", "ssh", "python", "pwsh", "claude"]
}
```

## What works

| Feature | Syntax |
|---|---|
| Pipes, sequencing | `a \| b`, `a ; b`, `a && b`, `a \|\| b` |
| Redirection | `<`, `>`, `>>`, `2>`, `2>>`, `2>&1`, `1>&2`, `&>`, `/dev/null` (maps to `NUL`) |
| Quoting | `'literal'`, `"with $vars"`, `\x` |
| Variables | `NAME=v`, `export`, `unset`, `$VAR`, `${VAR}`, `$?`, `$$`, `$1`, `$#`, `FOO=1 cmd` |
| Substitution | `$(cmd)`, `` `cmd` `` |
| Globbing | `*`, `?`, `[a-z]`, `[!x]` (case-insensitive on Windows), `~` |
| History | `history`, `!!`, `!n`, `~/.kaish_history`; arrow keys / F7 from the console |
| Startup | `~/.kaishrc` |
| Continuation | open quotes, trailing `\`, `\|`, `&&`, `\|\|` prompt with `> ` |

Builtins: `ls cd pwd echo cat head tail wc grep sort cp mv rm mkdir rmdir touch
export unset env set alias unalias history type which source . exit true false clear
help command term drop dup swap clr`. Default aliases: `ll`, `la`, `l`.

External programs are found on `PATH` using `PATHEXT`; `.bat`/`.cmd` go through
`cmd.exe`, `.ps1` through `pwsh` (or `powershell`). `command ls` skips the builtin.

Windows notes: an unquoted backslash only escapes shell metacharacters
(space, quotes, `$ | & ; < > ( ) # !`), so `cd C:\Users\chris` and `ls src\*.cpp`
work as typed. Ctrl-C interrupts the running program, not the shell; Ctrl-D or
Ctrl-Z then Enter exits.

## Not yet

Background jobs (`&`), subshells `( )`, `if/for/while`, functions, tab completion,
streaming pipes (stages currently hand off through temp files, so `yes | head` will
not terminate).
