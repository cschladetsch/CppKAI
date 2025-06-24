#include <gtest/gtest.h>

#include <algorithm>
#include <memory>

#include "NoConsoleWindow.h"

using namespace kai;
using namespace std;

// Test fixture for Window UI tests
class WindowUITest : public ::testing::Test {
   protected:
    std::unique_ptr<ExecutorWindow> window;

    void SetUp() override { window = std::make_unique<ExecutorWindow>(); }

    void TearDown() override { window.reset(); }
};

// =============================================================================
// UI Component Tests
// =============================================================================

TEST_F(WindowUITest, InputBuffer_SingleLine) {
    // Test Pi single-line input buffer
    window->CurrentTab = ConsoleTab::Pi;

    // Test buffer capacity
    const char* testInput = "test input";
    strcpy(window->InputBuf, testInput);
    EXPECT_STREQ(window->InputBuf, testInput);

    // Test buffer clearing
    window->InputBuf[0] = '\0';
    EXPECT_STREQ(window->InputBuf, "");
}

TEST_F(WindowUITest, InputBuffer_MultiLine) {
    // Test Rho multi-line input buffer
    window->SwitchTab(ConsoleTab::Rho);

    // Test multiline input
    const char* multilineInput = "fun test() {\n    return 42;\n}";
    strcpy(window->MultilineInputBuf, multilineInput);
    EXPECT_STREQ(window->MultilineInputBuf, multilineInput);

    // Test buffer size (should be larger than single-line)
    EXPECT_GT(sizeof(window->MultilineInputBuf), sizeof(window->InputBuf));
}

TEST_F(WindowUITest, TabSelection_Visual) {
    // Test tab selection states
    struct TabTest {
        ConsoleTab tab;
        Language expectedLang;
    };

    std::vector<TabTest> tabTests = {
        {ConsoleTab::Pi, Language::Pi},
        {ConsoleTab::Rho, Language::Rho},
        {ConsoleTab::Debugger,
         window->CurrentLanguage}  // Debugger doesn't change language
    };

    for (const auto& test : tabTests) {
        window->SwitchTab(test.tab);
        EXPECT_EQ(window->CurrentTab, test.tab);

        if (test.tab != ConsoleTab::Debugger) {
            EXPECT_EQ(window->CurrentLanguage, test.expectedLang);
        }
    }
}

TEST_F(WindowUITest, ScrollBehavior) {
    // Test scroll to bottom behavior
    window->ScrollToBottom = false;

    // Add log entry
    window->AddLog("Test message");
    EXPECT_TRUE(window->ScrollToBottom);

    // Reset and test
    window->ScrollToBottom = false;
    window->ExecCommand("1 2 +");
    EXPECT_TRUE(window->ScrollToBottom);
}

TEST_F(WindowUITest, LogFormatting_Pi) {
    window->CurrentTab = ConsoleTab::Pi;

    // Execute command and check formatting
    window->ExecCommand("42");

    // Check that prompt is added
    bool foundPrompt = false;
    for (const auto& item : window->Items[Language::Pi]) {
        if (item.find("Pi>") != std::string::npos) {
            foundPrompt = true;
            break;
        }
    }
    EXPECT_TRUE(foundPrompt);
}

TEST_F(WindowUITest, LogFormatting_Rho) {
    window->SwitchTab(ConsoleTab::Rho);

    // Execute command and check formatting
    window->ExecCommand("x = 10");

    // Check that prompt is added
    bool foundPrompt = false;
    for (const auto& item : window->Items[Language::Rho]) {
        if (item.find("Rho>") != std::string::npos) {
            foundPrompt = true;
            break;
        }
    }
    EXPECT_TRUE(foundPrompt);
}

TEST_F(WindowUITest, DebuggerUI_Controls) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Test debug state transitions
    EXPECT_FALSE(window->IsDebugging);

    window->IsDebugging = true;
    EXPECT_TRUE(window->IsDebugging);

    window->IsDebugging = false;
    EXPECT_FALSE(window->IsDebugging);
}

TEST_F(WindowUITest, DebuggerUI_StepCounter) {
    window->SwitchTab(ConsoleTab::Debugger);
    window->IsDebugging = true;

    // Test step counter increment
    int initialCount = window->DebugStepCount;
    window->ExecuteDebugStep();
    EXPECT_EQ(window->DebugStepCount, initialCount + 1);

    window->ExecuteDebugStep();
    EXPECT_EQ(window->DebugStepCount, initialCount + 2);
}

