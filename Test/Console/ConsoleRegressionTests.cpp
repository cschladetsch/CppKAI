#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

#include "KAI/Core/BuiltinTypes/String.h"
#include "KAI/Core/Console.h"
#include "KAI/Core/Object/Object.h"
#include "KAI/Executor/Executor.h"
#include "TestLangCommon.h"

using namespace kai;
using namespace std;

/*
 * CONSOLE / STACK REGRESSION TESTS
 * ---------------------------------
 * Locks down the exact behaviors that regressed during the "Windows app"
 * console polish work (2026-09):
 *
 *   1. PerformBinaryOp must combine operands in push order (A op B, where
 *      A was pushed first) - this matters for String concatenation and
 *      every order-sensitive numeric op (-, /, %, <, >, <=, >=).
 *   2. Console::WriteStackForExecutor / Console::ShowColoredStack must
 *      label and order stack entries consistently: index [0] is always
 *      the top of the stack (the most recently pushed item).
 *   3. Basic push/pop/drop sequences must behave exactly as they have for
 *      years - these are the literal repro sequences from the regression
 *      report.
 *
 * A capture-stdout helper is used for the ShowColoredStack tests since
 * that function writes directly to cout rather than returning a string.
 */

namespace {

// Redirects std::cout for the duration of `fn`, returning what it wrote.
// rang color codes are only emitted when cout is detected as a tty, so
// under gtest (piped output) the captured text is plain - safe to search
// for the "[N]: value" substrings without stripping ANSI escapes.
std::string CaptureStdout(const std::function<void()>& fn) {
    std::ostringstream capture;
    std::streambuf* old = std::cout.rdbuf(capture.rdbuf());
    fn();
    std::cout.rdbuf(old);
    return capture.str();
}

}  // namespace

struct ConsoleRegressionTests : TestLangCommon {
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(Language::Pi);
        reg_->AddClass<int>(Label("Int"));
        reg_->AddClass<bool>(Label("Bool"));
        reg_->AddClass<String>(Label("String"));
        exec_->ClearStacks();
        exec_->ClearContext();
    }

    // Runs Pi source and returns the top of the data stack.
    Object RunTop(const std::string& piSource) {
        console_.Execute(piSource.c_str());
        return data_->Top();
    }
};

// ---------------------------------------------------------------------
// Category A: operand order for order-sensitive binary operations
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, Minus_5_3_Equals2) {
    RunTop("5 3 -");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 2);
}

TEST_F(ConsoleRegressionTests, Minus_3_5_EqualsNeg2) {
    RunTop("3 5 -");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), -2);
}

TEST_F(ConsoleRegressionTests, Divide_10_2_Equals5) {
    RunTop("10 2 /");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 5);
}

TEST_F(ConsoleRegressionTests, Divide_2_10_TruncatesToZero) {
    RunTop("2 10 /");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 0);
}

TEST_F(ConsoleRegressionTests, Modulo_10_3_Equals1) {
    RunTop("10 3 %");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1);
}

TEST_F(ConsoleRegressionTests, Modulo_3_10_Equals3) {
    RunTop("3 10 %");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 3);
}

TEST_F(ConsoleRegressionTests, Less_3_5_True) {
    RunTop("3 5 <");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
}

TEST_F(ConsoleRegressionTests, Less_5_3_False) {
    RunTop("5 3 <");
    ASSERT_FALSE(ConstDeref<bool>(data_->Top()));
}

TEST_F(ConsoleRegressionTests, Greater_5_3_True) {
    RunTop("5 3 >");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
}

TEST_F(ConsoleRegressionTests, Greater_3_5_False) {
    RunTop("3 5 >");
    ASSERT_FALSE(ConstDeref<bool>(data_->Top()));
}

TEST_F(ConsoleRegressionTests, LessOrEquiv_5_5_True) {
    RunTop("5 5 <=");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
}

TEST_F(ConsoleRegressionTests, GreaterOrEquiv_5_5_True) {
    RunTop("5 5 >=");
    ASSERT_TRUE(ConstDeref<bool>(data_->Top()));
}

