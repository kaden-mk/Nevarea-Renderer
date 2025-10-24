#include "VulkanDevice.hpp"
#include "VulkanSpec.hpp"

#include <vector>
#include <iostream>
#include <set>

namespace Nevarea {
	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
		
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		int i = 0;
		for (const auto& queue_family : queue_families) {
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics_family = i;

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

			if (present_support)
				indices.present_family = i;

			if (indices.is_complete())
				break;
			i++;
		}

		return indices;
	}

	bool check_device_extension_support(VkPhysicalDevice device)
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

	bool check_device_compatibility(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		QueueFamilyIndices indices = find_queue_families(device, surface);
		bool extensions_supported = check_device_extension_support(device);

		bool swap_chain_adequate = false;
		if (extensions_supported) {
			swap_chain_adequate = true; // replace with the stuff below once added
			/*SwapChainSupportDetails swap_chain_support = query_swap_chain_support(device);
			swap_chain_adequate = !swap_chain_support.Formats.empty() && !swap_chain_support.PresentModes.empty();*/
		}

		return indices.is_complete()
			&& extensions_supported
			&& swap_chain_adequate
			&& device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	}

	void vulkan_device_pick_physical_device(VkInstance instance, VkPhysicalDevice* physical_device, VkSurfaceKHR surface)
	{
		uint32_t physical_device_count = 0;
		vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

		std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
		vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

		// TODO: check if the device has the highest amount of memory(?)
		for (const VkPhysicalDevice device : physical_devices) {
			if (check_device_compatibility(device, surface)) {
				*physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE)
			throw std::runtime_error("Could not find a compatible physical device!");

		// might add these later to the context struct or some shit... maybe have a device struct?
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceProperties(*physical_device, &device_properties);
		vkGetPhysicalDeviceFeatures(*physical_device, &device_features);

		std::cout << "Physical Device Chosen: " << device_properties.deviceName << std::endl;
	}

	void vulkan_device_create_logical_device(VkInstance instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkDevice* device)
	{
		QueueFamilyIndices indices = find_queue_families(physical_device, surface);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };

		float queue_priority = 1.0f;
		for (uint32_t queue_family : unique_queue_families) {
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{};
		indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexing_features.pNext = nullptr;
		indexing_features.runtimeDescriptorArray = true;
		indexing_features.descriptorBindingPartiallyBound = true;
		indexing_features.shaderStorageBufferArrayNonUniformIndexing = true;
		indexing_features.shaderSampledImageArrayNonUniformIndexing = true;
		indexing_features.shaderStorageImageArrayNonUniformIndexing = true;
		indexing_features.descriptorBindingStorageBufferUpdateAfterBind = true;
		indexing_features.descriptorBindingSampledImageUpdateAfterBind = true;
		indexing_features.descriptorBindingStorageImageUpdateAfterBind = true;

		VkPhysicalDeviceScalarBlockLayoutFeatures scalar_layout_features{};
		scalar_layout_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
		scalar_layout_features.scalarBlockLayout = VK_TRUE;

		VkPhysicalDeviceFeatures2 device_features{};
		device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		device_features.features.samplerAnisotropy = true;
		device_features.features.vertexPipelineStoresAndAtomics = true;
		device_features.features.fragmentStoresAndAtomics = true;
		device_features.pNext = &scalar_layout_features;

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pNext = &indexing_features;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		create_info.pQueueCreateInfos = queue_create_infos.data();
		create_info.pEnabledFeatures = &device_features.features;
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		create_info.ppEnabledExtensionNames = device_extensions.data();
		create_info.enabledLayerCount = 0;

		#ifdef NEVAREA_DEBUG
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
		#endif	

		if (vkCreateDevice(physical_device, &create_info, nullptr, device))
			throw std::runtime_error("Could not create logical device!");

		/*vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);*/
	}
}