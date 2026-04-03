#include "TestLangCommon.h"

struct PiMinMaxTests : kai::TestLangCommon {};

// Test fixture for mixed type tests
struct PiMinMaxMixedTypesTests : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test mixed type min operations
TEST_F(PiMinMaxMixedTypesTests, MinIntFloat) {
    // Test min with int and float
    console_.Execute("5 3.2 min");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    ASSERT_TRUE(stack->Top().IsType<float>());
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);
}

// Test mixed type max operations
TEST_F(PiMinMaxMixedTypesTests, MaxFloatInt) {
    // Test max with float and int
    console_.Execute("3.2 5 max");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // When comparing float and int, the result type depends on the
    // implementation In this case, int 5 is greater than float 3.2, so result
    // is int
    ASSERT_TRUE(stack->Top().IsType<int>());
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}