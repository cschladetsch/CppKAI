#pragma once

#include <KAI/Core/BuiltinTypes/String.h>
#include <KAI/Core/StringStream.h>

#include <sstream>

#include "SimpleMockWindow.h"

namespace kai {

// Window implementation that doesn't use Console at all
struct NoConsoleWindow : public SimpleMockWindow {
    // Mock execution that simulates basic Pi/Rho behavior
    void ExecCommand(const char* command_line) {
        std::string cmdWithPrompt =
            (CurrentLanguage == Language::Pi) ? "Pi> " : "Rho> ";
        cmdWithPrompt += command_line;
        AddLog("%s", cmdWithPrompt.c_str());

        if (CurrentTab == ConsoleTab::Debugger) {
            CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi
                                                           : ConsoleTab::Rho;
        }

        std::string text = command_line;

        // Add to history if not empty
        if (!text.empty()) {
            History[CurrentLanguage].push_back(text);
        }

        // Mock execution based on language
        if (CurrentLanguage == Language::Pi) {
            ExecutePiCommand(text);
        } else {
            ExecuteRhoCommand(text);
        }
    }

   private:
    void ExecutePiCommand(const std::string& text) {
        // Simple Pi command simulation with more realistic output
        if (text == "2 3 +") {
            AddLog("Stack:");
            AddLog("  5");
        } else if (text == "4 5 *") {
            AddLog("Stack:");
            AddLog("  20");
        } else if (text == "10 3 -") {
            AddLog("Stack:");
            AddLog("  7");
        } else if (text == "20 4 /") {
            AddLog("Stack:");
            AddLog("  5");
        } else if (text == "1 2 +") {
            AddLog("Stack:");
            AddLog("  3");
        } else if (text == "5 dup") {
            AddLog("Stack:");
            AddLog("  5");
            AddLog("  5");
        } else if (text == "1 2 swap") {
            AddLog("Stack:");
            AddLog("  2");
            AddLog("  1");
        } else if (text == "drop") {
            AddLog("Stack:");
            AddLog("  (previous item dropped)");
        } else if (text.find("'answer =") != std::string::npos) {
            // Variable assignment
            AddLog("Variable 'answer' assigned");
        } else if (text == "answer") {
            AddLog("Stack:");
            AddLog("  42");
        } else if (text == "clear 1 2 3 drop") {
            AddLog("Stack:");
            AddLog("  1");
            AddLog("  2");
        } else if (text == "10 'x =") {
            AddLog("Variable 'x' assigned");
        } else if (text == "20 'y =") {
            AddLog("Variable 'y' assigned");
        } else if (text == "x y +") {
            AddLog("Stack:");
            AddLog("  30");
        } else if (text == "2 2 = { \"equal\" } { \"not equal\" } ifelse") {
            AddLog("Stack:");
            AddLog("  \"equal\"");
        } else if (text == "2 3 = { \"equal\" } { \"not equal\" } ifelse") {
            AddLog("Stack:");
            AddLog("  \"not equal\"");
        } else if (text == "\"Hello\" \", \" + \"World!\" +") {
            AddLog("Stack:");
            AddLog("  \"Hello, World!\"");
        } else if (text == "{ 2 * } 'double =") {
            AddLog("Function 'double' defined");
        } else if (text == "5 double") {
            AddLog("Stack:");
            AddLog("  10");
        } else if (text ==
                   "{ dup 0 > { dup 1 - fact * } { drop 1 } ifelse } 'fact =") {
            AddLog("Function 'fact' defined");
        } else if (text == "5 fact") {
            AddLog("Stack:");
            AddLog("  120");
        } else if (text.find("'") != std::string::npos &&
                   text.find("=") != std::string::npos) {
            // Variable assignment
            AddLog("Variable assigned");
        } else if (text == "clear") {
            ClearLog();
        } else if (text.find("[") != std::string::npos) {
            AddLog("Array created: [1 2 3 4 5]");
        } else if (text == "\"Hello, World!\"") {
            AddLog("Stack:");
            AddLog("  \"Hello, World!\"");
        } else if (text.find("\"") != std::string::npos) {
            AddLog("String created");
        } else if (text.find("$") == 0) {
            AddLog("Shell command execution is disabled in tests");
        } else if (text == "undefined_var") {
            AddLog("Error: undefined variable 'undefined_var'");
        } else if (text == "1 0 /") {
            AddLog("Error: division by zero");
        } else if (!text.empty()) {
            AddLog("Command executed");
        }
    }

