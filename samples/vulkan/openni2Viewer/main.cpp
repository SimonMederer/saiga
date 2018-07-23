﻿#include "sample.h"
#include "saiga/framework.h"
#include "saiga/vulkan/window/SDLWindow.h"

#undef main

extern int maingsdgdfg();
int main(const int argc, const char *argv[])
{
    using namespace  Saiga;


    {
        WindowParameters windowParameters;
        windowParameters.fromConfigFile("config.ini");
        windowParameters.name = "Forward Rendering";

        windowParameters.width = 640 * 2;
        windowParameters.height = 480;

        Saiga::Vulkan::SDLWindow window(windowParameters);
        //        Saiga::Vulkan::GLFWWindow window(windowParameters);

        Saiga::Vulkan::VulkanParameters vulkanParams;
        vulkanParams.enableValidationLayer = true;
        Saiga::Vulkan::VulkanForwardRenderer renderer(window,vulkanParams);


        VulkanExample example(window,renderer);
        renderer.initChildren();
        //        example.init();

        MainLoopParameters params;
        params.mainLoopInfoTime = 1;
        params.framesPerSecond = 0;
        window.startMainLoop(params);

        renderer.waitIdle();
    }

    //    maingsdgdfg();
    return 0;
}
