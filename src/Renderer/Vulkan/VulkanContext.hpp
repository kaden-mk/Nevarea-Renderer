#pragma once

#include <vulkan/vulkan.h>

#include <Platform/WindowSystem.hpp>

#include "VulkanDevice.hpp"

namespace Nevarea::Renderer {
	// might seperate device stuff into a different struct?
	struct VulkanContext {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkDebugUtilsMessengerEXT debug_messenger;
		WindowSystemState window;
		NevareaDevice nevarea_device;
	};

	void vulkan_context_create_instance(VulkanContext& context);
	void vulkan_context_debug_messenger(VulkanContext& context);
	void vulkan_context_create_surface(VulkanContext& context);

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window);
	void vulkan_context_destroy(VulkanContext& context);
}