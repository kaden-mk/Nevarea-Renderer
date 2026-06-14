#pragma once

#include "VulkanSwapchain.hpp"

namespace Nevarea::Renderer {
    struct PushConstants {
        uint64_t vertex_buffer_address;
        uint32_t image_index;
        uint32_t sampler_index;
        uint32_t texture_index;
    };

    struct PipelineContext {
        VkPipeline pipeline;
        VkPipelineLayout layout;
        VkPipelineBindPoint bind_point;
    };

    void vulkan_compute_pipeline_init(PipelineContext& pipeline, VkDevice device, VkDescriptorSetLayout descriptor_layout, const char* compute);
    void vulkan_pipeline_init(PipelineContext& pipeline, VkDevice device, VkFormat color_format, VkDescriptorSetLayout descriptor_layout, const char* vert, const char* frag);
    void vulkan_pipeline_destroy(PipelineContext& pipeline, VkDevice device, FrameContext& frame);
}
