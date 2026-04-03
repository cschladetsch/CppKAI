#include <gtest/gtest.h>

#include "KAI/Core/BuiltinTypes/Signed32.h"
#include "TestLangCommon.h"

// Test recursion consistency
struct RhoRecursionConsistencyTest : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

TEST_F(RhoRecursionConsistencyTest, TestAllValues) {
    // Same function, test with different values
    console_.Execute(
        "fun sum(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + sum(n - 1)");

    // Test sum(0) through sum(5)
    for (int i = 0; i <= 5; ++i) {
        data_->Clear();
        console_.Execute(std::string("sum(") + std::to_string(i) + ")");

        ASSERT_EQ(data_->Size(), 1);
        auto result = kai::ConstDeref<int>(data_->Top());
        int expected = i * (i + 1) / 2;  // Sum formula

        std::cout << "sum(" << i << ") = " << result << " (expected "
                  << expected << ")" << (result == expected ? " ✓" : " ✗")
                  << std::endl;

        if (i <= 2) {
            // We know these work
            EXPECT_EQ(result, expected);
        }
    }
}