#include "renderer/semaphore.hpp"

namespace Renderer
{
    void Semaphore::Create(const vk::raii::Device& device)
    {
        m_semaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
    }

    const vk::raii::Semaphore& Semaphore::Get() const
    {
        return m_semaphore;
    }
}
