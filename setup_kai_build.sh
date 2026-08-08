#!/usr/bin/env bash
#
# setup_kai_build.sh
#
# Installs the toolchain needed to build the kai project natively under WSL
# (clang, ninja, Boost, libstdc++ with C++23 support), then does a clean
# CMake + Ninja reconfigure and build.
#
# Usage:
#   chmod +x setup_kai_build.sh
#   ./setup_kai_build.sh [path-to-kai-repo]
#
# If no path is given, assumes you're already inside the repo directory.

set -euo pipefail

REPO_DIR="${1:-$(pwd)}"

if [ ! -f "$REPO_DIR/CMakeLists.txt" ]; then
    echo "Error: $REPO_DIR does not look like the kai repo (no CMakeLists.txt found)."
    echo "Usage: ./setup_kai_build.sh [path-to-kai-repo]"
    exit 1
fi

echo "=== Updating package lists ==="
sudo apt update

echo "=== Installing toolchain: clang, ninja, boost, libstdc++-16, build-essential ==="
sudo apt install -y \
    clang \
    ninja-build \
    libboost-all-dev \
    libstdc++-16-dev \
    build-essential

echo
echo "=== Verifying tools are on PATH ==="
for tool in clang clang++ ninja cmake g++; do
    if command -v "$tool" >/dev/null 2>&1; then
        echo "  [OK] $tool -> $(command -v "$tool")"
    else
        echo "  [MISSING] $tool not found on PATH"
    fi
done

echo
echo "=== Checking for Boost CMake config ==="
BOOST_CONFIG=$(find /usr -iname "BoostConfig.cmake" -o -iname "boost-config.cmake" 2>/dev/null | head -n1 || true)
if [ -n "$BOOST_CONFIG" ]; then
    echo "  [OK] Found: $BOOST_CONFIG"
else
    echo "  [INFO] No BoostConfig.cmake found — CMake will fall back to its"
    echo "         built-in FindBoost.cmake module, which is fine unless"
    echo "         CMakeLists.txt explicitly requires CONFIG mode."
fi

echo
echo "=== Cleaning old build directory ==="
cd "$REPO_DIR"
rm -rf build

echo
echo "=== Configuring with CMake + Ninja ==="
cmake -B build -G Ninja -DCMAKE_CXX_STANDARD=23

echo
echo "=== Building ==="
cmake --build build

echo
echo "=== Done ==="
