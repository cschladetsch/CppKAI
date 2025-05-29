# KAI Documentation Guide

This document serves as a central entry point to the KAI documentation. It explains the organization of the documentation and provides navigation links to different sections.

## Documentation Organization

The KAI documentation is organized into the following main categories:

### 1. Project Overview
- [Architecture](Architecure.md) - System architecture overview
- [Building](BUILD.md) - Build instructions
- [Out-of-Source Build](OUT_OF_SOURCE_BUILD.md) - Best practices for building
- [Installation](Install.md) - Installation guide
- [Style Guide](StyleGuide.md) - Code style conventions

### 2. Language System
- [Language Guide](LanguageGuide.md) - Overview of the KAI language system
- [Common Language System](CommonLanguageSystem.md) - Architecture shared across languages

#### Language-Specific Guides
- [Pi Language](PiTutorial.md) - Foundation language (stack-based)
- [Rho Language](RhoLanguage.md) - Application language (infix notation)
- [Tau Language](TauTutorial.md) - Interface definition language
  - [Tau Code Generation](TauCodeGeneration.md) - Code generation architecture and usage

### 3. Advanced Topics
- [Event System](EventSystem.md) - Modern C++23 multi-cast event implementation with variadic templates
- [Continuation Control](ContinuationControl.md) - Advanced control flow
- [Network Iteration](NetworkIteration.md) - Distributed computing patterns
- [Meaning](Meaning.md) - Philosophical foundations of KAI

### 4. Networking System
- [Networking Overview](Networking.md) - Main networking documentation
  - [Network Architecture](NetworkArchitecture.md) - Detailed architecture
  - [Peer-to-Peer Summary](PeerToPeerSummary.md) - P2P implementation summary
  - [Peer-to-Peer Networking](PeerToPeerNetworking.md) - Detailed P2P documentation
  - [Network Performance](NetworkPerformance.md) - Performance optimization
  - [Network Security](NetworkSecurity.md) - Security features
  - [Connection Testing](ConnectionTesting.md) - Testing procedures
  - [Network Iteration](NetworkIteration.md) - Distributed operations
  - [Network Tau Interfaces](NetworkTauInterfaces.md) - Tau language network support

### 5. Technical Documentation
- [Console](Console.md) - Interactive REPL console with shell integration
- [Color Output](ColorOutput.md) - Console color formatting
- [Log Format](LogFormat.md) - Logging format specification
- [Null Registry Fix](NullRegistryFix.md) - Registry null pointer fix documentation

### 6. Testing
- [Test Guide](Test.md) - How to run and write tests
- [Test Fixes Summary](Test-Fixes-Summary.md) - Summary of test fixes
- [Test Improvements](Test-Improvements.md) - Test suite improvements
- [Test Summary Report](test_summary_report.md) - Current test status

### 7. Analysis and Fixes
- [Rho Fix Documentation](Rho-Fix-Documentation.md) - Details on recent Rho language fixes
- [Rho Analysis](Rho-Analysis.md) - Analysis of Rho language implementation
- [Rho Issues](Rho-Issues.md) - Known Rho language issues
- [Rho Regression Analysis](Rho-Regression-Analysis.md) - Regression test analysis
- [Tau Analysis](Tau-Analysis.md) - Analysis of Tau language implementation
- [Core Review](core_review_updated.md) - Core system review

## Getting Started

New users should start with the following documents in order:

1. **Project Setup**: [OUT_OF_SOURCE_BUILD](./OUT_OF_SOURCE_BUILD.md)
2. **System Overview**: [Architecture](Architecure.md)
3. **Language Introduction**: [Language Guide](LanguageGuide.md)
4. **Main Language**: [Rho Language](RhoLanguage.md) and [Rho Tutorial](RhoTutorial.md)

## Running Demos

The project includes several demos to showcase functionality:

- **Rho Language Demo**: Run `./Scripts/run_rho_demo.sh` to see the Rho language in action
- **Console Demo**: After building, run `./bin/Console` for an interactive environment with shell integration
- **Shell Command Demo**: See `Test/ShellCommandTests/demo_simple.txt` for shell command examples

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

- **Modern Event System**: New C++23 implementation of the multi-cast event system
- **Enhanced Rho Documentation**: Comprehensive coverage of the fixed binary operations
- **New Demo Script**: Interactive demonstration of all Rho language features
- **Build Documentation**: Improved guidance on building and testing the system
- **Cross-References**: Better navigation between related documents