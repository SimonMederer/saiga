﻿/**
 * Copyright (c) 2017 Darius Rückert
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "vulkanWindow.h"
#include "saiga/util/assert.h"
#include "SPIRV/GlslangToSpv.h"

namespace Saiga {

void init_resources(TBuiltInResource &Resources) {
    Resources.maxLights = 32;
    Resources.maxClipPlanes = 6;
    Resources.maxTextureUnits = 32;
    Resources.maxTextureCoords = 32;
    Resources.maxVertexAttribs = 64;
    Resources.maxVertexUniformComponents = 4096;
    Resources.maxVaryingFloats = 64;
    Resources.maxVertexTextureImageUnits = 32;
    Resources.maxCombinedTextureImageUnits = 80;
    Resources.maxTextureImageUnits = 32;
    Resources.maxFragmentUniformComponents = 4096;
    Resources.maxDrawBuffers = 32;
    Resources.maxVertexUniformVectors = 128;
    Resources.maxVaryingVectors = 8;
    Resources.maxFragmentUniformVectors = 16;
    Resources.maxVertexOutputVectors = 16;
    Resources.maxFragmentInputVectors = 15;
    Resources.minProgramTexelOffset = -8;
    Resources.maxProgramTexelOffset = 7;
    Resources.maxClipDistances = 8;
    Resources.maxComputeWorkGroupCountX = 65535;
    Resources.maxComputeWorkGroupCountY = 65535;
    Resources.maxComputeWorkGroupCountZ = 65535;
    Resources.maxComputeWorkGroupSizeX = 1024;
    Resources.maxComputeWorkGroupSizeY = 1024;
    Resources.maxComputeWorkGroupSizeZ = 64;
    Resources.maxComputeUniformComponents = 1024;
    Resources.maxComputeTextureImageUnits = 16;
    Resources.maxComputeImageUniforms = 8;
    Resources.maxComputeAtomicCounters = 8;
    Resources.maxComputeAtomicCounterBuffers = 1;
    Resources.maxVaryingComponents = 60;
    Resources.maxVertexOutputComponents = 64;
    Resources.maxGeometryInputComponents = 64;
    Resources.maxGeometryOutputComponents = 128;
    Resources.maxFragmentInputComponents = 128;
    Resources.maxImageUnits = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    Resources.maxCombinedShaderOutputResources = 8;
    Resources.maxImageSamples = 0;
    Resources.maxVertexImageUniforms = 0;
    Resources.maxTessControlImageUniforms = 0;
    Resources.maxTessEvaluationImageUniforms = 0;
    Resources.maxGeometryImageUniforms = 0;
    Resources.maxFragmentImageUniforms = 8;
    Resources.maxCombinedImageUniforms = 8;
    Resources.maxGeometryTextureImageUnits = 16;
    Resources.maxGeometryOutputVertices = 256;
    Resources.maxGeometryTotalOutputComponents = 1024;
    Resources.maxGeometryUniformComponents = 1024;
    Resources.maxGeometryVaryingComponents = 64;
    Resources.maxTessControlInputComponents = 128;
    Resources.maxTessControlOutputComponents = 128;
    Resources.maxTessControlTextureImageUnits = 16;
    Resources.maxTessControlUniformComponents = 1024;
    Resources.maxTessControlTotalOutputComponents = 4096;
    Resources.maxTessEvaluationInputComponents = 128;
    Resources.maxTessEvaluationOutputComponents = 128;
    Resources.maxTessEvaluationTextureImageUnits = 16;
    Resources.maxTessEvaluationUniformComponents = 1024;
    Resources.maxTessPatchComponents = 120;
    Resources.maxPatchVertices = 32;
    Resources.maxTessGenLevel = 64;
    Resources.maxViewports = 16;
    Resources.maxVertexAtomicCounters = 0;
    Resources.maxTessControlAtomicCounters = 0;
    Resources.maxTessEvaluationAtomicCounters = 0;
    Resources.maxGeometryAtomicCounters = 0;
    Resources.maxFragmentAtomicCounters = 8;
    Resources.maxCombinedAtomicCounters = 8;
    Resources.maxAtomicCounterBindings = 1;
    Resources.maxVertexAtomicCounterBuffers = 0;
    Resources.maxTessControlAtomicCounterBuffers = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers = 0;
    Resources.maxGeometryAtomicCounterBuffers = 0;
    Resources.maxFragmentAtomicCounterBuffers = 1;
    Resources.maxCombinedAtomicCounterBuffers = 1;
    Resources.maxAtomicCounterBufferSize = 16384;
    Resources.maxTransformFeedbackBuffers = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances = 8;
    Resources.maxCombinedClipAndCullDistances = 8;
    Resources.maxSamples = 4;
    Resources.limits.nonInductiveForLoops = 1;
    Resources.limits.whileLoops = 1;
    Resources.limits.doWhileLoops = 1;
    Resources.limits.generalUniformIndexing = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing = 1;
    Resources.limits.generalSamplerIndexing = 1;
    Resources.limits.generalVariableIndexing = 1;
    Resources.limits.generalConstantMatrixVectorIndexing = 1;
}
EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) {
    switch (shader_type) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;

        default:
            return EShLangVertex;
    }
}

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, std::vector<unsigned int> &spirv)
{

    EShLanguage stage = FindLanguage(shader_type);
    glslang::TShader shader(stage);
    glslang::TProgram program;
    const char *shaderStrings[1];
    TBuiltInResource Resources;
    init_resources(Resources);

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    shaderStrings[0] = pshader;
    shader.setStrings(shaderStrings, 1);

    if (!shader.parse(&Resources, 100, false, messages)) {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;  // something didn't work
    }

    program.addShader(&shader);

    //
    // Program-level processing...
    //

    if (!program.link(messages)) {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        fflush(stdout);
        return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

    return true;
}

VulkanWindow::VulkanWindow()
{


    // ======================= Layers =======================


    vk::Result res;


    init_global_layer_properties();
    init_instance();
    init_physical_device();

    createWindow();

    init_swapchain_extension();


    createDevice();

    // ======================= Command Buffer =======================

    {
        vk::CommandPoolCreateInfo cmd_pool_info = {};
        cmd_pool_info.queueFamilyIndex = graphics_queue_family_index;

        res = device.createCommandPool(&cmd_pool_info, nullptr, &cmd_pool);
        SAIGA_ASSERT(res == vk::Result::eSuccess);

        vk::CommandBufferAllocateInfo cmd_info = {};
        cmd_info.commandPool = cmd_pool;
        cmd_info.level = vk::CommandBufferLevel::ePrimary;
        cmd_info.commandBufferCount = 1;

        res = device.allocateCommandBuffers(&cmd_info,&cmd);
        SAIGA_ASSERT(res == vk::Result::eSuccess);

    }


    // begin command buffer
    {
        vk::CommandBufferBeginInfo cmd_buf_info = {};
//        cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//        cmd_buf_info.pNext = NULL;
//        cmd_buf_info.flags = 0;
        cmd_buf_info.pInheritanceInfo = NULL;

//        res = vkBeginCommandBuffer(info.cmd, &cmd_buf_info);
        cmd.begin( &cmd_buf_info);
//        assert(res == VK_SUCCESS);
    }



    // swap chain
    {

        vk::SurfaceCapabilitiesKHR surfCapabilities;

        res = physicalDevice.getSurfaceCapabilitiesKHR(surface,&surfCapabilities);
        SAIGA_ASSERT(res == vk::Result::eSuccess);

        std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

        vk::Extent2D swapchainExtent(width,height);


        // The FIFO present mode is guaranteed by the spec to be supported
        vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

        // Determine the number of VkImage's to use in the swap chain.
        // We need to acquire only 1 presentable image at at time.
        // Asking for minImageCount images ensures that we can acquire
        // 1 presentable image as long as we present it before attempting
        // to acquire another.
        uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

        vk::SurfaceTransformFlagBitsKHR preTransform;
        if (surfCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
            preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        } else {
            preTransform = surfCapabilities.currentTransform;
        }

        // Find a supported composite alpha mode - one of these is guaranteed to be set
        vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        //        vk::CompositeAlphaFlagBitsKHR compositeAlphaFlags[4] =
        //        {
        //            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        //            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        //            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        //            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        //        };
        //        for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) {
        //            if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
        //                compositeAlpha = compositeAlphaFlags[i];
        //                break;
        //            }
        //        }



        vk::SwapchainCreateInfoKHR swapchain_ci = {};
        swapchain_ci.surface = surface;
        swapchain_ci.minImageCount = desiredNumberOfSwapChainImages;
        swapchain_ci.imageFormat = format;
        swapchain_ci.imageExtent.width = swapchainExtent.width;
        swapchain_ci.imageExtent.height = swapchainExtent.height;
        swapchain_ci.preTransform = preTransform;
        swapchain_ci.compositeAlpha = compositeAlpha;
        swapchain_ci.imageArrayLayers = 1;
        swapchain_ci.presentMode = swapchainPresentMode;
        //        swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
        swapchain_ci.clipped = true;
        swapchain_ci.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        swapchain_ci.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        swapchain_ci.imageSharingMode = vk::SharingMode::eExclusive;
        swapchain_ci.queueFamilyIndexCount = 0;
        swapchain_ci.pQueueFamilyIndices = NULL;

        SAIGA_ASSERT(graphics_queue_family_index == present_queue_family_index);

        res = device.createSwapchainKHR(&swapchain_ci,nullptr,&swap_chain);
        SAIGA_ASSERT(res == vk::Result::eSuccess);


        swapChainImages = device.getSwapchainImagesKHR(swap_chain);
        swapChainImagesViews.resize(swapChainImages.size());


        for (uint32_t i = 0; i < swapChainImages.size(); i++)
        {
            vk::ImageViewCreateInfo color_image_view = {};
            //            color_image_view.flags = 0;
            color_image_view.image = swapChainImages[i];
            color_image_view.viewType = vk::ImageViewType::e2D;
            color_image_view.format = format;
            color_image_view.components.r = vk::ComponentSwizzle::eR;
            color_image_view.components.g = vk::ComponentSwizzle::eG;
            color_image_view.components.b = vk::ComponentSwizzle::eB;
            color_image_view.components.a = vk::ComponentSwizzle::eA;
            color_image_view.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            color_image_view.subresourceRange.baseMipLevel = 0;
            color_image_view.subresourceRange.levelCount = 1;
            color_image_view.subresourceRange.baseArrayLayer = 0;
            color_image_view.subresourceRange.layerCount = 1;

            //            res = vkCreateImageView(info.device, &color_image_view, NULL, &info.buffers[i].view);
            res = device.createImageView(&color_image_view,nullptr,&swapChainImagesViews[i]);
            SAIGA_ASSERT(res == vk::Result::eSuccess);
            //            assert(res == VK_SUCCESS);
        }
    }



    init_depth_buffer();


    init_uniform_buffer();

    // init render path

    {


        // The initial layout for the color and depth attachments will be
        // LAYOUT_UNDEFINED because at the start of the renderpass, we don't
        // care about their contents. At the start of the subpass, the color
        // attachment's layout will be transitioned to LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        // and the depth stencil attachment's layout will be transitioned to
        // LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL.  At the end of the renderpass,
        // the color attachment's layout will be transitioned to
        // LAYOUT_PRESENT_SRC_KHR to be ready to present.  This is all done as part
        // of the renderpass, no barriers are necessary.
        vk::AttachmentDescription attachments[2];
        attachments[0].format = format;
        attachments[0].samples = vk::SampleCountFlagBits::e1;
        attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
        attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
        attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eStore;
        attachments[0].initialLayout = vk::ImageLayout::eUndefined;
        attachments[0].finalLayout = vk::ImageLayout::ePresentSrcKHR;
//        attachments[0].flags = 0;

        attachments[1].format = depthFormat;
        attachments[1].samples = vk::SampleCountFlagBits::e1;
        attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
        attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
        attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        attachments[1].initialLayout = vk::ImageLayout::eUndefined;
        attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

//        attachments[1].samples = NUM_SAMPLES;
//        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//        attachments[1].flags = 0;


        vk::AttachmentReference color_reference = {};
        color_reference.attachment = 0;
        color_reference.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference depth_reference = {};
        depth_reference.attachment = 1;
        depth_reference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
//        subpass.flags = 0;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = NULL;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pResolveAttachments = NULL;
        subpass.pDepthStencilAttachment = &depth_reference;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = NULL;

        vk::RenderPassCreateInfo rp_info = {};
//        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO vk::st;
//        rp_info.pNext = NULL;
        rp_info.attachmentCount = 2;
        rp_info.pAttachments = attachments;
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpass;
        rp_info.dependencyCount = 0;
        rp_info.pDependencies = NULL;

//        res = vkCreateRenderPass(info.device, &rp_info, NULL, &info.render_pass);
        res = device.createRenderPass(&rp_info, NULL, &render_pass);
        SAIGA_ASSERT(res == vk::Result::eSuccess);



    }

    // init framebuffers

    {
        vk::ImageView attachments[2];
        attachments[1] = depthview;

        vk::FramebufferCreateInfo fb_info = {};
//        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        fb_info.pNext = NULL;
        fb_info.renderPass = render_pass;
        fb_info.attachmentCount = 2;
        fb_info.pAttachments = attachments;
        fb_info.width = width;
        fb_info.height = height;
        fb_info.layers = 1;

        uint32_t i;
//        info.framebuffers = (VkFramebuffer *)malloc(info.swapchainImageCount * sizeof(VkFramebuffer));
        framebuffers.resize(swapChainImages.size());
        SAIGA_ASSERT(swapChainImages.size() == 2);
//        assert(info.framebuffers);

        for (i = 0; i < framebuffers.size(); i++)
        {
            attachments[0] = swapChainImagesViews[i];
            res = device.createFramebuffer(&fb_info, NULL, &framebuffers[i]);
SAIGA_ASSERT(res == vk::Result::eSuccess);
//            assert(res == VK_SUCCESS);
        }


    }


    // vertex buffer

    init_vertex_buffer();

    // pipeline layout

    {
        vk::DescriptorSetLayoutBinding layout_binding = {};
        layout_binding.binding = 0;
        layout_binding.descriptorType = vk::DescriptorType::eUniformBuffer;
        layout_binding.descriptorCount = 1;
        layout_binding.stageFlags = vk::ShaderStageFlagBits::eVertex;
        layout_binding.pImmutableSamplers = NULL;

        /* Next take layout bindings and use them to create a descriptor set layout
         */
        vk::DescriptorSetLayoutCreateInfo descriptor_layout = {};
