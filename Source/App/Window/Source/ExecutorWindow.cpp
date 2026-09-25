#include <KAI/Console.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/FunctionBase.h>
#include <KAI/Core/Logger.h>
#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/GetStorageBase.h>
#include <KAI/Core/Object/MethodBase.h>
#include <KAI/Executor/Continuation.h>
#include <imgui.h>

#include <cstring>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

KAI_BEGIN

namespace {
std::string FormatStackValue(const Object& object) {
    // Function/Method/Class objects are stored via BasePointerBase (or, for
    // Class, a raw ClassBase*), whose generic StringStream operator<< is a
    // hard KAI_NOT_IMPLEMENTED() - that's what surfaces as
    // "Error: [StringStream.cpp:41] Not Implemented" from ordinary console
    // commands like "info" that print a Function/Method/Class value via the
    // stack, not just from the Tree tab. Each of those types does have a
    // working ToString() reachable by deref'ing down to its concrete
    // BasePointer<T>/ClassBase*, the same way the Tree tab's Details pane
    // does - use that instead of the generic (and broken) dispatch.
    if (object.Exists()) {
        std::string className = object.GetClass()
                                     ? object.GetClass()->GetName().ToString().CStr()
                                     : "";
        if (className == "Function") {
            return ConstDeref<BasePointer<FunctionBase>>(object)->ToString().CStr();
        }
        if (className == "Method") {
            return ConstDeref<BasePointer<MethodBase>>(object)->ToString().CStr();
        }
        if (className == "Class") {
            const ClassBase* cls = ConstDeref<const ClassBase*>(object);
            return cls ? ("Class: " + std::string(cls->GetName().ToString().CStr()))
                       : "Class: (null)";
        }
    }

    std::string text = object.ToString().StdString();
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }

    if (object.GetTypeNumber() == Type::Number::String) {
        return "\"" + text + "\"";
    }

    return text;
}

// Color-codes a log line by the KAI value type it represents, so ints,
// floats, strings, bools etc. are visually distinct at a glance rather than
// all rendering in the same default text color.
ImVec4 ColorForType(Type::Number t) {
    switch (t.value) {
        case Type::Number::Signed32:
            return ImVec4(0.55f, 0.85f, 0.55f, 1.0f);  // int - green
        case Type::Number::Single:
        case Type::Number::Double:
            return ImVec4(1.0f, 0.7f, 0.35f, 1.0f);  // float/double - orange
        case Type::Number::String:
            return ImVec4(1.0f, 0.9f, 0.4f, 1.0f);  // string - yellow
        case Type::Number::Bool:
            return ImVec4(0.85f, 0.55f, 1.0f, 1.0f);  // bool - purple
        default:
            return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);  // everything else
    }
}

// Cyan prompt shown before each echoed command: a real pi/rho glyph
// (U+03C0/U+03C1). Requires the Greek glyph merge in Main.cpp's SetupGui()
// (merges a system font's Greek range into the default font atlas *before*
// ImGui_ImplOpenGL3_Init() builds the GPU texture) - without that merge
// these render as missing-glyph boxes rather than crashing.
constexpr const char* kPiPrompt = "\xCF\x80 ";   // UTF-8 for U+03C0 GREEK SMALL LETTER PI
constexpr const char* kRhoPrompt = "\xCF\x81 ";  // UTF-8 for U+03C1 GREEK SMALL LETTER RHO
const ImVec4 kPromptColor(0.3f, 0.9f, 0.95f, 1.0f);  // cyan
const ImVec4 kErrorColor(1.0f, 0.35f, 0.35f, 1.0f);  // red, for "[Error] ..." log lines

// Rho's multi-line input needs ImGuiInputTextFlags_AllowTabInput so Tab
// doesn't just move keyboard focus away, but ImGui then inserts a literal
// '\t' character. Rho code is indentation-sensitive-ish and a raw tab looks
// inconsistent next to space-indented lines, so replace every tab that ends
// up immediately before the cursor with 4 spaces, in the same frame it was
// typed (ImGuiInputTextFlags_CallbackAlways runs after the keypress is
// applied but before the widget renders).
int RhoTabToSpacesCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways &&
        data->CursorPos > 0 && data->Buf[data->CursorPos - 1] == '\t') {
        data->DeleteChars(data->CursorPos - 1, 1);
        data->InsertChars(data->CursorPos, "    ");
    }
    return 0;
}
}  // namespace

