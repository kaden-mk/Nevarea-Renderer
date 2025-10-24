#include <vector>
#include <vulkan/vulkan.h>

namespace Nevarea {
	const std::vector<const char*> validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> instance_extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
}