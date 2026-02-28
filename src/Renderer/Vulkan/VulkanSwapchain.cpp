#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"
#include "Core/InternalState.hpp"

#include <stdexcept>
#include <iostream>

namespace Nevarea::Renderer {
	void query_swapchain_support(VkPhysicalDevice physical_device, SurfaceContext& surface) {
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface.surface, &surface.capabilities);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Could not get physical device surface capabilities!");

		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface.surface, &format_count, nullptr);
		if (format_count > 0) {
			surface.supported_formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface.surface, &format_count, surface.supported_formats.data());
		}

		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface.surface, &present_mode_count, nullptr);
		if (present_mode_count > 0) {
			surface.supported_present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface.surface, &present_mode_count, surface.supported_present_modes.data());
		}
	}

	static bool has_avaliable_swapchain_support(SurfaceContext surface) { return !surface.supported_formats.empty() && !surface.supported_present_modes.empty(); }

	static VkSurfaceFormatKHR choose_surface_format(std::vector<VkSurfaceFormatKHR> surface_formats) {
		for (const auto& format : surface_formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}

		return surface_formats[0];
	}

	static VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D initial_window_extent) {
		// If the platform has already chosen the extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;
		else { // Otherwise just clamp it to match the maximum extent using the framebuffer size
			VkExtent2D extent = initial_window_extent;

			extent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, extent.width));

			extent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, extent.height));

			return extent;
		}
	}

	static VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
		for (const auto& present_mode : present_modes) {
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "Present mode: Mailbox" << std::endl;
				return present_mode;
			}
		}

		std::cout << "Present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void vulkan_swapchain_init(VulkanContext& context, VkSwapchainKHR old_swapchain)
	{
        SwapchainContext& swapchain = context.swapchain;
		DeviceContext& device = context.device;
		SurfaceContext& surface = context.surface;

		query_swapchain_support(device.physical_device, surface);
		if (has_avaliable_swapchain_support(surface) == false)
			throw std::runtime_error("No avaliable swapchain support!");

		VkSurfaceFormatKHR surface_format = choose_surface_format(surface.supported_formats);
		VkExtent2D swapchain_extent = choose_swapchain_extent(surface.capabilities, context.window.get_extent());
		VkPresentModeKHR swapchain_present_mode = choose_swapchain_present_mode(surface.supported_present_modes);

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.surface = surface.surface;

		uint32_t image_count = surface.capabilities.minImageCount + 1;
		if (surface.capabilities.minImageCount > 0 && image_count > surface.capabilities.maxImageCount)
			image_count = surface.capabilities.maxImageCount;

		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = swapchain_extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO: add a check to switch between concurrent and exclusive
		create_info.preTransform = surface.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = swapchain_present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = old_swapchain;

		if (vkCreateSwapchainKHR(device.device, &create_info, nullptr, &context.swapchain.swapchain) != VK_SUCCESS)
			throw std::runtime_error("VkSwapchainKHR could not be created!");

		if (old_swapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(device.device, old_swapchain, nullptr);

		swapchain.image_format = surface_format.format;
		swapchain.extent = swapchain_extent;
		
		uint32_t swapchain_image_count = 0;
		vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchain_image_count, nullptr);
		swapchain.images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchain_image_count, swapchain.images.data());

		// this might need to be a function?
		swapchain.image_views.resize(swapchain.images.size());
		for (size_t i = 0; i < swapchain.images.size(); i++) {
			VkImageViewCreateInfo view_info{};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = swapchain.images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = swapchain.image_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device.device, &view_info, nullptr, &swapchain.image_views[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create texture image view!");
		}
	}

	static void recreate_swapchain(VulkanContext& context) {
		auto extent = context.window.get_extent();
		
		while (extent.width == 0 || extent.height == 0) {
			extent = context.window.get_extent();
			window_system_wait_events();
		}

		vkDeviceWaitIdle(context.device.device);

		for (auto imageView : context.swapchain.image_views)
			vkDestroyImageView(context.device.device, imageView, nullptr);

		context.swapchain.image_views.clear();
		
		VkSwapchainKHR old_handle = context.swapchain.swapchain;
		vulkan_swapchain_init(context, old_handle);
	}

	void vulkan_swapchain_destroy(const SwapchainContext swapchain, VkDevice device) {
		for (size_t i = 0; i < swapchain.image_views.size(); i++)
			vkDestroyImageView(device, swapchain.image_views[i], nullptr);
		
		vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
	}

	void vulkan_frame_sync_init(VulkanContext& context)
	{
		RendererConfig config = Internal::get_renderer_config();
		FrameContext& frame_sync = context.frame_sync;
		VkDevice device = context.device.device;
		VkPhysicalDevice physical_device = context.device.physical_device;
		VkSurfaceKHR surface = context.surface.surface;

		uint32_t MAX_FRAMES_IN_FLIGHT = config.max_frames_in_flight;

		frame_sync.current_frame = 0;

		frame_sync.image_available.resize(MAX_FRAMES_IN_FLIGHT);
		frame_sync.render_finished.resize(MAX_FRAMES_IN_FLIGHT);
		frame_sync.in_flight.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphore_info, nullptr, &frame_sync.image_available[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image_available semaphore!");

			if (vkCreateSemaphore(device, &semaphore_info, nullptr, &frame_sync.render_finished[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create render_finished semaphore!");

			if (vkCreateFence(device, &fence_info, nullptr, &frame_sync.in_flight[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create in_flight fence!");
		}

		QueueFamilyIndices indices = find_queue_families(physical_device, surface);

		// shouldnt this already be assumed that one exists?
		if (!indices.graphics_family.has_value())
			throw std::runtime_error("No graphics queue family found!");

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = indices.graphics_family.value();

		vkCreateCommandPool(device, &pool_info, nullptr, &frame_sync.command_pool);

		frame_sync.command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocation_info{};
		allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation_info.commandPool = frame_sync.command_pool;
		allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

		vkAllocateCommandBuffers(device, &allocation_info, frame_sync.command_buffers.data());
	}

	void vulkan_frame_sync_destroy(FrameContext& frame_sync, VkDevice device)
	{
		for (size_t i = 0; i < Internal::get_renderer_config().max_frames_in_flight; i++) {
			vkDestroySemaphore(device, frame_sync.image_available[i], nullptr);
			vkDestroySemaphore(device, frame_sync.render_finished[i], nullptr);
			vkDestroyFence(device, frame_sync.in_flight[i], nullptr);
		}

		if (frame_sync.command_pool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device, frame_sync.command_pool, nullptr);
			frame_sync.command_pool = VK_NULL_HANDLE;
		}
	}

	void draw_frame(VulkanContext& context)
	{
		FrameContext& frame = context.frame_sync;
		SwapchainContext& swapchain = context.swapchain;

		vkWaitForFences(context.device.device, 1, &frame.in_flight[frame.current_frame], VK_TRUE, UINT64_MAX);

		uint32_t image_index;
		VkResult result = vkAcquireNextImageKHR(context.device.device, context.swapchain.swapchain, UINT64_MAX, frame.image_available[frame.current_frame], VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreate_swapchain(context);
			return;
		}

		vkResetFences(context.device.device, 1, &frame.in_flight[frame.current_frame]);

		VkCommandBuffer& cmd = frame.command_buffers[frame.current_frame];
		if (cmd == VK_NULL_HANDLE)
			throw std::runtime_error("Drawing Command buffer is NULL!");

		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(cmd, &begin_info);

		VkImageMemoryBarrier begin_barrier{};
		begin_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		begin_barrier.srcAccessMask = 0;
		begin_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		begin_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		begin_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		begin_barrier.image = swapchain.images[image_index];
		begin_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &begin_barrier);

		VkRenderingAttachmentInfo color_attachment{};
		color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		color_attachment.imageView = context.swapchain.image_views[image_index];
		color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.clearValue = { {{0.0f, 0.0f, 0.0f, 1.0f}} };

		VkRenderingInfo rendering_info{};
		rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		rendering_info.renderArea = { {0, 0}, context.swapchain.extent };
		rendering_info.layerCount = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments = &color_attachment;

		vkCmdBeginRendering(cmd, &rendering_info);

		// draw commands

		vkCmdEndRendering(cmd);

		VkImageMemoryBarrier end_barrier{};
		end_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		end_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		end_barrier.dstAccessMask = 0;
		end_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		end_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		end_barrier.image = swapchain.images[image_index];
		end_barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &end_barrier);

		vkEndCommandBuffer(cmd);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = { frame.image_available[frame.current_frame] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd;

		VkSemaphore signal_semaphores[] = { frame.render_finished[frame.current_frame] };

		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		vkQueueSubmit(context.device.graphics_queue, 1, &submit_info, frame.in_flight[frame.current_frame]);

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;

		VkSwapchainKHR swapchains[] = { context.swapchain.swapchain };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapchains;
		present_info.pImageIndices = &image_index;

		result = vkQueuePresentKHR(context.device.present_queue, &present_info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			recreate_swapchain(context);

		frame.current_frame = (frame.current_frame + 1) % Internal::get_renderer_config().max_frames_in_flight;
	}
}