#pragma once

namespace Nevarea::Renderer {
    struct PipelineContext {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };

    void vulkan_pipeline_init(PipelineContext& pipeline, VkDevice device, VkFormat color_format);
    void vulkan_pipeline_destroy(PipelineContext& pipeline, VkDevice device);
}