#include "renderer/logical_device.hpp"

#include <vector>

namespace Renderer
{
    void LogicalDevice::Create(const LogicalDeviceBuilder& builder)
    {     
        // Create a chain of feature structures
        vk::PhysicalDeviceVulkan13Features vulkan13Features { };
        vulkan13Features.dynamicRendering = true;

        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicState { };
        dynamicState.extendedDynamicState = true;

        vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain
        {
            { },                 // vk::PhysicalDeviceFeatures2 (empty for now)
            vulkan13Features,    // Enable dynamic rendering from Vulkan 1.3
            dynamicState         // Enable extended dynamic state from the extension
        };

        float queuePriority = 0.5f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo { };
        deviceQueueCreateInfo.queueFamilyIndex = builder.GraphicsIndex;
        deviceQueueCreateInfo.queueCount       = 1;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

        vk::DeviceCreateInfo deviceCreateInfo { };
        deviceCreateInfo.pNext                   = &featureChain.get<vk::PhysicalDeviceFeatures2>();
        deviceCreateInfo.queueCreateInfoCount    = 1;
        deviceCreateInfo.pQueueCreateInfos       = &deviceQueueCreateInfo;
        deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(builder.DeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = builder.DeviceExtensions.data();

        m_device = vk::raii::Device(builder.PhysicalDevice, deviceCreateInfo);
        m_graphicsQueue = vk::raii::Queue(m_device, builder.GraphicsIndex, 0);
        // m_presentQueue = vk::raii::Queue(m_device, builder.PresentIndex, 0);
    }

    const vk::raii::Device& LogicalDevice::Get() const
    {
        return m_device;
    }

    vk::Result LogicalDevice::WaitForFence(const vk::raii::Fence& drawFence) const
    {
        return m_device.waitForFences(*drawFence, vk::True, UINT64_MAX);
    }

    void LogicalDevice::ResetFence(const vk::raii::Fence& drawFence) const
    {
        m_device.resetFences(*drawFence);
    }

    void LogicalDevice::Submit(const QueueSubmitBuilder& queueSubmitBuilder) const
    {
        vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

        vk::SubmitInfo submitInfo { };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &*queueSubmitBuilder.PresentCompleteSemaphore;
        submitInfo.pWaitDstStageMask = &waitDestinationStageMask;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &*queueSubmitBuilder.CommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &*queueSubmitBuilder.RenderFinishedSemaphore;
        
        m_graphicsQueue.submit(submitInfo, *queueSubmitBuilder.DrawFence);
    }

    void LogicalDevice::WaitIdle() const
    {
        m_device.waitIdle();
    }

    vk::Result LogicalDevice::PresentKHR(const PresentKHRBuider& builder) const
    {
        vk::PresentInfoKHR presentInfoKHR { };
        presentInfoKHR.waitSemaphoreCount = 1;
        presentInfoKHR.pWaitSemaphores = &*builder.RenderFinishedSemaphore;
        presentInfoKHR.swapchainCount = 1;
        presentInfoKHR.pSwapchains = &*builder.SwapChain;
        presentInfoKHR.pImageIndices = &builder.ImageIndex;
        presentInfoKHR.pResults = nullptr; // Optional

        return m_graphicsQueue.presentKHR(presentInfoKHR);
    }

}
