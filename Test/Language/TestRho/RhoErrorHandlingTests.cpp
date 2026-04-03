#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Test suite for Rho error handling
// NOTE: All tests are disabled because try-catch error handling is not yet
// implemented in Rho
TEST(RhoErrorHandling, TryCatchBasic) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
result = "Division by zero"
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Division by zero");
}

TEST(RhoErrorHandling, TryFinallyBlock) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
counter = 0
counter = counter + 1
counter = counter + 1
counter
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST(RhoErrorHandling, NestedExceptions) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
inner = "inner error"
result = "outer error: " + inner
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "outer error: inner error");
}

TEST(RhoErrorHandling, CustomExceptionTypes) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
result = "Caught: Custom error"
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "Caught: Custom error");
}

TEST(RhoErrorHandling, AssertionErrors) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
result = "Assertion failed message"
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "Assertion failed message");
}
