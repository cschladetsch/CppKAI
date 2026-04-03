#include <gtest/gtest.h>

#include "KAI/Console/Console.h"
#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Executor/Executor.h"

// Debug simple function execution
TEST(FunctionDebug, SimpleReturn) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Test 1: Define a simple function that returns a constant
    console.Execute(R"(
fun getConstant()
    42
)",
                    kai::Structure::Program);

    // Clear stack
    exec->GetDataStack()->Clear();

    // Test 2: Call the function
    console.Execute("getConstant()", kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_FALSE(stack->Empty()) << "Stack should have result";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(FunctionDebug, FunctionWithReturn) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Test with explicit return
    console.Execute(R"(
fun getNumber()
    return 100
)",
                    kai::Structure::Program);

    exec->GetDataStack()->Clear();

    console.Execute("getNumber()", kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_FALSE(stack->Empty()) << "Stack should have result";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
}

TEST(FunctionDebug, FunctionWithParams) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Test function with parameters
    console.Execute(R"(
fun add(a, b)
    a + b
)",
                    kai::Structure::Program);

    exec->GetDataStack()->Clear();

    console.Execute("add(3, 4)", kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_FALSE(stack->Empty()) << "Stack should have result";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
}