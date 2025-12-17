#include "renderer/device.hpp"

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace Renderer
{
    Device::Device(const Vulkan& vulkan) : m_vulkan { vulkan } { }

    Device::~Device()
    {
        Destroy();
    }

    void Device::Create()
    {
        if (pickPhysicalDevice() != VK_SUCCESS)
            throw std::runtime_error("Failed to find GPUs with Vulkan support");

        if (createLogicalDevice() != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device");
    }

    void Device::Destroy()
    {
        if (m_logicalDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_logicalDevice, nullptr);
            m_logicalDevice = VK_NULL_HANDLE;
        }
    }

    void Device::WaitIdle() const
    {
        vkDeviceWaitIdle(m_logicalDevice);
    }
    
    const VkDevice& Device::GetLogicalDevice() const
    {
        return m_logicalDevice;
    }

    const VkPhysicalDevice& Device::GetPhysicalDevice() const
    {
        return m_physicalDevice;
    }

    const VkQueue& Device::GetGraphicsQueue() const
    {
        return m_graphicsQueue;
    }
    
    const VkQueue& Device::GetPresentQueue() const
    {
        return m_presentQueue;
    }

    QueueFamilyIndices Device::FindQueueFamilies() const
    {
        QueueFamilyIndices indices;

        // Logic to find queue family indices to populate struct with
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.GraphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_vulkan.GetSurface(), &presentSupport);

            if (presentSupport)
                indices.PresentFamily = i;

            if (indices.IsComplete())
                break;

            i++;
        }

        return indices;
    }

    SwapChainSupportDetails Device::QuerySwapChainSupport() const
    {
        SwapChainSupportDetails details { };

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_vulkan.GetSurface(), &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_vulkan.GetSurface(), &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_vulkan.GetSurface(), &formatCount, details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_vulkan.GetSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_vulkan.GetSurface(), &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    int Device::rateDeviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties { };
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        
        VkPhysicalDeviceFeatures deviceFeatures { };
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        
        int score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader)
            return 0;

        return score;
    }

    VkResult Device::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_vulkan.GetInstance(), &deviceCount, nullptr);

        if (deviceCount == 0)
            return VK_ERROR_INCOMPATIBLE_DRIVER;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_vulkan.GetInstance(), &deviceCount, devices.data());

        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices)
        {
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0)
            m_physicalDevice = candidates.rbegin()->second;
        else
            return VK_ERROR_INCOMPATIBLE_DRIVER;
        
        return VK_SUCCESS;
    }

    bool Device::checkDeviceExtensionSupport()
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_logicalDeviceExtensions.begin(), m_logicalDeviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool Device::isDeviceSuitable()
    {
        QueueFamilyIndices indices = FindQueueFamilies();
        
        bool extensionsSupported = checkDeviceExtensionSupport();
        bool swapChainAdequate = false;

        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport();
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.IsComplete() && extensionsSupported && swapChainAdequate;
    }

    VkResult Device::createLogicalDevice()
    {
        QueueFamilyIndices indices = FindQueueFamilies();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies =
        {
            indices.GraphicsFamily.value(),
            indices.PresentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo { };
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures { };
        
        VkDeviceCreateInfo createInfo { };
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_logicalDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_logicalDeviceExtensions.data();

        if (m_vulkan.IsValidationLayerEnabled())
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
            createInfo.ppEnabledLayerNames = m_validationLayers.data();
        }
        else
            createInfo.enabledLayerCount = 0;

        VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice);
        if (result != VK_SUCCESS)
            return result;

        vkGetDeviceQueue(m_logicalDevice, indices.GraphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_logicalDevice, indices.PresentFamily.value(), 0, &m_presentQueue);

        return result;
    }
}
