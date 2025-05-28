#include <gtest/gtest.h>
#include <KAI/Console/Console.h>
#include <KAI/Language/Common/Language.h>

using namespace kai;

// Test fixture that doesn't inherit from TestLangCommon
struct PiMathStandaloneTest : ::testing::Test {
    Console console;
    
    void SetUp() override {
        console.SetLanguage(Language::Pi);
    }
};

TEST_F(PiMathStandaloneTest, MinMaxOperations) {
    auto exec = console.GetExecutor();

    // Test min with integers
    console.Execute("5 3 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);

    stack->Clear();

    // Test max with integers
    console.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5);

    stack->Clear();

    // Test min with negative numbers (using 0 - 5 to create -5)
    console.Execute("0 5 - 3 min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), -5);

    stack->Clear();

    // Test max with negative numbers
    console.Execute("0 5 - 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 3);
}

TEST_F(PiMathStandaloneTest, BasicArithmetic) {
    auto exec = console.GetExecutor();

    // Test addition
    console.Execute("5 3 +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 8);

    stack->Clear();

    // Test subtraction
    console.Execute("5 3 -");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 2);

    stack->Clear();

    // Test multiplication
    console.Execute("5 3 *");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 15);

    stack->Clear();

    // Test division
    console.Execute("15 3 /");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 5);

    stack->Clear();

    // Test modulo
    console.Execute("17 5 %");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(ConstDeref<int>(stack->Top()), 2);
}