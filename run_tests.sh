#!/bin/bash
set -e

echo "=== Building ==="
./b

echo ""
echo "=== TestTau (proxy generation) ==="
./Bin/Test/TestTau 2>&1 | tail -5

echo ""
echo "=== TestCore ==="
./Bin/Test/TestCore 2>&1 | tail -5

echo ""
echo "=== TestPi ==="
./Bin/Test/TestPi 2>&1 | tail -5

echo ""
echo "=== TestRho ==="
./Bin/Test/TestRho 2>&1 | tail -5

echo ""
echo "=== Test_ProxyGeneration ==="
./Bin/Test/Test_ProxyGeneration 2>&1 | tail -5

echo ""
echo "=== Summary ==="
for bin in ./Bin/Test/TestTau ./Bin/Test/TestCore ./Bin/Test/TestPi ./Bin/Test/TestRho ./Bin/Test/Test_ProxyGeneration; do
    if [ -f "$bin" ]; then
        result=$($bin 2>&1 | grep -E "PASSED|FAILED" | tail -1)
        echo "$bin: $result"
    fi
done
