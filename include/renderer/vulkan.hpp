#pragma once
#ifndef RENDERER_VULKAN_HPP_
#define RENDERER_VULKAN_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <filesystem>
#include <memory>

namespace Renderer
{
    class Instance;
    class PhysicalDevice;
    class LogicalDevice;
    class SwapChain;
    class Pipeline;

    class Vulkan
    {
    public:
        Vulkan();
        ~Vulkan();

        Vulkan(const Vulkan& other)             = delete;
        Vulkan(Vulkan&& other)                  = delete;
        Vulkan& operator=(const Vulkan& other)  = delete;
        Vulkan& operator=(const Vulkan&& other) = delete;

        void Init(GLFWwindow* window, const std::filesystem::path& shaderPath);
        void Destroy();

    private:
        std::unique_ptr<Instance> m_instance { nullptr };
        std::unique_ptr<PhysicalDevice> m_physicalDevice { nullptr };
        std::unique_ptr<LogicalDevice> m_logicalDevice { nullptr };
        std::unique_ptr<SwapChain> m_swapChain { nullptr };
        std::unique_ptr<Pipeline> m_pipeline { nullptr };
    };
}

#endif // RENDERER_VULKAN_HPP_