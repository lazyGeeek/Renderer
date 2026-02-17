#include "renderer/fence.hpp"

namespace Renderer
{
    void Fence::Create(const vk::raii::Device& device)
    {
        vk::FenceCreateInfo fenceCreateInfo { };
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        
        m_fence = vk::raii::Fence(device, fenceCreateInfo);
    }

    const vk::raii::Fence& Fence::Get() const
    {
        return m_fence;
    }
}
