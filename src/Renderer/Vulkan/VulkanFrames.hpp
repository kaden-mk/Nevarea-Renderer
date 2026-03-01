#pragma once

#include "VulkanContext.hpp"

namespace Nevarea::Renderer {
	VkCommandBuffer begin_frame_rendering(VulkanContext& context);
	void end_frame_rendering(VulkanContext& context, VkCommandBuffer cmd);
}