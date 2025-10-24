#pragma once

#include <vulkan/vulkan.h>
#include <optional>

namespace Nevarea {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool is_complete() const {
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);
	bool check_device_extension_support(VkPhysicalDevice device);
	bool check_device_compatibility(VkPhysicalDevice device, VkSurfaceKHR surface);

	void vulkan_device_pick_physical_device(VkInstance instance, VkPhysicalDevice* physical_device, VkSurfaceKHR surface);
}