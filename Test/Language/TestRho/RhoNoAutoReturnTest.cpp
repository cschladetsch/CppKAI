#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test functions without automatic returns
struct RhoNoAutoReturnTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoNoAutoReturnTest, FunctionWithoutReturn) {
    // This function has no return - should return void/null
    console_.Execute(
        "fun no_return()\n"
        "    x = 5");

    data_->Clear();
    console_.Execute("no_return()");

    // Without automatic return, this might leave nothing on stack
    // or might have undefined behavior
    EXPECT_EQ(data_->Size(), 0);
}

TEST_F(RhoNoAutoReturnTest, RecursionWithElse) {
    // Test our problematic recursion case
    console_.Execute(
        "fun sum_else(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + sum_else(n - 1)");

    data_->Clear();
    console_.Execute("sum_else(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "sum_else(3) = " << result << " (expected 6)" << std::endl;
    EXPECT_EQ(result, 6);
}