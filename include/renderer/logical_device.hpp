#pragma once
#ifndef RENDERER_LOGICAL_DEVICE_HPP_
#define RENDERER_LOGICAL_DEVICE_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace Renderer
{
    struct LogicalDeviceBuilder
    {
        vk::raii::PhysicalDevice PhysicalDevice { nullptr };
        uint32_t PresentIndex;
        uint32_t GraphicsIndex;
        std::vector<const char*> DeviceExtensions;
    };

    class LogicalDevice
    {
    public:
        LogicalDevice()  = default;
        ~LogicalDevice() = default;

        LogicalDevice(const LogicalDevice& other)             = delete;
        LogicalDevice(LogicalDevice&& other)                  = delete;
        LogicalDevice& operator=(const LogicalDevice& other)  = delete;
        LogicalDevice& operator=(const LogicalDevice&& other) = delete;

        void Create(const LogicalDeviceBuilder& builder);

        const vk::raii::Device& Get() const;

    private:
        vk::raii::Device m_device { nullptr };
        vk::raii::Queue m_graphicsQueue { nullptr };
        // vk::raii::Queue m_presentQueue { nullptr };
    };
}

#endif // RENDERER_LOGICAL_DEVICE_HPP_