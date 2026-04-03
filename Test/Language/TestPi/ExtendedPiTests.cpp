#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Executor/Continuation.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * EXTENDED PI LANGUAGE TESTS
 * -------------------------
 * These tests extend the Pi language test suite with 20 additional test cases.
 * They cover various aspects of the Pi language including arithmetic
 * operations, stack manipulation, numeric conversions, comparison operations,
 * logical operations, and more complex stack operations.
 */

struct ExtendedPiTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);

        // Ensure we have the basic types registered
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
        reg_->AddClass<Array>(Label("Array"));

        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Helper method to execute Pi code directly on the stack using Perform
    // This bypasses the string execution that might be causing issues
    template <typename T>
    void DirectExecuteAndVerify(int value1, int value2, Operation::Type op,
                                const T& expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Push values onto the stack
        Object val1 = reg_->New<int>(value1);
        Object val2 = reg_->New<int>(value2);
        data_->Push(val1);
        data_->Push(val2);

        // Perform the operation
        exec_->Perform(op);

        // Basic stack verification
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item";

        // Get and verify result
        Object result = data_->Top();
        ASSERT_TRUE(result.Exists()) << "Result should exist";
        ASSERT_TRUE(result.IsType<T>())
            << "Expected result type " << typeid(T).name() << " but got "
            << (result.Exists() ? result.GetClass()->GetName().ToString()
                                : "null");

        T value = ConstDeref<T>(result);
        ASSERT_EQ(value, expected)
            << "Expected value " << expected << " but got " << value;
    }

    // Helper method to test stack operations
    void TestStackOperation(Operation::Type op, int expectedSize) {
        // Clear the stack first
        exec_->ClearStacks();

        // Push test values onto the stack
        for (int i = 1; i <= 3; i++) {
            Object val = reg_->New<int>(i);
            data_->Push(val);
        }

        // Perform the operation
        exec_->Perform(op);

        // Verify the stack size
        ASSERT_EQ(data_->Size(), expectedSize)
            << "Stack should have " << expectedSize << " items after operation";
    }

    // Helper for boolean operations
    void TestBooleanOperation(bool val1, bool val2, Operation::Type op,
                              bool expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Push boolean values onto the stack
        Object bval1 = reg_->New<bool>(val1);
        Object bval2 = reg_->New<bool>(val2);
        data_->Push(bval1);
        data_->Push(bval2);

        // Perform the operation
        exec_->Perform(op);

        // Verify result
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item";
        ASSERT_TRUE(data_->Top().IsType<bool>())
            << "Result should be a boolean";
        ASSERT_EQ(ConstDeref<bool>(data_->Top()), expected)
            << "Boolean operation result incorrect";
    }
};

// Basic integer addition
TEST_F(ExtendedPiTests, IntegerAddition) {
    DirectExecuteAndVerify<int>(5, 7, Operation::Plus, 12);
}

// Basic integer subtraction
TEST_F(ExtendedPiTests, IntegerSubtraction) {
    DirectExecuteAndVerify<int>(10, 3, Operation::Minus, 7);
}

// Basic integer multiplication
TEST_F(ExtendedPiTests, IntegerMultiplication) {
    DirectExecuteAndVerify<int>(6, 7, Operation::Multiply, 42);
}

// Basic integer division
TEST_F(ExtendedPiTests, IntegerDivision) {
    DirectExecuteAndVerify<int>(20, 4, Operation::Divide, 5);
}

// Basic modulo operation
TEST_F(ExtendedPiTests, ModuloOperation) {
    DirectExecuteAndVerify<int>(17, 5, Operation::Modulo, 2);
}

// Stack duplication
TEST_F(ExtendedPiTests, StackDuplication) {
    // Clear the stack first
    exec_->ClearStacks();

    // Push test value onto the stack
    Object val = reg_->New<int>(5);
    data_->Push(val);

    // Perform the Dup operation
    exec_->Perform(Operation::Dup);

    // Verify the stack
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after Dup";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 5) << "First value should be 5";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 5) << "Second value should be 5";
}

