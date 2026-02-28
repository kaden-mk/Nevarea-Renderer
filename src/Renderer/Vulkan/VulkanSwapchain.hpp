#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Nevarea::Renderer {
    struct VulkanContext;
}

namespace Nevarea::Renderer {
	struct SwapchainContext {
        VkSwapchainKHR swapchain;
        VkFormat image_format;
        VkExtent2D extent;

        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
	};

    struct FrameContext {
        std::vector<VkSemaphore> image_available;
        std::vector<VkSemaphore> render_finished;
        std::vector<VkFence> in_flight;

        uint32_t current_frame;

        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;
    };

    struct SurfaceContext {
        VkSurfaceKHR surface;
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> supported_formats;
        std::vector<VkPresentModeKHR> supported_present_modes;
    };

    void query_swapchain_support(VkPhysicalDevice physical_device, SurfaceContext& surface);

    void vulkan_swapchain_init(VulkanContext& context, VkSwapchainKHR old_swapchain = VK_NULL_HANDLE);
    void vulkan_swapchain_destroy(SwapchainContext swapchain, VkDevice device);

    void vulkan_frame_sync_init(VulkanContext& context);
    void vulkan_frame_sync_destroy(FrameContext& frame_sync, VkDevice device);

    void draw_frame(VulkanContext& context);
}