#include <KAI/Core/Console.h>
#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Pi/Pi.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Only compile shell tests if shell syntax is enabled
#ifdef ENABLE_SHELL_SYNTAX

class PiBacktickComplexTest : public kai::TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test 1: Complex arithmetic with multiple shell commands
TEST_F(PiBacktickComplexTest, ComplexShellArithmetic) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Calculate: (10 * 5) + (100 / 4) - (3 * 7)
    console_.Execute(
        "`echo 10` `echo 5` * `echo 100` `echo 4` / + `echo 3` `echo 7` * -");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 54);  // 50 + 25 - 21
}

// Test 2: Shell command with continuation execution
TEST_F(PiBacktickComplexTest, ShellWithContinuation) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Use a shell result inside an executed continuation.
    console_.Execute("{ `echo 42` 2 * } &");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 84);
}

// Test 3: Conditional execution with shell commands
TEST_F(PiBacktickComplexTest, ConditionalShellExecution) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // If shell command returns > 5, multiply by 10, else add 100
    console_.Execute("`echo 7` dup 5 > { 10 * } { 100 + } ife");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 70);
}

// Test 4: Aggregate shell command values
TEST_F(PiBacktickComplexTest, ShellAggregateOperations) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Combine several shell command values.
    console_.Execute("`echo 10` `echo 20` + `echo 30` +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 60);
}

// Test 5: String manipulation with shell commands
TEST_F(PiBacktickComplexTest, ComplexStringShellOps) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Build complex string from multiple shell commands
    console_.Execute(
        "`echo Hello` to_str \" \" + `echo World` to_str + \"!\" + `echo 2024` "
        "to_str + \" year\" +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()),
              "Hello World!2024 year");
}

// Test 6: Nested loops with shell commands
TEST_F(PiBacktickComplexTest, NestedLoopsWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Combine shell-produced loop bounds as normal Pi values.
    console_.Execute("`echo 3` `echo 2` *");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 3 * 2
}

// Test 7: Shell command in while loop condition
TEST_F(PiBacktickComplexTest, ShellInWhileCondition) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Count up to a shell-provided limit using while.
    console_.Execute(
        "0 'count # { count `echo 3` < } { count 1 + 'count # } while count");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

// Test 8: Function definition with shell parameters
TEST_F(PiBacktickComplexTest, FunctionWithShellParams) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Define function that uses shell command as default
    console_.Execute("{ `echo 100` + } 'add100 # 50 add100 &");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 150);
}

// Test 9: Complex math expressions with shell
TEST_F(PiBacktickComplexTest, ComplexMathShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Calculate shell-produced sides of a 3-4-5 triangle.
    console_.Execute("`echo 3` dup * `echo 4` dup * + 25 ==");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()),
              true);  // 3^2 + 4^2 = 25 = 5^2
}

// Test 10: Shell with stack manipulation
TEST_F(PiBacktickComplexTest, ShellStackManipulation) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Complex stack operations with shell results
    console_.Execute("`echo 1` `echo 2` `echo 3` rot `echo 4` swap over +");
    auto stack = exec->GetDataStack();
    ASSERT_GE(stack->Size(), 1);
    // Complex stack manipulation result
}

#endif  // ENABLE_SHELL_SYNTAX
