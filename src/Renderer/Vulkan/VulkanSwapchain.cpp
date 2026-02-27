#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"

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

	bool has_avaliable_swapchain_support(SurfaceContext surface) { return !surface.supported_formats.empty() && !surface.supported_present_modes.empty(); }

	VkSurfaceFormatKHR choose_surface_format(std::vector<VkSurfaceFormatKHR> surface_formats) {
		for (const auto& format : surface_formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}

		return surface_formats[0];
	}

	VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D initial_window_extent) {
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

	VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
		for (const auto& present_mode : present_modes) {
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "Present mode: Mailbox" << std::endl;
				return present_mode;
			}
		}

		std::cout << "Present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void vulkan_swapchain_init(VulkanContext& context)
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
		create_info.oldSwapchain = VK_NULL_HANDLE; // TODO: change this to be the actual old swapchain

		if (vkCreateSwapchainKHR(device.device, &create_info, nullptr, &context.swapchain.swapchain) != VK_SUCCESS)
			throw std::runtime_error("VkSwapchainKHR could not be created!");

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
}