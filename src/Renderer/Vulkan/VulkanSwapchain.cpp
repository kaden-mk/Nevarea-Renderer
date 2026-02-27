#include "VulkanSwapchain.hpp"
#include <stdexcept>
#include "VulkanContext.hpp"

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

	void vulkan_swapchain_init(SwapchainContext& swapchain, VkPhysicalDevice physical_device, SurfaceContext& surface)
	{
		query_swapchain_support(physical_device, surface);
		if (has_avaliable_swapchain_support(surface) == false)
			throw std::runtime_error("No avaliable swapchain support!");


	}
}