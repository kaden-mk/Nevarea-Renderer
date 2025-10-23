#include <vector>

#include "VulkanContext.hpp"
#include "VulkanDebug.hpp"

namespace Nevarea {	
	// TODO: seperate this into smaller private functions
	void vulkan_context_init(VulkanContext& context, WindowSystemState* window) {
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = nullptr;
		app_info.pApplicationName = "Nevarea Renderer";
		app_info.pEngineName = "No Engine";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_4;

		// layer properties (put this in a seperate .hpp file)
		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation" // will probably do more?
		};

		// extension properties (TODO: validate extensions that can be used)
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> avaliable_extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, avaliable_extensions.data());

		// TODO: put this in a seperate .hpp file
		const std::vector<const char*> instance_extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		// TODO: make this a function
		VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
		debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_create_info.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_create_info.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_create_info.pfnUserCallback = debug_messenger_callback;
		debug_create_info.pUserData = nullptr;

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledLayerCount = 0;
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
		instance_create_info.ppEnabledLayerNames = nullptr;
		instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

		#ifdef NEVAREA_DEBUG
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			instance_create_info.ppEnabledLayerNames = validation_layers.data();
			instance_create_info.pNext = &debug_create_info;
		#endif

		VkResult instance_result = vkCreateInstance(&instance_create_info, nullptr, &context.instance);
		if (instance_result != VK_SUCCESS)
			throw std::runtime_error("VkInstance could not be created!");

		vulkan_context_debug_messenger(context);
	}

	void vulkan_context_debug_messenger(VulkanContext& context)
	{
		#ifdef NEVAREA_DEBUG
			bool debug = true;
		#else
			bool debug = false;
		#endif

		if (!debug) return;

		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = debug_messenger_callback;

		if (create_debug_utils_messenger_ext(context.instance, &create_info, nullptr, &context.debug_messenger) != VK_SUCCESS) 
			throw std::runtime_error("failed to set up debug messenger!");
	}
}