#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"

using namespace kai;

TEST(DebugBytecode, NestedCallExecution) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun inner()
    return 5
    
fun outer(x)
    return x + 1
)", Structure::Program);
    
    // Test execution with the supposedly fixed translator
    std::cout << "\n=== Testing nested call after translator fix ===\n";
    
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("outer(inner())", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    std::cout << "Stack size: " << stack->Size() << std::endl;
    
    if (!stack->Empty()) {
        auto val = stack->Top();
        std::cout << "Result: " << val.ToString() << std::endl;
        if (val.IsType<int>()) {
            int result = ConstDeref<int>(val);
            EXPECT_EQ(result, 6) << "outer(inner()) should be 6 after fix";
        }
    } else {
        FAIL() << "Stack is empty!";
    }
}