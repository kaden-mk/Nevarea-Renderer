#pragma once

#include <vulkan/vulkan.h>
#include <optional>

namespace Nevarea::Renderer {
	struct VulkanContext;
}

namespace Nevarea::Renderer {
	struct DeviceContext {
		VkPhysicalDevice physical_device;
		VkDevice device;
		VkQueue graphics_queue;
		uint32_t graphics_queue_family;
	};

	void vulkan_device_init(VulkanContext& context);
	void vulkan_device_destroy(DeviceContext* device_context);

	void vulkan_device_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, DeviceContext* device_context);
	void vulkan_device_create_logical_device(VkInstance instance, VkSurfaceKHR surface, DeviceContext* device_context);
}