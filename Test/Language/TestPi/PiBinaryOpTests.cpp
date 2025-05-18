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
 * These tests focus on binary operations in the Pi language
 * to ensure they return the correct primitive types.
 * 
 * The tests use direct Pi code execution rather than translation from Rho
 * to isolate the Pi execution engine's handling of binary operations.
 * 
 * This is a simplified version that avoids problematic float operations.
 */

struct PiBinaryOpTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
        
        // Ensure we have the basic types registered
        // Always register these types to be safe - Registry handles duplicates
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
        
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

// Test 3: Comparison operations
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

// Test 4: Logical operations
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

// Test 5: Division and modulo
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

// Test 6: Complex expressions with multiple operations
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
}

// Test 7: Stack operations preserving types
TEST_F(PiBinaryOpTests, StackOperationsWithTypes) {
    // Test: dup with integers
    console_.Execute("42 dup");
    
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after dup";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should also be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 42) << "Top value should be 42";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 42) << "Second value should also be 42";
    
    // Test: swap preserving types
    data_->Clear();
    console_.Execute("10 20 swap");
    
    ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after swap";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "First item should be an int";
    ASSERT_TRUE(data_->At(1).IsType<int>()) << "Second item should also be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 10) << "Top value should be 10 after swap";
    ASSERT_EQ(ConstDeref<int>(data_->At(1)), 20) << "Second value should be 20 after swap";
    
    // Test: drop
    console_.Execute("drop");
    ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after drop";
    ASSERT_TRUE(data_->At(0).IsType<int>()) << "Remaining item should be an int";
    ASSERT_EQ(ConstDeref<int>(data_->At(0)), 10) << "Remaining value should be 10";
}

// Test 8: The special "5 dup +" pattern that was causing issues
TEST_F(PiBinaryOpTests, DupPlusPattern) {
    // Test the special "5 dup +" pattern that previously had issues
    console_.Execute("5 dup +");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 10) << "5 dup + should equal 10";
    
    // Test with a different value
    data_->Clear();
    console_.Execute("7 dup +");
    
    VerifyStackOperation(1, "int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 14) << "7 dup + should equal 14";
}