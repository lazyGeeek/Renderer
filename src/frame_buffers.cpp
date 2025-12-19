#include "renderer/frame_buffers.hpp"

namespace Renderer
{
    FrameBuffers::FrameBuffers(const FrameBuffersInit& init) :
        m_device { init.Device },
        m_renderPass { init.RenderPass },
        m_swapChain { init.SwapChain } { }

    FrameBuffers::~FrameBuffers()
    {
        Destroy();
    }

    void FrameBuffers::Create()
    {
        const std::vector<VkImageView>& imageViews = m_swapChain.GetImageViews(); 
        m_framebuffers.resize(imageViews.size());

        for (size_t i = 0; i < m_framebuffers.size(); ++i)
        {
            VkImageView attachments[] =
            {
                imageViews[i]
            };

            const VkExtent2D& swapChainExtent = m_swapChain.GetExtent();

            VkFramebufferCreateInfo framebufferInfo { };
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass.Get();
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_device.GetLogicalDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("[SwapChain] Failed to create framebuffer");
        }
    }

    void FrameBuffers::Destroy()
    {
        for (auto framebuffer : m_framebuffers)
        {
            if (framebuffer != VK_NULL_HANDLE)
                vkDestroyFramebuffer(m_device.GetLogicalDevice(), framebuffer, nullptr);
        }

        m_framebuffers.clear();
    }

    void FrameBuffers::Recreate()
    {
        Destroy();
        Create();
    }

    const VkFramebuffer& FrameBuffers::Get(size_t imageIndex)
    {
        if (imageIndex >= m_framebuffers.size())
            throw std::runtime_error("[SwapChain] Image index buffer is not exist");

        return m_framebuffers[imageIndex];
    }
}
