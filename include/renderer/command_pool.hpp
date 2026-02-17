#pragma once
#ifndef RENDERER_COMMAND_POOL_HPP_
#define RENDERER_COMMAND_POOL_HPP_

#include <vulkan/vulkan_raii.hpp>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct CommandPoolBuilder
    {
        const vk::raii::Device& Device { nullptr };
        uint32_t QueueFamilyIndex;
    };

    class CommandPool : public Interfaces::NonCopyable
    {
    public:
        CommandPool()  = default;
        ~CommandPool() = default;

        void Create(const CommandPoolBuilder& builder);

        const vk::raii::CommandPool& Get() const;
        
    private:
        vk::raii::CommandPool m_commandPool = nullptr;
    };
}

#endif // RENDERER_COMMAND_POOL_HPP_