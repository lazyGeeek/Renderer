#pragma once
#ifndef RENDERER_COMMAND_POOL_HPP_
#define RENDERER_COMMAND_POOL_HPP_

#include <vulkan/vulkan_raii.hpp>

namespace Renderer
{
    struct CommandPoolBuilder
    {
        const vk::raii::Device& Device { nullptr };
        uint32_t QueueFamilyIndex;
    };

    class CommandPool
    {
    public:
        CommandPool()  = default;
        ~CommandPool() = default;

        CommandPool(const CommandPool& other)             = delete;
        CommandPool(CommandPool&& other)                  = delete;
        CommandPool& operator=(const CommandPool& other)  = delete;
        CommandPool& operator=(const CommandPool&& other) = delete;

        void Create(const CommandPoolBuilder& builder);
        
    private:
        vk::raii::CommandPool m_commandPool = nullptr;
    };
}

#endif // RENDERER_COMMAND_POOL_HPP_