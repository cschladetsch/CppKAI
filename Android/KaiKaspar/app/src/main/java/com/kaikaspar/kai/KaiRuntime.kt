package com.kaikaspar.kai

/**
 * Kotlin binding for the native KAI runtime (libkai.so).
 *
 * Each instance owns one native Rho console. [eval] runs a snippet and returns
 * the resulting data stack as text. The instance is [AutoCloseable] - use it
 * with `use { ... }` or call [close] when finished to free the native console.
 *
 * This file is the reusable binding: drop it into your real app's source set
 * (it only needs libkai.so on the JNI library path).
 */
class KaiRuntime : AutoCloseable {

    private var handle: Long = nativeCreate()

    /** The KAI runtime version, e.g. "0.3.0". */
    val version: String
        get() = nativeVersion()

    /**
     * Evaluate a Rho snippet, returning the resulting data stack as text.
     * Errors are returned as a string prefixed with "error:".
     */
    fun eval(source: String): String {
        check(handle != 0L) { "KaiRuntime has been closed" }
        return nativeEval(handle, source)
    }

    override fun close() {
        if (handle != 0L) {
            nativeDestroy(handle)
            handle = 0L
        }
    }

    private external fun nativeCreate(): Long
    private external fun nativeEval(handle: Long, source: String): String
    private external fun nativeVersion(): String
    private external fun nativeDestroy(handle: Long)

    companion object {
        init {
            System.loadLibrary("kai")
        }
    }
}
