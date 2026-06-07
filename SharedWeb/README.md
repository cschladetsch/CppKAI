# KAI Shared Web

SharedWeb contains KAI-aware web components and browser-facing runtime adapters.
It is intended to be consumed by CppKAI demos and external KAI applications
without CppKAI depending on those consumers.

## Structure

- `components/` contains static HTML fragments used by existing demos.
- `styles/` contains shared KAI web styling.
- `src/components/` contains TypeScript custom elements.
- `src/runtime/` contains KAI runtime interfaces and shared types.
- `src/adapters/` contains platform bridge adapters.

The intended dependency direction is:

```text
KAI core -> SharedWeb -> consumers
```
