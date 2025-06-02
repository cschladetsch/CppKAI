#include <gtest/gtest.h>

#include "TestLangCommon.h"

// NOTE: Pattern matching is not yet implemented in Rho.
// All tests in this file are disabled until the feature is implemented.

// Test suite for Rho pattern matching and destructuring
TEST(RhoPatternMatching, BasicPatternMatch) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Test basic pattern matching
    console.Execute(R"(
        value = 42
        result = match value
            0 -> "zero"
            42 -> "forty-two"
            _ -> "other"
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "forty-two");
}

TEST(RhoPatternMatching, TupleDestructuring) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        point = (10, 20)
        (x, y) = point
        x + y
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
}

TEST(RhoPatternMatching, ListPatternMatch) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        list = [1, 2, 3, 4]
        result = match list
            [] -> "empty"
            [x] -> "single"
            [x, y, ...rest] -> "multiple"
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "multiple");
}

TEST(RhoPatternMatching, GuardClauses) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        age = 25
        category = match age
            x if x < 18 -> "minor"
            x if x < 65 -> "adult"
            _ -> "senior"
        category
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "adult");
}

TEST(RhoPatternMatching, NestedPatterns) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
        data = [(1, "a"), (2, "b"), (3, "c")]
        result = match data
            [(1, x), ...] -> x
            _ -> "not found"
        result
    )");

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "a");
}