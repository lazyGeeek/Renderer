#include "renderer/command_pool.hpp"

namespace Renderer
{
    CommandPool::CommandPool(const Device& device) : m_device { device } { }

    CommandPool::~CommandPool()
    {
        Destroy();
    }

    void CommandPool::Create()
    {
        QueueFamilyIndices queueFamilyIndices = m_device.FindQueueFamilies();

        VkCommandPoolCreateInfo poolInfo { };
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

        if (vkCreateCommandPool(m_device.GetLogicalDevice(), &poolInfo, nullptr, &m_commandPool))
            throw std::runtime_error("Failed to create command pool");
    }

    void CommandPool::Destroy()
    {
        if (m_commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_device.GetLogicalDevice(), m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }
    }

    const VkCommandPool& CommandPool::Get() const
    {
        return m_commandPool;
    }
}
