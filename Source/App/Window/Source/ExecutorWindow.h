#pragma once

#include <KAI/Console.h>
#include <KAI/Core/Exception.h>
#include <KAI/Core/FunctionBase.h>
#include <KAI/Core/Logger.h>
#include <KAI/Core/Object/ClassBase.h>
#include <KAI/Core/Object/GetStorageBase.h>
#include <KAI/Core/Object/MethodBase.h>
#include <KAI/Executor/Continuation.h>
#include <imgui.h>

#include "ImGuiWindowControls.h"

#include <cstring>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// ExecutorWindow used to be a single ~1700-line ExecutorWindow.cpp; split
// (2026-09-14) into this header plus ExecutorWindowCore/Console/Debugger/
// Tree/Command.cpp, grouped by responsibility, since the single file had
// grown too large to navigate comfortably. `using namespace std;` below
// matches the original file's top-level using-declaration - every .cpp
// that includes this header inherits it, same net effect as before the
// split.
using namespace std;

KAI_BEGIN

// Formats a stack/tree value for display - see ExecutorWindowCore.cpp for
// why Function/Method/Class objects need special-casing.
std::string FormatStackValue(const Object& object);

// Color-codes a log line by the KAI value type it represents (ints green,
// floats orange, strings yellow, bools purple, arrays blue, everything else
// grey).
ImVec4 ColorForType(Type::Number t);

// Like FormatStackValue, but broken into (text, color) segments so a
// container's brackets/commas and its primitive elements can each carry
// their own ColorForType color on one rendered line, instead of the whole
// value sharing a single color. For a non-container value this is just a
// single segment equivalent to {FormatStackValue(object), ColorForType(...)}.
// Recurses for nested arrays.
std::vector<std::pair<std::string, ImVec4>> FormatStackValueSegments(
    const Object& object);

// InputTextMultiline callback that turns a typed Tab into 4 spaces for the
// Rho editor - see ExecutorWindowCore.cpp for why.
int RhoTabToSpacesCallback(ImGuiInputTextCallbackData* data);

// Cyan prompt shown before each echoed command: a real pi/rho glyph
// (U+03C0/U+03C1). Requires the Greek glyph merge in Main.cpp's
// SetupGui() (merges a system font's Greek range into the default font
// atlas *before* ImGui_ImplOpenGL3_Init() builds the GPU texture) -
// without that merge these render as missing-glyph boxes.
inline constexpr const char* kPiPrompt = "\xCF\x80 ";   // UTF-8 for U+03C0 GREEK SMALL LETTER PI
inline constexpr const char* kRhoPrompt = "\xCF\x81 ";  // UTF-8 for U+03C1 GREEK SMALL LETTER RHO
inline const ImVec4 kPromptColor(0.3f, 0.9f, 0.95f, 1.0f);  // cyan
inline const ImVec4 kErrorColor(1.0f, 0.35f, 0.35f, 1.0f);  // red, for "[Error] ..." log lines

// Enum for the available tabs in the console window
enum class ConsoleTab { Pi, Rho, Debugger, Tree };

// A tabbed console with Pi, Rho, and Debugger tabs
struct ExecutorWindow {
    // Minimize/maximize/restore state for this window's own title-bar-like
    // button row - see ImGuiWindowControls.h.
    ImGuiWindowLayoutState WindowLayout;

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
        // Optional multi-color body, used instead of plain `text`/`color`
        // when a value's own parts need distinct colors on one line - e.g.
        // an array's "[", "]" and commas in the array color, but each
        // element in its own type's color. Empty means "just use text/color"
        // (every non-container AddLog/AddStackLog call leaves this empty).
        std::vector<std::pair<std::string, ImVec4>> segments;
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


    ExecutorWindow();

    // Defined in ExecutorWindowCore.cpp
        void ClearLog(Language lang = Language::None);
        void ClearAllLogs();
        void AddLog(const char* fmt, ...);
        void AddLog(const ImVec4& color, const char* fmt, ...);
        void AddStackLog(int index, const Object& value);
        void AddProcessResultLog(const std::string& result);
        // Logs the Executor's current data stack, top-first, one AddStackLog
        // line per slot - the same dump ExecCommand does after every
        // command. Also called directly on a bare Enter (empty input), so
        // pressing Enter with nothing typed is a quick "show me the stack"
        // rather than a no-op.
        void DumpDataStackToLog();
        void SwitchLanguage(Language lang);
        void SwitchTab(ConsoleTab tab);
        void Draw(const char* title, bool* p_open);

    // Defined in ExecutorWindowConsole.cpp
        void DrawConsoleContent();

    // Defined in ExecutorWindowDebugger.cpp
        void DrawDebuggerContent();
        void ExecuteDebugStep();

    struct TreePopGuard {
        bool active;
        explicit TreePopGuard(bool a) : active(a) {}
        ~TreePopGuard() {
            if (active) ImGui::TreePop();
        }
    };

    // Defined in ExecutorWindowTree.cpp
        void RenderTreeObjectNode(const Object& node, const std::string& label,
                              const std::string& path, std::set<int>& seen,
                              int depth);
        void DrawTreeContent();

    // Defined in ExecutorWindowCommand.cpp
        void ExecCommand(const char* command_line);
};

void ShowExecutorWindow(bool* p_open);

KAI_END
