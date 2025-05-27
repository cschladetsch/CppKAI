#include <cmath>

#include "TestLangCommon.h"

struct PiMathOperationsTest : kai::TestLangCommon {};

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

TEST_F(PiMathOperationsTest, MinMax) {
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

    stack->Clear();

    // Test min with negative numbers (using 0 - 5 to create -5)
    console_.Execute("0 5 - 3 min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -5);

    stack->Clear();

    // Test max with negative numbers
    console_.Execute("0 5 - 3 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);

    stack->Clear();

    // Test min with floats
    console_.Execute("5.5 3.2 min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);

    stack->Clear();

    // Test max with floats
    console_.Execute("5.5 3.2 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 5.5f);

    stack->Clear();

    // Test min with mixed types (int and float)
    console_.Execute("5 3.2 min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 3.2f);

    stack->Clear();

    // Test max with mixed types (float and int)
    console_.Execute("3.2 5 max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 5.0f);

    stack->Clear();

    // Test chained min operations
    console_.Execute("10 20 30 min min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);

    stack->Clear();

    // Test chained max operations
    console_.Execute("10 20 30 max max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
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

TEST_F(PiMathOperationsTest, BasicArithmetic) {
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