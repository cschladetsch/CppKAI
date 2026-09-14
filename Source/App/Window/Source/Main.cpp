#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <KAI/Console/Console.h>
#include <KAI/Core/Logger.h>
#include <imgui.h>

#include <ctime>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "ImGuiWindowControls.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

KAI_BEGIN

void ShowExecutorWindow(bool*);

KAI_END

USING_NAMESPACE_KAI

static void error_callback(int error, const char* description) {
    // Route through KAI's common Logger (Logs/kai.log), same as Console,
    // instead of a raw cerr print that nothing else ever sees.
    Logger::Error("GLFW error " + std::to_string(error) + ": " +
                  (description ? description : "(no description)"));
    cerr << "Error " << error << ": " << description << endl;
}

enum class ThemePreset { Dark, Minimal, Industrial, Neon, System };

static const char* kLayoutIniPath = "./Bin/ImGui.imgui.ini";
static const char* kThemeSettingsPath = "./Bin/ImGui.theme";
static constexpr float kDefaultFontScale = 1.10f;

static const char* ThemeName(ThemePreset preset) {
    switch (preset) {
        case ThemePreset::Dark:
            return "Dark";
        case ThemePreset::Minimal:
            return "Minimal";
        case ThemePreset::Industrial:
            return "Industrial";
        case ThemePreset::Neon:
            return "Neon";
        case ThemePreset::System:
            return "System";
    }
    return "Dark";
}

static ThemePreset ThemeFromString(const std::string& value) {
    if (value == "Minimal") return ThemePreset::Minimal;
    if (value == "Industrial") return ThemePreset::Industrial;
    if (value == "Neon") return ThemePreset::Neon;
    if (value == "System") return ThemePreset::System;
    return ThemePreset::Dark;
}

static ThemePreset LoadThemePreference() {
    std::ifstream input(kThemeSettingsPath);
    std::string value;
    if (input >> value) return ThemeFromString(value);
    return ThemePreset::Dark;
}

static void SaveThemePreference(ThemePreset preset) {
    std::ofstream output(kThemeSettingsPath, std::ios::trunc);
    output << ThemeName(preset) << '\n';
}

static void ApplyTheme(ThemePreset preset) {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    switch (preset) {
        case ThemePreset::Dark:
            ImGui::StyleColorsDark();
            style.WindowRounding = 6.0f;
            style.FrameRounding = 4.0f;
            style.GrabRounding = 4.0f;
            break;

        case ThemePreset::Minimal:
            ImGui::StyleColorsLight();
            style.WindowRounding = 2.0f;
            style.FrameRounding = 2.0f;
            style.GrabRounding = 2.0f;
            colors[ImGuiCol_WindowBg] = ImVec4(0.96f, 0.96f, 0.95f, 1.0f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.90f, 0.90f, 0.89f, 1.0f);
            colors[ImGuiCol_TitleBgActive] =
                ImVec4(0.84f, 0.84f, 0.83f, 1.0f);
            colors[ImGuiCol_Button] = ImVec4(0.83f, 0.83f, 0.82f, 1.0f);
            colors[ImGuiCol_ButtonHovered] =
                ImVec4(0.75f, 0.75f, 0.74f, 1.0f);
            colors[ImGuiCol_ButtonActive] =
                ImVec4(0.68f, 0.68f, 0.67f, 1.0f);
            break;

        case ThemePreset::Industrial:
            ImGui::StyleColorsDark();
            style.WindowRounding = 1.0f;
            style.FrameRounding = 1.0f;
            style.GrabRounding = 1.0f;
            colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.0f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.22f, 0.21f, 0.18f, 1.0f);
            colors[ImGuiCol_TitleBgActive] =
                ImVec4(0.35f, 0.26f, 0.12f, 1.0f);
            colors[ImGuiCol_Header] = ImVec4(0.42f, 0.29f, 0.10f, 1.0f);
            colors[ImGuiCol_HeaderHovered] =
                ImVec4(0.56f, 0.38f, 0.12f, 1.0f);
            colors[ImGuiCol_Button] = ImVec4(0.38f, 0.28f, 0.12f, 1.0f);
            colors[ImGuiCol_ButtonHovered] =
                ImVec4(0.52f, 0.37f, 0.12f, 1.0f);
            colors[ImGuiCol_ButtonActive] =
                ImVec4(0.64f, 0.46f, 0.15f, 1.0f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.19f, 0.18f, 1.0f);
            break;

        case ThemePreset::Neon:
            ImGui::StyleColorsDark();
            style.WindowRounding = 8.0f;
            style.FrameRounding = 6.0f;
            style.GrabRounding = 6.0f;
            colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.06f, 0.09f, 1.0f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.10f, 0.16f, 1.0f);
            colors[ImGuiCol_TitleBgActive] =
                ImVec4(0.08f, 0.18f, 0.26f, 1.0f);
            colors[ImGuiCol_Header] = ImVec4(0.09f, 0.30f, 0.35f, 1.0f);
            colors[ImGuiCol_HeaderHovered] =
                ImVec4(0.10f, 0.52f, 0.56f, 1.0f);
            colors[ImGuiCol_Button] = ImVec4(0.15f, 0.18f, 0.42f, 1.0f);
            colors[ImGuiCol_ButtonHovered] =
                ImVec4(0.26f, 0.31f, 0.72f, 1.0f);
            colors[ImGuiCol_ButtonActive] =
                ImVec4(0.37f, 0.43f, 0.90f, 1.0f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.10f, 0.14f, 1.0f);
            colors[ImGuiCol_CheckMark] = ImVec4(0.22f, 0.98f, 0.77f, 1.0f);
            colors[ImGuiCol_SliderGrab] = ImVec4(0.98f, 0.20f, 0.75f, 1.0f);
            colors[ImGuiCol_SliderGrabActive] =
                ImVec4(0.99f, 0.42f, 0.84f, 1.0f);
            break;

        case ThemePreset::System:
            ImGui::StyleColorsClassic();
            style.WindowRounding = 4.0f;
            style.FrameRounding = 3.0f;
            style.GrabRounding = 3.0f;
            break;
    }
}

