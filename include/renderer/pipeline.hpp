#pragma once
#ifndef RENDERER_PIPELINE_HPP_
#define RENDERER_PIPELINE_HPP_

#include <vulkan/vulkan_raii.hpp>

#include <vector>

#include "renderer/interfaces/non_copyable.hpp"

namespace Renderer
{
    struct PipelineBuilder
    {
        const vk::raii::Device& Device { nullptr };
        const vk::Extent2D& Extent;
        const vk::SurfaceFormatKHR& SurfaceFormat;
        std::vector<vk::PipelineShaderStageCreateInfo> ShaderStages;
    };

    class Pipeline : public Interfaces::NonCopyable
    {
    public:
        Pipeline()  = default;
        ~Pipeline() = default;

        void Create(const PipelineBuilder& builder);

        const vk::raii::Pipeline& Get() const;

    private:
        vk::raii::PipelineLayout m_layout { nullptr };
        vk::raii::Pipeline m_graphicsPipeline { nullptr };
    };
}

#endif // RENDERER_PIPELINE_HPP_