#include "renderer/sync_object.hpp"

namespace Renderer
{
    void SyncObject::Create(const SyncObjectBuilder& builder)
    {
        const vk::raii::Device& device = builder.Device;
        m_presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
        m_renderFinishedSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());

        vk::FenceCreateInfo fenceCreateInfo { };
        fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
        
        m_drawFence = vk::raii::Fence(device, fenceCreateInfo);
    }

    const vk::raii::Semaphore& SyncObject::GetPresentCompleteSemaphore() const
    {
        return m_renderFinishedSemaphore;
    }

    const vk::raii::Semaphore& SyncObject::GetRenderFinishedSemaphore() const
    {
        return m_renderFinishedSemaphore;
    }

    const vk::raii::Fence& SyncObject::GetDrawFence() const
    {
        return m_drawFence;
    }
}
