#pragma once

#include <vulkan/vulkan.h>

#include "Core/n_pch.hpp"
#include "Core/InternalState.hpp"

namespace Nevarea::Renderer {
	struct DeviceContext {
		VkPhysicalDevice physical_device;
		VkDevice device;

		VkQueue graphics_queue;
		VkQueue present_queue;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;

		bool is_complete() const {
			return graphics_family.has_value() && present_family.has_value();
		}
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface);

	void vulkan_device_init(DeviceContext& device_context, VkInstance instance, VkSurfaceKHR surface);
	void vulkan_device_destroy(DeviceContext* device_context);

	void vulkan_device_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, DeviceContext* device_context);
	void vulkan_device_create_logical_device(VkSurfaceKHR surface, DeviceContext* device_context);
}