#pragma once

#include "VulkanSwapchain.hpp"

namespace Nevarea::Renderer {
	VkCommandBuffer begin_frame_rendering(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowSystemState* window);
	void end_frame_rendering(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowSystemState* window, VkCommandBuffer cmd);
}