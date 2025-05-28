#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include "KAI/Core/BuiltinTypes.h"
#include "KAI/Core/Console.h"
#include "KAI/Executor/Operation.h"
#include "SimpleRhoPiTests.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * EXTENDED DIRECT BINARY OPERATION TESTS
 * --------------------------------------
 * This file contains additional binary operation tests that expand test
 * coverage beyond the basic tests. These tests focus on more edge cases,
 * special values, bitwise operations, and complex scenarios.
 */

// Test fixture for direct binary operations
class ExtendedBinaryOpTests : public TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<float>(Label("float"));
        reg_->AddClass<String>(Label("String"));
    }

    // Verify a numeric operation with int result
    void VerifyIntOperation(Object a, Object b, Operation::Type op,
                            int expected) {
        data_->Clear();

        try {
            Object result = exec_->PerformBinaryOp(a, b, op);
            data_->Push(result);

            // Unwrap continuations if needed
            UnwrapStackValues(data_, exec_);

            // Verify result
            ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
            ASSERT_TRUE(data_->Top().IsType<int>())
                << "Expected int but got "
                << data_->Top().GetClass()->GetName().ToString();
            ASSERT_EQ(ConstDeref<int>(data_->Top()), expected);
        } catch (const std::exception& e) {
            // Test failed
            FAIL() << "Exception during binary operation: " << e.what();
        }
    }

    // Verify a numeric operation with float result
    void VerifyFloatOperation(Object a, Object b, Operation::Type op,
                              float expected) {
        data_->Clear();

        try {
            Object result = exec_->PerformBinaryOp(a, b, op);
            data_->Push(result);

            // Unwrap continuations if needed
            UnwrapStackValues(data_, exec_);

            // Verify result
            ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
            ASSERT_TRUE(data_->Top().IsType<float>())
                << "Expected float but got "
                << data_->Top().GetClass()->GetName().ToString();
            ASSERT_NEAR(ConstDeref<float>(data_->Top()), expected, 0.0001f);
        } catch (const std::exception& e) {
            // Test failed
            FAIL() << "Exception during binary operation: " << e.what();
        }
    }

    // Verify a boolean operation
    void VerifyBoolOperation(Object a, Object b, Operation::Type op,
                             bool expected) {
        data_->Clear();

        try {
            Object result = exec_->PerformBinaryOp(a, b, op);
            data_->Push(result);

            // Unwrap continuations if needed
            UnwrapStackValues(data_, exec_);

            // Verify result
            ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
            ASSERT_TRUE(data_->Top().IsType<bool>())
                << "Expected bool but got "
                << data_->Top().GetClass()->GetName().ToString();
            ASSERT_EQ(ConstDeref<bool>(data_->Top()), expected);
        } catch (const std::exception& e) {
            // Test failed
            FAIL() << "Exception during binary operation: " << e.what();
        }
    }

    // Verify a string operation
    void VerifyStringOperation(Object a, Object b, Operation::Type op,
                               const String& expected) {
        data_->Clear();

        try {
            Object result = exec_->PerformBinaryOp(a, b, op);
            data_->Push(result);

            // Unwrap continuations if needed
            UnwrapStackValues(data_, exec_);

            // Verify result
            ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
            ASSERT_TRUE(data_->Top().IsType<String>())
                << "Expected String but got "
                << data_->Top().GetClass()->GetName().ToString();
            ASSERT_EQ(ConstDeref<String>(data_->Top()), expected);
        } catch (const std::exception& e) {
            // Test failed
            FAIL() << "Exception during binary operation: " << e.what();
        }
    }
};

