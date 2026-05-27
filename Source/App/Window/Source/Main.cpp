#include <GLFW/glfw3.h>
#include <KAI/Console/Console.h>
#include <imgui.h>

#include <ctime>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

KAI_BEGIN

void ShowExecutorWindow(bool*);

KAI_END

USING_NAMESPACE_KAI

static void error_callback(int error, const char* description) {
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

    return window;
}

static void LoadFont() {
    ImGuiIO& io = ImGui::GetIO();
    // Scale the shared default font up slightly so every theme inherits the
    // same larger baseline text size.
    io.FontGlobalScale = kDefaultFontScale;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    GLFWwindow* window = SetupGui();
    if (!window) return -1;

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

        ImGui::SetNextWindowSize(ImVec2(380, 160), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("KAI Settings", nullptr, ImGuiWindowFlags_MenuBar);
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
            ImGui::SetNextWindowPos(ImVec2(650, 150), ImGuiCond_FirstUseEver);
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

    return 0;
}
