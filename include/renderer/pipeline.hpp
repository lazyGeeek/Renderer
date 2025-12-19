#pragma once
#ifndef RENDERER_PIPELINE_HPP_
#define RENDERER_PIPELINE_HPP_

#include "renderer/device.hpp"
#include "renderer/render_pass.hpp"
#include "renderer/shader.hpp"

#include <vulkan/vulkan.h>

#include <vector>

namespace Renderer
{
    class Pipeline
    {
    public:
        Pipeline(const Device& device, const RenderPass& renderPass);
        ~Pipeline();

        Pipeline(const Pipeline& other)             = delete;
        Pipeline(Pipeline&& other)                  = delete;
        Pipeline& operator=(const Pipeline& other)  = delete;
        Pipeline& operator=(const Pipeline&& other) = delete;

        void Create(const Shader& vertex, const Shader& fragment);
        void Destroy();

        const VkPipeline& GetGraphicsPipeline() const;

    private:
        const Device& m_device;
        const RenderPass& m_renderPass;

        VkPipeline m_graphicsPipeline     = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    };
}

#endif // RENDERER_PIPELINE_HPP_