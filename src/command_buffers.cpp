#include "renderer/command_buffers.hpp"

namespace Renderer
{
    void CommandBuffers::Create(const CommandBuffersBuilder& builder)
    {
        m_commandBuffers.clear();

        vk::CommandBufferAllocateInfo allocInfo { };
        allocInfo.commandPool = builder.CommandPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = builder.BuffersCount;

        m_commandBuffers = vk::raii::CommandBuffers(builder.Device, allocInfo);
    }

    void CommandBuffers::RecordCommandBuffer(const RecordCommandBufferBuilder& builder)
    {
        if (builder.FrameIndex >= m_commandBuffers.size())
            throw std::runtime_error("[Command Buffers][RecordCommandBuffer] Incorrect frame index");

        const vk::raii::CommandBuffer& commandBuffer = m_commandBuffers[builder.FrameIndex];
        commandBuffer.begin({ });

        ImageLayoutBuilder imageLayoutBuilder
        {
            .OldLayout = vk::ImageLayout::eUndefined,
            .NewLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .SrcAccessMask = { }, // srcAccessMask (no need to wait for previous operations)
            .DstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
            .SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
            .DstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
            .Image = builder.Image,
            .FrameIndex = builder.FrameIndex
        };

        // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
        transitionImageLayout(imageLayoutBuilder);

        vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        vk::RenderingAttachmentInfo attachmentInfo { };

        attachmentInfo.imageView = builder.ImageView;
        attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
        attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        attachmentInfo.clearValue = clearColor;

        vk::RenderingInfo renderingInfo { };

        const vk::Extent2D& swapChainExtent = builder.SwapChainExtent;
        vk::Rect2D rect2D { };
        rect2D.offset = vk::Offset2D { 0, 0 };
        rect2D.extent = swapChainExtent;
                
        renderingInfo.renderArea = rect2D;
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &attachmentInfo;

        commandBuffer.beginRendering(renderingInfo);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, builder.GraphicsPipeline);

        commandBuffer.setViewport(
            0, vk::Viewport(0.0f, 0.0f,
            static_cast<float>(swapChainExtent.width),
            static_cast<float>(swapChainExtent.height),
            0.0f, 1.0f));

        commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
        commandBuffer.draw(3, 1, 0, 0);
        commandBuffer.endRendering();

        imageLayoutBuilder =
        {         
            .OldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .NewLayout = vk::ImageLayout::ePresentSrcKHR,
            .SrcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,            // srcAccessMask
            .DstAccessMask = {},                                                    // dstAccessMask
            .SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
            .DstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,              // dstStage
            .Image = builder.Image,
            .FrameIndex = builder.FrameIndex
        };

        // After rendering, transition the swapchain image to PRESENT_SRC
        transitionImageLayout(imageLayoutBuilder);

        commandBuffer.end();
    }

    const vk::raii::CommandBuffer& CommandBuffers::Get(uint32_t frameIndex) const
    {
        if (frameIndex >= m_commandBuffers.size())
            throw std::runtime_error("[Command Buffers][RecordCommandBuffer] Incorrect frame index");
        
            return m_commandBuffers[frameIndex];
    }

    void CommandBuffers::Reset(uint32_t frameIndex) const
    {
        if (frameIndex >= m_commandBuffers.size())
            throw std::runtime_error("[Command Buffers][RecordCommandBuffer] Incorrect frame index");
        
        m_commandBuffers[frameIndex].reset();
    }

    void CommandBuffers::transitionImageLayout(const ImageLayoutBuilder& imageLayoutBuilder)
    {
        if (imageLayoutBuilder.FrameIndex >= m_commandBuffers.size())
            return;
        
        vk::ImageMemoryBarrier2 barrier { };
        barrier.srcStageMask = imageLayoutBuilder.SrcStageMask;
        barrier.srcAccessMask = imageLayoutBuilder.SrcAccessMask;
        barrier.dstStageMask = imageLayoutBuilder.DstStageMask;
        barrier.dstAccessMask = imageLayoutBuilder.DstAccessMask;
        barrier.oldLayout = imageLayoutBuilder.OldLayout;
        barrier.newLayout = imageLayoutBuilder.NewLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = imageLayoutBuilder.Image;

        vk::ImageSubresourceRange range { };
        range.aspectMask = vk::ImageAspectFlagBits::eColor,
        range.baseMipLevel = 0,
        range.levelCount = 1,
        range.baseArrayLayer = 0,
        range.layerCount = 1;

        barrier.subresourceRange = range;

        vk::DependencyInfo dependencyInfo { };
        dependencyInfo.dependencyFlags = { };
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;
        m_commandBuffers[imageLayoutBuilder.FrameIndex].pipelineBarrier2(dependencyInfo);
    }
}
