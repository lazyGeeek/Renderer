#include "renderer/swap_chain.hpp"

namespace Renderer
{
    SwapChain::SwapChain(GLFWwindow* window, const Device& device, const VkSurfaceKHR& surface) :
        m_window { window },
        m_device { device },
        m_surface { surface } { }

    SwapChain::~SwapChain()
    {
        Clear();
    }

    void SwapChain::Create()
    {
        SwapChainSupportDetails swapChainSupport = m_device.QuerySwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo { };
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = m_device.FindQueueFamilies();

        uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

        if (indices.GraphicsFamily != indices.PresentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        const VkDevice& device = m_device.GetLogicalDevice();
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
            throw std::runtime_error("[SwapChain] Failed to create swap chain");

        if (vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("[SwapChain] Failed to get swap chain image");

        m_swapChainImages.resize(imageCount);

        if (vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, m_swapChainImages.data()) != VK_SUCCESS)
            throw std::runtime_error("[SwapChain] Failed to get swap chain image");

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        createImageViews();
    }

    void SwapChain::Clear()
    {
        const VkDevice& device = m_device.GetLogicalDevice();

        for (auto framebuffer : m_swapChainFramebuffers)
        {
            if (framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        m_swapChainFramebuffers.clear();

        for (auto imageView : m_swapChainImageViews)
        {
            if (imageView != VK_NULL_HANDLE)
                vkDestroyImageView(device, imageView, nullptr);
        }

        m_swapChainImageViews.clear();

        if (m_swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(device, m_swapChain, nullptr);
            m_swapChain = VK_NULL_HANDLE;
        }
    }

    void SwapChain::Recreate(const RenderPass& renderPass)
    {
        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(m_window, &width, &height);
        
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }
        
        m_device.WaitIdle();

        Clear();

        Create();
        CreateFramebuffers(renderPass);
    }

    void SwapChain::CreateFramebuffers(const RenderPass& renderPass)
    {
        m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] =
            {
                m_swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo { };
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass.Get();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapChainExtent.width;
            framebufferInfo.height = m_swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_device.GetLogicalDevice(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("[SwapChain] Failed to create framebuffer");
        }
    }

    const VkSwapchainKHR& SwapChain::GetSwapChainKHR() const
    {
        return m_swapChain;
    }

    const VkFramebuffer& SwapChain::GetFramebuffer(size_t imageIndex)
    {
        if (imageIndex >= m_swapChainFramebuffers.size())
            throw std::runtime_error("[SwapChain] Image index buffer is not exist");

        return m_swapChainFramebuffers[imageIndex];
    }


    const VkFormat& SwapChain::GetImageFormat() const
    {
        return m_swapChainImageFormat;
    }

    const VkExtent2D& SwapChain::GetExtent() const
    {
        return m_swapChainExtent;
    }

    void SwapChain::createImageViews()
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_device.GetLogicalDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("[SwapChain] Failed to create image view");

        }
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
}
