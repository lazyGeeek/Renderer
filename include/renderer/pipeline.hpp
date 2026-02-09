#pragma once
#ifndef RENDERER_PIPELINE_HPP_
#define RENDERER_PIPELINE_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace Renderer
{
    struct PipelineBuilder
    {
        const vk::raii::Device& Device { nullptr };
        const vk::Extent2D& Extent;
        const vk::SurfaceFormatKHR& SurfaceFormat;
        std::vector<vk::PipelineShaderStageCreateInfo> ShaderStages;
    };

    class Pipeline
    {
    public:
        Pipeline()  = default;
        ~Pipeline() = default;

        Pipeline(const Pipeline& other)             = delete;
        Pipeline(Pipeline&& other)                  = delete;
        Pipeline& operator=(const Pipeline& other)  = delete;
        Pipeline& operator=(const Pipeline&& other) = delete;

        void Create(const PipelineBuilder& builder);

    private:
        vk::raii::PipelineLayout m_layout { nullptr };
        vk::raii::Pipeline m_graphicsPipeline { nullptr };
    };
}

#endif // RENDERER_PIPELINE_HPP_