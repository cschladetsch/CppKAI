# Language

This module defines a set of Languages.

Each language is in it's own static library, but they share a common core system.

Specifically, each language (Pi, Rho, Sigma, Tau) is implemented using the same core language systems defined in KAI/Language/Common.

## Pi

Post-fix notation with two stacks: one for data, one for context. Like Forth.

## Rho

In-fix notation. Translated to Pi code. Looks like Python.

## Sigma

In-fix notation with static types. Rho's syntax plus types; every program is type-checked, then translated to Rho. Headers are in `Sigma/`. See the [Sigma reference](../../../Doc/Sigma/README.md).

## Tau

An interface definition language (IDL) used to generate network agent and proxies.

## See Also

### Language Tutorials
- **[Pi Tutorial](../../../Doc/PiTutorial.md)** - Complete Pi language guide
- **[Rho Tutorial](../../../Doc/RhoTutorial.md)** - Comprehensive Rho language documentation  
- **[Sigma Reference](../../../Doc/Sigma/README.md)** - Statically typed Sigma language
- **[Tau Tutorial](../../../Doc/TauTutorial.md)** - Tau IDL complete reference
- **[Language Guide](../../../Doc/LanguageGuide.md)** - Multi-language overview

### Implementation Details
- **[Common Language System](../../../Doc/CommonLanguageSystem.md)** - Shared architecture
- **[Console Integration](../../../Source/App/Console/README.md)** - How languages work in the console

### Architecture Documentation
- **[Language System Architecture](../../../resources/diagrams/language-system-architecture.md)** - Complete language pipeline
- **[Main Documentation Hub](../../../Doc/Documentation.md)** - Central navigation point

