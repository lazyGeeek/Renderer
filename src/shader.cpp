#include "renderer/shader.hpp"

#include <format>
#include <fstream>
#include <vector>

namespace Renderer
{
    std::vector<char> ShaderConstructor::readFile(const std::filesystem::path& shaderFile)
    {
        if (!std::filesystem::exists(shaderFile))
            throw std::runtime_error(std::format("[ERROR] {} file doesn't exist", shaderFile.string()));

        std::ifstream shader(shaderFile.string(), std::ios::ate | std::ios::binary);

        if (!shader.is_open())
            throw std::runtime_error(std::format("[ERROR] Can't open {}", shaderFile.string()));

        size_t shaderSize = static_cast<size_t>(shader.tellg());
        std::vector<char> buffer(shaderSize);

        shader.seekg(0);
        shader.read(buffer.data(), shaderSize);
        shader.close();
        return buffer;
    }

    VkShaderModule ShaderConstructor::CreateShaderModule(const VkDevice& device, const std::filesystem::path& shaderPath)
    {
        std::vector<char> shaderCode = std::move(readFile(shaderPath));

        VkShaderModuleCreateInfo createInfo { };
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
                throw std::runtime_error("VULKAN: Failed to create shader module");

        return shaderModule;
    }

    Shader::Shader(const Device& device, const ShaderInfo& shaderInfo) : 
        m_device { device },
        m_shaderInfo { shaderInfo }
    {
        ShaderConstructor constructor;
        m_module = constructor.CreateShaderModule(m_device.GetLogicalDevice(), shaderInfo.FilePath);
    }

    Shader::~Shader()
    {
        if (m_module != VK_NULL_HANDLE)
            vkDestroyShaderModule(m_device.GetLogicalDevice(), m_module, nullptr);
    }

    const VkShaderModule& Shader::GetModule() const
    {
        return m_module;
    }

    VkPipelineShaderStageCreateInfo Shader::GenerateStageInfo() const
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo { };
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(m_shaderInfo.Type);
        shaderStageInfo.module = m_module;
        shaderStageInfo.pName = "main";

        return shaderStageInfo;
    }
}