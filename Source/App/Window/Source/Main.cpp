#include <GLFW/glfw3.h>
#include <KAI/Console/Console.h>
#include <imgui.h>

#include <ctime>
#include <iostream>

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

static std::string MakeWindowTitle() {
    std::time_t now = std::time(nullptr);
    std::tm local_time = *std::localtime(&now);

    char version[32];
    std::strftime(version, sizeof(version), "v%y-%m-%d-%H", &local_time);

    return std::string("KAI Window ") + version;
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
        glfwCreateWindow(800, 600, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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
    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 1;
    config.GlyphExtraSpacing.x = 1.0f;
    (void)config;
    // Use default font for now. The backend uploads it automatically.
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    GLFWwindow* window = SetupGui();
    if (!window) return -1;

    LoadFont();

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ShowExecutorWindow(nullptr);

        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::Begin("KAI Settings");
        ImGui::ColorEdit3("Background Color", (float*)&clear_color);
        if (ImGui::Button("ImGui Demo"))
            show_demo_window = !show_demo_window;
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
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
