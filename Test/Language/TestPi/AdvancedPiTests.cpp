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
 * ADVANCED PI LANGUAGE TESTS
 * -------------------------
 * This test suite adds 20 additional test cases for the Pi language,
 * focusing on more advanced features and edge cases that complement
 * the existing test coverage.
 */

struct AdvancedPiTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);

        // Register basic types needed for tests
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<float>(Label("float"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
        reg_->AddClass<Array>(Label("Array"));

        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Helper method to execute Pi code and verify the result
    template <typename T>
    void ExecutePiCodeAndVerify(const std::string& code, const T& expected) {
        // Clear the stack first
        exec_->ClearStacks();

        // Execute the Pi code
        console_.Execute(code);

        // Process the stack to extract values from continuations
        UnwrapStackValues();

        // Verify the result
        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after operation";
        ASSERT_TRUE(data_->Top().IsType<T>()) 
            << "Expected result type " << typeid(T).name() 
            << " but got " << (data_->Top().Exists() 
                             ? data_->Top().GetClass()->GetName().ToString() 
                             : "null");
        ASSERT_EQ(ConstDeref<T>(data_->Top()), expected) 
            << "Expected value " << expected << " but got " << ConstDeref<T>(data_->Top());
    }

    // Helper to check stack size after execution
    void ExecuteAndCheckStackSize(const std::string& code, size_t expectedSize) {
        // Clear the stack first
        exec_->ClearStacks();

        // Execute the Pi code
        console_.Execute(code);

        // Verify the stack size
        ASSERT_EQ(data_->Size(), expectedSize) 
            << "Expected stack size " << expectedSize 
            << " but got " << data_->Size() << " after executing: " << code;
    }

    // Helper for array operations
    void CreateArrayWithValues(const std::vector<int>& values) {
        // Clear the stack first
        exec_->ClearStacks();

        // Create a new array
        Object array = reg_->New<Array>();
        Pointer<Array> arr = array;

        // Add values to the array
        for (int val : values) {
            Object numObj = reg_->New<int>(val);
            arr->Append(numObj);  // Use Append instead of Add
        }

        // Push the array onto the stack
        data_->Push(array);
    }
};

// 1. Test handling of multiple stack operations in sequence
TEST_F(AdvancedPiTests, MultipleStackOperations) {
    ExecutePiCodeAndVerify<int>("5 10 swap drop 3 +", 8);
}

// 2. Test nested arithmetic expressions
TEST_F(AdvancedPiTests, NestedArithmetic) {
    ExecutePiCodeAndVerify<int>("2 3 + 4 * 5 -", 15);
}

// 3. Test bitwise operations
TEST_F(AdvancedPiTests, BitwiseOperations) {
    // Push two integers onto the stack
    Object val1 = reg_->New<int>(5);  // 101 in binary
    Object val2 = reg_->New<int>(3);  // 011 in binary
    data_->Push(val1);
    data_->Push(val2);

    // Perform bitwise AND
    exec_->Perform(Operation::BitwiseAnd);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1);  // 101 & 011 = 001

    // Clear and test bitwise OR
    exec_->ClearStacks();
    data_->Push(val1);
    data_->Push(val2);
    exec_->Perform(Operation::BitwiseOr);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 7);  // 101 | 011 = 111

    // Clear and test bitwise XOR
    exec_->ClearStacks();
    data_->Push(val1);
    data_->Push(val2);
    exec_->Perform(Operation::BitwiseXor);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 6);  // 101 ^ 011 = 110
}

// 4. Test logical operations with non-boolean values
TEST_F(AdvancedPiTests, LogicalOperationsWithNonBooleans) {
    // In Pi, non-zero values are treated as true for logical operations
    Object val1 = reg_->New<int>(1);
    Object val2 = reg_->New<int>(0);
    
    // Test logical AND
    data_->Push(val1);
    data_->Push(val1);
    exec_->Perform(Operation::LogicalAnd);
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
    
    // Test with one false value
    exec_->ClearStacks();
    data_->Push(val1);
    data_->Push(val2);
    exec_->Perform(Operation::LogicalAnd);
    ASSERT_FALSE(ConstDeref<bool>(data_->Top()));
    
    // Test logical OR
    exec_->ClearStacks();
    data_->Push(val2);
    data_->Push(val1);
    exec_->Perform(Operation::LogicalOr);
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
    
    // Test with both false
    exec_->ClearStacks();
    data_->Push(val2);
    data_->Push(val2);
    exec_->Perform(Operation::LogicalOr);
    ASSERT_FALSE(ConstDeref<bool>(data_->Top()));
}

