#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Nevarea::Renderer {
    struct VulkanContext;
}

namespace Nevarea::Renderer {
    static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

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
        uint32_t max_frames_in_flight;
    };

    struct SurfaceContext {
        VkSurfaceKHR surface;
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> supported_formats;
        std::vector<VkPresentModeKHR> supported_present_modes;
    };

    void query_swapchain_support(VkPhysicalDevice physical_device, SurfaceContext& surface);

    void vulkan_swapchain_init(VulkanContext& context);
    void vulkan_swapchain_destroy(SwapchainContext swapchain, VkDevice device);

    void vulkan_frame_sync_init(FrameContext& frame_sync, VkDevice device, uint32_t max_frames_in_flight);
    void vulkan_frame_sync_destroy(FrameContext& frame_sync, VkDevice device);
}