// Enum for the available tabs in the console window
enum class ConsoleTab { Pi, Rho, Debugger, Tree };

// A tabbed console with Pi, Rho, and Debugger tabs
struct ExecutorWindow {
    // Input and history state
    char InputBuf[256];
    char MultilineInputBuf[4096];  // Larger buffer for multi-line Rho input
    int HistoryPos;  // -1: new line, 0..History.Size-1 browsing history.
    bool ScrollToBottom;
    bool FocusInputNextFrame;

    // A single log line plus the color it should render in - lets the
    // console color-code prompts and typed stack values instead of every
    // line coming out in the same default text color.
    struct LogLine {
        std::string text;
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        // Optional leading segment rendered in its own (fixed) color before
        // `text` on the same line - used for the "[N] " stack index, which
        // should always read grey regardless of the value's type color.
        std::string prefix;
        ImVec4 prefixColor = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    };

    // Output for each language
    map<Language, vector<LogLine>> Items;
    map<Language, vector<string>> History;

    // Current active language and tab
    Language CurrentLanguage;
    ConsoleTab CurrentTab;

    // Debugger state
    bool IsDebugging = false;
    int DebugStepCount = 0;
    vector<string> DebugLog;
    int WatchIndex = 0;

    // Tree tab state - which node is currently selected, Explorer-style
    int SelectedTreeHandle = -1;
    Object SelectedTreeObject;
    std::string SelectedTreePath;

    // KAI console objects
    Console console_;
    Tree* tree_;
    Executor* exec_;
    Registry* reg_;

    ExecutorWindow() {
        HistoryPos = -1;
        CurrentLanguage = Language::Pi;
        CurrentTab = ConsoleTab::Pi;
        FocusInputNextFrame = true;

        // Initialize input buffers
        memset(InputBuf, 0, sizeof(InputBuf));
        memset(MultilineInputBuf, 0, sizeof(MultilineInputBuf));

        // Initialize console with Pi language by default
        console_.SetLanguage(CurrentLanguage);
        exec_ = &*console_.GetExecutor();
        reg_ = &console_.GetRegistry();
        tree_ = &console_.GetTree();

        // Initialize language-specific logs
        Items[Language::Pi] = vector<LogLine>();
        Items[Language::Rho] = vector<LogLine>();

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
            Items[CurrentLanguage].push_back(LogLine{buf});
        }

