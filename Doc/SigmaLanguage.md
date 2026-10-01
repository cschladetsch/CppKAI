# Sigma

Sigma is KAI's statically typed language. It uses Rho's indentation-based
syntax, adds types, and compiles to Rho, which compiles to Pi. A program that
does not type-check does not run.

```sigma
fun mean(xs: List[float]) -> float
    total: float = 0
    for x in xs
        total += x
    return total / xs.size()

samples: List[float] = [2, 4, 4, 4, 5, 5, 7, 9]
m = mean(samples)          // m: float, inferred
```

In the console, type `sigma` to switch language; the prompt becomes `Sigma σ`
(`rho` and `pi` switch back). `Console -l sigma` starts in Sigma, and `.sigma`
files run as Sigma. A line that opens a block (`fun`, `if`, `while`, `for`,
`do`) continues until an empty line, and declarations carry over between lines.

From C++, register it with the console and switch to it:

```cpp
console.AddTranslator(Language::Sigma, std::make_shared<SigmaTranslator>(console.GetRegistry()),
                      /*indentedBlocks*/ true, /*prompt*/ "σ");
console.SetLanguage(Language::Sigma);
```

or use `SigmaTranslator` directly.

## Types

| Type | Notes |
|------|-------|
| `bool` `int` `float` `str` | `string` is an alias of `str` |
| `List[T]` | list literals, `push`, `size`, `slice`, `xs[i]` |
| `Map[str, V]` | map literals (keys are string literals), `keys`, `size`, `m[k]` |
| `fun(T, ...) -> R` | named functions are values of this type |
| `any` | explicit escape hatch; never inferred |
| `void` | function results only |
| `ClassName` | any class registered with the Registry; methods and properties are checked against the signatures `ClassBuilder` records |

Generics are invariant. The only implicit conversion is `int` to `float`, and
the compiler makes it explicit in the generated Rho as `(e + 0.0)`, so a
`float` variable never holds an `Int` at runtime.

## Rules

- **Declarations.** `x: T = e` declares with a type. `x = e` on a name that is
  not yet declared declares it with the type of `e`, which is then fixed:
  `x = 1` followed by `x = "s"` is an error. Every declaration has an initial
  value. The type of an empty list or map must be given.
- **No implicit `any`.** Undeclared names are errors. A value of type `any`
  (for example from a `pi { ... }` block) can only be stored in a variable
  declared with a type.
- **No shadowing.** Declaring a name that already exists in an enclosing scope
  is an error, because Rho would assign to the outer variable instead of
  creating a new one. Function parameters are local and may reuse global names.
- **Functions.** Every parameter has a type. `-> T` gives the result; without
  it the function returns `void`. A function with a result must return a
  value on every path. Functions are defined at the top level and may be
  called before their definition and be mutually recursive.
- **Conditions** of `if`, `while`, `do ... while`, `for`, `?:` and `assert`
  must be `bool`. `&&`, `||` and `!` take `bool`.
- **Operators.** Arithmetic on numbers (int with int gives int, otherwise
  float); `+` also joins two strings or two lists of the same type; ordering
  on numbers or strings; `==` and `!=` on values of the same type (or two
  numbers); bitwise operators on ints.
- `break` and `continue` only inside loops; `return` only inside functions.

## Statements

```sigma
if c
    ...
else if d
    ...
else
    ...
while c
    ...
do
    ...
while c
for i = 0; i < n; i += 1
    ...
for x in xs
    ...
x op= e      // op is + - * / %, on a name
assert(c)
print(x)
n: int = pi { 2 3 + }   // one-line Pi, typed by the declaration
```

## Not supported yet

Anonymous functions, nested functions, `yield`, nullable types, union types,
user-defined generics, shell commands, pathnames, `self`, continuation
operators (`&`, `...`, `!` after calls), `++`/`--`, and multi-line `pi` blocks.
Calls, `.` and indexing must follow a name (`[1, 2][0]` is rejected, because
Rho would read it as two expressions).

## How it compiles

Sigma lives in CppKAI: headers in `Include/KAI/Language/Sigma`, sources in
`Source/Library/Language/Sigma`, built as the `SigmaLang` library. CppKaiCore's
console knows nothing about Sigma; the console app registers it with
`Console::AddTranslator`.

`SigmaLexer`, `SigmaParser` and `SigmaTranslator` are built on the
Language/Common framework (`LexerCommon`, `ParserCommon`, `AstNodeBase`,
`TranslatorBase`). `SigmaChecker` type-checks the AST, then the translator
emits Rho and hands it to `RhoTranslator`. `SigmaTranslator::Compile()` stops
before Rho and `GetRho()` returns the generated source.

The generated Rho is fully parenthesised and expands compound assignment,
because Rho:

- parses `-1 + 5` as `-1` (a leading sign returns early in `Additive()`),
- does not implement `+=` and friends at runtime ("Unimplemented operation").

```
x: float = 1          x = (1 + 0.0)
i = 0                 i = 0
i += 2          =>    i = (i + 2)
y = -i + x * 2        y = ((-i) + (x * 2))
```

## Known runtime issue

An early `return` inside an `if` is lost when the function is called from
inside a loop (`Executor::ExecuteContinuationInlineAndDrain` resets `break_`
on each drain step). This is a Rho/Pi executor bug; Sigma's checker cannot
detect it. See `SigmaTests.DISABLED_EarlyReturnInFunctionCalledFromLoop`.
Until it is fixed, prefer a single `return` in functions that are called from
loops (the example scripts do this).

## Tests

`Test/Language/TestSigma/SigmaTests.cpp` and the scripts in
`Test/Language/TestSigma/Scripts/*.sigma`, each of which must type-check, run,
and end with a true expression.
