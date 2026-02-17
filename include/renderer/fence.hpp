#pragma once
#ifndef RENDERER_FENCE_HPP_
#define RENDERER_FENCE_HPP_

#include <vulkan/vulkan_raii.hpp>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    class Fence : public Interfaces::NonCopyable
    {
    public:
        Fence()  = default;
        ~Fence() = default;

        void Create(const vk::raii::Device& device);

        const vk::raii::Fence& Get() const;
        
    private:
        vk::raii::Fence m_fence { nullptr };
    };
}

#endif // RENDERER_FENCE_HPP_