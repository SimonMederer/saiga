// ImGui License:
// The MIT License (MIT)

// Copyright (c) 2014-2015 Omar Cornut and ImGui contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// ImGui GLFW binding with OpenGL3 + shaders
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture identifier. Read the FAQ about ImTextureID
// in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see examples/README.txt and documentation at
// the top of imgui.cpp. https://github.com/ocornut/imgui

#include "imgui_impl_glfw_gl3.h"
#ifdef SAIGA_USE_GLFW
#    include "saiga/opengl/opengl.h"

#    include <GLFW/glfw3.h>
#    include <saiga/core/imgui/imgui.h>


#    ifdef _WIN32
#        undef APIENTRY
#        define GLFW_EXPOSE_NATIVE_WIN32
#        define GLFW_EXPOSE_NATIVE_WGL
#        include <GLFW/glfw3native.h>
#    endif

namespace Saiga
{
GLFWwindow* ImGui_GLFW_Renderer::g_Window = NULL;


const char* ImGui_GLFW_Renderer::ImGui_ImplGlfwGL3_GetClipboardText(void* user_data)
{
    return glfwGetClipboardString(g_Window);
}

void ImGui_GLFW_Renderer::ImGui_ImplGlfwGL3_SetClipboardText(void* user_data, const char* text)
{
    glfwSetClipboardString(g_Window, text);
}


ImGui_GLFW_Renderer::ImGui_GLFW_Renderer(GLFWwindow* window, const ImGuiParameters& params) : ImGui_GL_Renderer(params)
{
    g_Window    = window;
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] =
        GLFW_KEY_TAB;  // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow]  = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]   = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]       = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End]        = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete]     = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]      = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape]     = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C]          = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V]          = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X]          = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y]          = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z]          = GLFW_KEY_Z;

    io.RenderDrawListsFn = 0;  // Alternatively you can set this to NULL and call ImGui::GetDrawData() after
                               // ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = ImGui_ImplGlfwGL3_SetClipboardText;
    //    io.SetClipboardTextFn = [this](const char*text){this->ImGui_ImplGlfwGL3_SetClipboardText(text);};
    io.GetClipboardTextFn = ImGui_ImplGlfwGL3_GetClipboardText;
#    ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(g_Window);
#    endif

    glfw_EventHandler::addKeyListener(this, 15);
    glfw_EventHandler::addMouseListener(this, 15);



    std::cout << "Imgui Initialized!" << std::endl;
}

ImGui_GLFW_Renderer::~ImGui_GLFW_Renderer() {}

void ImGui_GLFW_Renderer::beginFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize             = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime        = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time              = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
    {
        double mouse_x, mouse_y;
        glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
        io.MousePos = ImVec2(
            (float)mouse_x,
            (float)
                mouse_y);  // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
        io.MousePos = ImVec2(-1, -1);
    }

    for (int i = 0; i < 3; i++)
    {
        io.MouseDown[i] =
            g_MousePressed[i] || glfwGetMouseButton(g_Window, i) !=
                                     0;  // If a mouse press event came, always pass it as "mouse held this frame", so
                                         // we don't miss click-release events that are shorter than 1 frame.
        g_MousePressed[i] = false;
    }

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel  = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    //    glfwSetInputMode(g_Window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}



bool ImGui_GLFW_Renderer::key_event(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS) io.KeysDown[key] = true;
    if (action == GLFW_RELEASE) io.KeysDown[key] = false;

    (void)mods;  // Modifiers are not reliable across systems
    io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
    //    return wantsCaptureMouse;
    return false;
}

bool ImGui_GLFW_Renderer::character_event(GLFWwindow* window, unsigned int codepoint)
{
    ImGuiIO& io = ImGui::GetIO();
    if (codepoint > 0 && codepoint < 0x10000) io.AddInputCharacter((unsigned short)codepoint);
    return false;
    //    return wantsCaptureMouse;
}

bool ImGui_GLFW_Renderer::cursor_position_event(GLFWwindow* window, double xpos, double ypos)
{
    return false;
}

bool ImGui_GLFW_Renderer::mouse_button_event(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS && button >= 0 && button < 3) g_MousePressed[button] = true;
    return false;
    //    return wantsCaptureMouse;
}

bool ImGui_GLFW_Renderer::scroll_event(GLFWwindow* window, double xoffset, double yoffset)
{
    g_MouseWheel += (float)yoffset;  // Use fractional mouse wheel, 1.0 unit 5 lines.
    return false;
    //    return wantsCaptureMouse;
}

}  // namespace Saiga
#endif
