#include "renderer/command_buffer.hpp"

namespace Renderer
{
    void CommandBuffer::Create(const CommandBufferBuilder& builder)
    {
        vk::CommandBufferAllocateInfo allocInfo { };
        allocInfo.commandPool = builder.CommandPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        m_commandBuffer = std::move(vk::raii::CommandBuffers(builder.Device, allocInfo).front());
    }

    void CommandBuffer::RecordCommandBuffer(const RecordCommandBufferBuilder& builder)
    {
        m_commandBuffer.begin({ });

        ImageLayoutBuilder imageLayoutBuilder
        {
            .OldLayout = vk::ImageLayout::eUndefined,
            .NewLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .SrcAccessMask = { }, // srcAccessMask (no need to wait for previous operations)
            .DstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
            .SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
            .DstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
            .Image = builder.Image
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

        m_commandBuffer.beginRendering(renderingInfo);

        m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, builder.GraphicsPipeline);

        m_commandBuffer.setViewport(
            0, vk::Viewport(0.0f, 0.0f,
            static_cast<float>(swapChainExtent.width),
            static_cast<float>(swapChainExtent.height),
            0.0f, 1.0f));

        m_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
        m_commandBuffer.draw(3, 1, 0, 0);
        m_commandBuffer.endRendering();

        imageLayoutBuilder =
        {         
            .OldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .NewLayout = vk::ImageLayout::ePresentSrcKHR,
            .SrcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,            // srcAccessMask
            .DstAccessMask = {},                                                    // dstAccessMask
            .SrcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
            .DstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,              // dstStage
            .Image = builder.Image
        };

        // After rendering, transition the swapchain image to PRESENT_SRC
        transitionImageLayout(imageLayoutBuilder);

        m_commandBuffer.end();
    }

    const vk::raii::CommandBuffer& CommandBuffer::Get() const
    {
        return m_commandBuffer;
    }

    void CommandBuffer::transitionImageLayout(const ImageLayoutBuilder& imageLayoutBuilder)
    {
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
        m_commandBuffer.pipelineBarrier2(dependencyInfo);
    }
}
