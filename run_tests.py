#!/usr/bin/env python3
"""
run_tests.py - Build then run the full CppKAI test suite.

Thin wrapper around `run.py tests`. Any arguments are forwarded as-is, so
build flags (--config, --no-network, --clean, ...) and --no-build all work
exactly as they do with run.py.

Usage:
  py run_tests.py                    build + run all tests (Release)
  py run_tests.py --config Debug     build + run all tests (Debug)
  py run_tests.py --no-build         just run the existing build's tests
"""

import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent
RUN_PY = REPO_ROOT / "run.py"


def main() -> None:
    cmd = [sys.executable, str(RUN_PY), "tests"] + sys.argv[1:]
    print(f"\n>>> {' '.join(cmd)}\n")
    sys.exit(subprocess.run(cmd).returncode)


if __name__ == "__main__":
    main()
