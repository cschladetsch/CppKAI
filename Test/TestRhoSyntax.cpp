#include <KAI/Console/Console.h>

#include <iostream>

using namespace kai;

int main() {
    try {
        Console console;
        console.SetLanguage(Language::Rho);

        // Test 1: Simple assignment and retrieval
        std::cout << "Test 1: Simple assignment\n";
        console.Execute("x = 42");
        console.Execute("x");

        auto executor = console.GetExecutor();
        auto dataStack = executor->GetDataStack();

        if (!dataStack->Empty()) {
            auto val = dataStack->Top();
            std::cout << "Result: " << val.ToString() << std::endl;
            dataStack->Pop();
        }

        // Test 2: If statement
        std::cout << "\nTest 2: If statement\n";
        console.Execute("result = 0");
        console.Execute("if true { result = 99 }");
        console.Execute("result");

        if (!dataStack->Empty()) {
            auto val = dataStack->Top();
            std::cout << "Result after if: " << val.ToString() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}