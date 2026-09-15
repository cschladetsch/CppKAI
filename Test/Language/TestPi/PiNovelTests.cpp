#include <gtest/gtest.h>

#include "TestLangCommon.h"

// New Pi tests: regression-style sequential-session coverage plus novel
// stack/arithmetic/string cases, following the confirmed-safe patterns from
// PiMathOperationsTests.cpp / PiStringOperationsTests.cpp (console_.Execute
// with a fresh stack per assertion via stack->Clear()).

struct PiNovelTests : kai::TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

TEST_F(PiNovelTests, SequentialArithmeticAcrossMultipleExecuteCalls) {
    auto exec = console_.GetExecutor();
    auto stack = exec->GetDataStack();

    console_.Execute("5 3 +");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
    stack->Clear();

    console_.Execute("10 4 -");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);
    stack->Clear();

    console_.Execute("6 7 *");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

TEST_F(PiNovelTests, ChainedAdditionWithoutClearingBetweenSteps) {
    // Push several values then combine them in one long postfix expression,
    // exercising a longer single Execute call.
    console_.Execute("1 2 + 3 + 4 + 5 +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);
}

TEST_F(PiNovelTests, MinMaxCombinedWithArithmetic) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("2 3 + 4 5 + min");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
    stack->Clear();

    console_.Execute("2 3 + 4 5 + max");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 9);
}

TEST_F(PiNovelTests, FloatArithmeticSequential) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("1.5 2.5 +");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 4.0f);
    stack->Clear();

    console_.Execute("10.0 4.0 /");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 2.5f);
}

TEST_F(PiNovelTests, ModuloChain) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("20 6 %");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
    stack->Clear();

    console_.Execute("100 9 % 2 %");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 1);
}

TEST_F(PiNovelTests, StringConcatenationMultipleParts) {
    console_.Execute("\"foo\" \"bar\" + \"baz\" +");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "foobarbaz");
}

TEST_F(PiNovelTests, StringSizeAfterConcatenation) {
    console_.Execute("\"Hello\" \" World\" + size");
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 11);
}

TEST_F(PiNovelTests, RepeatedIdenticalExecutionsAreIndependent) {
    auto stack = console_.GetExecutor()->GetDataStack();

    for (int i = 0; i < 5; ++i) {
        console_.Execute("7 6 *");
        ASSERT_EQ(stack->Size(), 1);
        EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
        stack->Clear();
    }
}

TEST_F(PiNovelTests, MultipleIndependentConsolesDoNotShareStack) {
    kai::Console other;
    other.SetLanguage(kai::Language::Pi);

    console_.Execute("1 1 +");
    other.Execute("9 9 +");

    auto stack = console_.GetExecutor()->GetDataStack();
    auto otherStack = other.GetExecutor()->GetDataStack();

    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);

    ASSERT_EQ(otherStack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(otherStack->Top()), 18);
}

TEST_F(PiNovelTests, LongPostfixChainOfTenAdditions) {
    std::string expr = "0";
    for (int i = 1; i <= 10; ++i) {
        expr += " " + std::to_string(i) + " +";
    }
    console_.Execute(expr.c_str());
    auto stack = console_.GetExecutor()->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 55);
}

TEST_F(PiNovelTests, NestedMinMaxDeeperChain) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("50 10 20 30 min max min");
    // 20 30 min -> 20 ; 10 20 min -> 10 ; 50 10 max -> 50
    // Stack ops apply left to right: 50 10 20 30 min max min
    // step: push 50,10,20,30 -> [50,10,20,30]
    // min -> pops 30,20 -> 20 -> [50,10,20]
    // max -> pops 20,10 -> 20 -> [50,20]
    // min -> pops 20,50 -> 20 -> [20]
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 20);
}

TEST_F(PiNovelTests, MixedIntAndFloatArithmeticSequential) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("3 4 +");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
    stack->Clear();

    console_.Execute("3.0 4.0 +");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_FLOAT_EQ(kai::ConstDeref<float>(stack->Top()), 7.0f);
}

TEST_F(PiNovelTests, SubtractionOrderIsPreservedNotCommutative) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("10 3 -");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 7);
    stack->Clear();

    console_.Execute("3 10 -");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), -7);
}

TEST_F(PiNovelTests, DivisionOrderIsPreservedNotCommutative) {
    auto stack = console_.GetExecutor()->GetDataStack();

    console_.Execute("20 4 /");
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}
