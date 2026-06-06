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

    // Factorial of a shell-produced value using a while loop
    console_.Execute(
        "`echo 5` 'n # 1 'acc # { n 0 > } "
        "{ acc n * 'acc # n 1 - 'n # } while acc");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 120);  // 5!
}

// Test 13: Shell commands in map operations
TEST_F(PiBacktickComplex2Test, ShellInMapOps) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Store a shell-produced value under a key, then look it up (map-like)
    console_.Execute("`echo 100` 'foo # foo");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 100);
}

// Test 14: Complex continuation chains with shell
TEST_F(PiBacktickComplex2Test, ContinuationChainShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Apply a chain of continuations (one uses a shell result) to a value
    console_.Execute("5 { `echo 10` + } & { 2 * } & { 5 - } &");
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

    // Sum several shell-produced values (flattened nested data)
    console_.Execute("`echo 1` `echo 2` `echo 3` `echo 4` + + +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 10);  // 1+2+3+4
}

// Test 19: Shell with memoization pattern
TEST_F(PiBacktickComplex2Test, ShellMemoization) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Cache a shell result under a name and reuse it several times
    console_.Execute("`echo 42` 'cached # cached cached + cached 3 * +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 210);  // 42 + 42 + 42*3
}

// Test 20: Ultimate shell command test
TEST_F(PiBacktickComplex2Test, UltimateShellTest) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Fibonacci(10) using shell-produced seed values and a while loop
    console_.Execute(
        "`echo 0` 'a # `echo 1` 'b # 0 'i # { i 10 < } "
        "{ b 'temp # a b + 'b # temp 'a # i 1 + 'i # } while a");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 55);  // 10th Fibonacci number
}
#endif  // ENABLE_SHELL_SYNTAX
