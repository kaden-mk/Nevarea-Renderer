#include "VulkanDevice.hpp"
#include "VulkanSpec.hpp"
#include "VulkanSwapchain.hpp"

namespace Nevarea::Renderer {
	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		for (uint32_t i = 0; i < queue_family_count; i++) {
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics_family = i;
			else if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.compute_family = i;

			if (!indices.compute_family.has_value())
				indices.compute_family = indices.graphics_family;

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

			if (present_support)
				indices.present_family = i;

			if (indices.is_complete()) break;
		}

		return indices;
	}

	static bool check_device_extension_support(VkPhysicalDevice device)
	{
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

		std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

		for (const auto& extension : extensions)
			required_extensions.erase(extension.extensionName);

		return required_extensions.empty();
	}

	static bool is_swapchain_adequate(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t format_count = 0;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr) != VK_SUCCESS || format_count == 0)
			return false;

		uint32_t present_mode_count = 0;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr) != VK_SUCCESS || present_mode_count == 0)
			return false;

		VkSurfaceCapabilitiesKHR capabilities;
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities) != VK_SUCCESS)
			return false;

		if (capabilities.maxImageCount > 0 && capabilities.minImageCount > capabilities.maxImageCount)
			return false;

		return true;
	}

	static bool check_device_compatibility(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		bool extensions_supported = check_device_extension_support(device);
		bool swapchain_adequate = is_swapchain_adequate(device, surface);
		bool queue_family_is_complete = find_queue_families(device, surface).is_complete();

		VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.pNext = &features12;
		VkPhysicalDeviceFeatures2 device_features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		device_features.pNext = &features13;

		vkGetPhysicalDeviceFeatures2(device, &device_features);

		return queue_family_is_complete
			&& extensions_supported
			&& swapchain_adequate
			&& device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			&& features13.dynamicRendering // TODO: make this not hardcoded
			&& features13.synchronization2
			&& features12.bufferDeviceAddress;
	}

	void vulkan_device_init(DeviceContext& device_context, VkInstance instance, VkSurfaceKHR surface)
	{
		vulkan_device_pick_physical_device(instance, surface, &device_context);
		vulkan_device_create_logical_device(surface, &device_context);
	}

	void vulkan_device_destroy(DeviceContext* device_context)
	{
		vkDestroyDevice(device_context->device, nullptr);
	}

	void vulkan_device_pick_physical_device(VkInstance instance, VkSurfaceKHR surface, DeviceContext* device_context)
	{
		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// TODO: check if the device has the highest amount of memory(?)
		for (const VkPhysicalDevice device : physical_devices) {
			if (check_device_compatibility(device, surface)) {
				device_context->physical_device = device;
				break;
			}
		}

		NEVAREA_ASSERT(device_context->physical_device != VK_NULL_HANDLE,
			"VULKAN DEVICE", "Could not find a compatible physical device!");

		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceProperties(device_context->physical_device, &device_properties);
		vkGetPhysicalDeviceFeatures(device_context->physical_device, &device_features);

		std::cout << "Physical Device Chosen: " << device_properties.deviceName << std::endl;
	}

	void vulkan_device_create_logical_device(VkSurfaceKHR surface, DeviceContext* device_context)
	{
		QueueFamilyIndices indices = find_queue_families(device_context->physical_device, surface);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_families = {
			indices.graphics_family.value(),
			indices.present_family.value(),
			indices.compute_family.value()
		};

		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;

			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceVulkan12Features features12{};
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.bufferDeviceAddress = VK_TRUE;
		features12.scalarBlockLayout = VK_TRUE;
		features12.runtimeDescriptorArray = VK_TRUE;
		features12.descriptorBindingPartiallyBound = VK_TRUE;
		features12.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
		features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		features12.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
		features12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
		features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		features12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;

		VkPhysicalDeviceVulkan13Features features13{};
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = VK_TRUE;
		features13.synchronization2 = VK_TRUE;
		features13.pNext = &features12;

		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.features.samplerAnisotropy = VK_TRUE;
		features2.features.vertexPipelineStoresAndAtomics = VK_TRUE;
		features2.features.fragmentStoresAndAtomics = VK_TRUE;
		features2.features.shaderInt64 = VK_TRUE;
		features2.pNext = &features13;

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pNext = &features2;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = nullptr;
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		create_info.ppEnabledExtensionNames = device_extensions.data();
		create_info.enabledLayerCount = 0;

		#ifdef NEVAREA_DEBUG
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
		#endif

		NEVAREA_ASSERT(vkCreateDevice(device_context->physical_device, &create_info, nullptr, &device_context->device) == VK_SUCCESS,
			"VULKAN DEVICE", "Could not create logical device!")

		device_context->graphics_family_index = indices.graphics_family.value();
		device_context->compute_family_index = indices.compute_family.value();

		vkGetDeviceQueue(device_context->device, indices.graphics_family.value(), 0, &device_context->graphics_queue);
		vkGetDeviceQueue(device_context->device, indices.present_family.value(), 0, &device_context->present_queue);
		vkGetDeviceQueue(device_context->device, indices.compute_family.value(), 0, &device_context->compute_queue);
	}
}