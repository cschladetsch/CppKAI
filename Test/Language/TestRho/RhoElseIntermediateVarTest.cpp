#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test intermediate variables in else blocks
struct RhoElseIntermediateVarTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoElseIntermediateVarTest, DirectVsIntermediate) {
    // Direct return in else
    console_.Execute(
        "fun direct(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + direct(n - 1)");

    // Intermediate variable in else
    console_.Execute(
        "fun intermediate(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        temp = intermediate(n - 1)\n"
        "        return n + temp");

    // Test both with n=3
    data_->Clear();
    console_.Execute("direct(3)");
    auto directResult = kai::ConstDeref<int>(data_->Top());

    data_->Clear();
    console_.Execute("intermediate(3)");
    auto intermediateResult = kai::ConstDeref<int>(data_->Top());

    std::cout << "direct(3) = " << directResult << " (expected 6)" << std::endl;
    std::cout << "intermediate(3) = " << intermediateResult << " (expected 6)"
              << std::endl;

    EXPECT_EQ(directResult, 6);
    EXPECT_EQ(intermediateResult, 6);
}

TEST_F(RhoElseIntermediateVarTest, MultipleIntermediates) {
    // Multiple intermediate variables
    console_.Execute(
        "fun multi(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        a = n\n"
        "        b = multi(n - 1)\n"
        "        c = a + b\n"
        "        return c");

    data_->Clear();
    console_.Execute("multi(2)");
    auto result = kai::ConstDeref<int>(data_->Top());
    std::cout << "multi(2) = " << result << " (expected 3)" << std::endl;
    EXPECT_EQ(result, 3);
}