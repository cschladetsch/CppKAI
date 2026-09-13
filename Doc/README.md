# KAI Documentation

This directory contains documentation for the KAI distributed object model
and language system.

## Quick Start Guides

- **[BUILD.md](BUILD.md)** - Building KAI from source (all platforms, all CMake options)
- **[Install.md](Install.md)** - Installation instructions
- **[Documentation.md](Documentation.md)** - Main documentation index
- **[Test.md](Test.md)** - Running the test suites and filtering failures

## Current Status

- Networking is built by default (`KAI_NETWORKING=ON`); pass `-DKAI_NETWORKING=OFF` to skip it.
- Shell/backtick syntax is disabled by default (`ENABLE_SHELL_SYNTAX=OFF`); pass `-DENABLE_SHELL_SYNTAX=ON` to enable it.
- See **[TEST_SUMMARY.md](TEST_SUMMARY.md)** for the current test status snapshot and **[TODO.md](TODO.md)** for currently-tracked gaps and known-failing tests.
- See **[REVIEW.md](REVIEW.md)** for a full architectural/code review of the codebase.

### Language Tutorials
- **[PiTutorial.md](PiTutorial.md)** - Pi stack-based language tutorial
- **[RhoTutorial.md](RhoTutorial.md)** - Rho infix language tutorial
- **[TauTutorial.md](TauTutorial.md)** - Tau IDL tutorial
- **[LanguageGuide.md](LanguageGuide.md)** - Multi-language overview

## Core System Documentation

### Architecture
- **[Architecure.md](Architecure.md)** - System architecture overview
- **[CommonLanguageSystem.md](CommonLanguageSystem.md)** - Language integration

### Networking
- **[Networking.md](Networking.md)** - Network system overview
- **[NetworkArchitecture.md](NetworkArchitecture.md)** - Network design
- **[PeerToPeerNetworking.md](PeerToPeerNetworking.md)** - P2P communication
- **[NetworkSecurity.md](NetworkSecurity.md)** - Security considerations
- **[NetworkTauInterfaces.md](NetworkTauInterfaces.md)** - Network interfaces in Tau
- **[NetworkPerformance.md](NetworkPerformance.md)** - Performance optimization

### Console System
- **[Console.md](Console.md)** - Interactive console documentation
- **[CONSOLE_NETWORKING.md](CONSOLE_NETWORKING.md)** - Console-to-console networking
- **[ColorOutput.md](ColorOutput.md)** - Console color and formatting

### Code Generation
- **[TauCodeGeneration.md](TauCodeGeneration.md)** - Tau code generation system
- **[EventSystem.md](EventSystem.md)** - Event handling and callbacks

### Language-Specific Documentation

#### Pi Language
- **[Meaning.md](Meaning.md)** - Pi language semantics
- **[ClosuresVsContinuations.md](ClosuresVsContinuations.md)** - Advanced Pi concepts
- **[ContinuationControl.md](ContinuationControl.md)** - Continuation control flow

#### Rho Language
- **[RhoLanguage.md](RhoLanguage.md)** - Rho language specification

#### Tau Language
- **[TauFormalDefinition.md](TauFormalDefinition.md)** - Tau formal definition
- **[TauArchitectureDiagrams.md](TauArchitectureDiagrams.md)** - Tau architecture

## Testing Documentation
- **[Test.md](Test.md)** - Testing guide and best practices
- **[TEST_SUMMARY.md](TEST_SUMMARY.md)** - Current test status snapshot

## Maintenance
- **[StyleGuide.md](StyleGuide.md)** - Code style guidelines
- **[TODO.md](TODO.md)** - Tracked gaps, in-progress work, known-failing tests

## Images and Diagrams
- **Images/** - Architecture diagrams and screenshots

## Historical / Superseded Reports

The documents below are point-in-time investigation notes, fix write-ups,
and status reports from earlier work sessions. They are kept for reference
but describe past states of the code — do **not** treat them as current.
Where they conflict with TODO.md, TEST_SUMMARY.md, or REVIEW.md, those three
are authoritative.

- Code reviews: `core_review.md`, `core_review_updated.md`, `KAI_Review.md` (superseded by [REVIEW.md](REVIEW.md))
- Rho investigation notes: `Rho-Analysis.md`, `Rho-Findings.md`, `Rho-Fix.md`, `Rho-Fix-Documentation.md`, `Rho-Issues.md`, `Rho-Regression-Analysis.md`, `Rho-SemicolonIssue.md`, `RhoTestFailureAnalysis.md`, `RhoTestStatusUpdate.md`, `RhoIterationMethodsReport.md`, `RhoModelTrainingPlan.md`
- Tau investigation notes: `Tau-Analysis.md`
- Test status snapshots: `TEST_BASELINE_REPORT.md`, `test_summary_report.md`, `Test-Fixes-Summary.md`, `Test-Improvements.md`, `Test-Suite-Summary-Feature.md`, `Test-Summary-Feature.md`, `TestStatusFinal.md`, `DisabledTestsAnalysis.md`
- Control-flow fix write-ups: `DoWhileStatus.md`, `DoWhileCombinedTestsDescription.md`, `WhileAndDoWhileTestsDescription.md`
- Migration/fix write-ups: `SmartPointerMigration.md`, `SmartPointerMigrationPlan.md`, `SmartPointerMigrationProgress.md`, `NullRegistryFix.md`, `raw_pointer_analysis.md`, `fixes-summary.md`, `future_work.md`
- Networking session notes: `ConnectionTesting.md`, `NetworkCalculationTest.md`, `NetworkIteration.md`, `NetworkingChanges.md`, `PeerToPeerSummary.md`, `PROJECT_ANALYSIS.md`

## Related Resources

- **[Main README](../README.md)** - Project overview
- **[Examples](../Examples/)** - Code examples
- **[Test Documentation](../Test/)** - Test guides

## Documentation Standards

When contributing documentation:
1. Use Markdown format (.md extension)
2. Include clear headings and code examples where appropriate
3. Link to related documentation
4. Keep information current with code changes — update TODO.md and
   TEST_SUMMARY.md together whenever test status changes
5. A one-off investigation or fix write-up belongs under "Historical /
   Superseded Reports" above, not the main navigation — fold anything still
   true into the relevant living doc (BUILD.md, TODO.md, TEST_SUMMARY.md,
   REVIEW.md, or a language/architecture guide) instead of leaving it to be
   rediscovered as a standalone file
