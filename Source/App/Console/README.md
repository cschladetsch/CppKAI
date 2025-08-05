# Console ![Console](/Images/Console.png)

Interactive REPL console for the KAI language system with shell command integration.

## Features

- **Multi-language Support**: Pi (stack-based), Rho (infix), and Tau languages
- **Shell Command Integration**: Execute shell commands with backtick syntax
- **Automatic Stack Display**: Colored stack visualization after each command
- **Cross-platform**: Works on Windows, Linux, and macOS
- **Colored Output**: Type-based coloring for better readability

## Shell Commands

Execute shell commands directly:
```
Pi λ `pwd
/home/user/project

Pi λ `ls -la | head -3
total 60
drwxr-xr-x  3 user user 4096 May 30 00:15 .
drwxr-xr-x 11 user user 4096 May 29 23:34 ..
```

Embed shell output in expressions:
```
Pi λ 10 `echo 5` +
[0]: 15

Pi λ 1 `echo 2` + 3 ==
[0]: true

Rho λ result = `echo 42`
[0]: 2
```

## Usage

Run the console after building:
```bash
./build/Bin/Console
```

## Implementation

The Console is built on top of the [Executor](/Source/Library/Executor/Source) and provides:
- Language-agnostic shell command preprocessing
- Automatic stack visualization with type-based coloring
- Integration with Pi, Rho, and Tau language translators

For full documentation, see [Console Documentation](../../../Doc/Console.md).

## Console Documentation

### Quick Start and Usage
- **[Quick Start Guide](Source/QuickStartGuide.md)** - Get up and running quickly
- **[Console Networking](../../../CONSOLE_NETWORKING.md)** - Console-to-console communication
- **[Implementation Summary](Source/ImplementationSummary.md)** - Technical implementation details

### Advanced Features
- **[Shell Mode Demo](Source/ShellModeDemo.md)** - Shell integration examples
- **[Interactive Demo](Source/InteractiveDemo.md)** - Comprehensive feature demonstration
- **[Advanced Features](Source/AdvancedZshFeatures.md)** - Power-user features

### Testing and Examples
- **[Console Tests](../../../Test/Console/README.md)** - Test suite documentation
- **[Typical Session](Source/TypicalSession.md)** - Example usage session

## Testing

Shell command functionality is tested in `Test/ShellCommandTests/` with over 50 test cases covering:
- Basic shell commands
- Embedded command substitution
- Error handling
- Cross-language support

The Pi and Rho language tests also indirectly test the Executor functionality.

