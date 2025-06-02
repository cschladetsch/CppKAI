#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test suite for Rho lambda expressions and closures
TEST(RhoLambda, SimpleLambdaExpression) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        double = lambda(x) x * 2
        double(21)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(RhoLambda, LambdaWithMultipleParams) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        add = lambda(x, y) x + y
        add(15, 27)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(RhoLambda, ClosureCapture) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        multiplier = 10
        scale = lambda(x) x * multiplier
        scale(5)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 50);
}

TEST(RhoLambda, HigherOrderFunctions) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        apply_twice = lambda(f, x) f(f(x))
        increment = lambda(n) n + 1
        apply_twice(increment, 5)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
}

TEST(RhoLambda, RecursiveLambda) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        factorial = lambda(n) 
            if n <= 1
                1
            else
                n * factorial(n - 1)
        factorial(5)
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 120);
}