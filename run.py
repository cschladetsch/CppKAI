#!/usr/bin/env python3
"""
run.py - Build then run a chosen CppKAI target.

Delegates all build logic to build.py, then launches the selected executable.

Usage:
  py run.py console              build + launch Console (Pi mode)
  py run.py rho                  build + launch Console in Rho mode
  py run.py tests                build + run all tests
  py run.py window                build + launch the ImGui/Window frontend
  py run.py console --no-build   skip build, just launch
  py run.py console -- -l rho    pass args to the exe after --

See also: run_tests.py, run_window.py (thin wrappers around 'tests'/'window').
"""

import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent
BIN_DIR = REPO_ROOT / "Bin"
BUILD_PY = REPO_ROOT / "build.py"

TARGETS = {
    "console":      "Console",
    "pi":           "Console",
    "rho":          "Console",
    "tests":        None,
    "test-pi":      "TestPi",
    "test-rho":     "TestRho",
    "test-tau":     "TestTau",
    "test-network": "TestNetwork",
    "demo":         "ContinuationMobilityDemo",
    "window":       "ImGui",
    "repoindex":    "RepoIndex",
    "rhodataset":   "RhoDataset",
}

CONSOLE_MODE_ARGS = {
    "rho": ["-l", "rho"],
    "pi":  [],
}

BUILD_FLAGS = {
    "--config", "--ninja", "--no-network", "--llm", "--imgui",
    "--enable-shell", "--clean", "--reconfigure", "--no-submodules",
    "--no-build",
}

def usage():
    targets = "  ".join(TARGETS.keys())
    print(f"""usage: py run.py <target> [build-flags] [-- exe-args]

targets:
  {targets}

build flags:
  --no-build          skip build, just run
  --config <cfg>      Release (default), Debug, RelWithDebInfo, MinSizeRel
  --ninja             use Ninja generator if available
  --no-network        KAI_BUILD_NETWORKING=OFF
  --llm               KAI_BUILD_LLM=ON
  --enable-shell      enable backtick shell syntax
  --imgui             build the ImGui/Window frontend (KAI_BUILD_IMGUI=ON; implied by the 'window' target)
  --clean             wipe build/ before configuring
  --reconfigure       same as --clean
  --no-submodules     skip git submodule update

examples:
  py run.py console
  py run.py rho
  py run.py tests
  py run.py window
  py run.py console --config Debug
  py run.py console --no-build
  py run.py console -- script.rho
""")


def parse_args():
    argv = sys.argv[1:]

    if not argv or argv[0] in ("-h", "--help"):
        usage()
        sys.exit(0)

    target = argv[0]
    if target not in TARGETS:
        print(f"error: unknown target '{target}'")
        usage()
        sys.exit(1)

    # Split on -- to separate build flags from exe args
    if "--" in argv:
        split = argv.index("--")
        build_argv = argv[1:split]
        exe_argv = argv[split+1:]
    else:
        build_argv = argv[1:]
        exe_argv = []

    # Parse build flags
    no_build = False
    config = "Release"
    build_flags = []
    i = 0
    while i < len(build_argv):
        a = build_argv[i]
        if a == "--no-build":
            no_build = True
        elif a == "--config" and i + 1 < len(build_argv):
            config = build_argv[i+1]
            build_flags += ["--config", config]
            i += 1
        elif a in BUILD_FLAGS:
            build_flags.append(a)
        else:
            print(f"error: unknown flag '{a}'")
            usage()
            sys.exit(1)
        i += 1

    return target, no_build, config, build_flags, exe_argv


def build(build_flags, target_name):
    cmd = [sys.executable, str(BUILD_PY)] + build_flags
    if target_name:
        cmd += ["--target", target_name]
    print(f"\n>>> {' '.join(cmd)}\n")
    result = subprocess.run(cmd)
    if result.returncode != 0:
        sys.exit(result.returncode)


def resolve_exe(target, config):
    exe_name = TARGETS[target]
    if exe_name is None:
        return None
    candidates = [
        BIN_DIR / config / f"{exe_name}.exe",
        BIN_DIR / f"{exe_name}.exe",
        BIN_DIR / config / exe_name,
        BIN_DIR / exe_name,
        BIN_DIR / "Test" / config / f"{exe_name}.exe",
        BIN_DIR / "Test" / f"{exe_name}.exe",
        BIN_DIR / "Test" / config / exe_name,
        BIN_DIR / "Test" / exe_name,
    ]
    return next((p for p in candidates if p.exists()), None)


def run_tests(config):
    test_script = REPO_ROOT / "run_all_tests.sh"
    bash = subprocess.run(["where", "bash"], capture_output=True).returncode == 0
    if test_script.exists() and bash:
        cmd = ["bash", str(test_script)]
    else:
        cmd = ["ctest", "--output-on-failure", "-C", config]
    print(f"\n>>> {' '.join(cmd)}\n")
    return subprocess.run(cmd, cwd=REPO_ROOT / "build").returncode


def main():
    target, no_build, config, build_flags, exe_argv = parse_args()

    if not no_build:
        cmake_target = TARGETS[target]
        # pi and rho both build Console
        if target in ("pi", "rho"):
            cmake_target = "Console"
        # window needs KAI_BUILD_IMGUI=ON to even configure the ImGui target
        if target == "window" and "--imgui" not in build_flags:
            build_flags = build_flags + ["--imgui"]
        build(build_flags + ["--config", config] if "--config" not in build_flags else build_flags,
              cmake_target)

    if target == "tests":
        sys.exit(run_tests(config))

    exe = resolve_exe(target, config)
    if exe is None:
        print(f"error: could not find executable for '{target}' under {BIN_DIR}")
        sys.exit(1)

    cmd = [str(exe)]
    if target in CONSOLE_MODE_ARGS:
        cmd += CONSOLE_MODE_ARGS[target]
    cmd += exe_argv

    print(f"\n>>> {' '.join(cmd)}\n")
    sys.exit(subprocess.run(cmd, cwd=REPO_ROOT).returncode)


if __name__ == "__main__":
    main()
