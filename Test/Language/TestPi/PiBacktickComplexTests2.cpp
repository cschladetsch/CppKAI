#include <KAI/Core/Console.h>
#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Pi/Pi.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Only compile shell tests if shell syntax is enabled
#ifdef ENABLE_SHELL_SYNTAX

class PiBacktickComplex2Test : public kai::TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test 11: Shell command with error handling
TEST_F(PiBacktickComplex2Test, ShellErrorHandling) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Try to execute command that might fail, provide default
    console_.Execute("`echo 42` dup 0 > { } { drop 0 } ife");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 42);
}

// Test 12: Recursive-like patterns with shell
TEST_F(PiBacktickComplex2Test, RecursiveShellPattern) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Factorial-like calculation using shell
    console_.Execute(
        "`echo 5` 1 { over 1 > { over * swap 1 - swap } { } ife dup 1 > } "
        "while drop");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 120);  // 5!
}

// Test 13: Shell commands in map operations
TEST_F(PiBacktickComplex2Test, ShellInMapOps) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Create map with shell command keys/values
    console_.Execute(
        "{ `echo foo` `echo 100` `echo bar` `echo 200` } tomap 'mymap #");
    console_.Execute("@mymap `echo foo` at");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
}

// Test 14: Complex continuation chains with shell
TEST_F(PiBacktickComplex2Test, ContinuationChainShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Chain multiple continuations with shell results
    console_.Execute("{ `echo 10` + } { 2 * } { 5 - } 5 rot & swap & swap &");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 25);  // ((5+10)*2)-5
}

// Test 15: Shell with type conversions
TEST_F(PiBacktickComplex2Test, ShellTypeConversions) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Mix numeric and string operations
    console_.Execute(
        "`echo 3.14` `echo 2` * to_str \" = \" + `echo 6.28` to_str +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // Result should be "6 = 6.28" or similar
}

// Test 16: Parallel-like shell operations
TEST_F(PiBacktickComplex2Test, ParallelShellOps) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Execute multiple shells and combine results
    console_.Execute(
        "[ `echo 1` `echo 2` `echo 3` ] [ `echo 4` `echo 5` `echo 6` ] { + } "
        "zip2");
    auto stack = exec->GetDataStack();
    ASSERT_GE(stack->Size(), 1);
    // Should produce array of sums
}

// Test 17: Shell in exception-like patterns
TEST_F(PiBacktickComplex2Test, ShellExceptionPattern) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Simulate try-catch with shell
    console_.Execute(
        "`echo 10` `echo 0` dup 0 == { drop drop \"Error: Division by zero\" } "
        "{ / } ife");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "Error: Division by zero");
}

// Test 18: Complex data structure with shell
TEST_F(PiBacktickComplex2Test, ComplexDataShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Build nested structure with shell commands
    console_.Execute(
        "[ [ `echo 1` `echo 2` ] [ `echo 3` `echo 4` ] ] 0 { 0 { + } fold + } "
        "fold");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);  // 1+2+3+4
}

// Test 19: Shell with memoization pattern
TEST_F(PiBacktickComplex2Test, ShellMemoization) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Cache shell result and reuse
    console_.Execute("`echo 42` dup 'cached # @cached @cached + @cached 3 * +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 210);  // 42 + 42 + 42*3
}

// Test 20: Ultimate shell command test
TEST_F(PiBacktickComplex2Test, UltimateShellTest) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Fibonacci using shell commands for initial values
    console_.Execute("`echo 0` `echo 1` `echo 10` { dup2 + } * drop");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 55);  // 10th Fibonacci number
}
#endif  // ENABLE_SHELL_SYNTAX