// ---------------------------------------------------------------------
// Category B: string concatenation - order and type safety
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, StringConcat_ab) {
    RunTop("\"a\" \"b\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "ab");
}

TEST_F(ConsoleRegressionTests, StringConcat_ba_OppositeOrder) {
    RunTop("\"b\" \"a\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "ba");
}

TEST_F(ConsoleRegressionTests, StringConcat_AB) {
    RunTop("\"A\" \"B\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "AB");
}

TEST_F(ConsoleRegressionTests, StringConcat_EmptyStrings) {
    RunTop("\"\" \"\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "");
}

TEST_F(ConsoleRegressionTests, StringConcat_LeftEmpty) {
    RunTop("\"\" \"b\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "b");
}

TEST_F(ConsoleRegressionTests, StringConcat_RightEmpty) {
    RunTop("\"a\" \"\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "a");
}

TEST_F(ConsoleRegressionTests, StringPlusInt_Throws) {
    exec_->ClearStacks();
    EXPECT_THROW(console_.Execute("\"a\" 3 +"), Exception::Base);
}

TEST_F(ConsoleRegressionTests, IntPlusString_Throws) {
    exec_->ClearStacks();
    EXPECT_THROW(console_.Execute("3 \"a\" +"), Exception::Base);
}

// ---------------------------------------------------------------------
// Category C: push / pop / drop sequences (the literal repro cases)
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, Push_SingleInt) {
    RunTop("42");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 42);
}

TEST_F(ConsoleRegressionTests, Push_MultipleInts_OrderPreserved) {
    console_.Execute("1 2 3");
    ASSERT_EQ(data_->Size(), 3u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 3);
}

TEST_F(ConsoleRegressionTests, Drop_RemovesTop) {
    console_.Execute("1 2 drop");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 1);
}

TEST_F(ConsoleRegressionTests, Drop_ThenPushWorks) {
    console_.Execute("1 2 drop 9");
    ASSERT_EQ(data_->Size(), 2u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 9);
}

// The exact sequence from the regression report:
// "a" 3 drop "b" + should leave "ab" on the stack.
TEST_F(ConsoleRegressionTests, Regression_StringDropConcat_LeavesAB) {
    console_.Execute("\"a\" 3 drop \"b\" +");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "ab");
}

// "A" "B" + appended after the above should leave ["ab", "AB"].
TEST_F(ConsoleRegressionTests, Regression_TwoConcats_StackHasBothResults) {
    console_.Execute("\"a\" 3 drop \"b\" + \"A\" \"B\" +");
    ASSERT_EQ(data_->Size(), 2u);
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "AB");
}

TEST_F(ConsoleRegressionTests, Regression_1_2_3_PlusPlus_Equals6) {
    console_.Execute("1 2 3 + +");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 6);
}

TEST_F(ConsoleRegressionTests, Regression_ChainedPushesAndAdds) {
    console_.Execute("1 2 3 + + 1 2 +");
    ASSERT_EQ(data_->Size(), 2u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 3);
}

TEST_F(ConsoleRegressionTests, StackSize_AfterMultiplePushes) {
    console_.Execute("1 2 3 4 5");
    ASSERT_EQ(data_->Size(), 5u);
}

TEST_F(ConsoleRegressionTests, StackSize_ZeroOnFreshExecutor) {
    ASSERT_EQ(data_->Size(), 0u);
}

// ---------------------------------------------------------------------
// Category D: Console::WriteStackForExecutor formatting
// (documents its CURRENT bottom-up numbering convention so a future
// change to it is a deliberate decision, not a silent regression)
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, WriteStack_EmptyStack_IsEmptyString) {
    ASSERT_TRUE(console_.WriteStack().empty());
}

TEST_F(ConsoleRegressionTests, WriteStack_SingleInt_ContainsValue) {
    console_.Execute("42");
    std::string dump = console_.WriteStack().c_str();
    ASSERT_NE(dump.find("42"), std::string::npos);
}

TEST_F(ConsoleRegressionTests, WriteStack_SingleString_IsQuoted) {
    console_.Execute("\"hi\"");
    std::string dump = console_.WriteStack().c_str();
    ASSERT_NE(dump.find("\"hi\""), std::string::npos);
}

TEST_F(ConsoleRegressionTests, WriteStack_TwoInts_BothPresent) {
    console_.Execute("1 2");
    std::string dump = console_.WriteStack().c_str();
    ASSERT_NE(dump.find("1"), std::string::npos);
    ASSERT_NE(dump.find("2"), std::string::npos);
}

TEST_F(ConsoleRegressionTests, WriteStack_ReflectsDrop) {
    console_.Execute("1 2 drop");
    std::string dump = console_.WriteStack().c_str();
    ASSERT_NE(dump.find("1"), std::string::npos);
    ASSERT_EQ(dump.find("2"), std::string::npos);
}

