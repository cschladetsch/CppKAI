#!/usr/bin/env python3
"""
run_window.py - Build then launch the ImGui/Window frontend.
 
Thin wrapper around `run.py window`, which configures with
KAI_BUILD_IMGUI=ON and builds/launches the ImGui target. Any arguments are
forwarded as-is, so build flags (--config, --no-network, --clean, ...) and
--no-build all work exactly as they do with run.py; use `-- <args>` to pass
arguments through to the ImGui executable itself.
 
Also makes sure a `cppcoder --serve` (CppLocalLlmCodeAssist) instance is
running on 127.0.0.1:8765 before launching the window, since the Assistant
tab talks to it over HTTP. If nothing is already listening there, one is
started here and torn down again once the window closes.
 
Usage:
  py run_window.py                   build + launch the Window app (Release)
  py run_window.py --config Debug    build + launch (Debug)
  py run_window.py --no-build        just launch the existing build
"""
 
import socket
import subprocess
import sys
import time
from pathlib import Path
 
REPO_ROOT = Path(__file__).resolve().parent
RUN_PY = REPO_ROOT / "run.py"
 
CPPCODER_HOST = "127.0.0.1"
CPPCODER_PORT = 8765
CPPCODER_STARTUP_TIMEOUT = 10  # seconds to wait for --serve to come up
 
 
def _port_open(host: str, port: int) -> bool:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(0.25)
        return s.connect_ex((host, port)) == 0
 
 
def ensure_cppcoder_serving():
    """Start `cppcoder --serve` if nothing is already listening on the
    Assistant tab's port. Returns the Popen handle if we started one,
    otherwise None (so we know not to kill someone else's server)."""
    if _port_open(CPPCODER_HOST, CPPCODER_PORT):
        print(f">>> cppcoder already serving on {CPPCODER_HOST}:{CPPCODER_PORT}")
        return None
 
    print(f">>> starting cppcoder --serve on {CPPCODER_HOST}:{CPPCODER_PORT}")
    proc = subprocess.Popen(
        ["cppcoder", "--serve"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
 
    deadline = time.monotonic() + CPPCODER_STARTUP_TIMEOUT
    while time.monotonic() < deadline:
        if _port_open(CPPCODER_HOST, CPPCODER_PORT):
            return proc
        if proc.poll() is not None:
            print(">>> WARNING: cppcoder --serve exited immediately; "
                  "Assistant tab will show a connection error.")
            return None
        time.sleep(0.2)
 
    print(">>> WARNING: cppcoder --serve did not come up in time; "
          "continuing anyway.")
    return proc
 
 
def main() -> None:
    cppcoder_proc = ensure_cppcoder_serving()
 
    cmd = [sys.executable, str(RUN_PY), "window"] + sys.argv[1:]
    print(f"\n>>> {' '.join(cmd)}\n")
    try:
        rc = subprocess.run(cmd).returncode
    finally:
        if cppcoder_proc is not None and cppcoder_proc.poll() is None:
            print(">>> stopping cppcoder --serve")
            cppcoder_proc.terminate()
            try:
                cppcoder_proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                cppcoder_proc.kill()
 
    sys.exit(rc)
 
 
if __name__ == "__main__":
    main()
