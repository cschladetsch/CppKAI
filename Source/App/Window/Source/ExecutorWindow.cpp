#include <KAI/Console.h>
#include <KAI/Core/Exception.h>
#include <KAI/ImGui/imgui.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

using namespace std;

KAI_BEGIN

// Enum for the available tabs in the console window
enum class ConsoleTab {
    Pi,
    Rho,
    Debugger
};

// A tabbed console with Pi, Rho, and Debugger tabs
struct ExecutorWindow {
    // Input and history state
    char InputBuf[256];
    int HistoryPos;  // -1: new line, 0..History.Size-1 browsing history.
    bool ScrollToBottom;

    // Output for each language
    map<Language, vector<string>> Items;
    map<Language, vector<string>> History;
    
    // Current active language and tab
    Language CurrentLanguage;
    ConsoleTab CurrentTab;
    
    // Debugger state
    bool IsDebugging = false;
    int DebugStepCount = 0;
    vector<string> DebugLog;
    int WatchIndex = 0;
    
    // KAI console objects
    Console console_;
    Tree* tree_;
    Executor* exec_;
    Registry* reg_;

    ExecutorWindow() {
        HistoryPos = -1;
        CurrentLanguage = Language::Pi;
        CurrentTab = ConsoleTab::Pi;

        // Initialize console with Pi language by default
        console_.SetLanguage(CurrentLanguage);
        exec_ = &*console_.GetExecutor();
        reg_ = &console_.GetRegistry();
        tree_ = &console_.GetTree();

        // Initialize language-specific logs
        Items[Language::Pi] = vector<string>();
        Items[Language::Rho] = vector<string>();
        
        // Initialize language-specific history
        History[Language::Pi] = vector<string>();
        History[Language::Rho] = vector<string>();
        
        // Initialize debugger log
        DebugLog.push_back("Debugger initialized");
        
        // Register core types
        reg_->AddClass<int>(Label("int"));
        reg_->AddClass<bool>(Label("bool"));
        reg_->AddClass<String>(Label("String"));
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
            console_.SetLanguage(CurrentLanguage);
            exec_ = &*console_.GetExecutor();
            
            // Clear the input buffer when switching languages
            InputBuf[0] = '\0';
        }
    }
    
    void SwitchTab(ConsoleTab tab) {
        if (CurrentTab != tab) {
            CurrentTab = tab;
            
            // If switching to a language tab, ensure the corresponding language is set
            if (tab == ConsoleTab::Pi) {
                SwitchLanguage(Language::Pi);
            } 
            else if (tab == ConsoleTab::Rho) {
                SwitchLanguage(Language::Rho);
            }
            
            // Clear the input buffer when switching tabs
            InputBuf[0] = '\0';
        }
    }

    void Draw(const char* title, bool* p_open) {
        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open)) {
            ImGui::End();
            return;
        }

        // Create tab selection buttons
        if (ImGui::Button("Pi")) {
            SwitchTab(ConsoleTab::Pi);
        }
        ImGui::SameLine();
        
        if (ImGui::Button("Rho")) {
            SwitchTab(ConsoleTab::Rho);
        }
        ImGui::SameLine();
        
        if (ImGui::Button("Debugger")) {
            SwitchTab(ConsoleTab::Debugger);
        }
        
        // Show current tab
        ImGui::SameLine();
        std::string currentTabName;
        switch (CurrentTab) {
            case ConsoleTab::Pi: currentTabName = "Pi"; break;
            case ConsoleTab::Rho: currentTabName = "Rho"; break;
            case ConsoleTab::Debugger: currentTabName = "Debugger"; break;
        }
        ImGui::Text("Current: %s", currentTabName.c_str());
        
        ImGui::Separator();
        
        // Draw content based on current tab
        if (CurrentTab == ConsoleTab::Debugger) {
            DrawDebuggerContent();
        } else {
            DrawConsoleContent();
        }

        ImGui::End();
    }
    
    void DrawConsoleContent() {
        // Control buttons
        if (ImGui::SmallButton("Clear")) {
            ClearLog();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear Stack")) {
            exec_->ClearStacks();
            AddLog("Stack cleared");
        }

        ImGui::Separator();

        // Output region
        ImGui::BeginChild("ScrollingRegion",
                        ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()),
                        false, ImGuiWindowFlags_HorizontalScrollbar);
        
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::Selectable("Clear")) ClearLog();
            ImGui::EndPopup();
        }

        // Display every line as a separate entry
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing
        
        const auto& currentItems = Items[CurrentLanguage];
        for (size_t i = 0; i < currentItems.size(); i++) {
            const string& item = currentItems[i];
            ImGui::TextUnformatted(item.c_str());
        }

        if (ScrollToBottom) ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
        
        // Show language indicator in the input field
        string inputLabel = (CurrentLanguage == Language::Pi) ? "Pi>" : "Rho>";
        
        if (ImGui::InputText(inputLabel.c_str(), InputBuf, sizeof(InputBuf), input_text_flags)) {
            char* input_end = InputBuf + strlen(InputBuf);
            while (input_end > InputBuf && input_end[-1] == ' ') input_end--;

            *input_end = 0;

            if (InputBuf[0]) {
                // Add to history
                History[CurrentLanguage].push_back(InputBuf);
                
                // Execute the command
                ExecCommand(InputBuf);
            }
            
            strcpy(InputBuf, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
    }
    
    void DrawDebuggerContent() {
        ImGui::BeginChild("DebuggerControls", ImVec2(0, 50), true);
        
        if (ImGui::Button(IsDebugging ? "Stop" : "Start Debugging")) {
            IsDebugging = !IsDebugging;
            if (IsDebugging) {
                AddLog("Debugging started");
            } else {
                AddLog("Debugging stopped");
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Step") && IsDebugging) {
            ExecuteDebugStep();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Clear")) {
            DebugLog.clear();
            DebugLog.push_back("Debugger log cleared");
        }
        
        ImGui::EndChild();
        
        // Split view with stack/context view on the left, log on the right
        ImGui::Columns(2, "debugger_columns");
        
        // Left column - Stack & Context
        ImGui::BeginChild("StackView", ImVec2(0, 200), true);
        ImGui::Text("Data Stack");
        
        if (exec_->GetDataStack()->Size() > 0) {
            for (int i = 0; i < exec_->GetDataStack()->Size(); i++) {
                auto obj = exec_->GetDataStack()->At(i);
                StringStream st;
                st << i << ": " << obj;
                
                if (ImGui::Selectable(st.ToString().c_str(), WatchIndex == i)) {
                    WatchIndex = i;
                }
            }
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Stack is empty");
        }
        
        ImGui::EndChild();
        
        ImGui::BeginChild("ContextView", ImVec2(0, 0), true);
        ImGui::Text("Context");
        
        // Show information about the currently selected variable if available
        if (exec_->GetDataStack()->Size() > 0 && WatchIndex >= 0 && WatchIndex < exec_->GetDataStack()->Size()) {
            auto obj = exec_->GetDataStack()->At(WatchIndex);
            
            ImGui::Separator();
            ImGui::Text("Watch - Stack Item %d", WatchIndex);
            
            // Display type information safely
            int typeNum = obj.GetTypeNumber().ToInt();
            ImGui::Text("Type: %d", typeNum);
            
            // Show string representation
            StringStream st;
            st << obj;
            ImGui::TextWrapped("Value: %s", st.ToString().c_str());
            
            // Show object information
            ImGui::Text("Valid: %s", obj.Exists() ? "Yes" : "No");
        }
        
        ImGui::EndChild();
        
        ImGui::NextColumn();
        
        // Right column - Debug Log
        ImGui::BeginChild("DebugLog", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
        
        for (const auto& log : DebugLog) {
            ImGui::TextUnformatted(log.c_str());
        }
        
        if (ScrollToBottom) ImGui::SetScrollHereY(1.0f);
        
        ImGui::PopStyleVar();
        ImGui::EndChild();
        
        ImGui::Columns(1);
    }
    
    void ExecuteDebugStep() {
        DebugStepCount++;
        AddLog("Step %d", DebugStepCount);
        
        // Show current executor state
        StringStream st;
        st << "Data Stack Size: " << exec_->GetDataStack()->Size();
        AddLog("%s", st.ToString().c_str());
        
        // Show all stack items
        if (exec_->GetDataStack()->Size() > 0) {
            AddLog("Stack:");
            for (int i = 0; i < exec_->GetDataStack()->Size(); i++) {
                auto obj = exec_->GetDataStack()->At(i);
                StringStream itemSt;
                itemSt << "  " << i << ": " << obj;
                AddLog("%s", itemSt.ToString().c_str());
            }
        }
        
        // Execute a simple operation to see the result (increment step counter)
        try {
            // Try to execute a simple Pi operation to see stack changes
            if (CurrentLanguage == Language::Pi) {
                console_.Execute("dup", Structure::Expression);
                AddLog("Executed 'dup' operation");
            }
            else {
                // For Rho, show scope information instead
                AddLog("Current scope information:");
                Object scope = exec_->GetScope();
                if (scope.Exists()) {
                    StringStream scopeSt;
                    scopeSt << scope;
                    AddLog("%s", scopeSt.ToString().c_str());
                }
                else {
                    AddLog("No active scope");
                }
            }
        }
        catch (Exception::Base& e) {
            AddLog("Debug operation failed: %s", e.ToString().c_str());
        }
    }

    void ExecCommand(const char* command_line) {
        // Add the command to the log first
        string cmdWithPrompt = (CurrentLanguage == Language::Pi) ? "Pi> " : "Rho> ";
        cmdWithPrompt += command_line;
        AddLog("%s", cmdWithPrompt.c_str());
        
        // If in debugger tab, automatically switch to the corresponding language tab
        if (CurrentTab == ConsoleTab::Debugger) {
            CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi : ConsoleTab::Rho;
        }
        
        // Execute the command
        try {
            Structure structure = (CurrentLanguage == Language::Pi) 
                ? Structure::Expression 
                : Structure::Statement;
                
            console_.Execute(command_line, structure);
            
            // Report stack contents
            if (exec_->GetDataStack()->Size() > 0) {
                AddLog("Stack:");
                for (auto obj : *exec_->GetDataStack()) {
                    StringStream st;
                    st << "  " << obj;
                    AddLog("%s", st.ToString().c_str());
                }
            }
            else {
                AddLog("Stack is empty");
            }
        } 
        catch (Exception::Base& e) {
            StringStream st;
            st << "Error: " << e.ToString();
            
            ImVec4 color(1, 0, 0, 1);
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            AddLog("%s", st.ToString().c_str());
            ImGui::PopStyleColor();
        }
    }
};

void ShowExecutorWindow(bool* p_open) {
    static ExecutorWindow console;
    console.Draw("KAI Languages Console", p_open);
}

KAI_END
