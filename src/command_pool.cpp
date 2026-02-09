#include "renderer/command_pool.hpp"

namespace Renderer
{
    void CommandPool::Create(const CommandPoolBuilder& builder)
    {
        vk::CommandPoolCreateInfo poolInfo { };
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = builder.QueueFamilyIndex;

        m_commandPool = vk::raii::CommandPool(builder.Device, poolInfo);
    }
}
