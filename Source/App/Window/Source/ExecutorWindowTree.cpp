#include "ExecutorWindow.h"

KAI_BEGIN

    void ExecutorWindow::RenderTreeObjectNode(const Object& node, const std::string& label,
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
                    ? node.GetClass()->GetName().ToString().c_str()
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
        } catch (Exception::Base& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "%s  (error: %s)", label.c_str(),
                               e.ToString().c_str());
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
                children.emplace_back(entry.first.ToString().c_str(),
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
        } catch (Exception::Base& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "  (error listing children: %s)",
                               e.ToString().c_str());
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "  (error listing children: %s)", e.what());
        } catch (...) {
            ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                               "  (unknown error listing children)");
        }
    }


    void ExecutorWindow::DrawTreeContent() {
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
                              .c_str()
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
                    ImGui::TextWrapped("%s", fn->ToString().c_str());
                    ImGui::Separator();
                    ImGui::Text("Name:        %s",
                                fn->GetName().ToString().c_str());
                    ImGui::Text("Return type: %s",
                                fn->GetReturnType().ToString().c_str());
                    const auto& args = fn->GetArgumentTypes();
                    ImGui::Text("Arguments:   %d", (int)args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        ImGui::BulletText("[%d] %s", (int)i,
                                          args[i].ToString().c_str());
                    }
                    if (!fn->Description.Empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("Description: %s",
                                           fn->Description.c_str());
                    }
                } else if (className == "Method") {
                    const BasePointer<MethodBase>& m =
                        ConstDeref<BasePointer<MethodBase>>(
                            SelectedTreeObject);
                    ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                       "Signature:");
                    ImGui::TextWrapped("%s", m->ToString().c_str());
                    ImGui::Separator();
                    ImGui::Text("Name:        %s",
                                m->GetName().ToString().c_str());
                    ImGui::Text("Class type:  %s",
                                m->GetClassType().ToString().c_str());
                    ImGui::Text("Return type: %s",
                                m->GetReturnType().ToString().c_str());
                    ImGui::Text(
                        "Const:       %s",
                        m->GetConstness() == Constness::Const ? "yes" : "no");
                    const auto& args = m->GetArgumentTypes();
                    ImGui::Text("Arguments:   %d", (int)args.size());
                    for (size_t i = 0; i < args.size(); ++i) {
                        ImGui::BulletText("[%d] %s", (int)i,
                                          args[i].ToString().c_str());
                    }
                    if (!m->Description.Empty()) {
                        ImGui::Separator();
                        ImGui::TextWrapped("Description: %s",
                                           m->Description.c_str());
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
                                    cls->GetName().ToString().c_str());
                        ImGui::Text("Type number: %s",
                                    cls->GetTypeNumber().ToString().c_str());
                        ImGui::Separator();
                        const auto& methods = cls->GetMethods();
                        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                           "Methods (%d):",
                                           (int)methods.size());
                        for (const auto& kv : methods) {
                            MethodBase* mb = kv.second;
                            if (mb) {
                                ImGui::BulletText("%s", mb->ToString().c_str());
                            } else {
                                ImGui::BulletText(
                                    "%s", kv.first.ToString().c_str());
                            }
                        }
                        ImGui::Separator();
                        const auto& props = cls->GetProperties();
                        ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f),
                                           "Properties (%d):",
                                           (int)props.size());
                        for (const auto& kv : props) {
                            ImGui::BulletText("%s",
                                              kv.first.ToString().c_str());
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
                                    "%s", kv.first.ToString().c_str());
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
                            ImGui::Selectable(mb->ToString().c_str());
                            if (ImGui::IsItemHovered() &&
                                ImGui::IsMouseDoubleClicked(
                                    /* ImGuiMouseButton_Left */ 0)) {
                                try {
                                    mb->Invoke(SelectedTreeObject,
                                              *exec_->GetDataStack());
                                    AddLog("Invoked %s -> result pushed to stack",
                                          mb->ToString().c_str());
                                } catch (Exception::Base& e) {
                                    AddLog(
                                        kErrorColor,
                                        "[Error] Failed to invoke %s: %s "
                                        "(push the required arguments onto "
                                        "the stack first)",
                                        mb->ToString().c_str(),
                                        e.ToString().c_str());
                                } catch (const std::exception& e) {
                                    AddLog(
                                        kErrorColor,
                                        "[Error] Failed to invoke %s: %s "
                                        "(push the required arguments onto "
                                        "the stack first)",
                                        mb->ToString().c_str(), e.what());
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
                                              kv.first.ToString().c_str());
                        }
                    }
                }
            } catch (Exception::Base& e) {
                ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
                                   "Error reading this object: %s",
                                   e.ToString().c_str());
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


KAI_END
