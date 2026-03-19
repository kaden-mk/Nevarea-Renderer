#include "Core/n_pch.hpp"
#include "lib/Core.hpp"

namespace Nevarea::Renderer {
	const std::vector<const char*> validation_layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> instance_extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

		#ifdef NEVAREA_PLATFORM_WINDOWS
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		#endif
	};

	const std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
	};
}