#!/bin/bash

# Exit on error
set -e

# Run all test executables
./Bin/Test/TestCore
./Bin/Test/TestPi
./Bin/Test/TestRho
./Bin/Test/TestTau
./Bin/Test/Test_Network

echo "All tests passed!"
