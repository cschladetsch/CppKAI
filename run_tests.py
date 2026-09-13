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

Any extra command-line arguments are appended to the ctest invocation, so
e.g. `py run_tests.py -R TestNetwork` or `py run_tests.py -C Debug` both
work as you'd expect - pass your own -R if you want a subset.
"""
import subprocess
import sys


def main() -> int:
    cmd = ["ctest", "--test-dir", "build", "--output-on-failure", *sys.argv[1:]]
    result = subprocess.run(cmd)
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
