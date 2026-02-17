#pragma once
#ifndef RENDERER_PHYSICAL_DEVICE_HPP_
#define RENDERER_PHYSICAL_DEVICE_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <vector>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct QueueFamilyIndices
    {
        uint32_t PresentIndex;
        uint32_t GraphicsIndex;
    };

    class PhysicalDevice : public Interfaces::NonCopyable
    {
    public:
        PhysicalDevice()  = default;
        ~PhysicalDevice() = default;

        void Create(const vk::raii::Instance& instance);

        const vk::raii::PhysicalDevice& Get() const;
        
        const std::vector<const char*>& GetDeviceExtensions() const;

        QueueFamilyIndices GetQueueIndex(const vk::SurfaceKHR& surface) const;

    private:
        bool isQueueExist(const vk::raii::PhysicalDevice& device);
        bool isExtensionExist(const vk::raii::PhysicalDevice& device);
        
        // void selectPhysicalDevice(const std::vector<vk::raii::PhysicalDevice>& devices);

        vk::raii::PhysicalDevice m_device { nullptr };

        vk::PhysicalDeviceFeatures m_deviceFeatures;

        std::vector<const char*> m_deviceExtensions =
        {
            vk::KHRSwapchainExtensionName
        };
    };
}

#endif // RENDERER_PHYSICAL_DEVICE_HPP_