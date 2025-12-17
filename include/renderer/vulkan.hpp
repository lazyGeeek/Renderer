#pragma once
#ifndef RENDERER_VULKAN_HPP_
#define RENDERER_VULKAN_HPP_

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>
#include <format>
#include <memory>

namespace Renderer
{
    class Device;
    class CommandBuffers;
    class CommandPool;
    class SyncObject;

    class Vulkan
    {
    public:
        Vulkan(GLFWwindow* window);
        ~Vulkan();

        Vulkan(const Vulkan& other)             = delete;
        Vulkan(Vulkan&& other)                  = delete;
        Vulkan& operator=(const Vulkan& other)  = delete;
        Vulkan& operator=(const Vulkan&& other) = delete;

        void Init(const std::filesystem::path& shaderPath);
        void DrawFrame();
        void SetFramebufferResized();

        const VkInstance& GetInstance() const;
        const VkSurfaceKHR& GetSurface() const;

        bool IsValidationLayerEnabled() const;

    private:
        inline void throwIfFailed(VkResult rs, const char* message)
        {
            if (rs != VK_SUCCESS)
                throw std::runtime_error(std::format("VULKAN: {}", message).c_str());
        }

        VkResult checkValidationLayerSupport();
        VkResult checkGflwRequiredInstanceExtensions(const std::vector<const char*>& requiredExtensions);
        VkResult createInstance();
        VkResult setupDebugMessenger();
        VkResult createSurface();
        VkResult createSwapChain();
        VkResult createImageViews();
        VkResult createRenderPass();
        VkResult createGraphicsPipeline(const std::filesystem::path& shaderPath);
        VkResult createFramebuffers();
        // VkResult createCommandBuffers();
        // VkResult recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        
        void cleanupSwapChain();
        void recreateSwapChain();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        GLFWwindow* m_window = nullptr;

        std::unique_ptr<Device> m_device = nullptr;
        std::unique_ptr<CommandPool> m_commandPool = nullptr;

        VkInstance m_instance                     = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface                    = VK_NULL_HANDLE;
        VkSwapchainKHR m_swapChain                = VK_NULL_HANDLE;
        VkRenderPass m_renderPass                 = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline             = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout         = VK_NULL_HANDLE;
        
        // std::vector<VkCommandBuffer> m_commandBuffers;

        std::unique_ptr<CommandBuffers> m_commandBuffers;
        std::vector<std::unique_ptr<SyncObject>> m_syncObjects;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;

        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
        const bool ENABLE_VALIDATION_LAYERS = false;
#else
        const bool ENABLE_VALIDATION_LAYERS = true;
#endif
    };
}

#endif // RENDERER_VULKAN_HPP_