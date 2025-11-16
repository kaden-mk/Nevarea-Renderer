#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Nevarea {
	struct SwapchainContext {
        VkSwapchainKHR swapchain;
        VkFormat image_format;
        VkExtent2D extent;

        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        std::vector<VkFramebuffer> framebuffers;

        VkRenderPass render_pass;

        VkSemaphore image_available_semaphore;
        VkSemaphore render_finished_semaphore;
        VkFence in_flight_fence;
	};

	void vulkan_swapchain_init(SwapchainContext& swapchain, VkPhysicalDevice physical_device, VkSurfaceKHR surface);
}