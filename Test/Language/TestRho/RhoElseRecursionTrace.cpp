#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Detailed trace of else recursion issue
struct RhoElseRecursionTrace : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoElseRecursionTrace, TraceSimpleRecursion) {
    // Enable maximum tracing
    kai::Process::trace = 10;

    console_.Execute(
        "fun f(n)\n"
        "    if n == 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + f(n - 1)");

    kai::Process::trace = 1;

    // Just trace f(1) to keep output manageable
    data_->Clear();
    std::cout << "\n=== Tracing f(1) ===" << std::endl;
    console_.Execute("f(1)");

    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\nf(1) returned: " << result << " (expected 1)" << std::endl;
}