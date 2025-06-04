#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Minimal test to debug the else recursion issue
struct RhoElseMinimalDebug : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoElseMinimalDebug, SimplestElseRecursion) {
    // The absolute simplest case that shows the bug
    console_.Execute(
        "fun f(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + f(n - 1)");

    // Test f(1) - should return 1
    data_->Clear();
    console_.Execute("f(1)");
    ASSERT_EQ(data_->Size(), 1);
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "f(1) = " << result1 << " (expected 1)" << std::endl;
    EXPECT_EQ(result1, 1);

    // Test f(2) - should return 3
    data_->Clear();
    console_.Execute("f(2)");
    ASSERT_EQ(data_->Size(), 1);
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "f(2) = " << result2 << " (expected 3)" << std::endl;
    EXPECT_EQ(result2, 3);
}

TEST_F(RhoElseMinimalDebug, WithoutElse) {
    // Same function but without else keyword
    console_.Execute(
        "fun g(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    return n + g(n - 1)");

    // Test g(2) - should return 3
    data_->Clear();
    console_.Execute("g(2)");
    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "g(2) = " << result << " (expected 3)" << std::endl;
    EXPECT_EQ(result, 3);
}

TEST_F(RhoElseMinimalDebug, ElseWithIntermediateVar) {
    // With intermediate variable to see what's happening
    console_.Execute("counter = 0");

    console_.Execute(
        "fun h(n)\n"
        "    counter = counter + 1\n"  // Count calls
        "    if n == 0\n"
        "        return 0\n"
        "    else\n"
        "        temp = h(n - 1)\n"
        "        return n + temp");

    // Test h(2)
    data_->Clear();
    console_.Execute("h(2)");
    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "h(2) = " << result << " (expected 3)" << std::endl;

    // Check how many times h was called
    data_->Clear();
    console_.Execute("counter");
    auto count = kai::ConstDeref<int>(data_->Top());
    std::cout << "h was called " << count << " times (expected 3)" << std::endl;
}