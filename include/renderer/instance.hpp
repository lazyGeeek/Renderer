#pragma once
#ifndef RENDERER_INSTANCE_HPP_
#define RENDERER_INSTANCE_HPP_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace Renderer
{
    class Instance
    {
    public:
        Instance()  = default;
        ~Instance() = default;

        Instance(const Instance& other)             = delete;
        Instance(Instance&& other)                  = delete;
        Instance& operator=(const Instance& other)  = delete;
        Instance& operator=(const Instance&& other) = delete;

        void Create(GLFWwindow* window);

        const vk::raii::Instance& Get() const;
        const vk::raii::SurfaceKHR& GetSurface() const;

        bool IsValidationLayerEnabled() const { return true; }

    private:
        std::vector<const char*> getRequiredLayers();
        std::vector<const char*> getRequiredExtensions();

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                              void* pUserData);

        void setupDebugMessenger();
        void createSurface(GLFWwindow* window);

        vk::raii::Context m_context;
	    vk::raii::Instance m_instance { nullptr };
        vk::raii::DebugUtilsMessengerEXT m_debugMessenger { nullptr };
        vk::raii::SurfaceKHR m_surface { nullptr };

        const std::vector<char const*> m_validationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };

#ifdef NDEBUG
        bool m_enableValidationLayers = false;
#else
        bool m_enableValidationLayers = true;
#endif
    };
}

#endif // RENDERER_INSTANCE_HPP_