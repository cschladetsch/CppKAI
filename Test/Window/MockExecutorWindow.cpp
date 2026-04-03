#include "MockExecutorWindow.h"

using namespace std;

namespace kai {

ExecutorWindow::ExecutorWindow() {
    HistoryPos = -1;
    CurrentLanguage = Language::Pi;
    CurrentTab = ConsoleTab::Pi;

    memset(InputBuf, 0, sizeof(InputBuf));
    memset(MultilineInputBuf, 0, sizeof(MultilineInputBuf));

    console_.SetLanguage(CurrentLanguage);
    exec_ = &*console_.GetExecutor();
    reg_ = &console_.GetRegistry();
    tree_ = &console_.GetTree();

    Items[Language::Pi] = vector<string>();
    Items[Language::Rho] = vector<string>();
    History[Language::Pi] = vector<string>();
    History[Language::Rho] = vector<string>();
    DebugLog.push_back("Debugger initialized");

    reg_->AddClass<int>(Label("int"));
    reg_->AddClass<bool>(Label("bool"));
    reg_->AddClass<String>(Label("String"));
}

void ExecutorWindow::ClearLog(Language lang) {
    if (lang == Language::None) {
        lang = CurrentLanguage;
    }
    Items[lang].clear();
    ScrollToBottom = true;
}

void ExecutorWindow::ClearAllLogs() {
    Items[Language::Pi].clear();
    Items[Language::Rho].clear();
    DebugLog.clear();
    DebugLog.push_back("Debugger reset");
    ScrollToBottom = true;
}

void ExecutorWindow::AddLog(const char* fmt, ...) {
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

void ExecutorWindow::SwitchLanguage(Language lang) {
    if (CurrentLanguage != lang) {
        CurrentLanguage = lang;
        console_.SetLanguage(CurrentLanguage);
        exec_ = &*console_.GetExecutor();
        InputBuf[0] = '\0';
        MultilineInputBuf[0] = '\0';
    }
}

void ExecutorWindow::SwitchTab(ConsoleTab tab) {
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

void ExecutorWindow::ExecuteDebugStep() {
    DebugStepCount++;
    AddLog("Step %d", DebugStepCount);

    StringStream st;
    st << "Data Stack Size: " << exec_->GetDataStack()->Size();
    AddLog("%s", st.ToString().c_str());

    if (exec_->GetDataStack()->Size() > 0) {
        AddLog("Stack:");
        for (int i = 0; i < exec_->GetDataStack()->Size(); i++) {
            auto obj = exec_->GetDataStack()->At(i);
            StringStream itemSt;
            itemSt << "  " << i << ": " << obj;
            AddLog("%s", itemSt.ToString().c_str());
        }
    }

    try {
        if (CurrentLanguage == Language::Pi) {
            console_.Execute("dup", Structure::Expression);
            AddLog("Executed 'dup' operation");
        } else {
            AddLog("Current scope information:");
            Object scope = exec_->GetScope();
            if (scope.Exists()) {
                StringStream scopeSt;
                scopeSt << scope;
                AddLog("%s", scopeSt.ToString().c_str());
            } else {
                AddLog("No active scope");
            }
        }
    } catch (Exception::Base& e) {
        AddLog("Debug operation failed: %s", e.ToString().c_str());
    }
}

void ExecutorWindow::ExecCommand(const char* command_line) {
    string cmdWithPrompt = (CurrentLanguage == Language::Pi) ? "Pi> " : "Rho> ";
    cmdWithPrompt += command_line;
    AddLog("%s", cmdWithPrompt.c_str());

    if (CurrentTab == ConsoleTab::Debugger) {
        CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi
                                                       : ConsoleTab::Rho;
    }

    try {
        std::string text = command_line;

        // Add to history if not empty
        if (!text.empty()) {
            History[CurrentLanguage].push_back(text);
        }

        // For testing purposes, we'll simplify shell command handling
        if (!text.empty() && text[0] == '$') {
            AddLog("Shell command execution is disabled in tests");
            return;
        }

        // Process the command through the console
        if (!text.empty()) {
            // Execute the command based on language
            if (CurrentLanguage == Language::Pi) {
                console_.Execute(text.c_str(), Structure::Expression);
            } else {
                // For Rho, determine if it's a statement or expression
                Structure structure = Structure::Statement;
                if (text.find('=') == std::string::npos &&
                    text.find("fun") == std::string::npos &&
                    text.find("if") == std::string::npos &&
                    text.find("for") == std::string::npos &&
                    text.find("while") == std::string::npos) {
                    structure = Structure::Expression;
                }
                console_.Execute(text.c_str(), structure);
            }
        }

        // Report stack contents
        if (exec_->GetDataStack()->Size() > 0) {
            AddLog("Stack:");
            for (auto obj : *exec_->GetDataStack()) {
                StringStream st;
                st << "  " << obj;
                AddLog("%s", st.ToString().c_str());
            }
        } else {
            AddLog("Stack is empty");
        }
    } catch (Exception::Base& e) {
        AddLog("Error: %s", e.what());
    }
}

}  // namespace kai