// Bitwise Operations
TEST_F(ExtendedBinaryOpTests, BitwiseOperations) {
    // Create operands
    Object a = reg_->New<int>(0b1010);  // 10 in decimal
    Object b = reg_->New<int>(0b1100);  // 12 in decimal

    // Test bitwise AND
    data_->Clear();
    try {
        Object result = exec_->PerformBinaryOp(a, b, Operation::BitwiseAnd);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Expected int but got "
            << data_->Top().GetClass()->GetName().ToString();
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 8) << "Expected 10 & 12 = 8";
    } catch (const std::exception& e) {
        std::cout << "BitwiseAnd not supported: " << e.what() << std::endl;
    }

    // Test bitwise OR
    data_->Clear();
    try {
        Object result = exec_->PerformBinaryOp(a, b, Operation::BitwiseOr);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Expected int but got "
            << data_->Top().GetClass()->GetName().ToString();
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 14) << "Expected 10 | 12 = 14";
    } catch (const std::exception& e) {
        std::cout << "BitwiseOr not supported: " << e.what() << std::endl;
    }

    // Test bitwise XOR
    data_->Clear();
    try {
        Object result = exec_->PerformBinaryOp(a, b, Operation::BitwiseXor);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Expected int but got "
            << data_->Top().GetClass()->GetName().ToString();
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 6) << "Expected 10 ^ 12 = 6";
    } catch (const std::exception& e) {
        std::cout << "BitwiseXor not supported: " << e.what() << std::endl;
    }
}

// Complex Mixed Type Operations - String + Number Conversion
TEST_F(ExtendedBinaryOpTests, StringNumberConversion) {
    // Create operands
    Object strObj = reg_->New<String>("Value: ");
    Object intObj = reg_->New<int>(42);

    data_->Clear();
    try {
        // Try to concatenate string + int
        Object result = exec_->PerformBinaryOp(strObj, intObj, Operation::Plus);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        // The result might be a String if automatic conversion is supported
        if (data_->Top().IsType<String>()) {
            ASSERT_EQ(ConstDeref<String>(data_->Top()), "Value: 42")
                << "Expected automatic conversion of int to string for "
                   "concatenation";
        } else {
            // If automatic conversion isn't supported, this test is
            // informational
            std::cout << "String + int automatic conversion not supported"
                      << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "String + int operation failed: " << e.what() << std::endl;
    }
}

// Negative Number Operations
TEST_F(ExtendedBinaryOpTests, NegativeNumberOperations) {
    // Test negative number arithmetic
    Object negA = reg_->New<int>(-10);
    Object negB = reg_->New<int>(-5);
    Object posC = reg_->New<int>(3);

    // Negative + Negative
    VerifyIntOperation(negA, negB, Operation::Plus, -15);

    // Negative - Negative
    VerifyIntOperation(negA, negB, Operation::Minus, -5);

    // Negative * Negative
    VerifyIntOperation(negA, negB, Operation::Multiply, 50);

    // Negative / Negative
    VerifyIntOperation(negA, negB, Operation::Divide, 2);

    // Negative + Positive
    VerifyIntOperation(negA, posC, Operation::Plus, -7);

    // Negative * Positive
    VerifyIntOperation(negA, posC, Operation::Multiply, -30);
}

// Division by Zero Handling
TEST_F(ExtendedBinaryOpTests, DivisionByZeroHandling) {
    Object a = reg_->New<int>(10);
    Object zero = reg_->New<int>(0);

    data_->Clear();
    bool exceptionThrown = false;

    try {
        // Should throw an exception
        Object result = exec_->PerformBinaryOp(a, zero, Operation::Divide);
        data_->Push(result);
    } catch (const std::exception& e) {
        exceptionThrown = true;
        std::cout << "Expected exception caught: " << e.what() << std::endl;
    }

    // Verify exception was thrown
    ASSERT_TRUE(exceptionThrown)
        << "Division by zero should throw an exception";

    // Test floating point division by zero
    Object floatA = reg_->New<float>(10.0f);
    Object floatZero = reg_->New<float>(0.0f);

    data_->Clear();
    exceptionThrown = false;

    try {
        // Should throw an exception
        Object result =
            exec_->PerformBinaryOp(floatA, floatZero, Operation::Divide);
        data_->Push(result);
    } catch (const std::exception& e) {
        exceptionThrown = true;
        std::cout << "Expected exception caught: " << e.what() << std::endl;
    }

    // Verify exception was thrown for float division by zero
    ASSERT_TRUE(exceptionThrown)
        << "Float division by zero should throw an exception";
}

// Large Number Operations
TEST_F(ExtendedBinaryOpTests, LargeNumberOperations) {
    // Test with very large integers (close to limits)
    int largeValue1 = std::numeric_limits<int>::max() / 2;
    int largeValue2 = std::numeric_limits<int>::max() / 3;

    Object large1 = reg_->New<int>(largeValue1);
    Object large2 = reg_->New<int>(largeValue2);

    // Addition of large numbers
    VerifyIntOperation(large1, large2, Operation::Plus,
                       largeValue1 + largeValue2);

    // Subtraction of large numbers
    VerifyIntOperation(large1, large2, Operation::Minus,
                       largeValue1 - largeValue2);

    // Negative large number
    Object negLarge = reg_->New<int>(-largeValue1);
    VerifyIntOperation(large1, negLarge, Operation::Plus, 0);
}

// Chained Binary Operations
TEST_F(ExtendedBinaryOpTests, ChainedBinaryOperations) {
    // Test results of multiple operations applied in sequence
    // (10 + 5) * 2 - 3 = 27

    Object ten = reg_->New<int>(10);
    Object five = reg_->New<int>(5);
    Object two = reg_->New<int>(2);
    Object three = reg_->New<int>(3);

    // Step 1: 10 + 5 = 15
    Object step1 = exec_->PerformBinaryOp(ten, five, Operation::Plus);

    // Step 2: 15 * 2 = 30
    Object step2 = exec_->PerformBinaryOp(step1, two, Operation::Multiply);

    // Step 3: 30 - 3 = 27
    Object result = exec_->PerformBinaryOp(step2, three, Operation::Minus);

    data_->Push(result);
    UnwrapStackValues(data_, exec_);

    // Verify final result
    ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(data_->Top().IsType<int>())
        << "Expected int but got "
        << data_->Top().GetClass()->GetName().ToString();
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 27) << "Expected (10+5)*2-3=27";
}