        ScrollToBottom = true;
    }

    // Same as AddLog, but the line renders in `color` instead of the default
    // white - used for the prompt echo and for type-colored stack values.
    void AddLog(const ImVec4& color, const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        buf[strlen(buf)] = 0;
        va_end(args);

        if (CurrentTab == ConsoleTab::Debugger) {
            // Debug Log panel doesn't carry per-line color today - fall back
            // to the plain line rather than losing the message.
            DebugLog.push_back(buf);
        } else {
            Items[CurrentLanguage].push_back(LogLine{buf, color});
        }

        ScrollToBottom = true;
    }

    // Logs one stack slot as "[index] value" on a single line, with the
    // "[index] " part always grey (LogLine::prefixColor's default) and only
    // `value`/`valueColor` varying by the value's type - the index is a
    // fixed frame of reference and shouldn't visually compete with, or be
    // mistaken for, a type-colored value.
    void AddStackLog(int index, const std::string& value,
                     const ImVec4& valueColor) {
        LogLine line;
        line.prefix = "[" + std::to_string(index) + "] ";
        line.text = value;
        line.color = valueColor;
        // line.prefixColor left at its grey default.

        if (CurrentTab == ConsoleTab::Debugger) {
            DebugLog.push_back(line.prefix + line.text);
        } else {
            Items[CurrentLanguage].push_back(std::move(line));
        }

        ScrollToBottom = true;
    }

    // Console::Process() (Ext/CppKaiCore/.../Console.cpp) catches parse and
    // execution errors internally and returns them as plain result text
    // ("Exception: ...", "StdException: ...", "UnknownException:") rather
    // than letting them propagate - so ExecCommand's own
    // catch (exception::Base&) block never sees a syntax error like an
    // unmatched brace; it only ever sees what Process() decided to hand
    // back as a normal string. Route that string through here so it still
    // renders red/"[Error]" like a caught exception would, instead of
    // silently blending in as a plain white log line.
    void AddProcessResultLog(const std::string& result) {
        static const char* kErrorPrefixes[] = {
            "Exception:", "StdException:", "UnknownException:"};
        for (const char* prefix : kErrorPrefixes) {
            if (result.compare(0, strlen(prefix), prefix) == 0) {
                AddLog(kErrorColor, "[Error] %s", result.c_str());
                return;
            }
        }
        AddLog("%s", result.c_str());
    }

    void SwitchLanguage(Language lang) {
        if (CurrentLanguage != lang) {
            CurrentLanguage = lang;
            console_.SetLanguage(CurrentLanguage);
            exec_ = &*console_.GetExecutor();

            // Clear the input buffers when switching languages
            InputBuf[0] = '\0';
            MultilineInputBuf[0] = '\0';
            FocusInputNextFrame = true;
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
            FocusInputNextFrame = true;
        }
    }

    void Draw(const char* title, bool* p_open) {
        ImGui::SetNextWindowSize(ImVec2(1080, 820), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open)) {
            ImGui::End();
            return;
        }

        // Create styled tab selection buttons with tab-like appearance
        float tabWidth = ImGui::GetContentRegionAvailWidth() / 4.0f - 4.0f;
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
        ImGui::SameLine();

        // Tree Tab Button
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (CurrentTab == ConsoleTab::Tree)
                                  ? ImVec4(0.3f, 0.6f, 0.8f, 1.0f)
                                  : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (CurrentTab == ConsoleTab::Tree)
                                  ? ImVec4(0.4f, 0.7f, 0.9f, 1.0f)
                                  : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // Custom button styling - only rounded on top for Tree tab
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (CurrentTab == ConsoleTab::Tree) {
            // Highlight active tab with bottom border that matches the tab
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImVec2(ImGui::GetCursorScreenPos().x + tabWidth,
                       ImGui::GetCursorScreenPos().y + tabHeight + 1),
                ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.8f, 1.0f)), 4.0f,
                ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight);
        }

        if (ImGui::Button("Tree", ImVec2(tabWidth, tabHeight - 4))) {
            SwitchTab(ConsoleTab::Tree);
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
        } else if (CurrentTab == ConsoleTab::Tree) {
            DrawTreeContent();
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

        // Removed the mini stack strip that used to live here: showing it
        // right next to the command-echo log (which lists "Pi> 1", "Pi> 2",
        // ... in chronological, not stack, order) read like two disagreeing
        // stacks. The Debugger tab's Data Stack panel is now the single,
        // unambiguous place to see the stack.
        ImGui::Separator();

        // Output region
        float reserved_input_height = ImGui::GetFrameHeightWithSpacing();
        if (CurrentLanguage == Language::Rho) {
            reserved_input_height =
                ImGui::GetTextLineHeightWithSpacing() * 11 +
                ImGui::GetFrameHeightWithSpacing() * 2 +
                ImGui::GetStyle().ItemSpacing.y * 6;
        }
        ImGui::BeginChild("ScrollingRegion",
                          ImVec2(0, -reserved_input_height),
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
            const LogLine& item = currentItems[i];
            if (!item.prefix.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, item.prefixColor);
                ImGui::TextUnformatted(item.prefix.c_str());
                ImGui::PopStyleColor();
                ImGui::SameLine(0.0f, 0.0f);
            }
            ImGui::PushStyleColor(ImGuiCol_Text, item.color);
            ImGui::TextUnformatted(item.text.c_str());
            ImGui::PopStyleColor();
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

            if (FocusInputNextFrame) {
                ImGui::SetKeyboardFocusHere();
                FocusInputNextFrame = false;
            }

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
                FocusInputNextFrame = true;
            }
        } else {
            ImGui::Text("Rho> (multi-line editor, Ctrl+Enter or Execute)");
            if (FocusInputNextFrame) {
                ImGui::SetKeyboardFocusHere();
                FocusInputNextFrame = false;
            }
            ImGui::Separator();

            ImGuiInputTextFlags multiline_flags =
                ImGuiInputTextFlags_AllowTabInput |
                ImGuiInputTextFlags_CallbackAlways;

            // Add a helpful hint
            ImGui::TextColored(
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "Enter Rho code below (supports functions, loops, etc.):");

            // Multi-line input field with a border
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                  ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

            ImGui::InputTextMultiline(
                "##RhoInput", MultilineInputBuf, sizeof(MultilineInputBuf),
                ImVec2(-1.0f, ImGui::GetTextLineHeightWithSpacing() * 6),
                multiline_flags, RhoTabToSpacesCallback);
            bool execute_rho =
                ImGui::IsItemActive() && ImGui::GetIO().KeyCtrl &&
                ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter));

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            float buttonWidth = 100.0f;

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
                execute_rho = true;
            }

            if (execute_rho) {
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
                FocusInputNextFrame = true;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();
            if (ImGui::Button("Clear", ImVec2(buttonWidth, 0))) {
                strcpy(MultilineInputBuf, "");
                FocusInputNextFrame = true;
            }
        }

        (void)reclaim_focus;
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
                int displayIndex = exec_->GetDataStack()->Size() - 1 - i;
                StringStream st;
                st << "[" << displayIndex << "]: "
                   << FormatStackValue(obj).c_str();

                if (ImGui::Selectable(st.ToString().CStr(), WatchIndex == i)) {
                    WatchIndex = i;
                }

                // Draw a thin separator between items
                if (i < exec_->GetDataStack()->Size() - 1) {
                    ImGui::Separator();
                }
            }

            ImGui::PopStyleColor(3);
        }
        // Empty stack: leave the panel blank rather than showing a
        // "Stack is empty" placeholder.

        ImGui::EndChild();
        ImGui::PopStyleColor();  // Pop the ChildBg color
        ImGui::PopStyleVar();

        // Continuation header - the executor's current continuation (the
        // in-flight execution state: instruction pointer, code size,
        // scope), previously not shown anywhere in this window even though
        // GetDataStack()/GetContextStack() were. Needed a new
        // Executor::GetContinuation() accessor (Executor.h) since
        // continuation_ was private with no getter.
        ImGui::BeginChild("ContinuationHeader", ImVec2(0, 25), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::SetCursorPosY((25 - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::SetCursorPosX(5);
        ImGui::Text("Continuation");
        ImGui::PopStyleColor();
        ImGui::EndChild();

        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::BeginChild("ContinuationView", ImVec2(0, 90), true);
        {
            Value<Continuation> cont = exec_->GetContinuation();
            if (cont.Exists()) {
                ImGui::Text("Instruction pointer: %d",
                           cont->GetInstructionPointer());
                ImGui::Text(
                    "Code size: %d",
                    cont->HasCode() ? (int)cont->GetCode()->Size() : 0);
                ImGui::Text("Has scope: %s",
                           cont->HasScope() ? "yes" : "no");
                Pointer<String> src = cont->GetSourceCode();
                if (src.Exists() && src->Size() > 0) {
                    ImGui::TextWrapped("Source: %s", src->c_str());
                }
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                   "No active continuation");
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        // Context Stack header - Executor::GetContextStack(), distinct
        // from the Data Stack above and from the "Context Viewer"
        // watch/inspector panel below (which despite its name only ever
        // showed the selected Data Stack item's details, not the
        // executor's actual context stack).
        ImGui::BeginChild("ContextStackHeader", ImVec2(0, 25), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::SetCursorPosY((25 - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::SetCursorPosX(5);
        ImGui::Text("Context Stack");
        ImGui::PopStyleColor();
        ImGui::EndChild();

        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::BeginChild("ContextStackView", ImVec2(0, 120), true);
        {
            Value<Stack> ctxStack = exec_->GetContextStack();
            if (ctxStack.Exists() && ctxStack->Size() > 0) {
                for (int i = 0; i < ctxStack->Size(); i++) {
                    auto obj = ctxStack->At(i);
                    int displayIndex = ctxStack->Size() - 1 - i;
                    ImGui::Text("[%d] %s", displayIndex,
                               FormatStackValue(obj).c_str());
                    if (i < ctxStack->Size() - 1) {
                        ImGui::Separator();
                    }
                }
            }
            // Empty context stack: leave the panel blank, same as the Data
            // Stack panel above, rather than a "Context stack is empty"
            // placeholder.
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

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
            ImGui::TextWrapped("%s", st.ToString().CStr());
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
        AddLog("%s", st.ToString().CStr());

        // Show all stack items
        if (exec_->GetDataStack()->Size() > 0) {
            for (int i = 0; i < exec_->GetDataStack()->Size(); i++) {
                auto obj = exec_->GetDataStack()->At(i);
                int displayIndex = exec_->GetDataStack()->Size() - 1 - i;
                StringStream itemSt;
                itemSt << "  [" << displayIndex << "]: "
                       << FormatStackValue(obj).c_str();
                AddLog("%s", itemSt.ToString().CStr());
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
                    AddLog("%s", scopeSt.ToString().CStr());
                } else {
                    AddLog("No active scope");
                }
            }
        } catch (exception::Base& e) {
            Logger::Error("ImGui Window debug step failed: " +
                          std::string(e.ToString().CStr()));
            AddLog("Debug operation failed: %s", e.ToString().CStr());
        }
    }

    // Recursively renders one node of the executor's object tree as a
    // collapsible ImGui tree item, Explorer-style: objects with children get
    // an expand arrow, leaves don't, and clicking a row selects it (shown in
    // the details pane in DrawTreeContent()). `seen` guards against cycles in
    // the object graph, `depth` is a hard recursion cap for the same reason.
    // RAII guard so a TreePush from TreeNodeEx() always gets its matching
    // TreePop, even if something between them throws - see the long comment
    // in RenderTreeObjectNode for why that matters here.
    struct TreePopGuard {
        bool active;
        explicit TreePopGuard(bool a) : active(a) {}
        ~TreePopGuard() {
            if (active) ImGui::TreePop();
        }
    };

    void RenderTreeObjectNode(const Object& node, const std::string& label,
                              const std::string& path, std::set<int>& seen,
                              int depth) {
        // Every KAI Object call below (GetDictionary, GetClass, ToString,
        // ...) can throw for objects the tree wasn't expecting to touch
        // (internal Type/Class descriptors, continuations mid-execution,
        // GC-transitional state, etc.) - the rest of this file always
        // wraps Object access in try/catch for exactly that reason, this
        // function was the one place that didn't, so any such exception
        // was propagating out of the ImGui frame uncaught and crashing the
        // app. Catch per-node so one bad subobject renders an error line
        // instead of crashing the process or breaking the rest of the tree.
        // TreePopGuard (not a bare ImGui::TreePop() call at the end)
        // guarantees the tree/ID stack stays balanced even if a child
        // throws mid-recursion.
        bool open = false;
        bool hasChildren = false;
        try {
            if (!node.Exists() || depth > 32) return;

            int handle = node.GetHandle().GetValue();
            if (!seen.insert(handle).second) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                                   "%s  (cycle)", label.c_str());
                return;
            }

            const Dictionary& dict = node.GetDictionary();
            hasChildren = !dict.empty();

            std::string className =
                node.GetClass()
                    ? node.GetClass()->GetName().ToString().CStr()
                    : "?";

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                                       ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (!hasChildren) {
                flags |=
                    ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }
            if (SelectedTreeHandle == handle) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            open = ImGui::TreeNodeEx((void*)(intptr_t)handle, flags,
                                     "%s  (%s)", label.c_str(),
                                     className.c_str());
            if (ImGui::IsItemClicked()) {
                SelectedTreeHandle = handle;
                SelectedTreeObject = node;
                SelectedTreePath = path;
            }
        } catch (exception::Base& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "%s  (error: %s)", label.c_str(),
                               e.ToString().CStr());
            return;
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "%s  (error: %s)", label.c_str(), e.what());
            return;
        } catch (...) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "%s  (unknown error)", label.c_str());
            return;
        }

        // TreeNodeEx already pushed the ID/indent for us when it returned
        // true on a non-leaf node - this guard pops it on every exit path.
        TreePopGuard popGuard(open && hasChildren);
        if (!open || !hasChildren) return;

        try {
            // Copy (label, child) pairs out first: recursing while holding
            // a reference into the live dictionary is unsafe if a child's
            // own rendering mutates this node's storage.
            std::vector<std::pair<std::string, Object>> children;
            const Dictionary& dict = node.GetDictionary();
            children.reserve(dict.size());
            for (const auto& entry : dict) {
                children.emplace_back(entry.first.ToString().CStr(),
                                      entry.second);
            }

            for (const auto& child : children) {
                const std::string& childName = child.first;
                std::string childPath = (path == "/") ? path + childName
                                                       : path + "/" + childName;
                // RenderTreeObjectNode catches its own exceptions, so a bad
                // grandchild can't skip this node's TreePop either.
                RenderTreeObjectNode(child.second, childName, childPath, seen,
                                     depth + 1);
            }
        } catch (exception::Base& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "  (error listing children: %s)",
                               e.ToString().CStr());
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "  (error listing children: %s)", e.what());
        } catch (...) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "  (unknown error listing children)");
        }
    }

    void DrawTreeContent() {
        // Header, styled consistently with the Console/Debugger tabs
        ImGui::PushStyleColor(ImGuiCol_ChildBg,
                              ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::BeginChild("TreeHeader",
                          ImVec2(ImGui::GetContentRegionAvailWidth(), 40),
                          true);
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        float headerTextHeight = ImGui::GetTextLineHeightWithSpacing();
        ImGui::SetCursorPosY((40 - headerTextHeight) * 0.5f);
        ImGui::SetCursorPosX(10);
        ImGui::Text("Executor Tree");
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 90);
        ImGui::SetCursorPosY((40 - ImGui::GetFrameHeightWithSpacing()) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        if (ImGui::Button("Refresh", ImVec2(75, 0))) {
            // The tree is walked fresh every frame anyway; this button just
            // clears the selection in case the selected object went away.
            SelectedTreeHandle = -1;
            SelectedTreeObject = Object();
            SelectedTreePath.clear();
        }
        ImGui::PopStyleColor(3);

        ImGui::EndChild();
        ImGui::PopStyleColor();  // ChildBg

        ImGui::Separator();

        // Left pane: the tree itself (Explorer's folder pane)
        ImGui::BeginChild("TreeView",
                          ImVec2(ImGui::GetContentRegionAvailWidth() * 0.6f, 0),
                          true, ImGuiWindowFlags_HorizontalScrollbar);

        Object root = tree_ ? tree_->GetRoot() : Object();
        if (!root.Exists()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "No tree available for this executor");
        } else {
            std::set<int> seen;
            RenderTreeObjectNode(root, "/", "/", seen, 0);
        }

        ImGui::EndChild();
        ImGui::SameLine();

        // Right pane: details of the selected node (Explorer's preview pane)
        ImGui::BeginChild("TreeDetails", ImVec2(0, 0), true);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::Text("Details");
        ImGui::PopStyleColor();
        ImGui::Separator();

        if (SelectedTreeObject.Exists()) {
            // Same reasoning as RenderTreeObjectNode: these are all live
            // KAI Object calls (GetClass, GetDictionary, ToString via
            // FormatStackValue) and can throw for internal object types
            // (Type/Class descriptors, in-flight continuations, etc.).
            // Don't let a bad selection crash the whole window.
            try {
                std::string className =
                    SelectedTreeObject.GetClass()
                        ? SelectedTreeObject.GetClass()
                              ->GetName()
                              .ToString()
                              .CStr()
                        : "?";
                ImGui::Text("Path:   %s", SelectedTreePath.c_str());
                ImGui::Text("Type:   %s", className.c_str());
                ImGui::Text("Handle: %d", SelectedTreeHandle);
                ImGui::Text("Children: %d",
                            (int)SelectedTreeObject.GetDictionary().size());
                ImGui::Separator();

                // Function/Method objects are stored via BasePointerBase,
                // whose generic StringStream operator<< is a hard
                // KAI_NOT_IMPLEMENTED() in StringStream.cpp - that's a gap
                // in KAI's generic-to-specific dispatch, not something
                // fixable from here. But FunctionBase/MethodBase both have
                // a working ToString() and rich CallableBase<T> accessors
                // (name, return type, argument types, description), reached
                // by deref'ing the Object down to the concrete
                // BasePointer<T> with ConstDeref<T>(Object) - the same
                // mechanism the rest of KAI uses to unwrap Objects.
                if (className == "Function") {
                    const BasePointer<FunctionBase>& fn =
                        ConstDeref<BasePointer<FunctionBase>>(
                            SelectedTreeObject);
                    ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                       "Signature:");
                    ImGui::TextWrapped("%s", fn->ToString().CStr());
                    ImGui::Separator();
                    ImGui::Text("Name:        %s",
                                fn->GetName().ToString().CStr());
                    ImGui::Text("Return type: %s",
                                fn->GetReturnType().ToString().CStr());
                    const auto& args = fn->GetArgumentTypes();
                    ImGui::Text("Arguments:   %d", (int)args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        ImGui::BulletText("[%d] %s", (int)i,
                                          args[i].ToString().CStr());
                    }
                    if (!fn->description.Empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("Description: %s",
                                           fn->description.CStr());
                    }
                } else if (className == "Method") {
                    const BasePointer<MethodBase>& m =
                        ConstDeref<BasePointer<MethodBase>>(
                            SelectedTreeObject);
                    ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                       "Signature:");
                    ImGui::TextWrapped("%s", m->ToString().CStr());
                    ImGui::Separator();
                    ImGui::Text("Name:        %s",
                                m->GetName().ToString().CStr());
                    ImGui::Text("Class type:  %s",
                                m->GetClassType().ToString().CStr());
                    ImGui::Text("Return type: %s",
                                m->GetReturnType().ToString().CStr());
                    ImGui::Text(
                        "Const:       %s",
                        m->GetConstness() == Constness::Const ? "yes" : "no");
                    const auto& args = m->GetArgumentTypes();
                    ImGui::Text("Arguments:   %d", (int)args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        ImGui::BulletText("[%d] %s", (int)i,
                                          args[i].ToString().CStr());
                    }
                    if (!m->description.Empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("Description: %s",
                                           m->description.CStr());
                    }
                } else if (className == "Class") {
                    // Class objects hold a `const ClassBase *` describing a
                    // registered KAI type - list its methods and properties,
                    // Explorer-"properties dialog"-style.
                    const ClassBase* cls =
                        ConstDeref<const ClassBase*>(SelectedTreeObject);
                    if (!cls) {
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                           "(null class)");
                    } else {
                        ImGui::Text("Class name:  %s",
                                    cls->GetName().ToString().CStr());
                        ImGui::Text("Type number: %s",
                                    cls->GetTypeNumber().ToString().CStr());
                        ImGui::Separator();
                        const auto& methods = cls->GetMethods();
                        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                           "Methods (%d):",
                                           (int)methods.size());
                        for (const auto& kv : methods) {
                            MethodBase* mb = kv.second;
                            if (mb) {
                                ImGui::BulletText("%s", mb->ToString().CStr());
                            } else {
                                ImGui::BulletText(
                                    "%s", kv.first.ToString().CStr());
                            }
                        }
                        ImGui::Separator();
                        const auto& props = cls->GetProperties();
                        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                           "Properties (%d):",
                                           (int)props.size());
                        for (const auto& kv : props) {
                            ImGui::BulletText("%s",
                                              kv.first.ToString().CStr());
                        }
                    }
                } else {
                    // Any other instance (String, Int, Vector3, a
                    // user-registered type, ...): show its value, then the
                    // same Methods/Properties breakdown the "Class"
                    // descriptor branch above shows - but sourced from
                    // *this instance's* class, via SelectedTreeObject
                    // .GetClass(), rather than treating the object itself
                    // as a class descriptor.
                    ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                       "Value:");
                    ImGui::TextWrapped(
                        "%s", FormatStackValue(SelectedTreeObject).c_str());

                    const ClassBase* cls = SelectedTreeObject.GetClass();
                    if (cls) {
                        ImGui::Separator();
                        const auto& methods = cls->GetMethods();
                        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                           "Methods (%d):",
                                           (int)methods.size());
                        for (const auto& kv : methods) {
                            MethodBase* mb = kv.second;
                            if (!mb) {
                                ImGui::BulletText(
                                    "%s", kv.first.ToString().CStr());
                                continue;
                            }
                            // Double-click to invoke: the method pops its
                            // own arguments off (and pushes its result
                            // onto) the main data stack - the same one the
                            // Debugger/Pi/Rho tabs already share, rather
                            // than a separate scratch stack for arguments.
                            // A zero-arg method just runs; an N-arg method
                            // needs N values already pushed (e.g. via the
                            // Pi tab) before double-clicking. Either way we
                            // switch to the Pi tab afterward so the result
                            // (or, on failure, the stack as it stands) is
                            // immediately visible.
                            ImGui::Selectable(mb->ToString().CStr());
                            if (ImGui::IsItemHovered() &&
                                ImGui::IsMouseDoubleClicked(
                                    /* ImGuiMouseButton_Left */ 0)) {
                                try {
                                    mb->Invoke(SelectedTreeObject,
                                              *exec_->GetDataStack());
                                    AddLog("Invoked %s -> result pushed to stack",
                                          mb->ToString().CStr());
                                } catch (exception::Base& e) {
                                    AddLog(
                                        kErrorColor,
                                        "[Error] Failed to invoke %s: %s "
                                        "(push the required arguments onto "
                                        "the stack first)",
                                        mb->ToString().CStr(),
                                        e.ToString().CStr());
                                } catch (const std::exception& e) {
                                    AddLog(
                                        kErrorColor,
                                        "[Error] Failed to invoke %s: %s "
                                        "(push the required arguments onto "
                                        "the stack first)",
                                        mb->ToString().CStr(), e.what());
                                }
                                SwitchTab(ConsoleTab::Pi);
                            }
                        }
                        ImGui::Separator();
                        const auto& props = cls->GetProperties();
                        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                           "Properties (%d):",
                                           (int)props.size());
                        for (const auto& kv : props) {
                            ImGui::BulletText("%s",
                                              kv.first.ToString().CStr());
                        }
                    }
                }
            } catch (exception::Base& e) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                                   "Error reading this object: %s",
                                   e.ToString().CStr());
            } catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                                   "Error reading this object: %s", e.what());
            } catch (...) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                                   "Unknown error reading this object");
            }
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                               "Select an item in the tree to see details");
        }

        ImGui::EndChild();
    }

    void ExecCommand(const char* command_line) {
        // Add the command to the log first - a cyan pi/rho glyph instead of
        // the literal "Pi>"/"Rho>" text prompt.
        string cmdWithPrompt =
            (CurrentLanguage == Language::Pi) ? kPiPrompt : kRhoPrompt;
        cmdWithPrompt += command_line;
        AddLog(kPromptColor, "%s", cmdWithPrompt.c_str());

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
                        AddProcessResultLog(result.StdString());
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
                        AddProcessResultLog(result.StdString());
                    }
                }
            }

            // Show the Executor's current data stack state right in this
            // log, top-of-stack first (same order/format as the Debugger
            // tab's Data Stack panel) - this is the log reporting on the
            // Executor after the command ran, not a second stack widget
            // sitting next to the log.
            if (exec_) {
                int stackSize = exec_->GetDataStack()->Size();
                for (int i = 0; i < stackSize; i++) {
                    int displayIndex = stackSize - 1 - i;
                    auto obj = exec_->GetDataStack()->At(displayIndex);
                    AddStackLog(displayIndex, FormatStackValue(obj),
                               ColorForType(obj.GetTypeNumber()));
                }
            }
        } catch (exception::Base& e) {
            // Same Logger (Logs/kai.log) the Console app and the rest of
            // KAI use, so command failures here show up alongside
            // everything else instead of only in this window's in-memory
            // log.
            Logger::Error("ImGui Window command failed: " +
                          std::string(command_line) + " -> " +
                          e.ToString().CStr());

            // NOTE: this used to wrap the AddLog() call in a
            // PushStyleColor(ImGuiCol_Text, red)/PopStyleColor() pair, which
            // is a no-op here - AddLog() only stores the line's text for the
            // console to render on a later frame, it doesn't draw anything
            // itself, so an ambient style color pushed during the AddLog()
            // call has nothing to apply to. Use the colored AddLog()
            // overload instead, which stores the color alongside the text
            // so DrawConsoleContent() can render it correctly later.
            AddLog(kErrorColor, "[Error] %s", e.ToString().CStr());
        }

        // Commands like "pi"/"rho" switch the console's language directly
        // (Console::SwitchLanguageWithHistory) without going through
        // SwitchLanguage()/SwitchTab() above, so the GUI's tab and input
        // widget can get out of sync with the console's actual language.
        // Resync both from the console's real state after every command.
        Language consoleLang = console_.GetLanguage();
        if (consoleLang != CurrentLanguage &&
            (consoleLang == Language::Pi || consoleLang == Language::Rho)) {
            CurrentLanguage = consoleLang;
            exec_ = &*console_.GetExecutor();
            InputBuf[0] = '\0';
            MultilineInputBuf[0] = '\0';
            FocusInputNextFrame = true;
            CurrentTab = (CurrentLanguage == Language::Pi) ? ConsoleTab::Pi
                                                            : ConsoleTab::Rho;
        }
    }
};

void ShowExecutorWindow(bool* p_open) {
    static ExecutorWindow console;
    console.Draw("KAI Languages Console", p_open);
}

KAI_END
