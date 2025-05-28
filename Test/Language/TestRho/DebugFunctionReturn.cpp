#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"

using namespace kai;

TEST(DebugFunctionReturn, SimpleReturn) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define a simple function
    console.Execute(R"(
fun getTwo()
    return 2
)", Structure::Program);
    
    // Test that the function returns a value
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("getTwo()", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty()) << "Stack should have the return value";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 2);
}

TEST(DebugFunctionReturn, NestedCallDebug) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun inner()
    return 5
    
fun outer(x)
    return x + 1
)", Structure::Program);
    
    // Test nested call
    console.GetExecutor()->GetDataStack()->Clear();
    
    // This should:
    // 1. Call inner() which returns 5
    // 2. Pass 5 to outer(x) 
    // 3. outer returns 5 + 1 = 6
    console.Execute("outer(inner())", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    
    // Debug: print stack size
    std::cout << "Stack size after nested call: " << stack->Size() << std::endl;
    
    if (!stack->Empty()) {
        auto top = stack->Top();
        std::cout << "Stack top type: " << top.GetClass()->GetName() << std::endl;
        if (top.IsType<int>()) {
            std::cout << "Stack top value: " << ConstDeref<int>(top) << std::endl;
        }
    }
    
    ASSERT_FALSE(stack->Empty()) << "Stack should have result";
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 6) << "outer(inner()) should be 6";
}

TEST(DebugFunctionReturn, TraceExecution) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun double(n)
    return n * 2
)", Structure::Program);
    
    // Execute double(double(2)) and see what happens
    console.GetExecutor()->GetDataStack()->Clear();
    
    // Enable more detailed tracing
    std::cout << "\n=== Executing: double(double(2)) ===\n";
    console.Execute("double(double(2))", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    std::cout << "Final stack size: " << stack->Size() << std::endl;
    if (!stack->Empty()) {
        std::cout << "Stack top: " << stack->Top().ToString() << std::endl;
    }
}