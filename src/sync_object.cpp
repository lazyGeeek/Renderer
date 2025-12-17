#include "renderer/sync_object.hpp"

#include <stdexcept>

namespace Renderer
{
    SyncObject::SyncObject(const Device& device) :
        m_device { device } { }
    
    SyncObject::~SyncObject()
    {
        Destroy();
    }

    void SyncObject::Create()
    {
        VkSemaphoreCreateInfo semaphoreInfo { };
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo { };
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(m_device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(m_device.GetLogicalDevice(), &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphores");
        }
    }

    void SyncObject::Destroy()
    {
        if (m_imageAvailableSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device.GetLogicalDevice(), m_imageAvailableSemaphore, nullptr);
            m_imageAvailableSemaphore = VK_NULL_HANDLE;
        }
        
        if (m_renderFinishedSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(m_device.GetLogicalDevice(), m_renderFinishedSemaphore, nullptr);
            m_renderFinishedSemaphore = VK_NULL_HANDLE;
        }

        if (m_inFlightFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(m_device.GetLogicalDevice(), m_inFlightFence, nullptr);
            m_inFlightFence = VK_NULL_HANDLE;
        }
    }

    const VkSemaphore& SyncObject::GetImageAvailableSemaphore() const
    {
        return m_imageAvailableSemaphore;
    }

    const VkSemaphore& SyncObject::GetRenderFinishedSemaphore() const
    {
        return m_renderFinishedSemaphore;
    }
    
    const VkFence& SyncObject::GetInFlightFence() const
    {
        return m_inFlightFence;
    }

    void SyncObject::WaitForFence() const
    {
        vkWaitForFences(m_device.GetLogicalDevice(), 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    }

    void SyncObject::ResetFence() const
    {
        vkResetFences(m_device.GetLogicalDevice(), 1, &m_inFlightFence);
    }
}
