plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.kaikaspar"
    compileSdk = 35

    // Any NDK r26+ provides clang with C++23 support. Adjust to one you have
    // installed (sdkmanager "ndk;<version>").
    ndkVersion = "27.0.12077973"

    defaultConfig {
        applicationId = "com.kaikaspar"
        minSdk = 26
        targetSdk = 35
        versionCode = 1
        versionName = "0.1"

        ndk {
            // The Galaxy S26 and every modern device are arm64. Add more ABIs
            // (e.g. "x86_64" for the emulator) if you need them.
            abiFilters += listOf("arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                // Build only the KAI library subset and the libkai.so aggregate.
                arguments += listOf(
                    "-DKAI_ANDROID=ON",
                    "-DANDROID_STL=c++_shared",
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            // Point at the KAI repository root. AGP supplies the NDK toolchain
            // and ANDROID_ABI automatically; our CMake builds the `kai` target.
            // NOTE: the KAI root requires CMake >= 3.28 - install it via
            // sdkmanager "cmake;3.31.6" (or newer) if the bundled one is older.
            path = file("../../../CMakeLists.txt")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.appcompat:appcompat:1.7.0")
}
