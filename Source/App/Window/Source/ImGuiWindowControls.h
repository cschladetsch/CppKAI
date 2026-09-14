#pragma once

#include <imgui.h>

// Small reusable maximize/minimize/restore control strip for ImGui windows.
//
// ImGui's own title bar (the one drawn by Begin()) has no extension point
// for adding custom buttons to it without reaching into ImGui's internal
// (imgui_internal.h) API, so this instead draws a compact "_ [] <>" button
// row as the very first thing inside the window body - call it right after
// ImGui::Begin() succeeds, before any other content. Because it's the very
// first item drawn (before the console log's/tree's/debugger's own
// scrolling BeginChild() regions), it stays put at the top of the window
// regardless of what's scrolled below it.
//
// (An earlier version of this tried to pin the row into the title bar's own
// screen-space rect via SetCursorScreenPos, to survive scrolling - that
// broke both button rendering and window resizing on this ImGui version
// (1.67), almost certainly because that rect sits outside the window's
// content clip region here. Reverted to this simpler, known-working
// approach.)
//
// - "_"  (minimize): collapses the window - the same effect as clicking
//   ImGui's own collapse triangle in the title bar. Click the title bar's
//   triangle (or drag/click the collapsed title bar) to restore it - ImGui
//   already handles that half for free.
// - "[]" (maximize): resizes/repositions the window to fill the OS window
//   (io.DisplaySize - this ImGui version predates viewports/GetMainViewport),
//   remembering the window's prior position/size.
// - "<>" (restore): shown in place of "[]" once maximized; puts the window
//   back exactly where "[]" found it.
//
// Deliberately not KAI-namespaced or KAI-dependent (just ImGui) so both
// ExecutorWindow.h/.cpp and Main.cpp can use it without coupling Main.cpp to
// the KAI namespace.
struct ImGuiWindowLayoutState {
    bool maximized = false;
    ImVec2 savedPos = ImVec2(0, 0);
    ImVec2 savedSize = ImVec2(0, 0);
};

// Draws the minimize/maximize-or-restore row (right-aligned) plus a
// trailing separator, for the CURRENT window (the one from the most recent
// ImGui::Begin()).
inline void DrawImGuiWindowControls(ImGuiWindowLayoutState& state) {
    const float buttonSize = ImGui::GetFrameHeight();
    // The maximize/restore label ("[]"/"<>") is two characters - a button
    // this square can't fit them, so their glyphs overflow the button rect
    // and get clipped by the window's right edge (this is what showed up as
    // "only a stray '[' visible, nothing after it"). Give that one button
    // extra width instead of trying to force two glyphs into a
    // single-character-wide square.
    const float wideButtonWidth = buttonSize * 1.6f;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float totalWidth = buttonSize + spacing + wideButtonWidth;

    // Right-align the button pair within the window's current width; if the
    // window's too narrow for that, just fall back to wherever the cursor
    // already is rather than pushing it negative.
    float x = ImGui::GetWindowWidth() - totalWidth - spacing;
    if (x > 0.0f) ImGui::SetCursorPosX(x);

    // Distinct IDs per window instance (state's address), since every
    // window using this draws the same "_"/"[]"/"<>" labels and would
    // otherwise collide in ImGui's ID stack.
    ImGui::PushID(&state);

    if (ImGui::Button("_", ImVec2(buttonSize, buttonSize))) {
        // Applies to the current window; ImGui's own title bar triangle (or
        // clicking the collapsed title bar) restores it again.
        ImGui::SetWindowCollapsed(true);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Minimize");
    ImGui::SameLine(0.0f, spacing);

    if (!state.maximized) {
        if (ImGui::Button("[]", ImVec2(wideButtonWidth, buttonSize))) {
            state.savedPos = ImGui::GetWindowPos();
            state.savedSize = ImGui::GetWindowSize();
            state.maximized = true;
            // This ImGui version (1.67) predates GetMainViewport()/
            // ImGuiViewport - there's only ever the one OS window, and
            // io.DisplaySize is its full framebuffer size in screen
            // coordinates, so that (with a (0,0) origin) is "the viewport".
            ImGuiIO& io = ImGui::GetIO();
            ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetWindowSize(io.DisplaySize);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Maximize");
    } else {
        if (ImGui::Button("<>", ImVec2(wideButtonWidth, buttonSize))) {
            state.maximized = false;
            ImGui::SetWindowPos(state.savedPos);
            ImGui::SetWindowSize(state.savedSize);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Restore");
    }

    ImGui::PopID();

    ImGui::Separator();
}
