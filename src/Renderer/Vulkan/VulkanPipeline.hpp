#pragma once

namespace Nevarea::Renderer {
    struct PushConstants {
        uint64_t vertex_buffer_address;
    };

    struct PipelineContext {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };

    void vulkan_pipeline_init(PipelineContext& pipeline, VkDevice device, VkFormat color_format, VkDescriptorSetLayout descriptor_layout);
    void vulkan_pipeline_destroy(PipelineContext& pipeline, VkDevice device);
}