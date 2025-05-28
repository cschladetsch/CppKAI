#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include "KAI/Console.h"

using namespace kai;

TEST(DebugNestedCalls, SimpleNested) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // First define the functions
    console.Execute(R"(
fun double(n)
    return n * 2

fun increment(n)
    return n + 1
)", Structure::Program);
    
    // Test 1: Single function call
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("double(2)", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 4) << "double(2) should be 4";
    
    // Test 2: Simple nested call
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("double(increment(2))", Structure::Expression);
    stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 6) << "double(increment(2)) should be 6";
    
    // Test 3: The failing case from the test
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("double(increment(increment(double(2))))", Structure::Expression);
    stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    int result = ConstDeref<int>(stack->Top());
    EXPECT_EQ(result, 12) << "double(increment(increment(double(2)))) should be 12, got " << result;
}

TEST(DebugNestedCalls, StepByStep) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define functions
    console.Execute(R"(
fun double(n)
    return n * 2

fun increment(n)
    return n + 1
)", Structure::Program);
    
    // Execute step by step to see intermediate results
    console.GetExecutor()->GetDataStack()->Clear();
    
    // Step 1: double(2) = 4
    console.Execute("x = double(2)", Structure::Statement);
    console.Execute("x", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 4) << "x should be 4";
    stack->Pop();
    
    // Step 2: increment(x) = 5
    console.Execute("y = increment(x)", Structure::Statement);
    console.Execute("y", Structure::Expression);
    stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5) << "y should be 5";
    stack->Pop();
    
    // Step 3: increment(y) = 6
    console.Execute("z = increment(y)", Structure::Statement);
    console.Execute("z", Structure::Expression);
    stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 6) << "z should be 6";
    stack->Pop();
    
    // Step 4: double(z) = 12
    console.Execute("result = double(z)", Structure::Statement);
    console.Execute("result", Structure::Expression);
    stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 12) << "result should be 12";
}

TEST(DebugNestedCalls, ArgumentOrdering) {
    Console console;
    console.SetLanguage(Language::Rho);
    
    // Define a function that shows argument order
    console.Execute(R"(
fun subtract(a, b)
    return a - b
)", Structure::Program);
    
    // Test simple call
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("subtract(10, 3)", Structure::Expression);
    auto stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 7) << "subtract(10, 3) should be 7";
    
    // Test nested call - this helps verify argument evaluation order
    console.GetExecutor()->GetDataStack()->Clear();
    console.Execute("subtract(subtract(10, 3), 2)", Structure::Expression);
    stack = console.GetExecutor()->GetDataStack();
    ASSERT_FALSE(stack->Empty());
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5) << "subtract(subtract(10, 3), 2) should be 5";
}