# Sigma design note: continuation operators

**Status: `&` and `!` are implemented; `...` is not.** The Sigma reference is
[Doc/Sigma/README.md](Sigma/README.md#continuation-operators); this note
records how the operators map onto Rho and Pi, and why the rules are what
they are.

## What the operators do in Rho

Rho lets a call end with one of three postfix operators that choose how
control passes to the callee. `RhoParser` accepts the operator directly after
the closing `)` of a call, and `RhoTranslator::TranslateCall` picks the Pi
operation that runs the callee:

| Rho | Pi operation | Effect |
|-----|--------------|--------|
| `f(x)` | `Suspend` | push the caller on the context stack, run `f`, come back |
| `f(x)&` | `Suspend` | the same as a plain call, written out |
| `f(x)!` | `Replace` | run `f` in place of the caller; the caller never resumes (a tail call) |
| `f(x)...` | `Resume` | clear the context stack and stop |

See [ContinuationControl.md](ContinuationControl.md) for `Suspend`, `Replace`
and `Resume` at the Pi level.

These are not force, spread or borrow operators. `...` in particular never
expands a list into arguments, and Sigma has no variadic functions.

## Spacing

In Sigma an operator only belongs to the call when it is written directly
after the `)`:

- `f(x)&` is a suspend; `f(x) & mask` is a bitwise and of the result.
- `f(x)!` is a tail call; `f(x) !` is a syntax error, since `!` is not a
  binary operator.
- `...` cannot be confused with anything else, so spacing would not matter,
  but it is not supported (below).

Rho itself ignores the spacing and reads `&` after any call's `)` as a
suspend, so when Sigma emits a bitwise and whose left operand is a call it
wraps the call: `((f(x)) & mask)`.

## The rules, and why

- **`&`** has the same type as the call and is allowed wherever the call is.
- **`!`** must be the last statement of a function body: `return f(...)!`,
  or `f(...)!` in a `void` function calling a `void` function. Rho only
  replaces correctly from there. Inside an `if` block (which `If` runs
  inline) a replace leaves extra values on the data stack, and inside a
  `while` it hangs. If the executor learns to replace from inline blocks,
  the rule can be relaxed to "any statement that ends a path".
- **No widening across `!`.** Sigma normally widens `int` to `float` with
  `(e + 0.0)`, but after a tail call no code of the calling function runs,
  so the callee must return exactly the function's result type.
- **Not on built-ins or methods.** Rho lowers `print`, `size`, `push` and
  friends to Pi operations directly, so a continuation operator on them
  means nothing.
- **`...` is rejected.** Rho's `Resume` clears the context stack and nulls
  the current continuation without ever calling the function: `g(4)...`
  leaves `g` and `4` on the data stack and stops the program. There is
  nothing there worth a type rule yet.

## Implementation

| Part | Change |
|------|--------|
| `SigmaParser::Postfix` | after a call, takes `&` or `!` if `SigmaParser::Adjacent` says it touches the `)`, and stores it as the call's third child |
| `SigmaChecker::Call` / `TailCall` / `TailCallIn` | the rules above, with `line:col` errors |
| `SigmaTranslator` | writes the operator back after the call; wraps a call on the left of a bitwise `&` |
| `SigmaLexer` | explains why `...` is rejected |
| `Test/Language/TestSigma/SigmaContinuationTests.cpp` | runs, generated Rho, and every rejected use |
