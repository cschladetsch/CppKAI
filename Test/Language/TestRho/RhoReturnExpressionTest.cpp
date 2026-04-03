#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test to understand how return expressions are evaluated
struct RhoReturnExpressionTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoReturnExpressionTest, SimpleReturnExpression) {
    // Test simple return with expression
    console_.Execute(
        "fun test_return()\n"
        "    return 1 + 2");

    data_->Clear();
    console_.Execute("test_return()");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    EXPECT_EQ(result, 3);
}

TEST_F(RhoReturnExpressionTest, ReturnWithFunctionCall) {
    // Test return with function call in expression
    console_.Execute(
        "fun get_five()\n"
        "    return 5");

    console_.Execute(
        "fun test_return_func()\n"
        "    return 1 + get_five()");

    data_->Clear();
    console_.Execute("test_return_func()");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "test_return_func() returned: " << result << " (expected 6)"
              << std::endl;
    EXPECT_EQ(result, 6);
}

TEST_F(RhoReturnExpressionTest, ReturnWithComplexExpression) {
    // Test return with complex expression including function calls
    console_.Execute(
        "fun double(x)\n"
        "    return x * 2");

    console_.Execute(
        "fun complex_return(n)\n"
        "    return 10 + double(n) + 5");

    data_->Clear();
    console_.Execute("complex_return(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "complex_return(3) returned: " << result
              << " (expected 10 + 6 + 5 = 21)" << std::endl;
    EXPECT_EQ(result, 21);
}

TEST_F(RhoReturnExpressionTest, ReturnOrderAnalysis) {
    // Analyze order of operations in return
    console_.Execute("counter = 0");

    console_.Execute(
        "fun inc_counter()\n"
        "    counter = counter + 1\n"
        "    return counter");

    console_.Execute(
        "fun test_order()\n"
        "    return inc_counter() + inc_counter()");

    data_->Clear();
    console_.Execute("test_order()");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "test_order() returned: " << result << " (expected 1 + 2 = 3)"
              << std::endl;

    // Check final counter value
    data_->Clear();
    console_.Execute("counter");
    auto counter = kai::ConstDeref<int>(data_->Top());
    std::cout << "Final counter value: " << counter << " (expected 2)"
              << std::endl;
}