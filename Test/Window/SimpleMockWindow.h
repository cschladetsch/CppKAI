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
    char inputBuf[256]{};
    char multilineInputBuf[4096]{};
    std::map<Language, std::vector<std::string>> items;
    std::map<Language, std::vector<std::string>> history;
    Language CurrentLanguage;
    ConsoleTab CurrentTab;
    int HistoryPos;
    bool ScrollToBottom;
    bool IsDebugging;
    int DebugStepCount;
    std::vector<std::string> debugLog;
    int WatchIndex;

    SimpleMockWindow()
        : HistoryPos(-1), CurrentLanguage(Language::Pi), CurrentTab(ConsoleTab::Pi), ScrollToBottom(false),
          IsDebugging(false), DebugStepCount(0), WatchIndex(0)
    {
        memset(inputBuf, 0, sizeof(inputBuf));
        memset(multilineInputBuf, 0, sizeof(multilineInputBuf));

        items[Language::Pi] = std::vector<std::string>();
        items[Language::Rho] = std::vector<std::string>();
        history[Language::Pi] = std::vector<std::string>();
        history[Language::Rho] = std::vector<std::string>();
        debugLog.emplace_back("Debugger initialized");
    }

    void ClearLog(Language lang = Language::None) {
        if (lang == Language::None) {
            lang = CurrentLanguage;
        }
        items[lang].clear();
        ScrollToBottom = true;
    }

    void ClearAllLogs() {
        items[Language::Pi].clear();
        items[Language::Rho].clear();
        debugLog.clear();
        debugLog.emplace_back("Debugger reset");
        ScrollToBottom = true;
    }

    void AddLog(const char* fmt, ...) {
        char buf[1024];
        va_list args = nullptr;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        buf[strlen(buf)] = 0;
        va_end(args);

        if (CurrentTab == ConsoleTab::Debugger) {
            debugLog.emplace_back(buf);
        } else {
            items[CurrentLanguage].emplace_back(buf);
        }
        ScrollToBottom = true;
    }

    void SwitchLanguage(Language lang) {
        if (CurrentLanguage != lang) {
            CurrentLanguage = lang;
            inputBuf[0] = '\0';
            multilineInputBuf[0] = '\0';
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
            inputBuf[0] = '\0';
            multilineInputBuf[0] = '\0';
        }
    }

    void ExecuteDebugStep() {
        DebugStepCount++;
        AddLog("Step %d", DebugStepCount);
        AddLog("Debug step executed");
    }

    void ExecCommand(const char* commandLine)
    {
        std::string cmdWithPrompt =
            (CurrentLanguage == Language::Pi) ? "Pi> " : "Rho> ";
        cmdWithPrompt += commandLine;
        AddLog("%s", cmdWithPrompt.c_str());

        if (CurrentTab == ConsoleTab::Debugger) {
            CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi
                                                           : ConsoleTab::Rho;
        }

        // Simple mock execution
        std::string text = commandLine;

        // Add to history if not empty
        if (!text.empty()) {
            history[CurrentLanguage].push_back(text);
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