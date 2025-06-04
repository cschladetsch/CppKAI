#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

// Minimal test to debug recursion issue
struct RhoRecursionDebug : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoRecursionDebug, MinimalRecursion) {
    // Enable maximum tracing
    kai::Process::trace = 20;

    // Simplest possible recursive function
    console_.Execute(
        "fun simple(n)\n"
        "    if n <= 0\n"
        "        return 42\n"
        "    else\n"
        "        return simple(n - 1)");

    // Test simple(1) - should return 42
    data_->Clear();
    std::cout << "\n\n===== CALLING simple(1) =====\n" << std::endl;
    console_.Execute("simple(1)");

    kai::Process::trace = 0;

    ASSERT_EQ(data_->Size(), 1);
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "\nsimple(1) returned: " << result << " (expected 42)"
              << std::endl;
    EXPECT_EQ(result, 42);
}