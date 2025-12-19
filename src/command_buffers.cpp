#include "renderer/command_buffers.hpp"
#include "renderer/command_pool.hpp"
#include "renderer/sync_object.hpp"

namespace Renderer
{
    CommandBuffers::CommandBuffers(const Device& device) :
        m_device { device } { }

    void CommandBuffers::Create(const VkCommandPool& commandPool, const size_t frameNumbers)
    {
        m_frameNumbers = frameNumbers;

        m_commandBuffers.resize(m_frameNumbers);

        VkCommandBufferAllocateInfo allocInfo { };
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        if (vkAllocateCommandBuffers(m_device.GetLogicalDevice(), &allocInfo, m_commandBuffers.data()))
            throw std::runtime_error("Failed to create allocate buffer");
    }

    const VkCommandBuffer& CommandBuffers::Get(size_t frameIndex) const
    {
        if (frameIndex > m_frameNumbers)
            throw std::runtime_error("[Command Buffers] Incorrect buffer number");
        
            return m_commandBuffers[frameIndex];
    }

    void CommandBuffers::Reset(size_t frameIndex) const
    {
        if (frameIndex > m_frameNumbers)
            throw std::runtime_error("[Command Buffers] Incorrect buffer number");

        if (vkResetCommandBuffer(m_commandBuffers[frameIndex], 0) != VK_SUCCESS)
            throw std::runtime_error("[Command Buffers] Failed to reset command buffer");
    }

    void CommandBuffers::Record(size_t frameIndex, const CommandBufferRecordInfo& recordInfo) const
    {
        if (frameIndex > m_frameNumbers)
            throw std::runtime_error("[Command Buffers] Incorrect buffer number");

        const VkCommandBuffer& commandBuffer = m_commandBuffers[frameIndex];

        VkCommandBufferBeginInfo beginInfo { };
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("[Command Buffers] Failed to begin command buffer");

        VkRenderPassBeginInfo renderPassInfo { };
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = recordInfo.RenderPass;
        renderPassInfo.framebuffer = recordInfo.FrameBuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = recordInfo.SwapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, recordInfo.GraphicsPipeline);

        VkViewport viewport { };
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(recordInfo.SwapChainExtent.width);
        viewport.height = static_cast<float>(recordInfo.SwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor { };
        scissor.offset = { 0, 0 };
        scissor.extent = recordInfo.SwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("[Command Buffers] Failed to end command buffer");
    }

    void CommandBuffers::SubmitQueue(uint32_t frameIndex, const SyncObject& syncObject) const
    {
        if (frameIndex >= m_commandBuffers.size())
            throw std::runtime_error("[Vulkan] Incorrect frame index");

        VkSubmitInfo submitInfo { };
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore waitSemaphores[] = { syncObject.GetImageAvailableSemaphore() };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[frameIndex];
        
        VkSemaphore signalSemaphores[] = { syncObject.GetRenderFinishedSemaphore() };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, syncObject.GetInFlightFence()) != VK_SUCCESS)
            throw std::runtime_error("[Vulkan] Failed to submit draw command buffer");
    }
}
