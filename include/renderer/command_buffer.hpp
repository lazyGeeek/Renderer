#pragma once
#ifndef RENDERER_COMMAND_BUFFER_HPP_
#define RENDERER_COMMAND_BUFFER_HPP_

#include <vulkan/vulkan_raii.hpp>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct CommandBufferBuilder
    {
        const vk::raii::Device& Device { nullptr };
        const vk::raii::CommandPool& CommandPool { nullptr };
    };

    struct RecordCommandBufferBuilder
    {
        const vk::Image& Image;
        const vk::ImageView& ImageView;
        const vk::Extent2D& SwapChainExtent;
        const vk::raii::Pipeline& GraphicsPipeline { nullptr };
    };

    class CommandBuffer : public Interfaces::NonCopyable
    {
    public:
        CommandBuffer()  = default;
        ~CommandBuffer() = default;

        void Create(const CommandBufferBuilder& builder);

        void RecordCommandBuffer(const RecordCommandBufferBuilder& builder);

        const vk::raii::CommandBuffer& Get() const;
        
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
        };

        void transitionImageLayout(const ImageLayoutBuilder& imageLayoutBuilder);

        vk::raii::CommandBuffer m_commandBuffer = nullptr;
    };
}

#endif // RENDERER_COMMAND_BUFFER_HPP_