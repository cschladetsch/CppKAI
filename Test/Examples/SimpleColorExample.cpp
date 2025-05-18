#include <iostream>

#include "rang.hpp"

/**
 * @file SimpleColorExample.cpp
 * @brief Simple example demonstrating terminal color functionality with rang
 *
 * This is a standalone program that demonstrates how to use the rang library
 * to produce colored output in the terminal.
 */

int main() {
    std::cout << "=== TERMINAL COLOR EXAMPLE ===" << std::endl;
    std::cout << "This example demonstrates colored terminal output with rang."
              << std::endl;
    std::cout << std::endl;

    // Basic foreground colors
    std::cout << rang::fg::green << "This is green text" << rang::fg::reset
              << std::endl;
    std::cout << rang::fg::red << "This is red text" << rang::fg::reset
              << std::endl;
    std::cout << rang::fg::yellow << "This is yellow text" << rang::fg::reset
              << std::endl;
    std::cout << rang::fg::blue << "This is blue text" << rang::fg::reset
              << std::endl;
    std::cout << rang::fg::magenta << "This is magenta text" << rang::fg::reset
              << std::endl;
    std::cout << rang::fg::cyan << "This is cyan text" << rang::fg::reset
              << std::endl;
    std::cout << rang::fg::gray << "This is gray text" << rang::fg::reset
              << std::endl;
    std::cout << std::endl;

    // Text styles
    std::cout << rang::style::bold << "This is bold text" << rang::style::reset
              << std::endl;
    std::cout << rang::style::italic << "This is italic text"
              << rang::style::reset << std::endl;
    std::cout << rang::style::underline << "This is underlined text"
              << rang::style::reset << std::endl;
    std::cout << std::endl;

    // Combining styles and colors
    std::cout << rang::style::bold << rang::fg::green
              << "This is bold green text" << rang::style::reset
              << rang::fg::reset << std::endl;

    std::cout << rang::style::underline << rang::fg::blue
              << "This is underlined blue text" << rang::style::reset
              << rang::fg::reset << std::endl;
    std::cout << std::endl;

    // Background colors
    std::cout << rang::bg::yellow << rang::fg::black
              << "This is black text on yellow background" << rang::bg::reset
              << rang::fg::reset << std::endl;

    std::cout << rang::bg::blue << rang::fg::cyan
              << "This is cyan text on blue background" << rang::bg::reset
              << rang::fg::reset << std::endl;
    std::cout << std::endl;

    // Log-like output
    std::cout << rang::fg::green << "[INFO] " << rang::fg::reset
              << "This is an informational message" << std::endl;

    std::cout << rang::fg::yellow << "[WARNING] " << rang::fg::reset
              << "This is a warning message" << std::endl;

    std::cout << rang::fg::red << rang::style::bold << "[ERROR] "
              << rang::style::reset << rang::fg::reset
              << "This is an error message" << std::endl;

    return 0;
}