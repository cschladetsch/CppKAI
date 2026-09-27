// PiCenturyTests.cpp
//
// 100 additional Pi language tests, added as a fresh regression sweep after
// fixing the GET16BITS/SearchPath/field-rename build breakage that was
// blocking compilation on generic 64-bit GCC/Clang. Pi is RPN, so every
// expression is unambiguous; expected values were computed by hand and then
// confirmed against the live Console. Follows the harness pattern already
// established by PiExtendedOperationsTests.cpp / PiAdditionalOperationsTests.cpp
// (Exec/ExpectInt/ExpectBool/ExpectFloat/ExpectString/ExpectIntStack).

#include "TestLangCommon.h"

using namespace kai;

struct PiCenturyTests : TestLangCommon {
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
// Integer arithmetic combos (12)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, AddSelfSame) { Exec("9 9 +"); ExpectInt(18); }
TEST_F(PiCenturyTests, SubSelfSame) { Exec("9 9 -"); ExpectInt(0); }
TEST_F(PiCenturyTests, MulSelfSame) { Exec("9 9 *"); ExpectInt(81); }
TEST_F(PiCenturyTests, DivExactSmall) { Exec("9 3 /"); ExpectInt(3); }
TEST_F(PiCenturyTests, ModSymbolTwenty) { Exec("20 3 %"); ExpectInt(2); }
TEST_F(PiCenturyTests, ModKeywordTwenty) { Exec("20 3 mod"); ExpectInt(2); }
TEST_F(PiCenturyTests, AddMulSub) { Exec("1 2 + 3 * 4 -"); ExpectInt(5); }
TEST_F(PiCenturyTests, DivThenAdd) { Exec("10 2 / 3 +"); ExpectInt(8); }
TEST_F(PiCenturyTests, TripleMul) { Exec("2 3 * 4 * 5 *"); ExpectInt(120); }
TEST_F(PiCenturyTests, DoubleDecrement) { Exec("100 1 - 1 -"); ExpectInt(98); }
TEST_F(PiCenturyTests, SquareThenSub) { Exec("7 7 * 7 -"); ExpectInt(42); }
TEST_F(PiCenturyTests, LargeSubToOne) { Exec("1000 999 -"); ExpectInt(1); }

// --------------------------------------------------------------------------
// Comparisons -> bool (12)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, EqOnesTrue) { Exec("1 1 =="); ExpectBool(true); }
TEST_F(PiCenturyTests, EqDifferentFalse) { Exec("1 2 =="); ExpectBool(false); }
TEST_F(PiCenturyTests, NeqDifferentTrue) { Exec("1 2 !="); ExpectBool(true); }
TEST_F(PiCenturyTests, NeqSameFalse) { Exec("2 2 !="); ExpectBool(false); }
TEST_F(PiCenturyTests, LessTrueSmallFirst) { Exec("5 6 <"); ExpectBool(true); }
TEST_F(PiCenturyTests, LessFalseBigFirst) { Exec("6 5 <"); ExpectBool(false); }
TEST_F(PiCenturyTests, GreaterFalseSmallFirst) { Exec("5 6 >"); ExpectBool(false); }
TEST_F(PiCenturyTests, GreaterTrueBigFirst) { Exec("6 5 >"); ExpectBool(true); }
TEST_F(PiCenturyTests, LessEqualSameTrue) { Exec("5 5 <="); ExpectBool(true); }
TEST_F(PiCenturyTests, LessEqualFalse) { Exec("5 4 <="); ExpectBool(false); }
TEST_F(PiCenturyTests, GreaterEqualSameTrue) { Exec("5 5 >="); ExpectBool(true); }
TEST_F(PiCenturyTests, GreaterEqualFalse) { Exec("4 5 >="); ExpectBool(false); }

// --------------------------------------------------------------------------
// Boolean logic (10)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, AndBothTrue) { Exec("true true and"); ExpectBool(true); }
TEST_F(PiCenturyTests, AndBothFalse) { Exec("false false and"); ExpectBool(false); }
TEST_F(PiCenturyTests, OrOneTrue) { Exec("true false or"); ExpectBool(true); }
TEST_F(PiCenturyTests, OrBothFalse) { Exec("false false or"); ExpectBool(false); }
TEST_F(PiCenturyTests, NotTrueIsFalse) { Exec("true not"); ExpectBool(false); }
TEST_F(PiCenturyTests, NotFalseIsTrue) { Exec("false not"); ExpectBool(true); }
TEST_F(PiCenturyTests, XorBothTrueIsFalse) { Exec("true true xor"); ExpectBool(false); }
TEST_F(PiCenturyTests, XorOneTrueIsTrue) { Exec("false true xor"); ExpectBool(true); }
TEST_F(PiCenturyTests, AndChain) { Exec("true true and true and"); ExpectBool(true); }
TEST_F(PiCenturyTests, AndThenOrFalse) { Exec("false true and false or"); ExpectBool(false); }

// --------------------------------------------------------------------------
// Min / Max (8)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, MinSmallFirst) { Exec("3 7 min"); ExpectInt(3); }
TEST_F(PiCenturyTests, MaxSmallFirst) { Exec("3 7 max"); ExpectInt(7); }
TEST_F(PiCenturyTests, MinBigFirst) { Exec("10 2 min"); ExpectInt(2); }
TEST_F(PiCenturyTests, MaxBigFirst) { Exec("10 2 max"); ExpectInt(10); }
TEST_F(PiCenturyTests, MinEqual) { Exec("5 5 min"); ExpectInt(5); }
TEST_F(PiCenturyTests, MaxEqual) { Exec("5 5 max"); ExpectInt(5); }
TEST_F(PiCenturyTests, MinOfSumAndLiteral) { Exec("1 2 + 5 min"); ExpectInt(3); }
TEST_F(PiCenturyTests, MaxOfSumAndLiteral) { Exec("1 2 + 5 max"); ExpectInt(5); }

// --------------------------------------------------------------------------
// Abs (6)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, AbsPositiveInt) { Exec("5 abs"); ExpectInt(5); }
TEST_F(PiCenturyTests, AbsNegativeInt) { Exec("0 5 - abs"); ExpectInt(5); }
TEST_F(PiCenturyTests, AbsZero) { Exec("0 abs"); ExpectInt(0); }
TEST_F(PiCenturyTests, AbsPositiveFloat) { Exec("3.5 abs"); ExpectFloat(3.5f); }
TEST_F(PiCenturyTests, AbsNegativeFloat) { Exec("0.0 4.5 - abs"); ExpectFloat(4.5f); }
TEST_F(PiCenturyTests, AbsSumOfTwoAbs) { Exec("0 9 - abs 0 1 - abs +"); ExpectInt(10); }

// --------------------------------------------------------------------------
// Stack manipulation (24)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, StackDup) { Exec("1 dup"); ExpectIntStack({1, 1}); }
TEST_F(PiCenturyTests, StackDrop) { Exec("1 2 drop"); ExpectIntStack({1}); }
TEST_F(PiCenturyTests, StackSwap) { Exec("1 2 swap"); ExpectIntStack({2, 1}); }
TEST_F(PiCenturyTests, StackOver) { Exec("1 2 over"); ExpectIntStack({1, 2, 1}); }
TEST_F(PiCenturyTests, StackRotOnce) { Exec("1 2 3 rot"); ExpectIntStack({2, 3, 1}); }
TEST_F(PiCenturyTests, StackDup2Pair) { Exec("1 2 dup2"); ExpectIntStack({1, 2, 1, 2}); }
TEST_F(PiCenturyTests, StackDrop2Quad) { Exec("1 2 3 4 drop2"); ExpectIntStack({1, 2}); }
TEST_F(PiCenturyTests, StackPickZero) { Exec("1 2 3 4 0 pick"); ExpectIntStack({1, 2, 3, 4, 4}); }
TEST_F(PiCenturyTests, StackPickOne) { Exec("1 2 3 4 1 pick"); ExpectIntStack({1, 2, 3, 4, 3}); }
TEST_F(PiCenturyTests, StackPickTwo) { Exec("1 2 3 4 2 pick"); ExpectIntStack({1, 2, 3, 4, 2}); }
TEST_F(PiCenturyTests, StackPickThree) { Exec("1 2 3 4 3 pick"); ExpectIntStack({1, 2, 3, 4, 1}); }
TEST_F(PiCenturyTests, StackRollZero) { Exec("1 2 3 4 0 roll"); ExpectIntStack({1, 2, 3, 4}); }
TEST_F(PiCenturyTests, StackRollOne) { Exec("1 2 3 4 1 roll"); ExpectIntStack({1, 2, 4, 3}); }
TEST_F(PiCenturyTests, StackRollTwo) { Exec("1 2 3 4 2 roll"); ExpectIntStack({1, 3, 4, 2}); }
TEST_F(PiCenturyTests, StackRollThree) { Exec("1 2 3 4 3 roll"); ExpectIntStack({2, 3, 4, 1}); }
TEST_F(PiCenturyTests, StackDepthEmpty) { Exec("depth"); ExpectIntStack({0}); }
TEST_F(PiCenturyTests, StackDepthOne) { Exec("1 depth"); ExpectIntStack({1, 1}); }
TEST_F(PiCenturyTests, StackDepthThree) { Exec("1 2 3 depth"); ExpectIntStack({1, 2, 3, 3}); }
TEST_F(PiCenturyTests, StackClearAll) { Exec("1 2 3 clear"); ExpectIntStack({}); }
TEST_F(PiCenturyTests, StackDupThenDrop) { Exec("1 2 dup drop"); ExpectIntStack({1, 2}); }
TEST_F(PiCenturyTests, StackSwapThenDrop) { Exec("1 2 3 swap drop"); ExpectIntStack({1, 3}); }
TEST_F(PiCenturyTests, StackDupTwice) { Exec("5 dup dup"); ExpectIntStack({5, 5, 5}); }
TEST_F(PiCenturyTests, StackDup2Quad) { Exec("1 2 3 4 dup2"); ExpectIntStack({1, 2, 3, 4, 3, 4}); }
TEST_F(PiCenturyTests, StackRotTwice) { Exec("9 8 7 rot rot"); ExpectIntStack({7, 9, 8}); }

// --------------------------------------------------------------------------
// Float arithmetic (10)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, FloatAddHalves) { Exec("1.5 1.5 +"); ExpectFloat(3.0f); }
TEST_F(PiCenturyTests, FloatSubHalf) { Exec("2.5 0.5 -"); ExpectFloat(2.0f); }
TEST_F(PiCenturyTests, FloatMulWhole) { Exec("2.0 4.0 *"); ExpectFloat(8.0f); }
TEST_F(PiCenturyTests, FloatDivToHalf) { Exec("9.0 2.0 /"); ExpectFloat(4.5f); }
TEST_F(PiCenturyTests, FloatAddTenths) { Exec("1.1 1.1 +"); ExpectFloat(2.2f); }
TEST_F(PiCenturyTests, FloatDivToQuarter) { Exec("10.0 4.0 /"); ExpectFloat(2.5f); }
TEST_F(PiCenturyTests, FloatSquare) { Exec("3.0 3.0 *"); ExpectFloat(9.0f); }
TEST_F(PiCenturyTests, FloatAddChain) { Exec("1.0 2.0 + 3.0 +"); ExpectFloat(6.0f); }
TEST_F(PiCenturyTests, FloatAddHalfToWhole) { Exec("5.5 0.5 +"); ExpectFloat(6.0f); }
TEST_F(PiCenturyTests, FloatDivBigByFour) { Exec("100.0 4.0 /"); ExpectFloat(25.0f); }

// --------------------------------------------------------------------------
// String operations (8)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, StringConcatWords) { Exec("\"foo\" \"bar\" +"); ExpectString("foobar"); }
TEST_F(PiCenturyTests, StringConcatLetters) { Exec("\"a\" \"b\" +"); ExpectString("ab"); }
TEST_F(PiCenturyTests, ToStrOne) { Exec("1 to_str"); ExpectString("1"); }
TEST_F(PiCenturyTests, ToStrHundred) { Exec("100 to_str"); ExpectString("100"); }
TEST_F(PiCenturyTests, StringConcatWithSpace) {
    Exec("\"Hello\" \" \" + \"World\" +");
    ExpectString("Hello World");
}
TEST_F(PiCenturyTests, ToStrZero) { Exec("0 to_str"); ExpectString("0"); }
TEST_F(PiCenturyTests, StringConcatThreeParts) { Exec("\"x\" \"y\" + \"z\" +"); ExpectString("xyz"); }
TEST_F(PiCenturyTests, StringConcatFourLetters) { Exec("\"ab\" \"cd\" + \"ef\" +"); ExpectString("abcdef"); }

// --------------------------------------------------------------------------
// Compound / mixed expressions (10)
// --------------------------------------------------------------------------

TEST_F(PiCenturyTests, CompoundSumsNotEqual) { Exec("2 3 + 4 5 + =="); ExpectBool(false); }
TEST_F(PiCenturyTests, CompoundMulEquals) { Exec("2 3 * 6 =="); ExpectBool(true); }
TEST_F(PiCenturyTests, CompoundDivEquals) { Exec("10 2 / 5 =="); ExpectBool(true); }
TEST_F(PiCenturyTests, CompoundAddThenMulEquals) { Exec("3 4 + 2 * 14 =="); ExpectBool(true); }
TEST_F(PiCenturyTests, CompoundSubEqualsThenNot) { Exec("5 3 - 2 == not"); ExpectBool(false); }
TEST_F(PiCenturyTests, CompoundLongAddChain) { Exec("1 2 + 3 + 4 + 5 + 6 +"); ExpectInt(21); }
TEST_F(PiCenturyTests, CompoundLongMulChain) { Exec("2 2 * 2 * 2 * 2 *"); ExpectInt(32); }
TEST_F(PiCenturyTests, CompoundLongSubChain) { Exec("100 50 - 25 - 10 -"); ExpectInt(15); }
TEST_F(PiCenturyTests, CompoundMinsSummed) { Exec("3 4 min 5 6 min +"); ExpectInt(8); }
TEST_F(PiCenturyTests, CompoundMaxesSummed) { Exec("3 4 max 5 6 max +"); ExpectInt(10); }
