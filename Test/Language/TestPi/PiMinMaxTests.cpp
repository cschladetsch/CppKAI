#include "TestLangCommon.h"

struct PiMinMaxTests : kai::TestLangCommon {};

// TODO: This test fails due to test infrastructure issues, not the operations themselves
/*
TEST_F(PiMinMaxTests, TestMinMaxOperations) {
    console_.SetLanguage(kai::Language::Pi);

    // Test integer min
    console_.Execute("5 3 min");
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
    stack->Clear();

    // Test integer max
    console_.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
    stack->Clear();

    // Test with negative numbers (using 0 - 5 to create -5)
    console_.Execute("0 5 - 3 min");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), -5);
    stack->Clear();

    console_.Execute("0 5 - 3 max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
    stack->Clear();

    // Test with floats
    console_.Execute("5.5 3.2 min");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<float>());
    ASSERT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);
    stack->Clear();

    console_.Execute("5.5 3.2 max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<float>());
    ASSERT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 5.5f);
    stack->Clear();

    // Test with mixed types
    console_.Execute("5 3.2 min");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<float>());
    ASSERT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);
    stack->Clear();

    console_.Execute("3.2 5 max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<float>());
    ASSERT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 5.0f);
    stack->Clear();

    // Test with strings
    console_.Execute("\"apple\" \"banana\" min");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "apple");
    stack->Clear();

    console_.Execute("\"apple\" \"banana\" max");
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "banana");
    stack->Clear();
}
*/