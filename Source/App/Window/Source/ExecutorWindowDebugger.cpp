#include "ExecutorWindow.h"

KAI_BEGIN

    void ExecutorWindow::DrawDebuggerContent() {
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

                if (ImGui::Selectable(st.ToString().c_str(), WatchIndex == i)) {
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


    void ExecutorWindow::ExecuteDebugStep() {
        DebugStepCount++;
        AddLog("Step %d", DebugStepCount);

        // Show current executor state
        StringStream st;
        st << "Data Stack Size: " << exec_->GetDataStack()->Size();
        AddLog("%s", st.ToString().c_str());

        // Show all stack items
        if (exec_->GetDataStack()->Size() > 0) {
            for (int i = 0; i < exec_->GetDataStack()->Size(); i++) {
                auto obj = exec_->GetDataStack()->At(i);
                int displayIndex = exec_->GetDataStack()->Size() - 1 - i;
                StringStream itemSt;
                itemSt << "  [" << displayIndex << "]: "
                       << FormatStackValue(obj).c_str();
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
            Logger::Error("ImGui Window debug step failed: " +
                          std::string(e.ToString().c_str()));
            AddLog("Debug operation failed: %s", e.ToString().c_str());
        }
    }


KAI_END
