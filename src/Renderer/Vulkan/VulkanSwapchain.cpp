#include "VulkanSwapchain.hpp"
#include <stdexcept>

namespace Nevarea::Renderer {
	struct SwapchainSupport {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	SwapchainSupport query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
		SwapchainSupport support{};

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support.capabilities);
		if (result != VK_SUCCESS)
			throw std::runtime_error("Could not get physical device surface capabilities!");

		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
		if (format_count > 0) {
			support.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, support.formats.data());
		}

		uint32_t present_mode_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
		if (present_mode_count > 0) {
			support.present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, support.present_modes.data());
		}

		return support;
	}

	bool has_avaliable_swapchain_support(SwapchainSupport support) { return !support.formats.empty() && !support.present_modes.empty(); }

	void vulkan_swapchain_init(SwapchainContext& swapchain, VkPhysicalDevice physical_device, VkSurfaceKHR surface)
	{
		SwapchainSupport support = query_swapchain_support(physical_device, surface);
		if (has_avaliable_swapchain_support(support) == false)
			throw std::runtime_error("No avaliable swapchain support!");


	}
}