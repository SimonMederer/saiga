﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/config.h"
#ifdef SAIGA_USE_GLFW

#    include "saiga/opengl/imgui/imgui_impl_glfw_gl3.h"

#    include <GLFW/glfw3.h>

#    include "glfw_window.h"

namespace Saiga
{
glfw_Window::glfw_Window(WindowParameters windowParameters, OpenGLParameters openglParameters)
    : OpenGLWindow(windowParameters, openglParameters)
{
    create();
}

glfw_Window::~glfw_Window()
{
    destroy();
}

void glfw_Window::destroy()
{
    cleanupSaiga();
    freeContext();
}



void glfw_Window::loadGLFunctions()
{
    initOpenGL(glfwGetProcAddress);
}


void glfw_Window::getCurrentPrimaryMonitorResolution(int* width, int* height)
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    cout << "Video Mode: " << mode->width << " x " << mode->height << " @" << mode->refreshRate << "Hz" << endl;

    *width  = mode->width;
    *height = mode->height;
}

void glfw_Window::getMaxResolution(int* width, int* height)
{
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    // get max video mode resolution
    int count;
    const GLFWvidmode* mode = glfwGetVideoModes(primary, &count);
    //    cout << "Video modes:" << endl;
    //    for (int i = 0; i < count; i++){
    //        cout << "Mode "<< i << ": " << mode[i].width << " x "<< mode[i].height << " @" << mode[i].refreshRate <<
    //        "Hz" << endl;
    //    }

    cout << "Native Video Mode: " << mode[count - 1].width << " x " << mode[count - 1].height << " @"
         << mode[count - 1].refreshRate << "Hz" << endl;
    *width  = mode[count - 1].width;
    *height = mode[count - 1].height;
}

std::shared_ptr<ImGui_GL_Renderer> glfw_Window::createImGui()
{
    if (windowParameters.imguiParameters.enable)
    {
        return std::make_shared<ImGui_GLFW_Renderer>(window, windowParameters.imguiParameters);
    }
    return {};
}

void glfw_Window::setCursorPosition(int x, int y)
{
    glfwSetCursorPos(window, x, y);
}


void glfw_Window::hideMouseCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void glfw_Window::showMouseCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void glfw_Window::disableMouseCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool glfw_Window::initGlfw()
{
    glfwSetErrorCallback(glfw_Window::error_callback);
    cout << "Initializing GLFW." << endl;
    /* Initialize the library */
    if (!glfwInit()) return false;
    cout << "Initializing GLFW sucessfull!" << endl;
    return true;
}



bool glfw_Window::initWindow()
{
    if (!glfw_Window::initGlfw())
    {
        cout << "Could not initialize GLFW" << endl;
        return false;
    }

    int monitorCount;
    GLFWmonitor** monitors     = glfwGetMonitors(&monitorCount);
    windowParameters.monitorId = clamp(windowParameters.monitorId, 0, monitorCount - 1);

    GLFWmonitor* monitor    = monitors[windowParameters.monitorId];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);


    //    //don't allow other resolutions than the native monitor ones in fullscreen mode
    if (windowParameters.fullscreen())
    {
        windowParameters.width  = mode->width;
        windowParameters.height = mode->height;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openglParameters.versionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openglParameters.versionMinor);

    switch (openglParameters.profile)
    {
        case OpenGLParameters::Profile::ANY:
            // that is the default value
            break;
        case OpenGLParameters::Profile::CORE:
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            break;
        case OpenGLParameters::Profile::COMPATIBILITY:
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
            break;
    }

    if (windowParameters.hidden) glfwWindowHint(GLFW_VISIBLE, 0);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, openglParameters.debug);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, openglParameters.forwardCompatible);



    glfwWindowHint(GLFW_DECORATED, !windowParameters.borderLess());
    glfwWindowHint(GLFW_FLOATING, windowParameters.alwaysOnTop);
    glfwWindowHint(GLFW_RESIZABLE, windowParameters.resizeAble);

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    // GLFW_REFRESH_RATE, GLFW_DONT_CARE = highest
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);

    std::cout << "Creating GLFW Window. " << getWidth() << "x" << getHeight()
              << " Fullscreen=" << windowParameters.fullscreen() << " Borderless=" << windowParameters.borderLess()
              << std::endl;


    switch (windowParameters.mode)
    {
        case WindowParameters::Mode::windowed:
            window = glfwCreateWindow(getWidth(), getHeight(), getName().c_str(), NULL, NULL);
            break;
        case WindowParameters::Mode::fullscreen:
            window = glfwCreateWindow(getWidth(), getHeight(), getName().c_str(), monitor, NULL);
            break;
        case WindowParameters::Mode::borderLessWindowed:
            window = glfwCreateWindow(getWidth(), getHeight(), getName().c_str(), NULL, NULL);
            break;
        case WindowParameters::Mode::borderLessFullscreen:
#    ifndef WIN32
            std::cerr << "Windowed Fullscreen may not work on your system." << std::endl;
#    endif

            // works in windows 7. Does not work in ubuntu with gnome
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

            window = glfwCreateWindow(getWidth(), getHeight(), getName().c_str(), NULL, NULL);

            // move to correct monitor
            int xpos, ypos;
            glfwGetMonitorPos(monitor, &xpos, &ypos);
            glfwSetWindowPos(window, xpos, ypos);
            break;
    }

    if (!window)
    {
        glfwTerminate();
        cerr << "glfwCreateWindow returned false!" << endl;
        return false;
    }

    glfwMakeContextCurrent(window);

    // vsync
    glfwSwapInterval(windowParameters.vsync ? 1 : 0);


    // framebuffer size != window size
    glfwGetFramebufferSize(window, &windowParameters.width, &windowParameters.height);


    // not needed but makes start cleaner
    glfwPollEvents();
    glfwSwapBuffers(window);


    if (windowParameters.updateJoystick)
    {
        glfwSetJoystickCallback(glfw_Joystick::joystick_callback);
        glfw_Joystick::enableFirstJoystick();
    }


    return true;
}

