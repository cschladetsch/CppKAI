# Calc (kaicalc)

A command-line calculator that fixes what's wrong with Windows' `calc`:
no expression argument, nothing to pipe into it, no scripting, no
programmability. `kaicalc` is a thin CLI front-end onto KAI's actual
Pi/Rho expression evaluator - the same `Console` class KAI's full
`Console` app uses for whole programs - so it gets a real parser, real
arithmetic, and a real interactive session for free, instead of being a
calculator written from scratch.

## Usage

```
kaicalc 2 + 3 * 4          # one-shot: evaluate, print, exit -> 14
kaicalc -p 2 3 4 * +       # RPN (Pi) instead of infix (Rho) -> 14
echo "2 ^ 10" | kaicalc    # pipe mode: one expression per line -> 1024
kaicalc                    # interactive session (full KAI Console)
```

Rho (infix) is the default, matching what anyone typing `calc 2+2` would
expect. Pass `-p`/`--pi` for RPN if you'd rather work the way an HP-48
does.

The interactive session isn't a separate, stripped-down calculator REPL -
it's the actual KAI `Console::Run()` loop, so you get expression history,
`help`, variables, and everything else Console already supports.

## Building

`Calc` is a normal KAI app, built the same way as `Console`:

```
cmake -B build
cmake --build build --target Calc
```

The binary is written to `Bin/` (as `Calc`, plus a `kaicalc` copy
alongside it - see `CMakeLists.txt` for the exact copy/rename step per
platform).

## Making `calc` actually mean this, on Windows

Windows reserves `calc.exe` in `C:\Windows\System32`, and nothing short of
admin rights lets you overwrite or delete it - don't try. The fix is the
same one used for `cleanrepos` elsewhere in this setup: a PowerShell
function takes priority over anything on `PATH`, including files in
`System32`, when PowerShell resolves a bare command name. Add this to your
`profile.ps1` (`$PROFILE.CurrentUserAllHosts`):

```powershell
function calc { & "<path-to-Bin>\kaicalc.exe" @args }
```

Reload with `. $PROFILE.CurrentUserAllHosts`, and `calc 2 + 3 * 4` in any
PowerShell session runs kaicalc instead of launching the GUI. `calc` with
no arguments drops into the interactive session rather than opening a
window - if you sometimes want the actual Windows app, keep a separate
alias for it (e.g. `winturi` or `calc-gui`) pointing at the real
`System32\calc.exe`.

## Notes

- Each one-shot argument list or piped line is evaluated independently -
  the data stack is cleared after printing each result, so expressions
  don't see leftovers from a previous line.
- Exit code is 0 on success, 1 if any expression failed to evaluate (bad
  syntax, etc.) - errors go to stderr, results to stdout, so `kaicalc` is
  safe to use in a pipeline or script.
