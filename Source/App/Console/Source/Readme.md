# KAI Console Documentation

## What is the KAI Console?

The KAI Console is an interactive REPL (Read-Eval-Print-Loop) for the Pi and Rho programming languages. Pi is a stack-based language, while Rho provides a more traditional syntax.

## Key Features

1. **Multi-Language Support**: Switch between Pi and Rho languages
2. **Integrated Shell Access**: Execute system commands without leaving the Console
3. **Advanced History**: Zsh-like history expansion for maximum productivity
4. **Stack Visualization**: See your computation stack after each operation

## Documentation Index

### Getting Started
- [**QuickStartGuide.md**](QuickStartGuide.md) - Essential commands and basic usage
- [**ShellModeDemo.md**](ShellModeDemo.md) - How to use shell commands (3 methods!)
- [**TestAllFeatures.md**](TestAllFeatures.md) - Complete feature test checklist

### Core Features
- [**TestZshFeatures.md**](TestZshFeatures.md) - Overview of all zsh-like features implemented
- [**ZshQuickReference.md**](ZshQuickReference.md) - Quick reference card for all commands
- [**AdvancedZshFeatures.md**](AdvancedZshFeatures.md) - Detailed documentation of advanced features

### Interactive Examples
- [**VisualDemo.md**](VisualDemo.md) - Visual demonstration of key features
- [**CommonUsage.md**](CommonUsage.md) - Common usage patterns and examples
- [**InteractiveExamples.md**](InteractiveExamples.md) - Comprehensive interactive examples
- [**InteractiveDemo.md**](InteractiveDemo.md) - Step-by-step interactive tutorial

### Demo Sessions
- [**TypicalSession.md**](TypicalSession.md) - Complete typical usage session
- [**AdvancedDemo.md**](AdvancedDemo.md) - Advanced feature demonstrations
- [**ClarificationDemo.md**](ClarificationDemo.md) - Common misconceptions clarified

## Quick Start

### Basic Pi Programming
```console
Pi λ 2 3 +         # Stack-based: push 2, push 3, add
[0]: 5

Pi λ 10 20 * 
[0]: 200

Pi λ fun double { 2 * }   # Define a function
Pi λ 7 double
[0]: 14
```

### Shell Integration (3 Methods)

Sometimes you need to interact with your system while programming. The Console provides three ways:

1. **Backticks**: `` `ls -la` `` - Run a shell command and continue in Pi
2. **Dollar Prefix**: `$ ls -la` - Quick shell command (like a shell alias)
3. **Shell Mode**: `sh` - Switch to full shell mode (prompt changes to "Bash λ")

### History Expansion Features

1. **Basic History**: `!!`, `!n`, `!-n`, `!string`
2. **Word Designators**: `:0`, `:^`, `:$`, `:*`, `:n`, `:n-m`, `:n*`
3. **Quick Substitution**: `^old^new^`
4. **Search Anywhere**: `!?string?`
5. **Argument Shortcuts**: `!$`, `!^`
6. **Modifiers**: `:h`, `:t`, `:r`, `:e`, `:u`, `:l`, `:q`, `:x`
7. **Substitutions**: `:s/old/new/`, `:gs/old/new/`

## Example Usage

### Why Shell Integration?

When programming, you often need to:
- Check files: `$ ls *.txt`
- See where you are: `$ pwd`
- Process data: `$ grep pattern file.txt`
- Save results: `$ echo "result" > output.txt`

### Shell Command Examples
```console
Pi λ # Calculate something
Pi λ 100 25 /
[0]: 4

Pi λ # Save result to file using shell
Pi λ $ echo "Division result: 4" > calc_result.txt

Pi λ # Or use backticks to embed shell output in Pi
Pi λ print "Current directory: `pwd`"
[0]: "Current directory: /home/user/KAI"

Pi λ # For extended shell work, use shell mode
Pi λ sh
Entering shell mode. Type 'exit' to return to Pi mode.
Bash λ grep -r "function" *.cpp | wc -l
42
Bash λ exit
Pi λ # Back to Pi programming
```

### History Expansion Examples
```console
Pi λ fun calculate { + * 2 }    # Made a mistake?
Pi λ ^+^dup +^                  # Quick fix!
=> fun calculate { dup + * 2 }

Pi λ process_file input.txt output.json
Pi λ !!:s/json/xml/            # Change output format
=> process_file input.txt output.xml

Pi λ /path/to/some/file.txt
Pi λ !!:h                      # Get directory: /path/to/some
Pi λ !!:t                      # Get filename: file.txt
```

## Implementation

The features are implemented in:
- `/Include/KAI/Console/Console.h` - Header with new method declarations
- `/Source/Library/Executor/Source/Console.cpp` - Core implementation
- `/Source/App/Window/Source/ExecutorWindow.cpp` - Window app integration

## Testing

See `/Test/Console/` for:
- Unit tests (TestConsoleZshFeatures.cpp, TestAdvancedZshFeatures.cpp)
- Shell test scripts (RunConsoleTests.sh)
- Python test suite (TestConsoleZsh.py)
- Interactive test cases (InteractiveTests.txt)

All features work identically in both Console and Window applications!