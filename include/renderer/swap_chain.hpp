#pragma once
#ifndef RENDERER_SWAP_CHAIN_HPP_
#define RENDERER_SWAP_CHAIN_HPP_

#include "renderer/device.hpp"

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Renderer
{
    class SwapChain
    {
    public:
        SwapChain(GLFWwindow* window, const Device& device, const VkSurfaceKHR& surface);
        ~SwapChain();

        SwapChain(const SwapChain& other)             = delete;
        SwapChain(SwapChain&& other)                  = delete;
        SwapChain& operator=(const SwapChain& other)  = delete;
        SwapChain& operator=(const SwapChain&& other) = delete;

        void Create();
        void Clear();
        void Recreate(const VkRenderPass& renderPass);
        void CreateFramebuffers(const VkRenderPass& renderPass);

        const VkSwapchainKHR& GetSwapChainKHR() const;
        const VkFramebuffer& GetFramebuffer(size_t imageIndex);
        const VkFormat& GetImageFormat() const;
        const VkExtent2D& GetExtent() const;

    private:
        void createImageViews();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        GLFWwindow* m_window = nullptr;
        const Device& m_device;
        const VkSurfaceKHR& m_surface;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
    };
}

#endif // RENDERER_SWAP_CHAIN_HPP_