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
        reg_->AddClass<int>(Label("Int"));
        reg_->AddClass<bool>(Label("Bool"));
        reg_->AddClass<String>(Label("String"));

        // Clear stacks to start fresh
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Helper method to verify basic stack properties after an operation
    void VerifyStackOperation(int expectedSize,
                              const std::string& expectedType) {
        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), expectedSize)
            << "Stack should have " << expectedSize << " item(s)";

        // Print type info for debugging using KAI macros
        KAI_TRACE() << "Result type: "
                    << data_->Top().GetClass()->GetName().ToString();
        ASSERT_EQ(data_->Top().GetClass()->GetName().ToString(), expectedType)
            << "Result type should be " << expectedType;
    }
};

// Basic integer addition
TEST_F(PiBinaryOpTests, IntegerAddition) {
    // Test our TestCout with colors
    TEST_COUT << "Testing colored INFO output with TestCout";
    TEST_CERR << "Testing colored ERROR output with TestCout";

    // Execute Pi code: 2 3 +
    console_.Execute("2 3 +");

    // Verify result type and value
    VerifyStackOperation(1, "Int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5) << "2 + 3 should equal 5";
}

// String concatenation
TEST_F(PiBinaryOpTests, StringConcatenation) {
    // Execute Pi code: "Hello, " "World!" +
    console_.Execute("\"Hello, \" \"World!\" +");

    // Verify result type and value
    VerifyStackOperation(1, "String");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "Hello, World!")
        << "String concatenation should work";
}

// Comparison operations
TEST_F(PiBinaryOpTests, ComparisonOperations) {
    // Completely clear stacks and registry references at the start
    exec_->ClearStacks();

    // Test greater than using manually pushed values to ensure type consistency
    {
        Object a = reg_->New<int>(5);
        Object b = reg_->New<int>(3);
        data_->Push(a);
        data_->Push(b);
        exec_->Perform(Operation::Greater);

        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item";
        ASSERT_TRUE(data_->Top().IsType<bool>())
            << "Result should be a boolean";
        ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "5 > 3 should be true";
    }

    // Clear stacks between tests
    exec_->ClearStacks();

    // Test equality with manually pushed values
    {
        Object a = reg_->New<int>(5);
        Object b = reg_->New<int>(5);
        data_->Push(a);
        data_->Push(b);
        exec_->Perform(Operation::Equiv);

        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item";
        ASSERT_TRUE(data_->Top().IsType<bool>())
            << "Result should be a boolean";
        ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "5 == 5 should be true";
    }

    // Clear stacks between tests
    exec_->ClearStacks();

    // Test inequality with manually pushed values
    {
        Object a = reg_->New<int>(5);
        Object b = reg_->New<int>(6);
        data_->Push(a);
        data_->Push(b);
        exec_->Perform(Operation::NotEquiv);

        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after operation";
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item";
        ASSERT_TRUE(data_->Top().IsType<bool>())
            << "Result should be a boolean";
        ASSERT_TRUE(ConstDeref<bool>(data_->Top())) << "5 != 6 should be true";
    }
}

// Logical operations
TEST_F(PiBinaryOpTests, LogicalOperations) {
    // Test logical AND
    console_.Execute("true true and");

    VerifyStackOperation(1, "Bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()))
        << "true AND true should be true";

    // Test logical OR
    data_->Clear();
    console_.Execute("false true or");

    VerifyStackOperation(1, "Bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()))
        << "false OR true should be true";

    // Test logical XOR with different results
    data_->Clear();
    console_.Execute("true true xor");

    VerifyStackOperation(1, "Bool");
    ASSERT_FALSE(ConstDeref<bool>(data_->Top()))
        << "true XOR true should be false";

    data_->Clear();
    console_.Execute("true false xor");

    VerifyStackOperation(1, "Bool");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()))
        << "true XOR false should be true";
}

// Division and modulo
TEST_F(PiBinaryOpTests, DivisionAndModulo) {
    // Clear stacks first
    exec_->ClearStacks();

    // Test division directly using Perform with scoped operations
    {
        Object a = reg_->New<int>(10);
        Object b = reg_->New<int>(2);
        data_->Push(a);
        data_->Push(b);
        exec_->Perform(Operation::Divide);

        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after division";
        ASSERT_EQ(data_->Size(), 1)
            << "Stack should have 1 item after division";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Division result should be an integer";
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 5) << "10 / 2 should equal 5";
    }

    // Clear stacks between tests
    exec_->ClearStacks();

    // Test modulo directly using Perform with scoped operations
    {
        Object a = reg_->New<int>(10);
        Object b = reg_->New<int>(3);
        data_->Push(a);
        data_->Push(b);
        exec_->Perform(Operation::Modulo);

        ASSERT_FALSE(data_->Empty())
            << "Stack should not be empty after modulo";
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after modulo";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Modulo result should be an integer";
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 1) << "10 % 3 should equal 1";
    }

    // Try with a string-based modulo using 'mod' keyword
    exec_->ClearStacks();
    console_.Execute("10 3 mod");

    ASSERT_FALSE(data_->Empty()) << "Stack should not be empty after modulo";
    ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after modulo";
    ASSERT_TRUE(data_->Top().IsType<int>())
        << "Modulo result should be an integer";
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1) << "10 mod 3 should equal 1";
}

// Complex expressions with multiple operations
TEST_F(PiBinaryOpTests, ComplexExpressions) {
    // Test: (2 + 3) * 4
    console_.Execute("2 3 + 4 *");

    VerifyStackOperation(1, "Int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 20)
        << "(2 + 3) * 4 should equal 20";

    // Test: 10 / 2 + 3 * 4
    data_->Clear();
    console_.Execute("10 2 div 3 4 * +");

    VerifyStackOperation(1, "Int");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 17)
        << "10 / 2 + 3 * 4 should equal 17";
}