//        descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//        descriptor_layout.pNext = NULL;
        descriptor_layout.bindingCount = 1;
        descriptor_layout.pBindings = &layout_binding;

        /* Number of descriptor sets needs to be the same at alloc,       */
        /* pipeline layout creation, and descriptor set layout creation   */
        #define NUM_DESCRIPTOR_SETS 1


        desc_layout.resize(NUM_DESCRIPTOR_SETS);
//        res = vkCreateDescriptorSetLayout(info.device, &descriptor_layout, NULL, info.desc_layout.data());
        res = device.createDescriptorSetLayout(&descriptor_layout,nullptr,desc_layout.data());
        SAIGA_ASSERT(res == vk::Result::eSuccess);


        /* Now use the descriptor layout to create a pipeline layout */
        vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
//        pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//        pPipelineLayoutCreateInfo.pNext = NULL;
        pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
        pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
        pPipelineLayoutCreateInfo.pSetLayouts = desc_layout.data();

        res = device.createPipelineLayout(&pPipelineLayoutCreateInfo, NULL, &pipeline_layout);
        SAIGA_ASSERT(res == vk::Result::eSuccess);
//        assert(res == VK_SUCCESS);
    }



    // descriptor set

    {
        vk::DescriptorPoolSize type_count[1];
        type_count[0].type = vk::DescriptorType::eUniformBuffer;
        type_count[0].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo descriptor_pool = {};
//        descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO vk::;
//        descriptor_pool.pNext = NULL;
        descriptor_pool.maxSets = 1;
        descriptor_pool.poolSizeCount = 1;
        descriptor_pool.pPoolSizes = type_count;

//        res = vkCreateDescriptorPool(info.device, &descriptor_pool, NULL, &info.desc_pool);
        res = device.createDescriptorPool(&descriptor_pool, NULL, &desc_pool);
           SAIGA_ASSERT(res == vk::Result::eSuccess);


           vk::DescriptorSetAllocateInfo alloc_info[1];
//           alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//           alloc_info[0].pNext = NULL;
           alloc_info[0].descriptorPool = desc_pool;
           alloc_info[0].descriptorSetCount = NUM_DESCRIPTOR_SETS;
           alloc_info[0].pSetLayouts = desc_layout.data();

           desc_set.resize(NUM_DESCRIPTOR_SETS);
//           res = vkAllocateDescriptorSets(info.device, alloc_info, info.desc_set.data());
           res = device.allocateDescriptorSets(alloc_info, desc_set.data());
           SAIGA_ASSERT(res == vk::Result::eSuccess);


           vk::WriteDescriptorSet writes[1];

//           writes[0] = {};
//           writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//           writes[0].pNext = NULL;
           writes[0].dstSet = desc_set[0];
           writes[0].descriptorCount = 1;
           writes[0].descriptorType = vk::DescriptorType::eUniformBuffer;
           writes[0].pBufferInfo = &uniformbuffer_info;
           writes[0].dstArrayElement = 0;
           writes[0].dstBinding = 0;

//           vkUpdateDescriptorSets(info.device, 1, writes, 0, NULL);
           device.updateDescriptorSets(1, writes, 0, NULL);

    }




    // shader

    {

        static const char *vertShaderText =
            "#version 400\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding = 0) uniform bufferVals {\n"
            "    mat4 mvp;\n"
            "} myBufferVals;\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 1) in vec4 inColor;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = myBufferVals.mvp * pos;\n"
            "}\n";

        static const char *fragShaderText =
            "#version 400\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = color;\n"
            "}\n";

        std::vector<unsigned int> vtx_spv;
//        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//        shaderStages[0].pNext = NULL;
        shaderStages[0].pSpecializationInfo = NULL;
//        shaderStages[0].flags = 0;
        shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
        shaderStages[0].pName = "main";

            glslang::InitializeProcess();

            bool retVal = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertShaderText, vtx_spv);
            SAIGA_ASSERT(retVal);

            cout << "shader loaded." << endl;

            vk::ShaderModuleCreateInfo moduleCreateInfo;
