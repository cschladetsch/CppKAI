#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "KAI/Core/Console.h"
#include "KAI/Core/BuiltinTypes/Array.h"
#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Executor/Continuation.h"

#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * PI LANGUAGE BINARY OPERATION TESTS
 * ---------------------------------
 * These tests specifically focus on binary operations in the Pi language
 * to ensure they return the correct primitive types after our fixes.
 * 
 * The tests use direct Pi code execution rather than translation from Rho
 * to isolate the Pi execution engine's handling of binary operations.
 */

struct PiBinaryOpTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
        
        // Ensure we have the basic types registered
        if (!reg_->GetClass(TypeNumber::Int).Exists()) {
            reg_->AddClass<int>(Label("int"));
        }
        if (!reg_->GetClass(TypeNumber::Float).Exists()) {
            reg_->AddClass<float>(Label("float"));
        }
        if (!reg_->GetClass(TypeNumber::Bool).Exists()) {
            reg_->AddClass<bool>(Label("bool"));
        }
        if (!reg_->GetClass(TypeNumber::String).Exists()) {
            reg_->AddClass<String>(Label("String"));
        }
        
        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }
    
    // Helper method to verify basic stack properties after an operation
    void VerifyStackOperation(int expectedSize, const std::string& expectedType) {
        ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), expectedSize) << "Stack should have " << expectedSize << " item(s)";
        
        // Print type info for debugging
        std::cout << "Result type: " << data_->Top().GetClass()->GetName().ToString() << std::endl;
        ASSERT_EQ(data_->Top().GetClass()->GetName().ToString(), expectedType) 
                << "Result type should be " << expectedType;
    }
};

// Test 1: Basic integer addition
TEST_F(PiBinaryOpTests, IntegerAddition) {
    // Execute Pi code: 2 3 +
    console_.Execute("2 3 +");
    
    // Verify result type and value
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5) << "2 + 3 should equal 5";
}

// Test 2: String concatenation
TEST_F(PiBinaryOpTests, StringConcatenation) {
    // Execute Pi code: "Hello, " "World!" +
    console_.Execute("\"Hello, \" \"World!\" +");
    
    // Verify result type and value
    VerifyStackOperation(1, "String");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "Hello, World!") 
             << "String concatenation should work";
}

// Test 3: Mixed type operations (int and float)
TEST_F(PiBinaryOpTests, MixedTypeAddition) {
    // Execute Pi code: 5 2.5 +
    console_.Execute("5 2.5 +");
    
    // Verify result type and value
    VerifyStackOperation(1, "float");
    ASSERT_NEAR(ConstDeref<float>(data_->Top()), 7.5f, 0.0001f) 
              << "5 + 2.5 should equal 7.5";
}

// Test 4: Comparison operations
TEST_F(PiBinaryOpTests, ComparisonOperations) {
    // Execute Pi code: 5 3 >
    console_.Execute("5 3 >");
    
    // Verify result type and value
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "5 > 3 should be true";
    
    // Test less than
    data_->Clear();
    console_.Execute("3 5 <");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "3 < 5 should be true";
    
    // Test equality
    data_->Clear();
    console_.Execute("5 5 ==");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "5 == 5 should be true";
    
    // Test inequality
    data_->Clear();
    console_.Execute("5 6 !=");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "5 != 6 should be true";
}

// Test 5: Logical operations
TEST_F(PiBinaryOpTests, LogicalOperations) {
    // Test logical AND
    console_.Execute("true true and");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "true AND true should be true";
    
    // Test logical OR
    data_->Clear();
    console_.Execute("false true or");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "false OR true should be true";
    
    // Test logical XOR with different results
    data_->Clear();
    console_.Execute("true true xor");
    
    VerifyStackOperation(1, "bool");
    ASSERT_FALSE(ConstDeref<bool>(data_->Top())) << "true XOR true should be false";
    
    data_->Clear();
    console_.Execute("true false xor");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "true XOR false should be true";
}

// Test 6: Division and modulo
TEST_F(PiBinaryOpTests, DivisionAndModulo) {
    // Test division
    console_.Execute("10 2 div");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5) << "10 / 2 should equal 5";
    
    // Test modulo
    data_->Clear();
    console_.Execute("10 3 mod");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1) << "10 % 3 should equal 1";
}

// Test 7: Complex expressions with multiple operations
TEST_F(PiBinaryOpTests, ComplexExpressions) {
    // Test: (2 + 3) * 4
    console_.Execute("2 3 + 4 *");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 20) << "(2 + 3) * 4 should equal 20";
    
    // Test: 10 / 2 + 3 * 4
    data_->Clear();
    console_.Execute("10 2 div 3 4 * +");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 17) << "10 / 2 + 3 * 4 should equal 17";
    
    // Test with mixed types: 5.5 + 2 * 3
    data_->Clear();
    console_.Execute("5.5 2 3 * +");
    
    VerifyStackOperation(1, "float");
    ASSERT_NEAR(ConstDeref<float>(data_->Top()), 11.5f, 0.0001f) 
              << "5.5 + 2 * 3 should equal 11.5";
}

// Test 8: Stack operations preserving types
TEST_F(PiBinaryOpTests, StackOperationsWithTypes) {
    // Test: dup with integers
    console_.Execute("42 dup");
    
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after dup";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "Top of stack should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should also be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 42) << "Top value should be 42";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 42) << "Second value should also be 42";
    
    // Test: swap preserving types
    data_->Clear();
    console_.Execute("10 20 swap");
    
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after swap";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "Top of stack should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should also be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 10) << "Top value should be 10 after swap";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 20) << "Second value should be 20 after swap";
    
    // Test: drop
    console_.Execute("drop");
    ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after drop";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "Remaining item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 10) << "Remaining value should be 10";
}

// Test 9: Boolean expressions with multiple operations
TEST_F(PiBinaryOpTests, ComplexBooleanExpressions) {
    // Test: (5 > 3) and (10 == 10)
    console_.Execute("5 3 > 10 10 == and");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "(5 > 3) AND (10 == 10) should be true";
    
    // Test: (5 < 3) or (4 != 7)
    data_->Clear();
    console_.Execute("5 3 < 4 7 != or");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "(5 < 3) OR (4 != 7) should be true";
    
    // Test: not (5 < 3)
    data_->Clear();
    console_.Execute("5 3 < not");
    
    VerifyStackOperation(1, "bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "NOT (5 < 3) should be true";
}

// Test 10: Advanced Pi language features
TEST_F(PiBinaryOpTests, AdvancedTypePreservation) {
    // Define a variable and use it in binary operations
    console_.Execute("5 :a"); // Store 5 in variable a
    console_.Execute("a 3 +");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 8) << "a + 3 should equal 8";
    
    // Test auto-conversion between numeric types
    data_->Clear();
    console_.Execute("10 2.5 * a 1.5 + /");
    
    VerifyStackOperation(1, "float");
    ASSERT_NEAR(ConstDeref<float>(data_->Top()), 4.0f, 0.0001f) 
              << "(10 * 2.5) / (a + 1.5) should equal 4.0";
    
    // Test with string operations
    data_->Clear();
    console_.Execute("\"Value: \" a tostring +");
    
    VerifyStackOperation(1, "String");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "Value: 5") 
             << "String concatenation with converted int should work";
}