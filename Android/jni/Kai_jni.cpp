// JNI bridge for the KAI runtime: the native side of libkai.so.
//
// Exposes a stateful Rho console to Kotlin/Java via org.kai.runtime.KaiRuntime
// (System.loadLibrary("kai")). Each KaiRuntime instance owns one kai::Console;
// eval() runs a snippet and returns the resulting data stack as text.
//
// This file is host-compilable (against a JDK's <jni.h>) for a link sanity
// check; Android-specific bits are guarded by __ANDROID__.

#include <jni.h>

#include <exception>
#include <string>

#include <KAI/Console/Console.h>
#include <KAI/Language/Common/Language.h>

#ifdef __ANDROID__
#include <android/log.h>
#define KAI_JNI_LOG(...) __android_log_print(ANDROID_LOG_INFO, "KAI", __VA_ARGS__)
#else
#define KAI_JNI_LOG(...) ((void)0)
#endif

using kai::Console;
using kai::Language;
using kai::String;

namespace {

jstring ToJString(JNIEnv *env, const std::string &s) {
    return env->NewStringUTF(s.c_str());
}

std::string FromJString(JNIEnv *env, jstring s) {
    if (s == nullptr) return {};
    const char *chars = env->GetStringUTFChars(s, nullptr);
    std::string out(chars != nullptr ? chars : "");
    if (chars != nullptr) env->ReleaseStringUTFChars(s, chars);
    return out;
}

Console *AsConsole(jlong handle) { return reinterpret_cast<Console *>(handle); }

}  // namespace

extern "C" {

JNIEXPORT jlong JNICALL
Java_org_kai_runtime_KaiRuntime_nativeCreate(JNIEnv *, jobject) {
    try {
        auto *console = new Console();
        console->SetLanguage(Language::Rho);
        KAI_JNI_LOG("KAI runtime created");
        return reinterpret_cast<jlong>(console);
    } catch (...) {
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_org_kai_runtime_KaiRuntime_nativeDestroy(JNIEnv *, jobject, jlong handle) {
    delete AsConsole(handle);
}

JNIEXPORT jstring JNICALL Java_org_kai_runtime_KaiRuntime_nativeEval(
    JNIEnv *env, jobject, jlong handle, jstring source) {
    Console *console = AsConsole(handle);
    if (console == nullptr) return ToJString(env, "error: runtime not initialised");

    const std::string src = FromJString(env, source);
    try {
        console->Execute(String(src));
        return ToJString(env, console->WriteStack().StdString());
    } catch (const std::exception &e) {
        return ToJString(env, std::string("error: ") + e.what());
    } catch (...) {
        return ToJString(env, "error: unknown KAI exception");
    }
}

JNIEXPORT jstring JNICALL
Java_org_kai_runtime_KaiRuntime_nativeVersion(JNIEnv *env, jobject) {
    return ToJString(env, std::to_string(KAI_VERSION_MAJOR) + "." +
                              std::to_string(KAI_VERSION_MINOR) + "." +
                              std::to_string(KAI_VERSION_PATCH));
}

}  // extern "C"
