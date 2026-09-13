# Installing KAI

See [Doc/Install.md](Doc/Install.md) for installation instructions and
[Doc/BUILD.md](Doc/BUILD.md) for the full build reference (CMake options,
platform-specific steps, running tests).

Quick start:

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
