#include "renderer/vulkan.hpp"

#include "renderer/instance.hpp"
#include "renderer/physical_device.hpp"
#include "renderer/logical_device.hpp"
#include "renderer/swap_chain.hpp"
#include "renderer/shader.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/command_pool.hpp"
#include "renderer/command_buffer.hpp"
#include "renderer/sync_object.hpp"

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

        m_swapChain = std::make_unique<SwapChain>();

        SwapChainBuilder swapChainBuilder
        {
            .PhysicalDevice = m_physicalDevice->Get(),
            .LogicalDevice = m_logicalDevice->Get(),
            .Surface = *m_instance->GetSurface(),
            .Window = window,
            .GraphicsFamilyIndex = indices.GraphicsIndex,
            .PresentFamilyIndex = indices.PresentIndex,
        };

        m_swapChain->Create(swapChainBuilder);

        ShaderBuilder vertexBuilder
        {
            .Device = m_logicalDevice->Get(),
            .FilePath = shaderPath / "triangle_vert.spv",
            .Type = vk::ShaderStageFlagBits::eVertex
        };

        Shader vertexShader { };
        vertexShader.Create(vertexBuilder);

        ShaderBuilder fragmentBuilder
        {
            .Device = m_logicalDevice->Get(),
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
            .Device = m_logicalDevice->Get(),
            .Extent = m_swapChain->GetExtent(),
            .SurfaceFormat = m_swapChain->GetSurfaceFormat(),
            .ShaderStages = std::move(stages)
        };

        m_pipeline = std::make_unique<Pipeline>();
        m_pipeline->Create(pipelineBuilder);

        CommandPoolBuilder commandPoolBuilder
        {
            .Device = m_logicalDevice->Get(),
            .QueueFamilyIndex = indices.GraphicsIndex
        };

        m_commandPool = std::make_unique<CommandPool>();
        m_commandPool->Create(commandPoolBuilder);

        CommandBufferBuilder commandBufferBuilder
        {
            .Device = m_logicalDevice->Get(),
            .CommandPool = m_commandPool->Get()
        };

        m_commandBuffer = std::make_unique<CommandBuffer>();
        m_commandBuffer->Create(commandBufferBuilder);

        SyncObjectBuilder syncObjectBuilder
        {
            .Device = m_logicalDevice->Get()
        };

        m_syncObject = std::make_unique<SyncObject>();
        m_syncObject->Create(syncObjectBuilder);
    }

    void Vulkan::Destroy()
    {
        if (m_logicalDevice)
            m_logicalDevice->WaitIdle();

        if (m_syncObject)
            m_syncObject = nullptr;

        if (m_commandBuffer)
            m_commandBuffer = nullptr;

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

    void Vulkan::Draw()
    {
        m_logicalDevice->WaitIdle();

        auto [result, imageIndex] = m_swapChain->AcquireNextImage(m_syncObject->GetPresentCompleteSemaphore());

        RecordCommandBufferBuilder builder
        {
            .Image = m_swapChain->GetImage(imageIndex),
            .ImageView = m_swapChain->GetImageView(imageIndex),
            .SwapChainExtent = m_swapChain->GetExtent(),
            .GraphicsPipeline = m_pipeline->Get()
        };

        m_commandBuffer->RecordCommandBuffer(builder);

        m_logicalDevice->ResetFence(m_syncObject->GetDrawFence());

        QueueSubmitBuilder queueSubmitBuilder
        {
            .CommandBuffer = m_commandBuffer->Get(),
            .PresentCompleteSemaphore = m_syncObject->GetPresentCompleteSemaphore(),
            .RenderFinishedSemaphore = m_syncObject->GetRenderFinishedSemaphore(),
            .DrawFence = m_syncObject->GetDrawFence()
        };
        m_logicalDevice->Submit(queueSubmitBuilder);
        
        result = m_logicalDevice->WaitForFence(m_syncObject->GetDrawFence());

        if (result != vk::Result::eSuccess)
            throw std::runtime_error("[Vulkan][Draw] Failed to wait for fence");

        PresentKHRBuider presentKHRBuilder
        {
            .RenderFinishedSemaphore = m_syncObject->GetRenderFinishedSemaphore(),
            .SwapChain = m_swapChain->Get(),
            .ImageIndex = imageIndex
        };

        result = m_logicalDevice->PresentKHR(presentKHRBuilder);
		
        switch (result)
		{
			case vk::Result::eSuccess:
				break;
			case vk::Result::eSuboptimalKHR:
				std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR\n";
				break;
			default:
				break;        // an unexpected result is returned!
		}
    }
}
