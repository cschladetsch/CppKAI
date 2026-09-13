# Installing

KAI is cross-platform across Windows, Linux, and macOS, with a mix of
internal components and a handful of git-submodule dependencies (ENet for
networking, imgui for the optional windowed frontend, and others under
`Ext/`). Boost has been fully removed — there is nothing else to install
before building. If a third-party frontend dependency (e.g. imgui) can't be
found, you'll still get the [TestSuite](/Test) and the colored-text
[Console](Source/App/Console).

## Downloading

```bash
git clone https://github.com/cschladetsch/CppKAI.git
cd CppKAI
git submodule init
git submodule update --recursive
```

## Building

See [BUILD.md](BUILD.md) for full instructions, including all CMake options.
Short version:

**Windows (native, Clang + Ninja by default):**
```powershell
py build.py
py run.py console
```

**Linux / macOS:**
```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

## Output

- `Bin/` — executables (`Console`, `ImGui` when built, etc.)
- `Bin/Test/` — test suite binaries (`TestCore`, `TestPi`, `TestRho`, `TestTau`, `TestNetwork`, ...)
