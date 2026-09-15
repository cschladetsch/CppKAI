// Regression coverage for the Rho translator-reuse bug fixed today:
// a long-lived RhoTranslator, held inside MultiLangTranslator and reused
// across every REPL/Execute() call, accumulated state that corrupted later
// translations. Assignment ("a = 2") would throw an Empty Stack exception
// and arithmetic on an identifier ("a+4") would throw InvalidPathname,
// because the bytecode for Store/Retreive/binary-op came out with the wrong
// operand order or malformed identifiers. The fix makes Console build a
// fresh RhoTranslator per Rho translation instead of reusing one, in every
// call site: Console::Process, Console::ExecuteWithExecutor,
// Console::Compile, and the compiler's registered translate function.
//
// This file also adds a batch of genuinely novel Rho tests: multi-statement
// sessions run through repeated Execute() calls (the exact shape that
// exposed the bug), plus coverage of areas the existing 160+ TestRho files
// don't exercise together in one place - string/int/float mixed arithmetic
// across sequential statements, chained assignment, and reassignment.

#include <gtest/gtest.h>

#include "TestLangCommon.h"

using namespace kai;

struct RhoTranslatorReuseTests : TestLangCommon {};

// --- Exact regression cases from the original bug report ------------------

TEST_F(RhoTranslatorReuseTests, AssignNoSpaceBeforeValue) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("a =2", Structure::Program);
    console_.Execute("a", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(2, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, AssignWithSpaces) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("a = 2", Structure::Program);
    console_.Execute("a", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(2, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, ArithmeticOnIdentifierNoSpace) {
    console_.SetLanguage(Language::Rho);
    console_.Execute("a = 2", Structure::Program);
    console_.Execute("a+4", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(6, ConstDeref<int>(result));
}

// --- State-reuse stress: many sequential statements on one console --------
// This is the shape that actually exposed the bug - a single long-lived
// Console object (and therefore a single long-lived RhoTranslator) handling
// many statements one after another, exactly like an interactive REPL
// session or a script run one line at a time.

TEST_F(RhoTranslatorReuseTests, ManySequentialAssignmentsStayIndependent) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("a = 1", Structure::Program);
    console_.Execute("b = 2", Structure::Program);
    console_.Execute("c = 3", Structure::Program);
    console_.Execute("d = 4", Structure::Program);
    console_.Execute("e = 5", Structure::Program);

    console_.Execute("a + b + c + d + e", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(15, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, ReassignmentAcrossCallsUsesLatestValue) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("x = 10", Structure::Program);
    console_.Execute("x = x + 1", Structure::Program);
    console_.Execute("x = x + 1", Structure::Program);
    console_.Execute("x", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(12, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, MixedNoSpaceAndSpacedAssignmentsInterleaved) {
    console_.SetLanguage(Language::Rho);

    // Deliberately alternate spacing styles across calls on the same
    // console - the original bug was sensitive to exactly this pattern.
    console_.Execute("p =1", Structure::Program);
    console_.Execute("q = 2", Structure::Program);
    console_.Execute("r=3", Structure::Program);
    console_.Execute("p+q+r", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(6, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, StoreThenArithmeticThenStoreAgain) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("n = 5", Structure::Program);
    console_.Execute("n+1", Structure::Expression);
    console_.Execute("n = n * 2", Structure::Program);
    console_.Execute("n", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(10, ConstDeref<int>(result));
}

// --- Novel coverage: mixed types and float arithmetic across statements ---

TEST_F(RhoTranslatorReuseTests, FloatAssignmentAndArithmeticSequential) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("f = 1.5", Structure::Program);
    console_.Execute("f = f + 2.5", Structure::Program);
    console_.Execute("f", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<float>());
    EXPECT_FLOAT_EQ(4.0f, ConstDeref<float>(result));
}

TEST_F(RhoTranslatorReuseTests, StringAssignmentAndConcatSequential) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("s = \"foo\"", Structure::Program);
    console_.Execute("s = s + \"bar\"", Structure::Program);
    console_.Execute("s", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<String>());
    EXPECT_EQ(String("foobar"), ConstDeref<String>(result));
}

TEST_F(RhoTranslatorReuseTests, BoolAssignmentSequential) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("flag = true", Structure::Program);
    console_.Execute("flag", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<bool>());
    EXPECT_TRUE(ConstDeref<bool>(result));
}

TEST_F(RhoTranslatorReuseTests, MultipleIndependentSessionsDoNotLeakState) {
    // Two logically separate consoles must never see each other's
    // variables, regardless of how many statements ran on either one.
    Console other;
    other.SetLanguage(Language::Rho);

    console_.SetLanguage(Language::Rho);
    console_.Execute("shared = 100", Structure::Program);

    other.Execute("shared = 999", Structure::Program);
    other.Execute("shared", Structure::Expression);

    auto otherExecutor = other.GetExecutor();
    auto otherStack = otherExecutor->GetDataStack();
    ASSERT_FALSE(otherStack->Empty());
    EXPECT_EQ(999, ConstDeref<int>(otherStack->Top()));

    console_.Execute("shared", Structure::Expression);
    ASSERT_FALSE(data_->Empty());
    EXPECT_EQ(100, ConstDeref<int>(data_->Top()));
}

// --- Repeated failing-then-succeeding calls must not poison the console ---
// A translate failure on one call (e.g. a genuine syntax error) must not
// leave stale state behind that breaks the next, otherwise-valid call.

TEST_F(RhoTranslatorReuseTests, SyntaxErrorThenValidAssignmentRecovers) {
    console_.SetLanguage(Language::Rho);

    // This is intentionally malformed (dangling operator).
    console_.Execute("bad = +", Structure::Program);

    // A perfectly valid statement must still work afterwards.
    console_.Execute("good = 7", Structure::Program);
    console_.Execute("good", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(7, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, RepeatedIdenticalAssignmentIsIdempotent) {
    console_.SetLanguage(Language::Rho);

    for (int i = 0; i < 5; ++i) {
        console_.Execute("z = 42", Structure::Program);
    }
    console_.Execute("z", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(42, ConstDeref<int>(result));
}

TEST_F(RhoTranslatorReuseTests, LongChainOfTenSequentialStatements) {
    console_.SetLanguage(Language::Rho);

    console_.Execute("total = 0", Structure::Program);
    for (int i = 1; i <= 10; ++i) {
        console_.Execute(
            ("total = total + " + std::to_string(i)).c_str(),
            Structure::Program);
    }
    console_.Execute("total", Structure::Expression);

    ASSERT_FALSE(data_->Empty());
    Object result = data_->Top();
    ASSERT_TRUE(result.IsType<int>());
    EXPECT_EQ(55, ConstDeref<int>(result));
}
