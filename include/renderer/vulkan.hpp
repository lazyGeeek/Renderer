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
    class Device;
    class CommandBuffers;
    class CommandPool;
    class FrameBuffers;
    class Instance;
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

        bool IsValidationLayerEnabled() const;

    private:
        GLFWwindow* m_window = nullptr;

        std::unique_ptr<CommandBuffers> m_commandBuffers = nullptr;
        std::unique_ptr<CommandPool> m_commandPool       = nullptr;
        std::unique_ptr<Device> m_device                 = nullptr;
        std::unique_ptr<FrameBuffers> m_frameBuffers     = nullptr;
        std::unique_ptr<Instance> m_instance             = nullptr;
        std::unique_ptr<Pipeline> m_pipeline             = nullptr;
        std::unique_ptr<RenderPass> m_renderPass         = nullptr;
        std::unique_ptr<SwapChain> m_swapChain           = nullptr;

        std::vector<std::unique_ptr<SyncObject>> m_syncObjects;

        bool m_framebufferResized = false;
        uint32_t m_currentFrame = 0;

        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
        const bool ENABLE_VALIDATION_LAYERS = false;
#else
        const bool ENABLE_VALIDATION_LAYERS = true;
#endif
    };
}

#endif // RENDERER_VULKAN_HPP_