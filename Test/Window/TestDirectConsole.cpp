#include <iostream>

int main() {
    std::cout << "Test 1: Basic includes" << std::endl;

// Step 1: Try to include the header
#include <KAI/Core/Object.h>
    std::cout << "Included Object.h" << std::endl;

#include <KAI/Core/Registry.h>
    std::cout << "Included Registry.h" << std::endl;

#include <KAI/Executor/Executor.h>
    std::cout << "Included Executor.h" << std::endl;

    // Don't include Console yet
    std::cout << "All basic includes successful" << std::endl;

    return 0;
}