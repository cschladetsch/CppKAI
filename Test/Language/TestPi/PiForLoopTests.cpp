#include "TestLangCommon.h"

// Test suite for Pi for loops - simplified and focused
struct PiForLoopTest : kai::TestLangCommon {};

// Range-based for loop syntax: accumulator start end { body } for
TEST_F(PiForLoopTest, SimpleRangeSum) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Sum numbers from 1 to 5: 0 1 5 { + } for
    console_.Execute("0 1 5 { + } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15); // 1+2+3+4+5
}

TEST_F(PiForLoopTest, RangeProduct) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Product of numbers from 1 to 4: 1 1 4 { * } for
    console_.Execute("1 1 4 { * } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 24); // 1*2*3*4
}

TEST_F(PiForLoopTest, CollectSquares) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Collect squares: [] 1 4 { dup * + } for
    console_.Execute("[] 1 4 { dup * + } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    auto array = kai::ConstDeref<kai::Array>(stack->Top());
    ASSERT_EQ(array.Size(), 4);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(0)), 1);  // 1*1
    EXPECT_EQ(kai::ConstDeref<int>(array.At(1)), 4);  // 2*2
    EXPECT_EQ(kai::ConstDeref<int>(array.At(2)), 9);  // 3*3
    EXPECT_EQ(kai::ConstDeref<int>(array.At(3)), 16); // 4*4
}

TEST_F(PiForLoopTest, ConditionalSum) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Sum only even numbers from 1 to 6
    // Stack: accumulator current -> accumulator'
    console_.Execute("0 1 6 { dup 2 % 0 == { + } { drop } ife } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 12); // 2+4+6
}

// Traditional 4-continuation syntax if needed
TEST_F(PiForLoopTest, TraditionalSyntax) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Traditional for loop: { init } { condition } { increment } { body } for
    // Count from 0 to 4
    console_.Execute("{ 0 } { dup 5 < } { 1 + } { } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiForLoopTest, TraditionalWithBody) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Sum using traditional syntax
    console_.Execute("0 { 0 } { dup 5 < } { 1 + } { swap over + swap } for drop");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10); // 0+1+2+3+4
}

TEST_F(PiForLoopTest, NestedRangeLoops) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Nested range loops to build multiplication table entry
    // Outer: 2, Inner: 1-3, calculate 2*1 + 2*2 + 2*3 = 12
    console_.Execute("0 2 2 { 1 3 { over * + } for drop } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 12);
}

TEST_F(PiForLoopTest, EmptyRange) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Empty range (start > end) should just return accumulator
    console_.Execute("42 5 1 { + } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST_F(PiForLoopTest, SingleIteration) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Single iteration (start == end)
    console_.Execute("10 3 3 { + } for");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 13); // 10 + 3
}

TEST_F(PiForLoopTest, ArrayBuilding) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Build an array using range loop
    console_.Execute("[] 1 3 { swap over swap + } for drop");
    auto stack = exec->GetDataStack();
    
    ASSERT_EQ(stack->Size(), 1);
    auto array = kai::ConstDeref<kai::Array>(stack->Top());
    ASSERT_EQ(array.Size(), 3);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(0)), 1);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(1)), 2);
    EXPECT_EQ(kai::ConstDeref<int>(array.At(2)), 3);
}