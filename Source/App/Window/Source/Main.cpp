#include <GLFW/glfw3.h>
#include <KAI/Console/Console.h>
#include <imgui.h>
#include <stdio.h>

#include <iostream>

using namespace std;
using namespace ImGui;

KAI_BEGIN

void ShowExecutorWindow(bool*);

KAI_END

USING_NAMESPACE_KAI

// Simple ImGui implementation for GLFW+OpenGL
// Based on imgui example implementations
static GLFWwindow* g_Window = NULL;
static double g_Time = 0.0f;
static bool g_MousePressed[3] = {false, false, false};
static float g_MouseWheel = 0.0f;
static GLuint g_FontTexture = 0;

// Forward declarations
static void RenderDrawLists(ImDrawData* draw_data);

// This is the main rendering function
void RenderDrawLists(ImDrawData* draw_data) {
    // Rendering
    int fb_width = (int)(ImGui::GetIO().DisplaySize.x *
                         ImGui::GetIO().DisplayFramebufferScale.x);
    int fb_height = (int)(ImGui::GetIO().DisplaySize.y *
                          ImGui::GetIO().DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0) return;
    draw_data->ScaleClipRects(ImGui::GetIO().DisplayFramebufferScale);

    // Simple rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y,
            0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

// Render command lists
#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE*)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                        (const GLvoid*)((const char*)vtx_buffer +
                                        OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                          (const GLvoid*)((const char*)vtx_buffer +
                                          OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
                       (const GLvoid*)((const char*)vtx_buffer +
                                       OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x,
                          (int)(fb_height - pcmd->ClipRect.w),
                          (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                          (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                               sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                                      : GL_UNSIGNED_INT,
                               idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
#undef OFFSETOF

    // Restore modified state
    glDisable(GL_SCISSOR_TEST);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

static void error_callback(int error, const char* description) {
    cerr << "Error " << error << ": " << description << endl;
}

static const char* GetClipboardText(void* user_data) {
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void SetClipboardText(void* user_data, const char* text) {
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                  int mods) {
    if (action == GLFW_PRESS && button >= 0 && button < 3)
        g_MousePressed[button] = true;
}

static void scroll_callback(GLFWwindow* window, double xoffset,
                            double yoffset) {
    g_MouseWheel += (float)yoffset;  // Use fractional mouse wheel.
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS) io.KeysDown[key] = true;
    if (action == GLFW_RELEASE) io.KeysDown[key] = false;

    (void)mods;  // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] ||
                 io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift =
        io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt =
        io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper =
        io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void char_callback(GLFWwindow* window, unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000) io.AddInputCharacter((unsigned short)c);
}

void InitImGui(GLFWwindow* window) {
    g_Window = window;
    ImGuiIO& io = ImGui::GetIO();

    // Input mapping
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    // Callbacks
    io.RenderDrawListsFn = RenderDrawLists;
    io.SetClipboardTextFn = SetClipboardText;
    io.GetClipboardTextFn = GetClipboardText;
    io.ClipboardUserData = g_Window;

    // Install callbacks
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    // Create font texture
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void*)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
}

void UpdateImGui() {
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                        h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime =
        g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;

    // Setup inputs
    if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED)) {
        double mouse_x, mouse_y;
        glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
        io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    } else {
        io.MousePos = ImVec2(-1, -1);
    }

    for (int i = 0; i < 3; i++) {
        io.MouseDown[i] =
            g_MousePressed[i] || glfwGetMouseButton(g_Window, i) != 0;
        g_MousePressed[i] = false;
    }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode(
        g_Window, GLFW_CURSOR,
        io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}

void ShutdownImGui() {
    if (g_FontTexture) {
        glDeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }

    ImGui::DestroyContext();
}

GLFWwindow* SetupGui() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) return 0;

    // Create window with OpenGL 2.1 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow* window = glfwCreateWindow(800, 600, "KAI Window", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Setup ImGui binding
    ImGui::CreateContext();
    InitImGui(window);

    return window;
}

void LoadFont() {
    // Get IO but don't save reference to avoid unused variable warning
    ImGui::GetIO();

    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 1;
    config.GlyphExtraSpacing.x = 1.0f;
    // Use default font instead of loading from file
    // ImGui::GetIO().Fonts->AddFontFromFileTTF("CourierNew.ttf", 20, &config);
}

int main(int argc, char** argv) {
    GLFWwindow* window = SetupGui();
    if (!window) return -1;

    LoadFont();

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        UpdateImGui();

        // Show the KAI Languages Console as the main window
        ShowExecutorWindow(0);

        // Show settings window with ImGui demo toggle
        {
            ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("KAI Settings");
            ImGui::ColorEdit3("Background Color", (float*)&clear_color);
            if (ImGui::Button("ImGui Demo"))
                show_demo_window = !show_demo_window;
            ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Show demo window if enabled
        if (show_demo_window) {
            ImGui::SetNextWindowPos(ImVec2(650, 150), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Setup for ImGui rendering
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnable(GL_TEXTURE_2D);

        ImGui::Render();

        // Cleanup after ImGui rendering
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);
    }

    // Cleanup
    ShutdownImGui();
    glfwTerminate();

    return 0;
}
