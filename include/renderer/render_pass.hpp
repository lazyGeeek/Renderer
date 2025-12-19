#pragma once
#ifndef RENDERER_RENDER_PASS_HPP_
#define RENDERER_RENDER_PASS_HPP_

#include "renderer/device.hpp"
#include "renderer/swap_chain.hpp"

#include <vulkan/vulkan.h>

namespace Renderer
{
    class RenderPass
    {
    public:
        RenderPass(const Device& device, const SwapChain& swapChain);
        ~RenderPass();

        RenderPass(const RenderPass& other)             = delete;
        RenderPass(RenderPass&& other)                  = delete;
        RenderPass& operator=(const RenderPass& other)  = delete;
        RenderPass& operator=(const RenderPass&& other) = delete;

        void Create();
        void Destroy();

        const VkRenderPass& Get() const;
        
    private:
        const Device& m_device;
        const SwapChain& m_swapChain;

        VkRenderPass m_renderPass = VK_NULL_HANDLE;
    };
}

#endif // RENDERER_RENDER_PASS_HPP_