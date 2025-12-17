#pragma once
#ifndef RENDERER_SYNC_OBJECT_HPP_
#define RENDERER_SYNC_OBJECT_HPP_

#include "renderer/device.hpp"

#include <vulkan/vulkan.h>

namespace Renderer
{
    class SyncObject
    {
    public:
        SyncObject(const Device& device);
        ~SyncObject();

        SyncObject(const SyncObject& other)             = delete;
        SyncObject(SyncObject&& other)                  = delete;
        SyncObject& operator=(const SyncObject& other)  = delete;
        SyncObject& operator=(const SyncObject&& other) = delete;

        void Create();
        void Destroy();

        const VkSemaphore& GetImageAvailableSemaphore() const;
        const VkSemaphore& GetRenderFinishedSemaphore() const;
        const VkFence& GetInFlightFence() const;

        void WaitForFence() const;
        void ResetFence() const;

    private:
        const Device& m_device;

        VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
        VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
        VkFence m_inFlightFence               = VK_NULL_HANDLE;
    };
}

#endif // RENDERER_SYNC_OBJECT_HPP_