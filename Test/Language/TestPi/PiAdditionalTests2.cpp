// Additional Pi language coverage, following the direct
// console_.Execute()/exec->GetDataStack() pattern used by
// PiMathOperationsTests.cpp: more arithmetic operand combinations, and more
// stack-manipulation word sequences than PiStackManipulationTests.cpp's
// existing coverage exercises.

#include "TestLangCommon.h"

struct PiAdditionalTests2 : kai::TestLangCommon {
    void SetUp() override {
        kai::TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// --- Arithmetic, across operand combinations ---

TEST_F(PiAdditionalTests2, Add_PositiveOperands) {
    console_.Execute("2 3 +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiAdditionalTests2, Add_Zero) {
    console_.Execute("0 0 +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 0);
}

TEST_F(PiAdditionalTests2, Subtract_Basic) {
    console_.Execute("10 3 -");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
}

TEST_F(PiAdditionalTests2, Subtract_NegativeResult) {
    console_.Execute("3 10 -");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -7);
}

TEST_F(PiAdditionalTests2, Multiply_Basic) {
    console_.Execute("6 7 *");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST_F(PiAdditionalTests2, Multiply_ByZero) {
    console_.Execute("123 0 *");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 0);
}

TEST_F(PiAdditionalTests2, Divide_Basic) {
    console_.Execute("20 4 /");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiAdditionalTests2, Min_FirstSmaller) {
    console_.Execute("3 5 min");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST_F(PiAdditionalTests2, Max_FirstSmaller) {
    console_.Execute("3 5 max");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiAdditionalTests2, MinMax_EqualOperands) {
    console_.Execute("4 4 min");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 4);
}

TEST_F(PiAdditionalTests2, ChainedArithmetic_AddThenMultiply) {
    console_.Execute("2 3 + 4 *");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 20);
}

TEST_F(PiAdditionalTests2, ChainedArithmetic_SubtractThenDivide) {
    console_.Execute("20 10 - 5 /");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

// --- Stack manipulation words ---

TEST_F(PiAdditionalTests2, Dup_DuplicatesTop) {
    console_.Execute("9 dup");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 9);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 9);
}

TEST_F(PiAdditionalTests2, Drop_RemovesTop) {
    console_.Execute("1 2 3 drop");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    // Stack trace: 1 2 3 -> drop -> 1 2 (bottom 1, top 2)
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 1);
}

TEST_F(PiAdditionalTests2, Swap_ExchangesTopTwo) {
    console_.Execute("1 2 swap");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 2);
}

TEST_F(PiAdditionalTests2, Over_CopiesSecondToTop) {
    console_.Execute("1 2 over");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 3);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 1);
}

TEST_F(PiAdditionalTests2, Rot_RotatesTopThree) {
    console_.Execute("1 2 3 rot");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 3);
    // Stack trace: 1 2 3 -> rot -> 2 3 1 (bottom to top); At(0) is top.
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 3);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(2)), 2);
}

TEST_F(PiAdditionalTests2, Dup2_DuplicatesTopTwo) {
    console_.Execute("1 2 dup2");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 4);
}

TEST_F(PiAdditionalTests2, Drop2_RemovesTopTwo) {
    console_.Execute("1 2 3 4 drop2");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 1);
}

TEST_F(PiAdditionalTests2, Depth_ReflectsStackSize) {
    console_.Execute("1 2 3 depth");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 4);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST_F(PiAdditionalTests2, Clear_EmptiesStack) {
    console_.Execute("1 2 3 clear");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 0);
}

TEST_F(PiAdditionalTests2, ClearThenPush_OnlyNewValueRemains) {
    console_.Execute("1 2 3 clear 4");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 4);
}

TEST_F(PiAdditionalTests2, Pick_CopiesNthElement) {
    console_.Execute("1 2 3 4 0 pick");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 5);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 4);
}

TEST_F(PiAdditionalTests2, Roll_MovesNthElementToTop) {
    console_.Execute("1 2 3 4 3 roll");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 4);
    // Stack trace: 1 2 3 4 -> 3 roll -> 2 3 4 1
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 1);
}

// --- String operations ---

TEST_F(PiAdditionalTests2, StringConcatenation_TwoWords) {
    console_.Execute("\"foo\" \"bar\" +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "foobar");
}

TEST_F(PiAdditionalTests2, StringConcatenation_WithSpace) {
    console_.Execute("\"hello \" \"world\" +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "hello world");
}

TEST_F(PiAdditionalTests2, StringConcatenation_EmptyOperand) {
    console_.Execute("\"\" \"nonempty\" +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "nonempty");
}

// --- Composite sequences ---

TEST_F(PiAdditionalTests2, Composite_SwapThenDrop) {
    console_.Execute("1 2 3 swap drop");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    // Stack trace: 1 2 3 -> swap -> 1 3 2 -> drop -> 1 3 (bottom 1, top 3)
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 3);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 1);
}

TEST_F(PiAdditionalTests2, Composite_OverThenAdd) {
    console_.Execute("3 4 over +");
    auto stack = console_.GetExecutor()->GetDataStack();
    // Stack trace: 3 4 -> over -> 3 4 3 -> + -> 3 7
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
}

TEST_F(PiAdditionalTests2, Composite_DupThenAdd) {
    console_.Execute("5 dup +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);
}
