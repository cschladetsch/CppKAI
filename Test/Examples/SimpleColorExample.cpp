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
    std::cout << rang::Fg::Green << "This is green text" << rang::Fg::Reset
              << std::endl;
    std::cout << rang::Fg::Red << "This is red text" << rang::Fg::Reset
              << std::endl;
    std::cout << rang::Fg::Yellow << "This is yellow text" << rang::Fg::Reset
              << std::endl;
    std::cout << rang::Fg::Blue << "This is blue text" << rang::Fg::Reset
              << std::endl;
    std::cout << rang::Fg::Magenta << "This is magenta text" << rang::Fg::Reset
              << std::endl;
    std::cout << rang::Fg::Cyan << "This is cyan text" << rang::Fg::Reset
              << std::endl;
    std::cout << rang::Fg::Gray << "This is gray text" << rang::Fg::Reset
              << std::endl;
    std::cout << std::endl;

    // Text styles
    std::cout << rang::Style::Bold << "This is bold text" << rang::Style::Reset
              << std::endl;
    std::cout << rang::Style::Italic << "This is italic text"
              << rang::Style::Reset << std::endl;
    std::cout << rang::Style::Underline << "This is underlined text"
              << rang::Style::Reset << std::endl;
    std::cout << std::endl;

    // Combining styles and colors
    std::cout << rang::Style::Bold << rang::Fg::Green
              << "This is bold green text" << rang::Style::Reset
              << rang::Fg::Reset << std::endl;

    std::cout << rang::Style::Underline << rang::Fg::Blue
              << "This is underlined blue text" << rang::Style::Reset
              << rang::Fg::Reset << std::endl;
    std::cout << std::endl;

    // Background colors
    std::cout << rang::Bg::Yellow << rang::Fg::Black
              << "This is black text on yellow background" << rang::Bg::Reset
              << rang::Fg::Reset << std::endl;

    std::cout << rang::Bg::Blue << rang::Fg::Cyan
              << "This is cyan text on blue background" << rang::Bg::Reset
              << rang::Fg::Reset << std::endl;
    std::cout << std::endl;

    // Log-like output
    std::cout << rang::Fg::Green << "[INFO] " << rang::Fg::Reset
              << "This is an informational message" << std::endl;

    std::cout << rang::Fg::Yellow << "[WARNING] " << rang::Fg::Reset
              << "This is a warning message" << std::endl;

    std::cout << rang::Fg::Red << rang::Style::Bold << "[ERROR] "
              << rang::Style::Reset << rang::Fg::Reset
              << "This is an error message" << std::endl;

    return 0;
}
