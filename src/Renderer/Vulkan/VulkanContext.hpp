#pragma once

#include "Platform/WindowSystemInternal.hpp"
#include "Core/n_pch.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanResourceManager.hpp"
#include "VulkanPipeline.hpp"

#include <vk_mem_alloc.h>

namespace Nevarea::Renderer {
	struct VulkanContext {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		VmaAllocator allocator;

		ResourceManager resource_manager;
		WindowHandle window;
		DeviceContext device;
		SurfaceContext surface;
		SwapchainContext swapchain;
		FrameContext frame_sync;
		PipelineContext pipeline;

		std::vector<Mesh> draw_list;
	};

	void vulkan_context_init(VulkanContext& context, WindowHandle window);
	void vulkan_context_draw(VulkanContext& context);
	void vulkan_context_destroy(VulkanContext& context);
}