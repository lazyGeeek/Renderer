#pragma once
#ifndef RENDERER_FRAME_BUFFERS_HPP_
#define RENDERER_FRAME_BUFFERS_HPP_

#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"
#include "renderer/swap_chain.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace Renderer
{
    struct FrameBuffersInit
    {
        const Device& Device;
        const RenderPass& RenderPass;
        const SwapChain& SwapChain;
    };

    class FrameBuffers
    {
    public:
        FrameBuffers(const FrameBuffersInit& init);
        ~FrameBuffers();

        FrameBuffers(const FrameBuffers& other)             = delete;
        FrameBuffers(FrameBuffers&& other)                  = delete;
        FrameBuffers& operator=(const FrameBuffers& other)  = delete;
        FrameBuffers& operator=(const FrameBuffers&& other) = delete;

        void Create();
        void Destroy();

        void Recreate();

        const VkFramebuffer& Get(size_t imageIndex);

    private:
        const Device& m_device;
        const RenderPass& m_renderPass;
        const SwapChain& m_swapChain;

        std::vector<VkFramebuffer> m_framebuffers;
    };
}

#endif // RENDERER_FRAME_BUFFERS_HPP_