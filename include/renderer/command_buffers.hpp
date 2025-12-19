#pragma once
#ifndef RENDERER_COMMAND_BUFFERS_HPP_
#define RENDERER_COMMAND_BUFFERS_HPP_

#include "renderer/command_pool.hpp"
#include "renderer/device.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace Renderer
{
    class SyncObject;

    struct CommandBufferRecordInfo
    {
        const VkFramebuffer& FrameBuffer;
        const VkPipeline& GraphicsPipeline;
        const VkRenderPass& RenderPass;
        const VkExtent2D& SwapChainExtent;
    };

    class CommandBuffers
    {
    public:
        CommandBuffers(const Device& device);
        ~CommandBuffers() = default;

        CommandBuffers(const CommandBuffers& other)             = delete;
        CommandBuffers(CommandBuffers&& other)                  = delete;
        CommandBuffers& operator=(const CommandBuffers& other)  = delete;
        CommandBuffers& operator=(const CommandBuffers&& other) = delete;

        void Create(const VkCommandPool& commandPool, size_t size);

        const VkCommandBuffer& Get(size_t frameIndex) const;

        void Reset(size_t frameIndex) const;
        void Record(size_t frameIndex, const CommandBufferRecordInfo& recordInfo) const;

        void SubmitQueue(uint32_t frameIndex, const SyncObject& syncObject) const;

    private:
        const Device& m_device;
        size_t m_frameNumbers = 0;
        std::vector<VkCommandBuffer> m_commandBuffers;
    };
}

#endif // RENDERER_COMMAND_BUFFERS_HPP_