// Stack drop
TEST_F(ExtendedPiTests, StackDrop) {
    // Clear the stack first
    exec_->ClearStacks();

    // Push test values onto the stack
    Object val1 = reg_->New<int>(5);
    Object val2 = reg_->New<int>(10);
    data_->Push(val1);
    data_->Push(val2);

    // Perform the Drop operation
    exec_->Perform(Operation::Drop);

    // Verify the stack
    ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after Drop";
    ASSERT_TRUE(data_->Top().IsType<int>())
        << "Remaining item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5)
        << "Remaining value should be 5";
}

// Stack swap
TEST_F(ExtendedPiTests, StackSwap) {
    // Clear the stack first
    exec_->ClearStacks();

    // Push test values onto the stack
    Object val1 = reg_->New<int>(5);
    Object val2 = reg_->New<int>(10);
    data_->Push(val1);
    data_->Push(val2);

    // Perform the Swap operation
    exec_->Perform(Operation::Swap);

    // Verify the stack
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after Swap";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 5) << "First value should be 5";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 10) << "Second value should be 10";
}

// Stack over
TEST_F(ExtendedPiTests, StackOver) {
    // Clear the stack first
    exec_->ClearStacks();

    // Push test values onto the stack
    Object val1 = reg_->New<int>(5);
    Object val2 = reg_->New<int>(10);
    data_->Push(val1);
    data_->Push(val2);

    // Perform the Over operation
    exec_->Perform(Operation::Over);

    // Verify the stack
    ASSERT_EQ(data_->Size(), 3) << "Stack should have 3 items after Over";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should be an int";
    ASSERT_TRUE(data_->At(2).IsType<int>()) << "Third item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 5) << "First value should be 5";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 10) << "Second value should be 10";
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 5) << "Third value should be 5";
}

// Stack rotation
TEST_F(ExtendedPiTests, StackRotation) {
    // Clear the stack first
    exec_->ClearStacks();

    // Push test values onto the stack
    Object val1 = reg_->New<int>(1);
    Object val2 = reg_->New<int>(2);
    Object val3 = reg_->New<int>(3);
    data_->Push(val1);
    data_->Push(val2);
    data_->Push(val3);

    // Perform the Rot operation
    exec_->Perform(Operation::Rot);

    // Verify the stack
    ASSERT_EQ(data_->Size(), 3) << "Stack should have 3 items after Rot";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should be an int";
    ASSERT_TRUE(data_->At(2).IsType<int>()) << "Third item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 1) << "First value should be 1";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 3) << "Second value should be 3";
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 2) << "Third value should be 2";
}

// Equality comparison
TEST_F(ExtendedPiTests, EqualityComparison) {
    DirectExecuteAndVerify<bool>(5, 5, Operation::Equiv, true);
    DirectExecuteAndVerify<bool>(5, 6, Operation::Equiv, false);
}

// Inequality comparison
TEST_F(ExtendedPiTests, InequalityComparison) {
    DirectExecuteAndVerify<bool>(5, 6, Operation::NotEquiv, true);
    DirectExecuteAndVerify<bool>(5, 5, Operation::NotEquiv, false);
}

// Less than comparison
TEST_F(ExtendedPiTests, LessThanComparison) {
    DirectExecuteAndVerify<bool>(5, 10, Operation::Less, true);
    DirectExecuteAndVerify<bool>(10, 5, Operation::Less, false);
}

// Greater than comparison
TEST_F(ExtendedPiTests, GreaterThanComparison) {
    DirectExecuteAndVerify<bool>(10, 5, Operation::Greater, true);
    DirectExecuteAndVerify<bool>(5, 10, Operation::Greater, false);
}

