# Pi ![Pi](/Images/Pi.png)
[![Build status](https://ci.appveyor.com/api/projects/status/github/cschladetsch/kai/Source/Library/Language/Pi/?svg=true)](https://ci.appveyor.com/project/cschladetsch/kai)
[![CodeFactor](https://www.codefactor.io/repository/github/cschladetsch/kai/badge)](https://www.codefactor.io/repository/github/cschladetsch/kai/Source/Library/Language/Pi/)
[![License](https://img.shields.io/github/license/cschladetsch/flow.svg?label=License&maxAge=86400)](./LICENSE.txt)
[![Release](https://img.shields.io/github/release/cschladetsch/flow.svg?label=Release&maxAge=60)](https://github.com/cschladetsch/kai/releases/latest)

Pi is a simple post-fix language. Meant to be fast for a machine to parse and execute. Not so great for humans.

Will be easier to use for those with backgrounds with HP48 calculators.

## Hello, World

`"Hello, world" print &`

This first pushes the string literal `"Hello, world"` onto the data stack. It then pushes the the `print` function onto the stack. The ampersand `&` means "continue execution using the top of the data stack".

The result is that hello world is printed to stdout:

```
pi> "Hello, world" print &
Hello, world
pi>
```

The same for adding two numbers: `1 2 +` yields `3`, etc. See [Reverse Polish Notation](https://en.wikipedia.org/wiki/Reverse_Polish_notation).

It's worth noting how simple it is to write functions in C++ that are addressable by _Pi_:

```cpp
void Print(Object obj)
{
   std::cout << obj.ToString();
}
```

## Integration with Executor

Pi code is executed by the Executor component, which implements all the stack operations, arithmetic functions, and control flow constructs needed by the language. The Executor handles Pi operations in a type-safe and efficient manner, with appropriate error handling for operations like:

- Stack manipulation (Dup, Swap, Drop, Over)
- Variable operations (Store with `#`, Retrieve with `@`)
- Arithmetic (Plus, Minus, Multiply, Divide)
- Logical operations (And, Or, Xor, Not)
- Comparison (Equiv, Less, Greater)
- Array and collection handling

All Pi operations preserve the stack-based semantics while ensuring type safety. For example, the `Dup` operation duplicates the top item on the stack, ensuring both the original and the duplicate maintain their full type information.

## A brief example
```
pi> { 'a # a 3 * } 4 swap & 12 == assert
pi>
```

This makes a contuation with contents `{ 'a # a 3 * }` and pushes it onto the stack. It then pushes the integer `4` onto the stack and swaps the top two elements, so the stack now looks like:

```
[1] 4
[0] { 'a # a 3 * }
```

It then runs the continuation using the `&` operator. The continuation first stores the top the stack, `4`, into local variable `a` using the `#` operator. It then pushes `a` onto the stack (resulting in `4`), then the integer `3`, then muliplies them with the `*` operator. It then asserts that the top of the stack (TOS) is equivalent to `12`.

## Stacks
There are two stacks in _Pi_:

### Data Stack
Provides context for *operations* such as add, multiply, call method etc.

### Context Stack
Provides context for *continuations*. It keeps track of where we've come from and where we're going. You can freely manipulate the Context Stack in _Pi_, unlike most other languages.

The reason this is very uncommon is that it can make things... hard to understand. However, it does allow for trivial control flows like continuations and tail recursion, as well as contextual jumps that are just not available in any other language system that does not provide direct manipulation of the context stack.

Of course, this is not meant to be used to make non-trivial code constructs. But it could, of course.

Rather, Pi is used to transmit data and code between Nodes efficiently, as well as to load-balance between nodes. 

There is a more conventional language, _Rho_, which transpiles to _Pi_ code. Rho looks a lot like Python and similar languages. It uses indents for scopeing etc, and provides common flow control and block scoping rules.

However, Rho can also directly access and inject Pi code using { curly braces } as expression objects. This makes for a uniquely rich programming environment.

Elements can be moved between the stacks.

## Keywords
There are a lot of _Pi_ keywords. Think of it as a CISC-like Forth interpreter.

The current _Pi_ command list is: [this is out of date]

```
true	# insert boolean true
false	# insert boolean false
assert	# assert that top of stack equates to true
#	# store a value to a name
@	# retrieve a value from a variable name
&	# push current context, then continue what is on top of data stack
...	# resume top of context stack
!	# replace current context with top of context stack
drop	# drop top of data stack
dup	# duplicate top of data stack
==	# apply equivalency operator to top two elements of data stack
swap	# swap top two elements of data stack
+	# add top two elements of data stack
-	# give difference of top two elements of data stack
*	# multiply top two elements of data stack
div	# divide top two elements of data stack
>	# pop top two elements of data stack and push true if second is greater than first, else false
<	# pop top two elements of data stack and push true if second is less than first, else false
>= # greator or equal
<= # less or equal
rot	# rotate the 3rd data stack element: A B C => B C A
rotn	# rotate the nth data stack element
over	# duplicate the second element of the data stack: A B => A B A
pickn	# duplicate the Nth element of datastack: A B C D 3 pickn => A B C D A
clear	# clear the data stack
gc	# perform garbage collection
size	# return size of container on top of stack
toarray	# make an array: 1 2 3 3 toarray => [1 2 3]
tolist	# an a list: 
todict	# make a dict
expand	# expand contents of a container, leaving the numnber of elements on TOS: [1 2 3] expand => 1 2 3 3
depth	# push current depth of data stack
&&	# perform logical and
new	# make a new object given a type 
xor	# perform exclusive or on top two elements on stack: 1 0 xor => true
or	# perform inclusive or: 0 0 or => false
if	# evaluate TOS. if true, execute 2nd element: "foo" true if => "foo"
ifelse	# evaluate TOS. if true, execute 2nd element, else 3rd: "foo" "bar" false iflse => "bar"
!=	# return true of top two elements are equivalent
freeze	# convert an arbirary compound object (even an Executor itself!) into a binary blob
thaw	# inverse of freeze
self	# push current top of context stack onto data stack
contents	# push contents of object on top of stack, including all properties, methods and dict entries
dropn	# drop N elemenets from data stack: 1 2 3 4 2 dropn => 1 2
todata   # move top of context stack to data stack
tocontext    # move top of data stack to context stack
```

For more information you can read the [Executor](/Source/Library/Executor/Source/Executor.cpp) code, and the unit tests.

## Literals
Integers are a sequence of numeric characters: `1`, `20`, `12313`.

Floats are `243.1575` etc. Exponentials are not currently supported.

Strings are surrounded by double quotes `"`. Escape internal quotes with a backslash `\"`

Lists are surrounded by `[` and `]`.

Continuations are surrounded by `{` and `}`.

All compounds are recursive.

### Comments
Comments start with a `//` and everything after that symbol on the same line is ignored. There are no multi-line 
comments as Pi is basically a sequence of tokens with little structure.

