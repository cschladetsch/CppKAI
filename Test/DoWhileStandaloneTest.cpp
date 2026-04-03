#include <KAI/Core/Console.h>

#include <iostream>

KAI_BEGIN

int main() {
    std::cout << "=== Do-While Loop Standalone Test ===" << std::endl;

    // Create registry and console
    Registry reg;
    reg.AddClass<int>("int");
    reg.AddClass<bool>("bool");
    reg.AddClass<float>("float");
    reg.AddClass<String>("String");

    Console console(&reg);
    console.SetLanguage(Language::Rho);

    auto exec = console.GetExecutor();
    exec->SetTraceLevel(5);  // Maximum tracing for debugging

    try {
        std::cout << "Executing simple do-while loop..." << std::endl;

        // Execute a very simple do-while loop
        console.Execute(
            "i = 0;\n"
            "do {\n"
            "    i = i + 1;\n"
            "} while (i < 3);\n");

        // Print result
        auto stack = exec->GetDataStack();
        std::cout << "Stack size after execution: " << stack->Size()
                  << std::endl;

        // Verify result by checking the value of i
        console.Execute("i");

        std::cout << "Final value of i: " << exec->GetDataStack()->Top()
                  << std::endl;

        std::cout << "Do-while loop test completed successfully!" << std::endl;
        return 0;
    } catch (const Exception::Base& e) {
        std::cerr << "KAI exception: " << e.ToString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
    }

    return 1;
}

KAI_END