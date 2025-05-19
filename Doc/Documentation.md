# KAI Documentation Guide

This document serves as a central entry point to the KAI documentation. It explains the organization of the documentation and provides navigation links to different sections.

## Documentation Organization

The KAI documentation is organized into the following main categories:

### 1. Project Overview
- [Architecture](Architecure.md) - System architecture overview
- [OUT_OF_SOURCE_BUILD](./OUT_OF_SOURCE_BUILD.md) - Build instructions and best practices

### 2. Language System
- [Language Guide](LanguageGuide.md) - Overview of the KAI language system
- [Common Language System](CommonLanguageSystem.md) - Architecture shared across languages

#### Language-Specific Guides
- [Pi Language](PiTutorial.md) - Foundation language (stack-based)
- [Rho Language](RhoLanguage.md) - Application language (infix notation)
- [Tau Language](TauTutorial.md) - Interface definition language

### 3. Advanced Topics
- [Continuation Control](ContinuationControl.md) - Advanced control flow
- [Network Iteration](NetworkIteration.md) - Distributed computing patterns
- [Meaning](Meaning.md) - Philosophical foundations of KAI

### 4. Technical Documentation
- [Color Output](ColorOutput.md) - Console color formatting
- [Rho Fix Documentation](Rho-Fix-Documentation.md) - Details on recent Rho language fixes
- [Rho Analysis](Rho-Analysis.md) - Analysis of Rho language implementation

## Getting Started

New users should start with the following documents in order:

1. **Project Setup**: [OUT_OF_SOURCE_BUILD](./OUT_OF_SOURCE_BUILD.md)
2. **System Overview**: [Architecture](Architecure.md)
3. **Language Introduction**: [Language Guide](LanguageGuide.md)
4. **Main Language**: [Rho Language](RhoLanguage.md) and [Rho Tutorial](RhoTutorial.md)

## Running Demos

The project includes several demos to showcase functionality:

- **Rho Language Demo**: Run `./run_rho_demo.sh` to see the Rho language in action
- **Console Demo**: After building, run `./bin/Console` for an interactive environment

## Example Scripts

Example scripts for each language can be found in:

- Pi scripts: `Test/Language/TestPi/Scripts/*.pi`
- Rho scripts: `Test/Language/TestRho/Scripts/*.rho`
- Tau scripts: `Test/Language/TestTau/Scripts/*.tau`

## Documentation Conventions

Throughout the documentation:

- Code examples are provided in language-specific syntax highlighting
- External references are included as hyperlinks
- Command line instructions are shown in code blocks
- Key concepts are **bold** for emphasis

## Contributing to Documentation

When contributing to the documentation:

1. Keep language-specific details in the appropriate tutorial files
2. Update the main language guide when adding features that affect multiple languages
3. Create specific analysis or fix documents for technical changes
4. Ensure all code examples are correct and tested
5. Add cross-references between related documents

## Recent Updates

Recent documentation improvements include:

- **Enhanced Rho Documentation**: Comprehensive coverage of the fixed binary operations
- **New Demo Script**: Interactive demonstration of all Rho language features
- **Build Documentation**: Improved guidance on building and testing the system
- **Cross-References**: Better navigation between related documents