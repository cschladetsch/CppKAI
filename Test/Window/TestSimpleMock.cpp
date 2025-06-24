#include <gtest/gtest.h>

#include <memory>

#include "SimpleMockWindow.h"

using namespace kai;
using namespace std;

class SimpleMockTest : public ::testing::Test {
   protected:
    std::unique_ptr<SimpleMockWindow> window;

    void SetUp() override { window = std::make_unique<SimpleMockWindow>(); }

    void TearDown() override { window.reset(); }
};

TEST_F(SimpleMockTest, Initialization) {
    EXPECT_EQ(window->CurrentLanguage, Language::Pi);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Pi);
    EXPECT_EQ(window->HistoryPos, -1);
    EXPECT_FALSE(window->IsDebugging);
    EXPECT_EQ(window->DebugStepCount, 0);
}

TEST_F(SimpleMockTest, TabSwitching) {
    // Switch to Rho
    window->SwitchTab(ConsoleTab::Rho);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Rho);
    EXPECT_EQ(window->CurrentLanguage, Language::Rho);

    // Switch to Debugger
    window->SwitchTab(ConsoleTab::Debugger);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Debugger);

    // Switch back to Pi
    window->SwitchTab(ConsoleTab::Pi);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Pi);
    EXPECT_EQ(window->CurrentLanguage, Language::Pi);
}

TEST_F(SimpleMockTest, CommandExecution) {
    // Execute a command
    window->ExecCommand("1 2 +");

    // Check it was logged
    EXPECT_GE(window->Items[Language::Pi].size(), 1);

    // Check history
    EXPECT_EQ(window->History[Language::Pi].size(), 1);
    EXPECT_EQ(window->History[Language::Pi][0], "1 2 +");
}

TEST_F(SimpleMockTest, Logging) {
    window->AddLog("Test message %d", 42);
    EXPECT_GE(window->Items[Language::Pi].size(), 1);

    // Switch to debugger tab
    window->SwitchTab(ConsoleTab::Debugger);
    window->AddLog("Debug message");
    EXPECT_GE(window->DebugLog.size(), 2);  // Initial message + new one
}

TEST_F(SimpleMockTest, ClearOperations) {
    // Add some logs
    window->AddLog("Message 1");
    window->AddLog("Message 2");
    window->AddLog("Message 3");

    size_t logCount = window->Items[Language::Pi].size();
    EXPECT_GE(logCount, 3);

    // Clear current language log
    window->ClearLog();
    EXPECT_TRUE(window->Items[Language::Pi].empty());

    // Add logs to different tabs
    window->AddLog("Pi log");
    window->SwitchTab(ConsoleTab::Rho);
    window->AddLog("Rho log");
    window->SwitchTab(ConsoleTab::Debugger);
    window->AddLog("Debug log");

    // Clear all
    window->ClearAllLogs();
    EXPECT_TRUE(window->Items[Language::Pi].empty());
    EXPECT_TRUE(window->Items[Language::Rho].empty());
    EXPECT_EQ(window->DebugLog.size(), 1);  // Just the reset message
}

TEST_F(SimpleMockTest, DebugStepping) {
    window->SwitchTab(ConsoleTab::Debugger);
    window->IsDebugging = true;

    // Execute debug steps
    window->ExecuteDebugStep();
    EXPECT_EQ(window->DebugStepCount, 1);

    window->ExecuteDebugStep();
    EXPECT_EQ(window->DebugStepCount, 2);

    // Check debug log has entries
    EXPECT_GT(window->DebugLog.size(), 2);
}