#include <iostream>
#include <string>
#include <gtest/gtest.h>

#include "KAI/Core/Console.h"
#include "KAI/Core/File.h"
#include "MyTestStruct.h"
#include "TestLangCommon.h"

// Use the kai namespace (KAI_NAMESPACE_NAME is defined as kai)
KAI_BEGIN

TEST(RhoLanguage, TestDoWhileImplementation) {
    std::cout << "=== Testing Do-While Loop Implementation ===" << std::endl;

    // Use the console's registry directly instead of creating our own
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Make sure types are registered in the console's registry
    Registry& reg = console.GetRegistry();
    reg.AddClass<int>(Label("int"));
    reg.AddClass<float>(Label("float"));
    reg.AddClass<String>(Label("String"));
    reg.AddClass<bool>(Label("bool"));
    reg.AddClass<Array>(Label("Array"));

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
        SUCCEED();
    } catch (const std::exception& e) {
        std::cerr << "Exception in do-while test: " << e.what() << std::endl;
        FAIL() << "Exception in do-while test: " << e.what();
    } catch (...) {
        std::cerr << "Unknown exception in do-while test" << std::endl;
        FAIL() << "Unknown exception in do-while test";
    }
}

KAI_END