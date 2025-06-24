#pragma once

#include <cstdarg>
#include <cstring>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace kai {

enum class Language { None = 0, Pi = 1, Rho = 2 };

enum class ConsoleTab { Pi = 0, Rho = 1, Debugger = 2 };

struct SimpleMockWindow {
    char InputBuf[256];
    char MultilineInputBuf[4096];
    std::map<Language, std::vector<std::string>> Items;
    std::map<Language, std::vector<std::string>> History;
    Language CurrentLanguage;
    ConsoleTab CurrentTab;
    int HistoryPos;
    bool ScrollToBottom;
    bool IsDebugging;
    int DebugStepCount;
    std::vector<std::string> DebugLog;
    int WatchIndex;

    SimpleMockWindow() {
        HistoryPos = -1;
        CurrentLanguage = Language::Pi;
        CurrentTab = ConsoleTab::Pi;
        ScrollToBottom = false;
        IsDebugging = false;
        DebugStepCount = 0;
        WatchIndex = 0;

        memset(InputBuf, 0, sizeof(InputBuf));
        memset(MultilineInputBuf, 0, sizeof(MultilineInputBuf));

        Items[Language::Pi] = std::vector<std::string>();
        Items[Language::Rho] = std::vector<std::string>();
        History[Language::Pi] = std::vector<std::string>();
        History[Language::Rho] = std::vector<std::string>();
        DebugLog.push_back("Debugger initialized");
    }

    void ClearLog(Language lang = Language::None) {
        if (lang == Language::None) {
            lang = CurrentLanguage;
        }
        Items[lang].clear();
        ScrollToBottom = true;
    }

    void ClearAllLogs() {
        Items[Language::Pi].clear();
        Items[Language::Rho].clear();
        DebugLog.clear();
        DebugLog.push_back("Debugger reset");
        ScrollToBottom = true;
    }

    void AddLog(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        buf[strlen(buf)] = 0;
        va_end(args);

        if (CurrentTab == ConsoleTab::Debugger) {
            DebugLog.push_back(buf);
        } else {
            Items[CurrentLanguage].push_back(buf);
        }
        ScrollToBottom = true;
    }

    void SwitchLanguage(Language lang) {
        if (CurrentLanguage != lang) {
            CurrentLanguage = lang;
            InputBuf[0] = '\0';
            MultilineInputBuf[0] = '\0';
        }
    }

    void SwitchTab(ConsoleTab tab) {
        if (CurrentTab != tab) {
            CurrentTab = tab;
            if (tab == ConsoleTab::Pi) {
                SwitchLanguage(Language::Pi);
            } else if (tab == ConsoleTab::Rho) {
                SwitchLanguage(Language::Rho);
            }
            InputBuf[0] = '\0';
            MultilineInputBuf[0] = '\0';
        }
    }

    void ExecuteDebugStep() {
        DebugStepCount++;
        AddLog("Step %d", DebugStepCount);
        AddLog("Debug step executed");
    }

    void ExecCommand(const char* command_line) {
        std::string cmdWithPrompt =
            (CurrentLanguage == Language::Pi) ? "Pi> " : "Rho> ";
        cmdWithPrompt += command_line;
        AddLog("%s", cmdWithPrompt.c_str());

        if (CurrentTab == ConsoleTab::Debugger) {
            CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi
                                                           : ConsoleTab::Rho;
        }

        // Simple mock execution
        std::string text = command_line;

        // Add to history if not empty
        if (!text.empty()) {
            History[CurrentLanguage].push_back(text);
        }

        // Mock some simple responses
        if (text == "1 2 +") {
            AddLog("Stack: 3");
        } else if (text == "clear") {
            ClearLog();
        } else if (!text.empty()) {
            AddLog("Command executed: %s", text.c_str());
        }
    }
};

}  // namespace kai