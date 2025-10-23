#include <vector>

#include "VulkanContext.hpp"

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

		// layer properties
		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation" // will probably do more?
		};

		// extension properties (TODO: validate extensions that can be used)
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> avaliable_extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, avaliable_extensions.data());

		const std::vector<const char*> instance_extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME
		};

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
		instance_create_info.ppEnabledLayerNames = validation_layers.data();
		instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

		VkResult instance_result = vkCreateInstance(&instance_create_info, nullptr, &context.instance);
		if (instance_result != VK_SUCCESS)
			throw std::runtime_error("VkInstance could not be created!");
	}
}