#!/usr/bin/env python3
"""
build.py - Windows build script for CppKAI

Default toolchain: Clang + Ninja, with ENABLE_SHELL_SYNTAX=OFF (matches the
root CMakeLists.txt's own default - backtick shell execution is opt-in, not
opt-out, since it lets any evaluated Pi/Rho source run arbitrary shell
commands). Pass --enable-shell to turn it on, or --msvc to opt into MSVC +
Visual Studio generator + vcpkg instead.
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent
BUILD_DIR = REPO_ROOT / "build"
BIN_DIR = REPO_ROOT / "Bin"

# Common vcpkg install locations - first match wins
VCPKG_SEARCH_PATHS = [
    Path(r'C:/vcpkg'),
    Path(r'C:/src/vcpkg'),
    Path(r'C:/tools/vcpkg'),
    Path.home() / 'vcpkg',
    Path.home() / 'local' / 'vcpkg',
]

MSVC_GENERATORS = [
    "Visual Studio 18 2026",
    "Visual Studio 17 2022",
    "Visual Studio 16 2019",
]

# Common Clang install locations on Windows - first match wins. `shutil.which`
# already covers the case where clang++ is on PATH; these are fallbacks for
# the common LLVM installer default location when it isn't.
CLANG_SEARCH_PATHS = [
    Path(r'C:/Program Files/LLVM/bin/clang++.exe'),
    Path(r'C:/Program Files (x86)/LLVM/bin/clang++.exe'),
]
CLANGCC_SEARCH_PATHS = [
    Path(r'C:/Program Files/LLVM/bin/clang.exe'),
    Path(r'C:/Program Files (x86)/LLVM/bin/clang.exe'),
]


def run(cmd: list[str], cwd: Path = REPO_ROOT, check: bool = True) -> int:
    print(f"\n>>> {' '.join(str(c) for c in cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if check and result.returncode != 0:
        print(f"ERROR: command failed with exit code {result.returncode}", file=sys.stderr)
        sys.exit(result.returncode)
    return result.returncode


def find_cmake() -> str:
    cmake = shutil.which("cmake")
    if not cmake:
        sys.exit("ERROR: cmake not found on PATH")
    return cmake


def find_ninja() -> str | None:
    return shutil.which("ninja")


def find_clangxx() -> str | None:
    """Return the path to clang++ if found, else None."""
    on_path = shutil.which("clang++")
    if on_path:
        return on_path
    for candidate in CLANG_SEARCH_PATHS:
        if candidate.exists():
            return str(candidate)
    return None


def find_clangcc() -> str | None:
    """Return the path to clang (the C compiler) if found, else None."""
    on_path = shutil.which("clang")
    if on_path:
        return on_path
    for candidate in CLANGCC_SEARCH_PATHS:
        if candidate.exists():
            return str(candidate)
    return None


def find_vcpkg_toolchain() -> str | None:
    """Return the vcpkg toolchain path if vcpkg is installed, else None."""
    # Honour an explicit env var first
    env = os.environ.get("VCPKG_ROOT")
    if env:
        tc = Path(env) / "scripts" / "buildsystems" / "vcpkg.cmake"
        if tc.exists():
            return str(tc)
    for base in VCPKG_SEARCH_PATHS:
        tc = base / "scripts" / "buildsystems" / "vcpkg.cmake"
        if tc.exists():
            print(f"[vcpkg] found toolchain: {tc}")
            return str(tc)
    return None


def find_msvc_generator() -> str:
    for gen in MSVC_GENERATORS:
        # Quick probe - cmake --help lists available generators
        result = subprocess.run(
            ["cmake", "--help"], capture_output=True, text=True
        )
        if gen in result.stdout:
            return gen
    return MSVC_GENERATORS[0]  # Fall back and let cmake complain


def submodule_init(cmake: str) -> None:
    run(["git", "submodule", "init"])
    run(["git", "submodule", "update", "--recursive"])


def configure(args, cmake: str, generator: str, ninja: str | None) -> None:
    BUILD_DIR.mkdir(parents=True, exist_ok=True)

    cmd = [cmake, ".."]

    if args.msvc:
        cmd += ["-G", generator, "-A", "x64"]
    else:
        if not ninja:
            sys.exit(
                "ERROR: ninja not found on PATH, which is required for the "
                "default Clang/Ninja build. Install Ninja (e.g. via your "
                "LLVM/MSYS2/scoop/choco package manager) and ensure it's on "
                "PATH, or pass --msvc to use Visual Studio + MSVC instead."
            )
        cmd += ["-G", "Ninja"]

        clangxx = find_clangxx()
        clangcc = find_clangcc()
        if not clangxx or not clangcc:
            sys.exit(
                "ERROR: clang++/clang not found (checked PATH and the "
                "default LLVM install location). Install LLVM/Clang for "
                "Windows (https://releases.llvm.org/), or pass --msvc to "
                "use MSVC instead."
            )
        print(f"[clang] using compiler: {clangxx}")
        cmd += [
            f"-DCMAKE_CXX_COMPILER={clangxx}",
            f"-DCMAKE_C_COMPILER={clangcc}",
        ]

    cmd += [
        f"-DCMAKE_BUILD_TYPE={args.config}",
        # NOTE: the CMake option is named KAI_NETWORKING (CMakeLists.txt), not
        # KAI_BUILD_NETWORKING - this used to define an unused variable and
        # silently leave networking on regardless of --no-network.
        f"-DKAI_NETWORKING={'OFF' if args.no_network else 'ON'}",
        f"-DKAI_BUILD_LLM={'ON' if args.llm else 'OFF'}",
        f"-DKAI_BUILD_IMGUI={'ON' if args.imgui else 'OFF'}",
        f"-DENABLE_SHELL_SYNTAX={'OFF' if args.disable_shell or not args.enable_shell else 'ON'}",
        f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={BIN_DIR}",
    ]

    # Auto-inject vcpkg toolchain unless the user already supplied one.
    # Useful with either generator - e.g. pulling in a vcpkg-installed
    # library (glfw3 for KAI_BUILD_IMGUI) works fine with Clang/Ninja too,
    # since LLVM's Windows target defaults to the MSVC ABI.
    if not args.no_vcpkg and not any("CMAKE_TOOLCHAIN_FILE" in a for a in args.extra):
        toolchain = find_vcpkg_toolchain()
        if toolchain:
            cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")
        else:
            print("[vcpkg] not found - GTest and other vcpkg deps may be missing")
            print("[vcpkg] run: git clone https://github.com/microsoft/vcpkg C:/vcpkg && C:/vcpkg/bootstrap-vcpkg.bat")

    if args.extra:
        cmd += args.extra

    run(cmd, cwd=BUILD_DIR)


def build(args, cmake: str) -> None:
    cmd = [
        cmake, "--build", str(BUILD_DIR),
        "--config", args.config,
        "--parallel", str(os.cpu_count() or 4),
    ]
    if args.target:
        cmd += ["--target", args.target]
    run(cmd)


def run_tests(args) -> None:
    test_script = REPO_ROOT / "run_all_tests.sh"
    if test_script.exists():
        run(["bash", str(test_script)])
    else:
        # Fall back to ctest
        run(["ctest", "--output-on-failure", "-C", args.config], cwd=BUILD_DIR)


def clean() -> None:
    if BUILD_DIR.exists():
        print(f"Removing {BUILD_DIR}")
        shutil.rmtree(BUILD_DIR)
    # Bin/ is NOT removed - it may contain assets or manually placed binaries


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Build CppKAI on Windows",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python build.py                       default Release build (Clang + Ninja, shell syntax OFF)
  python build.py --config Debug        debug build
  python build.py --msvc                use MSVC + Visual Studio generator + vcpkg instead
  python build.py --no-network          disable networking
  python build.py --llm                 enable LLM layer
  python build.py --imgui               build the ImGui/Window frontend
  python build.py --clean               clean build dirs
  python build.py --test                build then run tests
  python build.py --target Console      build specific target
  python build.py --enable-shell        enable shell/backtick syntax (off by default)
  python build.py --msvc --no-vcpkg     MSVC build, skip vcpkg toolchain injection
  python build.py -- -DFOO=BAR         pass extra CMake args
        """,
    )

    parser.add_argument("--config", default="Release",
                        choices=["Release", "Debug", "RelWithDebInfo", "MinSizeRel"],
                        help="Build configuration (default: Release)")
    parser.add_argument("--msvc", action="store_true",
                        help="Use MSVC + Visual Studio generator + vcpkg instead of the default Clang/Ninja build")
    parser.add_argument("--no-network", action="store_true",
                        help="Disable networking (KAI_BUILD_NETWORKING=OFF)")
    parser.add_argument("--llm", action="store_true",
                        help="Enable LLM layer (KAI_BUILD_LLM=ON)")
    parser.add_argument("--imgui", action="store_true",
                        help="Build the ImGui/Window frontend (KAI_BUILD_IMGUI=ON)")
    parser.add_argument("--enable-shell", action="store_true",
                        help="Enable shell/backtick syntax (off by default; "
                             "lets evaluated Pi/Rho source run shell commands)")
    parser.add_argument("--disable-shell", action="store_true",
                        help="Deprecated no-op: shell/backtick syntax is off "
                             "by default now. Kept so old invocations don't "
                             "fail; use --enable-shell to turn it on.")
    parser.add_argument("--clean", action="store_true",
                        help="Remove build/ and Bin/ before building")
    parser.add_argument("--clean-only", action="store_true",
                        help="Remove build/ and Bin/ then exit")
    parser.add_argument("--reconfigure", action="store_true",
                        help="Remove build/ then reconfigure")
    parser.add_argument("--no-submodules", action="store_true",
                        help="Skip git submodule init/update")
    parser.add_argument("--test", action="store_true",
                        help="Run tests after build")
    parser.add_argument("--target", metavar="TARGET",
                        help="Build a specific CMake target")
    parser.add_argument("--no-vcpkg", action="store_true",
                        help="Skip vcpkg toolchain auto-detection")
    args, extra = parser.parse_known_args()
    # argparse's parse_known_args() (unlike parse_args()) leaves a literal
    # "--" separator in the leftover list instead of stripping it, so
    # `python build.py -- -DFOO=BAR` would otherwise forward a bare "--" to
    # cmake as well, which cmake's own argument parser rejects outright.
    if extra and extra[0] == "--":
        extra = extra[1:]
    args.extra = extra

    cmake = find_cmake()
    ninja = find_ninja()
    generator = find_msvc_generator() if args.msvc else ""

    if args.clean_only:
        clean()
        return

    if args.clean or args.reconfigure:
        clean()

    if not args.no_submodules:
        submodule_init(cmake)

    configure(args, cmake, generator, ninja)
    build(args, cmake)

    if args.test:
        run_tests(args)

    print(f"\nBuild complete. Binaries in {BIN_DIR}")


if __name__ == "__main__":
    main()
