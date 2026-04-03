#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Simplest possible test to debug recursion issue
struct RhoRecursionSimpleDebug : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoRecursionSimpleDebug, BasicRecursionDebug) {
    // Enable detailed tracing
    kai::Process::trace = 10;

    // Define a simple recursive function
    console_.Execute(
        "fun simple_rec(n)\n"
        "    if n <= 0\n"
        "        return 100\n"
        "    return simple_rec(n - 1)");

    // Test with n=1
    data_->Clear();
    std::cout << "\n=== Calling simple_rec(1) ===" << std::endl;
    console_.Execute("simple_rec(1)");

    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\nsimple_rec(1) returned: " << result << " (expected 100)"
              << std::endl;
    EXPECT_EQ(result, 100);
}

TEST_F(RhoRecursionSimpleDebug, RecursionWithAddition) {
    // Test what happens when we try to add to the recursive result
    console_.Execute(
        "fun add_rec(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    return 1 + add_rec(n - 1)");

    // Test with n=2
    data_->Clear();
    console_.Execute("add_rec(2)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "add_rec(2) returned: " << result << " (expected 2)"
              << std::endl;
    EXPECT_EQ(result, 2);
}

TEST_F(RhoRecursionSimpleDebug, TailRecursion) {
    // Test tail recursion specifically
    console_.Execute(
        "fun tail_rec(n, acc)\n"
        "    if n <= 0\n"
        "        return acc\n"
        "    return tail_rec(n - 1, acc + n)");

    // Test tail_rec(3, 0) = 3 + 2 + 1 + 0 = 6
    data_->Clear();
    console_.Execute("tail_rec(3, 0)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "tail_rec(3, 0) returned: " << result << " (expected 6)"
              << std::endl;
    EXPECT_EQ(result, 6);
}