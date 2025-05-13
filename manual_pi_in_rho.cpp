#include <iostream>
#include <KAI/Console/Console.h>

// Simple program to manually test Pi assert in Rho
int main() {
    // Initialization
    try {
        std::cout << "Running manual Pi in Rho assert test" << std::endl;
        kai::Console console;
        console.SetLanguage(kai::Language::Rho);
        
        std::cout << "Testing Pi assertion in Rho:" << std::endl;
        console.Process("pi { 1 1 + 2 == assert }");
        
        // If we get here, the assertion passed
        std::cout << "✅ Assertion passed!" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }
}