#include "ExecutorWindow.h"

KAI_BEGIN

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
                                     ? object.GetClass()->GetName().ToString().c_str()
                                     : "";
        if (className == "Function") {
            return ConstDeref<BasePointer<FunctionBase>>(object)->ToString().c_str();
        }
        if (className == "Method") {
            return ConstDeref<BasePointer<MethodBase>>(object)->ToString().c_str();
        }
        if (className == "Class") {
            const ClassBase* cls = ConstDeref<const ClassBase*>(object);
            return cls ? ("Class: " + std::string(cls->GetName().ToString().c_str()))
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
        case Type::Number::Array:
            return ImVec4(0.4f, 0.75f, 1.0f, 1.0f);  // array - blue
        default:
            return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);  // everything else
    }
}

namespace {
// Neutral color for array punctuation that isn't itself a value - the
// ", " between elements. Kept separate from the array's own bracket color
// so brackets read as "this is an array" while commas stay unobtrusive.
const ImVec4 kArraySeparatorColor(0.85f, 0.85f, 0.85f, 1.0f);
}  // namespace

// Appends `object`'s formatted representation to `segments` as one or more
// (text, color) pairs. Non-array values append exactly one segment, colored
// the same as ColorForType(object.GetTypeNumber()) would give any other
// caller - this is what keeps a primitive's color identical whether it's
// sitting directly on the stack or nested inside an array. Arrays append
// their own bracket color for "[" and "]", a neutral color for the ", "
// separators, and recurse into each element (so nested arrays get their own
// nested brackets, each still in the array color).
void AppendStackValueSegments(const Object& object,
                              std::vector<std::pair<std::string, ImVec4>>& segments) {
    if (object.Exists() && object.GetTypeNumber() == Type::Number::Array) {
        Pointer<Array> arr = object;
        if (arr.Exists()) {
            segments.push_back({"[", ColorForType(Type::Number::Array)});
            int size = static_cast<int>(arr->Size());
            for (int i = 0; i < size; ++i) {
                AppendStackValueSegments(arr->At(i), segments);
                if (i + 1 < size) {
                    segments.push_back({", ", kArraySeparatorColor});
                }
            }
            segments.push_back({"]", ColorForType(Type::Number::Array)});
            return;
        }
    }
    segments.push_back({FormatStackValue(object), ColorForType(object.GetTypeNumber())});
}

std::vector<std::pair<std::string, ImVec4>> FormatStackValueSegments(const Object& object) {
    std::vector<std::pair<std::string, ImVec4>> segments;
    AppendStackValueSegments(object, segments);
    return segments;
}

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


    ExecutorWindow::ExecutorWindow() {
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
            Items[CurrentLanguage].push_back(LogLine{buf});
        }

        ScrollToBottom = true;
    }


    // Same as AddLog, but the line renders in `color` instead of the default
    // white - used for the prompt echo and for type-colored stack values.
    void ExecutorWindow::AddLog(const ImVec4& color, const char* fmt, ...) {
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
    void ExecutorWindow::AddStackLog(int index, const Object& value) {
        LogLine line;
        line.prefix = "[" + std::to_string(index) + "] ";
        line.segments = FormatStackValueSegments(value);
        // line.prefixColor left at its grey default; line.text/color unused
        // whenever line.segments is non-empty (see LogLine's comment).

        if (CurrentTab == ConsoleTab::Debugger) {
            std::string plain = line.prefix;
            for (const auto& seg : line.segments) plain += seg.first;
            DebugLog.push_back(plain);
        } else {
            Items[CurrentLanguage].push_back(std::move(line));
        }

        ScrollToBottom = true;
    }



    // Console::Process() (Ext/CppKaiCore/.../Console.cpp) catches parse and
    // execution errors internally and returns them as plain result text
    // ("Exception: ...", "StdException: ...", "UnknownException:") rather
    // than letting them propagate - so ExecCommand's own
    // catch (Exception::Base&) block never sees a syntax error like an
    // unmatched brace; it only ever sees what Process() decided to hand
    // back as a normal string. Route that string through here so it still
    // renders red/"[Error]" like a caught exception would, instead of
    // silently blending in as a plain white log line.
    void ExecutorWindow::AddProcessResultLog(const std::string& result) {
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

    void ExecutorWindow::DumpDataStackToLog() {
        if (!exec_) return;
        int stackSize = exec_->GetDataStack()->Size();
        for (int i = 0; i < stackSize; i++) {
            int displayIndex = stackSize - 1 - i;
            auto obj = exec_->GetDataStack()->At(displayIndex);
            AddStackLog(displayIndex, obj);
        }
    }


    void ExecutorWindow::SwitchLanguage(Language lang) {
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


    void ExecutorWindow::SwitchTab(ConsoleTab tab) {
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


    void ExecutorWindow::Draw(const char* title, bool* p_open) {
        // Sized/positioned to leave clear room on the right for the KAI
        // Settings window (and the optional ImGui Demo window) at their own
        // default positions, on a typical 1440x960 first launch - previously
        // this had no explicit position and sized to 1080x820, which left so
        // little free space that Settings/Demo defaulted to opening
        // stacked directly on top of it, making both look "stuck"/
        // unresizable when really only overlapped.
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(900, 900), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open)) {
            ImGui::End();
            return;
        }

        DrawImGuiWindowControls(WindowLayout);

        // Create styled tab selection buttons with tab-like appearance
        float tabWidth = ImGui::GetContentRegionAvailWidth() / 5.0f - 4.0f;
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
        ImGui::SameLine();

        // Assistant Tab Button - talks to a local cppcoder chat server, see
        // ExecutorWindowAssistant.cpp.
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (CurrentTab == ConsoleTab::Assistant)
                                  ? ImVec4(0.3f, 0.6f, 0.8f, 1.0f)
                                  : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (CurrentTab == ConsoleTab::Assistant)
                                  ? ImVec4(0.4f, 0.7f, 0.9f, 1.0f)
                                  : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        if (CurrentTab == ConsoleTab::Assistant) {
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImGui::GetCursorScreenPos(),
                ImVec2(ImGui::GetCursorScreenPos().x + tabWidth,
                       ImGui::GetCursorScreenPos().y + tabHeight + 1),
                ImGui::GetColorU32(ImVec4(0.3f, 0.6f, 0.8f, 1.0f)), 4.0f,
                ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_TopRight);
        }

        if (ImGui::Button("Assistant", ImVec2(tabWidth, tabHeight - 4))) {
            SwitchTab(ConsoleTab::Assistant);
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
        } else if (CurrentTab == ConsoleTab::Assistant) {
            DrawAssistantContent();
        } else {
            DrawConsoleContent();
        }

        ImGui::End();
    }


void ShowExecutorWindow(bool* p_open) {
    static ExecutorWindow console;
    console.Draw("KAI Languages Console", p_open);
}

KAI_END