// Less than or equal comparison
TEST_F(ExtendedPiTests, LessThanOrEqualComparison) {
    DirectExecuteAndVerify<bool>(5, 5, Operation::LessOrEquiv, true);
    DirectExecuteAndVerify<bool>(5, 10, Operation::LessOrEquiv, true);
    DirectExecuteAndVerify<bool>(10, 5, Operation::LessOrEquiv, false);
}

// Greater than or equal comparison
TEST_F(ExtendedPiTests, GreaterThanOrEqualComparison) {
    DirectExecuteAndVerify<bool>(5, 5, Operation::GreaterOrEquiv, true);
    DirectExecuteAndVerify<bool>(10, 5, Operation::GreaterOrEquiv, true);
    DirectExecuteAndVerify<bool>(5, 10, Operation::GreaterOrEquiv, false);
}

// Complex stack manipulation
TEST_F(ExtendedPiTests, ComplexStackManipulation) {
    // Clear the stack first
    exec_->ClearStacks();

    // Push test values onto the stack
    for (int i = 1; i <= 4; i++) {
        Object val = reg_->New<int>(i);
        data_->Push(val);
    }

    // Now our stack has: 1 2 3 4 (with 4 at the top)
    // In KAI's Stack implementation, At(0) refers to the top of the stack

    // Execute Drop: removes 4, leaving 1 2 3
    exec_->Perform(Operation::Drop);

    // Execute Swap: swaps 2 and 3, leaving 1 3 2
    exec_->Perform(Operation::Swap);

    // Execute Over: copies the second item to the top, leaving 1 3 2 3
    exec_->Perform(Operation::Over);

    // Verify the stack
    ASSERT_EQ(data_->Size(), 4)
        << "Stack should have 4 items after complex manipulation";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should be an int";
    ASSERT_TRUE(data_->At(2).IsType<int>()) << "Third item should be an int";
    ASSERT_TRUE(data_->At(3).IsType<int>()) << "Fourth item should be an int";

    // Stack after operations should be: 1 3 2 3
    // But in KAI's Stack implementation, At(0) refers to the top of the stack
    // (most recently pushed) So At(0) should be 3, At(1) should be 2, At(2)
    // should be 3, and At(3) should be 1
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 3) << "Top value should be 3";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 2) << "Second value should be 2";
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 3) << "Third value should be 3";
    ASSERT_EQ(ConstDeref<int>(data_->At(3)), 1) << "Bottom value should be 1";
}

// Boolean NOT operation
TEST_F(ExtendedPiTests, BooleanNotOperation) {
    // Clear the stack first
    exec_->ClearStacks();

    // Test with true
    Object trueVal = reg_->New<bool>(true);
    data_->Push(trueVal);
    exec_->Perform(Operation::LogicalNot);

    ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after operation";
    ASSERT_TRUE(data_->Top().IsType<bool>()) << "Result should be a boolean";
    ASSERT_FALSE(ConstDeref<bool>(data_->Top())) << "NOT true should be false";

    // Test with false
    exec_->ClearStacks();
    Object falseVal = reg_->New<bool>(false);
    data_->Push(falseVal);
    exec_->Perform(Operation::LogicalNot);

    ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after operation";
    ASSERT_TRUE(data_->Top().IsType<bool>()) << "Result should be a boolean";
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "NOT false should be true";
}

// Boolean AND operation
TEST_F(ExtendedPiTests, BooleanAndOperation) {
    TestBooleanOperation(true, true, Operation::LogicalAnd, true);
    TestBooleanOperation(true, false, Operation::LogicalAnd, false);
    TestBooleanOperation(false, false, Operation::LogicalAnd, false);
}

// Boolean OR operation
TEST_F(ExtendedPiTests, BooleanOrOperation) {
    TestBooleanOperation(true, true, Operation::LogicalOr, true);
    TestBooleanOperation(true, false, Operation::LogicalOr, true);
    TestBooleanOperation(false, false, Operation::LogicalOr, false);
}