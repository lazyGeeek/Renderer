#pragma once
#ifndef RENDERER_SYNC_OBJECT_HPP_
#define RENDERER_SYNC_OBJECT_HPP_

#include <vulkan/vulkan_raii.hpp>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    class Semaphore : public Interfaces::NonCopyable
    {
    public:
        Semaphore()  = default;
        ~Semaphore() = default;

        void Create(const vk::raii::Device& device);

        const vk::raii::Semaphore& Get() const;
        
    private:
        vk::raii::Semaphore m_semaphore { nullptr };
    };
}

#endif // RENDERERSYNC_OBJECT_HPP_