#include "renderer/vulkan.hpp"

#include "renderer/instance.hpp"
#include "renderer/physical_device.hpp"
#include "renderer/logical_device.hpp"
#include "renderer/swap_chain.hpp"
#include "renderer/shader.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/command_pool.hpp"
#include "renderer/command_buffers.hpp"
#include "renderer/semaphore.hpp"
#include "renderer/fence.hpp"

#include <iostream>

namespace Renderer
{
    Vulkan::Vulkan() { }

    Vulkan::~Vulkan()
    {
        Destroy();
    }

    void Vulkan::Init(GLFWwindow* window, const std::filesystem::path& shaderPath)
    {
        m_instance = std::make_unique<Instance>();
        m_instance->Create(window);

        m_physicalDevice = std::make_unique<PhysicalDevice>();
        m_physicalDevice->Create(m_instance->Get());

        m_logicalDevice = std::make_unique<LogicalDevice>();

        QueueFamilyIndices indices = m_physicalDevice->GetQueueIndex(*m_instance->GetSurface());

        LogicalDeviceBuilder logicalDeviceBuilder
        {
            .PhysicalDevice = m_physicalDevice->Get(),
            .PresentIndex = indices.PresentIndex,
            .GraphicsIndex = indices.GraphicsIndex,
            .DeviceExtensions = m_physicalDevice->GetDeviceExtensions()
        };

        m_logicalDevice->Create(logicalDeviceBuilder);

        const vk::raii::Device& logicalDevice = m_logicalDevice->Get();

        m_swapChain = std::make_unique<SwapChain>();

        SwapChainBuilder swapChainBuilder
        {
            .PhysicalDevice = m_physicalDevice->Get(),
            .LogicalDevice = logicalDevice,
            .Surface = *m_instance->GetSurface(),
            .Window = window,
            .GraphicsFamilyIndex = indices.GraphicsIndex,
            .PresentFamilyIndex = indices.PresentIndex,
        };

        m_swapChain->Create(swapChainBuilder);

        ShaderBuilder vertexBuilder
        {
            .Device = logicalDevice,
            .FilePath = shaderPath / "triangle_vert.spv",
            .Type = vk::ShaderStageFlagBits::eVertex
        };

        Shader vertexShader { };
        vertexShader.Create(vertexBuilder);

        ShaderBuilder fragmentBuilder
        {
            .Device = logicalDevice,
            .FilePath = shaderPath / "triangle_frag.spv",
            .Type = vk::ShaderStageFlagBits::eFragment
        };

        Shader fragmentShader { };
        fragmentShader.Create(fragmentBuilder);

        std::vector<vk::PipelineShaderStageCreateInfo> stages
        {
            vertexShader.GetPipelineStageCreateInfo(),
            fragmentShader.GetPipelineStageCreateInfo()
        };

        PipelineBuilder pipelineBuilder
        {
            .Device = logicalDevice,
            .Extent = m_swapChain->GetExtent(),
            .SurfaceFormat = m_swapChain->GetSurfaceFormat(),
            .ShaderStages = std::move(stages)
        };

        m_pipeline = std::make_unique<Pipeline>();
        m_pipeline->Create(pipelineBuilder);

        CommandPoolBuilder commandPoolBuilder
        {
            .Device = logicalDevice,
            .QueueFamilyIndex = indices.GraphicsIndex
        };

        m_commandPool = std::make_unique<CommandPool>();
        m_commandPool->Create(commandPoolBuilder);

        CommandBuffersBuilder commandBufferBuilder
        {
            .Device = logicalDevice,
            .CommandPool = m_commandPool->Get(),
            .BuffersCount = MAX_FRAMES_IN_FLIGHT
        };

        m_commandBuffers = std::make_unique<CommandBuffers>();
        m_commandBuffers->Create(commandBufferBuilder);

        m_renderFinishedSemaphores.clear();

        for (size_t i = 0; i < m_swapChain->GetImagesCount(); ++i)
		{
            std::unique_ptr<Semaphore> renderer = std::make_unique<Semaphore>();
            renderer->Create(logicalDevice);
            m_renderFinishedSemaphores.emplace_back(std::move(renderer));
		}

        m_presentCompleteSemaphores.clear();
        m_inFlightFences.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
            std::unique_ptr<Semaphore> present = std::make_unique<Semaphore>();
            present->Create(logicalDevice);
            m_presentCompleteSemaphores.emplace_back(std::move(present));

            std::unique_ptr<Fence> fence = std::make_unique<Fence>();
            fence->Create(logicalDevice);
            m_inFlightFences.emplace_back(std::move(fence));
		}
    }

    void Vulkan::Destroy()
    {
        if (m_logicalDevice)
            m_logicalDevice->WaitIdle();

        m_presentCompleteSemaphores.clear();
        m_renderFinishedSemaphores.clear();
        m_inFlightFences.clear();

        if (m_commandBuffers)
            m_commandBuffers = nullptr;

        if (m_commandPool)
            m_commandPool = nullptr;

        if (m_pipeline)
            m_pipeline = nullptr;

        if (m_swapChain)
            m_swapChain = nullptr;

        if (m_logicalDevice)
            m_logicalDevice = nullptr;

        if (m_physicalDevice)
            m_physicalDevice = nullptr;

        if (m_instance)
            m_instance = nullptr;
    }

    void Vulkan::Draw(GLFWwindow* window)
    {
        const vk::raii::Fence& fence = m_inFlightFences[m_frameIndex]->Get();
        const vk::raii::Semaphore& presentCompleteSemaphores = m_presentCompleteSemaphores[m_frameIndex]->Get();

        vk::Result result = m_logicalDevice->WaitForFence(fence);
        if (result != vk::Result::eSuccess)
            throw std::runtime_error("[Vulkan][DrawFrame] Failed to wait for fence");

        auto [nextImageResult, imageIndex] = m_swapChain->AcquireNextImage(presentCompleteSemaphores);

        if (nextImageResult == vk::Result::eErrorOutOfDateKHR)
        {
            recreateSwapChain(window);
            return;
        }
        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
            throw std::runtime_error("[SwapChain][Draw] Failed to acquire swap chain image");
        }

        const vk::raii::Semaphore& renderFinishedSemaphore = m_renderFinishedSemaphores[imageIndex]->Get();

        m_logicalDevice->ResetFence(fence);
        m_commandBuffers->Reset(m_frameIndex);

        RecordCommandBufferBuilder builder
        {
            .Image = m_swapChain->GetImage(imageIndex),
            .ImageView = m_swapChain->GetImageView(imageIndex),
            .SwapChainExtent = m_swapChain->GetExtent(),
            .GraphicsPipeline = m_pipeline->Get(),
            .FrameIndex = m_frameIndex
        };

        m_commandBuffers->RecordCommandBuffer(builder);

        QueueSubmitBuilder queueSubmitBuilder
        {
            .CommandBuffer = m_commandBuffers->Get(m_frameIndex),
            .PresentCompleteSemaphore = presentCompleteSemaphores,
            .RenderFinishedSemaphore = renderFinishedSemaphore,
            .DrawFence = fence
        };
        m_logicalDevice->Submit(queueSubmitBuilder);

        PresentKHRBuider presentKHRBuilder
        {
            .RenderFinishedSemaphore = renderFinishedSemaphore,
            .SwapChain = m_swapChain->Get(),
            .ImageIndex = imageIndex
        };

        try
        {
            result = m_logicalDevice->PresentKHR(presentKHRBuilder);
        }
        catch (std::exception& ex)
        {
            std::cerr << "[Vulkan][DrawFrame] Exception: " << ex.what() << "\n";
        }

        if ((result == vk::Result::eSuboptimalKHR) || (result == vk::Result::eErrorOutOfDateKHR) || m_framebufferResized)
        {
            m_framebufferResized = false;
            recreateSwapChain(window);
        }
        else
            // There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
            assert(result == vk::Result::eSuccess);

        m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Vulkan::ResizeFramebuffer()
    {
        m_framebufferResized = true;
    }

    void Vulkan::recreateSwapChain(GLFWwindow* window)
    {
        QueueFamilyIndices indices = m_physicalDevice->GetQueueIndex(*m_instance->GetSurface());

        SwapChainBuilder swapChainBuilder
        {
            .PhysicalDevice = m_physicalDevice->Get(),
            .LogicalDevice = m_logicalDevice->Get(),
            .Surface = *m_instance->GetSurface(),
            .Window = window,
            .GraphicsFamilyIndex = indices.GraphicsIndex,
            .PresentFamilyIndex = indices.PresentIndex,
        };
        m_swapChain->Recreate(swapChainBuilder);
    }
}
