#include <iostream>
#include <string>

#include "KAI/Core/Logger.h"
#include "TestCommon.h"
#include "rang.hpp"

/**
 * @file Main.cpp
 * @brief Main entry point for KAI test programs with colored output support
 *
 * This file implements:
 * - Command-line argument handling for colored output control
 * - Global state for color output preference
 * - Helper functions for enabling/disabling colors
 *
 * By default, colored output is enabled to improve readability.
 * Use --no-color to disable colors, or --color to explicitly enable.
 *
 * @see /Doc/ColorOutput.md for more details
 */

// Global flag to control whether color output is enabled - default to ON
bool g_useColorOutput = true;

/**
 * @brief Enable or disable colored console output
 *
 * @param enabled True to force colors on, false to auto-detect/disable
 */
void SetColorOutput(bool enabled) {
    g_useColorOutput = enabled;

    // Set the rang control mode based on user preference
    if (enabled) {
        // Force color even if output is redirected
        rang::setControlMode(rang::control::Force);
    } else {
        // Let rang auto-detect (usually disables color for redirected output)
        rang::setControlMode(rang::control::Auto);
    }
}

/**
 * @brief Check if colored output is currently enabled
 *
 * This function is used by TestCout and Logger to determine
 * whether to apply colors to output.
 *
 * @return bool True if colors should be used
 */
bool IsColorOutputEnabled() { return g_useColorOutput; }

int main(int argc, char **argv) {
    // Set color mode on by default
    SetColorOutput(true);

    // Process our custom arguments first
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--no-color") {
            // Disable colored output if explicitly requested
            SetColorOutput(false);

            // Remove this argument to prevent Google Test from complaining
            // about unknown flags
            for (int j = i; j < argc - 1; ++j) {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--;  // Process the next argument at the current position

            std::cout << "Disabled colored test output" << std::endl;
        } else if (arg == "--debug-color" || arg == "--color") {
            // This is now redundant since color is on by default, but kept for
            // compatibility
            SetColorOutput(true);

            // Remove this argument to prevent Google Test from complaining
            // about unknown flags
            for (int j = i; j < argc - 1; ++j) {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--;  // Process the next argument at the current position

            std::cout << rang::fg::green
                      << "Colored test output is enabled (default)"
                      << rang::fg::reset << std::endl;
        }
    }

    // Initialize Google Test with the possibly modified arguments
    ::testing::InitGoogleTest(&argc, argv);

    // Run the tests
    return RUN_ALL_TESTS();
}
