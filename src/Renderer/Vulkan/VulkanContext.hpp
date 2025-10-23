#pragma once

#include <vulkan/vulkan.h>

#include <Platform/WindowSystem.hpp>

namespace Nevarea {
	struct VulkanContext {
		VkInstance instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
	};

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window);
	void vulkan_context_debug_messenger(VulkanContext& context);
}