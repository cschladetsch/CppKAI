#!/bin/bash

# Exit on error
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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
    return 1
}

# Run all test executables from the in-tree output paths used by this
# repository.
run_test TestCore \
    "$SCRIPT_DIR/Bin/Test/TestCore"
run_test TestPi \
    "$SCRIPT_DIR/Bin/Test/TestPi"
run_test TestRho \
    "$SCRIPT_DIR/Bin/Test/TestRho"
run_test TestTau \
    "$SCRIPT_DIR/Bin/Test/TestTau"
run_test TestNetwork \
    "$SCRIPT_DIR/Bin/Test/TestNetwork"

echo "All tests passed!"
