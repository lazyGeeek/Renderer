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

}
