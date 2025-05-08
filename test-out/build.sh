#!/bin/bash
# Simple build script for the do-while test

echo "Building do-while test..."
cd /home/xian/local/KAI
g++ -std=c++17 -I./Include test-out/test-dowhile.cpp -o test-out/test-dowhile -L./Lib -lCore -lLanguage -lExecutor
echo "Build complete. Running test..."
cd /home/xian/local/KAI/test-out
./test-dowhile