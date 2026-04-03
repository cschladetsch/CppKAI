#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test to understand inline execution of if/else
struct RhoIfElseInlineTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoIfElseInlineTest, SimpleIfElse) {
    // Simple if-else without function calls
    console_.Execute(
        "fun simple(n)\n"
        "    if n == 0\n"
        "        return 100\n"
        "    else\n"
        "        return 200");

    data_->Clear();
    console_.Execute("simple(0)");
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 100);

    data_->Clear();
    console_.Execute("simple(1)");
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 200);
}

TEST_F(RhoIfElseInlineTest, IfElseWithExpression) {
    // If-else with expressions
    console_.Execute(
        "fun expr(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    else\n"
        "        return n * 2");

    data_->Clear();
    console_.Execute("expr(5)");
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoIfElseInlineTest, IfElseWithFunctionCall) {
    // Helper function
    console_.Execute(
        "fun double(x)\n"
        "    return x * 2");

    // If-else with function call
    console_.Execute(
        "fun with_call(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    else\n"
        "        return double(n)");

    data_->Clear();
    console_.Execute("with_call(5)");
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 10);
}

TEST_F(RhoIfElseInlineTest, RecursiveSimple) {
    // The problematic recursive case
    console_.Execute(
        "fun rec(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + rec(n - 1)");

    data_->Clear();
    console_.Execute("rec(1)");
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "rec(1) = " << result1 << " (expected 1)" << std::endl;
    EXPECT_EQ(result1, 1);

    data_->Clear();
    console_.Execute("rec(2)");
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "rec(2) = " << result2 << " (expected 3)" << std::endl;
    EXPECT_EQ(result2, 3);
}