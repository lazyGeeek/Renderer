#pragma once
#ifndef RENDERER_INSTANCE_HPP_
#define RENDERER_INSTANCE_HPP_

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

namespace Renderer
{
    class DebugMessanger;

    class Instance
    {
    public:
        Instance(GLFWwindow* window, bool enableValidationLayer);
        ~Instance();

        Instance(const Instance& other)             = delete;
        Instance(Instance&& other)                  = delete;
        Instance& operator=(const Instance& other)  = delete;
        Instance& operator=(const Instance&& other) = delete;

        void Create();
        void Destroy();

        const VkInstance& GetInstance() const;
        const VkSurfaceKHR& GetSurface() const;

        bool IsValidationLayerEnabled() const;

    private:
        void checkValidationLayerSupport();
        void checkGflwRequiredInstanceExtensions(const std::vector<const char*>& requiredExtensions);

        GLFWwindow* m_window = nullptr;

        std::unique_ptr<DebugMessanger> m_debugMessanger = nullptr;

        VkInstance m_instance  = VK_NULL_HANDLE;
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

        bool m_enableValidationLayer = false;
    };
}

#endif // RENDERER_INSTANCE_HPP_