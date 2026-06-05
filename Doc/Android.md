# Building KAI for Android

The Android-reusable subset of KAI is the interpreter stack plus networking:

- **CppKaiCore** submodule - Core, Executor, CommonLang
- **CppKaiLanguage** submodule - Pi and Rho
- **Network** + ENet

ImGui, LLM, Tau, the desktop apps, the demos and the test suite are excluded
from Android builds. This subset is selected by the `KAI_ANDROID` CMake option.

## Prerequisites

- **Android NDK r26 or newer** (KAI is C++23; r26 ships clang 17, r27 ships
  clang 18). Point one of `ANDROID_NDK_HOME` / `ANDROID_NDK_ROOT` / `ANDROID_NDK`
  at it:

      export ANDROID_NDK_HOME=$HOME/Android/Sdk/ndk/27.0.12077973

- CMake 3.20+ and Ninja.

## Quick start

    git submodule update --init --recursive
    Scripts/build-android.sh                 # arm64-v8a, API 26 (Galaxy S26)
    Scripts/build-android.sh arm64-v8a 30    # explicit ABI / API

Output archives land in `build-android-<abi>/Bin/`.

## How it works

`CMake/AndroidToolchain.cmake` is a thin wrapper over the NDK's own
`build/cmake/android.toolchain.cmake`. It defaults to `ANDROID_ABI=arm64-v8a`,
`ANDROID_PLATFORM=android-26` and `ANDROID_STL=c++_shared` (KAI uses exceptions
and RTTI, so a real STL is required). To invoke CMake directly:

    cmake -S . -B build-android -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=CMake/AndroidToolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-26 \
      -DKAI_ANDROID=ON \
      -DCMAKE_BUILD_TYPE=Release
    cmake --build build-android -j

## Dependencies

KAI is fully Boost-free. The library subset needs only the NDK's C++ standard
library (libc++). CommonLang's lexer can optionally use `std::pmr` arena
allocation (`-DKAI_USE_MONOTONIC_ALLOCATOR`); by default it uses plain
`std::vector` / `std::map`. Nothing third-party is required.

## Running on the device: libkai.so + JNI

The `KAI_ANDROID` build also produces an aggregate **`libkai.so`** (the CMake
`kai` target): all the static libraries linked into one shared object with a
small JNI bridge (`Android/jni/Kai_jni.cpp`). An app loads it with
`System.loadLibrary("kai")` and drives the runtime through
`com.kaikaspar.kai.KaiRuntime`:

    val result = KaiRuntime().use { kai -> kai.eval("x = 6; y = 7; x * y") }

The JNI bridge exposes `nativeCreate` / `nativeEval` / `nativeVersion` /
`nativeDestroy`; `KaiRuntime` wraps them with `eval()`, `version` and
`AutoCloseable`. Each `KaiRuntime` owns one Rho console, so state persists
across `eval` calls.

### KaiKaspar consumer

`Android/KaiKaspar/` is a minimal Gradle app showing the integration end to end:

- `app/build.gradle.kts` points `externalNativeBuild` at the repo-root
  `CMakeLists.txt` with `-DKAI_ANDROID=ON` and `abiFilters = ["arm64-v8a"]`.
  AGP supplies the NDK toolchain and ABI automatically.
- `KaiRuntime.kt` is the reusable binding (drop it into your real app).
- `MainActivity.kt` evaluates Rho on startup and logs the result (tag
  `KaiKaspar`).

Requirements: an installed NDK r26+ and **CMake >= 3.28** (the KAI root requires
it - install via `sdkmanager "cmake;3.31.6"` if the bundled CMake is older).

The aggregate also builds (and link-checks) on the host:

    JAVA_HOME=/path/to/jdk cmake -B build-host -G Ninja -DKAI_ANDROID=ON
    cmake --build build-host --target kai      # -> build-host/Bin/libkai.so