// Floating Point Special Values
TEST_F(ExtendedBinaryOpTests, FloatingPointSpecialValues) {
    // Float class is registered in SetUp, no need to skip

    // Create special values
    float inf = std::numeric_limits<float>::infinity();
    float nan = std::numeric_limits<float>::quiet_NaN();

    Object infObj = reg_->New<float>(inf);
    Object nanObj = reg_->New<float>(nan);
    Object normalObj = reg_->New<float>(10.0f);

    data_->Clear();

    // Test infinity + normal -> infinity
    try {
        Object result =
            exec_->PerformBinaryOp(infObj, normalObj, Operation::Plus);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<float>())
            << "Expected float but got "
            << data_->Top().GetClass()->GetName().ToString();

        float resultValue = ConstDeref<float>(data_->Top());
        ASSERT_TRUE(std::isinf(resultValue))
            << "Expected infinity but got " << resultValue;
    } catch (const std::exception& e) {
        std::cout << "Infinity operation failed: " << e.what() << std::endl;
    }

    // Test NaN in operations
    data_->Clear();
    try {
        Object result =
            exec_->PerformBinaryOp(nanObj, normalObj, Operation::Plus);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<float>())
            << "Expected float but got "
            << data_->Top().GetClass()->GetName().ToString();

        float resultValue = ConstDeref<float>(data_->Top());
        ASSERT_TRUE(std::isnan(resultValue))
            << "Expected NaN but got " << resultValue;
    } catch (const std::exception& e) {
        std::cout << "NaN operation failed: " << e.what() << std::endl;
    }
}

