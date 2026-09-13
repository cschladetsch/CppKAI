# KAI Windows Port - File Placement Guide

Extract this package to your CppKAI repo root, then place files as follows:

| File | Destination |
|------|-------------|
| build.py | repo root (alongside b, be) |
| run.py | repo root |
| README.md | repo root (replaces existing) |
| Main.cpp | Source/App/Console/Source/Main.cpp |
| fix_console.py | repo root (run once, then delete) |

## After extracting

1. Run the Console fix script once:
   py fix_console.py

2. Then build:
   py run.py console

## Notes

- fix_console.py wraps popen/pclose calls in #ifndef _WIN32 guards
- Main.cpp replaces getopt with a stdlib-only arg parser
- build.py defaults to Clang + Ninja with shell/backtick syntax disabled (pass `--enable-shell` to turn it on); pass `--msvc` for VS 2022/2026 + vcpkg instead
- run.py delegates all build logic to build.py
