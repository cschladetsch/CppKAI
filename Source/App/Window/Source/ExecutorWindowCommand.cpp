#include "ExecutorWindow.h"

KAI_BEGIN

    void ExecutorWindow::ExecCommand(const char* command_line) {
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
                if (substituted.Size() > 0) {
                    AddLog("=> %s", substituted.CStr());

                    // Process the substituted command
                    String expandedText =
                        console_.ExpandShellCommands(substituted);
                    String result = console_.Process(expandedText);
                    if (!result.Empty()) {
                        AddProcessResultLog(result.StdString());
                    }
                } else {
                    AddLog("Substitution failed: no match found");
                }
            } else if (!text.empty()) {
                // Check for shell commands
                if (text[0] == '`') {
                    String output = console_.ProcessShellCommand(String(text));
                    AddLog("%s", output.CStr());
                } else {
                    // Handle zsh-like history commands
                    std::string processedText = text;

                    // If it's a pure history command (just !!, !n, etc), expand
                    // it
                    if (text[0] == '!' && text.find(' ') == std::string::npos) {
                        String expanded =
                            console_.ProcessZshCommand(String(text));
                        if (expanded.Size() > 0) {
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
                    if (!result.Empty()) {
                        AddProcessResultLog(result.StdString());
                    }
                }
            }

            // Show the Executor's current data stack state right in this
            // log, top-of-stack first (same order/format as the Debugger
            // tab's Data Stack panel) - this is the log reporting on the
            // Executor after the command ran, not a second stack widget
            // sitting next to the log.
            DumpDataStackToLog();
        } catch (exception::Base& e) {
            // Same Logger (Logs/kai.log) the Console app and the rest of
            // KAI use, so command failures here show up alongside
            // everything else instead of only in this window's in-memory
            // log.
            Logger::Error("ImGui Window command failed: " +
                          std::string(command_line) + " -> " +
                          e.ToString().c_str());

            // NOTE: this used to wrap the AddLog() call in a
            // PushStyleColor(ImGuiCol_Text, red)/PopStyleColor() pair, which
            // is a no-op here - AddLog() only stores the line's text for the
            // console to render on a later frame, it doesn't draw anything
            // itself, so an ambient style color pushed during the AddLog()
            // call has nothing to apply to. Use the colored AddLog()
            // overload instead, which stores the color alongside the text
            // so DrawConsoleContent() can render it correctly later.
            AddLog(kErrorColor, "[Error] %s", e.ToString().c_str());
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


KAI_END