static void ResetLayoutAndTheme(ThemePreset& active_theme) {
    std::remove(kLayoutIniPath);
    std::remove(kThemeSettingsPath);

    active_theme = ThemePreset::Dark;
    ApplyTheme(active_theme);
    SaveThemePreference(active_theme);
    ImGui::LoadIniSettingsFromDisk(kLayoutIniPath);
}

static std::string MakeWindowTitle() {
    std::time_t now = std::time(nullptr);
    std::tm local_time = *std::localtime(&now);

    char version[32];
    std::strftime(version, sizeof(version), "v%y-%m-%d-%H", &local_time);

    return std::string("KAI ImGui ") + version;
}

static GLFWwindow* SetupGui() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return nullptr;

    // Keep the existing compatibility context and use the matching GLSL
    // version string for the official OpenGL3 backend.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    const std::string title = MakeWindowTitle();
    GLFWwindow* window =
        glfwCreateWindow(1440, 960, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = kLayoutIniPath;

    // Load the default (ASCII) font, then merge in real Greek glyphs
    // (U+0370-U+03FF, covers pi U+03C0/rho U+03C1 used by the Pi/Rho
    // console prompt) from a system font on top of it, into the same
    // ImFont - the standard ImGui pattern for adding glyphs the base font
    // doesn't have (same technique used for icon fonts). This has to
    // happen before ImGui_ImplOpenGL3_Init() below, which builds the GPU
    // font texture from whatever's in io.Fonts at that point - rebuilding
    // the atlas *after* Init() (what an earlier attempt here did, via
    // io.Fonts->Clear()/CreateFontsTexture()) crashed with no useful
    // diagnostic, so this avoids that path entirely rather than trying to
    // fix it.
    io.Fonts->AddFontDefault();
    static const ImWchar greekRanges[] = {0x0370, 0x03FF, 0};
    ImFontConfig greekMergeConfig;
    greekMergeConfig.MergeMode = true;
    ImFont* greekFont = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\arial.ttf", 13.0f, &greekMergeConfig,
        greekRanges);
    if (!greekFont) {
        // Missing/unreadable font file - AddFontFromFileTTF() returns
        // nullptr rather than throwing, so this is safe to just log and
        // continue with ASCII-only glyphs (pi/rho would render as a
        // missing-glyph box, same as before this change).
        Logger::Warning(
            "KAI ImGui Window: could not load "
            "C:\\Windows\\Fonts\\arial.ttf for Greek glyph support "
            "(pi/rho prompt symbols will show as missing-glyph boxes)");
    }

    ApplyTheme(LoadThemePreference());

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 120")) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

	// Initialize GLEW extension loader to prevent null function pointer crashes
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
	    glfwDestroyWindow(window);
	    glfwTerminate();
	    return nullptr;
	}

    // NOTE: this used to call IMGUI_CHECKVERSION()/ImGui::CreateContext()
    // again here, on top of the context already created and wired up to
    // the GLFW/OpenGL3 backends above (lines ~197-219). That second call
    // silently swapped in a fresh, backend-less context as "current" -
    // nothing noticed before because nothing looked up the OpenGL3
    // backend's own state directly, but LoadFont() now calls
    // ImGui_ImplOpenGL3_CreateFontsTexture() explicitly, which does look
    // it up on the current context and crashed since that second context
    // was never Init()'d. The redundant glfwMakeContextCurrent()/
    // glfwSwapInterval() right above it (duplicating lines ~194-195) are
    // gone too, for the same reason: harmless on their own, but signs of
    // this function having been pasted together twice.

    return window;
}

