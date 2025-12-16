#pragma once
#ifndef RENDERER_SHADER_HPP_
#define RENDERER_SHADER_HPP_

#include <vulkan/vulkan.hpp>

#include <filesystem>

namespace Renderer
{
    enum class EShaderType
    {
        Vertex = VK_SHADER_STAGE_VERTEX_BIT,
        Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        Geometry = VK_SHADER_STAGE_GEOMETRY_BIT
    };

    struct ShaderInfo
    {
        std::filesystem::path FilePath;
        EShaderType Type;
    };

    class ShaderConstructor
    {
    public:
        VkShaderModule CreateShaderModule(VkDevice& device, const std::filesystem::path& shaderFile);        
    
    private:
        std::vector<char> readFile(const std::filesystem::path& shaderFile);
    };

    class Shader
    {
    public:
        Shader(VkDevice& device, const ShaderInfo& shaderInfo);
        ~Shader();

        Shader(const Shader& other)             = delete;
        Shader(Shader&& other)                  = delete;
        Shader& operator=(const Shader& other)  = delete;
        Shader& operator=(const Shader&& other) = delete;

        const VkShaderModule& GetModule() const;
        VkPipelineShaderStageCreateInfo GenerateStageInfo() const;

    private:
        VkDevice& m_device;
        VkShaderModule m_module = VK_NULL_HANDLE;
        ShaderInfo m_shaderInfo;
    };
}

#endif // RENDERER_SHADER_HPP_