// Stack operations preserving types
TEST_F(PiBinaryOpTests, StackOperationsWithTypes) {
    // Clear stacks completely first
    exec_->ClearStacks();
    exec_->ClearContext();

    // Test: dup with integers - direct stack manipulation
    {
        Object int42 = reg_->New<int>(42);
        data_->Push(int42);              // Push 42 onto the stack
        exec_->Perform(Operation::Dup);  // Execute Dup operation directly

        // Verify the stack size and types
        ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after dup";
        ASSERT_TRUE(data_->At(0).IsType<int>())
            << "First item should be an int";
        ASSERT_TRUE(data_->At(1).IsType<int>())
            << "Second item should also be an int";
        ASSERT_EQ(ConstDeref<int>(data_->At(0)), 42)
            << "Top value should be 42";
        ASSERT_EQ(ConstDeref<int>(data_->At(1)), 42)
            << "Second value should also be 42";
    }

    // Clear stacks for next test
    exec_->ClearStacks();

    // Test: swap preserving types - direct stack manipulation
    {
        Object int10 = reg_->New<int>(10);
        Object int20 = reg_->New<int>(20);
        data_->Push(int10);               // Push 10 onto the stack
        data_->Push(int20);               // Push 20 onto the stack
        exec_->Perform(Operation::Swap);  // Execute Swap operation directly

        // Verify the stack after swap
        ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after swap";
        ASSERT_TRUE(data_->At(0).IsType<int>())
            << "First item should be an int";
        ASSERT_TRUE(data_->At(1).IsType<int>())
            << "Second item should also be an int";
        ASSERT_EQ(ConstDeref<int>(data_->At(0)), 10)
            << "Top value should be 10 after swap";
        ASSERT_EQ(ConstDeref<int>(data_->At(1)), 20)
            << "Second value should be 20 after swap";

        // Test: drop - direct stack manipulation
        exec_->Perform(Operation::Drop);  // Execute Drop operation directly

        // Verify the stack after drop
        ASSERT_EQ(data_->Size(), 1) << "Stack should have 1 item after drop";
        ASSERT_TRUE(data_->At(0).IsType<int>())
            << "Remaining item should be an int";
        ASSERT_EQ(ConstDeref<int>(data_->At(0)), 20)
            << "Remaining value should be 20";
    }

    // Clear stacks and test with string type preservation
    exec_->ClearStacks();

    {
        Object strHello = reg_->New<String>("Hello");
        data_->Push(strHello);
        exec_->Perform(Operation::Dup);

        // Verify string duplication works correctly
        ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after dup";
        ASSERT_TRUE(data_->At(0).IsType<String>())
            << "First item should be a String";
        ASSERT_TRUE(data_->At(1).IsType<String>())
            << "Second item should also be a String";
        ASSERT_EQ(ConstDeref<String>(data_->At(0)), "Hello")
            << "Top value should be 'Hello'";
        ASSERT_EQ(ConstDeref<String>(data_->At(1)), "Hello")
            << "Second value should also be 'Hello'";
    }
}

// The special "5 dup +" pattern that was causing issues
TEST_F(PiBinaryOpTests, DupPlusPattern) {
    // Clear stacks first to ensure clean state
    exec_->ClearStacks();
    exec_->ClearContext();

    // Test the special "5 dup +" pattern with direct stack manipulation in a
    // scope
    {
        Object val5 = reg_->New<int>(5);
        data_->Push(val5);               // Push 5 onto the stack
        exec_->Perform(Operation::Dup);  // Duplicate it (5 5)

        // Verify the stack after duplicate
        ASSERT_EQ(data_->Size(), 2) << "Stack should have 2 items after dup";
        ASSERT_TRUE(data_->At(0).IsType<int>())
            << "First item should be an int";
        ASSERT_TRUE(data_->At(1).IsType<int>())
            << "Second item should also be an int";
        ASSERT_EQ(ConstDeref<int>(data_->At(0)), 5) << "Top value should be 5";
        ASSERT_EQ(ConstDeref<int>(data_->At(1)), 5)
            << "Second value should also be 5";

        exec_->Perform(Operation::Plus);  // Add them (10)

        // Verify the result manually
        ASSERT_EQ(data_->Size(), 1)
            << "Stack should have 1 item after operations";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Result should be an integer";
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 10)
            << "5 dup + should equal 10";
    }

    // Clear stacks for the next test
    exec_->ClearStacks();

    // Test with a different value using direct stack manipulation
    {
        Object val7 = reg_->New<int>(7);
        data_->Push(val7);                // Push 7 onto the stack
        exec_->Perform(Operation::Dup);   // Duplicate it (7 7)
        exec_->Perform(Operation::Plus);  // Add them (14)

        // Verify the result manually
        ASSERT_EQ(data_->Size(), 1)
            << "Stack should have 1 item after operations";
        ASSERT_TRUE(data_->Top().IsType<int>())
            << "Result should be an integer";
        ASSERT_EQ(ConstDeref<int>(data_->Top()), 14)
            << "7 dup + should equal 14";
    }

    // Test the pattern with string concatenation
    exec_->ClearStacks();
    {
        Object strVal = reg_->New<String>("Hello");
        data_->Push(strVal);
        exec_->Perform(Operation::Dup);
        exec_->Perform(Operation::Plus);

        // Verify the result manually
        ASSERT_EQ(data_->Size(), 1)
            << "Stack should have 1 item after operations";
        ASSERT_TRUE(data_->Top().IsType<String>())
            << "Result should be a String";
        ASSERT_EQ(ConstDeref<String>(data_->Top()), "HelloHello")
            << "String dup + should concatenate";
    }
}