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

## Important: boost.monotonic needs upstream Boost headers

KAI is Boost-free except for `boost.monotonic` (the in-house CppMonotonic
submodule), used by the Common lexer. CppMonotonic is **not** self-contained -
it `#include`s around three dozen upstream Boost headers (`boost/config`,
`boost/interprocess`, `boost/ptr_container`, `boost/thread`, `boost/unordered`,
`boost/type_traits`, ...). On the desktop these come from the system Boost
install. The NDK has no system Boost, so you must supply them:

    export KAI_ANDROID_BOOST_INCLUDE_DIR=/path/to/boost   # dir containing boost/
    Scripts/build-android.sh

Boost is almost entirely header-only, so a Boost source tree (or a vendored
subset) on the include path is enough; nothing extra needs to be compiled.

If you would rather drop this dependency for Android, the modern standard-library
replacement for boost.monotonic's arena allocation is `std::pmr`
(`std::pmr::monotonic_buffer_resource` with `std::pmr::vector` / `std::pmr::map`)
in `Include/KAI/Language/Common/LexerCommon.h`. That would make the Android build
fully self-contained with no Boost headers at all.

## Getting the libraries onto the device

The build produces static archives (`.a`). The usual path to running on a
device (e.g. a Galaxy S26) is to link them into your app's JNI shared library
via `externalNativeBuild` in Gradle, or to add an aggregate `libkai.so` shared
target. Ask if you want that aggregate target and a minimal Gradle/JNI consumer
scaffolded.
