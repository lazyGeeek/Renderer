#include "renderer/vulkan.hpp"

#include "renderer/instance.hpp"
#include "renderer/physical_device.hpp"
#include "renderer/logical_device.hpp"
#include "renderer/swap_chain.hpp"
#include "renderer/shader.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/command_pool.hpp"

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
    }

    void Vulkan::Destroy()
    {
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
}
