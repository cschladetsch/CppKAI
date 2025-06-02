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
        result = try
            10 / 0
        catch e
            "Division by zero"
        result
    )");

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
        try
            counter = counter + 1
            42
        finally
            counter = counter + 1
        counter
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST(RhoErrorHandling, NestedExceptions) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        result = try
            try
                throw "inner error"
            catch e
                throw "outer error: " + e
        catch e
            e
        result
    )");

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
        class MyError(message)
            self.message = message
        
        result = try
            throw MyError("Custom error")
        catch MyError as e
            "Caught: " + e.message
        catch e
            "Unknown error"
        
        result
    )");

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
        result = try
            assert(false, "Assertion failed message")
            "Should not reach here"
        catch AssertionError as e
            e.message
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "Assertion failed message");
}