// PiCenturyTests2.cpp
//
// 50 more Pi language tests, following on from PiCenturyTests.cpp. This
// round focuses on control flow (if / ife / while, including nested and
// combined-with-arithmetic forms) plus a further sweep of arithmetic,
// comparison and boolean edge cases not covered by the first century file.
// Uses the same Exec/ExpectInt/ExpectBool/ExpectIntStack harness pattern
// established there and in PiExtendedOperationsTests.cpp.

#include "TestLangCommon.h"

using namespace kai;

struct PiCenturyTests2 : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }

    void Exec(const char *code) {
        exec_->ClearStacks();
        console_.Execute(code);
    }

    void ExpectInt(int expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single int on the stack";
        ASSERT_TRUE(data_->Top().IsType<int>()) << "top is not an int";
        EXPECT_EQ(ConstDeref<int>(data_->Top()), expected);
    }

    void ExpectBool(bool expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single bool on the stack";
        ASSERT_TRUE(data_->Top().IsType<bool>()) << "top is not a bool";
        EXPECT_EQ(ConstDeref<bool>(data_->Top()), expected);
    }

    void ExpectEmptyStack() {
        EXPECT_EQ(data_->Size(), 0) << "expected an empty stack";
    }

    void ExpectIntStack(std::initializer_list<int> bottomToTop) {
        ASSERT_EQ(static_cast<size_t>(data_->Size()), bottomToTop.size())
            << "stack size mismatch";
        size_t position = 0;
        for (int value : bottomToTop) {
            int stackIndex = data_->Size() - 1 - static_cast<int>(position);
            ASSERT_TRUE(data_->At(stackIndex).IsType<int>())
                << "non-int at position " << position;
            EXPECT_EQ(ConstDeref<int>(data_->At(stackIndex)), value)
                << "value mismatch at position " << position;
            ++position;
        }
    }
};

// --------------------------------------------------------------------------
// if / ife conditionals (10)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests2, IfTrueOnlyPushesBody) { Exec("true { 42 } if"); ExpectInt(42); }
TEST_F(PiCenturyTests2, IfFalseIsNoOp) { Exec("false { 42 } if"); ExpectEmptyStack(); }
TEST_F(PiCenturyTests2, IfTrueWithArithmeticBody) { Exec("true { 2 3 + } if"); ExpectInt(5); }
TEST_F(PiCenturyTests2, IfFalseLeavesPriorStackUntouched) { Exec("10 false { 99 } if"); ExpectInt(10); }
TEST_F(PiCenturyTests2, IfeTrueBranch) { Exec("true { 1 } { 2 } ife"); ExpectInt(1); }
TEST_F(PiCenturyTests2, IfeFalseBranch) { Exec("false { 1 } { 2 } ife"); ExpectInt(2); }
TEST_F(PiCenturyTests2, IfeWithComputedConditionTrue) { Exec("5 3 > { 100 } { 200 } ife"); ExpectInt(100); }
TEST_F(PiCenturyTests2, IfeWithComputedConditionFalse) { Exec("3 5 > { 100 } { 200 } ife"); ExpectInt(200); }
TEST_F(PiCenturyTests2, IfeNestedInElseBranch) { Exec("false { 1 } { true { 2 } { 3 } ife } ife"); ExpectInt(2); }
TEST_F(PiCenturyTests2, IfeArithmeticInBothBranches) { Exec("true { 2 2 + } { 3 3 + } ife"); ExpectInt(4); }

// --------------------------------------------------------------------------
// while loops (8)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests2, WhileCountUpToFive) { Exec("0 { dup 5 < } { 1 + } while"); ExpectInt(5); }
TEST_F(PiCenturyTests2, WhileCountUpToTen) { Exec("0 { dup 10 < } { 1 + } while"); ExpectInt(10); }
TEST_F(PiCenturyTests2, WhileNeverRunsWhenConditionAlreadyFalse) {
    Exec("5 { dup 3 < } { 1 + } while");
    ExpectInt(5);
}
TEST_F(PiCenturyTests2, WhileDecrementToZero) { Exec("5 { dup 0 > } { 1 - } while"); ExpectInt(0); }
TEST_F(PiCenturyTests2, WhileDoublingUntilThreshold) { Exec("1 { dup 100 < } { 2 * } while"); ExpectInt(128); }
TEST_F(PiCenturyTests2, WhileSubtractUntilZero) { Exec("20 { dup 0 != } { 5 - } while"); ExpectInt(0); }
TEST_F(PiCenturyTests2, WhileResultUsedInFurtherArithmetic) {
    Exec("0 { dup 3 < } { 1 + } while 10 +");
    ExpectInt(13);
}
TEST_F(PiCenturyTests2, WhileSummingOneToFour) {
    // Same accumulator pattern as PiControlFlowTest.ForLoop (which sums 1..5
    // to 15), bounded to 4 instead: 1+2+3+4 = 10.
    Exec("0 1 { dup 4 <= } { swap over + swap 1 + } while drop");
    ExpectInt(10);
}

