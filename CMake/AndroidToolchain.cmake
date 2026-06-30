# CMake/AndroidToolchain.cmake
#
# Thin wrapper over the NDK's own android.toolchain.cmake with sensible KAI
# defaults (arm64, the Galaxy S26 / modern arm64 devices). Use it together with
# -DKAI_ANDROID=ON:
#
#   cmake -B build-android -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=CMake/AndroidToolchain.cmake \
#     -DKAI_ANDROID=ON
#
# Override ANDROID_ABI / ANDROID_PLATFORM / ANDROID_STL on the command line as
# needed. The Scripts/build-android.ps1 helper wraps all of this.

# --- Locate the NDK ---------------------------------------------------------
if(NOT DEFINED ANDROID_NDK)
    if(DEFINED ENV{ANDROID_NDK_HOME})
        set(ANDROID_NDK $ENV{ANDROID_NDK_HOME})
    elseif(DEFINED ENV{ANDROID_NDK_ROOT})
        set(ANDROID_NDK $ENV{ANDROID_NDK_ROOT})
    elseif(DEFINED ENV{ANDROID_NDK})
        set(ANDROID_NDK $ENV{ANDROID_NDK})
    endif()
endif()

if(NOT ANDROID_NDK OR NOT EXISTS "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
    message(FATAL_ERROR
        "Android NDK not found. Set ANDROID_NDK_HOME (or pass -DANDROID_NDK=...) "
        "to an NDK r26 or newer install (required for C++23). "
        "Looked at: '${ANDROID_NDK}'.")
endif()

# --- KAI defaults (override on the command line) ----------------------------
# Target arm64 by default - the Galaxy S26 and all modern flagship devices.
if(NOT DEFINED ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a")
endif()

# Minimum supported API level. android-26 (Android 8.0) is a safe modern floor.
if(NOT DEFINED ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-26")
endif()

# KAI uses C++ exceptions and RTTI (typeid + <cxxabi.h> demangling), so a real
# C++ STL is required. c++_shared is the standard choice for a library that is
# loaded into an app; switch to c++_static for a fully self-contained binary.
if(NOT DEFINED ANDROID_STL)
    set(ANDROID_STL "c++_shared")
endif()

# Hand off to the real NDK toolchain file.
include("${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
