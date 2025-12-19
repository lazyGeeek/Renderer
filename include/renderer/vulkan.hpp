#pragma once
#ifndef RENDERER_VULKAN_HPP_
#define RENDERER_VULKAN_HPP_

#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>
#include <format>
#include <memory>

namespace Renderer
{
    class DebugMessanger;
    class Device;
    class CommandBuffers;
    class CommandPool;
    class FrameBuffers;
    class Pipeline;
    class RenderPass;
    class SyncObject;
    class SwapChain;

    class Vulkan
    {
    public:
        Vulkan(GLFWwindow* window);
        ~Vulkan();

        Vulkan(const Vulkan& other)             = delete;
        Vulkan(Vulkan&& other)                  = delete;
        Vulkan& operator=(const Vulkan& other)  = delete;
        Vulkan& operator=(const Vulkan&& other) = delete;

        void Init(const std::filesystem::path& shaderPath);
        void DrawFrame();
        void SetFramebufferResized();

        const VkInstance& GetInstance() const;
        const VkSurfaceKHR& GetSurface() const;

        bool IsValidationLayerEnabled() const;

    private:
        void checkValidationLayerSupport();
        void checkGflwRequiredInstanceExtensions(const std::vector<const char*>& requiredExtensions);
        void createInstance();
        void createSurface();

        GLFWwindow* m_window = nullptr;

        VkInstance m_instance                     = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface                    = VK_NULL_HANDLE;

        std::unique_ptr<CommandBuffers> m_commandBuffers = nullptr;
        std::unique_ptr<CommandPool> m_commandPool       = nullptr;
        std::unique_ptr<DebugMessanger> m_debugMessanger = nullptr;
        std::unique_ptr<Device> m_device                 = nullptr;
        std::unique_ptr<FrameBuffers> m_frameBuffers     = nullptr;
        std::unique_ptr<Pipeline> m_pipeline             = nullptr;
        std::unique_ptr<RenderPass> m_renderPass         = nullptr;
        std::unique_ptr<SwapChain> m_swapChain           = nullptr;

        std::vector<std::unique_ptr<SyncObject>> m_syncObjects;

        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;

        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
        const bool ENABLE_VALIDATION_LAYERS = false;
#else
        const bool ENABLE_VALIDATION_LAYERS = true;
#endif
    };
}

#endif // RENDERER_VULKAN_HPP_