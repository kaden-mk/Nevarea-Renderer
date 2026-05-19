#pragma once

#include "VulkanSwapchain.hpp"

namespace Nevarea::Renderer {
	VkCommandBuffer begin_frame(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window);
	void begin_rendering(VkCommandBuffer cmd, SwapchainContext& swapchain);
	void end_frame_rendering(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window, VkCommandBuffer cmd);
}