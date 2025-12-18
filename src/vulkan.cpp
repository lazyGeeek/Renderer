#include "renderer/vulkan.hpp"
#include "renderer/command_buffers.hpp"
#include "renderer/command_pool.hpp"
#include "renderer/device.hpp"
#include "renderer/shader.hpp"
#include "renderer/sync_object.hpp"
#include "renderer/swap_chain.hpp"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_set>

namespace Renderer
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                        void* pUserData)
    {
        std::cerr << "Validation layer: " << callbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
                                          const VkAllocationCallbacks* allocator,
                                          VkDebugUtilsMessengerEXT* debugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, createInfo, allocator, debugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* allocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, allocator);
    }

    Vulkan::Vulkan(GLFWwindow* window) : m_window { window } { }

    void Vulkan::Init(const std::filesystem::path& shaderPath)
    {
        throwIfFailed(createInstance(), "Failed to create instance");
        throwIfFailed(setupDebugMessenger(), "Failed to setup debug messanger");
        throwIfFailed(createSurface(), "Failed to create surface");

        m_device = std::make_unique<Device>(*this);
        m_device->Create();

        m_swapChain =std::make_unique<SwapChain>(m_window, *m_device.get(), m_surface);
        m_swapChain->Create();

        throwIfFailed(createRenderPass(), "Failed to create render pass");
        throwIfFailed(createGraphicsPipeline(shaderPath), "Failed to create graphics pipeline");
        m_swapChain->CreateFramebuffers(m_renderPass);

        m_commandPool = std::make_unique<CommandPool>(*m_device.get());
        m_commandPool->Create();

        m_commandBuffers = std::make_unique<CommandBuffers>(*m_device.get());
        m_commandBuffers->Create(m_commandPool->Get(), MAX_FRAMES_IN_FLIGHT);

        m_syncObjects.clear();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            std::unique_ptr<SyncObject> syncObject = std::make_unique<SyncObject>(*m_device.get());
            syncObject->Create();
            m_syncObjects.emplace_back(std::move(syncObject));
        }
    }

    Vulkan::~Vulkan()
    {
        m_device->WaitIdle();
        const VkDevice& device = m_device->GetLogicalDevice(); 

        m_swapChain->Clear();
        m_swapChain = nullptr;

        if (m_graphicsPipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(device, m_graphicsPipeline, nullptr);

        if (m_pipelineLayout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
        
        if (m_renderPass != VK_NULL_HANDLE)
            vkDestroyRenderPass(device, m_renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_syncObjects[i]->Destroy();
            m_syncObjects[i] = nullptr;
        }

        m_commandPool->Destroy();
        m_commandPool = nullptr;
            
        m_device->Destroy();
        m_device = nullptr;

        if (m_instance != VK_NULL_HANDLE)
        {
            if (ENABLE_VALIDATION_LAYERS)
                destroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

            if (m_surface != VK_NULL_HANDLE)
                vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

            vkDestroyInstance(m_instance, nullptr);
        }
    }

    void Vulkan::DrawFrame()
    {
        m_syncObjects[m_currentFrame]->WaitForFence();
        
        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(m_device->GetLogicalDevice(),
                                                m_swapChain->GetSwapChainKHR(),
                                                UINT64_MAX, m_syncObjects[m_currentFrame]->GetImageAvailableSemaphore(),
                                                VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_swapChain->Recreate(m_renderPass);
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image");
        }
        
        m_syncObjects[m_currentFrame]->ResetFence();
        
        m_commandBuffers->Reset(m_currentFrame);

        CommandBufferRecordInfo recordInfo
        {
            .FrameBuffer = m_swapChain->GetFramebuffer(imageIndex),
            .GraphicsPipeline = m_graphicsPipeline,
            .RenderPass = m_renderPass,
            .SwapChainExtent = m_swapChain->GetExtent()
        };

        m_commandBuffers->Record(m_currentFrame, recordInfo);

        VkSubmitInfo submitInfo { };
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_syncObjects[m_currentFrame]->GetImageAvailableSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers->Get(m_currentFrame);

        VkSemaphore signalSemaphores[] = { m_syncObjects[m_currentFrame]->GetRenderFinishedSemaphore() };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        throwIfFailed(vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, m_syncObjects[m_currentFrame]->GetInFlightFence()), "Failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo { };
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_swapChain->GetSwapChainKHR() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_device->GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            m_swapChain->Recreate(m_renderPass);
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("[Vulkan] Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Vulkan::SetFramebufferResized()
    {
        m_framebufferResized = true;
    }

    const VkInstance& Vulkan::GetInstance() const
    {
        return m_instance;
    }

    const VkSurfaceKHR& Vulkan::GetSurface() const
    {
        return m_surface;
    }

    bool Vulkan::IsValidationLayerEnabled() const
    {
        return ENABLE_VALIDATION_LAYERS;
    }

    VkResult Vulkan::checkValidationLayerSupport()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : m_validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
                return VK_ERROR_UNKNOWN;
        }

        return VK_SUCCESS;
    }

    VkResult Vulkan::checkGflwRequiredInstanceExtensions(const std::vector<const char*>& requiredExtensions)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::unordered_set<std::string> available;
        for (const auto& extension : extensions)
        {
            available.insert(extension.extensionName);
        }

        for (const auto& required : requiredExtensions)
        {
            if (available.find(required) == available.end())
                return VK_ERROR_UNKNOWN;
        }

        return VK_SUCCESS;
    }

    VkResult Vulkan::createInstance()
    {
        if (ENABLE_VALIDATION_LAYERS)
            throwIfFailed(checkValidationLayerSupport(), "Validation layers requested, but not available!");

        VkApplicationInfo appInfo { };
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instanceInfo { };
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;

        uint32_t extensionCount = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<const char*> requiredExtensions;

        for (uint32_t i = 0; i < extensionCount; ++i)
        {
            requiredExtensions.emplace_back(extensions[i]);
        }

        if (ENABLE_VALIDATION_LAYERS)
            requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        throwIfFailed(checkGflwRequiredInstanceExtensions(requiredExtensions), "Missing required GLFW extension");

#ifdef __APPLE__
            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo { };
        if (ENABLE_VALIDATION_LAYERS)
        {
            instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
            instanceInfo.ppEnabledLayerNames = m_validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            instanceInfo.enabledLayerCount = 0;
            instanceInfo.pNext = nullptr;
        }

        return vkCreateInstance(&instanceInfo, nullptr, &m_instance);
    }

    void Vulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = { };
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        
        createInfo.pfnUserCallback = debugCallback;
    }

    VkResult Vulkan::setupDebugMessenger()
    {
        if (!ENABLE_VALIDATION_LAYERS)
            return VK_SUCCESS;

        VkDebugUtilsMessengerCreateInfoEXT createInfo { };
        populateDebugMessengerCreateInfo(createInfo);
        return createDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger);
    }

    VkResult Vulkan::createSurface()
    {
        return glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);
    }

    VkResult Vulkan::createRenderPass()
    {
        VkAttachmentDescription colorAttachment { };
        colorAttachment.format = m_swapChain->GetImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef { };
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass { };
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency { };
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo { };
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        return vkCreateRenderPass(m_device->GetLogicalDevice(), &renderPassInfo, nullptr, &m_renderPass);
    }

    VkResult Vulkan::createGraphicsPipeline(const std::filesystem::path& shaderPath)
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo { };
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly { };
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState { };
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer { };
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling { };
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment { };
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending { };
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState { };
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo { };
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        const VkDevice& device = m_device->GetLogicalDevice(); 

        VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);

        if (result != VK_SUCCESS)
            return result;

        ShaderInfo shaderInfo
        {
            .FilePath = shaderPath / "triangle_vert.spv",
            .Type = EShaderType::Vertex
        };

        Shader vertexShader(device, shaderInfo);

        shaderInfo.FilePath = shaderPath / "triangle_frag.spv";
        shaderInfo.Type = EShaderType::Fragment;

        Shader fragmentShader(device, shaderInfo);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = std::move(vertexShader.GenerateStageInfo());
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = std::move(fragmentShader.GenerateStageInfo());

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkGraphicsPipelineCreateInfo pipelineInfo { };
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline);
        if (result != VK_SUCCESS)
            return result;

        return result;
    }
}