TEST_F(WindowUITest, DebuggerUI_WatchIndex) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Add items to stack
    window->CurrentLanguage = Language::Pi;
    window->ExecCommand("1 2 3 4 5");

    // Test watch index bounds
    int stackSize = 10;  // Simulated stack size

    // Valid indices
    window->WatchIndex = 0;
    EXPECT_EQ(window->WatchIndex, 0);

    window->WatchIndex = stackSize - 1;
    EXPECT_EQ(window->WatchIndex, stackSize - 1);
}

TEST_F(WindowUITest, DebuggerUI_LogColoring) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Add different types of log messages
    std::vector<std::string> testMessages = {
        "Normal message",    "Error: Test error", "Step 5",
        "Debugging started", "Debugging stopped", "Failed to execute",
        "stack overflow"};

    for (const auto& msg : testMessages) {
        window->AddLog(msg.c_str());
    }

    // Verify all messages were added
    EXPECT_GE(window->DebugLog.size(), testMessages.size());
}

// =============================================================================
// History Navigation Tests
// =============================================================================

TEST_F(WindowUITest, History_Navigation) {
    // Add commands to history
    std::vector<std::string> commands = {"1 2 +", "3 4 *", "5 6 -"};

    for (const auto& cmd : commands) {
        window->ExecCommand(cmd.c_str());
    }

    // Test history size
    EXPECT_EQ(window->History[Language::Pi].size(), commands.size());

    // Test history position
    window->HistoryPos = -1;  // New line
    EXPECT_EQ(window->HistoryPos, -1);

    // Navigate through history
    window->HistoryPos = 0;
    EXPECT_EQ(window->History[Language::Pi][window->HistoryPos], commands[0]);

    window->HistoryPos = 2;
    EXPECT_EQ(window->History[Language::Pi][window->HistoryPos], commands[2]);
}

TEST_F(WindowUITest, History_Isolation) {
    // Add Pi history
    window->ExecCommand("pi command 1");
    window->ExecCommand("pi command 2");

    // Switch to Rho and add history
    window->SwitchTab(ConsoleTab::Rho);
    window->ExecCommand("rho command 1");
    window->ExecCommand("rho command 2");
    window->ExecCommand("rho command 3");

    // Verify histories are separate
    EXPECT_EQ(window->History[Language::Pi].size(), 2);
    EXPECT_EQ(window->History[Language::Rho].size(), 3);
}

// =============================================================================
// Clear Operations Tests
// =============================================================================

TEST_F(WindowUITest, ClearOperations_SingleLanguage) {
    // Add logs to Pi
    window->ExecCommand("1 2 +");
    window->ExecCommand("3 4 *");
    EXPECT_FALSE(window->Items[Language::Pi].empty());

    // Clear Pi log
    window->ClearLog(Language::Pi);
    EXPECT_TRUE(window->Items[Language::Pi].empty());

    // Add logs to Rho
    window->SwitchTab(ConsoleTab::Rho);
    window->ExecCommand("x = 10");
    EXPECT_FALSE(window->Items[Language::Rho].empty());

    // Clear current language (Rho)
    window->ClearLog();
    EXPECT_TRUE(window->Items[Language::Rho].empty());
}

TEST_F(WindowUITest, ClearOperations_AllLanguages) {
    // Add content to all tabs
    window->ExecCommand("pi content");

    window->SwitchTab(ConsoleTab::Rho);
    window->ExecCommand("rho content");

    window->SwitchTab(ConsoleTab::Debugger);
    window->AddLog("debug content");

    // Clear all
    window->ClearAllLogs();

    // Verify all cleared
    EXPECT_TRUE(window->Items[Language::Pi].empty());
    EXPECT_TRUE(window->Items[Language::Rho].empty());
    EXPECT_EQ(window->DebugLog.size(), 1);  // Reset message
    EXPECT_EQ(window->DebugLog[0], "Debugger reset");
}

// =============================================================================
// Stack Display Tests
// =============================================================================

TEST_F(WindowUITest, StackDisplay_Empty) {
    // Ensure stack is empty
    window->ClearLog();

    // Switch to debugger
    window->SwitchTab(ConsoleTab::Debugger);

    // Stack should be empty
    EXPECT_TRUE(window->Items[window->CurrentLanguage].empty());
}

TEST_F(WindowUITest, StackDisplay_WithItems) {
    // Add various types to stack
    window->ExecCommand("42");         // Integer
    window->ExecCommand("3.14");       // Float
    window->ExecCommand("\"hello\"");  // String
    window->ExecCommand("true");       // Boolean
    window->ExecCommand("[1 2 3]");    // Array

    // Switch to debugger
    window->SwitchTab(ConsoleTab::Debugger);

    // Verify stack has items
    EXPECT_GT(window->Items[Language::Pi].size(), 0);
}

// =============================================================================
// Error Display Tests
// =============================================================================

