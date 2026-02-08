#include "renderer/physical_device.hpp"

#include <algorithm>
#include <map>
#include <ranges>

namespace Renderer
{
    void PhysicalDevice::Create(const vk::raii::Instance& instance)
    {
        auto devices = instance.enumeratePhysicalDevices();

        if (devices.empty())
            throw std::runtime_error("[Vulkan][Physical Device] Failed to find GPUs with Vulkan support");

        // selectPhysicalDevice(devices);

        const auto deviceIt = std::ranges::find_if(devices, [&](const auto& device)
        {
            bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
            isSuitable = isSuitable && isQueueExist(device) && isExtensionExist(device);
            
            if (isSuitable)
                m_device = device;
            
            return isSuitable;
        });

        if (deviceIt == devices.end())
            throw std::runtime_error("[Vulkan][Physical Device] Failed to find a suitable GPU");
    }

    const std::vector<const char*>& PhysicalDevice::GetDeviceExtensions() const
    {
        return m_deviceExtensions;
    }

    QueueFamilyIndices PhysicalDevice::GetQueueIndex(const vk::SurfaceKHR& surface) const
    {
        QueueFamilyIndices indices { };

        // Find the index of the first queue family that supports graphics
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_device.getQueueFamilyProperties();

        // Get the first index into queueFamilyProperties which supports graphics
        auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](const auto& qfp)
        {
            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        });

        auto graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

        // Determine a queueFamilyIndex that supports present
        // First check if the graphicsIndex is good enough
        auto presentIndex = m_device.getSurfaceSupportKHR(graphicsIndex, surface)
                                        ? graphicsIndex
                                        : static_cast<uint32_t>(queueFamilyProperties.size());
        
        if (presentIndex == queueFamilyProperties.size())
        {
            // The graphicsIndex doesn't support present -> look for another family index that supports both
            // Graphics and present
            for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
            {
                if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                    m_device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
                {
                    graphicsIndex = static_cast<uint32_t>(i);
                    presentIndex  = graphicsIndex;
                    break;
                }
            }

            if (presentIndex == queueFamilyProperties.size())
            {
                // There's nothing like a single family index that supports both graphics and present -> look for another
                // Family index that supports present
                for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
                {
                    if (m_device.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
                    {
                        presentIndex = static_cast<uint32_t>(i);
                        break;
                    }
                }
            }
        }
        
        if ((graphicsIndex == queueFamilyProperties.size()) || (presentIndex == queueFamilyProperties.size()))
        {
            throw std::runtime_error("[Vulkan][Physical Device] Could not find a queue for graphics or present");
        }

        return 
        {
            .PresentIndex = presentIndex,
            .GraphicsIndex = graphicsIndex
        };
    }

    bool PhysicalDevice::isQueueExist(const vk::raii::PhysicalDevice& device)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = device.getQueueFamilyProperties();
        const auto qfpIter = std::ranges::find_if(queueFamilyProperties, [](const vk::QueueFamilyProperties& qfp)
        {
            return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        });

        return qfpIter != queueFamilyProperties.end();
    }

    const vk::raii::PhysicalDevice& PhysicalDevice::Get() const
    {
        return m_device;
    }

    bool PhysicalDevice::isExtensionExist(const vk::raii::PhysicalDevice& device)
    {
        auto extensions = device.enumerateDeviceExtensionProperties( );
        bool found = true;

        for (auto const & extension : m_deviceExtensions)
        {
            auto extensionIter = std::ranges::find_if(extensions, [extension](const auto& ext)
            {
                return strcmp(ext.extensionName, extension) == 0;
            });

            found = found && extensionIter != extensions.end();
        }

        return found;
    }

    // void PhysicalDevice::selectPhysicalDevice(const std::vector<vk::raii::PhysicalDevice>& devices)
    // {
    //     // Use an ordered map to automatically sort candidates by increasing score
    //     std::multimap<int, vk::raii::PhysicalDevice> candidates;

    //     for (const auto& device : devices)
    //     {
    //         auto deviceProperties = m_physicalDevice.getProperties();
    //         auto deviceFeatures = m_physicalDevice.getFeatures();

    //         uint32_t score = 0;

    //         if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
    //             score += 1000;

    //         // Maximum possible size of textures affects graphics quality
    //         score += deviceProperties.limits.maxImageDimension2D;

    //         // Application can't function without geometry shaders
    //         if (!deviceFeatures.geometryShader)
    //             continue;

    //         candidates.insert(std::make_pair(score, device));
    //     }

    //     // Check if the best candidate is suitable at all
    //     if (candidates.rbegin()->first > 0)
    //         m_physicalDevice = candidates.rbegin()->second;
    //     else
    //         throw std::runtime_error("[Vulkan][Physical Device] Failed to find a suitable GPU");
    // }
}
