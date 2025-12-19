#pragma once
#ifndef RENDERER_DEBUG_MESSANGER_HPP_
#define RENDERER_DEBUG_MESSANGER_HPP_

#include <vulkan/vulkan.h>

namespace Renderer
{
    class DebugMessanger
    {
    public:
        DebugMessanger(const VkInstance& instance);
        ~DebugMessanger();

        DebugMessanger(const DebugMessanger& other)             = delete;
        DebugMessanger(DebugMessanger&& other)                  = delete;
        DebugMessanger& operator=(const DebugMessanger& other)  = delete;
        DebugMessanger& operator=(const DebugMessanger&& other) = delete;

        void Create();
        void Destroy();

        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    private:
        const VkInstance& m_instance;

        VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    };
}

#endif // RENDERER_DEBUG_MESSANGER_HPP_