// 5. Test floating-point operations precision
TEST_F(AdvancedPiTests, FloatingPointPrecision) {
    // Create float objects
    Object f1 = reg_->New<float>(3.14159f);
    Object f2 = reg_->New<float>(2.71828f);
    
    // Test addition
    data_->Push(f1);
    data_->Push(f2);
    exec_->Perform(Operation::Plus);
    
    // Test with small epsilon for floating point comparison
    float result = ConstDeref<float>(data_->Top());
    float expected = 3.14159f + 2.71828f;
    float epsilon = 0.00001f;
    
    ASSERT_TRUE(std::abs(result - expected) < epsilon)
        << "Expected approximately " << expected << " but got " << result;
        
    // Test multiplication
    exec_->ClearStacks();
    data_->Push(f1);
    data_->Push(f2);
    exec_->Perform(Operation::Multiply);
    
    result = ConstDeref<float>(data_->Top());
    expected = 3.14159f * 2.71828f;
    
    ASSERT_TRUE(std::abs(result - expected) < epsilon)
        << "Expected approximately " << expected << " but got " << result;
}

// 6. Test rotation of large stack
TEST_F(AdvancedPiTests, LargeStackRotation) {
    // Push multiple values onto the stack
    for (int i = 1; i <= 5; i++) {
        Object val = reg_->New<int>(i);
        data_->Push(val);
    }
    // Stack now has: 1 2 3 4 5 (with 5 at the top)
    
    // Perform Rot operation (rotates top three items)
    exec_->Perform(Operation::Rot);
    
    // Stack should now be: 1 2 5 3 4
    // with 4 at the top (index 0)
    ASSERT_EQ(data_->Size(), 5);
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 4);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 5);
    ASSERT_EQ(ConstDeref<int>(data_->At(3)), 2);
    ASSERT_EQ(ConstDeref<int>(data_->At(4)), 1);
}

// 7. Test manual type conversion handling
TEST_F(AdvancedPiTests, TypeConversions) {
    // Create an integer object
    Object intObj = reg_->New<int>(42);
    data_->Push(intObj);
    
    // Check that the value exists
    ASSERT_TRUE(data_->Top().Exists());
    ASSERT_TRUE(data_->Top().IsType<int>());
    
    // Create a floating-point object
    exec_->ClearStacks();
    Object floatObj = reg_->New<float>(3.14f);
    data_->Push(floatObj);
    
    // Check that it exists
    ASSERT_TRUE(data_->Top().Exists());
    ASSERT_TRUE(data_->Top().IsType<float>());
}

// 8. Test string operations
TEST_F(AdvancedPiTests, StringOperations) {
    // Create string objects
    Object str1 = reg_->New<String>("Hello");
    Object str2 = reg_->New<String>(" World");
    
    // Test concatenation
    data_->Push(str1);
    data_->Push(str2);
    exec_->Perform(Operation::Plus);
    
    ASSERT_TRUE(data_->Top().IsType<String>());
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "Hello World");
    
    // Test string comparison
    exec_->ClearStacks();
    data_->Push(str1);
    data_->Push(str1);  // Same string
    exec_->Perform(Operation::Equiv);
    
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
    
    // Test string inequality
    exec_->ClearStacks();
    data_->Push(str1);
    data_->Push(str2);
    exec_->Perform(Operation::Equiv);
    
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_FALSE(ConstDeref<bool>(data_->Top()));
}

// 9. Test array construction and manipulation
TEST_F(AdvancedPiTests, ArrayOperations) {
    // Create array with values [1, 2, 3, 4, 5]
    CreateArrayWithValues({1, 2, 3, 4, 5});
    
    // Get the array object
    Pointer<Array> arr = data_->Top();
    
    // Verify array size
    ASSERT_EQ(arr->Size(), 5);
    
    // Verify array contents
    for (int i = 0; i < 5; i++) {
        ASSERT_TRUE(arr->At(i).IsType<int>());
        ASSERT_EQ(ConstDeref<int>(arr->At(i)), i + 1);
    }
    
    // Test array expansion (add element)
    Object newVal = reg_->New<int>(6);
    arr->Append(newVal);  // Use Append instead of Add
    
    ASSERT_EQ(arr->Size(), 6);
    ASSERT_EQ(ConstDeref<int>(arr->At(5)), 6);
}

