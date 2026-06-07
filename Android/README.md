# Android

Everything needed to run the KAI runtime on Android.

    Android/
      jni/Kai_jni.cpp     JNI bridge - the native side of libkai.so

The CMake `kai` target (built when `-DKAI_ANDROID=ON`) links the KAI library
subset (CppKaiCore + CppKaiLanguage + Network) plus `jni/Kai_jni.cpp` into a
single **`libkai.so`**. An app loads it with `System.loadLibrary("kai")` and
drives the runtime through its own Kotlin or Java JNI wrapper.

See [../Doc/Android.md](../Doc/Android.md) for prerequisites, the toolchain, the
`Scripts/build-android.sh` helper, and downstream app integration details.

## Quick paths

Command-line cross-compile (needs `ANDROID_NDK_HOME`, NDK r26+):

    ../Scripts/build-android.sh            # -> build-android-arm64-v8a/Bin/libkai.so

Downstream apps should point their Android Gradle `externalNativeBuild` at the
CppKAI repo root, pass `-DKAI_ANDROID=ON`, and load the resulting `libkai.so`.