// --------------------------------------------------------------------------
// Nested control flow / combined with arithmetic (8)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests2, IfeConditionFromArithmeticChain) {
    Exec("2 3 + 4 > { 1 } { 0 } ife");
    ExpectInt(1);
}
TEST_F(PiCenturyTests2, IfeResultUsedInFurtherMath) {
    Exec("true { 10 } { 20 } ife 5 +");
    ExpectInt(15);
}
TEST_F(PiCenturyTests2, NestedIfeBothOuterAndInnerFalse) {
    Exec("false { 1 } { false { 2 } { 3 } ife } ife");
    ExpectInt(3);
}
TEST_F(PiCenturyTests2, NestedIfeBothOuterAndInnerTrue) {
    Exec("true { true { 1 } { 2 } ife } { 3 } ife");
    ExpectInt(1);
}
TEST_F(PiCenturyTests2, WhileResultFeedsIfe) {
    Exec("0 { dup 3 < } { 1 + } while 3 == { 100 } { 200 } ife");
    ExpectInt(100);
}
TEST_F(PiCenturyTests2, IfeWithAndCondition) {
    Exec("true false and { 1 } { 2 } ife");
    ExpectInt(2);
}
TEST_F(PiCenturyTests2, IfeWithOrCondition) {
    Exec("false true or { 1 } { 2 } ife");
    ExpectInt(1);
}
TEST_F(PiCenturyTests2, IfTrueBodyPerformsStackOp) {
    Exec("1 2 true { swap } if");
    ExpectIntStack({2, 1});
}

// --------------------------------------------------------------------------
// Extended arithmetic / comparison edge cases (14)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests2, NegativeTimesNegative) { Exec("0 5 - 0 3 - *"); ExpectInt(15); }
TEST_F(PiCenturyTests2, NegativeDividedByPositive) { Exec("0 10 - 2 /"); ExpectInt(-5); }
TEST_F(PiCenturyTests2, ModOfNegativeDividend) { Exec("0 7 - 3 %"); ExpectInt(-1); }
TEST_F(PiCenturyTests2, ChainedComparisonAndBoolean) { Exec("3 4 < 5 6 < and"); ExpectBool(true); }
TEST_F(PiCenturyTests2, ChainedComparisonOrBothFalse) { Exec("5 3 < 2 1 < or"); ExpectBool(false); }
TEST_F(PiCenturyTests2, DeepAddChainOfTwos) { Exec("2 2 + 2 + 2 + 2 +"); ExpectInt(10); }
TEST_F(PiCenturyTests2, SubThenDivExact) { Exec("20 5 - 3 /"); ExpectInt(5); }
TEST_F(PiCenturyTests2, AddNegativeResultCompare) { Exec("3 10 - 0 <"); ExpectBool(true); }
TEST_F(PiCenturyTests2, MulNegativeOperand) { Exec("0 4 - 5 *"); ExpectInt(-20); }
TEST_F(PiCenturyTests2, DivNegativeByNegative) { Exec("0 20 - 0 4 - /"); ExpectInt(5); }
TEST_F(PiCenturyTests2, CompareChainedEquality) { Exec("2 2 + 2 2 + =="); ExpectBool(true); }
TEST_F(PiCenturyTests2, MinOfTwoNegatives) { Exec("0 5 - 0 3 - min"); ExpectInt(-5); }
TEST_F(PiCenturyTests2, AbsOfMinResult) { Exec("0 5 - 0 3 - min abs"); ExpectInt(5); }
TEST_F(PiCenturyTests2, LargeModThenCompare) { Exec("1000 7 % 0 >"); ExpectBool(true); }

// --------------------------------------------------------------------------
// Extended boolean logic combos (10)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests2, DoubleNotIsIdentity) { Exec("true not not"); ExpectBool(true); }
TEST_F(PiCenturyTests2, XorChainedTwice) { Exec("true false xor false xor"); ExpectBool(true); }
TEST_F(PiCenturyTests2, AndThenOrCombined) { Exec("true false and true or"); ExpectBool(true); }
TEST_F(PiCenturyTests2, OrThenAndCombined) { Exec("true false or true and"); ExpectBool(true); }
TEST_F(PiCenturyTests2, NotOfEqualityComparison) { Exec("5 5 == not"); ExpectBool(false); }
TEST_F(PiCenturyTests2, NotOfAndOfTwoTrues) { Exec("true true and not"); ExpectBool(false); }
TEST_F(PiCenturyTests2, AndOrNotChain) { Exec("true true and false or not"); ExpectBool(false); }
TEST_F(PiCenturyTests2, OrThenAndFalseResult) { Exec("true false or false and"); ExpectBool(false); }
TEST_F(PiCenturyTests2, TripleAndAllTrue) { Exec("true true and true and true and"); ExpectBool(true); }
TEST_F(PiCenturyTests2, TripleOrAllFalse) { Exec("false false or false or false or"); ExpectBool(false); }
