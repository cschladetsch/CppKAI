#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Debug the specific issue with else in recursion
struct RhoRecursionElseDebug : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoRecursionElseDebug, TraceRecSum2) {
    // Enable detailed tracing for rec_sum(2)
    kai::Process::trace = 10;

    console_.Execute(
        "fun rec_sum(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + rec_sum(n - 1)");

    kai::Process::trace = 0;

    // Now trace rec_sum(2)
    kai::Process::trace = 1;
    data_->Clear();
    std::cout << "\n=== Calling rec_sum(2) with tracing ===" << std::endl;
    console_.Execute("rec_sum(2)");
    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\nrec_sum(2) returned: " << result << std::endl;
}

TEST_F(RhoRecursionElseDebug, StepByStep) {
    // Test each step manually
    console_.Execute(
        "fun rec_test(n)\n"
        "    print(n)\n"  // Print to see what n is
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        temp = rec_test(n - 1)\n"
        "        print(temp)\n"  // Print the recursive result
        "        result = n + temp\n"
        "        print(result)\n"  // Print the sum
        "        return result");

    std::cout << "\n=== Step by step rec_test(2) ===" << std::endl;
    data_->Clear();
    console_.Execute("rec_test(2)");

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\nrec_test(2) final result: " << result << std::endl;
}