# Colored Console Output

KAI supports colored console output for logs and test results, making it easier to distinguish between different types of messages (INFO, WARNING, ERROR) and improving overall readability.

## Features

- **Colored Log Messages**: Different types of log messages are displayed in distinct colors:
  - INFO messages in green
  - WARNING messages in yellow
  - ERROR and FATAL messages in red (with bold formatting)
  
- **Colored Test Output**: Test output messages use the same color scheme:
  - Grey for console metadata (e.g., line prefixes)
  - Green for INFO messages
  - Red (bold) for ERROR messages

- **Default Behavior**: Color output is enabled by default in all test programs.

## Command-Line Options

The following command-line options control colored output:

- `--debug-color` or `--color`: Explicitly enable colored output (redundant as color is on by default)
- `--no-color`: Disable colored output

Example usage:
```bash
# Run with colored output (default)
./Bin/Test/TestPi

# Explicitly enable colored output
./Bin/Test/TestPi --color

# Disable colored output
./Bin/Test/TestPi --no-color
```

## Implementation Details

The color output system uses the [rang library](https://github.com/agauniyal/rang) for terminal color and formatting. Key components:

- Global color state in `Main.cpp`
- Integration with the `Logger` class for consistent coloring of all logs
- Grey console input formatting
- Support for output redirection (colors are forced on with `--color`, automatically disabled with output redirection unless `--color` is specified)

## Examples

Here's an example of colored output:

```
[2025-05-18 23:03:08] [INFO] TranslatorCommon::Append: Directly evaluated 2 Plus 3 = 5
[2025-05-18 23:03:08] [WARNING] Saved continuation is not valid, setting to empty continuation
[2025-05-18 23:03:08] [ERROR] Failed to execute operation
```

In a terminal, these would appear as:
- INFO message in green
- WARNING in yellow
- ERROR in red bold text

## Related Files

- [`Main.cpp`](/home/xian/local/KAI/Test/Source/Main.cpp): Command-line argument handling and global color settings
- [`TestCommon.h`](/home/xian/local/KAI/Test/Include/TestCommon.h): TestCout implementation with color support
- [`Debug.cpp`](/home/xian/local/KAI/Source/Library/Core/Source/Debug.cpp): Integration with Logger for consistent coloring
- [`Logger.cpp`](/home/xian/local/KAI/Source/Library/Core/Source/Logger.cpp): Color formatting for different log levels