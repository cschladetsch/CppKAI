# Library

These are the source codes for the libraries of the KAI system.

1. **Core**. The underlying required systems: describes what an Object is, the type traits system, Registry, Tree etc.
1. **Executor**. The virtual machine that runs on two stacks: one for data and one for context.
1. **Language**. The language library is quite sophisticated (but not complex!)
  * *LangCommon*. Contains common functionality for lexing, parsing, ast generation and walking. All parameterised over Token and AST Node types. Pi, Rho and Tau all share a great deal of functionality from *LangCommon*. This greatly reduces the amount of code required, at the expense of adding sophistication.
  * *PiLang*. The post-fix language mostly used for low-latency and low-bandwidth networking. 
  * *RhoLang*. In-fix language that looks and smells a lot like Python. Compiles to Pi code.
  * *TauLang*. An Interface Defition Language (IDL) used to generate Proxy and Agent code for Network library
1. **Network** Cross-platform networking system with distributed object model, type-safe RPC and Future results.
1. **Platform** Platform-specific code.

