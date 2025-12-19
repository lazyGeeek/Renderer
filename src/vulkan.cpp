#include "renderer/vulkan.hpp"
#include "renderer/command_buffers.hpp"
#include "renderer/command_pool.hpp"
#include "renderer/device.hpp"
#include "renderer/frame_buffers.hpp"
#include "renderer/instance.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/render_pass.hpp"
#include "renderer/shader.hpp"
#include "renderer/sync_object.hpp"
#include "renderer/swap_chain.hpp"

namespace Renderer
{
    Vulkan::Vulkan(GLFWwindow* window) : m_window { window } { }

    void Vulkan::Init(const std::filesystem::path& shaderPath)
    {
        m_instance = std::make_unique<Instance>(m_window, ENABLE_VALIDATION_LAYERS);
        m_instance->Create();

        m_device = std::make_unique<Device>(*m_instance.get());
        m_device->Create();

        m_swapChain =std::make_unique<SwapChain>(m_window, *m_device.get(), *m_instance.get());
        m_swapChain->Create();

        m_renderPass = std::make_unique<RenderPass>(*m_device.get(), *m_swapChain.get());
        m_renderPass->Create();

        FrameBuffersInit init
        {
            .Device = *m_device.get(),
            .RenderPass = *m_renderPass.get(),
            .SwapChain = *m_swapChain.get()
        };

        m_frameBuffers = std::make_unique<FrameBuffers>(init);
        m_frameBuffers->Create();

        ShaderInfo shaderInfo
        {
            .FilePath = shaderPath / "triangle_vert.spv",
            .Type = EShaderType::Vertex
        };

        Shader vertexShader(*m_device.get(), shaderInfo);

        shaderInfo.FilePath = shaderPath / "triangle_frag.spv";
        shaderInfo.Type = EShaderType::Fragment;

        Shader fragmentShader(*m_device.get(), shaderInfo);

        m_pipeline = std::make_unique<Pipeline>(*m_device.get(), *m_renderPass.get());
        m_pipeline->Create(vertexShader, fragmentShader);

        m_commandPool = std::make_unique<CommandPool>(*m_device.get());
        m_commandPool->Create();

        m_commandBuffers = std::make_unique<CommandBuffers>(*m_device.get());
        m_commandBuffers->Create(m_commandPool->Get(), MAX_FRAMES_IN_FLIGHT);

        m_syncObjects.clear();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            std::unique_ptr<SyncObject> syncObject = std::make_unique<SyncObject>(*m_device.get());
            syncObject->Create();
            m_syncObjects.emplace_back(std::move(syncObject));
        }
    }

    Vulkan::~Vulkan()
    {
        if (!m_device)
            return;

        m_device->WaitIdle();
        const VkDevice& device = m_device->GetLogicalDevice(); 

        if (m_frameBuffers)
        {
            m_frameBuffers->Destroy();
            m_frameBuffers = nullptr;
        }

        if (m_swapChain)
        {
            m_swapChain->Clear();
            m_swapChain = nullptr;
        }

        if (m_pipeline)
        {
            m_pipeline->Destroy();
            m_pipeline = nullptr;
        }

        if (m_renderPass)
        {
            m_renderPass->Destroy();
            m_renderPass = nullptr;
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            if (m_syncObjects[i])
            {
                m_syncObjects[i]->Destroy();
                m_syncObjects[i] = nullptr;
            }
        }

        if (m_commandPool)
        {
            m_commandPool->Destroy();
            m_commandPool = nullptr;
        }

        if (m_device)
        {
            m_device->Destroy();
            m_device = nullptr;
        }

        if (m_instance)
        {
            m_instance->Destroy();
            m_instance = nullptr;
        }
    }

    void Vulkan::DrawFrame()
    {
        m_syncObjects[m_currentFrame]->WaitForFence();
        
        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(m_device->GetLogicalDevice(),
                                                m_swapChain->GetSwapChainKHR(),
                                                UINT64_MAX, m_syncObjects[m_currentFrame]->GetImageAvailableSemaphore(),
                                                VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_swapChain->Recreate();
            m_frameBuffers->Recreate();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("[Vulkan] Failed to acquire swap chain image");
        }
        
        m_syncObjects[m_currentFrame]->ResetFence();
        
        m_commandBuffers->Reset(m_currentFrame);

        CommandBufferRecordInfo recordInfo
        {
            .FrameBuffer = m_frameBuffers->Get(imageIndex),
            .GraphicsPipeline = m_pipeline->GetGraphicsPipeline(),
            .RenderPass = m_renderPass->Get(),
            .SwapChainExtent = m_swapChain->GetExtent()
        };

        m_commandBuffers->Record(m_currentFrame, recordInfo);
        m_commandBuffers->SubmitQueue(m_currentFrame, *m_syncObjects[m_currentFrame].get());
        result = m_swapChain->PresentKHR(imageIndex, *m_syncObjects[m_currentFrame].get());

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
        {
            m_framebufferResized = false;
            m_swapChain->Recreate();
            m_frameBuffers->Recreate();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("[Vulkan] Failed to present swap chain image");
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Vulkan::SetFramebufferResized()
    {
        m_framebufferResized = true;
    }

    bool Vulkan::IsValidationLayerEnabled() const
    {
        return ENABLE_VALIDATION_LAYERS;
    }
}