//            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//            moduleCreateInfo.pNext = NULL;
//            moduleCreateInfo.flags = 0;
            moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
            moduleCreateInfo.pCode = vtx_spv.data();
            res = device.createShaderModule(&moduleCreateInfo, NULL, &shaderStages[0].module);
//            assert(res == VK_SUCCESS);
            SAIGA_ASSERT(res == vk::Result::eSuccess);


            std::vector<unsigned int> frag_spv;
//            info.shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//            info.shaderStages[1].pNext = NULL;
//            info.shaderStages[1].flags = 0;
            shaderStages[1].pSpecializationInfo = NULL;
            shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
            shaderStages[1].pName = "main";

            retVal = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragShaderText, frag_spv);
//            assert(retVal);
            SAIGA_ASSERT(retVal);

//            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//            moduleCreateInfo.pNext = NULL;
//            moduleCreateInfo.flags = 0;
            moduleCreateInfo.codeSize = frag_spv.size() * sizeof(unsigned int);
            moduleCreateInfo.pCode = frag_spv.data();
            res = device.createShaderModule(&moduleCreateInfo, NULL, &shaderStages[1].module);
            SAIGA_ASSERT(res == vk::Result::eSuccess);

            glslang::FinalizeProcess();
    }


    vk::DynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE ];
    vk::PipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
