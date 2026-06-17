#pragma once

#include "lib/Rendering.hpp"
#include <vulkan/vulkan.h>

namespace Nevarea {
    struct VulkanHandles {
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice device;
        VkQueue graphics_queue;
        uint32_t graphics_family;
    };

    struct VulkanImage {
        VkImage image;
        VkImageView image_view;
        VkFormat format;
        VkExtent2D extent;
        uint32_t mips, layers;
        VkImageLayout current_layout;
        VkImageUsageFlags usage;
    };

    struct VulkanBuffer {
        VkBuffer buffer;
        VkDeviceSize size;
        VkDeviceAddress address;
    };

    NEVAREA_API VulkanHandles renderer_vk_context(RenderContext context);
    NEVAREA_API VulkanImage renderer_vk_image(RenderContext context, Image image);
    NEVAREA_API VulkanBuffer renderer_vk_buffer(RenderContext context, Buffer buffer);

    NEVAREA_API VkImageLayout renderer_vk_image_layout(RenderContext context, Image image);
    NEVAREA_API void renderer_vk_set_image_layout(RenderContext context, Image image, VkImageLayout image_layout);

    NEVAREA_API void renderer_vk_immediate_submit(RenderContext context, void (*record)(VkCommandBuffer cmd, void* user), void* user);
    NEVAREA_API VkSemaphore renderer_vk_timeline(RenderContext context, uint64_t* out_last_value);

    NEVAREA_API void renderer_vk_request_features(RenderContext context, const void* feature_chain);
}