static void LoadFont() {
    ImGuiIO& io = ImGui::GetIO();
    // Scale the shared default font up slightly so every theme inherits the
    // same larger baseline text size.
    io.FontGlobalScale = kDefaultFontScale;

    // Previously attempted rebuilding the font atlas here to add a Greek
    // glyph range (for a symbolic pi/rho console prompt) - reverted after
    // it caused a silent crash on startup with no useful diagnostic in
    // Logs/kai.log. Revisit with a real Unicode-covering .ttf file (via
    // AddFontFromFileTTF()) and a debugger attached, rather than rebuilding
    // the built-in default font's atlas blind.
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // Use the same Logger (Logs/kai.log) as Console and the rest of KAI,
    // instead of the Window app's own separate/ad-hoc logging.
    Logger::Init();
    Logger::Info("KAI ImGui Window starting");

    GLFWwindow* window = SetupGui();
    if (!window) {
        Logger::Error("KAI ImGui Window: SetupGui() failed, exiting");
        return -1;
    }

    LoadFont();

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ThemePreset active_theme = LoadThemePreference();
    ApplyTheme(active_theme);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowExecutorWindow(nullptr);

        // Positioned clear of the Console window's default 900x900 rect at
        // (20,20) (see ExecutorWindowCore.cpp's Draw()) so the two don't
        // default to stacking on top of each other.
        ImGui::SetNextWindowSize(ImVec2(480, 220), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(940, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("KAI Settings", nullptr, ImGuiWindowFlags_MenuBar);
        static ImGuiWindowLayoutState settingsWindowLayout;
        DrawImGuiWindowControls(settingsWindowLayout);
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Theme")) {
                const ThemePreset presets[] = {ThemePreset::Dark,
                                               ThemePreset::Minimal,
                                               ThemePreset::Industrial,
                                               ThemePreset::Neon,
                                               ThemePreset::System};
                for (ThemePreset preset : presets) {
                    const bool selected = active_theme == preset;
                    if (ImGui::MenuItem(ThemeName(preset), nullptr,
                                        selected)) {
                        active_theme = preset;
                        ApplyTheme(active_theme);
                        SaveThemePreference(active_theme);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::ColorEdit3("Background Color", (float*)&clear_color);
        if (ImGui::Button("ImGui Demo"))
            show_demo_window = !show_demo_window;
        ImGui::SameLine();
        ImGui::TextUnformatted(ThemeName(active_theme));
        if (ImGui::Button("Reset Layout")) {
            ResetLayoutAndTheme(active_theme);
        }
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::End();

        if (show_demo_window) {
            // Below KAI Settings (see its pos/size above), clear of both it
            // and the Console window's default rect.
            ImGui::SetNextWindowPos(ImVec2(940, 260), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Render();

        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::SaveIniSettingsToDisk(kLayoutIniPath);
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    Logger::Info("KAI ImGui Window exiting");

    return 0;
}
