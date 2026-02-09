#include "renderer/shader.hpp"

#include <format>
#include <fstream>
#include <vector>

namespace Renderer
{
    std::vector<char> ShaderConstructor::readFile(const std::filesystem::path& shaderFile)
    {
        if (!std::filesystem::exists(shaderFile))
            throw std::runtime_error(std::format("[Vulkan][Shader Constructor] {} file doesn't exist", shaderFile.string()));

        std::ifstream shader(shaderFile.string(), std::ios::ate | std::ios::binary);

        if (!shader.is_open())
            throw std::runtime_error(std::format("[Vulkan][Shader Constructor] Can't open {}", shaderFile.string()));

        size_t shaderSize = static_cast<size_t>(shader.tellg());
        std::vector<char> buffer(shaderSize);

        shader.seekg(0, std::ios::beg);
        shader.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        shader.close();

        return buffer;
    }

    [[nodiscard]] vk::raii::ShaderModule ShaderConstructor::CreateShaderModule(const vk::raii::Device& device, const std::filesystem::path& shaderPath)
    {
        std::vector<char> shaderCode = std::move(readFile(shaderPath));

        vk::ShaderModuleCreateInfo createInfo { };
        createInfo.codeSize = shaderCode.size() * sizeof(char);
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        vk::raii::ShaderModule shaderModule(device, createInfo);
        return shaderModule;
    }

    void Shader::Create(const ShaderBuilder& builder)
    {
        ShaderConstructor constructor;
        m_shaderModule = constructor.CreateShaderModule(builder.Device, builder.FilePath);

        m_type = builder.Type;
        m_stageInfo.stage  = builder.Type;
        m_stageInfo.module = *m_shaderModule;
        m_stageInfo.pName  = "main";
    }

    vk::PipelineShaderStageCreateInfo Shader::GetPipelineStageCreateInfo() const
    {
        vk::PipelineShaderStageCreateInfo stageInfo { };
        stageInfo.stage = m_type;
        stageInfo.module = *m_shaderModule;
        stageInfo.pName = "main";

        return stageInfo;
    }
}