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
    std::cout << rang::fg::Green << "This is green text" << rang::fg::Reset
              << std::endl;
    std::cout << rang::fg::Red << "This is red text" << rang::fg::Reset
              << std::endl;
    std::cout << rang::fg::Yellow << "This is yellow text" << rang::fg::Reset
              << std::endl;
    std::cout << rang::fg::Blue << "This is blue text" << rang::fg::Reset
              << std::endl;
    std::cout << rang::fg::Magenta << "This is magenta text" << rang::fg::Reset
              << std::endl;
    std::cout << rang::fg::Cyan << "This is cyan text" << rang::fg::Reset
              << std::endl;
    std::cout << rang::fg::Gray << "This is gray text" << rang::fg::Reset
              << std::endl;
    std::cout << std::endl;

    // Text styles
    std::cout << rang::style::Bold << "This is bold text" << rang::style::Reset
              << std::endl;
    std::cout << rang::style::Italic << "This is italic text"
              << rang::style::Reset << std::endl;
    std::cout << rang::style::Underline << "This is underlined text"
              << rang::style::Reset << std::endl;
    std::cout << std::endl;

    // Combining styles and colors
    std::cout << rang::style::Bold << rang::fg::Green
              << "This is bold green text" << rang::style::Reset
              << rang::fg::Reset << std::endl;

    std::cout << rang::style::Underline << rang::fg::Blue
              << "This is underlined blue text" << rang::style::Reset
              << rang::fg::Reset << std::endl;
    std::cout << std::endl;

    // Background colors
    std::cout << rang::bg::Yellow << rang::fg::Black
              << "This is black text on yellow background" << rang::bg::Reset
              << rang::fg::Reset << std::endl;

    std::cout << rang::bg::Blue << rang::fg::Cyan
              << "This is cyan text on blue background" << rang::bg::Reset
              << rang::fg::Reset << std::endl;
    std::cout << std::endl;

    // Log-like output
    std::cout << rang::fg::Green << "[INFO] " << rang::fg::Reset
              << "This is an informational message" << std::endl;

    std::cout << rang::fg::Yellow << "[WARNING] " << rang::fg::Reset
              << "This is a warning message" << std::endl;

    std::cout << rang::fg::Red << rang::style::Bold << "[ERROR] "
              << rang::style::Reset << rang::fg::Reset
              << "This is an error message" << std::endl;

    return 0;
}

