#!/bin/bash

# Exit on error
set -e

run_test() {
    local name="$1"
    shift

    for path in "$@"; do
        if [ -x "$path" ]; then
            "$path"
            return 0
        fi
    done

    echo "Error: ${name} binary not found" >&2
    exit 1
}

# Run all test executables. Prefer the out-of-source build tree, then fall
# back to legacy in-tree output paths.
run_test TestCore ./build/Bin/Test/TestCore ./Bin/Test/TestCore
run_test TestPi ./build/Bin/Test/TestPi ./Bin/Test/TestPi
run_test TestRho ./build/Bin/Test/TestRho ./Bin/Test/TestRho
run_test TestTau ./build/Bin/Test/TestTau ./Bin/Test/TestTau
run_test TestNetwork \
    ./build/Bin/Test/TestNetwork \
    ./build/Bin/TestNetwork \
    ./Bin/Test/TestNetwork \
    ./Bin/TestNetwork

echo "All tests passed!"
