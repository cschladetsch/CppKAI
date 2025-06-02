#include "TestLangCommon.h"

// Advanced test suite for Pi language features
struct PiAdvancedTest : kai::TestLangCommon {};

// Test range-based for loop syntax
TEST_F(PiAdvancedTest, RangeBasedForLoop) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test 1: Sum numbers from 1 to 5
    console_.Execute("0 1 5 { + } for");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5 = 15

    stack->Clear();

    // Test 2: Product of numbers from 1 to 4
    console_.Execute("1 1 4 { * } for");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 24);  // 1*2*3*4 = 24

    stack->Clear();

    // Test 3: Count iterations
    console_.Execute("0 1 10 { drop 1 + } for");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);  // Counted 10 iterations

    stack->Clear();

    // Test 4: Sum of squares
    console_.Execute("0 1 3 { dup * + } for");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 14);  // 1*1 + 2*2 + 3*3 = 14
}

// Test traditional 4-continuation for loop syntax
TEST_F(PiAdvancedTest, TraditionalForLoop) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Traditional C-style for loop: for(i=0; i<3; i++)
    console_.Execute("{ 0 } { dup 3 < } { 1 + } { } for");
    auto stack = exec->GetDataStack();

    // Should have final counter value on stack
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);

    stack->Clear();

    // Sum using traditional for loop
    console_.Execute("0 { 1 } { dup 5 <= } { 1 + } { over + swap } for drop");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 1+2+3+4+5 = 15
}

// Test nested loops
TEST_F(PiAdvancedTest, NestedForLoops) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Nested range-based for loops to calculate sum of products
    // For i from 1 to 2, for j from 1 to 3, sum i*j
    console_.Execute("0 1 2 { 1 3 { over * + } for drop } for");
    auto stack = exec->GetDataStack();

    // Expected: 1*1 + 1*2 + 1*3 + 2*1 + 2*2 + 2*3 = 1+2+3+2+4+6 = 18
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 18);
}

// Test edge cases for range-based for loop
TEST_F(PiAdvancedTest, ForLoopEdgeCases) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test 1: Single iteration (start == end)
    console_.Execute("10 5 5 { + } for");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 10 + 5 = 15

    stack->Clear();

    // Test 2: Zero range (start > end should do nothing or iterate 0 times)
    console_.Execute("100 10 5 { + } for");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);  // No iterations

    stack->Clear();

    // Test 3: Negative range
    console_.Execute("0 -3 -1 { + } for");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -6);  // -3 + -2 + -1 = -6
}

// Test complex operations within for loop body
TEST_F(PiAdvancedTest, ComplexForLoopBody) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Factorial using range-based for loop
    console_.Execute("1 1 5 { * } for");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 120);  // 5! = 120

    stack->Clear();

    // Fibonacci-like sequence: each iteration adds previous two
    // Start with [1, 1], compute next 3 terms
    console_.Execute("1 1 1 3 { over + swap } for");
    // Stack should have: 5 3 (where 5 is F(5) and 3 is F(4))
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
    stack->Pop();
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}