#include <gtest/gtest.h>
#include "TestLangCommon.h"
#include <cmath>

// Test suite for Pi mathematical operations
TEST(PiMathOperations, TrigonometricFunctions) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Test sin
    console.Execute("0.0 sin");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 0.0f);
    
    stack->Clear();
    
    // Test cos
    console.Execute("0.0 cos");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 1.0f);
}

TEST(PiMathOperations, PowerAndSqrt) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Test power
    console.Execute("2 3 pow");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
    
    stack->Clear();
    
    // Test square root
    console.Execute("16.0 sqrt");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 4.0f);
}

TEST(PiMathOperations, MinMaxOperations) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    // Test min
    console.Execute("5 3 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
    
    stack->Clear();
    
    // Test max
    console.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST(PiMathOperations, AbsoluteValue) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    console.Execute("-42 abs");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST(PiMathOperations, ModuloOperation) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();
    
    console.Execute("17 5 %");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}