#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"

using namespace kai;

TEST(DebugNestedExecution, SimpleTrace) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun inner()
    return 5
    
fun outer(x)
    return x + 1
)", Structure::Program);
    
    // Test the nested call with detailed output
    std::cout << "\n=== Testing: outer(inner()) ===\n";
    
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("outer(inner())", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    std::cout << "Final stack size: " << stack->Size() << std::endl;
    
    if (!stack->Empty()) {
        auto val = stack->Top();
        std::cout << "Stack top: " << val.ToString() << std::endl;
        if (val.IsType<int>()) {
            int result = ConstDeref<int>(val);
            std::cout << "Result: " << result << std::endl;
            EXPECT_EQ(result, 6) << "outer(inner()) should be 6";
        }
    }
}

TEST(DebugNestedExecution, DoubleNested) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define function
    console.Execute(R"(
fun double(n)
    return n * 2
)", Structure::Program);
    
    // Test double(double(2))
    std::cout << "\n=== Testing: double(double(2)) ===\n";
    
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("double(double(2))", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    std::cout << "Final stack size: " << stack->Size() << std::endl;
    
    if (!stack->Empty()) {
        auto val = stack->Top();
        std::cout << "Stack top: " << val.ToString() << std::endl;
        if (val.IsType<int>()) {
            int result = ConstDeref<int>(val);
            std::cout << "Result: " << result << std::endl;
            EXPECT_EQ(result, 8) << "double(double(2)) should be 8";
        }
    }
}

TEST(DebugNestedExecution, ComplexNested) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun double(n)
    return n * 2

fun increment(n)
    return n + 1
)", Structure::Program);
    
    // Test the complex nested call from the failing test
    std::cout << "\n=== Testing: double(increment(increment(double(2)))) ===\n";
    
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("double(increment(increment(double(2))))", Structure::Expression);
    
    auto stack = console.GetExecutor()->GetDataStack();
    std::cout << "Final stack size: " << stack->Size() << std::endl;
    
    if (!stack->Empty()) {
        auto val = stack->Top();
        std::cout << "Stack top: " << val.ToString() << std::endl;
        if (val.IsType<int>()) {
            int result = ConstDeref<int>(val);
            std::cout << "Result: " << result << std::endl;
            // Expected: double(2) = 4, increment(4) = 5, increment(5) = 6, double(6) = 12
            EXPECT_EQ(result, 12) << "double(increment(increment(double(2)))) should be 12";
        }
    } else {
        FAIL() << "Stack is empty!";
    }
}