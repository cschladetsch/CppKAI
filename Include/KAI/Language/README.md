# Language

This module defines a set of Languages.

Each language is in it's own static library, but they share a common core system.

Specifically, each language (Pi, Rho Tau) are all implemented using the same core language systems defined in KAI/Language/Common.

## Pi

Post-fix notation with two stacks: one for data, one for context. Like Forth.

## Rho

In-fix notation. Translated to Pi code. Looks like Python.

## Tau

An interface definition language (IDL) used to generate network agent and proxies.

## See Also

### Language Tutorials
- **[Pi Tutorial](../../../Doc/PiTutorial.md)** - Complete Pi language guide
- **[Rho Tutorial](../../../Doc/RhoTutorial.md)** - Comprehensive Rho language documentation  
- **[Tau Tutorial](../../../Doc/TauTutorial.md)** - Tau IDL complete reference
- **[Language Guide](../../../Doc/LanguageGuide.md)** - Multi-language overview

### Implementation Details
- **[Common Language System](../../../Doc/CommonLanguageSystem.md)** - Shared architecture
- **[Console Integration](../../../Source/App/Console/README.md)** - How languages work in the console

### Architecture Documentation
- **[Language System Architecture](../../../resources/diagrams/language-system-architecture.md)** - Complete language pipeline
- **[Main Documentation Hub](../../../Doc/Documentation.md)** - Central navigation point

