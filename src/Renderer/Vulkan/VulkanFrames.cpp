#include "VulkanFrames.hpp"
#include "Core/InternalState.hpp"

namespace Nevarea::Renderer {
	static NEVAREA_FORCE_INLINE VkCommandBuffer prepare_command_buffer(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, NevareaWindowState window) {
		vkWaitForFences(device.device, 1, &frame.in_flight[frame.current_frame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX,
			frame.image_available[frame.current_frame], VK_NULL_HANDLE, &swapchain.current_image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreate_swapchain(swapchain, device, surface, window);
			return VK_NULL_HANDLE;
		}

		vkResetFences(device.device, 1, &frame.in_flight[frame.current_frame]);

		VkCommandBuffer cmd = frame.command_buffers[frame.current_frame];
		NEVAREA_ASSERT(cmd != VK_NULL_HANDLE, "VULKAN FRAMES", "The Drawing Command Buffer is Null!");

		vkResetCommandBuffer(cmd, 0);

		return cmd;
	}

	VkCommandBuffer begin_frame_rendering(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, NevareaWindowState window) {
		VkCommandBuffer cmd = prepare_command_buffer(frame, swapchain, device, surface, window);
		if (cmd == VK_NULL_HANDLE) return VK_NULL_HANDLE;

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(cmd, &begin_info);

		VkImageMemoryBarrier begin_barrier{};
		begin_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		begin_barrier.srcAccessMask = 0;
		begin_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		begin_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		begin_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		begin_barrier.image = swapchain.images[swapchain.current_image_index];
		begin_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &begin_barrier);

		VkRenderingAttachmentInfo color_attachment{};
		color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		color_attachment.imageView = swapchain.image_views[swapchain.current_image_index];
		color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.clearValue = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

		VkRenderingInfo rendering_info{};
		rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		rendering_info.renderArea = { {0, 0}, swapchain.extent };
		rendering_info.layerCount = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments = &color_attachment;

		vkCmdBeginRendering(cmd, &rendering_info);

		return cmd;
	}

	static NEVAREA_FORCE_INLINE void handle_queues(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, NevareaWindowState window, VkCommandBuffer cmd) {
		VkSemaphore wait_semaphores[] = { frame.image_available[frame.current_frame] };
		VkSemaphore signal_semaphores[] = { frame.render_finished[frame.current_frame] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		vkQueueSubmit(device.graphics_queue, 1, &submit_info, frame.in_flight[frame.current_frame]);

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swapchains[] = { swapchain.swapchain };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &swapchain.current_image_index;

		VkResult result = vkQueuePresentKHR(device.present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			recreate_swapchain(swapchain, device, surface, window);
	}

	void end_frame_rendering(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, NevareaWindowState window, VkCommandBuffer cmd)
	{
		vkCmdEndRendering(cmd);

		VkImageMemoryBarrier end_barrier{};
		end_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		end_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		end_barrier.dstAccessMask = 0;
		end_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		end_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		end_barrier.image = swapchain.images[swapchain.current_image_index];
		end_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &end_barrier);

		vkEndCommandBuffer(cmd);

		handle_queues(frame, swapchain, device, surface, window, cmd);

		frame.current_frame = (frame.current_frame + 1) % Internal::get_renderer_config().max_frames_in_flight;
	}
}