#pragma once
#ifndef RENDERER_SHADER_HPP_
#define RENDERER_SHADER_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <filesystem>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct ShaderBuilder
    {
        const vk::raii::Device& Device;
        std::filesystem::path FilePath;
        vk::ShaderStageFlagBits Type;
    };

    class ShaderConstructor
    {
    public:
        [[nodiscard]] vk::raii::ShaderModule CreateShaderModule(const vk::raii::Device& device,
                                                                const std::filesystem::path& shaderFile);        
    
    private:
        std::vector<char> readFile(const std::filesystem::path& shaderFile);
    };

    class Shader : public Interfaces::NonCopyable
    {
    public:
        Shader()  = default;
        ~Shader() = default;

        void Create(const ShaderBuilder& builder);

        vk::PipelineShaderStageCreateInfo GetPipelineStageCreateInfo() const;

    private:
        vk::raii::ShaderModule m_shaderModule { nullptr };
        vk::PipelineShaderStageCreateInfo m_stageInfo { };
        vk::ShaderStageFlagBits m_type = vk::ShaderStageFlagBits::eVertex;
    };
}

#endif // RENDERER_SHADER_HPP_