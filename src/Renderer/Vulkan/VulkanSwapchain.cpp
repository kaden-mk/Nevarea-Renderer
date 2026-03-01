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

	static VkExtent2D wait_for_valid_extent(VulkanContext& context) {
		DeviceContext& device = context.device;
		SurfaceContext& surface = context.surface;

		while (true) {
			auto extent = context.window.get_extent();
			if (extent.width == 0 || extent.height == 0) {
				window_system_wait_events();
				continue;
			}

			query_swapchain_support(device.physical_device, surface);
			if (!has_avaliable_swapchain_support(surface))
				throw std::runtime_error("No avaliable swapchain support!");

			VkExtent2D swapchain_extent = choose_swapchain_extent(surface.capabilities, extent);
			if (swapchain_extent.width > 0 && swapchain_extent.height > 0)
				return swapchain_extent;

			window_system_wait_events();
		}
	}

	void vulkan_swapchain_init(VulkanContext& context, VkSwapchainKHR old_swapchain)
	{
		SwapchainContext& swapchain = context.swapchain;
		DeviceContext& device = context.device;
		SurfaceContext& surface = context.surface;

		VkExtent2D swapchain_extent = wait_for_valid_extent(context);
		VkSurfaceFormatKHR surface_format = choose_surface_format(surface.supported_formats);
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

	void recreate_swapchain(VulkanContext& context) {
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
}