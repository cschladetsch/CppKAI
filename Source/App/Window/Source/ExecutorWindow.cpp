#include <KAI/Console.h>
#include <KAI/Core/Exception.h>
#include <imgui.h>

#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

KAI_BEGIN

// Enum for the available tabs in the console window
enum class ConsoleTab { Pi, Rho, Debugger };

// A tabbed console with Pi, Rho, and Debugger tabs
struct ExecutorWindow {
    // Input and history state
    char InputBuf[256];
    char MultilineInputBuf[4096];  // Larger buffer for multi-line Rho input
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

        // Initialize input buffers
        memset(InputBuf, 0, sizeof(InputBuf));
        memset(MultilineInputBuf, 0, sizeof(MultilineInputBuf));

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

            // Clear the input buffers when switching languages
            InputBuf[0] = '\0';
            MultilineInputBuf[0] = '\0';
        }
    }

    void SwitchTab(ConsoleTab tab) {
        if (CurrentTab != tab) {
            CurrentTab = tab;

            // If switching to a language tab, ensure the corresponding language
            // is set
            if (tab == ConsoleTab::Pi) {
                SwitchLanguage(Language::Pi);
            } else if (tab == ConsoleTab::Rho) {
                SwitchLanguage(Language::Rho);
            }

            // Clear the input buffers when switching tabs
            InputBuf[0] = '\0';
            MultilineInputBuf[0] = '\0';
        }
    }

    void Draw(const char* title, bool* p_open) {
        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open)) {
            ImGui::End();
            return;
        }

        // Create styled tab selection buttons with tab-like appearance
        float tabWidth = ImGui::GetContentRegionAvailWidth() / 3.0f - 4.0f;
        float tabHeight = 30.0f;

        // Style adjustments for all tabs
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,
                            4.0f);  // Rounded corners for tab-like buttons
        ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(10, 8));  // More padding for better tab appearance
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(1, 0));  // Reduce spacing between tabs

        // Draw background tab bar
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::BeginChild(
            "TabBar", ImVec2(ImGui::GetContentRegionAvailWidth(), tabHeight),
            false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Adjust position to align tabs
        ImGui::SetCursorPos(ImVec2(2, 2));

        // Pi Tab Button
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (CurrentTab == ConsoleTab::Pi)
                                  ? ImVec4(0.3f, 0.6f, 0.8f, 1.0f)
                                  : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (CurrentTab == ConsoleTab::Pi)
                                  ? ImVec4(0.4f, 0.7f, 0.9f, 1.0f)
                                  : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Custom button styling - only rounded on top for Pi tab
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (CurrentTab == ConsoleTab::Pi) {
            // Highlight active tab with bottom border that matches the tab
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImVec2(ImGui::GetCursorScreenPos().x + tabWidth,
                       ImGui::GetCursorScreenPos().y + tabHeight + 1),
                ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.8f, 1.0f)), 4.0f,
                ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight);
        }

        if (ImGui::Button("Pi", ImVec2(tabWidth, tabHeight - 4))) {
            SwitchTab(ConsoleTab::Pi);
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        ImGui::SameLine();

        // Rho Tab Button
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (CurrentTab == ConsoleTab::Rho)
                                  ? ImVec4(0.3f, 0.6f, 0.8f, 1.0f)
                                  : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (CurrentTab == ConsoleTab::Rho)
                                  ? ImVec4(0.4f, 0.7f, 0.9f, 1.0f)
                                  : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Custom button styling - only rounded on top for Rho tab
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (CurrentTab == ConsoleTab::Rho) {
            // Highlight active tab with bottom border that matches the tab
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImVec2(ImGui::GetCursorScreenPos().x + tabWidth,
                       ImGui::GetCursorScreenPos().y + tabHeight + 1),
                ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.8f, 1.0f)), 4.0f,
                ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight);
        }

        if (ImGui::Button("Rho", ImVec2(tabWidth, tabHeight - 4))) {
            SwitchTab(ConsoleTab::Rho);
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        ImGui::SameLine();

        // Debugger Tab Button
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (CurrentTab == ConsoleTab::Debugger)
                                  ? ImVec4(0.3f, 0.6f, 0.8f, 1.0f)
                                  : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (CurrentTab == ConsoleTab::Debugger)
                                  ? ImVec4(0.4f, 0.7f, 0.9f, 1.0f)
                                  : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Custom button styling - only rounded on top for Debugger tab
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (CurrentTab == ConsoleTab::Debugger) {
            // Highlight active tab with bottom border that matches the tab
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImVec2(ImGui::GetCursorScreenPos().x + tabWidth,
                       ImGui::GetCursorScreenPos().y + tabHeight + 1),
                ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.8f, 1.0f)), 4.0f,
                ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight);
        }

        if (ImGui::Button("Debugger", ImVec2(tabWidth, tabHeight - 4))) {
            SwitchTab(ConsoleTab::Debugger);
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        ImGui::EndChild();
        ImGui::PopStyleColor();  // Pop tab bar background color
        ImGui::PopStyleVar(3);   // Pop style vars for all tabs

        // Draw a separator line below the tabs
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(ImGui::GetCursorScreenPos().x,
                   ImGui::GetCursorScreenPos().y),
            ImVec2(ImGui::GetCursorScreenPos().x +
                       ImGui::GetContentRegionAvailWidth(),
                   ImGui::GetCursorScreenPos().y),
            ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.8f, 1.0f)), 1.0f);

        ImGui::Dummy(ImVec2(0, 4));  // Add some space after the tabs

        // Draw content based on current tab
        if (CurrentTab == ConsoleTab::Debugger) {
            DrawDebuggerContent();
        } else {
            DrawConsoleContent();
        }

        ImGui::End();
    }

    void DrawConsoleContent() {
        // Create a styled header section
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::BeginChild("ConsoleHeader",
                          ImVec2(ImGui::GetContentRegionAvailWidth(), 40),
                          true);

        // Show language name with larger font and better styling
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        std::string header =
            (CurrentLanguage == Language::Pi) ? "Pi Console" : "Rho Console";

        // Center the header text vertically
        float headerTextHeight = ImGui::GetTextLineHeightWithSpacing();
        ImGui::SetCursorPosY((40 - headerTextHeight) * 0.5f);
        ImGui::SetCursorPosX(10);  // Indent from left edge
        ImGui::Text("%s", header.c_str());
        ImGui::PopStyleColor();
        ImGui::PopFont();

        // Position control buttons on the right side of the header
        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() -
                        160);  // Position from right edge
        ImGui::SetCursorPosY((40 - ImGui::GetFrameHeightWithSpacing()) *
                             0.5f);  // Center buttons vertically

        // Style the control buttons to match the tab theme
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        if (ImGui::Button("Clear Log", ImVec2(75, 0))) {
            ClearLog();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Stack", ImVec2(75, 0))) {
            exec_->ClearStacks();
            AddLog("Stack cleared");
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::EndChild();
        ImGui::PopStyleColor();  // ChildBg

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
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(4, 1));  // Tighten spacing

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

        // Use different input methods for Pi and Rho
        if (CurrentLanguage == Language::Pi) {
            // Pi uses single-line input
            ImGuiInputTextFlags input_text_flags =
                ImGuiInputTextFlags_EnterReturnsTrue;

            if (ImGui::InputText("Pi>", InputBuf, sizeof(InputBuf),
                                 input_text_flags)) {
                char* input_end = InputBuf + strlen(InputBuf);
                while (input_end > InputBuf && input_end[-1] == ' ')
                    input_end--;

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
        } else {
            // Rho uses multi-line input
            ImGui::Text("Rho> (Multi-line input - Press button to execute)");
            ImGui::Separator();

            // Create a child window for better layout control
            ImGui::BeginChild(
                "RhoInputArea",
                ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 7), false);

            ImGuiInputTextFlags multiline_flags =
                ImGuiInputTextFlags_AllowTabInput;

            // Calculate width to leave room for Execute button
            float buttonWidth = 80.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float inputWidth =
                ImGui::GetContentRegionAvailWidth() - buttonWidth - spacing;

            // Add a helpful hint
            ImGui::TextColored(
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Enter Rho code below (supports functions, loops, etc.):");

            // Multi-line input field with a border
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                  ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

            ImGui::PushItemWidth(inputWidth);
            ImGui::InputTextMultiline(
                "##RhoInput", MultilineInputBuf, sizeof(MultilineInputBuf),
                ImVec2(inputWidth, ImGui::GetTextLineHeightWithSpacing() * 5),
                multiline_flags);
            ImGui::PopItemWidth();

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            // Execute button aligned to the right
            ImGui::SameLine();
            ImGui::BeginGroup();

            // Style the execute button
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.4f, 0.8f, 0.4f, 1.0f));

            if (ImGui::Button(
                    "Execute",
                    ImVec2(buttonWidth,
                           ImGui::GetTextLineHeightWithSpacing() * 2))) {
                char* input_end = MultilineInputBuf + strlen(MultilineInputBuf);
                while (input_end > MultilineInputBuf && input_end[-1] == ' ')
                    input_end--;
                *input_end = 0;

                if (MultilineInputBuf[0]) {
                    // Add to history
                    History[CurrentLanguage].push_back(MultilineInputBuf);

                    // Execute the command
                    ExecCommand(MultilineInputBuf);
                }

                strcpy(MultilineInputBuf, "");
                reclaim_focus = true;
            }

            ImGui::PopStyleColor(3);

            // Clear button below Execute
            if (ImGui::Button("Clear", ImVec2(buttonWidth, 0))) {
                strcpy(MultilineInputBuf, "");
                reclaim_focus = true;
            }

            ImGui::EndGroup();
            ImGui::EndChild();
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1);  // Auto focus previous widget
    }

    void DrawDebuggerContent() {
        // Create a styled header section consistent with the console tabs
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::BeginChild("DebuggerHeader",
                          ImVec2(ImGui::GetContentRegionAvailWidth(), 40),
                          true);

        // Show debugger header with matching style
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));

        // Center the header text vertically
        float headerTextHeight = ImGui::GetTextLineHeightWithSpacing();
        ImGui::SetCursorPosY((40 - headerTextHeight) * 0.5f);
        ImGui::SetCursorPosX(10);  // Indent from left edge
        ImGui::Text("Debugger Console");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::EndChild();
        ImGui::PopStyleColor();  // ChildBg

        // Debugger controls with improved styling
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::BeginChild("DebuggerControls", ImVec2(0, 50), true);

        // Style the control buttons to match the tab theme
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        // Center buttons vertically
        ImGui::SetCursorPosY((50 - ImGui::GetFrameHeightWithSpacing()) * 0.5f);
        ImGui::SetCursorPosX(10);  // Indent from left edge

        // Debugging control button with color indication
        if (IsDebugging) {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.8f, 0.2f, 0.2f, 1.0f));  // Red for stop
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            if (ImGui::Button("Stop Debugging", ImVec2(120, 0))) {
                IsDebugging = false;
                AddLog("Debugging stopped");
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.2f, 0.7f, 0.2f, 1.0f));  // Green for start
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
            if (ImGui::Button("Start Debugging", ImVec2(120, 0))) {
                IsDebugging = true;
                AddLog("Debugging started");
            }
            ImGui::PopStyleColor(3);
        }

        ImGui::SameLine();

        // Step button - active only when debugging
        if (IsDebugging) {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.2f, 0.5f, 0.8f, 1.0f));  // Blue for step
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
        } else {
            ImGui::PushStyleColor(
                ImGuiCol_Button,
                ImVec4(0.5f, 0.5f, 0.5f, 0.5f));  // Gray when inactive
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                                  ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                  ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
        }

        if (ImGui::Button("Step", ImVec2(75, 0)) && IsDebugging) {
            ExecuteDebugStep();
        }
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        if (ImGui::Button("Clear Log", ImVec2(75, 0))) {
            DebugLog.clear();
            DebugLog.push_back("Debugger log cleared");
            ScrollToBottom = true;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);  // Button styles

        ImGui::EndChild();
        ImGui::PopStyleColor();  // ChildBg

        // Split view with stack/context view on the left, log on the right
        ImGui::Columns(2, "debugger_columns");

        // Left column - Stack & Context
        // Push once for all headers in this section
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.22f, 0.22f, 0.22f, 1.0f));

        // Stack header
        ImGui::BeginChild("StackHeader", ImVec2(0, 25), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::SetCursorPosY((25 - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::SetCursorPosX(5);
        ImGui::Text("Data Stack");
        ImGui::PopStyleColor();
        ImGui::EndChild();

        // Stack view with styled selectable items
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
        ImGui::BeginChild("StackView", ImVec2(0, 180), true);

        if (exec_->GetDataStack()->Size() > 0) {
            // Style for stack items
            ImGui::PushStyleColor(ImGuiCol_Header,
                                  ImVec4(0.3f, 0.6f, 0.8f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                                  ImVec4(0.3f, 0.6f, 0.8f, 0.8f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                                  ImVec4(0.3f, 0.6f, 0.8f, 1.0f));

            for (int i = 0; i < exec_->GetDataStack()->Size(); i++) {
                auto obj = exec_->GetDataStack()->At(i);
                StringStream st;
                st << i << ": " << obj;

                if (ImGui::Selectable(st.ToString().c_str(), WatchIndex == i)) {
                    WatchIndex = i;
                }

                // Draw a thin separator between items
                if (i < exec_->GetDataStack()->Size() - 1) {
                    ImGui::Separator();
                }
            }

            ImGui::PopStyleColor(3);
        } else {
            ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.5f -
                                 ImGui::GetTextLineHeight() * 0.5f);
            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5f -
                                 ImGui::CalcTextSize("Stack is empty").x *
                                     0.5f);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "Stack is empty");
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();  // Pop the ChildBg color
        ImGui::PopStyleVar();

        // Context header
        ImGui::BeginChild("ContextHeader", ImVec2(0, 25), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::SetCursorPosY((25 - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::SetCursorPosX(5);
        ImGui::Text("Context Viewer");
        ImGui::PopStyleColor();
        ImGui::EndChild();

        // Context view
        ImGui::BeginChild("ContextView", ImVec2(0, 0), true);

        // Show information about the currently selected variable if available
        if (exec_->GetDataStack()->Size() > 0 && WatchIndex >= 0 &&
            WatchIndex < exec_->GetDataStack()->Size()) {
            auto obj = exec_->GetDataStack()->At(WatchIndex);

            // Create a styled header for the watch panel
            ImGui::PushStyleColor(ImGuiCol_Header,
                                  ImVec4(0.3f, 0.6f, 0.8f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                                  ImVec4(0.3f, 0.6f, 0.8f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive,
                                  ImVec4(0.3f, 0.6f, 0.8f, 0.6f));

            ImGui::CollapsingHeader(
                "Watch - Stack Item",
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf);
            ImGui::PopStyleColor(3);

            // Add some padding
            ImGui::Indent(10);

            // Object index
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Index: %d",
                               WatchIndex);

            // Display type information safely
            int typeNum = obj.GetTypeNumber().ToInt();
            ImGui::TextColored(ImVec4(0.9f, 0.7f, 1.0f, 1.0f), "Type: %d",
                               typeNum);

            // Show string representation
            StringStream st;
            st << obj;

            ImGui::BeginChild("ValueView", ImVec2(0, 80), true);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.7f, 1.0f), "Value:");
            ImGui::Separator();
            ImGui::TextWrapped("%s", st.ToString().c_str());
            ImGui::EndChild();

            // Show object information
            ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f), "Valid: %s",
                               obj.Exists() ? "Yes" : "No");

            ImGui::Unindent(10);
        } else {
            // Show a message when no item is selected
            ImGui::SetCursorPosY(ImGui::GetContentRegionAvail().y * 0.5f -
                                 ImGui::GetTextLineHeight() * 0.5f);
            ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x * 0.5f -
                                 ImGui::CalcTextSize("No item selected").x *
                                     0.5f);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "No item selected");
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();  // ChildBg for section headers

        ImGui::NextColumn();

        // Right column - Debug Log
        // Log header - push a new style color for this column
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::BeginChild("LogHeader", ImVec2(0, 25), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::SetCursorPosY((25 - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::SetCursorPosX(5);
        ImGui::Text("Debug Log");
        ImGui::PopStyleColor();
        ImGui::EndChild();

        // Debug log with syntax highlighting for different message types
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::BeginChild("DebugLog", ImVec2(0, 0), true,
                          ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

        for (const auto& log : DebugLog) {
            // Simple syntax highlighting based on log content
            if (log.find("Error") != std::string::npos ||
                log.find("error") != std::string::npos ||
                log.find("failed") != std::string::npos ||
                log.find("Failed") != std::string::npos) {
                // Red for errors
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s",
                                   log.c_str());
            } else if (log.find("Step") != std::string::npos ||
                       log.find("stack") != std::string::npos) {
                // Blue for step operations
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s",
                                   log.c_str());
            } else if (log.find("Debugging started") != std::string::npos) {
                // Green for start debugging
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s",
                                   log.c_str());
            } else if (log.find("Debugging stopped") != std::string::npos) {
                // Yellow for stop debugging
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "%s",
                                   log.c_str());
            } else {
                // Default text color
                ImGui::TextUnformatted(log.c_str());
            }
        }

        if (ScrollToBottom) ImGui::SetScrollHereY(1.0f);

        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopStyleColor(1);  // ChildBg for debug log
        ImGui::PopStyleColor(1);  // ChildBg for headers in this column

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
            } else {
                // For Rho, show scope information instead
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

    void ExecCommand(const char* command_line) {
        // Add the command to the log first
        string cmdWithPrompt =
            (CurrentLanguage == Language::Pi) ? "Pi> " : "Rho> ";
        cmdWithPrompt += command_line;
        AddLog("%s", cmdWithPrompt.c_str());

        // If in debugger tab, automatically switch to the corresponding
        // language tab
        if (CurrentTab == ConsoleTab::Debugger) {
            CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi
                                                           : ConsoleTab::Rho;
        }

        // Execute the command with all zsh-like features
        try {
            std::string text = command_line;

            // Store current command for !# support
            console_.currentCommand = text;

            // Commands starting with $ are shell commands
            if (!text.empty() && text[0] == '$') {
                // Execute as shell command (strip the $ and any leading space)
                std::string shellCmd = text.substr(1);
                // Trim leading whitespace
                size_t firstNonSpace = shellCmd.find_first_not_of(" \t");
                if (firstNonSpace != std::string::npos) {
                    shellCmd = shellCmd.substr(firstNonSpace);
                }

                // Execute the shell command
                FILE* pipe = popen(shellCmd.c_str(), "r");
                if (pipe) {
                    std::string result;
                    char buffer[128];
                    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                        result += buffer;
                    }
                    pclose(pipe);
                    AddLog("%s", result.c_str());
                } else {
                    AddLog("Failed to execute: %s", shellCmd.c_str());
                }
            } else if (!text.empty() && text[0] == '^') {
                // Handle quick substitution ^old^new^
                String substituted =
                    console_.ProcessQuickSubstitution(String(text));
                if (substituted.size() > 0) {
                    AddLog("=> %s", substituted.c_str());

                    // Process the substituted command
                    String expandedText =
                        console_.ExpandShellCommands(substituted);
                    String result = console_.Process(expandedText);
                    if (!result.empty()) {
                        AddLog("%s", result.c_str());
                    }
                } else {
                    AddLog("Substitution failed: no match found");
                }
            } else if (!text.empty()) {
                // Check for shell commands
                if (text[0] == '`') {
                    String output = console_.ProcessShellCommand(String(text));
                    AddLog("%s", output.c_str());
                } else {
                    // Handle zsh-like history commands
                    std::string processedText = text;

                    // If it's a pure history command (just !!, !n, etc), expand
                    // it
                    if (text[0] == '!' && text.find(' ') == std::string::npos) {
                        String expanded =
                            console_.ProcessZshCommand(String(text));
                        if (expanded.size() > 0) {
                            processedText = expanded.StdString();
                            // Show what command is being executed
                            AddLog("=> %s", processedText.c_str());
                        } else {
                            AddLog("No matching command in history");
                            return;
                        }
                    } else {
                        // Expand any history references within the command
                        processedText =
                            console_.ExpandHistoryReferences(String(text))
                                .StdString();
                    }

                    // Expand any embedded shell commands first
                    String expandedText =
                        console_.ExpandShellCommands(String(processedText));
                    String result = console_.Process(expandedText);
                    if (!result.empty()) {
                        AddLog("%s", result.c_str());
                    }
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
