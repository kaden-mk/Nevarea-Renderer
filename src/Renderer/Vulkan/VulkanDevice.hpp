#pragma once

#include <vulkan/vulkan.h>
#include <optional>

namespace Nevarea::Renderer {
	struct QueueFamilyInfo {
		int index;
		VkDeviceQueueCreateInfo create_info;
	};

	struct NevareaDevice {
		VkPhysicalDevice physical_device;
		VkDevice device;
		VkQueue queue;
	};

	void vulkan_device_init(NevareaDevice* nevarea_device, VkInstance instance, VkSurfaceKHR surface);
	void vulkan_device_destroy(NevareaDevice* nevarea_device);

	void vulkan_device_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, NevareaDevice* nevarea_device);
	void vulkan_device_create_logical_device(VkInstance instance, VkSurfaceKHR surface, NevareaDevice* nevarea_device);
}