//    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    vk::PipelineVertexInputStateCreateInfo vi;
//    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    vi.pNext = NULL;
//    vi.flags = 0;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &vi_binding;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = vi_attribs;

    vk::PipelineInputAssemblyStateCreateInfo ia;
//    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    ia.pNext = NULL;
//    ia.flags = 0;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology = vk::PrimitiveTopology::eTriangleList;

    vk::PipelineRasterizationStateCreateInfo rs;
//    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rs.pNext = NULL;
//    rs.flags = 0;
    rs.polygonMode = vk::PolygonMode::eFill;
    rs.cullMode = vk::CullModeFlagBits::eBack;
    rs.frontFace = vk::FrontFace::eClockwise;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0;
    rs.depthBiasClamp = 0;
    rs.depthBiasSlopeFactor = 0;
    rs.lineWidth = 1.0f;


    vk::PipelineColorBlendStateCreateInfo cb;
//    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    cb.pNext = NULL;
//    cb.flags = 0;
    vk::PipelineColorBlendAttachmentState att_state[1];
    att_state[0].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB| vk::ColorComponentFlagBits::eA;
    att_state[0].blendEnable = VK_FALSE;
    att_state[0].alphaBlendOp = vk::BlendOp::eAdd;
    att_state[0].colorBlendOp = vk::BlendOp::eAdd;
    att_state[0].srcColorBlendFactor = vk::BlendFactor::eZero;
    att_state[0].dstColorBlendFactor = vk::BlendFactor::eZero;
    att_state[0].srcAlphaBlendFactor = vk::BlendFactor::eZero;
    att_state[0].dstAlphaBlendFactor = vk::BlendFactor::eZero;
    cb.attachmentCount = 1;
    cb.pAttachments = att_state;
    cb.logicOpEnable = VK_FALSE;
    cb.logicOp = vk::LogicOp::eNoOp;
    cb.blendConstants[0] = 1.0f;
    cb.blendConstants[1] = 1.0f;
    cb.blendConstants[2] = 1.0f;
    cb.blendConstants[3] = 1.0f;

    vk::PipelineViewportStateCreateInfo vp = {};
