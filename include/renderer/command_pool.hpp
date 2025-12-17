#pragma once
#ifndef RENDERER_COMMAND_POOL_HPP_
#define RENDERER_COMMAND_POOL_HPP_

#include "renderer/device.hpp"

#include <vulkan/vulkan.h>

namespace Renderer
{
    class CommandPool
    {
    public:
        CommandPool(const Device& device);
        ~CommandPool();

        CommandPool(const CommandPool& other)             = delete;
        CommandPool(CommandPool&& other)                  = delete;
        CommandPool& operator=(const CommandPool& other)  = delete;
        CommandPool& operator=(const CommandPool&& other) = delete;

        void Create();
        void Destroy();

        const VkCommandPool& Get() const;

    private:
        const Device& m_device;
        VkCommandPool m_commandPool = VK_NULL_HANDLE;
    };
}

#endif // RENDERER_COMMAND_POOL_HPP_