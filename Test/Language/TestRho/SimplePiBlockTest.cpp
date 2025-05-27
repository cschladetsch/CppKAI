#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Simple test for pi block functionality
TEST(SimplePiBlock, SingleLineExpression) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Single line pi block expression - no newlines
    console.Execute("pi { 2 3 + }");
    auto stack = exec->GetDataStack();

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_EQ(stack->Size(), 1) << "Stack should have exactly one element";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST(SimplePiBlock, SingleLineAssignment) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Single line assignment with pi block - use semicolon separator
    console.Execute("result = pi { 10 20 + }; result");
    auto stack = exec->GetDataStack();

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_EQ(stack->Size(), 1) << "Stack should have exactly one element";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
}

TEST(SimplePiBlock, DirectArithmetic) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Test direct arithmetic without pi block first
    console.Execute("2 + 3");
    auto stack = exec->GetDataStack();

    ASSERT_FALSE(stack->Empty()) << "Stack should not be empty";
    ASSERT_EQ(stack->Size(), 1) << "Stack should have exactly one element";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}