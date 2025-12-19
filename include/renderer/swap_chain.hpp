#pragma once
#ifndef RENDERER_SWAP_CHAIN_HPP_
#define RENDERER_SWAP_CHAIN_HPP_

#include "renderer/device.hpp"
#include "renderer/instance.hpp"

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Renderer
{
    class SyncObject;

    class SwapChain
    {
    public:
        SwapChain(GLFWwindow* window, const Device& device, const Instance& instance);
        ~SwapChain();

        SwapChain(const SwapChain& other)             = delete;
        SwapChain(SwapChain&& other)                  = delete;
        SwapChain& operator=(const SwapChain& other)  = delete;
        SwapChain& operator=(const SwapChain&& other) = delete;

        void Create();
        void Clear();
        void Recreate();

        VkResult PresentKHR(uint32_t imageIndex, const SyncObject& syncObject) const;

        const VkSwapchainKHR& GetSwapChainKHR() const;
        const VkFormat& GetImageFormat() const;
        const VkExtent2D& GetExtent() const;

        const std::vector<VkImageView>& GetImageViews() const;

    private:
        void createImageViews();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        GLFWwindow* m_window = nullptr;
        const Device& m_device;
        const Instance& m_instance;

        VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
    };
}

#endif // RENDERER_SWAP_CHAIN_HPP_