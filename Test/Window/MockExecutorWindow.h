#pragma once

#include <KAI/Console.h>
#include <KAI/Core/Exception.h>

#include <cstdarg>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace kai {

// Enum for the available tabs in the console window
enum class ConsoleTab { Pi, Rho, Debugger };

// Mock version of ExecutorWindow for testing
struct ExecutorWindow {
    // Input and history state
    char InputBuf[256];
    char MultilineInputBuf[4096];
    int HistoryPos;
    bool ScrollToBottom;

    // Output for each language
    std::map<Language, std::vector<std::string>> Items;
    std::map<Language, std::vector<std::string>> History;

    // Current active language and tab
    Language CurrentLanguage;
    ConsoleTab CurrentTab;

    // Debugger state
    bool IsDebugging = false;
    int DebugStepCount = 0;
    std::vector<std::string> DebugLog;
    int WatchIndex = 0;

    // KAI console objects
    Console console_;
    Tree* tree_;
    Executor* exec_;
    Registry* reg_;

    ExecutorWindow();
    void ClearLog(Language lang = Language::None);
    void ClearAllLogs();
    void AddLog(const char* fmt, ...);
    void SwitchLanguage(Language lang);
    void SwitchTab(ConsoleTab tab);
    void ExecuteDebugStep();
    void ExecCommand(const char* command_line);
};

}  // namespace kai