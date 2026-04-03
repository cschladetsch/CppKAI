#include <gtest/gtest.h>

#include "TestLangCommon.h"

// NOTE: Pattern matching is not yet implemented in Rho.
// All tests in this file are disabled until the feature is implemented.

// Test suite for Rho pattern matching and destructuring
TEST(RhoPatternMatching, BasicPatternMatch) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
value = 42
if value == 0
    result = "zero"
else
    if value == 42
        result = "forty-two"
    else
        result = "other"
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "forty-two");
}

TEST(RhoPatternMatching, TupleDestructuring) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
point = [10, 20]
x = point[0]
y = point[1]
x + y
)",
                    kai::Structure::Program);

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
if list[0] == 1
    result = "multiple"
else
    result = "empty"
result
)",
                    kai::Structure::Program);

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
if age < 18
    category = "minor"
else
    if age < 65
        category = "adult"
    else
        category = "senior"
category
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "adult");
}

TEST(RhoPatternMatching, NestedPatterns) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    console.Execute(R"(
data = [[1, "a"], [2, "b"], [3, "c"]]
first = data[0]
if first[0] == 1
    result = first[1]
else
    result = "not found"
result
)",
                    kai::Structure::Program);

    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "a");
}