//    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    vp.pNext = NULL;
//    vp.flags = 0;
    vp.viewportCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eViewport;
    vp.scissorCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = vk::DynamicState::eScissor;
    vp.pScissors = NULL;
    vp.pViewports = NULL;

    vk::PipelineDepthStencilStateCreateInfo ds;
//    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    ds.pNext = NULL;
//    ds.flags = 0;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = vk::CompareOp::eLessOrEqual;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.minDepthBounds = 0;
    ds.maxDepthBounds = 0;
    ds.stencilTestEnable = VK_FALSE;
    ds.back.failOp = vk::StencilOp::eKeep;
    ds.back.passOp = vk::StencilOp::eKeep;
    ds.back.compareOp = vk::CompareOp::eAlways;
    ds.back.compareMask = 0;
    ds.back.reference = 0;
    ds.back.depthFailOp = vk::StencilOp::eKeep;
    ds.back.writeMask = 0;
    ds.front = ds.back;


    vk::PipelineMultisampleStateCreateInfo ms;
//    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    ms.pNext = NULL;
//    ms.flags = 0;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = vk::SampleCountFlagBits::e1;
    ms.sampleShadingEnable = VK_FALSE;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable = VK_FALSE;
    ms.minSampleShading = 0.0;

    vk::GraphicsPipelineCreateInfo pipeline_info;
