# KAI Project Overview

## CppKAI (Core Runtime)
CppKAI is the foundational C++ repository implementing a multi-language runtime system. It includes:
- **Core Engine**: Object system, garbage collection, and stack-based executor.
- **Languages**: Interpreters for Pi, Rho, Tau, Lisp, and Hlsl.
- **Networking**: Peer-to-peer networking capabilities.
- **Tooling**: Debugging, logging, and LLM-assisted code analysis tools.

## SharedWeb
SharedWeb contains KAI-aware web components, shared browser styling, and TypeScript runtime adapter interfaces. It sits above the CppKAI runtime and below demos or external KAI applications.

## Relationship
CppKAI serves as the core library and runtime. SharedWeb depends on KAI concepts and provides reusable web-facing surfaces for demos and downstream applications without introducing a dependency from CppKAI back to those consumers.
