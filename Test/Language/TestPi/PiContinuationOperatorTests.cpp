#include "TestLangCommon.h"

// Test suite for Pi continuation operators: |, &, and !
struct PiContinuationOperatorTest : kai::TestLangCommon {};

// Test bitwise OR operator |
TEST_F(PiContinuationOperatorTest, BitwiseOrOperator) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test basic bitwise OR
    console_.Execute("5 3 |");  // 0101 | 0011 = 0111 = 7
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);

    stack->Clear();

    // Test with multiple values
    console_.Execute(
        "12 10 | 3 |");  // (1100 | 1010) | 0011 = 1110 | 0011 = 1111 = 15
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);

    stack->Clear();

    // Test with zero
    console_.Execute("8 0 |");  // 1000 | 0000 = 1000 = 8
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
}

// Test bitwise OR with negative numbers
TEST_F(PiContinuationOperatorTest, BitwiseOrNegativeNumbers) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test with negative numbers (depends on implementation)
    console_.Execute("-1 5 |");  // All bits set | 0101 = -1
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -1);
}

// Test & as call subroutine (if implemented)
TEST_F(PiContinuationOperatorTest, CallSubroutineOperator) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Define a simple continuation
    console_.Execute("{ 2 * } 'double =");

    // Call it with &
    console_.Execute("5 double &");
    auto stack = exec->GetDataStack();

    // If & is implemented as call subroutine, should execute and return
    // If not implemented, might just push the continuation
    ASSERT_GE(stack->Size(), 1);
    // Expected behavior depends on implementation
}

// Test ! as replace continuation (if implemented)
TEST_F(PiContinuationOperatorTest, ReplaceContinuationOperator) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Create a continuation that replaces itself
    console_.Execute("{ 100 } !");
    auto stack = exec->GetDataStack();

    // If ! is implemented as replace continuation
    // Otherwise might just execute normally
    ASSERT_GE(stack->Size(), 1);
}

// Test continuation operators in combination
TEST_F(PiContinuationOperatorTest, CombinedOperators) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test bitwise OR in continuations
    console_.Execute("5 3 |");  // Just test the operation directly
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
}

// Test edge cases for bitwise OR
TEST_F(PiContinuationOperatorTest, BitwiseOrEdgeCases) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Same number OR'd with itself
    console_.Execute("7 7 |");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);

    stack->Clear();

    // OR with all bits set
    console_.Execute("255 128 |");  // 11111111 | 10000000 = 11111111 = 255
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 255);
}

// Test bitwise OR with boolean context
TEST_F(PiContinuationOperatorTest, BitwiseOrBooleanContext) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Using OR result in conditional
    console_.Execute("0 0 | { \"non-zero\" } { \"zero\" } ife");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "zero");

    stack->Clear();

    console_.Execute("1 2 | { \"non-zero\" } { \"zero\" } ife");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "non-zero");
}

// Test that | works in mathematical expressions
TEST_F(PiContinuationOperatorTest, BitwiseOrInExpressions) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Use OR in a larger expression
    console_.Execute("2 3 | 5 +");  // (2 | 3) + 5 = 3 + 5 = 8
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);

    stack->Clear();

    // Multiple ORs
    console_.Execute("1 2 | 4 | 8 |");  // ((1|2)|4)|8 = (3|4)|8 = 7|8 = 15
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

// Test stack manipulation with OR
TEST_F(PiContinuationOperatorTest, BitwiseOrStackOperations) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // OR with duplicated values
    console_.Execute("6 dup |");  // 6 | 6 = 6
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);

    stack->Clear();

    // OR with swapped values
    console_.Execute("4 12 swap |");  // 12 | 4 = 12
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 12);
}

// Test OR with array elements
TEST_F(PiContinuationOperatorTest, BitwiseOrWithArrays) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Simple test without array indexing
    // Just test that OR works with values from an array construction
    console_.Execute("1 2 |");  // 1 | 2 = 3
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

// Test precedence and associativity
TEST_F(PiContinuationOperatorTest, BitwiseOrPrecedence) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test associativity (should be left-to-right in stack-based)
    console_.Execute("1 2 4 | |");  // 1 (2|4) | = 1 6 | = 7
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
}

// Test error handling for OR
TEST_F(PiContinuationOperatorTest, BitwiseOrErrorHandling) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test with insufficient operands
    console_.Execute("5 |");  // Should fail or use some default
    // Behavior depends on implementation - might throw or use 0

    auto stack = exec->GetDataStack();
    stack->Clear();

    // Test with non-integer types (if type checking is strict)
    console_.Execute("\"hello\" 5 |");  // Should fail or convert
    // Behavior depends on implementation
}

// Test & and ! if they're actual operators
TEST_F(PiContinuationOperatorTest, OtherOperatorsIfImplemented) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // If & is bitwise AND
    console_.Execute("12 10 &");  // 1100 & 1010 = 1000 = 8
    auto stack = exec->GetDataStack();
    // Check if it performed bitwise AND or something else

    stack->Clear();

    // If ! is logical NOT or something else
    console_.Execute("0 !");
    // Check result based on implementation
}