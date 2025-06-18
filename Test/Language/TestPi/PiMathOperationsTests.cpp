#include <cmath>

#include "TestLangCommon.h"

struct PiMathOperationsTest : kai::TestLangCommon {};

TEST_F(PiMathOperationsTest, AAA_SimplestTest) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("5 3 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);

    stack->Clear();

    console_.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

// Test suite for Pi mathematical operations
// TODO: sin, cos operations not implemented in Pi language yet
// TEST_F(PiMathOperationsTest, TrigonometricFunctions) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//
//     // Test sin
//     console_.Execute("0.0 sin");
//     auto stack = exec->GetDataStack();
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 0.0f);
//
//     stack->Clear();
//
//     // Test cos
//     console_.Execute("0.0 cos");
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 1.0f);
// }

// TODO: pow, sqrt operations not implemented in Pi language yet
// TEST_F(PiMathOperationsTest, PowerAndSqrt) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//
//     // Test power
//     console_.Execute("2.0 3.0 pow");
//     auto stack = exec->GetDataStack();
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 8.0f);
//
//     stack->Clear();
//
//     // Test sqrt
//     console_.Execute("9.0 sqrt");
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.0f);
// }

// Test fixture for Pi min/max standalone tests
struct PiMinMaxStandaloneTests : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test 1: Min with negative numbers
TEST_F(PiMinMaxStandaloneTests, MinWithNegativeNumbers) {
    // Test min with negative numbers (using 0 - 5 to create -5)
    console_.Execute("0 5 - 3 min");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -5);
}

// Test 2: Max with negative numbers
TEST_F(PiMinMaxStandaloneTests, MaxWithNegativeNumbers) {
    console_.Execute("0 5 - 3 max");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

// Test 3: Min with floats
TEST_F(PiMinMaxStandaloneTests, MinWithFloats) {
    auto exec = console_.GetExecutor();

    console_.Execute("5.5 3.2 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);
}

// Test 4: Max with floats
TEST_F(PiMinMaxStandaloneTests, MaxWithFloats) {
    auto exec = console_.GetExecutor();

    console_.Execute("5.5 3.2 max");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 5.5f);
}

// Test 5: Chained min operations
TEST_F(PiMinMaxStandaloneTests, ChainedMinOperations) {
    auto exec = console_.GetExecutor();

    console_.Execute("10 20 30 min min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);
}

TEST_F(PiMathOperationsTest, NewMinMaxTest) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test min with integers
    console_.Execute("5 3 min");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);

    stack->Clear();

    // Test max with integers
    console_.Execute("5 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

// TODO: abs operation not implemented in Pi language yet
// TEST_F(PiMathOperationsTest, AbsoluteValue) {
//     console_.SetLanguage(kai::Language::Pi);
//     auto exec = console_.GetExecutor();
//
//     // Test abs with negative
//     console_.Execute("-5 abs");
//     auto stack = exec->GetDataStack();
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
//
//     stack->Clear();
//
//     // Test abs with positive
//     console_.Execute("5 abs");
//     ASSERT_EQ(stack->Size(), 1);
//     EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
// }

TEST_F(PiMathOperationsTest, TestBasicArithmetic) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test addition
    console_.Execute("5 3 +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);

    stack->Clear();

    // Test subtraction
    console_.Execute("5 3 -");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);

    stack->Clear();

    // Test multiplication
    console_.Execute("5 3 *");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);

    stack->Clear();

    // Test division
    console_.Execute("15 3 /");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);

    stack->Clear();

    // Test modulo
    console_.Execute("17 5 %");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

// Test fixture for Pi arithmetic standalone tests
struct PiArithmeticStandaloneTests : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test basic arithmetic operations
TEST_F(PiArithmeticStandaloneTests, Addition) {
    console_.Execute("5 3 +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
}

TEST_F(PiArithmeticStandaloneTests, Subtraction) {
    console_.Execute("5 3 -");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST_F(PiArithmeticStandaloneTests, Multiplication) {
    console_.Execute("5 3 *");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

TEST_F(PiArithmeticStandaloneTests, Division) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("15 3 /");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiArithmeticStandaloneTests, Modulo) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("17 5 %");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}