// 10. Test complex conditionals with boolean operations
TEST_F(AdvancedPiTests, ComplexConditionals) {
    // Execute complex boolean expression: (5 > 3) && (10 != 5)
    ExecutePiCodeAndVerify<bool>("5 3 > 10 5 != &&", true);
    
    // Execute complex boolean expression: (5 < 3) || (10 == 10)
    ExecutePiCodeAndVerify<bool>("5 3 < 10 10 == ||", true);
    
    // Execute complex boolean expression: (5 > 3) && (10 == 5)
    ExecutePiCodeAndVerify<bool>("5 3 > 10 5 == &&", false);
}

// 11. Test stack depth check and manipulation
TEST_F(AdvancedPiTests, StackDepthCheck) {
    // Push multiple values
    for (int i = 1; i <= 3; i++) {
        Object val = reg_->New<int>(i);
        data_->Push(val);
    }
    
    // Check stack depth
    ASSERT_EQ(data_->Size(), 3);
    
    // Test clear stack operation
    exec_->ClearStacks();
    ASSERT_EQ(data_->Size(), 0);
    
    // Test depth after operations
    Object val = reg_->New<int>(42);
    data_->Push(val);
    exec_->Perform(Operation::Dup);
    ASSERT_EQ(data_->Size(), 2);
    exec_->Perform(Operation::Drop);
    ASSERT_EQ(data_->Size(), 1);
}

// 12. Test error handling for division by zero
TEST_F(AdvancedPiTests, DivisionByZero) {
    // Create integer objects
    Object num = reg_->New<int>(10);
    Object zero = reg_->New<int>(0);
    
    // Push onto stack
    data_->Push(num);
    data_->Push(zero);
    
    // Attempt division by zero
    try {
        exec_->Perform(Operation::Divide);
        // If we reach here without exception, test fails
        FAIL() << "Expected exception for division by zero";
    } catch (const std::exception& ex) {
        // Expected exception
        SUCCEED();
    } catch (...) {
        // Unknown exception type
        FAIL() << "Unexpected exception type";
    }
}

// 13. Test floating point comparison with epsilon
TEST_F(AdvancedPiTests, FloatComparisonWithEpsilon) {
    // Create float objects that are very close but not exactly equal
    float val1 = 0.1f + 0.2f;  // This is usually not exactly 0.3 due to floating point precision
    float val2 = 0.3f;
    
    Object f1 = reg_->New<float>(val1);
    Object f2 = reg_->New<float>(val2);
    
    // Direct comparison might fail due to floating point precision
    data_->Push(f1);
    data_->Push(f2);
    exec_->Perform(Operation::Equiv);
    
    // Result could be true or false depending on floating point representation
    // but we can check that the result is a boolean
    ASSERT_TRUE(data_->Top().IsType<bool>());
    
    // Test with epsilon comparison (abs(f1 - f2) < epsilon)
    exec_->ClearStacks();
    data_->Push(f1);
    data_->Push(f2);
    exec_->Perform(Operation::Minus);
    data_->Push(reg_->New<float>(std::abs(ConstDeref<float>(data_->Top()))));
    data_->Push(reg_->New<float>(0.000001f));  // Epsilon
    exec_->Perform(Operation::Less);
    
    ASSERT_TRUE(data_->Top().IsType<bool>());
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
}

// 14. Test register accessing and manipulation
TEST_F(AdvancedPiTests, RegisterHandling) {
    // In Pi, we can store values in registers with @ and retrieve them with $
    
    // Clear the stack and store a value in register 'a'
    exec_->ClearStacks();
    console_.Execute("42 'a @");  // Store 42 in register 'a'
    
    // Verify that the stack is empty after storing
    ASSERT_TRUE(data_->Empty());
    
    // Retrieve the value from register 'a'
    console_.Execute("'a $");
    
    // Verify the retrieved value
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 42);
    
    // Update the value in register 'a'
    console_.Execute("99 'a @");
    
    // Retrieve the updated value
    console_.Execute("'a $");
    
    // Verify the updated value
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 99);
}

// 15. Test nested stack operations with complex manipulations
TEST_F(AdvancedPiTests, NestedStackManipulations) {
    // Execute a complex sequence of stack manipulations
    ExecuteAndCheckStackSize("1 2 3 dup 5 swap drop rot", 4);
    
    // Stack should be: 1 5 2 3
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 2);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 5);
    ASSERT_EQ(ConstDeref<int>(data_->At(3)), 1);
}

