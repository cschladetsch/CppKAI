#include <KAI/Core/Console.h>
#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Pi/Pi.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Only compile shell tests if shell syntax is enabled
#ifdef ENABLE_SHELL_SYNTAX

class PiBacktickNestedTest : public kai::TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// Test nested backticks using escaped backticks
TEST_F(PiBacktickNestedTest, EscapedBacktick) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Test escaping backticks - we want literal backticks in output
    // Using double quotes and escaped backticks
    console_.Execute("`echo \"\\`test\\`\"`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    auto result = kai::ConstDeref<kai::String>(stack->Top());
    // The shell should output: `test`
    EXPECT_EQ(result, "`test`");
}

// Test using $() for command substitution instead of nested backticks
TEST_F(PiBacktickNestedTest, CommandSubstitution) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Use $() for nested command execution
    console_.Execute("`echo $(date +%Y)`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    auto result = kai::ConstDeref<int>(stack->Top());
    // Should be current year
    EXPECT_GT(result, 2020);
}

// Test multiple levels of command substitution
TEST_F(PiBacktickNestedTest, MultiLevelSubstitution) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Use $() for nested command execution with arithmetic
    console_.Execute("`echo $(($(echo 5) + $(echo 3)))`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 8);
}

// Test backtick in string within backtick
TEST_F(PiBacktickNestedTest, BacktickInString) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Our lexer doesn't track quote state, so we can't have literal backticks
    // inside quotes. Use escaped backtick instead.
    console_.Execute("`echo 'test \\` string'`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // Shell will interpret the escaped backtick
    auto result = kai::ConstDeref<kai::String>(stack->Top());
    // With single quotes, the backslash is preserved
    EXPECT_EQ(result, "test \\` string");
}

// Test complex nested expression
TEST_F(PiBacktickNestedTest, ComplexNesting) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Complex expression with command substitution
    console_.Execute("`expr $(echo 2) \\* $(echo 3)`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 6);
}

// Test that actual nested backticks are not supported
TEST_F(PiBacktickNestedTest, ActualNestedBackticksFail) {
    console_.SetLanguage(kai::Language::Pi);

    // Nested backticks should fail to parse correctly
    // The first backtick pair will be parsed as `echo `
    // Then the remaining text will cause a parse error

    // Capture any exceptions that occur
    bool parseSucceeded = true;
    try {
        console_.Execute("`echo `date +%Y``");
    } catch (const std::exception& e) {
        // Expected - nested backticks cause parse errors
        parseSucceeded = false;
    }

    // We expect the parse to fail
    EXPECT_FALSE(parseSucceeded)
        << "Nested backticks should not parse successfully";
}
#endif  // ENABLE_SHELL_SYNTAX
