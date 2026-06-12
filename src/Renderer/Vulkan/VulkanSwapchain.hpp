#pragma once

#include "VulkanDevice.hpp"
#include "lib/WindowSystem.hpp"

#include "Core/n_pch.hpp"
#include <lib/Rendering.hpp>

namespace Nevarea::Renderer {
	struct DeletionQueue {
		std::vector<std::function<void()>> deletors;
	};

	struct SwapchainContext {
        VkSwapchainKHR swapchain;
        VkFormat image_format;
        VkExtent2D extent;

        uint32_t current_image_index;

        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
	};

    struct FrameContext {
        std::vector<VkSemaphore> image_available;
        std::vector<VkSemaphore> render_finished;

        VkSemaphore timeline;
        uint64_t timeline_value;
        uint64_t frame_timeline_target[MAX_FRAMES_IN_FLIGHT];

        uint32_t current_frame;

        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;

        DeletionQueue deletion_queues[MAX_FRAMES_IN_FLIGHT];
    };

    struct SurfaceContext {
        VkSurfaceKHR surface;
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> supported_formats;
        std::vector<VkPresentModeKHR> supported_present_modes;
    };

    NEVAREA_FORCE_INLINE bool has_available_swapchain_support(const SurfaceContext& surface) {
        return !surface.supported_formats.empty() && !surface.supported_present_modes.empty();
    }

    void query_swapchain_support(VkPhysicalDevice physical_device, SurfaceContext& surface);

    void vulkan_swapchain_init(SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window, VkSwapchainKHR old_swapchain = VK_NULL_HANDLE);
    void recreate_swapchain(SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window);
    void vulkan_swapchain_destroy(SwapchainContext& swapchain, VkDevice device);

    void vulkan_frame_sync_init(FrameContext& frame_sync, DeviceContext& device_context, uint32_t swapchain_image_count);
    void vulkan_frame_sync_destroy(FrameContext& frame_sync, VkDevice device);

    void vulkan_frame_sync_ensure_present_semaphores(FrameContext& frame_sync, VkDevice device, uint32_t swapchain_image_count);
}