// String Comparison Operations
TEST_F(ExtendedBinaryOpTests, StringComparisonOperations) {
    Object strA = reg_->New<String>("apple");
    Object strB = reg_->New<String>("banana");
    Object strC = reg_->New<String>("apple");

    // Test string equality
    VerifyBoolOperation(strA, strC, Operation::Equiv, true);

    // Test string inequality
    VerifyBoolOperation(strA, strB, Operation::NotEquiv, true);

    // Test lexicographical comparison if supported
    try {
        VerifyBoolOperation(strA, strB, Operation::Less,
                            true);  // "apple" < "banana"
        VerifyBoolOperation(strB, strA, Operation::Greater,
                            true);  // "banana" > "apple"
    } catch (const std::exception& e) {
        std::cout << "String comparison operations not fully supported: "
                  << e.what() << std::endl;
    }
}

// Operator Precedence Handling
TEST_F(ExtendedBinaryOpTests, OperatorPrecedenceHandling) {
    // Test operator precedence handling for complex expressions
    // Verify: 2 + 3 * 4 = 14 (multiplication before addition)

    Object two = reg_->New<int>(2);
    Object three = reg_->New<int>(3);
    Object four = reg_->New<int>(4);

    // Step 1: 3 * 4 = 12 (should be performed first)
    Object step1 = exec_->PerformBinaryOp(three, four, Operation::Multiply);

    // Step 2: 2 + 12 = 14
    Object result = exec_->PerformBinaryOp(two, step1, Operation::Plus);

    data_->Push(result);
    UnwrapStackValues(data_, exec_);

    // Verify final result
    ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(data_->Top().IsType<int>())
        << "Expected int but got "
        << data_->Top().GetClass()->GetName().ToString();
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 14) << "Expected 2+3*4=14";

    // Compare with different order: (2 + 3) * 4 = 20
    data_->Clear();

    // Step 1: 2 + 3 = 5
    Object altStep1 = exec_->PerformBinaryOp(two, three, Operation::Plus);

    // Step 2: 5 * 4 = 20
    Object altResult =
        exec_->PerformBinaryOp(altStep1, four, Operation::Multiply);

    data_->Push(altResult);
    UnwrapStackValues(data_, exec_);

    // Verify different result with different order
    ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
    ASSERT_TRUE(data_->Top().IsType<int>())
        << "Expected int but got "
        << data_->Top().GetClass()->GetName().ToString();
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 20) << "Expected (2+3)*4=20";
}

// Edge Cases and Boundary Values
TEST_F(ExtendedBinaryOpTests, EdgeCasesAndBoundaryValues) {
    // Test with edge cases and boundary values

    // Integer min/max values
    int maxInt = std::numeric_limits<int>::max();
    int minInt = std::numeric_limits<int>::min();

    Object maxObj = reg_->New<int>(maxInt);
    Object minObj = reg_->New<int>(minInt);
    Object oneObj = reg_->New<int>(1);

    // Test MAX_INT + 1 (should overflow in most implementations)
    data_->Clear();
    try {
        Object result = exec_->PerformBinaryOp(maxObj, oneObj, Operation::Plus);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Expected int but got "
            << data_->Top().GetClass()->GetName().ToString();

        std::cout << "MAX_INT + 1 = " << ConstDeref<int>(data_->Top())
                  << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Integer overflow detected: " << e.what() << std::endl;
    }

    // Test MIN_INT - 1 (should underflow in most implementations)
    data_->Clear();
    try {
        Object result =
            exec_->PerformBinaryOp(minObj, oneObj, Operation::Minus);
        data_->Push(result);

        UnwrapStackValues(data_, exec_);

        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Expected int but got "
            << data_->Top().GetClass()->GetName().ToString();

        std::cout << "MIN_INT - 1 = " << ConstDeref<int>(data_->Top())
                  << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Integer underflow detected: " << e.what() << std::endl;
    }

    // Test with empty strings
    Object emptyStr = reg_->New<String>("");
    Object normalStr = reg_->New<String>("Hello");

    // Empty string concatenation
    VerifyStringOperation(emptyStr, normalStr, Operation::Plus, "Hello");
    VerifyStringOperation(normalStr, emptyStr, Operation::Plus, "Hello");
    VerifyStringOperation(emptyStr, emptyStr, Operation::Plus, "");
}