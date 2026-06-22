# Console ![Console](/Images/Console.png)

Interactive REPL console for the KAI language system with shell command integration.

## Features

- **Multi-language Support**: Pi (stack-based), Rho (infix), and Tau languages
- **Shell Command Integration**: Execute shell commands with backtick syntax
- **Automatic Stack Display**: Top-first stack visualization after each command,
  with `[0]` on the bottom line
- **Cross-platform**: Works on Windows, Linux, and macOS
- **Colored Output**: Strings and integers retain type cues; floating-point
  values use the normal neutral value color
- **Executor Inspection**: Machine-readable snapshots enumerate every live
  Executor and each Executor's own Tree, root, scope, and stack sizes
- **KAI Logging**: Console startup, Tree snapshots, debugger attachments/actions,
  and failures use the native `Logger`

## Shell Commands

Execute shell commands directly:
```
π `pwd
/home/user/project

π `ls -la | head -3
total 60
drwxr-xr-x  3 user user 4096 May 30 00:15 .
drwxr-xr-x 11 user user 4096 May 29 23:34 ..
```

Embed shell output in expressions:
```
π 10 `echo 5` +
[0]: 15

π 1 `echo 2` + 3 ==
[0]: true

ρ result = `echo 42`
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
- Automatic top-first stack visualization
- Multi-Executor Tree inspection and handle-targeted debugger operations
- Integration with Pi, Rho, and Tau language translators

For full documentation, see [Console Documentation](../../../Doc/Console.md).

## Console Documentation

### Quick Start and Usage
- **[Quick Start Guide](Source/QuickStartGuide.md)** - Get up and running quickly
- **[Console Networking](../../../Doc/CONSOLE_NETWORKING.md)** - Console-to-console communication
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
