#include "VulkanContext.hpp"
#include "VulkanDebug.hpp"
#include "VulkanSpec.hpp"
#include "VulkanFrames.hpp"

#include "Core/InternalState.hpp"
#include "Core/n_pch.hpp"

namespace Nevarea::Renderer {
	static void vulkan_context_create_instance(VkInstance& instance)
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
		populate_debug_create_info(debug_create_info);

		// glfw extensions
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

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

		NEVAREA_ASSERT(vkCreateInstance(&instance_create_info, nullptr, &instance) == VK_SUCCESS,
			"VULKAN CONTEXT", "VkInstance could not be created!");
	}

	static void vulkan_context_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT& debug_messenger)
	{
		#ifdef NEVAREA_DEBUG
		bool debug = true;
		#else
		bool debug = false;
		#endif

		if (!debug) return;

		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		populate_debug_create_info(create_info);
		
		NEVAREA_ASSERT(create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger) == VK_SUCCESS,
			"VULKAN DEBUG CONTEXT", "Debug Messenger could not be setup!");
	}

	static void vulkan_context_create_surface(WindowSystemState* window, VkInstance instance, VkSurfaceKHR& surface)
	{
		window_system_create_surface(window, instance, &surface);
	}

	void vulkan_context_init(VulkanContext& context, WindowSystemState* window) {
		context.window = window;

		vulkan_context_create_instance(context.instance);
		vulkan_context_debug_messenger(context.instance, context.debug_messenger);
		vulkan_context_create_surface(context.window, context.instance, context.surface.surface);
		vulkan_device_init(context.device, context.instance, context.surface.surface);
		vulkan_swapchain_init(context.swapchain, context.device, context.surface, context.window);
		vulkan_frame_sync_init(context.frame_sync, context.device, context.surface);
	}

	void vulkan_context_draw(VulkanContext& context) {
		if (VkCommandBuffer cmd = begin_frame_rendering(context.frame_sync, context.swapchain, context.device, context.surface, context.window); cmd != VK_NULL_HANDLE) {
			// drawing stuff here

			end_frame_rendering(context.frame_sync, context.swapchain, context.device, context.surface, context.window, cmd);
		}
	}

	void vulkan_context_destroy(VulkanContext& context)
	{
		vkDeviceWaitIdle(context.device.device);

		vulkan_frame_sync_destroy(context.frame_sync, context.device.device);
		vulkan_swapchain_destroy(context.swapchain, context.device.device);
		vulkan_device_destroy(&context.device);

		#ifdef NEVAREA_DEBUG
		destroy_debug_utils_messenger_ext(context.instance, context.debug_messenger, nullptr);
		#endif // NEVAREA_DEBUG

		vkDestroySurfaceKHR(context.instance, context.surface.surface, nullptr);
		vkDestroyInstance(context.instance, nullptr);
	}
}