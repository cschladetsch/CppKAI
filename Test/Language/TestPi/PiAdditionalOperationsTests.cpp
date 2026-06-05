// PiAdditionalOperationsTests.cpp
//
// 50 additional Pi language tests covering integer arithmetic, comparisons,
// boolean logic, combined expressions, stack manipulation, floats and strings.
// Each expectation was validated against the interactive Console before being
// committed here, so these exercise behaviour the executor actually produces.
//
// All tests use the direct execute-then-inspect pattern: a Pi snippet is run on
// the console's executor and the resulting data stack is checked. Stack values
// are addressed with At(0) == top of stack (see Stack::At reverse indexing).

#include "TestLangCommon.h"

using namespace kai;

// Fixture providing small typed assertion helpers over the shared data stack.
struct PiAdditionalOpsTest : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
    }

    // Run a Pi snippet on a freshly cleared stack.
    void Exec(const char *code) {
        exec_->ClearStacks();
        console_.Execute(code);
    }

    void ExpectInt(int expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single int on the stack";
        ASSERT_TRUE(data_->Top().IsType<int>()) << "top of stack is not an int";
        EXPECT_EQ(ConstDeref<int>(data_->Top()), expected);
    }

    void ExpectBool(bool expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single bool on the stack";
        ASSERT_TRUE(data_->Top().IsType<bool>())
            << "top of stack is not a bool";
        EXPECT_EQ(ConstDeref<bool>(data_->Top()), expected);
    }

    void ExpectFloat(float expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single float on the stack";
        ASSERT_TRUE(data_->Top().IsType<float>())
            << "top of stack is not a float";
        EXPECT_FLOAT_EQ(ConstDeref<float>(data_->Top()), expected);
    }

    void ExpectString(const char *expected) {
        ASSERT_EQ(data_->Size(), 1) << "expected a single string on the stack";
        ASSERT_TRUE(data_->Top().IsType<String>())
            << "top of stack is not a string";
        EXPECT_EQ(ConstDeref<String>(data_->Top()), String(expected));
    }

    // Verify the whole integer stack, listed from bottom to top.
    void ExpectIntStack(std::initializer_list<int> bottomToTop) {
        ASSERT_EQ(static_cast<size_t>(data_->Size()), bottomToTop.size())
            << "stack size mismatch";
        size_t position = 0;
        for (int value : bottomToTop) {
            // At(0) is the top, so bottom-to-top position maps to a reverse
            // stack index.
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
// Integer arithmetic (12)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, IntAddition) {
    Exec("5 3 +");
    ExpectInt(8);
}

TEST_F(PiAdditionalOpsTest, IntSubtraction) {
    Exec("10 4 -");
    ExpectInt(6);
}

TEST_F(PiAdditionalOpsTest, IntMultiplication) {
    Exec("6 7 *");
    ExpectInt(42);
}

TEST_F(PiAdditionalOpsTest, IntDivision) {
    Exec("20 4 /");
    ExpectInt(5);
}

TEST_F(PiAdditionalOpsTest, IntModulo) {
    Exec("17 5 %");
    ExpectInt(2);
}

TEST_F(PiAdditionalOpsTest, IntDivKeyword) {
    Exec("17 5 div");
    ExpectInt(3);
}

TEST_F(PiAdditionalOpsTest, NegativeViaSubtraction) {
    Exec("3 10 -");
    ExpectInt(-7);
}

TEST_F(PiAdditionalOpsTest, LeftToRightPrecedence) {
    Exec("2 3 + 4 *");
    ExpectInt(20);
}

TEST_F(PiAdditionalOpsTest, NestedMultiplyAdd) {
    Exec("2 3 4 + *");
    ExpectInt(14);
}

TEST_F(PiAdditionalOpsTest, ChainedDivision) {
    Exec("100 10 / 5 /");
    ExpectInt(2);
}

TEST_F(PiAdditionalOpsTest, ModuloEvenRemainder) {
    Exec("6 2 %");
    ExpectInt(0);
}

TEST_F(PiAdditionalOpsTest, LargeMultiplication) {
    Exec("123 456 *");
    ExpectInt(56088);
}

// --------------------------------------------------------------------------
// Comparisons producing bool (12)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, GreaterTrue) {
    Exec("5 3 >");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, GreaterFalse) {
    Exec("3 5 >");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, LessTrue) {
    Exec("3 5 <");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, LessFalse) {
    Exec("5 3 <");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, EqualTrue) {
    Exec("4 4 ==");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, EqualFalse) {
    Exec("4 5 ==");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, NotEqualTrue) {
    Exec("4 5 !=");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, NotEqualFalse) {
    Exec("4 4 !=");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, GreaterOrEqualBoundary) {
    Exec("5 5 >=");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, GreaterOrEqualFalse) {
    Exec("4 5 >=");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, LessOrEqualTrue) {
    Exec("3 5 <=");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, LessOrEqualBoundary) {
    Exec("5 5 <=");
    ExpectBool(true);
}