//    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipeline.pNext = NULL;
    pipeline_info.layout = pipeline_layout;
//    pipeli_infone.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = 0;
//    pipeli_infone.flags = 0;
    pipeline_info.pVertexInputState = &vi;
    pipeline_info.pInputAssemblyState = &ia;
    pipeline_info.pRasterizationState = &rs;
    pipeline_info.pColorBlendState = &cb;
    pipeline_info.pTessellationState = NULL;
    pipeline_info.pMultisampleState = &ms;
    pipeline_info.pDynamicState = &dynamicState;
    pipeline_info.pViewportState = &vp;
    pipeline_info.pDepthStencilState = &ds;
    pipeline_info.pStages = shaderStages;
    pipeline_info.stageCount = 2;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    res = device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline);
}


VulkanWindow::~VulkanWindow()
{
//    for (i = 0; i < info.swapchainImageCount; i++) {
//        vkDestroyFramebuffer(info.device, info.framebuffers[i], NULL);
//    }

//    vkDestroyShaderModule(info.device, info.shaderStages[0].module, NULL);
//    vkDestroyShaderModule(info.device, info.shaderStages[1].module, NULL);
//    vkDestroyRenderPass(info.device, info.render_pass, NULL);
//    for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++) vkDestroyDescriptorSetLayout(info.device, info.desc_layout[i], NULL);
//    vkDestroyPipelineLayout(info.device, info.pipeline_layout, NULL);

    for(vk::ImageView& iv : swapChainImagesViews)
    {
        device.destroyImageView(iv);
    }
    device.destroySwapchainKHR(swap_chain);

    device.freeCommandBuffers(cmd_pool,cmd);
    device.destroyCommandPool(cmd_pool);
    device.destroy();
    inst.destroy();
}



}