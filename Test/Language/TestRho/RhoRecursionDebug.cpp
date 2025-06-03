#include <gtest/gtest.h>
#include "TestLangCommon.h"

// Debug recursion issues
struct RhoRecursionDebug : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Rho);
    }
};

// Test simple countdown to verify basic recursion
TEST_F(RhoRecursionDebug, SimpleCountdown) {
    console_.Execute(
        "fun countdown(n)\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return n + countdown(n - 1)");
    
    // Test countdown(3) = 3 + 2 + 1 + 0 = 6
    data_->Clear();
    console_.Execute("countdown(3)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
}

// Test factorial step by step
TEST_F(RhoRecursionDebug, FactorialStepByStep) {
    console_.Execute(
        "fun factorial(n)\n"
        "    if n <= 1\n"
        "        return 1\n"
        "    else\n"
        "        return n * factorial(n - 1)");
    
    // Test factorial(1) = 1
    data_->Clear();
    console_.Execute("factorial(1)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 1);
    
    // Test factorial(2) = 2 * 1 = 2
    data_->Clear();
    console_.Execute("factorial(2)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 2);
    
    // Test factorial(3) = 3 * 2 * 1 = 6
    data_->Clear();
    console_.Execute("factorial(3)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 6);
    
    // Test factorial(4) = 4 * 3 * 2 * 1 = 24
    data_->Clear();
    console_.Execute("factorial(4)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 24);
}

// Test if recursion preserves local state
TEST_F(RhoRecursionDebug, RecursionWithLocalState) {
    console_.Execute(
        "fun sumWithLocal(n)\n"
        "    local = n * 2\n"
        "    if n <= 0\n"
        "        return 0\n"
        "    else\n"
        "        return local + sumWithLocal(n - 1)");
    
    // sumWithLocal(3): local=6, returns 6 + sumWithLocal(2)
    // sumWithLocal(2): local=4, returns 4 + sumWithLocal(1)
    // sumWithLocal(1): local=2, returns 2 + sumWithLocal(0)
    // sumWithLocal(0): returns 0
    // Total: 6 + 4 + 2 + 0 = 12
    data_->Clear();
    console_.Execute("sumWithLocal(3)");
    ASSERT_EQ(data_->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(data_->Top()), 12);
}