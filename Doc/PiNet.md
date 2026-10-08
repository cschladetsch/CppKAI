# PiNet

PiNet is the rule a continuation has to satisfy before it can be sent to
another node:

> A continuation that travels may only use names it binds itself.

Everything it needs from the sender comes in on the stack, or is bound inside
the payload. Nothing is looked up on the sender, proxied, or shared, so there
are no races and no faults that only show up on the far node. If the rule is
broken, `send` fails at once, on the sender, and names what is at fault.

```
{ 'n # n n * }      // can travel: n is bound by the block
{ a + }             // refused:
PiNet: not transportable: it uses 'a', which it does not bind. Pass values
in on the stack, or bind them in the payload with 'name #
```

## Why

Pi resolves a bare name when it runs, in this order: the continuation's own
scope, the scope of every continuation on the context stack, then the tree.
On the sender, `{ a + }` finds `a` in whatever called it. On another node it
finds whatever `a` means there, or nothing. The block looks self-contained
and isn't. PiNet makes that visible at the only point that matters, the
send.

## The rule

| | |
|---|---|
| **Bound** | `'x #` anywhere in the block, or in a block enclosing it within the payload; a function's parameters (the continuation's arguments, as Rho and Sigma produce) |
| **Used** | a bare name `x` or `a/b`; a quoted name given to `@` (Retreive), Lookup, Assign or Remove |
| **Allowed unbound** | system names every node has: the children of `/Bin`, `/Sys` and `/Types` |
| **Never allowed** | an absolute path outside those three, and operations that reach into the running node: `self`, `this`, the current context and continuation, scope access, `cd`, `ls`, the executor, ExecFile |

A quoted name that is only data, such as a peer name, is not a use. Nested
blocks see the bindings of the blocks around them, since they travel
together; a binding in a nested block is not visible outside it.

The check is flow-insensitive: a name bound anywhere in its block counts as
bound throughout that block.

## Payloads

`send` checks whatever it is given:

- a `Continuation`: checked, along with every continuation in its code;
- an `Array`: each element is checked;
- a frozen `BinaryStream`: thawed to look inside, then rewound for sending;
- anything else is plain data and passes.

## Functions from Rho and Sigma

A function's parameters are bound, but other functions it calls are names
like any other. In

```sigma
fun tens(n: int) -> int
    return n * 10

fun scaled(n: int) -> int
    return tens(n) + 1
```

`scaled` uses `tens`, which it does not bind, so `scaled` cannot be sent on
its own: send both, or have the receiver provide `tens`. Globals are the
same: a function that reads a global `y` is refused.

## Where it lives

PiNet is in CppKAI (`Include/KAI/Language/PiNet`,
`Source/Library/Language/PiNet`, the `PiNetLang` library), not in CppKaiCore,
and CppKaiConsoleLib does not depend on it. ConsoleLib has a generic hook,
`Console::AddSendCheck`, which runs every registered check before `send`
does anything else; the Console app registers PiNet with it:

```cpp
console.AddSendCheck([&console](Object payload) {
    PiNet::Require(payload, &console.GetTree());
});
```

`PiNet::Check` returns a report (unbound names, and what reaches into the
node) without throwing, for tools that want to show it.

## Tests

`Test/Language/TestPiNet` (20 tests): what passes, what fails, payload
kinds, the message, `send` refusing before it checks the network, and
functions compiled from Sigma.

## Not covered

- Pi's `send` word doesn't currently reach `/Bin/send` at all, with or
  without PiNet. PiTranslator emits it as an unquoted `send` followed by
  Lookup, so the function is resolved first and Lookup is handed the
  function itself ("Looking up: Unknown type", then a StringStream "Not
  Implemented"). Quoting the label isn't enough: Lookup then doesn't strip
  the quote, and nothing is called. The tests call `/Bin/send` directly,
  which is the function the check is attached to.
- Other ways of moving continuations between nodes, such as Tau proxies,
  don't go through `Console::send` and aren't checked yet.
- Pi has no literal for an absolute path (`/Home/x` is division), so the
  absolute-path rule applies to Pathnames built by other code.
