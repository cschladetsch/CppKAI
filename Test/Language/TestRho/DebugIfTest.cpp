#include <gtest/gtest.h>
#include "KAI/Core/Console.h"

TEST(RhoDebug, SimpleIf) {
    kai::Console console;
    console.SetLanguage(kai::Language::Rho);
    auto exec = console.GetExecutor();
    
    // Test 1: Simple boolean literal
    console.Execute("true");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1) << "Stack should have boolean";
    ASSERT_TRUE(stack->Top().IsType<bool>()) << "Should be bool type";
    EXPECT_TRUE(kai::ConstDeref<bool>(stack->Top())) << "Should be true";
    stack->Clear();
    
    // Test 2: Simple assignment
    console.Execute("x = 42");
    stack->Clear();
    console.Execute("x");
    ASSERT_EQ(stack->Size(), 1) << "Stack should have x";
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42) << "x should be 42";
    stack->Clear();
    
    // Test 3: Simple if with literal condition - using 'then'
    try {
        console.Execute("if true then y = 100");
        stack->Clear();
        console.Execute("y");
        ASSERT_EQ(stack->Size(), 1) << "Stack should have y";
        EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100) << "y should be 100";
    } catch (const std::exception& e) {
        FAIL() << "Exception in if statement: " << e.what();
    }
}