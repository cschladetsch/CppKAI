# Rho ![Rho](/Images/DoPLOGO-01.png)
[![Build status](https://ci.appveyor.com/api/projects/status/github/cschladetsch/kai/Source/Library/Language/Rho/?svg=true)](https://ci.appveyor.com/project/cschladetsch/kai)
[![CodeFactor](https://www.codefactor.io/repository/github/cschladetsch/kai/badge)](https://www.codefactor.io/repository/github/cschladetsch/kai/Source/Library/Language/Rho/)
[![License](https://img.shields.io/github/license/cschladetsch/flow.svg?label=License&maxAge=86400)](./LICENSE.txt)
[![Release](https://img.shields.io/github/release/cschladetsch/flow.svg?label=Release&maxAge=60)](https://github.com/cschladetsch/kai/releases/latest)

Rho is an in-fix language that translates to [Pi](/Source/Library/Language/Pi/Readme.md).

Both Rho and Pi use the same [Executor](/Source/Library/Executor/Source/Executor.cpp).

Rho is much like _Python_, but with an even less verbose syntax.

The 'return value' for a Rho function (continuation) is whatever is left on the stack(s) when the continuatin leaves.

_Rho_ is internally translated to _Pi_ for use in a single _Executor_ that supports continuations natively.

## Hello World
```
rho> print("Hello, world")
Hello, world
rho>
```

## Example Test Suite
This is code taken directly from a [file](/Test/Language/TestRho/Scripts/Function.rho) in the test suite:

```
fun a(b, c)
	b + c

assert a(1, 2) == 3

fun d(e)
	fun f(g)
		g*2
	f(e)
  
assert d(2) == 4
assert exists a
assert exists d
assert !(exists f)

fun h(i,j,k)
	a(i, d(j)*k)

assert h(1,2,3) == 16
```
