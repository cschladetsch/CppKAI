#include <KAI/Console/Console.h>
#include <KAI/Language/Common/Language.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

// Test fixture that inherits from TestLangCommon
struct PiMathStandaloneTest : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }
};

TEST_F(PiMathStandaloneTest, MinMaxOperations) {
    auto exec = console_.GetExecutor();

    // Test min with integers
    console_.Execute("5 3 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);

    stack->Clear();

    // Test max with integers
    console_.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5);

    stack->Clear();

    // Test min with negative numbers (using 0 - 5 to create -5)
    console_.Execute("0 5 - 3 min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), -5);

    stack->Clear();

    // Test max with negative numbers
    console_.Execute("0 5 - 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);
}

TEST_F(PiMathStandaloneTest, BasicArithmetic) {
    auto exec = console_.GetExecutor();

    // Test addition
    console_.Execute("5 3 +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 8);

    stack->Clear();

    // Test subtraction
    console_.Execute("5 3 -");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 2);

    stack->Clear();

    // Test multiplication
    console_.Execute("5 3 *");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 15);

    stack->Clear();

    // Test division
    console_.Execute("15 3 /");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5);

    stack->Clear();

    // Test modulo
    console_.Execute("17 5 %");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 2);
}