// PiExtendedOperationsTests.cpp
//
// 100 additional Pi language tests. These broaden coverage of integer
// arithmetic, comparisons, boolean logic, min/max/abs, stack manipulation,
// compound expressions, floats and strings. Pi is RPN, so every expression is
// unambiguous; expected values were validated against the live Console.
//
// Stack values are addressed with At(0) == top of stack. ExpectIntStack lists
// values from bottom to top.

#include "TestLangCommon.h"

using namespace kai;

struct PiExtendedOpsTest : TestLangCommon {
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

    void ExpectFloat(float expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single float on the stack";
        ASSERT_TRUE(data_->Top().IsType<float>()) << "top is not a float";
        EXPECT_FLOAT_EQ(ConstDeref<float>(data_->Top()), expected);
    }

    void ExpectString(const char *expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single string on the stack";
        ASSERT_TRUE(data_->Top().IsType<String>()) << "top is not a string";
        EXPECT_EQ(ConstDeref<String>(data_->Top()), String(expected));
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
// Integer arithmetic (24)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, AddSmall) { Exec("7 8 +"); ExpectInt(15); }
TEST_F(PiExtendedOpsTest, AddLarge) { Exec("100 250 +"); ExpectInt(350); }
TEST_F(PiExtendedOpsTest, AddZero) { Exec("0 0 +"); ExpectInt(0); }
TEST_F(PiExtendedOpsTest, SubPositive) { Exec("50 20 -"); ExpectInt(30); }
TEST_F(PiExtendedOpsTest, SubToZero) { Exec("9 9 -"); ExpectInt(0); }
TEST_F(PiExtendedOpsTest, SubToNegative) { Exec("5 12 -"); ExpectInt(-7); }
TEST_F(PiExtendedOpsTest, MulSmall) { Exec("8 9 *"); ExpectInt(72); }
TEST_F(PiExtendedOpsTest, MulSquare) { Exec("12 12 *"); ExpectInt(144); }
TEST_F(PiExtendedOpsTest, MulByZero) { Exec("0 99 *"); ExpectInt(0); }
TEST_F(PiExtendedOpsTest, DivExact) { Exec("100 4 /"); ExpectInt(25); }
TEST_F(PiExtendedOpsTest, DivPerfectSquare) { Exec("81 9 /"); ExpectInt(9); }
TEST_F(PiExtendedOpsTest, DivTruncates) { Exec("7 2 /"); ExpectInt(3); }
TEST_F(PiExtendedOpsTest, DivKeyword) { Exec("100 7 div"); ExpectInt(14); }
TEST_F(PiExtendedOpsTest, DivKeywordExact) { Exec("50 5 div"); ExpectInt(10); }
TEST_F(PiExtendedOpsTest, ModSymbol) { Exec("10 3 %"); ExpectInt(1); }
TEST_F(PiExtendedOpsTest, ModSymbolLarge) { Exec("100 7 %"); ExpectInt(2); }
TEST_F(PiExtendedOpsTest, ModKeyword) { Exec("17 5 mod"); ExpectInt(2); }
TEST_F(PiExtendedOpsTest, ModKeyword2) { Exec("50 8 mod"); ExpectInt(2); }
TEST_F(PiExtendedOpsTest, AddChain) { Exec("1 2 + 3 + 4 +"); ExpectInt(10); }
TEST_F(PiExtendedOpsTest, SubChain) { Exec("100 10 - 20 -"); ExpectInt(70); }
TEST_F(PiExtendedOpsTest, MulChain) { Exec("2 2 * 2 * 2 *"); ExpectInt(16); }
TEST_F(PiExtendedOpsTest, AddThenMul) { Exec("3 4 + 5 *"); ExpectInt(35); }
TEST_F(PiExtendedOpsTest, MulThenAdd) { Exec("3 4 * 5 +"); ExpectInt(17); }
TEST_F(PiExtendedOpsTest, BigMultiplication) { Exec("1000 1000 *"); ExpectInt(1000000); }

// --------------------------------------------------------------------------
// Integer comparisons -> bool (20)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, GreaterTrue) { Exec("10 2 >"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, GreaterFalse) { Exec("2 10 >"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, GreaterEqualValues) { Exec("5 5 >"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, LessTrue) { Exec("2 10 <"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, LessFalse) { Exec("10 2 <"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, LessEqualValues) { Exec("5 5 <"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, GreaterEqualBoundary) { Exec("5 5 >="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, GreaterEqualTrue) { Exec("6 5 >="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, GreaterEqualFalse) { Exec("4 5 >="); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, LessEqualBoundary) { Exec("5 5 <="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, LessEqualTrue) { Exec("4 5 <="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, LessEqualFalse) { Exec("6 5 <="); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, EqualTrue) { Exec("7 7 =="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, EqualFalse) { Exec("7 8 =="); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, EqualZeros) { Exec("0 0 =="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, NotEqualTrue) { Exec("7 8 !="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, NotEqualFalse) { Exec("7 7 !="); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, CompareSum) { Exec("2 3 + 5 =="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, CompareProduct) { Exec("3 3 * 9 =="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, NegativeLessThanZero) { Exec("0 5 - 0 <"); ExpectBool(true); }

// --------------------------------------------------------------------------
// Boolean logic (12)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, AndBothTrue) { Exec("true true and"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, AndOneFalse) { Exec("true false and"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, AndBothFalse) { Exec("false false and"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, OrOneTrue) { Exec("true false or"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, OrBothFalse) { Exec("false false or"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, OrBothTrue) { Exec("true true or"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, NotOfTrue) { Exec("true not"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, NotOfFalse) { Exec("false not"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, XorDifferent) { Exec("true false xor"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, XorBothTrue) { Exec("true true xor"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, XorBothFalse) { Exec("false false xor"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, AndThenOr) { Exec("true false and false or"); ExpectBool(false); }

// --------------------------------------------------------------------------
// min / max / abs (12)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, MinLeftSmaller) { Exec("3 8 min"); ExpectInt(3); }
TEST_F(PiExtendedOpsTest, MinRightSmaller) { Exec("8 3 min"); ExpectInt(3); }
TEST_F(PiExtendedOpsTest, MinEqual) { Exec("5 5 min"); ExpectInt(5); }
TEST_F(PiExtendedOpsTest, MaxLeftLarger) { Exec("8 3 max"); ExpectInt(8); }
TEST_F(PiExtendedOpsTest, MaxRightLarger) { Exec("3 8 max"); ExpectInt(8); }
TEST_F(PiExtendedOpsTest, MaxEqual) { Exec("5 5 max"); ExpectInt(5); }
TEST_F(PiExtendedOpsTest, MinOfNegatives) { Exec("0 5 - 0 9 - min"); ExpectInt(-9); }
TEST_F(PiExtendedOpsTest, MaxOfNegatives) { Exec("0 5 - 0 9 - max"); ExpectInt(-5); }
TEST_F(PiExtendedOpsTest, AbsOfNegative) { Exec("0 12 - abs"); ExpectInt(12); }
TEST_F(PiExtendedOpsTest, AbsOfPositive) { Exec("15 abs"); ExpectInt(15); }
TEST_F(PiExtendedOpsTest, MinChained) { Exec("10 20 30 min min"); ExpectInt(10); }
TEST_F(PiExtendedOpsTest, MaxChained) { Exec("10 20 30 max max"); ExpectInt(30); }

// --------------------------------------------------------------------------
// Stack manipulation (12)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, DupLeavesTwo) { Exec("9 dup"); ExpectIntStack({9, 9}); }
TEST_F(PiExtendedOpsTest, DropRemovesTop) { Exec("1 2 3 drop"); ExpectIntStack({1, 2}); }
TEST_F(PiExtendedOpsTest, SwapTopTwo) { Exec("7 8 swap"); ExpectIntStack({8, 7}); }
TEST_F(PiExtendedOpsTest, OverCopiesSecond) { Exec("3 4 over"); ExpectIntStack({3, 4, 3}); }
TEST_F(PiExtendedOpsTest, RotThree) { Exec("1 2 3 rot"); ExpectIntStack({2, 3, 1}); }
TEST_F(PiExtendedOpsTest, RotFourTouchesTopThree) { Exec("1 2 3 4 rot"); ExpectIntStack({1, 3, 4, 2}); }
TEST_F(PiExtendedOpsTest, Dup2Pair) { Exec("1 2 dup2"); ExpectIntStack({1, 2, 1, 2}); }
TEST_F(PiExtendedOpsTest, Dup2TopPair) { Exec("1 2 3 4 dup2"); ExpectIntStack({1, 2, 3, 4, 3, 4}); }
TEST_F(PiExtendedOpsTest, Drop2RemovesTopTwo) { Exec("1 2 3 4 drop2"); ExpectIntStack({1, 2}); }
TEST_F(PiExtendedOpsTest, DepthPushesCount) { Exec("1 2 3 depth"); ExpectIntStack({1, 2, 3, 3}); }
TEST_F(PiExtendedOpsTest, PickCopiesIndexedValue) { Exec("1 2 3 4 2 pick"); ExpectIntStack({1, 2, 3, 4, 2}); }
TEST_F(PiExtendedOpsTest, DupThenMultiplySquares) { Exec("6 dup *"); ExpectInt(36); }

// --------------------------------------------------------------------------
// Compound expressions (12)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, EvenNumberIsEven) { Exec("8 2 % 0 =="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, OddNumberIsNotEven) { Exec("9 2 % 0 =="); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, ValueWithinRange) { Exec("5 1 >= 5 10 <= and"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, ValueOutsideRange) { Exec("15 1 >= 15 10 <= and"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, SumLessThanSum) { Exec("10 5 + 20 5 + <"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, ProductNotGreater) { Exec("4 5 * 3 7 * >"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, NotOfAnd) { Exec("true true and not"); ExpectBool(false); }
TEST_F(PiExtendedOpsTest, OrOfComparisons) { Exec("1 2 > 5 3 > or"); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, NestedArithmetic) { Exec("2 3 + 4 * 5 -"); ExpectInt(15); }
TEST_F(PiExtendedOpsTest, NestedArithmetic2) { Exec("100 5 / 4 - 2 *"); ExpectInt(32); }
TEST_F(PiExtendedOpsTest, DoubleThenCompare) { Exec("5 dup + 10 =="); ExpectBool(true); }
TEST_F(PiExtendedOpsTest, AbsThenCompare) { Exec("0 7 - abs 7 =="); ExpectBool(true); }

// --------------------------------------------------------------------------
// Float arithmetic (4)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, FloatAdd) { Exec("1.25 2.75 +"); ExpectFloat(4.0f); }
TEST_F(PiExtendedOpsTest, FloatSub) { Exec("10.5 0.5 -"); ExpectFloat(10.0f); }
TEST_F(PiExtendedOpsTest, FloatMul) { Exec("2.5 4.0 *"); ExpectFloat(10.0f); }
TEST_F(PiExtendedOpsTest, FloatDiv) { Exec("9.0 2.0 /"); ExpectFloat(4.5f); }

// --------------------------------------------------------------------------
// String operations (4)
// --------------------------------------------------------------------------

TEST_F(PiExtendedOpsTest, StringConcatShort) { Exec("\"foo\" \"bar\" +"); ExpectString("foobar"); }
TEST_F(PiExtendedOpsTest, StringConcatWithSpace) { Exec("\"Hello, \" \"World!\" +"); ExpectString("Hello, World!"); }
TEST_F(PiExtendedOpsTest, StringSize) { Exec("\"hello\" size"); ExpectInt(5); }
TEST_F(PiExtendedOpsTest, IntegerToString) { Exec("256 to_str"); ExpectString("256"); }
