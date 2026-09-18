# Continuations as the Unit of Distribution: How KAI Makes Remote Calls Look Local

For the last twenty-odd years I've been building — on and off, between paying jobs — a
distributed object model for C++ called [KAI](https://cschladetsch.github.io/CppKAI/).
It started as a curiosity: what if the thing you serialize across a network wasn't a
function call, but the *entire suspended state of execution*? Turns out that idea, taken
seriously, reshapes almost everything else about the system: how the VM works, how
garbage collection has to behave, and how you write networked code without it looking
like networked code.

This post is about that core idea — continuations as the unit of distribution — and why
it's a genuinely different way to think about RPC than the usual proxy-stub approach.

## The usual way RPC lies to you

Every RPC framework — gRPC, Cap'n Proto, Thrift, your hand-rolled JSON-over-HTTP — makes
the same promise: call this function, and it'll look like a local call even though it
ran somewhere else. The lie is in the "looks like." Under the hood you get a stub that
blocks (or returns a future), marshals arguments, sends a message, waits, unmarshals a
result. The call boundary is *baked into the interface*. You know exactly where the
network is, because someone (an IDL compiler, usually) drew the line for you at compile
time.

That's fine, mostly. But it means the unit of distribution is *a function call*. Nothing
about the actual state of the calling code — what it was doing before, what it was about
to do next, what local variables were live — crosses the wire. It can't, because a
function call isn't a thing you can suspend and resume; it's just an event.

## What if the unit of distribution was the whole computation?

KAI's execution model isn't call-stack-based, it's continuation-based. A `Continuation`
in KAI is a first-class object representing an *ordered sequence of operations still to
be executed* — closer to a coroutine's state than to a stack frame. The executor doesn't
call functions; it fetches the next operation from a continuation, executes it against
two stacks (`Data` and `Context`), and either continues, suspends (`&`), resumes (`...`),
or replaces (`!`) the running continuation.

Because a continuation is just an `Object` in KAI's reflected type system, it can be
serialized through the same `BinaryStream` mechanism as any other value. Suspend a
computation, serialize the continuation, ship it to another `Node` over the wire,
deserialize it there, resume it. The computation doesn't know it moved. That's the whole
trick: distribution isn't a special code path, it's a side effect of continuations being
ordinary values.

```pi
& { compute-something } send-to-peer
```

is a real, if simplified, sketch of what this looks like in Pi (KAI's foundation
language) — suspend a computation, hand the suspended continuation off. The receiving
node resumes it as if it had always been running there.

## Why this needs a different kind of garbage collector

Once continuations move, the objects they reference move conceptually with them —
except the objects usually *don't* physically move, the network reference does. This is
where KAI's `Registry` — the combined factory/GC — earns its complexity. It needs to:

- Track object liveness across process boundaries, not just within one heap
- Avoid stop-the-world pauses, because a live continuation might be sitting on a network
  round-trip and you can't halt the whole VM to wait for it
- Collect incrementally, in small bounded steps, interleaved with normal execution

The answer is a tri-colour incremental collector (white/grey/black marking,
generational), running interleaved with the executor rather than as a separate phase.
Every `Object` is a managed reference through the `Registry`; nothing is ever a raw
pointer the collector can't see. That uniformity — the executor *never* touches a raw
C++ type, only `Object` — is what makes incremental collection tractable here. There's
no escape hatch where a continuation could be holding a reference the collector doesn't
know about.

## The Agent/Proxy layer this enables

On top of continuation mobility, KAI has a more conventional-looking RPC layer — `Agent`
(server-side handler), `Proxy` (client-side stub), `Domain` (a `Node` grouping with
`MakeAgent<T>()` / `MakeProxy<T>()` factories) — built on ENet over UDP. This is the part
that looks like normal RPC: you define an interface in Tau (KAI's IDL), generate a
Proxy/Agent pair, and call `Invoke<R>(...)` or `FetchProperty<T>(...)` and get back a
`Future<T>`.

```tau
interface ICalc {
    Future<int> Add(int a, int b);
    int Value;
}
```

What's different is that this conventional layer sits *on top of* the continuation
machinery rather than being the whole story. The Proxy/Agent pattern handles the "call a
known remote interface" case cleanly. Continuation mobility handles the harder case:
moving a computation you didn't plan in advance, mid-flight, to wherever makes sense —
load balancing a long computation across peers, migrating a coroutine-like task to a
node with better data locality, or resuming work after a node comes back online.

## What this cost, and what it's worth

Being honest about the tradeoffs: this is a genuinely harder system to build than a
proxy-stub RPC library, and it shows. Every value in KAI being a reflected, GC-managed
`Object` — rather than plain C++ types — is a real overhead compared to, say, Cap'n
Proto's zero-copy wire format. This isn't the fastest way to move bytes between machines.
It's a different bet: that the interesting distributed systems problems aren't about
shaving nanoseconds off serialization, they're about *where does execution state live,
and how cheaply can it move*.

Twenty years is a long time to work on one idea in the gaps between jobs. I don't know
that "continuations as the unit of distribution" is the right way to build distributed
systems in general — plenty of production systems get by fine without it. But it's a
genuinely different lens than the RPC-stub orthodoxy, and building a whole language
runtime, GC, and network layer around taking it seriously has been the most interesting
systems problem I've worked on.

The source, including the three languages, the VM, the GC, and the networking layer, is
on GitHub: [github.com/cschladetsch/CppKAI](https://github.com/cschladetsch/CppKAI).
There's a live architecture writeup and a working demo of continuation mobility at
[cschladetsch.github.io/CppKAI](https://cschladetsch.github.io/CppKAI/).

---

*Christian Schladetsch has been a C++ systems engineer for 30+ years, across games
(Path of Exile, Shatter), XR, and defence. He currently writes about local-first AI
tooling and systems programming at
[machinesdoitbetter.ai](https://machinesdoitbetter.ai/).*
