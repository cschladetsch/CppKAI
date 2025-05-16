#include <iostream>
#include <string>

// Include the rang.hpp header for colored output
#include "Include/rang.hpp"

// Simple test for colored logging
int main() {
    std::cout << "Testing colored logging output\n" << std::endl;

    // Test different colors for different log levels
    std::cout << rang::fg::green << "[INFO] " << rang::fg::gray << "This is an info message" << rang::fg::reset << std::endl;
    std::cout << rang::fg::yellow << "[WARNING] " << rang::fg::gray << "This is a warning message" << rang::fg::reset << std::endl;
    std::cout << rang::fg::red << "[ERROR] " << rang::fg::gray << "This is an error message" << rang::fg::reset << std::endl;
    std::cout << rang::style::bold << rang::fg::red << "[FATAL] " << rang::fg::gray << "This is a fatal message" 
              << rang::style::reset << rang::fg::reset << std::endl;

    // Test file location formatting
    std::cout << "\nTesting file location formatting:\n" << std::endl;
    
    std::cout << rang::fg::gray << "[test_file.cpp:42] " 
              << rang::fg::green << "[INFO] " 
              << rang::fg::reset << "Log message with file and line"
              << rang::fg::reset << std::endl;
              
    std::cout << rang::fg::gray << "[network_client.cpp:123] " 
              << rang::fg::magenta << "[NETWORK] "
              << rang::fg::reset << "connection: Connected to server"
              << rang::fg::reset << std::endl;
              
    std::cout << "\nDifferent network categories:\n" << std::endl;
    
    std::cout << rang::style::bold << rang::fg::magenta << "[NETWORK] connection: New connection established" << rang::style::reset << rang::fg::reset << std::endl;
    std::cout << rang::style::bold << rang::fg::blue << "[NETWORK] message: Received data packet" << rang::style::reset << rang::fg::reset << std::endl;
    std::cout << rang::style::bold << rang::fg::cyan << "[NETWORK] discovery: Found server at 192.168.1.1" << rang::style::reset << rang::fg::reset << std::endl;
    std::cout << rang::style::bold << rang::fg::green << "[NETWORK] status: Network is healthy" << rang::style::reset << rang::fg::reset << std::endl;
    
    std::cout << "\nTest completed successfully!" << std::endl;
    
    return 0;
}