// 16. Test power and exponential operations
TEST_F(AdvancedPiTests, PowerOperations) {
    // Test square function (implemented with dup and multiply)
    ExecutePiCodeAndVerify<int>("5 dup *", 25);
    
    // Test cube function (implemented with dup, dup, multiply, multiply)
    ExecutePiCodeAndVerify<int>("3 dup dup * *", 27);
    
    // Test more complex power sequence
    ExecutePiCodeAndVerify<int>("2 dup dup * dup *", 16);  // 2^4 = 16
}

// 17. Test stack manipulation with multiple operations
TEST_F(AdvancedPiTests, MultipleStackManipulations) {
    // Execute a sequence that creates a stack with specific values
    exec_->ClearStacks();
    
    // Push values 1, 2, 3
    data_->Push(reg_->New<int>(1));
    data_->Push(reg_->New<int>(2));
    data_->Push(reg_->New<int>(3));
    
    // Duplicate the top value (3)
    exec_->Perform(Operation::Dup);
    
    // Stack should now be: 1, 2, 3, 3
    ASSERT_EQ(data_->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 2);
    ASSERT_EQ(ConstDeref<int>(data_->At(3)), 1);
    
    // Swap the top two values
    exec_->Perform(Operation::Swap);
    
    // Stack should now be: 1, 2, 3, 3 (unchanged because we swapped two identical values)
    ASSERT_EQ(data_->Size(), 4);
    
    // Rotate the top three values
    exec_->Perform(Operation::Rot);
    
    // Stack should now be: 1, 3, 3, 2
    ASSERT_EQ(data_->Size(), 4);
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 2);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(3)), 1);
    
    // Drop the top value
    exec_->Perform(Operation::Drop);
    
    // Stack should now be: 1, 3, 3
    ASSERT_EQ(data_->Size(), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 3);
    ASSERT_EQ(ConstDeref<int>(data_->At(2)), 1);
}

// 18. Test mixed type operations (int and float)
TEST_F(AdvancedPiTests, MixedTypeOperations) {
    // Create objects of different types
    Object intObj = reg_->New<int>(5);
    Object floatObj = reg_->New<float>(2.5f);
    
    // Test addition of int and float
    data_->Push(intObj);
    data_->Push(floatObj);
    exec_->Perform(Operation::Plus);
    
    ASSERT_TRUE(data_->Top().IsType<float>());
    float result = ConstDeref<float>(data_->Top());
    float expected = 5 + 2.5f;
    float epsilon = 0.00001f;
    
    ASSERT_TRUE(std::abs(result - expected) < epsilon)
        << "Expected " << expected << " but got " << result;
        
    // Test multiplication of int and float
    exec_->ClearStacks();
    data_->Push(intObj);
    data_->Push(floatObj);
    exec_->Perform(Operation::Multiply);
    
    ASSERT_TRUE(data_->Top().IsType<float>());
    result = ConstDeref<float>(data_->Top());
    expected = 5 * 2.5f;
    
    ASSERT_TRUE(std::abs(result - expected) < epsilon)
        << "Expected " << expected << " but got " << result;
}

// 19. Test Pi code blocks and evaluation
TEST_F(AdvancedPiTests, CodeBlocksAndEvaluation) {
    // Define a code block for squaring a number
    console_.Execute("{ dup * } 'square @");
    
    // Use the code block
    console_.Execute("5 'square $ &");
    
    // Verify the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 25);
    
    // Try a different value with the same function
    exec_->ClearStacks();
    console_.Execute("7 'square $ &");
    
    // Verify the result
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 49);
}

// 20. Test stack operations for complex arithmetic
TEST_F(AdvancedPiTests, ComplexArithmetic) {
    // Test a sequence that computes (2^2 + 3^2) * 2
    console_.Execute("2 dup * 3 dup * + 2 *");
    
    // Verify the result: (2^2 + 3^2) * 2 = (4 + 9) * 2 = 13 * 2 = 26
    ASSERT_FALSE(data_->Empty());
    ASSERT_TRUE(data_->Top().IsType<int>());
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 26);
    
    // Test computing (a + b) * c using stack operations
    exec_->ClearStacks();
    data_->Push(reg_->New<int>(5));    // a = 5
    data_->Push(reg_->New<int>(7));    // b = 7
    exec_->Perform(Operation::Plus);   // a + b = 12
    data_->Push(reg_->New<int>(3));    // c = 3
    exec_->Perform(Operation::Multiply); // (a + b) * c = 36
    
    // Verify the result
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 36);
}