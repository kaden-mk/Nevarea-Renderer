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
        std::vector<VkFramebuffer> framebuffers;

        VkRenderPass render_pass;
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
}