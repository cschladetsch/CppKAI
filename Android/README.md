# Android

Everything needed to run the KAI runtime on Android.

    Android/
      jni/Kai_jni.cpp     JNI bridge - the native side of libkai.so
      KaiKaspar/          minimal Gradle app demonstrating the integration

The CMake `kai` target (built when `-DKAI_ANDROID=ON`) links the KAI library
subset (CppKaiCore + CppKaiLanguage + Network) plus `jni/Kai_jni.cpp` into a
single **`libkai.so`**. An app loads it with `System.loadLibrary("kai")` and
drives the runtime through `com.kaikaspar.kai.KaiRuntime`.

See [../Doc/Android.md](../Doc/Android.md) for prerequisites, the toolchain, the
`Scripts/build-android.sh` helper, and device/Gradle details.

## Quick paths

Command-line cross-compile (needs `ANDROID_NDK_HOME`, NDK r26+):

    ../Scripts/build-android.sh            # -> build-android-arm64-v8a/Bin/libkai.so

Gradle app: open `KaiKaspar/` (its `externalNativeBuild` builds `libkai.so` for
arm64 and bundles it into the APK).
