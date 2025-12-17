#pragma once
#ifndef RENDERER_DEVICE_HPP_
#define RENDERER_DEVICE_HPP_

#include "renderer/vulkan.hpp"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace Renderer
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool IsComplete()
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
        Device(const Vulkan& vulkan);
        ~Device();

        Device(const Device& other)             = delete;
        Device(Device&& other)                  = delete;
        Device& operator=(const Device& other)  = delete;
        Device& operator=(const Device&& other) = delete;

        void Create();
        void Destroy();

        void WaitIdle() const;
        
        const VkDevice& GetLogicalDevice() const;
        const VkPhysicalDevice& GetPhysicalDevice() const;
        const VkQueue& GetGraphicsQueue() const;
        const VkQueue& GetPresentQueue() const;
    
        QueueFamilyIndices FindQueueFamilies() const ;
        SwapChainSupportDetails QuerySwapChainSupport() const;

    private:
        VkResult pickPhysicalDevice();
        VkResult createLogicalDevice();

        int rateDeviceSuitability(VkPhysicalDevice device);
        bool isDeviceSuitable();
        bool checkDeviceExtensionSupport();

        const Vulkan& m_vulkan;

        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_logicalDevice          = VK_NULL_HANDLE;
        VkQueue m_graphicsQueue           = VK_NULL_HANDLE;
        VkQueue m_presentQueue            = VK_NULL_HANDLE;

        const std::vector<const char*> m_validationLayers        = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char*> m_logicalDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    };
}

#endif // RENDERER_DEVICE_HPP_