bool glfw_Window::initInput()
{
    glfwSetFramebufferSizeCallback(window, glfw_EventHandler::window_size_callback);

    // mouse
    glfwSetCursorPosCallback(window, glfw_EventHandler::cursor_position_callback);
    glfwSetMouseButtonCallback(window, glfw_EventHandler::mouse_button_callback);
    glfwSetScrollCallback(window, glfw_EventHandler::scroll_callback);
    // keyboard
    glfwSetCharCallback(window, glfw_EventHandler::character_callback);
    glfwSetKeyCallback(window, glfw_EventHandler::key_callback);

    glfw_EventHandler::addResizeListener(this, 0);
    return true;
}



void glfw_Window::freeContext()
{
    glfwDestroyWindow(window);
    glfwTerminate();
    terminateOpenGL();
    cout << "GLFW: Terminated." << endl;
}



bool glfw_Window::window_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    this->resize(width, height);
    return false;
}

void glfw_Window::error_callback(int error, const char* description)
{
    cout << "glfw error: " << error << " " << description << endl;
}

void glfw_Window::setGLFWcursor(GLFWcursor* cursor)
{
    glfwSetCursor(window, cursor);
}


GLFWcursor* glfw_Window::createGLFWcursor(Image* image, int midX, int midY)
{
    if (image->type != UC4)
    {
        cout << "glfw_Window::createGLFWcursor(Image *image): image has the wrong format." << endl;
        cout << "Required format: RGBA8" << endl;
        SAIGA_ASSERT(0);
    }


    GLFWimage glfwimage;
    glfwimage.width  = image->width;
    glfwimage.height = image->height;
    glfwimage.pixels = image->data8();

    return glfwCreateCursor(&glfwimage, midX, midY);
}

bool glfw_Window::shouldClose()
{
    return glfwWindowShouldClose(window) || !running;
}

void glfw_Window::swapBuffers()
{
    glfwSwapBuffers(window);
}

void glfw_Window::checkEvents()
{
    glfwPollEvents();
    if (windowParameters.updateJoystick) glfw_Joystick::update();
}


void glfw_Window::setWindowIcon(Image* image)
{
    SAIGA_ASSERT(window);
    if (image->type != UC4)
    {
        cout << "glfw_Window::setWindowIcon(Image *image): image has the wrong format." << endl;
        cout << "Required format: RGBA8" << endl;
        SAIGA_ASSERT(0);
    }


    GLFWimage glfwimage;
    glfwimage.width  = image->width;
    glfwimage.height = image->height;
    glfwimage.pixels = image->data8();

    // only works with glfw version 3.2 and up
    glfwSetWindowIcon(window, 1, &glfwimage);
}



}  // namespace Saiga
#endif
