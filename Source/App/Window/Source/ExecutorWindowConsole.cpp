#include "ExecutorWindow.h"

KAI_BEGIN

    void ExecutorWindow::DrawConsoleContent() {
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
            if (!item.segments.empty()) {
                // Multi-color body (e.g. an array: bracket color for "[" "]",
                // each element in its own type's color) - render each piece
                // with SameLine so they all stay on one log line.
                for (size_t s = 0; s < item.segments.size(); ++s) {
                    if (s > 0) ImGui::SameLine(0.0f, 0.0f);
                    ImGui::PushStyleColor(ImGuiCol_Text, item.segments[s].second);
                    ImGui::TextUnformatted(item.segments[s].first.c_str());
                    ImGui::PopStyleColor();
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, item.color);
                ImGui::TextUnformatted(item.text.c_str());
                ImGui::PopStyleColor();
            }
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
                } else {
                    // Bare Enter with nothing typed - just show the current
                    // stack rather than doing nothing.
                    DumpDataStackToLog();
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
                } else {
                    // Bare Ctrl+Enter/Execute with nothing typed - just show
                    // the current stack rather than doing nothing.
                    DumpDataStackToLog();
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


KAI_END
