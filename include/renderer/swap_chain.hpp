#pragma once
#ifndef RENDERER_SWAP_CHAIN_HPP_
#define RENDERER_SWAP_CHAIN_HPP_

#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct SwapChainBuilder
    {
        vk::raii::PhysicalDevice PhysicalDevice { nullptr };
        const vk::raii::Device& LogicalDevice;
        vk::SurfaceKHR Surface;
        GLFWwindow* Window { nullptr };
        uint32_t GraphicsFamilyIndex;
        uint32_t PresentFamilyIndex;
    };

    class SwapChain : public Interfaces::NonCopyable
    {
    public:
        SwapChain()  = default;
        ~SwapChain() = default;

        void Create(const SwapChainBuilder& builder);

        const vk::raii::SwapchainKHR& Get() const;
        const vk::Extent2D& GetExtent() const;
        const vk::SurfaceFormatKHR& GetSurfaceFormat() const;
        const vk::Image& GetImage(uint32_t imageIndex);
        const vk::ImageView& GetImageView(uint32_t imageIndex);

        vk::ResultValue<uint32_t> AcquireNextImage(const vk::raii::Semaphore& presentCompleteSemaphore) const;

    private:
        static vk::PresentModeKHR getPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
        static vk::SurfaceFormatKHR getSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        static uint32_t getMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);

        vk::Extent2D selectExtent2D(GLFWwindow* window, const vk::SurfaceCapabilitiesKHR& capabilities);

        void createImageViews(const vk::raii::Device& device, const vk::Format& format);

        vk::raii::SwapchainKHR m_swapChain { nullptr };
        vk::SurfaceFormatKHR m_surfaceFormat;
        vk::Extent2D m_extent;

        std::vector<vk::Image> m_images;
        std::vector<vk::raii::ImageView> m_imageViews;

        inline static uint32_t m_defaultImageCount { 3u };
    };
}

#endif // RENDERER_SWAP_CHAIN_HPP_