// ---------------------------------------------------------------------
// Category E: Console::ShowColoredStack ordering (the reported bug)
//
// Contract being locked down: [0] is always the TOP of the stack (the
// most recently pushed item), and it is printed on the LAST line, with
// the bottom of the stack printed first - see Console.cpp,
// ShowColoredStack(). If this function's iteration direction or index
// arithmetic regresses again, these tests catch it.
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, ShowColoredStack_EmptyStack_NoOutput) {
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_TRUE(out.empty());
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_SingleInt_LabeledZero) {
    console_.Execute("42");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_NE(out.find("[0]"), std::string::npos);
    ASSERT_NE(out.find("42"), std::string::npos);
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_TwoInts_TopIsIndexZero) {
    console_.Execute("1 2");  // bottom=1, top=2
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });

    auto zeroPos = out.find("[0]");
    auto onePos = out.find("[1]");
    ASSERT_NE(zeroPos, std::string::npos);
    ASSERT_NE(onePos, std::string::npos);

    // [0] must be the line containing the top value (2), [1] the bottom (1).
    std::string zeroLine = out.substr(zeroPos, out.find('\n', zeroPos) - zeroPos);
    std::string oneLine = out.substr(onePos, out.find('\n', onePos) - onePos);
    ASSERT_NE(zeroLine.find('2'), std::string::npos);
    ASSERT_NE(oneLine.find('1'), std::string::npos);
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_BottomPrintedBeforeTop) {
    console_.Execute("1 2");  // bottom=1 (label [1]), top=2 (label [0])
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });

    // Bottom of stack ([1]) must appear on an earlier line than the top
    // of stack ([0]) - i.e. printed first, closest to the previous
    // output, with the freshest value ([0]) printed last, right above
    // the next prompt.
    ASSERT_LT(out.find("[1]"), out.find("[0]"));
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_ThreeItems_LabelsSequential) {
    console_.Execute("10 20 30");  // bottom=10, ..., top=30
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_NE(out.find("[0]"), std::string::npos);
    ASSERT_NE(out.find("[1]"), std::string::npos);
    ASSERT_NE(out.find("[2]"), std::string::npos);
    // Bottom-to-top print order: [2] (10) first, [0] (30) last.
    ASSERT_LT(out.find("[2]"), out.find("[1]"));
    ASSERT_LT(out.find("[1]"), out.find("[0]"));
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_StringTop_IsQuoted) {
    console_.Execute("1 \"top\"");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_NE(out.find("\"top\""), std::string::npos);
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_AfterPlus_ReflectsResult) {
    console_.Execute("\"a\" \"b\" +");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_NE(out.find("[0]"), std::string::npos);
    ASSERT_NE(out.find("\"ab\""), std::string::npos);
    ASSERT_EQ(out.find("[1]"), std::string::npos);  // only one item
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_AfterDrop_OnlyRemainingItem) {
    console_.Execute("1 2 drop");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_NE(out.find("[0]"), std::string::npos);
    ASSERT_NE(out.find("1"), std::string::npos);
    ASSERT_EQ(out.find("[1]"), std::string::npos);
}

TEST_F(ConsoleRegressionTests, ShowColoredStack_RegressionSequence_ABAndAB) {
    // The exact end-to-end sequence from the bug report: stack should
    // read [0]: "AB" (top), [1]: "ab" (bottom).
    console_.Execute("\"a\" 3 drop \"b\" + \"A\" \"B\" +");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });

    auto zeroPos = out.find("[0]");
    auto onePos = out.find("[1]");
    ASSERT_NE(zeroPos, std::string::npos);
    ASSERT_NE(onePos, std::string::npos);
    ASSERT_LT(onePos, zeroPos);  // bottom ("ab") printed first

    std::string zeroLine = out.substr(zeroPos, out.find('\n', zeroPos) - zeroPos);
    std::string oneLine = out.substr(onePos, out.find('\n', onePos) - onePos);
    ASSERT_NE(zeroLine.find("AB"), std::string::npos);
    ASSERT_NE(oneLine.find("ab"), std::string::npos);
}

// ---------------------------------------------------------------------
// Category F: the Pi/Rho language "clear" keyword (Operation::Clear)
//
// NOTE: this only covers the language-level keyword reached via
// Console::Execute()/Process(). It does NOT cover the interactive
// Console::Run() REPL loop's separate "clear"/"cls" screen-clear
// shortcut, which currently shadows this keyword when typed at an
// interactive prompt (see Console.cpp ~line 1641) - that code path has
// no standalone entry point to call from a unit test. Recommend
// extracting the REPL's per-line handling into a testable
// Console::ProcessLine(const std::string&) so that shadowing bug (and
// any future regression of it) can be covered the same way.
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, ClearKeyword_EmptiesStack) {
    console_.Execute("1 2 3 clear");
    ASSERT_EQ(data_->Size(), 0u);
}

