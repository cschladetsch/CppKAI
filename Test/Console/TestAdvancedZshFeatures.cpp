#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "KAI/Console/Console.h"

// Test fixture for advanced Console zsh-like features
class AdvancedConsoleZshTest : public ::testing::Test {
   protected:
    kai::Console console;

    void SetUp() override { console.SetLanguage(kai::Language::Pi); }

    // Helper to simulate command execution
    void SimulateCommand(const std::string& cmd) {
        console.Process(kai::String(cmd));
    }
};

// Test Quick Substitution ^old^new^
TEST_F(AdvancedConsoleZshTest, QuickSubstitution) {
    SimulateCommand("print \"Hello Wrold!\"");

    kai::String result =
        console.ProcessQuickSubstitution(kai::String("^Wrold^World^"));
    EXPECT_EQ(result.StdString(), "print \"Hello World!\"");
}

// Test !$ - last argument
TEST_F(AdvancedConsoleZshTest, LastArgumentShortcut) {
    SimulateCommand("echo first second third");

    kai::String result = console.ProcessZshCommand(kai::String("!$"));
    EXPECT_EQ(result.StdString(), "third");
}

// Test !^ - first argument
TEST_F(AdvancedConsoleZshTest, FirstArgumentShortcut) {
    SimulateCommand("echo first second third");

    kai::String result = console.ProcessZshCommand(kai::String("!^"));
    EXPECT_EQ(result.StdString(), "first");
}

// Test !?string? - search anywhere
TEST_F(AdvancedConsoleZshTest, SearchAnywhere) {
    SimulateCommand("calculate_area circle 5");
    SimulateCommand("print result");
    SimulateCommand("show_circle_info radius");

    kai::String result = console.SearchHistoryAnywhere(kai::String("circle"));
    EXPECT_TRUE(result.StdString().find("circle") != std::string::npos);
}

// Test :h modifier (head - directory)
TEST_F(AdvancedConsoleZshTest, HeadModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("/home/user/file.txt"), 'h');
    EXPECT_EQ(result.StdString(), "/home/user");
}

// Test :t modifier (tail - filename)
TEST_F(AdvancedConsoleZshTest, TailModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("/home/user/file.txt"), 't');
    EXPECT_EQ(result.StdString(), "file.txt");
}

// Test :r modifier (root - remove extension)
TEST_F(AdvancedConsoleZshTest, RootModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("/home/user/file.txt"), 'r');
    EXPECT_EQ(result.StdString(), "/home/user/file");
}

// Test :e modifier (extension)
TEST_F(AdvancedConsoleZshTest, ExtensionModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("/home/user/file.txt"), 'e');
    EXPECT_EQ(result.StdString(), "txt");
}

// Test :u modifier (uppercase)
TEST_F(AdvancedConsoleZshTest, UppercaseModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("hello world"), 'u');
    EXPECT_EQ(result.StdString(), "HELLO WORLD");
}

// Test :l modifier (lowercase)
TEST_F(AdvancedConsoleZshTest, LowercaseModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("HELLO WORLD"), 'l');
    EXPECT_EQ(result.StdString(), "hello world");
}

// Test :q modifier (quote)
TEST_F(AdvancedConsoleZshTest, QuoteModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("hello world"), 'q');
    EXPECT_EQ(result.StdString(), "\"hello world\"");
}

// Test :x modifier (quote each word)
TEST_F(AdvancedConsoleZshTest, QuoteEachWordModifier) {
    kai::String result =
        console.ProcessHistoryModifier(kai::String("hello world test"), 'x');
    EXPECT_EQ(result.StdString(), "\"hello\" \"world\" \"test\"");
}

// Test :s/old/new/ substitution
TEST_F(AdvancedConsoleZshTest, SubstitutionModifier) {
    kai::String result = console.ApplyModifiers(
        kai::String("hello world hello"), "s/hello/goodbye/");
    EXPECT_EQ(result.StdString(), "goodbye world hello");
}

// Test :gs/old/new/ global substitution
TEST_F(AdvancedConsoleZshTest, GlobalSubstitutionModifier) {
    kai::String result = console.ApplyModifiers(
        kai::String("hello world hello"), "gs/hello/goodbye/");
    EXPECT_EQ(result.StdString(), "goodbye world goodbye");
}

// Test chained modifiers
TEST_F(AdvancedConsoleZshTest, ChainedModifiers) {
    kai::String result =
        console.ApplyModifiers(kai::String("/HOME/USER/FILE.TXT"), "l:t:r");
    EXPECT_EQ(result.StdString(), "file");
}

// Test complex substitution patterns
TEST_F(AdvancedConsoleZshTest, ComplexSubstitution) {
    SimulateCommand("process input.csv output.json");

    kai::String result =
        console.ParseHistoryExpansion(kai::String("!!:1:s/csv/tsv/"));
    EXPECT_EQ(result.StdString(), "input.tsv");
}

// Test path manipulation
TEST_F(AdvancedConsoleZshTest, PathManipulation) {
    SimulateCommand("compile /project/src/main.cpp");

    kai::String result = console.ParseHistoryExpansion(kai::String("!!:$:h"));
    EXPECT_EQ(result.StdString(), "/project/src");

    result = console.ParseHistoryExpansion(kai::String("!!:$:t"));
    EXPECT_EQ(result.StdString(), "main.cpp");

    result = console.ParseHistoryExpansion(kai::String("!!:$:t:r"));
    EXPECT_EQ(result.StdString(), "main");
}

// Test edge cases
TEST_F(AdvancedConsoleZshTest, EdgeCases) {
    // Empty history for !$
    kai::String result = console.ProcessZshCommand(kai::String("!$"));
    EXPECT_EQ(result.StdString(), "");

    // No extension for :e
    result = console.ProcessHistoryModifier(kai::String("noextension"), 'e');
    EXPECT_EQ(result.StdString(), "");

    // Root path for :h
    result = console.ProcessHistoryModifier(kai::String("file.txt"), 'h');
    EXPECT_EQ(result.StdString(), ".");
}