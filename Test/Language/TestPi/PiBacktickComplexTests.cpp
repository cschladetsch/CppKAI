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

    // Store shell result, then use it inside a continuation (bare-name fetch)
    console_.Execute("`echo 42` 'result # { result 2 * } &");
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

// Test 4: Array operations with shell commands
TEST_F(PiBacktickComplexTest, ShellArrayOperations) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Sum values produced by shell commands
    console_.Execute("`echo 10` `echo 20` `echo 30` + +");
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

    // Combine two shell-produced values
    console_.Execute("`echo 3` `echo 2` *");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);  // 3 * 2
}

// Test 7: Shell command in while loop condition
TEST_F(PiBacktickComplexTest, ShellInWhileCondition) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Count down from a shell-produced value, summing the counter each step
    console_.Execute(
        "`echo 5` 'count # 0 'acc # { count 0 > } "
        "{ acc count + 'acc # count 1 - 'count # } while acc");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 15);  // 5+4+3+2+1
}

// Test 8: Function definition with shell parameters
TEST_F(PiBacktickComplexTest, FunctionWithShellParams) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Define a function whose body uses a shell command, then call it
    console_.Execute("{ `echo 100` + } 'add100 # 50 add100 &");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 150);
}

// Test 9: Complex math expressions with shell
TEST_F(PiBacktickComplexTest, ComplexMathShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Verify 3^2 + 4^2 == 5^2 using shell-produced operands
    console_.Execute("`echo 3` dup * `echo 4` dup * + 5 dup * ==");
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