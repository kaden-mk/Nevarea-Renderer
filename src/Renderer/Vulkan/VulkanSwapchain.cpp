#include "VulkanSwapchain.hpp"
#include "VulkanDebug.hpp"

namespace Nevarea::Renderer {
	void query_swapchain_support(VkPhysicalDevice physical_device, SurfaceContext& surface) {
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface.surface, &surface.capabilities);
		NEVAREA_ASSERT(result == VK_SUCCESS, "VULKAN SWAPCHAIN", "Could not get physical device surface capabilities!");

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

	static VkSurfaceFormatKHR choose_surface_format(std::vector<VkSurfaceFormatKHR> surface_formats) {
		for (const auto& format : surface_formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}

		return surface_formats[0];
	}

	static VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D initial_window_extent) {
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
			return capabilities.currentExtent;
		else {
			VkExtent2D extent = initial_window_extent;

			extent.width = (std::max)(
				capabilities.minImageExtent.width,
				(std::min)(capabilities.maxImageExtent.width, extent.width));

			extent.height = (std::max)(
				capabilities.minImageExtent.height,
				(std::min)(capabilities.maxImageExtent.height, extent.height));

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

	static VkExtent2D wait_for_valid_extent(WindowHandle window, DeviceContext& device, SurfaceContext& surface) {
		while (true) {
			NvWinExtent win_extent = window_get_extent(window);
			VkExtent2D extent = { win_extent.width, win_extent.height };
			if (extent.width == 0 || extent.height == 0) {
				window_system_wait_events();
				continue;
			}

			query_swapchain_support(device.physical_device, surface);
			NEVAREA_ASSERT(has_available_swapchain_support(surface), "VULKAN SWAPCHAIN", "No available swapchain support!");

			VkExtent2D swapchain_extent = choose_swapchain_extent(surface.capabilities, extent);
			if (swapchain_extent.width > 0 && swapchain_extent.height > 0)
				return swapchain_extent;

			window_system_wait_events();
		}
	}

	static void vulkan_swapchain_image_views(SwapchainContext& swapchain, VkDevice device) {
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

			VK_ASSERT(vkCreateImageView(device, &view_info, nullptr, &swapchain.image_views[i]));

			char name[64];
			std::snprintf(name, sizeof(name), "swapchain_image_view[%zu]", i);
			VK_NAME(device, VK_OBJECT_TYPE_IMAGE_VIEW, swapchain.image_views[i], name);
		}
	}

	void vulkan_swapchain_init(SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window, VkSwapchainKHR old_swapchain)
	{
		VkExtent2D swapchain_extent = wait_for_valid_extent(window, device, surface);
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

		VK_ASSERT(vkCreateSwapchainKHR(device.device, &create_info, nullptr, &swapchain.swapchain));
		VK_NAME(device.device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, swapchain.swapchain, "swapchain");

		if (old_swapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(device.device, old_swapchain, nullptr);

		swapchain.image_format = surface_format.format;
		swapchain.extent = swapchain_extent;
		
		uint32_t swapchain_image_count = 0;
		vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchain_image_count, nullptr);
		swapchain.images.resize(swapchain_image_count);
		vkGetSwapchainImagesKHR(device.device, swapchain.swapchain, &swapchain_image_count, swapchain.images.data());

		vulkan_swapchain_image_views(swapchain, device.device);
	}

	void recreate_swapchain(SwapchainContext& swapchain, DeviceContext& device, SurfaceContext& surface, WindowHandle window) {
		vkDeviceWaitIdle(device.device);

		for (auto imageView : swapchain.image_views)
			vkDestroyImageView(device.device, imageView, nullptr);

		swapchain.image_views.clear();
		
		VkSwapchainKHR old_handle = swapchain.swapchain;
		vulkan_swapchain_init(swapchain, device, surface, window, old_handle);
	}

	void vulkan_swapchain_destroy(SwapchainContext& swapchain, VkDevice device) {
		for (size_t i = 0; i < swapchain.image_views.size(); i++)
			vkDestroyImageView(device, swapchain.image_views[i], nullptr);
		
		vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
	}

	void vulkan_frame_sync_init(FrameContext& frame_sync, DeviceContext& device_context)
	{
		VkDevice device = device_context.device;

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
			VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, nullptr, &frame_sync.image_available[i]));
			VK_ASSERT(vkCreateSemaphore(device, &semaphore_info, nullptr, &frame_sync.render_finished[i]));
			VK_ASSERT(vkCreateFence(device, &fence_info, nullptr, &frame_sync.in_flight[i]));

			char name[64];
			std::snprintf(name, sizeof(name), "image_available[%zu]", i);
			VK_NAME(device, VK_OBJECT_TYPE_SEMAPHORE, frame_sync.image_available[i], name);
			std::snprintf(name, sizeof(name), "render_finished[%zu]", i);
			VK_NAME(device, VK_OBJECT_TYPE_SEMAPHORE, frame_sync.render_finished[i], name);
			std::snprintf(name, sizeof(name), "in_flight[%zu]", i);
			VK_NAME(device, VK_OBJECT_TYPE_FENCE, frame_sync.in_flight[i], name);
		}

		//QueueFamilyIndices indices = find_queue_families(physical_device, surface);

		//NEVAREA_ASSERT(indices.graphics_family.has_value(), "VULKAN SWAPCHAIN", "No graphcis queue family found!");

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = device_context.graphics_family_index;

		VK_ASSERT(vkCreateCommandPool(device, &pool_info, nullptr, &frame_sync.command_pool));
		VK_NAME(device, VK_OBJECT_TYPE_COMMAND_POOL, frame_sync.command_pool, "frame_command_pool");

		frame_sync.command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocation_info{};
		allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation_info.commandPool = frame_sync.command_pool;
		allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

		VK_ASSERT(vkAllocateCommandBuffers(device, &allocation_info, frame_sync.command_buffers.data()));

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			char name[64];
			std::snprintf(name, sizeof(name), "command_buffer[%zu]", i);
			VK_NAME(device, VK_OBJECT_TYPE_COMMAND_BUFFER, frame_sync.command_buffers[i], name);
		}
	}

	void vulkan_frame_sync_destroy(FrameContext& frame_sync, VkDevice device)
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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