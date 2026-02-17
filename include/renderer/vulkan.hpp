#pragma once
#ifndef RENDERER_VULKAN_HPP_
#define RENDERER_VULKAN_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>
#include <memory>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    class Instance;
    class PhysicalDevice;
    class LogicalDevice;
    class SwapChain;
    class Pipeline;
    class CommandPool;
    class CommandBuffer;
    class SyncObject;

    class Vulkan  : public Interfaces::NonCopyable
    {
    public:
        Vulkan();
        ~Vulkan();

        void Init(GLFWwindow* window, const std::filesystem::path& shaderPath);
        void Destroy();

        void Draw();

    private:
        std::unique_ptr<Instance> m_instance { nullptr };
        std::unique_ptr<PhysicalDevice> m_physicalDevice { nullptr };
        std::unique_ptr<LogicalDevice> m_logicalDevice { nullptr };
        std::unique_ptr<SwapChain> m_swapChain { nullptr };
        std::unique_ptr<Pipeline> m_pipeline { nullptr };
        std::unique_ptr<CommandPool> m_commandPool { nullptr };
        std::unique_ptr<CommandBuffer> m_commandBuffer { nullptr };
        std::unique_ptr<SyncObject> m_syncObject { nullptr };
    };
}

#endif // RENDERER_VULKAN_HPP_