# KAI Manifesto

KAI is a distributed object system for C++. It exists to make code, state, and execution visible, movable, and testable.

## Core Beliefs

- Objects should be first-class, not hidden behind ad hoc wrappers.
- Types should be reflected at runtime without requiring macros in user code.
- Execution state should be serializable, not trapped in a live process.
- Continuations are data. If they can be frozen, they can be moved, stored, resumed, and inspected.
- Network boundaries are transport details, not architectural walls.
- A feature is not complete until it survives binary serialization, network transport, and re-execution.

## Design Rules

- Prefer explicit state over implicit behavior.
- Prefer deterministic binary formats over ambiguous text interchange.
- Prefer narrow, verifiable protocols over clever shortcuts.
- Prefer local reasoning. If an object crosses a process boundary, its reconstruction must be bounded and exact.
- Prefer compatibility with tools, tests, and scripts over hidden runtime magic.

## What This Means

- A continuation must freeze cleanly, thaw cleanly, and execute the same way after transport.
- A networked object must behave the same whether it is local or remote.
- A failing test is a design signal, not an inconvenience.
- A skipped proof is a gap in the system, not a success.

## Non-Negotiables

- If state is serialized, the format must be self-describing or self-bounded.
- If execution is migrated, the payload must be complete enough to resume without guessing.
- If a test claims to prove behavior, it must actually run that path.
- If a demo claims to prove a capability, it must be runnable end to end.

## Goal

Build systems that can be understood, moved, restarted, and trusted.

If KAI cannot be frozen, shipped, thawed, and executed again, then the system is not finished.
