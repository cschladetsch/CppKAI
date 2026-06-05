package com.kaikaspar

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.kaikaspar.kai.KaiRuntime

/**
 * Minimal demonstration of driving the KAI runtime from an Android app.
 * Watch logcat (tag "KaiKaspar") to see the runtime evaluate Rho on the device.
 */
class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        KaiRuntime().use { kai ->
            Log.i(TAG, "KAI runtime version ${kai.version}")

            kai.eval("x = 6")
            kai.eval("y = 7")
            val result = kai.eval("x * y")

            Log.i(TAG, "x * y => $result")
        }
    }

    private companion object {
        const val TAG = "KaiKaspar"
    }
}
