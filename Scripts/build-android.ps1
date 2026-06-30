#!/usr/bin/env bash
# Cross-compile the KAI Android library subset (CppKaiCore + CppKaiLanguage +
# Network) for a device using the Android NDK.
#
# Usage:
#   Scripts/build-android.ps1 [ABI] [API]
#     ABI  target ABI    (default: arm64-v8a -- the Galaxy S26 / modern arm64)
#     API  min API level (default: 26)
#
# Requirements:
#   - ANDROID_NDK_HOME (or ANDROID_NDK_ROOT / ANDROID_NDK) pointing at an NDK
#     r26 or newer (needed for C++23).
# KAI is Boost-free; no other dependencies are required.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ABI="${1:-arm64-v8a}"
API="${2:-26}"
BUILD_DIR="$ROOT/build-android-$ABI"

NDK="${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-${ANDROID_NDK:-}}}"
if [ -z "$NDK" ] || [ ! -f "$NDK/build/cmake/android.toolchain.cmake" ]; then
    echo "ERROR: Android NDK not found." >&2
    echo "  Set ANDROID_NDK_HOME to an NDK r26+ install, e.g.:" >&2
    echo "    export ANDROID_NDK_HOME=\$HOME/Android/Sdk/ndk/27.0.12077973" >&2
    exit 1
fi
export ANDROID_NDK_HOME="$NDK"

echo "Building KAI for Android: ABI=$ABI  API=$API"
echo "  NDK: $ANDROID_NDK_HOME"

cmake -S "$ROOT" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/CMake/AndroidToolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$API" \
    -DKAI_ANDROID=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DBIN_HOME="$BUILD_DIR/Bin"

cmake --build "$BUILD_DIR" -j"$(nproc)"

echo
echo "Done. Android $ABI libraries in $BUILD_DIR/Bin:"
ls -1 "$BUILD_DIR/Bin/"*.a 2>/dev/null || echo "  (no archives found)"