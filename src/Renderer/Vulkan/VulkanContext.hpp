#pragma once

#include <vulkan/vulkan.h>

#include <Platform/WindowSystem.hpp>

namespace Nevarea {
	struct VulkanContext {
		VkInstance instance;
	};

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window);
}