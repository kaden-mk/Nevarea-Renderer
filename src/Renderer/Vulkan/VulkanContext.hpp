#pragma once

#include <vulkan/vulkan.h>

#include <Platform/WindowSystem.hpp>

namespace Nevarea {
	// might seperate device stuff into a different struct?
	struct VulkanContext {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkDebugUtilsMessengerEXT debug_messenger;
		WindowSystemState window;
		VkPhysicalDevice physical_device;
		VkDevice device;
		uint32_t graphics_queue_family;
		VkQueue graphics_queue;
		VkQueue present_queue;
	};

	void vulkan_context_create_instance(VulkanContext& context);
	void vulkan_context_debug_messenger(VulkanContext& context);
	void vulkan_context_create_surface(VulkanContext& context);
	void vulkan_context_pick_physical_device(VulkanContext& context);

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window);
	void vulkan_context_destroy(VulkanContext& context);

	void helper_populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_create_info);
}