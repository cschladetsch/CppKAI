# Building KAI

KAI uses CMake (3.28+) with a C++23 compiler. Boost has been fully removed
from KAI — no external dependency needs to be installed before building.

## Prerequisites

- CMake 3.28+
- C++23 compiler: Clang 16+ (default on Linux/macOS; also supported natively
  on Windows), GCC 13+, or MSVC 19.5+ (VS 2022/2026)
- Ninja (recommended; used automatically when available)
- Windows only: Python 3.10+ (drives `build.py`/`run.py`), and vcpkg if using
  `--msvc`

## Windows (native)

This is the primary, actively-used build path. `build.py` configures and
builds with Clang + Ninja by default (no vcpkg needed for that path); pass
`--msvc` to use MSVC + Visual Studio + vcpkg instead.

```powershell
git clone https://github.com/cschladetsch/CppKAI.git
cd CppKAI
git submodule init
git submodule update --recursive

py build.py                     # Release build (Clang + Ninja, shell syntax OFF)
py build.py --config Debug      # Debug build
py build.py --msvc              # MSVC + Visual Studio generator + vcpkg
py build.py --no-network        # Disable networking
py build.py --enable-shell      # Turn on backtick/shell syntax (see Security below)
py build.py --reconfigure       # Clean and reconfigure

py run.py console               # Build + launch Console (Pi mode)
py run.py rho                   # Build + launch Console in Rho mode
py run.py tests                 # Build + run all tests
py run.py test-pi               # Build + run TestPi only
py run.py demo                  # Build + run ContinuationMobilityDemo
py run.py console --no-build    # Just launch (skip build)
```

Run the Clang+Ninja path from a Developer PowerShell for VS (or the
`x64 Native Tools Command Prompt`) so Clang can find the MSVC headers/libs it
still links against on Windows.

## Linux / macOS

There is no top-level convenience script (`./b`, `Makefile`, etc.) in this
tree — build with plain CMake, optionally via `Scripts/build.sh` for a quick
Debug build:

```bash
git clone https://github.com/cschladetsch/CppKAI.git
cd CppKAI
git submodule init && git submodule update

# Quick Debug build
./Scripts/build.sh

# Or a standard out-of-source build with full control over options
mkdir -p build && cd build
cmake ..
cmake --build . -j"$(nproc)"
```

Binaries (Console, test suites, etc.) are written to `Bin/` and `Bin/Test/`
at the repository root regardless of which of the above you use, because
`BIN_HOME` defaults to `<repo root>/Bin`.

## CMake Build Options

```bash
cmake .. -DBUILD_GCC=ON                   # Use GCC instead of Clang (default: Clang)
cmake .. -DCMAKE_BUILD_TYPE=Release       # Debug is CMake's implicit default here
cmake .. -DKAI_BUILD_TEST_ALL=ON          # Build test targets (default: ON)
cmake .. -DKAI_BUILD_CORE_TEST=ON         # Build core/unit tests (default: ON)
cmake .. -DKAI_BUILD_TEST_LANG=ON         # Build language tests (default: ON)
cmake .. -DKAI_BUILD_IMGUI=ON             # Build the ImGui frontend (default: OFF)
cmake .. -DKAI_BUILD_LLM=ON               # Build local model-cache support (default: OFF)
cmake .. -DKAI_NETWORKING=ON              # Networking, Tau network codegen, TestNetwork (default: ON)
cmake .. -DENABLE_SHELL_SYNTAX=ON         # Backtick/shell command execution (default: OFF)
cmake .. -DKAI_ENABLE_TRACE=ON            # KAI_TRACE()/KAI_DEBUG_TRACE diagnostic logging to Logs/kai.log (default: OFF)
```

`KAI_BUILD_WINDOW` is a deprecated alias for `KAI_BUILD_IMGUI`.

When LLM support is enabled, models are stored in `~/.cache/deepseek/models`
by default, or `$XDG_CACHE_HOME/deepseek/models` if set. The cache is backed
by `Ext/CppLmmModelStore`.

### Security Configuration

**Shell Command Integration is disabled by default** (`ENABLE_SHELL_SYNTAX=OFF`).
Evaluating Pi/Rho source containing a backtick expression (`` `command` ``) or
a `$ command` line runs a real shell command with the Console process's
privileges, so this is opt-in, not opt-out. On native Windows, when enabled,
backtick/shell commands are routed through WSL2's bash (`wsl.exe`) — a WSL2
distro with bash/coreutils and `wsl` on PATH is required there. Linux/macOS/WSL2
use the system shell directly.

```bash
cmake .. -DENABLE_SHELL_SYNTAX=ON

# Or on Windows
py build.py --enable-shell
```

Only enable this in trusted environments.

## Running Applications

### Console

```bash
./Bin/Console                    # Interactive Pi mode
./Bin/Console --help             # Show all options
./Bin/Console -l rho             # Start in Rho mode
./Bin/Console script.pi          # Execute a script
./Bin/Console -t 2 script.rho    # Execute with trace level 2
```

The Console automatically copies itself to `~/bin/Console` during the build
if that directory exists.

### Window (ImGui frontend)

Built as `Bin/ImGui` when `KAI_BUILD_IMGUI=ON`:

```bash
mkdir -p build && cd build
cmake .. -DKAI_BUILD_IMGUI=ON
cmake --build . --target ImGui
./Bin/ImGui   # from the repo root
```

### Running Tests

After building, test binaries are written to `Bin/Test/` at the repository
root:

```bash
./Bin/Test/TestCore
./Bin/Test/TestPi
./Bin/Test/TestRho
./Bin/Test/TestTau
./Bin/Test/TestNetwork          # Only built when KAI_NETWORKING=ON (default)

# Filter a specific test
./Bin/Test/TestPi --gtest_filter="*Continuation*"
```

See [Doc/Test.md](Test.md) for more on filtering and interpreting failures,
and [Doc/TEST_SUMMARY.md](TEST_SUMMARY.md) for the current per-suite status.

## Common Issues

- If compilation fails with C++23 features not recognized, use a newer
  compiler (Clang 16+, GCC 13+, MSVC 19.5+).
- If tests fail intermittently or crash on a fresh checkout after a merge,
  do a clean rebuild:
  ```bash
  rm -rf build          # Unix
  rmdir /S /Q build     # Windows
  ```
  or `py build.py --reconfigure` on Windows.

## Development Best Practices

1. Always use an out-of-source `build/` directory — never run CMake directly
   in the source tree.
2. Never commit build artifacts.
3. If you hit strange build issues, try a fresh `build/` directory before
   anything else.