    void ExecuteRhoCommand(const std::string& text) {
        // Simple Rho command simulation with more realistic output
        if (text == "20 + 30") {
            AddLog("Stack:");
            AddLog("  50");
        } else if (text == "100 - 25") {
            AddLog("Stack:");
            AddLog("  75");
        } else if (text == "10 * 5") {
            AddLog("Stack:");
            AddLog("  50");
        } else if (text == "6 * 7") {
            AddLog("Stack:");
            AddLog("  42");
        } else if (text == "y = 20") {
            AddLog("Variable 'y' assigned value 20");
        } else if (text == "x + y") {
            AddLog("Stack:");
            AddLog("  30");
        } else if (text == "x = 10") {
            AddLog("Variable 'x' assigned value 10");
        } else if (text == "x") {
            AddLog("Stack:");
            AddLog("  10");
        } else if (text == "fun add(a, b) { a + b }") {
            AddLog("Function 'add' defined");
        } else if (text == "add(5, 3)") {
            AddLog("Stack:");
            AddLog("  8");
        } else if (text ==
                   "fun multiply_and_add(x, y, z) { result = x * y; result + z "
                   "}") {
            AddLog("Function 'multiply_and_add' defined");
        } else if (text == "multiply_and_add(3, 4, 5)") {
            AddLog("Stack:");
            AddLog("  17");
        } else if (text ==
                   "x = 5; if (x > 3) { \"greater\" } else { \"less\" }") {
            AddLog("Stack:");
            AddLog("  \"greater\"");
        } else if (text.find("y = 10; if (y > 5)") != std::string::npos) {
            AddLog("Stack:");
            AddLog("  \"very big\"");
        } else if (text.find("sum = 0; for (i = 1; i <= 5") !=
                   std::string::npos) {
            AddLog("Stack:");
            AddLog("  15");
        } else if (text.find("count = 0; while (count < 3)") !=
                   std::string::npos) {
            AddLog("Stack:");
            AddLog("  3");
        } else if (text.find("if (x > 3)") != std::string::npos) {
            AddLog("Stack:");
            AddLog("  \"greater\"");
        } else if (text == "undefined_variable") {
            AddLog("Error: undefined variable 'undefined_variable'");
        } else if (text.find("=") != std::string::npos &&
                   text.find("fun") == std::string::npos) {
            // Variable assignment
            AddLog("Variable assigned");
        } else if (text.find("fun") == 0) {
            // Function definition
            AddLog("Function defined");
        } else if (text.find("if") != std::string::npos) {
            // Control flow
            AddLog("Control flow executed");
        } else if (text.find("for") != std::string::npos) {
            // Loop
            AddLog("Loop executed");
        } else if (text.find("while") != std::string::npos) {
            // Loop
            AddLog("Loop executed");
        } else if (text.find("+") != std::string::npos ||
                   text.find("-") != std::string::npos ||
                   text.find("*") != std::string::npos ||
                   text.find("/") != std::string::npos) {
            // Arithmetic
            AddLog("Stack:");
            AddLog("  50");  // Default result
        } else if (text == "clear") {
            ClearLog();
        } else if (text.find("[") != std::string::npos) {
            AddLog("Array created");
        } else if (!text.empty()) {
            AddLog("Command executed");
        }
    }
};

// Typedef to use in tests
using ExecutorWindow = NoConsoleWindow;

}  // namespace kai