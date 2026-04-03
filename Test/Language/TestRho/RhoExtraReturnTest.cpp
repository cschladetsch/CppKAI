#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test to verify the extra return issue
struct RhoExtraReturnTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoExtraReturnTest, IfElseWithReturns) {
    // Both branches have returns - should not add extra return
    console_.Execute(
        "fun both_return(n)\n"
        "    if n == 0\n"
        "        return 100\n"
        "    else\n"
        "        return 200");

    data_->Clear();
    console_.Execute("both_return(0)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 100);

    data_->Clear();
    console_.Execute("both_return(1)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 200);
}

TEST_F(RhoExtraReturnTest, IfWithoutElseButReturn) {
    // Only if branch has return, no else
    console_.Execute(
        "fun if_only_return(n)\n"
        "    if n == 0\n"
        "        return 100\n"
        "    return 200");

    data_->Clear();
    console_.Execute("if_only_return(0)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 100);

    data_->Clear();
    console_.Execute("if_only_return(1)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 200);
}

TEST_F(RhoExtraReturnTest, RecursionWithIfElse) {
    // The actual recursive case that's failing
    console_.Execute(
        "fun rec_sum(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + rec_sum(n - 1)");

    // Test small values first
    data_->Clear();
    console_.Execute("rec_sum(1)");
    ASSERT_EQ(data_->Size(), 1);
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "rec_sum(1) = " << result1 << " (expected 1)" << std::endl;
    EXPECT_EQ(result1, 1);

    data_->Clear();
    console_.Execute("rec_sum(2)");
    ASSERT_EQ(data_->Size(), 1);
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "rec_sum(2) = " << result2 << " (expected 3)" << std::endl;
    EXPECT_EQ(result2, 3);

    data_->Clear();
    console_.Execute("rec_sum(3)");
    ASSERT_EQ(data_->Size(), 1);
    auto result3 = kai::ConstDeref<int>(data_->Top());
    std::cout << "rec_sum(3) = " << result3 << " (expected 6)" << std::endl;
    EXPECT_EQ(result3, 6);
}