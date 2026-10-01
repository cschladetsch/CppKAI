# Sigma design note: continuation operators

**Status: planned, not implemented.** The Sigma reference is
[Doc/Sigma/README.md](Sigma/README.md); this note covers one feature it lists
under *Not supported yet*.

Rho lets a call end with one of three postfix operators that choose how
control passes to the callee. Sigma does not accept them yet:
`SigmaParser::Postfix` stops after `.`, `(...)` and `[...]`, and `SigmaLexer`
reports `'...' is not supported in Sigma`.

## What the operators do in Rho

`RhoParser` accepts the operator only directly after the closing `)` of a
call, and `RhoTranslator::TranslateCall` picks the Pi operation that runs the
callee:

| Rho | Pi operation | Effect |
|-----|--------------|--------|
| `f(x)` | `Suspend` | push the caller on the context stack, run `f`, come back |
| `f(x)&` | `Suspend` | the same as a plain call, written out |
| `f(x)!` | `Replace` | run `f` in place of the caller; the caller never resumes (a tail call) |
| `f(x)...` | `Resume` | leave the current continuation and resume the most recently suspended one |

See [ContinuationControl.md](ContinuationControl.md) for `Suspend`, `Replace`
and `Resume` at the Pi level.

These are not force, spread or borrow operators. `...` in particular never
expands a list into arguments, and Sigma has no variadic functions.

## What Sigma has to decide

The operators change control flow, so the checker has to treat them as
control flow, not just give the call expression a type.

- **`f(x)&`.** Same type as `f(x)`. This is the easy case and could be
  accepted as soon as the parser takes it.
- **`f(x)!`.** The caller does not continue, so the result goes to the
  caller's caller. A natural rule is that `f(x)!` is only allowed as
  `return f(x)!` (or as the last statement of a `void` function), with `f`'s
  result type equal to the enclosing function's result type. The checker's
  "every path returns" analysis would count it as a return.
- **`f(x)...`.** Control does not come back at all. It would be a statement,
  not an expression, and the code after it on the same path is unreachable.
- **Function values.** The operators apply to any callee of `fun(...)` type,
  so they work with values passed as arguments as well as named functions.

## Implementation outline

1. `SigmaLexer`: lex `...` as a token instead of reporting an error.
2. `SigmaParser::Postfix`: after a call, accept `&`, `!` or `...` and record it
   on the `Call` node, as `RhoParser` does.
3. `SigmaChecker`: apply the rules above, with `line:col` errors for misuse
   (for example `x = f(1)!`).
4. `SigmaTranslator`: write the operator back out after the call in the
   generated Rho.
5. Tests in `Test/Language/TestSigma`: valid and rejected uses of each
   operator, plus a script in `Scripts/` that ends with a `true` expression.
   Tests should be written in Sigma's own syntax (typed parameters, `//`
   comments, indented blocks).
