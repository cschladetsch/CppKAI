#include <gtest/gtest.h>

#include <iostream>

#include "KAI/Core/Logger.h"
#include "TestCommon.h"

// Forward declaration
extern void SetColorOutput(bool enabled);

/**
 * @file ColorExample.cpp
 * @brief Example test demonstrating colored output functionality
 *
 * This file shows how to use the colored output features in KAI tests:
 * - TestCout colored messages
 * - Color formatting with rang
 */

// Example test suite demonstrating colored output
class ColorOutputExample : public ::testing::Test {
   protected:
    void SetUp() override {
        // Force color on for demo purposes
        SetColorOutput(true);
    }
};

// Example test that demonstrates different types of colored output
TEST_F(ColorOutputExample, DemonstrateColors) {
    // Simple colored output without Logger
    std::cout << rang::fg::green << "[INFO] This is an INFO message (green)"
              << rang::fg::reset << std::endl;
    std::cout << rang::fg::yellow
              << "[WARNING] This is a WARNING message (yellow)"
              << rang::fg::reset << std::endl;
    std::cout << rang::fg::red << "[ERROR] This is an ERROR message (red)"
              << rang::fg::reset << std::endl;

    // TestCout colored output
    TEST_COUT << "This is a TestCout INFO message (green)";
    TEST_CERR << "This is a TestCout ERROR message (red)";

    // Direct rang usage for custom colors
    std::cout << rang::fg::magenta << "Custom colored text (magenta)"
              << rang::fg::reset << std::endl;

    std::cout << rang::fg::blue << rang::style::bold << "Bold blue text"
              << rang::style::reset << rang::fg::reset << std::endl;

    // Demonstrate background color
    std::cout << rang::bg::yellow << rang::fg::black
              << "Black text on yellow background" << rang::bg::reset
              << rang::fg::reset << std::endl;
}

// This test would only be run manually for demonstration purposes
int main(int argc, char **argv) {
    std::cout << "=== COLOR OUTPUT EXAMPLE ===" << std::endl;
    std::cout << "This example demonstrates the colored output functionality."
              << std::endl;
    std::cout << "Run with --no-color to disable colors." << std::endl;
    std::cout << std::endl;

    // We need to preprocess the args here
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--color" || arg == "--debug-color") {
            SetColorOutput(true);
        } else if (arg == "--no-color") {
            SetColorOutput(false);
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}