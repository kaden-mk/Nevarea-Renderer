#include <vector>

#include "VulkanContext.hpp"
#include "VulkanDebug.hpp"
#include "VulkanSpec.hpp"
#include "VulkanDevice.hpp"

namespace Nevarea {
	void vulkan_context_create_instance(VulkanContext& context)
	{
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = nullptr;
		app_info.pApplicationName = "Nevarea Application";
		app_info.pEngineName = "No Engine";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_4;

		// extension properties (TODO: validate extensions that can be used)
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> avaliable_extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, avaliable_extensions.data());

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
		helper_populate_debug_create_info(debug_create_info);

		// glfw extensions
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		// ??? (is this needed??)
		std::vector<const char*> combined_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
		combined_extensions.insert(combined_extensions.end(), instance_extensions.begin(), instance_extensions.end());

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledLayerCount = 0;
		instance_create_info.ppEnabledLayerNames = nullptr;
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(combined_extensions.size());
		instance_create_info.ppEnabledExtensionNames = combined_extensions.data();

		#ifdef NEVAREA_DEBUG
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		instance_create_info.ppEnabledLayerNames = validation_layers.data();
		instance_create_info.pNext = &debug_create_info;
		#endif

		VkResult instance_result = vkCreateInstance(&instance_create_info, nullptr, &context.instance);
		if (instance_result != VK_SUCCESS)
			throw std::runtime_error("VkInstance could not be created!");
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
		helper_populate_debug_create_info(create_info);

		if (create_debug_utils_messenger_ext(context.instance, &create_info, nullptr, &context.debug_messenger) != VK_SUCCESS)
			throw std::runtime_error("failed to set up debug messenger!");
	}

	void vulkan_context_create_surface(VulkanContext& context)
	{
		window_system_create_surface(&context.window, context.instance, &context.surface);
	}

	void vulkan_context_pick_physical_device(VulkanContext& context)
	{
		vulkan_device_pick_physical_device(context.instance, &context.physical_device, context.surface);
	}

	void vulkan_context_create_logical_device(VulkanContext& context)
	{
		vulkan_device_create_logical_device(context.instance, context.physical_device, context.surface, &context.device);
	}

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window) {
		context.window = *window;

		vulkan_context_create_instance(context);
		vulkan_context_debug_messenger(context);
		vulkan_context_create_surface(context);
		vulkan_context_pick_physical_device(context);
		vulkan_context_create_logical_device(context);
	}

	void vulkan_context_destroy(VulkanContext& context)
	{
		vkDestroyDevice(context.device, nullptr);

		#ifdef NEVAREA_DEBUG
		destroy_debug_utils_messenger_ext(context.instance, context.debug_messenger, nullptr);
		#endif // NEVAREA_DEBUG

		vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
		vkDestroyInstance(context.instance, nullptr);
	}

	void helper_populate_debug_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_create_info)
	{
		debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_create_info.pfnUserCallback = debug_messenger_callback;
		debug_create_info.pUserData = nullptr;
	}
}