#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Debug specific recursion issues
struct RhoRecursionDebugSpecific : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoRecursionDebugSpecific, DebugSum3) {
    // Test debug_sum(3) step by step
    console_.Execute(
        "fun debug_sum(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        prev = debug_sum(n - 1)\n"
        "        result = n + prev\n"
        "        return result");

    // First test debug_sum(1)
    data_->Clear();
    console_.Execute("debug_sum(1)");
    ASSERT_EQ(data_->Size(), 1);
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "debug_sum(1) = " << result1 << " (expected 1)" << std::endl;
    EXPECT_EQ(result1, 1);

    // Test debug_sum(2)
    data_->Clear();
    console_.Execute("debug_sum(2)");
    ASSERT_EQ(data_->Size(), 1);
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "debug_sum(2) = " << result2 << " (expected 3)" << std::endl;
    EXPECT_EQ(result2, 3);

    // Test debug_sum(3)
    data_->Clear();
    console_.Execute("debug_sum(3)");
    ASSERT_EQ(data_->Size(), 1);
    auto result3 = kai::ConstDeref<int>(data_->Top());
    std::cout << "debug_sum(3) = " << result3 << " (expected 6)" << std::endl;
    EXPECT_EQ(result3, 6);
}

TEST_F(RhoRecursionDebugSpecific, SimpleAddRecursion) {
    // Simplify to understand the issue
    console_.Execute(
        "fun add_up(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    return n + add_up(n - 1)");

    // Test add_up(1)
    data_->Clear();
    console_.Execute("add_up(1)");
    ASSERT_EQ(data_->Size(), 1);
    auto result1 = kai::ConstDeref<int>(data_->Top());
    std::cout << "add_up(1) = " << result1 << " (expected 1)" << std::endl;
    EXPECT_EQ(result1, 1);

    // Test add_up(2)
    data_->Clear();
    console_.Execute("add_up(2)");
    ASSERT_EQ(data_->Size(), 1);
    auto result2 = kai::ConstDeref<int>(data_->Top());
    std::cout << "add_up(2) = " << result2 << " (expected 3)" << std::endl;
    EXPECT_EQ(result2, 3);

    // Test add_up(3)
    data_->Clear();
    console_.Execute("add_up(3)");
    ASSERT_EQ(data_->Size(), 1);
    auto result3 = kai::ConstDeref<int>(data_->Top());
    std::cout << "add_up(3) = " << result3 << " (expected 6)" << std::endl;
    EXPECT_EQ(result3, 6);
}

TEST_F(RhoRecursionDebugSpecific, RecursionWithPrint) {
    // Add print statements to debug
    console_.Execute(
        "fun trace_sum(n)\n"
        "    print(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    sub = trace_sum(n - 1)\n"
        "    total = n + sub\n"
        "    return total");

    data_->Clear();
    std::cout << "\nCalling trace_sum(3):" << std::endl;
    console_.Execute("trace_sum(3)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "trace_sum(3) = " << result << " (expected 6)" << std::endl;
    EXPECT_EQ(result, 6);
}