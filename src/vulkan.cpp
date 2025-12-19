#include "renderer/vulkan.hpp"
#include "renderer/command_buffers.hpp"
#include "renderer/command_pool.hpp"
#include "renderer/debug_messanger.hpp"
#include "renderer/device.hpp"
#include "renderer/frame_buffers.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/render_pass.hpp"
#include "renderer/shader.hpp"
#include "renderer/sync_object.hpp"
#include "renderer/swap_chain.hpp"

#include <unordered_set>

namespace Renderer
{
    Vulkan::Vulkan(GLFWwindow* window) : m_window { window } { }

    void Vulkan::Init(const std::filesystem::path& shaderPath)
    {
        createInstance();

        if (ENABLE_VALIDATION_LAYERS)
        {
            m_debugMessanger = std::make_unique<DebugMessanger>(m_instance);
            m_debugMessanger->Create();
        }

        createSurface();            

        m_device = std::make_unique<Device>(*this);
        m_device->Create();

        m_swapChain =std::make_unique<SwapChain>(m_window, *m_device.get(), m_surface);
        m_swapChain->Create();

        m_renderPass = std::make_unique<RenderPass>(*m_device.get(), *m_swapChain.get());
        m_renderPass->Create();

        FrameBuffersInit init
        {
            .Device = *m_device.get(),
            .RenderPass = *m_renderPass.get(),
            .SwapChain = *m_swapChain.get()
        };

        m_frameBuffers = std::make_unique<FrameBuffers>(init);
        m_frameBuffers->Create();

        ShaderInfo shaderInfo
        {
            .FilePath = shaderPath / "triangle_vert.spv",
            .Type = EShaderType::Vertex
        };

        Shader vertexShader(*m_device.get(), shaderInfo);

        shaderInfo.FilePath = shaderPath / "triangle_frag.spv";
        shaderInfo.Type = EShaderType::Fragment;

        Shader fragmentShader(*m_device.get(), shaderInfo);

        m_pipeline = std::make_unique<Pipeline>(*m_device.get(), *m_renderPass.get());
        m_pipeline->Create(vertexShader, fragmentShader);

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

        m_frameBuffers->Destroy();
        m_frameBuffers = nullptr;

        m_swapChain->Clear();
        m_swapChain = nullptr;

        m_pipeline->Destroy();
        m_pipeline = nullptr;
        
        m_renderPass->Destroy();
        m_renderPass = nullptr;

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
            if (ENABLE_VALIDATION_LAYERS && m_debugMessanger)
            {
                m_debugMessanger->Destroy();
                m_debugMessanger = nullptr;
            }

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
            m_swapChain->Recreate();
            m_frameBuffers->Recreate();
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
            .FrameBuffer = m_frameBuffers->Get(imageIndex),
            .GraphicsPipeline = m_pipeline->GetGraphicsPipeline(),
            .RenderPass = m_renderPass->Get(),
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

        if (vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, m_syncObjects[m_currentFrame]->GetInFlightFence()) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit draw command buffer");

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
            m_swapChain->Recreate();
            m_frameBuffers->Recreate();
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

    void Vulkan::checkValidationLayerSupport()
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
                throw std::runtime_error("Validation layers requested, but not available");
        }
    }

    void Vulkan::checkGflwRequiredInstanceExtensions(const std::vector<const char*>& requiredExtensions)
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
                throw std::runtime_error("Missing required GLFW extension");
        }
    }

    void Vulkan::createInstance()
    {
        if (ENABLE_VALIDATION_LAYERS)
            checkValidationLayerSupport();

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

        checkGflwRequiredInstanceExtensions(requiredExtensions);

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

            m_debugMessanger->PopulateDebugMessengerCreateInfo(debugCreateInfo);
            instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            instanceInfo.enabledLayerCount = 0;
            instanceInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance");
    }

    void Vulkan::createSurface()
    {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create surface");
    }
}
