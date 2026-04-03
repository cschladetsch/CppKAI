#include <KAI/Core/Console.h>
#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Pi/Pi.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Only compile shell tests if shell syntax is enabled
#ifdef ENABLE_SHELL_SYNTAX

class PiBacktickShellTest : public kai::TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Basic arithmetic with shell commands
TEST_F(PiBacktickShellTest, SimpleAddition) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("1 `echo 2` + 3 ==");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickShellTest, MultipleShellCommands) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 10` `echo 20` +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 30);
}

TEST_F(PiBacktickShellTest, SubtractionWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 100` `echo 25` -");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 75);
}

TEST_F(PiBacktickShellTest, MultiplicationWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("5 `echo 3` * 2 +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 17);
}

TEST_F(PiBacktickShellTest, DivisionWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 20` `echo 4` /");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

// String operations with shell commands
TEST_F(PiBacktickShellTest, StringConcatenation) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("\"Hello \" `echo World` +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "Hello World");
}

TEST_F(PiBacktickShellTest, StringFromShellCommand) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo \"test string\"`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "test string");
}

// Complex expressions
TEST_F(PiBacktickShellTest, NestedArithmetic) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 5` `echo 3` + `echo 2` *");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 16);  // (5+3)*2
}

TEST_F(PiBacktickShellTest, ModuloWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 17` `echo 5` %");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

// Boolean operations
TEST_F(PiBacktickShellTest, BooleanComparison) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 10` `echo 10` ==");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickShellTest, LessThanComparison) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 5` `echo 10` <");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickShellTest, GreaterThanComparison) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 20` `echo 15` >");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Stack operations with shell commands
TEST_F(PiBacktickShellTest, DupWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 42` dup");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 42);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 42);
}

TEST_F(PiBacktickShellTest, SwapWithShell) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 1` `echo 2` swap");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 2);
    // Stack::At(0) is top of stack, At(1) is second from top
    // After swap: top should be 1, second should be 2
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(0)), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->At(1)), 2);
}

// Complex shell command outputs
TEST_F(PiBacktickShellTest, PipelineCommand) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo \"1 2 3\" | wc -w`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 3);
}

TEST_F(PiBacktickShellTest, MultilineOutput) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Note: multiline output is concatenated, newline is stripped
    console_.Execute("`printf 'line1\\nline2' | head -1`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "line1");
}

// Math expressions
TEST_F(PiBacktickShellTest, MathExpressionEval) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo $((2+3*4))`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 14);
}

TEST_F(PiBacktickShellTest, ComplexCalculation) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 2` `echo 3` + `echo 4` * `echo 5` +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 25);  // ((2+3)*4)+5
}

// Mixed operations
TEST_F(PiBacktickShellTest, MixedStringAndNumber) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Execute the full command as one string
    console_.Execute("`echo 5` `echo 3` + to_str \" items\" +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "8 items");
}

// Edge cases
TEST_F(PiBacktickShellTest, EmptyShellOutput) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Empty echo should produce empty string
    console_.Execute("`echo -n \"\"` \"empty\" +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "empty");
}

#endif  // ENABLE_SHELL_SYNTAX