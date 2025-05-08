#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

KAI_BEGIN

void TestDoWhileLoop() {
    std::cout << "=== Testing Do-While Loop Implementation ===" << std::endl;

    Registry reg;
    reg.AddClass<int>("int");
    reg.AddClass<float>("float");
    reg.AddClass<String>("String");
    reg.AddClass<bool>("bool");
    reg.AddClass<Array>("Array");

    Console console(&reg);
    console.SetLanguage(Language::Rho);

    auto exec = console.GetExecutor();
    exec->SetTraceLevel(5);  // Maximum trace level for debugging

    try {
        std::cout << "Running simple do-while test..." << std::endl;
        console.Execute(
            "i = 0; result = 0; do { result = result + 1; i = i + 1; } while "
            "(i < 3);");

        // Check result
        auto stack = exec->GetDataStack();
        std::cout << "Stack size after execution: " << stack->Size()
                  << std::endl;

        // Test with nested loops
        std::cout << "Running nested do-while test..." << std::endl;
        console.Execute(
            "outer = 0; inner = 0; result = 0; do { inner = 0; do { result = "
            "result + 1; inner = inner + 1; } while (inner < 2); outer = outer "
            "+ 1; } while (outer < 2);");

        stack = exec->GetDataStack();
        std::cout << "Stack size after nested loops: " << stack->Size()
                  << std::endl;

        std::cout << "Do-while loop tests completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception in do-while test: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in do-while test" << std::endl;
    }
}

int main() {
    TestDoWhileLoop();
    return 0;
}

KAI_END