// --------------------------------------------------------------------------
// Boolean logic (8)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, AndTrue) {
    Exec("true true and");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, AndFalse) {
    Exec("true false and");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, OrTrue) {
    Exec("false true or");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, OrFalse) {
    Exec("false false or");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, NotTrue) {
    Exec("true not");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, NotFalse) {
    Exec("false not");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, XorTrue) {
    Exec("true false xor");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, XorFalse) {
    Exec("true true xor");
    ExpectBool(false);
}

// --------------------------------------------------------------------------
// Combined comparison + boolean expressions (6)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, CombinedComparisonAnd) {
    Exec("5 3 > 2 1 > and");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, CombinedComparisonOr) {
    Exec("1 2 > 3 1 > or");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, EvenNumberCheck) {
    Exec("6 2 % 0 ==");
    ExpectBool(true);
}

TEST_F(PiAdditionalOpsTest, OddNumberCheck) {
    Exec("7 2 % 0 ==");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, NotOfComparison) {
    Exec("3 4 < not");
    ExpectBool(false);
}

TEST_F(PiAdditionalOpsTest, RangeCheck) {
    Exec("5 1 > 5 10 < and");
    ExpectBool(true);
}

// --------------------------------------------------------------------------
// Stack manipulation (8)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, Dup) {
    Exec("5 dup");
    ExpectIntStack({5, 5});
}

TEST_F(PiAdditionalOpsTest, Drop) {
    Exec("5 6 7 drop");
    ExpectIntStack({5, 6});
}

TEST_F(PiAdditionalOpsTest, Swap) {
    Exec("5 6 swap");
    ExpectIntStack({6, 5});
}

TEST_F(PiAdditionalOpsTest, Over) {
    Exec("5 6 over");
    ExpectIntStack({5, 6, 5});
}

TEST_F(PiAdditionalOpsTest, Rot) {
    Exec("1 2 3 rot");
    ExpectIntStack({2, 3, 1});
}

TEST_F(PiAdditionalOpsTest, Dup2) {
    Exec("1 2 dup2");
    ExpectIntStack({1, 2, 1, 2});
}

TEST_F(PiAdditionalOpsTest, Drop2) {
    Exec("1 2 3 4 drop2");
    ExpectIntStack({1, 2});
}

TEST_F(PiAdditionalOpsTest, DupThenAdd) {
    Exec("5 dup +");
    ExpectInt(10);
}

// --------------------------------------------------------------------------
// Float arithmetic (2)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, FloatAddition) {
    Exec("1.5 2.5 +");
    ExpectFloat(4.0f);
}

TEST_F(PiAdditionalOpsTest, FloatMultiplication) {
    Exec("2.0 3.5 *");
    ExpectFloat(7.0f);
}

// --------------------------------------------------------------------------
// String operations (2)
// --------------------------------------------------------------------------

TEST_F(PiAdditionalOpsTest, StringConcatenation) {
    Exec("\"Hello, \" \"World!\" +");
    ExpectString("Hello, World!");
}

TEST_F(PiAdditionalOpsTest, IntegerToString) {
    Exec("42 to_str");
    ExpectString("42");
}
