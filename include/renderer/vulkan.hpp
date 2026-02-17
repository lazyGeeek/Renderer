#pragma once
#ifndef RENDERER_VULKAN_HPP_
#define RENDERER_VULKAN_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    class Instance;
    class PhysicalDevice;
    class LogicalDevice;
    class SwapChain;
    class Pipeline;
    class CommandPool;
    class CommandBuffers;
    class Semaphore;
    class Fence;

    class Vulkan  : public Interfaces::NonCopyable
    {
    public:
        Vulkan();
        ~Vulkan();

        void Init(GLFWwindow* window, const std::filesystem::path& shaderPath);
        void Destroy();

        void Draw(GLFWwindow* window);

        void ResizeFramebuffer();

    private:
        void recreateSwapChain(GLFWwindow* window);

        std::unique_ptr<Instance> m_instance { nullptr };
        std::unique_ptr<PhysicalDevice> m_physicalDevice { nullptr };
        std::unique_ptr<LogicalDevice> m_logicalDevice { nullptr };
        std::unique_ptr<SwapChain> m_swapChain { nullptr };
        std::unique_ptr<Pipeline> m_pipeline { nullptr };
        std::unique_ptr<CommandPool> m_commandPool { nullptr };
        std::unique_ptr<CommandBuffers> m_commandBuffers { nullptr };

        std::vector<std::unique_ptr<Semaphore>> m_presentCompleteSemaphores;
        std::vector<std::unique_ptr<Semaphore>> m_renderFinishedSemaphores;
        std::vector<std::unique_ptr<Fence>> m_inFlightFences;

        uint32_t m_frameIndex = 0;
        bool m_framebufferResized = false;

        const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    };
}

#endif // RENDERER_VULKAN_HPP_