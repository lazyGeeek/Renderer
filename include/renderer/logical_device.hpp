#pragma once
#ifndef RENDERER_LOGICAL_DEVICE_HPP_
#define RENDERER_LOGICAL_DEVICE_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <vector>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct LogicalDeviceBuilder
    {
        vk::raii::PhysicalDevice PhysicalDevice { nullptr };
        uint32_t PresentIndex;
        uint32_t GraphicsIndex;
        std::vector<const char*> DeviceExtensions;
    };

    struct QueueSubmitBuilder
    {
        const vk::raii::CommandBuffer& CommandBuffer { nullptr }; 
        const vk::raii::Semaphore& PresentCompleteSemaphore { nullptr };
        const vk::raii::Semaphore& RenderFinishedSemaphore { nullptr };
        const vk::raii::Fence& DrawFence { nullptr };
    };

    struct PresentKHRBuider
    {
        const vk::raii::Semaphore& RenderFinishedSemaphore { nullptr };
        const vk::raii::SwapchainKHR& SwapChain { nullptr };
        uint32_t ImageIndex;
    };

    class LogicalDevice : public Interfaces::NonCopyable
    {
    public:
        LogicalDevice()  = default;
        ~LogicalDevice() = default;

        void Create(const LogicalDeviceBuilder& builder);

        const vk::raii::Device& Get() const;

        vk::Result WaitForFence(const vk::raii::Fence& drawFence) const;
        void ResetFence(const vk::raii::Fence& drawFence) const;
        void Submit(const QueueSubmitBuilder& queueSubmitBuilder) const;

        void WaitIdle() const;

        vk::Result PresentKHR(const PresentKHRBuider& builder) const;

    private:
        vk::raii::Device m_device { nullptr };
        vk::raii::Queue m_graphicsQueue { nullptr };
        // vk::raii::Queue m_presentQueue { nullptr };
    };
}

#endif // RENDERER_LOGICAL_DEVICE_HPP_