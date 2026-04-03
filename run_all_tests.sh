#!/bin/bash

# Exit on error
set -e

# Run all test executables
./Bin/Test/TestCore
./Bin/Test/TestPi
./Bin/Test/TestRho
./Bin/Test/TestTau
if [ -x ./Bin/Test/Test_Network ]; then
    ./Bin/Test/Test_Network
elif [ -x ./Bin/Test_Network ]; then
    ./Bin/Test_Network
else
    echo "Error: Test_Network binary not found" >&2
    exit 1
fi

echo "All tests passed!"
