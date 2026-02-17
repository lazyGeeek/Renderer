#pragma once
#ifndef RENDERER_COMMAND_BUFFER_HPP_
#define RENDERER_COMMAND_BUFFER_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <vector>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct CommandBuffersBuilder
    {
        const vk::raii::Device& Device { nullptr };
        const vk::raii::CommandPool& CommandPool { nullptr };
        uint32_t BuffersCount;
    };

    struct RecordCommandBufferBuilder
    {
        const vk::Image& Image;
        const vk::ImageView& ImageView;
        const vk::Extent2D& SwapChainExtent;
        const vk::raii::Pipeline& GraphicsPipeline { nullptr };
        uint32_t FrameIndex;
    };

    class CommandBuffers : public Interfaces::NonCopyable
    {
    public:
        CommandBuffers()  = default;
        ~CommandBuffers() = default;

        void Create(const CommandBuffersBuilder& builder);

        void RecordCommandBuffer(const RecordCommandBufferBuilder& builder);

        const vk::raii::CommandBuffer& Get(uint32_t frameIndex) const;
        void Reset(uint32_t frameIndex) const;
        
    private:
        struct ImageLayoutBuilder
        {
            vk::ImageLayout OldLayout;
            vk::ImageLayout NewLayout;
            vk::AccessFlags2 SrcAccessMask;
            vk::AccessFlags2 DstAccessMask;
            vk::PipelineStageFlags2 SrcStageMask;
            vk::PipelineStageFlags2 DstStageMask;
            vk::Image Image;
            uint32_t FrameIndex;
        };

        void transitionImageLayout(const ImageLayoutBuilder& imageLayoutBuilder);

        std::vector<vk::raii::CommandBuffer> m_commandBuffers;
    };
}

#endif // RENDERER_COMMAND_BUFFER_HPP_