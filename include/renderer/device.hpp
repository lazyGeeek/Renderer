#pragma once
#ifndef RENDERER_DEVICE_HPP_
#define RENDERER_DEVICE_HPP_

// #include "renderer/shader.hpp"

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>
#include <format>
#include <optional>

namespace Renderer
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool isComplete()
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class Device
    {
    public:
        Device(GLFWwindow* window, const std::filesystem::path& shaderPath);
        ~Device();

        Device(const Device& other)             = delete;
        Device(Device&& other)                  = delete;
        Device& operator=(const Device& other)  = delete;
        Device& operator=(const Device&& other) = delete;
        
        void DrawFrame();
        void SetFramebufferResized();

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
        VkResult pickPhysicalDevice();
        VkResult createLogicalDevice();
        VkResult createSwapChain();
        VkResult createImageViews();
        VkResult createRenderPass();
        VkResult createGraphicsPipeline(const std::filesystem::path& shaderPath);
        VkResult createFramebuffers();
        VkResult createCommandPool();
        VkResult createCommandBuffers();
        VkResult recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        VkResult createSyncObjects();
        
        void cleanupSwapChain();
        void recreateSwapChain();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        int rateDeviceSuitability(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        GLFWwindow* m_window = nullptr;

        VkInstance m_instance                     = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
        VkPhysicalDevice m_physicalDevice         = VK_NULL_HANDLE;
        VkDevice m_device                         = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface                    = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue                   = VK_NULL_HANDLE;
        VkQueue m_presentQueue                    = VK_NULL_HANDLE;
        VkSwapchainKHR m_swapChain                = VK_NULL_HANDLE;
        VkRenderPass m_renderPass                 = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline             = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout         = VK_NULL_HANDLE;
        VkCommandPool m_commandPool               = VK_NULL_HANDLE;
        
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;

        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
        const bool ENABLE_VALIDATION_LAYERS = false;
#else
        const bool ENABLE_VALIDATION_LAYERS = true;
#endif
    };
}

#endif // RENDERER_DEVICE_HPP_