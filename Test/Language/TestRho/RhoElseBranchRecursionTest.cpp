#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test to understand else branch in recursion
struct RhoElseBranchRecursionTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoElseBranchRecursionTest, ElseBranchVariations) {
    // Version 1: Without else keyword
    console_.Execute(
        "fun sum_no_else(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    return n + sum_no_else(n - 1)");

    data_->Clear();
    console_.Execute("sum_no_else(3)");
    ASSERT_EQ(data_->Size(), 1);
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "sum_no_else(3) = " << result1 << " (expected 6)" << std::endl;
    EXPECT_EQ(result1, 6);

    // Version 2: With else keyword and direct return
    console_.Execute(
        "fun sum_else_direct(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + sum_else_direct(n - 1)");

    data_->Clear();
    console_.Execute("sum_else_direct(3)");
    ASSERT_EQ(data_->Size(), 1);
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "sum_else_direct(3) = " << result2 << " (expected 6)"
              << std::endl;
    EXPECT_EQ(result2, 6);

    // Version 3: With else and intermediate variables (like debug_sum)
    console_.Execute(
        "fun sum_else_vars(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        prev = sum_else_vars(n - 1)\n"
        "        result = n + prev\n"
        "        return result");

    data_->Clear();
    console_.Execute("sum_else_vars(3)");
    ASSERT_EQ(data_->Size(), 1);
    auto result3 = kai::ConstDeref<int>(data_->Top());
    std::cout << "sum_else_vars(3) = " << result3 << " (expected 6)"
              << std::endl;
    EXPECT_EQ(result3, 6);
}

TEST_F(RhoElseBranchRecursionTest, ElseScopeIssue) {
    // Test if else creates a separate scope
    console_.Execute("outer = 100");

    console_.Execute(
        "fun test_else_scope(n)\n"
        "    outer = 200\n"  // Should create local 'outer'
        "    if n == 0\n"
        "        return outer\n"
        "    else\n"
        "        inner = 300\n"
        "        return inner");

    data_->Clear();
    console_.Execute("test_else_scope(0)");
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "test_else_scope(0) = " << result1 << " (expected 200)"
              << std::endl;
    EXPECT_EQ(result1, 200);

    data_->Clear();
    console_.Execute("test_else_scope(1)");
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "test_else_scope(1) = " << result2 << " (expected 300)"
              << std::endl;
    EXPECT_EQ(result2, 300);

    // Check if global outer was modified
    data_->Clear();
    console_.Execute("outer");
    auto global_outer = kai::ConstDeref<int>(data_->Top());
    std::cout << "Global outer = " << global_outer << " (expected 100)"
              << std::endl;
    EXPECT_EQ(global_outer, 100);
}