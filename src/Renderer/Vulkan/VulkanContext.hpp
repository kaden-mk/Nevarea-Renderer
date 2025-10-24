#pragma once

#include <vulkan/vulkan.h>

#include <Platform/WindowSystem.hpp>

namespace Nevarea {
	struct VulkanContext {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkDebugUtilsMessengerEXT debug_messenger;
		WindowSystemState window;
	};

	void vulkan_context_create_instance(VulkanContext& context);
	void vulkan_context_debug_messenger(VulkanContext& context);
	void vulkan_context_create_surface(VulkanContext& context);

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window);
	void vulkan_context_destroy(VulkanContext& context);

	void helper_populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_create_info);
}