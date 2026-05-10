#pragma once

#include "lib/WindowSystem.hpp"
#include "Core/n_pch.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

#include <vk_mem_alloc.h>

namespace Nevarea::Renderer {
	struct VulkanContext {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		VmaAllocator allocator;

		WindowHandle window;
		DeviceContext device;
		SurfaceContext surface;
		SwapchainContext swapchain;
		FrameContext frame_sync;
	};

	void vulkan_context_init(VulkanContext& context, WindowHandle window);
	void vulkan_context_draw(VulkanContext& context);
	void vulkan_context_destroy(VulkanContext& context);
}