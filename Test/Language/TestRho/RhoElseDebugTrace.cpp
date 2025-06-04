#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Debug the else branch issue with detailed tracing
struct RhoElseDebugTrace : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoElseDebugTrace, TraceElseBranch) {
    // Enable detailed tracing
    kai::Process::trace = 1;

    // Simple function with else that shows the issue
    console_.Execute(
        "fun test_else(n)\n"
        "    if n <= 0\n"
        "        return 99\n"
        "    else\n"
        "        return n");

    // Test with n=1 to see what happens
    data_->Clear();
    std::cout << "\n=== Calling test_else(1) ===" << std::endl;
    console_.Execute("test_else(1)");

    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\ntest_else(1) returned: " << result << " (expected 1)"
              << std::endl;
    EXPECT_EQ(result, 1);
}

TEST_F(RhoElseDebugTrace, CompareIfElseStructure) {
    // Compare how if-else is structured vs plain if

    // Version 1: Plain if with implicit else
    console_.Execute(
        "fun plain_if(n)\n"
        "    if n == 0\n"
        "        return 100\n"
        "    return 200");

    // Version 2: Explicit else
    console_.Execute(
        "fun explicit_else(n)\n"
        "    if n == 0\n"
        "        return 100\n"
        "    else\n"
        "        return 200");

    // Test both versions
    data_->Clear();
    console_.Execute("plain_if(1)");
    auto result1 = kai::ConstDeref<int>(data_->Top());

    data_->Clear();
    console_.Execute("explicit_else(1)");
    auto result2 = kai::ConstDeref<int>(data_->Top());

    std::cout << "plain_if(1) = " << result1 << std::endl;
    std::cout << "explicit_else(1) = " << result2 << std::endl;

    EXPECT_EQ(result1, 200);
    EXPECT_EQ(result2, 200);
}