TEST_F(ConsoleRegressionTests, ClearKeyword_ThenPushWorks) {
    console_.Execute("1 2 clear 9");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 9);
}

TEST_F(ConsoleRegressionTests, ClearKeyword_OnEmptyStack_NoThrow) {
    exec_->ClearStacks();
    EXPECT_NO_THROW(console_.Execute("clear"));
    ASSERT_EQ(data_->Size(), 0u);
}

TEST_F(ConsoleRegressionTests, ClearKeyword_WriteStack_EmptyAfterClear) {
    console_.Execute("1 2 3 clear");
    ASSERT_TRUE(console_.WriteStack().empty());
}

TEST_F(ConsoleRegressionTests, ClearKeyword_ShowColoredStack_NoOutputAfterClear) {
    console_.Execute("1 2 3 clear");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_TRUE(out.empty());
}

// ---------------------------------------------------------------------
// Category G: broader stack-integrity regression coverage
// ---------------------------------------------------------------------

TEST_F(ConsoleRegressionTests, MultipleDrops_EmptyTheStack) {
    console_.Execute("1 2 3 drop drop drop");
    ASSERT_EQ(data_->Size(), 0u);
}

TEST_F(ConsoleRegressionTests, InterleavedPushDropPush) {
    console_.Execute("1 drop 2 drop 3");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 3);
}

TEST_F(ConsoleRegressionTests, RepeatedStringConcat_BuildsLeftToRight) {
    console_.Execute("\"a\" \"b\" + \"c\" +");
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "abc");
}

TEST_F(ConsoleRegressionTests, RepeatedStringConcat_OrderMattersEachStep) {
    console_.Execute("\"c\" \"b\" + \"a\" +");
    // ("c"+"b")="cb", then ("cb"+"a")="cba"
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "cba");
}

TEST_F(ConsoleRegressionTests, MixedArithmeticAndConcat_IndependentStacksOfType) {
    console_.Execute("1 2 + \"x\" \"y\" +");
    ASSERT_EQ(data_->Size(), 2u);
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "xy");
}

TEST_F(ConsoleRegressionTests, LargeStack_SizeAndTopCorrect) {
    console_.Execute("1 2 3 4 5 6 7 8 9 10");
    ASSERT_EQ(data_->Size(), 10u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 10);
}

TEST_F(ConsoleRegressionTests, LargeStack_ShowColoredStack_BottomFirstTopLast) {
    console_.Execute("1 2 3 4 5");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    // Bottom (1, label [4]) must print before top (5, label [0]).
    ASSERT_LT(out.find("[4]"), out.find("[0]"));
}

TEST_F(ConsoleRegressionTests, DropAll_ThenShowColoredStack_Empty) {
    console_.Execute("1 2 3 4 5 drop drop drop drop drop");
    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    ASSERT_TRUE(out.empty());
}

TEST_F(ConsoleRegressionTests, SequentialSessions_StacksDoNotLeak) {
    console_.Execute("1 2 +");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 3);
    exec_->ClearStacks();
    console_.Execute("\"x\" \"y\" +");
    ASSERT_EQ(data_->Size(), 1u);
    ASSERT_EQ(ConstDeref<String>(data_->Top()), "xy");
}

TEST_F(ConsoleRegressionTests, ExactBugReportTranscript_1_2_3PlusPlus_Then_1_2Plus) {
    // Reproduces the exact multi-line transcript from the bug report:
    //   pi 1 2 3 + +
    //   pi 1 2 +
    //   pi
    // Expected final stack: [6, 3] (bottom-to-top), i.e. top() == 3.
    console_.Execute("1 2 3 + +");
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 6);
    console_.Execute("1 2 +");
    ASSERT_EQ(data_->Size(), 2u);
    ASSERT_EQ(ConstDeref<int>(data_->Top()), 3);

    std::string out = CaptureStdout([&] { console_.ShowColoredStack(); });
    // [0] (top=3) must print after [1] (bottom=6).
    ASSERT_LT(out.find("[1]"), out.find("[0]"));
    std::string zeroLine =
        out.substr(out.find("[0]"), out.find('\n', out.find("[0]")) - out.find("[0]"));
    ASSERT_NE(zeroLine.find('3'), std::string::npos);
}
