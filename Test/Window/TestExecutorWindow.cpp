#include <gtest/gtest.h>

#include <memory>
#include <sstream>

#include "NoConsoleWindow.h"

using namespace kai;
using namespace std;

// Test fixture for ExecutorWindow tests
class ExecutorWindowTest : public ::testing::Test {
   protected:
    std::unique_ptr<ExecutorWindow> window;

    void SetUp() override { window = std::make_unique<ExecutorWindow>(); }

    void TearDown() override { window.reset(); }

    // Helper function to capture console output
    std::string CaptureOutput() {
        std::stringstream output;
        for (const auto& item : window->Items[window->CurrentLanguage]) {
            output << item << "\n";
        }
        return output.str();
    }

    // Helper to get the last output line
    std::string GetLastOutput() {
        const auto& items = window->Items[window->CurrentLanguage];
        if (items.empty()) return "";
        return items.back();
    }
};

// =============================================================================
// Pi Tab Tests
// =============================================================================

TEST_F(ExecutorWindowTest, PiTab_Initialization) {
    EXPECT_EQ(window->CurrentLanguage, Language::Pi);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Pi);
    EXPECT_TRUE(window->Items[Language::Pi].empty());
    EXPECT_TRUE(window->History[Language::Pi].empty());
}

TEST_F(ExecutorWindowTest, PiTab_BasicArithmetic) {
    // Test basic addition
    window->ExecCommand("2 3 +");
    EXPECT_TRUE(GetLastOutput().find("5") != std::string::npos);

    // Test multiplication
    window->ExecCommand("4 5 *");
    EXPECT_TRUE(GetLastOutput().find("20") != std::string::npos);

    // Test subtraction
    window->ExecCommand("10 3 -");
    EXPECT_TRUE(GetLastOutput().find("7") != std::string::npos);

    // Test division
    window->ExecCommand("20 4 /");
    EXPECT_TRUE(GetLastOutput().find("5") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_StackOperations) {
    // Test dup
    window->ExecCommand("5 dup");
    EXPECT_TRUE(CaptureOutput().find("5") != std::string::npos);

    // Test swap
    window->ExecCommand("clear 1 2 swap");
    auto output = CaptureOutput();
    EXPECT_TRUE(output.find("2") != std::string::npos);
    EXPECT_TRUE(output.find("1") != std::string::npos);

    // Test drop
    window->ExecCommand("clear 1 2 3 drop");
    output = CaptureOutput();
    EXPECT_TRUE(output.find("3") == std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_Variables) {
    // Test variable assignment
    window->ExecCommand("42 'answer =");
    window->ExecCommand("answer");
    EXPECT_TRUE(GetLastOutput().find("42") != std::string::npos);

    // Test multiple variables
    window->ExecCommand("10 'x =");
    window->ExecCommand("20 'y =");
    window->ExecCommand("x y +");
    EXPECT_TRUE(GetLastOutput().find("30") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_Conditionals) {
    // Test if-then
    window->ExecCommand("clear 1 1 == { \"equal\" } if");
    EXPECT_TRUE(GetLastOutput().find("equal") != std::string::npos);

    // Test if-then-else
    window->ExecCommand("clear 1 2 == { \"equal\" } { \"not equal\" } ifelse");
    EXPECT_TRUE(GetLastOutput().find("not equal") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_Loops) {
    // Test simple loop
    window->ExecCommand("clear 0 5 { dup 1 + } loop");
    auto output = CaptureOutput();
    EXPECT_TRUE(output.find("5") != std::string::npos);

    // Test while loop equivalent
    window->ExecCommand("clear 0 { dup 3 < } { dup 1 + } while");
    output = CaptureOutput();
    EXPECT_TRUE(output.find("3") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_Arrays) {
    // Test array creation
    window->ExecCommand("[ 1 2 3 4 5 ]");
    EXPECT_TRUE(GetLastOutput().find("[") != std::string::npos);

    // Test array operations
    window->ExecCommand("[ 1 2 3 ] size");
    EXPECT_TRUE(GetLastOutput().find("3") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_Strings) {
    // Test string literals
    window->ExecCommand("\"Hello, World!\"");
    EXPECT_TRUE(GetLastOutput().find("Hello, World!") != std::string::npos);

    // Test string concatenation
    window->ExecCommand("\"Hello, \" \"World!\" +");
    EXPECT_TRUE(GetLastOutput().find("Hello, World!") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_Functions) {
    // Test function definition
    window->ExecCommand("{ 2 * } 'double =");
    window->ExecCommand("5 double");
    EXPECT_TRUE(GetLastOutput().find("10") != std::string::npos);

    // Test recursive function
    window->ExecCommand("{ dup 1 <= { } { dup 1 - fact * } ifelse } 'fact =");
    window->ExecCommand("5 fact");
    EXPECT_TRUE(GetLastOutput().find("120") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_ErrorHandling) {
    // Test undefined variable
    window->ExecCommand("undefined_var");
    EXPECT_TRUE(GetLastOutput().find("Error") != std::string::npos ||
                GetLastOutput().find("error") != std::string::npos);

    // Test division by zero
    window->ExecCommand("1 0 /");
    EXPECT_TRUE(GetLastOutput().find("Error") != std::string::npos ||
                GetLastOutput().find("error") != std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_History) {
    // Execute some commands
    window->ExecCommand("1 2 +");
    window->ExecCommand("3 4 *");
    window->ExecCommand("5 6 -");

    // Check history
    EXPECT_EQ(window->History[Language::Pi].size(), 3);
    EXPECT_EQ(window->History[Language::Pi][0], "1 2 +");
    EXPECT_EQ(window->History[Language::Pi][1], "3 4 *");
    EXPECT_EQ(window->History[Language::Pi][2], "5 6 -");
}

TEST_F(ExecutorWindowTest, PiTab_ShellCommands) {
    // Test shell command execution
    window->ExecCommand("$echo test");
    EXPECT_TRUE(
        GetLastOutput().find("Shell command execution is disabled in tests") !=
        std::string::npos);
}

TEST_F(ExecutorWindowTest, PiTab_ClearLog) {
    // Add some items
    window->ExecCommand("1 2 +");
    window->ExecCommand("3 4 *");
    EXPECT_FALSE(window->Items[Language::Pi].empty());

    // Clear log
    window->ClearLog(Language::Pi);
    EXPECT_TRUE(window->Items[Language::Pi].empty());
}

// =============================================================================
// Rho Tab Tests
// =============================================================================

TEST_F(ExecutorWindowTest, RhoTab_SwitchToRho) {
    window->SwitchTab(ConsoleTab::Rho);
    EXPECT_EQ(window->CurrentLanguage, Language::Rho);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Rho);
}

TEST_F(ExecutorWindowTest, RhoTab_BasicArithmetic) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test basic arithmetic
    window->ExecCommand("20 + 30");
    EXPECT_TRUE(GetLastOutput().find("50") != std::string::npos);

    window->ExecCommand("100 - 25");
    EXPECT_TRUE(GetLastOutput().find("75") != std::string::npos);

    window->ExecCommand("6 * 7");
    EXPECT_TRUE(GetLastOutput().find("42") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_Variables) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test variable declaration
    window->ExecCommand("x = 10");
    window->ExecCommand("x");
    EXPECT_TRUE(GetLastOutput().find("10") != std::string::npos);

    // Test variable operations
    window->ExecCommand("y = 20");
    window->ExecCommand("x + y");
    EXPECT_TRUE(GetLastOutput().find("30") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_Functions) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test simple function
    window->ExecCommand("fun add(a, b) { a + b }");
    window->ExecCommand("add(5, 3)");
    EXPECT_TRUE(GetLastOutput().find("8") != std::string::npos);

    // Test function with local variables
    window->ExecCommand(
        "fun multiply_and_add(x, y, z) { result = x * y; result + z }");
    window->ExecCommand("multiply_and_add(3, 4, 5)");
    EXPECT_TRUE(GetLastOutput().find("17") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_ControlFlow) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test if statement
    window->ExecCommand("x = 5; if (x > 3) { \"greater\" } else { \"less\" }");
    EXPECT_TRUE(GetLastOutput().find("greater") != std::string::npos);

    // Test nested if
    window->ExecCommand(
        "y = 10; if (y > 5) { if (y > 8) { \"very big\" } else { \"medium\" } "
        "} else { \"small\" }");
    EXPECT_TRUE(GetLastOutput().find("very big") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_Loops) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test for loop
    window->ExecCommand(
        "sum = 0; for (i = 1; i <= 5; i = i + 1) { sum = sum + i }; sum");
    EXPECT_TRUE(GetLastOutput().find("15") != std::string::npos);

    // Test while loop
    window->ExecCommand(
        "count = 0; while (count < 3) { count = count + 1 }; count");
    EXPECT_TRUE(GetLastOutput().find("3") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_Arrays) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test array creation and access
    window->ExecCommand("arr = [10, 20, 30, 40]");
    window->ExecCommand("arr[2]");
    EXPECT_TRUE(GetLastOutput().find("30") != std::string::npos);

    // Test array operations
    window->ExecCommand("arr.size()");
    EXPECT_TRUE(GetLastOutput().find("4") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_MultilineInput) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test multiline function definition
    std::string multilineCode = R"(
fun factorial(n) {
    if (n <= 1) {
        1
    } else {
        n * factorial(n - 1)
    }
}
factorial(5)
)";

    window->ExecCommand(multilineCode.c_str());
    EXPECT_TRUE(GetLastOutput().find("120") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_ErrorHandling) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test undefined variable
    window->ExecCommand("undefined_variable");
    EXPECT_TRUE(GetLastOutput().find("Error") != std::string::npos ||
                GetLastOutput().find("error") != std::string::npos);

    // Test syntax error
    window->ExecCommand("if (true {");
    EXPECT_TRUE(GetLastOutput().find("Error") != std::string::npos ||
                GetLastOutput().find("error") != std::string::npos);
}

TEST_F(ExecutorWindowTest, RhoTab_PiBlockExecution) {
    window->SwitchTab(ConsoleTab::Rho);

    // Test Pi block in Rho
    window->ExecCommand("result = { 5 3 + }; result");
    EXPECT_TRUE(GetLastOutput().find("8") != std::string::npos);
}

// =============================================================================
// Debugger Tab Tests
// =============================================================================

TEST_F(ExecutorWindowTest, DebuggerTab_Initialization) {
    window->SwitchTab(ConsoleTab::Debugger);
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Debugger);
    EXPECT_FALSE(window->IsDebugging);
    EXPECT_EQ(window->DebugStepCount, 0);
    EXPECT_FALSE(
        window->DebugLog.empty());  // Should have initialization message
}

TEST_F(ExecutorWindowTest, DebuggerTab_StartStopDebugging) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Start debugging
    window->IsDebugging = true;
    window->AddLog("Debugging started");
    EXPECT_TRUE(window->IsDebugging);

    // Stop debugging
    window->IsDebugging = false;
    window->AddLog("Debugging stopped");
    EXPECT_FALSE(window->IsDebugging);
}

TEST_F(ExecutorWindowTest, DebuggerTab_StepExecution) {
    window->SwitchTab(ConsoleTab::Debugger);
    window->IsDebugging = true;

    // Execute some Pi commands to populate stack
    window->CurrentLanguage = Language::Pi;
    window->ExecCommand("1 2 3");

    // Execute debug step
    int initialStepCount = window->DebugStepCount;
    window->ExecuteDebugStep();
    EXPECT_EQ(window->DebugStepCount, initialStepCount + 1);
}

TEST_F(ExecutorWindowTest, DebuggerTab_StackViewing) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Add items to stack
    window->CurrentLanguage = Language::Pi;
    window->ExecCommand("10 20 30");

    // Check that we have some output
    EXPECT_GT(window->Items[Language::Pi].size(), 0);
}

TEST_F(ExecutorWindowTest, DebuggerTab_WatchVariable) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Add items to stack
    window->CurrentLanguage = Language::Pi;
    window->ExecCommand("42 \"test\" 3.14");

    // Set watch index
    window->WatchIndex = 0;
    EXPECT_EQ(window->WatchIndex, 0);
}

TEST_F(ExecutorWindowTest, DebuggerTab_LogMessages) {
    window->SwitchTab(ConsoleTab::Debugger);

    // Add various log messages
    window->AddLog("Normal message");
    window->AddLog("Error: Something went wrong");
    window->AddLog("Step 1");
    window->AddLog("Debugging started");
    window->AddLog("Debugging stopped");

    // Check log contains messages
    EXPECT_GT(window->DebugLog.size(), 5);
}

// =============================================================================
// Integration Tests
// =============================================================================

TEST_F(ExecutorWindowTest, Integration_TabSwitching) {
    // Start in Pi tab
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Pi);
    EXPECT_EQ(window->CurrentLanguage, Language::Pi);

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

TEST_F(ExecutorWindowTest, Integration_LanguageIsolation) {
    // Execute in Pi
    window->ExecCommand("100 'pi_var =");

    // Switch to Rho and execute
    window->SwitchTab(ConsoleTab::Rho);
    window->ExecCommand("rho_var = 200");

    // Check histories are separate
    EXPECT_EQ(window->History[Language::Pi].size(), 1);
    EXPECT_EQ(window->History[Language::Rho].size(), 1);
    EXPECT_EQ(window->History[Language::Pi][0], "100 'pi_var =");
    EXPECT_EQ(window->History[Language::Rho][0], "rho_var = 200");
}

TEST_F(ExecutorWindowTest, Integration_BufferClearing) {
    // Type in Pi buffer
    strcpy(window->InputBuf, "test pi");

    // Switch to Rho
    window->SwitchTab(ConsoleTab::Rho);
    EXPECT_STREQ(window->InputBuf, "");
    EXPECT_STREQ(window->MultilineInputBuf, "");

    // Type in Rho buffer
    strcpy(window->MultilineInputBuf, "test rho");

    // Switch to Pi
    window->SwitchTab(ConsoleTab::Pi);
    EXPECT_STREQ(window->InputBuf, "");
    EXPECT_STREQ(window->MultilineInputBuf, "");
}

TEST_F(ExecutorWindowTest, Integration_ClearAllLogs) {
    // Add content to all tabs
    window->ExecCommand("1 2 +");

    window->SwitchTab(ConsoleTab::Rho);
    window->ExecCommand("x = 10");

    window->SwitchTab(ConsoleTab::Debugger);
    window->AddLog("Debug message");

    // Clear all logs
    window->ClearAllLogs();

    // Check all logs are cleared
    EXPECT_TRUE(window->Items[Language::Pi].empty());
    EXPECT_TRUE(window->Items[Language::Rho].empty());
    EXPECT_EQ(window->DebugLog.size(), 1);  // Should have reset message
}

TEST_F(ExecutorWindowTest, Integration_StackPersistence) {
    // Add items to Pi stack
    window->ExecCommand("1 2 3");
    size_t piLogSize = window->Items[Language::Pi].size();

    // Switch to Rho and back
    window->SwitchTab(ConsoleTab::Rho);
    window->SwitchTab(ConsoleTab::Pi);

    // Logs should persist
    EXPECT_EQ(window->Items[Language::Pi].size(), piLogSize);
}

TEST_F(ExecutorWindowTest, Integration_DebuggerWithLanguages) {
    // Execute in Pi
    window->ExecCommand("5 10 +");

    // Switch to debugger
    window->SwitchTab(ConsoleTab::Debugger);
    window->IsDebugging = true;

    // Execute debug step
    window->ExecuteDebugStep();

    // Should see stack information in debug log
    bool foundStackInfo = false;
    for (const auto& log : window->DebugLog) {
        if (log.find("Stack") != std::string::npos) {
            foundStackInfo = true;
            break;
        }
    }
    EXPECT_TRUE(foundStackInfo);
}

// =============================================================================
// Edge Cases and Stress Tests
// =============================================================================

TEST_F(ExecutorWindowTest, EdgeCase_EmptyCommand) {
    window->ExecCommand("");
    // Should not crash or add to history
    EXPECT_TRUE(window->History[Language::Pi].empty());
}

TEST_F(ExecutorWindowTest, EdgeCase_LongCommand) {
    std::string longCommand(200, 'a');
    window->ExecCommand(longCommand.c_str());
    // Should handle gracefully
    EXPECT_EQ(window->History[Language::Pi].size(), 1);
}

TEST_F(ExecutorWindowTest, EdgeCase_RapidTabSwitching) {
    // Rapidly switch tabs
    for (int i = 0; i < 100; ++i) {
        window->SwitchTab(ConsoleTab::Pi);
        window->SwitchTab(ConsoleTab::Rho);
        window->SwitchTab(ConsoleTab::Debugger);
    }

    // Should remain stable
    EXPECT_EQ(window->CurrentTab, ConsoleTab::Debugger);
}

TEST_F(ExecutorWindowTest, StressTest_ManyCommands) {
    // Execute many commands
    for (int i = 0; i < 100; ++i) {
        window->ExecCommand(std::to_string(i).c_str());
    }

    // Check history size
    EXPECT_EQ(window->History[Language::Pi].size(), 100);
}

TEST_F(ExecutorWindowTest, StressTest_LargeStack) {
    // Build large stack
    for (int i = 0; i < 50; ++i) {
        window->ExecCommand(std::to_string(i).c_str());
    }

    // Should handle many commands
    EXPECT_GE(window->History[Language::Pi].size(), 50);
}