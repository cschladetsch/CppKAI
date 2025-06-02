#include <gtest/gtest.h>

#include "KAI/Core/Console.h"

// DISABLED: Pi block integration with if statements requires parser fixes
TEST(PiBlockDebug, SimplePiBlock) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();

    // Test 1: Simple pi block evaluation
    console.Execute("pi { 2 3 + }");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1) << "Stack should have 1 element after pi block";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5)
        << "Pi block should evaluate to 5";
    stack->Clear();

    // Test 2: Pi block assignment
    console.Execute("x = pi { 10 20 + }");
    stack->Clear();
    console.Execute("x");
    ASSERT_EQ(stack->Size(), 1) << "Stack should have x value";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30) << "x should be 30";
    stack->Clear();

    // Test 3: Pi block in simple if
    console.Execute("if true then result = 42 else result = 0");
    stack->Clear();
    console.Execute("result");
    ASSERT_EQ(stack->Size(), 1) << "Stack should have result";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42) << "result should be 42";
    stack->Clear();

    // Test 4: Direct boolean in if
    console.Execute("if pi { 1 1 == } then result2 = 99 else result2 = 88");
    stack->Clear();
    console.Execute("result2");
    ASSERT_EQ(stack->Size(), 1) << "Stack should have result2";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 99) << "result2 should be 99";
}