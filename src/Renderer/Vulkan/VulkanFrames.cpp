#include "VulkanFrames.hpp"
#include "Core/InternalState.hpp"
#include "VulkanResourceManager.hpp"

namespace Nevarea::Renderer {
	static NEVAREA_FORCE_INLINE VkCommandBuffer prepare_command_buffer(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window) {
		VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX,
			frame.image_available[frame.current_frame], VK_NULL_HANDLE, &swapchain.current_image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain(swapchain, device, surface, window);
			vulkan_frame_sync_ensure_present_semaphores(frame, device.device, static_cast<uint32_t>(swapchain.images.size()));
			return VK_NULL_HANDLE;
		}

		VkCommandBuffer cmd = frame.command_buffers[frame.current_frame];
		NEVAREA_ASSERT(cmd != VK_NULL_HANDLE, "VULKAN FRAMES", "The Drawing Command Buffer is Null!");

		VK_ASSERT(vkResetCommandBuffer(cmd, 0));

		return cmd;
	}

	VkCommandBuffer begin_frame(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window) {		
		uint64_t wait_value = frame.frame_timeline_target[frame.current_frame];

		VkSemaphoreWaitInfo wait_info{};
		wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		wait_info.semaphoreCount = 1;
		wait_info.pSemaphores = &frame.timeline;
		wait_info.pValues = &wait_value;

		VK_ASSERT(vkWaitSemaphores(device.device, &wait_info, UINT64_MAX));

		vulkan_resources_flush_deletors(frame.deletion_queues[frame.current_frame]);

		VkCommandBuffer cmd = prepare_command_buffer(frame, swapchain, device, surface, window);
		if (cmd == VK_NULL_HANDLE) return VK_NULL_HANDLE;

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_ASSERT(vkBeginCommandBuffer(cmd, &begin_info));

		return cmd;
	}

	void begin_rendering(VkCommandBuffer cmd, SwapchainContext& swapchain) {
		transition_image(cmd, swapchain.images[swapchain.current_image_index],
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_2_NONE, 0,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

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
	}

	static NEVAREA_FORCE_INLINE void handle_queues(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window, VkCommandBuffer cmd) {
		uint32_t img_idx = swapchain.current_image_index;
		VkSemaphore signal_semaphores[] = { frame.render_finished[img_idx] };

		VkSemaphoreSubmitInfo wait_info{};
		wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		wait_info.semaphore = frame.image_available[frame.current_frame];
		wait_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		uint64_t signal_value = ++frame.timeline_value;

		VkSemaphoreSubmitInfo signal_infos[2]{};
		signal_infos[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signal_infos[0].semaphore = frame.render_finished[img_idx];
		signal_infos[0].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		signal_infos[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signal_infos[1].semaphore = frame.timeline;
		signal_infos[1].value = signal_value;
		signal_infos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

		VkCommandBufferSubmitInfo cmd_info{};
		cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
		cmd_info.commandBuffer = cmd;

		VkSubmitInfo2 submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submit_info.waitSemaphoreInfoCount = 1;
		submit_info.pWaitSemaphoreInfos = &wait_info;
		submit_info.signalSemaphoreInfoCount = 2;
		submit_info.pSignalSemaphoreInfos = signal_infos;
		submit_info.commandBufferInfoCount = 1;
		submit_info.pCommandBufferInfos = &cmd_info;

		VK_ASSERT(vkQueueSubmit2(device.graphics_queue, 1, &submit_info, VK_NULL_HANDLE));

		frame.frame_timeline_target[frame.current_frame] = signal_value;

		VkSwapchainKHR swapchains[] = { swapchain.swapchain };

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &swapchain.current_image_index;

		VkResult result = vkQueuePresentKHR(device.present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreate_swapchain(swapchain, device, surface, window);
			vulkan_frame_sync_ensure_present_semaphores(frame, device.device, static_cast<uint32_t>(swapchain.images.size()));
		}
	}

	void end_frame_rendering(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window, VkCommandBuffer cmd)
	{
		vkCmdEndRendering(cmd);

		VkImageMemoryBarrier2 end_barrier{};
		end_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		end_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		end_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		end_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
		end_barrier.dstAccessMask = 0;
		end_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		end_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		end_barrier.image = swapchain.images[swapchain.current_image_index];
		end_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkDependencyInfo dependency_info{};
		dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dependency_info.dependencyFlags = 0;
		dependency_info.memoryBarrierCount = 0;
		dependency_info.pMemoryBarriers = NULL;
		dependency_info.bufferMemoryBarrierCount = 0;
		dependency_info.pBufferMemoryBarriers = NULL;
		dependency_info.imageMemoryBarrierCount = 1;
		dependency_info.pImageMemoryBarriers = &end_barrier;

		vkCmdPipelineBarrier2(cmd, &dependency_info);

		VK_ASSERT(vkEndCommandBuffer(cmd));

		handle_queues(frame, swapchain, device, surface, window, cmd);

		frame.current_frame = (frame.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void transition_image(VkCommandBuffer cmd, VkImage image,
		VkImageLayout old_layout, VkImageLayout new_layout,
		VkPipelineStageFlags2 src_stage, VkAccessFlags2 src_access,
		VkPipelineStageFlags2 dst_stage, VkAccessFlags2 dst_access)
	{
		VkImageMemoryBarrier2 barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.srcStageMask = src_stage;
		barrier.srcAccessMask = src_access;
		barrier.dstStageMask = dst_stage;
		barrier.dstAccessMask = dst_access;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.image = image;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		VkDependencyInfo dep{};
		dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		dep.imageMemoryBarrierCount = 1;
		dep.pImageMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(cmd, &dep);
	}

	void blit_image(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D src_extent, VkExtent2D dst_extent)
	{
		VkImageBlit2 region{};
		region.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
		region.srcOffsets[1] = { static_cast<int32_t>(src_extent.width), static_cast<int32_t>(src_extent.height), 1 };
		region.dstOffsets[1] = { static_cast<int32_t>(dst_extent.width), static_cast<int32_t>(dst_extent.height), 1 };
		region.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		region.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };

		VkBlitImageInfo2 blit{};
		blit.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
		blit.srcImage = src;
		blit.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blit.dstImage = dst;
		blit.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blit.regionCount = 1;
		blit.pRegions = &region;
		blit.filter = VK_FILTER_LINEAR;

		vkCmdBlitImage2(cmd, &blit);
	}

	void end_frame_present(FrameContext& frame, SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window, VkCommandBuffer cmd)
	{
		VK_ASSERT(vkEndCommandBuffer(cmd));
		handle_queues(frame, swapchain, device, surface, window, cmd);
		frame.current_frame = (frame.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}