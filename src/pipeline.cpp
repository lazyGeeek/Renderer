#include "renderer/pipeline.hpp"

namespace Renderer
{
    void Pipeline::Create(const PipelineBuilder& builder)
    {
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo { };

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly; { };
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

        const vk::Extent2D& extent = builder.Extent;

        vk::Viewport viewport
        {
            0.0f, 0.0f,
            static_cast<float>(extent.width),
            static_cast<float>(extent.height),
            0.0f, 1.0f
        };

        vk::Rect2D{ vk::Offset2D { 0, 0 }, extent };

        std::vector dynamicStates =
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState { };
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        vk::PipelineViewportStateCreateInfo viewportState { };
        viewportState.viewportCount = 1;
        viewportState.scissorCount  = 1;

        vk::PipelineRasterizationStateCreateInfo rasterizer { };
        rasterizer.depthClampEnable        = vk::False;
        rasterizer.rasterizerDiscardEnable = vk::False;
        rasterizer.polygonMode             = vk::PolygonMode::eFill;
        rasterizer.cullMode                = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace               = vk::FrontFace::eClockwise;
        rasterizer.depthBiasEnable         = vk::False;
        rasterizer.depthBiasSlopeFactor    = 1.0f;
        rasterizer.lineWidth               = 1.0f;

        vk::PipelineMultisampleStateCreateInfo multisampling { };
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampling.sampleShadingEnable  = vk::False;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment { };
        colorBlendAttachment.blendEnable    = vk::False;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
                                              vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB |
                                              vk::ColorComponentFlagBits::eA;

        vk::PipelineColorBlendStateCreateInfo colorBlending { };
        colorBlending.logicOpEnable   = vk::False;
        colorBlending.logicOp         = vk::LogicOp::eCopy;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments    = &colorBlendAttachment;

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo { };
        pipelineLayoutInfo.setLayoutCount         = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        m_layout = vk::raii::PipelineLayout(builder.Device, pipelineLayoutInfo);

        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo { };
        graphicsPipelineCreateInfo.stageCount          = 2,
        graphicsPipelineCreateInfo.pStages             = builder.ShaderStages.data(),
        graphicsPipelineCreateInfo.pVertexInputState   = &vertexInputInfo,
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly,
        graphicsPipelineCreateInfo.pViewportState      = &viewportState,
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizer,
        graphicsPipelineCreateInfo.pMultisampleState   = &multisampling,
        graphicsPipelineCreateInfo.pColorBlendState    = &colorBlending,
        graphicsPipelineCreateInfo.pDynamicState       = &dynamicState,
        graphicsPipelineCreateInfo.layout              = *m_layout,
        graphicsPipelineCreateInfo.renderPass          = nullptr;

        vk::PipelineRenderingCreateInfo renderingCreateInfo { };
        renderingCreateInfo.colorAttachmentCount = 1;
        renderingCreateInfo.pColorAttachmentFormats = &builder.SurfaceFormat.format;

        vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain =
        {
            graphicsPipelineCreateInfo,
            renderingCreateInfo
        };

        m_graphicsPipeline = vk::raii::Pipeline(builder.Device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
    }

    const vk::raii::Pipeline& Pipeline::Get() const
    {
        return m_graphicsPipeline;
    }

}
