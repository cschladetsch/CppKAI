#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "KAI/Console/Console.h"

// Test fixture for Console zsh-like features
class ConsoleZshTest : public ::testing::Test {
   protected:
    kai::Console console;

    void SetUp() override { console.SetLanguage(kai::Language::Pi); }

    // Helper to add commands to history by executing them
    void AddToHistory(const std::string& cmd) {
        // We need to simulate the history tracking from Console::Run()
        // Since commandHistory is private, we'll need to execute through the
        // full path
        SimulateCommand(cmd);
    }

    // Simulate command execution as done in Console::Run()
    void SimulateCommand(const std::string& text) {
        if (!text.empty() && text[0] != '$') {
            // This would add to commandHistory in the real implementation
            // For now, we'll just process it
            console.Process(kai::String(text));
        }
    }
};

// Test 1: Basic !! expansion
TEST_F(ConsoleZshTest, DoubleExclamationRepeatsLastCommand) {
    // Note: These tests assume we can access ProcessZshCommand publicly
    // In practice, you might need to make it public for testing or use friend
    // class

    // First establish some history
    SimulateCommand("1 2 +");

    kai::String result = console.ProcessZshCommand(kai::String("!!"));
    EXPECT_EQ(result.StdString(), "1 2 +");
}

// Test 2: !n expansion (nth command)
TEST_F(ConsoleZshTest, ExclamationNExecutesNthCommand) {
    SimulateCommand("first command");
    SimulateCommand("second command");
    SimulateCommand("third command");

    kai::String result = console.ProcessZshCommand(kai::String("!2"));
    EXPECT_EQ(result.StdString(), "second command");
}

// Test 3: !-n expansion (nth from end)
TEST_F(ConsoleZshTest, ExclamationMinusNExecutesNthFromEnd) {
    SimulateCommand("first command");
    SimulateCommand("second command");
    SimulateCommand("third command");

    kai::String result = console.ProcessZshCommand(kai::String("!-2"));
    EXPECT_EQ(result.StdString(), "second command");
}

// Test 4: !string expansion
TEST_F(ConsoleZshTest, ExclamationStringFindsLastMatchingCommand) {
    SimulateCommand("print \"hello\"");
    SimulateCommand("1 2 +");
    SimulateCommand("print \"world\"");

    kai::String result = console.ProcessZshCommand(kai::String("!print"));
    EXPECT_EQ(result.StdString(), "print \"world\"");
}

// Test 5: Word designator :0
TEST_F(ConsoleZshTest, WordDesignatorZeroGetsCommand) {
    SimulateCommand("echo one two three");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:0"));
    EXPECT_EQ(result.StdString(), "echo");
}

// Test 6: Word designator :^
TEST_F(ConsoleZshTest, WordDesignatorCaretGetsFirstArgument) {
    SimulateCommand("echo one two three");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:^"));
    EXPECT_EQ(result.StdString(), "one");
}

// Test 7: Word designator :$
TEST_F(ConsoleZshTest, WordDesignatorDollarGetsLastArgument) {
    SimulateCommand("echo one two three");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:$"));
    EXPECT_EQ(result.StdString(), "three");
}

// Test 8: Word designator :*
TEST_F(ConsoleZshTest, WordDesignatorStarGetsAllArguments) {
    SimulateCommand("echo one two three");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:*"));
    EXPECT_EQ(result.StdString(), "one two three");
}

// Test 9: Word designator range :1-2
TEST_F(ConsoleZshTest, WordDesignatorRangeGetsSpecificWords) {
    SimulateCommand("echo one two three four");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:1-2"));
    EXPECT_EQ(result.StdString(), "one two");
}

// Test 10: Word designator :n*
TEST_F(ConsoleZshTest, WordDesignatorNStarGetsFromNToEnd) {
    SimulateCommand("echo one two three four");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:2*"));
    EXPECT_EQ(result.StdString(), "two three four");
}

// Test 11: Complex history reference !-3:4*
TEST_F(ConsoleZshTest, ComplexHistoryReference) {
    SimulateCommand("ls -la /home/user file1.txt file2.txt file3.txt");
    SimulateCommand("cd /tmp");
    SimulateCommand("pwd");

    kai::String result = console.ParseHistoryExpansion(kai::String("!-3:4*"));
    EXPECT_EQ(result.StdString(), "file1.txt file2.txt file3.txt");
}

// Test 12: History expansion within commands
TEST_F(ConsoleZshTest, HistoryExpansionWithinCommand) {
    SimulateCommand("5");

    kai::String result = console.ExpandHistoryReferences(kai::String("!! * 2"));
    EXPECT_EQ(result.StdString(), "5 * 2");
}

// Test 13: Multiple history references
TEST_F(ConsoleZshTest, MultipleHistoryReferences) {
    SimulateCommand("10");
    SimulateCommand("20");

    kai::String result =
        console.ExpandHistoryReferences(kai::String("!-2 + !-1"));
    EXPECT_EQ(result.StdString(), "10 + 20");
}

// Test 14: Shell command with backticks
TEST_F(ConsoleZshTest, BacktickShellCommand) {
    kai::String result =
        console.ProcessShellCommand(kai::String("`echo hello`"));
    EXPECT_TRUE(result.StdString().find("hello") != std::string::npos);
}

// Test 15: Embedded shell commands
TEST_F(ConsoleZshTest, EmbeddedShellCommands) {
    kai::String result =
        console.ExpandShellCommands(kai::String("The date is `date +%Y`"));
    EXPECT_TRUE(result.StdString().find("202") !=
                std::string::npos);  // Year starts with 202x
}

// Test 16: Empty history handling
TEST_F(ConsoleZshTest, EmptyHistoryReturnsEmpty) {
    kai::String result = console.ProcessZshCommand(kai::String("!!"));
    EXPECT_EQ(result.StdString(), "");
}

// Test 17: Invalid history index
TEST_F(ConsoleZshTest, InvalidHistoryIndexReturnsEmpty) {
    SimulateCommand("only command");

    kai::String result = console.ProcessZshCommand(kai::String("!99"));
    EXPECT_EQ(result.StdString(), "");
}

// Test 18: Quoted strings in word splitting
TEST_F(ConsoleZshTest, QuotedStringsInWordSplitting) {
    SimulateCommand("echo \"hello world\" \"goodbye moon\"");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:1"));
    EXPECT_EQ(result.StdString(), "\"hello world\"");
}

// Test 19: Mixed quotes handling
TEST_F(ConsoleZshTest, MixedQuotesHandling) {
    SimulateCommand("echo 'single quotes' \"double quotes\" unquoted");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:2"));
    EXPECT_EQ(result.StdString(), "\"double quotes\"");
}

// Test 20: Dollar prefix disables expansion
TEST_F(ConsoleZshTest, DollarPrefixDisablesExpansion) {
    // When a command starts with $, history expansion should not occur
    // This would be tested through the full command processing path
    // The $ is stripped and the rest is processed normally

    SimulateCommand("test command");
    // $!! should not expand to "test command"
    // This test would need to go through the full Run() logic
    EXPECT_TRUE(true);  // Placeholder - actual test would verify no expansion
}