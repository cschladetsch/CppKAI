#!/usr/bin/env python3
"""
run_window.py - Build then launch the ImGui/Window frontend.

Thin wrapper around `run.py window`, which configures with
KAI_BUILD_IMGUI=ON and builds/launches the ImGui target. Any arguments are
forwarded as-is, so build flags (--config, --no-network, --clean, ...) and
--no-build all work exactly as they do with run.py; use `-- <args>` to pass
arguments through to the ImGui executable itself.

Usage:
  py run_window.py                   build + launch the Window app (Release)
  py run_window.py --config Debug    build + launch (Debug)
  py run_window.py --no-build        just launch the existing build
"""

import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent
RUN_PY = REPO_ROOT / "run.py"


def main() -> None:
    cmd = [sys.executable, str(RUN_PY), "window"] + sys.argv[1:]
    print(f"\n>>> {' '.join(cmd)}\n")
    sys.exit(subprocess.run(cmd).returncode)


if __name__ == "__main__":
    main()
