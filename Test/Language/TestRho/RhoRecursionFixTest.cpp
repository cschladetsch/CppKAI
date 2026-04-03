#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Test to understand and fix the recursion issue
struct RhoRecursionFixTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoRecursionFixTest, AnalyzeRecursionStructure) {
    // Enable tracing to see the continuation structure
    kai::Process::trace = 1;

    // Very simple recursive function
    console_.Execute(
        "fun rec_test(n)\n"
        "    if n <= 0\n"
        "        return 10\n"
        "    result = rec_test(n - 1)\n"
        "    return result + 1");

    // Test with n=1
    data_->Clear();
    console_.Execute("rec_test(1)");

    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\nrec_test(1) returned: " << result << " (expected 11)"
              << std::endl;
    EXPECT_EQ(result, 11);
}

TEST_F(RhoRecursionFixTest, SimplestPossibleRecursion) {
    // The absolute simplest recursive case
    console_.Execute(
        "fun simple(n)\n"
        "    if n == 0\n"
        "        return 1\n"
        "    return simple(n - 1)");

    data_->Clear();
    console_.Execute("simple(2)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "simple(2) returned: " << result << " (expected 1)"
              << std::endl;
    EXPECT_EQ(result, 1);
}

TEST_F(RhoRecursionFixTest, RecursionWithLocalVariable) {
    // Test if local variables are preserved across recursive calls
    console_.Execute(
        "fun local_test(n)\n"
        "    my_n = n\n"
        "    if my_n <= 0\n"
        "        return 100\n"
        "    inner_result = local_test(my_n - 1)\n"
        "    return inner_result + my_n");

    data_->Clear();
    console_.Execute("local_test(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "local_test(3) returned: " << result
              << " (expected 100 + 1 + 2 + 3 = 106)" << std::endl;
    EXPECT_EQ(result, 106);
}

TEST_F(RhoRecursionFixTest, DebugScopeInRecursion) {
    // Test to understand scope handling in recursion
    console_.Execute("global_counter = 0");

    console_.Execute(
        "fun scope_debug(n)\n"
        "    global_counter = global_counter + 1\n"
        "    local_value = n * 10\n"
        "    if n <= 0\n"
        "        return local_value\n"
        "    sub_result = scope_debug(n - 1)\n"
        "    return local_value + sub_result");

    data_->Clear();
    console_.Execute("scope_debug(2)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "scope_debug(2) returned: " << result
              << " (expected 20 + 10 + 0 = 30)" << std::endl;

    // Check how many times the function was called
    data_->Clear();
    console_.Execute("global_counter");
    auto counter = kai::ConstDeref<int>(data_->Top());
    std::cout << "Function was called " << counter << " times (expected 3)"
              << std::endl;
}