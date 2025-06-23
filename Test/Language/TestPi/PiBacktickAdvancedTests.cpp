#include <KAI/Core/Console.h>
#include <KAI/Language/Common/LangCommon.h>
#include <KAI/Language/Pi/Pi.h>
#include <gtest/gtest.h>

#include "TestLangCommon.h"

// Only compile shell tests if shell syntax is enabled
#ifdef ENABLE_SHELL_SYNTAX

class PiBacktickAdvancedTest : public kai::TestLangCommon {
   protected:
    void SetUp() override {
        TestLangCommon::SetUp();
        console_.SetLanguage(kai::Language::Pi);
    }
};

// File system operations
TEST_F(PiBacktickAdvancedTest, FileCount) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Count files in current directory
    console_.Execute("`ls -1 2>/dev/null | wc -l` 0 >");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickAdvancedTest, CurrentDirectory) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`pwd` \"\" !=");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickAdvancedTest, DateCommand) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`date +%Y` 2000 >");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Environment variables
TEST_F(PiBacktickAdvancedTest, HomeDirectory) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo $HOME` \"\" !=");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickAdvancedTest, PathVariable) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo $PATH | grep -c ':'` 0 >");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Text processing
TEST_F(PiBacktickAdvancedTest, WordCount) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 'one two three four five' | wc -w`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiBacktickAdvancedTest, CharacterCount) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo -n 'hello' | wc -c`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

TEST_F(PiBacktickAdvancedTest, SortNumbers) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo -e '3\\n1\\n2' | sort -n | head -1`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 1);
}

// Complex pipelines
TEST_F(PiBacktickAdvancedTest, GrepCount) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo -e 'apple\\nbanana\\napricot' | grep -c '^a'`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 2);
}

TEST_F(PiBacktickAdvancedTest, TailCommand) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo -e '1\\n2\\n3\\n4\\n5' | tail -1`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 5);
}

// Arithmetic in shell
TEST_F(PiBacktickAdvancedTest, ShellArithmeticComplex) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo $((10 * 5 + 3))`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 53);
}

TEST_F(PiBacktickAdvancedTest, BashArrayLength) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`bash -c 'arr=(a b c d); echo ${#arr[@]}'`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 4);
}

// String manipulation
TEST_F(PiBacktickAdvancedTest, StringUppercase) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 'hello' | tr '[:lower:]' '[:upper:]'`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "HELLO");
}

TEST_F(PiBacktickAdvancedTest, StringReplace) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 'hello world' | sed 's/world/universe/'`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "hello universe");
}

TEST_F(PiBacktickAdvancedTest, CutCommand) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 'one,two,three' | cut -d',' -f2`");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<kai::String>(stack->Top()), "two");
}

// Multiple backticks in complex expressions
TEST_F(PiBacktickAdvancedTest, TripleBacktick) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 2` `echo 3` + `echo 5` * `echo 1` +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 26);  // (2+3)*5+1
}

TEST_F(PiBacktickAdvancedTest, NestedCalculations) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`echo 10` `echo 2` / `echo 3` * `echo 7` +");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<int>(stack->Top()), 22);  // (10/2)*3+7 = 15+7
}

// System information
TEST_F(PiBacktickAdvancedTest, UnameInfo) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`uname -s | wc -c` 0 >");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

TEST_F(PiBacktickAdvancedTest, ProcessCount) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    console_.Execute("`ps aux 2>/dev/null | wc -l` 1 >");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    EXPECT_EQ(kai::ConstDeref<bool>(stack->Top()), true);
}

// Error handling
TEST_F(PiBacktickAdvancedTest, NonExistentCommand) {
    console_.SetLanguage(kai::Language::Pi);
    auto exec = console_.GetExecutor();

    // Non-existent command should produce empty output or error
    console_.Execute("`nonexistentcommand123 2>/dev/null` \"\" ==");
    auto stack = exec->GetDataStack();
    ASSERT_EQ(stack->Size(), 1);
    // Either empty string or [shell command failed]
    auto result = kai::ConstDeref<bool>(stack->Top());
    EXPECT_TRUE(result);  // Should be empty
}

#endif  // ENABLE_SHELL_SYNTAX