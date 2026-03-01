#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include <Platform/WindowSystem.hpp>

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"

namespace Nevarea::Renderer {
	struct VulkanContext {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;

		WindowSystemState window;
		DeviceContext device;
		SurfaceContext surface;
		SwapchainContext swapchain;
		FrameContext frame_sync;
	};

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window);
	void vulkan_context_draw(VulkanContext& context);
	void vulkan_context_destroy(VulkanContext& context);
}