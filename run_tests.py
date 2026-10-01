#!/usr/bin/env python3
"""Thin wrapper around `ctest` so `py run_tests.py` works directly.

Equivalent to running, from the repo root:
    ctest --test-dir build --output-on-failure

Deliberately runs *every* discovered test (no name filter) - the previous
version of this file used `-R "^Test"`, which only matches CTest test names
that literally start with "Test" and so silently excluded any suite named
otherwise (e.g. NetworkAdditionalTests, CoreAdditionalTests2,
PiAdditionalTests2, RhoAdditionalTests2, TauAdditionalTests2, and others),
cutting a 1676-test run down to 101 with no warning.

What gets run is whatever the root CMakeLists.txt registers: every suite
under Test/ (TestCore, TestPi, TestRho, TestSigma, TestTau, TestNetwork,
TestConsole, LogTest, PerformanceTests, Test_ProxyGeneration, ...) plus
KshUnitTests from ksh/. Before running, this script reports how many tests
are registered, names any main suite that is missing, and warns if CTest
knows about suspiciously few tests, which is what happens when the root
CMakeLists.txt stops calling add_subdirectory(Test).

Any extra command-line arguments are appended to the ctest invocation, so
e.g. `py run_tests.py -R TestNetwork` or `py run_tests.py -C Debug` both
work as you'd expect - pass your own -R if you want a subset.
"""
import re
import subprocess
import sys
from collections import Counter

BUILD_DIR = "build"

# A full build registers ~2000 CTest entries (gtest_discover_tests adds one
# per gtest case). Far fewer means the Test/ tree is not wired into CMake.
MIN_EXPECTED_TESTS = 500

# Suites a full build should register (checked by name, so a missing one is
# reported instead of silently skipped). TestNetwork needs KAI_NETWORKING=ON.
KEY_SUITES = ["SigmaTests", "TestConsole", "TestNetwork", "KshUnitTests",
              "LogTest", "PerformanceTests"]


def list_suites(extra_args):
    """Return a Counter of suite name -> number of registered tests."""
    cmd = ["ctest", "--test-dir", BUILD_DIR, "-N", *extra_args]
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    suites = Counter()
    for line in out.splitlines():
        m = re.match(r"\s*Test\s+#\d+:\s+(\S+)", line)
        if m:
            suites[m.group(1).split(".")[0]] += 1
    return suites


def main() -> int:
    extra = sys.argv[1:]
    suites = list_suites(extra)
    total = sum(suites.values())

    if total == 0:
        print(f"error: CTest found no tests in '{BUILD_DIR}'. "
              "Build first with `py build.py`.", file=sys.stderr)
        return 1

    print(f"{total} tests registered in {len(suites)} suites "
          f"(including {', '.join(n for n in KEY_SUITES if n in suites) or 'none of the main suites'}).")
    missing = [n for n in KEY_SUITES if n not in suites]
    if missing and total >= MIN_EXPECTED_TESTS:
        print(f"note: not registered in this build: {', '.join(missing)}")
    print()

    filtered = any(a in ("-R", "-E", "-L", "-LE", "-I") or a.startswith(("-R", "-E"))
                   for a in extra)
    if not filtered and total < MIN_EXPECTED_TESTS:
        print(f"warning: only {total} tests are registered (expected well over "
              f"{MIN_EXPECTED_TESTS}). Check that the root CMakeLists.txt still "
              "calls enable_testing() and add_subdirectory(Test), then "
              "reconfigure.\n", file=sys.stderr)

    cmd = ["ctest", "--test-dir", BUILD_DIR, "--output-on-failure", *extra]
    return subprocess.run(cmd).returncode


if __name__ == "__main__":
    sys.exit(main())
