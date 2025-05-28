#include "TestLangCommon.h"

struct PiMinMaxTests : kai::TestLangCommon {};

// These tests have been converted to standalone pattern to avoid TestLangCommon issues
// Test mixed type min operations
TEST(PiMinMaxMixedTypes, MinIntFloat) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();

    // Test min with int and float
    console.Execute("5 3.2 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<float>());
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);
}

// Test mixed type max operations  
TEST(PiMinMaxMixedTypes, MaxFloatInt) {
    kai::Console console;
    console.SetLanguage(kai::Language::Pi);
    auto exec = console.GetExecutor();

    // Test max with float and int
    console.Execute("3.2 5 max");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // When comparing float and int, the result type depends on the implementation
    // In this case, int 5 is greater than float 3.2, so result is int
    ASSERT_TRUE(stack->Top().IsType<int>());
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}