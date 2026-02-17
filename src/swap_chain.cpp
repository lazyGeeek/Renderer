#include "renderer/swap_chain.hpp"

#include <limits>
#include <stdexcept>

namespace Renderer
{
    void SwapChain::Create(const SwapChainBuilder& builder)
    {
        if (!builder.Window)
            throw std::runtime_error("[Vulkan][Swap Chain] Windows pointer is nullptr");

        const vk::raii::PhysicalDevice& device = builder.PhysicalDevice;
        const vk::SurfaceKHR& surface = builder.Surface;
        const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = device.getSurfaceCapabilitiesKHR(surface);

        m_surfaceFormat = getSurfaceFormat(device.getSurfaceFormatsKHR(surface));
        m_extent = selectExtent2D(builder.Window, surfaceCapabilities);

        vk::SwapchainCreateInfoKHR swapChainCreateInfo { };
        swapChainCreateInfo.flags            = vk::SwapchainCreateFlagsKHR(),
        swapChainCreateInfo.surface          = surface;
        swapChainCreateInfo.minImageCount    = getMinImageCount(surfaceCapabilities);
        swapChainCreateInfo.imageFormat      = m_surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace  = m_surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent      = m_extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
        swapChainCreateInfo.preTransform     = surfaceCapabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        swapChainCreateInfo.presentMode      = getPresentMode(device.getSurfacePresentModesKHR(surface));
        swapChainCreateInfo.clipped          = true;
        swapChainCreateInfo.oldSwapchain     = nullptr;

        if (builder.GraphicsFamilyIndex != builder.PresentFamilyIndex)
        {
            uint32_t queueFamilyIndices[] = { builder.GraphicsFamilyIndex, builder.PresentFamilyIndex };
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
            swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
            swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        m_swapChain = vk::raii::SwapchainKHR(builder.LogicalDevice, swapChainCreateInfo);
        m_images = m_swapChain.getImages();

        createImageViews(builder.LogicalDevice, m_surfaceFormat.format);
    }

    void SwapChain::Recreate(const SwapChainBuilder& builder)
    {
        int width  = 0;
        int height = 0;
        glfwGetFramebufferSize(builder.Window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(builder.Window, &width, &height);
            glfwWaitEvents();
        }
        
        builder.LogicalDevice.waitIdle();

        cleanupSwapChain();
        Create(builder);
    }

    const vk::raii::SwapchainKHR& SwapChain::Get() const
    {
        return m_swapChain;
    }

    const vk::Extent2D& SwapChain::GetExtent() const
    {
        return m_extent;
    }

    const vk::SurfaceFormatKHR& SwapChain::GetSurfaceFormat() const
    {
        return m_surfaceFormat;
    }

    const vk::Image& SwapChain::GetImage(uint32_t imageIndex)
    {
        if (imageIndex >= m_imageViews.size())
            throw std::runtime_error("[SwapChain][GetImage] Incorrect imageIndex");
            
        return m_images[imageIndex];
    }
    
    const vk::ImageView& SwapChain::GetImageView(uint32_t imageIndex)
    {
        if (imageIndex >= m_imageViews.size())
            throw std::runtime_error("[SwapChain][GetImageView] Incorrect imageIndex");

        return m_imageViews[imageIndex];
    }

    size_t SwapChain::GetImagesCount() const
    {
        return m_images.size();
    }

    vk::ResultValue<uint32_t> SwapChain::AcquireNextImage(const vk::raii::Semaphore& presentCompleteSemaphore) const
    {
        return m_swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);
    }

    vk::PresentModeKHR SwapChain::getPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        if (availablePresentModes.empty())
            throw std::runtime_error("[Vulkan][Swap Chain] There are no available present modes");

        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox)
                return availablePresentMode;
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::SurfaceFormatKHR SwapChain::getSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.empty())
            throw std::runtime_error("[Vulkan][Swap Chain] There are no available formats");

        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return availableFormat;
        }

        return availableFormats[0];
    }
    
    uint32_t SwapChain::getMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities)
    {
        uint32_t minImageCount = std::max(m_defaultImageCount, surfaceCapabilities.minImageCount);
        if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
            minImageCount = surfaceCapabilities.maxImageCount;

        return minImageCount;
    }
        
    vk::Extent2D SwapChain::selectExtent2D(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;

        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(window, &width, &height);

        return
        {
            std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };
    }

    void SwapChain::createImageViews(const vk::raii::Device& device, const vk::Format& format)
    {
        m_imageViews.clear();

        vk::ImageViewCreateInfo createInfo { };

        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;

        createInfo.viewType                        = vk::ImageViewType::e2D;
        createInfo.format                          = format;
        createInfo.subresourceRange                = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        createInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        for (auto image : m_images)
        {
            createInfo.image = image;
            m_imageViews.emplace_back(device, createInfo);
        }
    }

    void SwapChain::cleanupSwapChain()
    {
        m_imageViews.clear();
        m_swapChain = nullptr;
    }
}