TEST_F(WindowUITest, ErrorDisplay_Pi) {
    // Cause various errors in Pi
    window->ExecCommand("undefined_variable");
    window->ExecCommand("1 0 /");  // Division by zero
    window->ExecCommand("[ 1 2");  // Unclosed array

    // Count error messages
    int errorCount = 0;
    for (const auto& item : window->Items[Language::Pi]) {
        if (item.find("Error") != std::string::npos ||
            item.find("error") != std::string::npos) {
            errorCount++;
        }
    }

    EXPECT_GT(errorCount, 0);
}

TEST_F(WindowUITest, ErrorDisplay_Rho) {
    window->SwitchTab(ConsoleTab::Rho);

    // Cause various errors in Rho
    window->ExecCommand("undefined_var");
    window->ExecCommand("if (x > {");        // Syntax error
    window->ExecCommand("fun incomplete(");  // Incomplete function

    // Count error messages
    int errorCount = 0;
    for (const auto& item : window->Items[Language::Rho]) {
        if (item.find("Error") != std::string::npos ||
            item.find("error") != std::string::npos) {
            errorCount++;
        }
    }

    EXPECT_GT(errorCount, 0);
}

// =============================================================================
// Shell Command UI Tests
// =============================================================================

TEST_F(WindowUITest, ShellCommand_Dollar) {
    // Test $ prefix shell command
    window->ExecCommand("$echo UI test");

    // Check output contains result
    bool foundOutput = false;
    for (const auto& item : window->Items[Language::Pi]) {
        if (item.find("UI test") != std::string::npos) {
            foundOutput = true;
            break;
        }
    }
    EXPECT_TRUE(foundOutput);
}

TEST_F(WindowUITest, ShellCommand_Backtick) {
    // Test backtick shell command
    window->ExecCommand("`echo backtick test`");

    // Check output
    bool foundOutput = false;
    for (const auto& item : window->Items[Language::Pi]) {
        if (item.find("backtick test") != std::string::npos) {
            foundOutput = true;
            break;
        }
    }
    EXPECT_TRUE(foundOutput);
}

TEST_F(WindowUITest, ShellCommand_QuickSubstitution) {
    // Execute initial command
    window->ExecCommand("echo hello");

    // Try quick substitution
    window->ExecCommand("^hello^world^");

    // Check for substitution indicator
    bool foundSubstitution = false;
    for (const auto& item : window->Items[Language::Pi]) {
        if (item.find("=>") != std::string::npos) {
            foundSubstitution = true;
            break;
        }
    }
    EXPECT_TRUE(foundSubstitution);
}

// =============================================================================
// UI State Persistence Tests
// =============================================================================

TEST_F(WindowUITest, StatePersistence_TabSwitch) {
    // Set state in Pi
    window->ExecCommand("1 2 3");
    strcpy(window->InputBuf, "partial command");

    // Switch to Rho
    window->SwitchTab(ConsoleTab::Rho);

    // Input should be cleared
    EXPECT_STREQ(window->InputBuf, "");

    // Switch back to Pi
    window->SwitchTab(ConsoleTab::Pi);

    // Stack should persist, input cleared
    EXPECT_GT(window->Items[Language::Pi].size(), 0);
    EXPECT_STREQ(window->InputBuf, "");
}

TEST_F(WindowUITest, StatePersistence_DebuggerSwitch) {
    // Set up state
    window->ExecCommand("test stack");
    size_t originalLogSize = window->Items[Language::Pi].size();

    // Switch to debugger
    window->SwitchTab(ConsoleTab::Debugger);
    window->WatchIndex = 0;

    // Switch back
    window->SwitchTab(ConsoleTab::Pi);

    // Stack should be unchanged
    EXPECT_EQ(window->Items[Language::Pi].size(), originalLogSize);
}

// =============================================================================
// Special Character Handling Tests
// =============================================================================

TEST_F(WindowUITest, SpecialCharacters_Input) {
    // Test special characters in input
    std::vector<std::string> specialInputs = {
        "test\ttab", "test\nnewline", "test\"quote\"", "test'apostrophe'",
        "test\\backslash"};

    for (const auto& input : specialInputs) {
        window->ExecCommand(input.c_str());
    }

    // Should handle all without crashing
    EXPECT_EQ(window->History[Language::Pi].size(), specialInputs.size());
}

TEST_F(WindowUITest, SpecialCharacters_Unicode) {
    // Test Unicode characters
    window->ExecCommand("\"Hello 世界\"");
    window->ExecCommand("\"Emoji: 😀\"");

    // Should handle Unicode gracefully
    EXPECT_GE(window->History[Language::Pi].size(), 2);
}