#pragma once
#ifndef RENDERER_SYNC_OBJECT_HPP_
#define RENDERER_SYNC_OBJECT_HPP_

#include <vulkan/vulkan_raii.hpp>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct SyncObjectBuilder
    {
        const vk::raii::Device& Device { nullptr };
    };

    class SyncObject : public Interfaces::NonCopyable
    {
    public:
        SyncObject()  = default;
        ~SyncObject() = default;

        void Create(const SyncObjectBuilder& builder);

        const vk::raii::Semaphore& GetPresentCompleteSemaphore() const;
        const vk::raii::Semaphore& GetRenderFinishedSemaphore() const;
        const vk::raii::Fence& GetDrawFence() const;
        
    private:
        vk::raii::Semaphore m_presentCompleteSemaphore { nullptr };
        vk::raii::Semaphore m_renderFinishedSemaphore { nullptr };
        vk::raii::Fence m_drawFence { nullptr };
    };
}

#endif // RENDERERSYNC_OBJECT_HPP_