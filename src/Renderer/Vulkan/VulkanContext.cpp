#include "VulkanContext.hpp"
#include "VulkanDebug.hpp"
#include "VulkanSpec.hpp"
#include "VulkanFrames.hpp"

#include "Core/InternalState.hpp"
#include "Core/n_pch.hpp"
#include <lib/Rendering.hpp>

#define NEVAREA_VULKAN_VERSION VK_API_VERSION_1_4

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
		app_info.apiVersion = NEVAREA_VULKAN_VERSION;

		// extension properties (TODO: validate extensions that can be used)
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> avaliable_extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, avaliable_extensions.data());

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
		populate_debug_create_info(debug_create_info);

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledLayerCount = 0;
		instance_create_info.ppEnabledLayerNames = nullptr;
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
		instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

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

	static void vulkan_context_create_surface(WindowHandle window, VkInstance instance, VkSurfaceKHR& surface)
	{
		#ifdef NEVAREA_PLATFORM_WINDOWS
			VkWin32SurfaceCreateInfoKHR create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.hwnd = window_get_hwnd(window);
			create_info.hinstance = window_get_hinstance(window);

			NEVAREA_ASSERT(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface) == VK_SUCCESS,
				"VULKAN CONTEXT", "Could not create Win32 Surface!");
		#endif
	}

	static void vulkan_context_create_allocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, VmaAllocator& allocator) {
		VmaAllocatorCreateInfo create_info{};
		create_info.instance = instance;
		create_info.device = device;
		create_info.physicalDevice = physical_device;
		create_info.vulkanApiVersion = NEVAREA_VULKAN_VERSION;
		create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

		NEVAREA_ASSERT(vmaCreateAllocator(&create_info, &allocator) == VK_SUCCESS,
			"VULKAN CONTEXT", "Could not create VulkanMemoryAllocator!");
	}

	void vulkan_context_init(VulkanContext& context, WindowHandle window) {
		context.window = window;

		vulkan_context_create_instance(context.instance);
		vulkan_context_debug_messenger(context.instance, context.debug_messenger);
		vulkan_context_create_surface(context.window, context.instance, context.surface.surface);
		vulkan_device_init(context.device, context.instance, context.surface.surface);
		vulkan_context_create_allocator(context.instance, context.device.physical_device, context.device.device, context.allocator);
		vulkan_resources_init(context.resource_manager, context.allocator, context.device.device);
		vulkan_swapchain_init(context.swapchain, context.device, context.surface, context.window);
		vulkan_frame_sync_init(context.frame_sync, context.device);
	}

	void vulkan_context_draw(VulkanContext& context) {
		VkCommandBuffer cmd = begin_frame(context.frame_sync, context.swapchain, context.device, context.surface, context.window);
		if (cmd == VK_NULL_HANDLE) return;

		// TODO: barriers
		for (const ComputeDispatch& dispatch : context.compute_dispatches) {
			const PipelineContext& pipeline = context.pipelines[dispatch.pipeline.id];
			vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.pipeline);
			vkCmdBindDescriptorSets(cmd, pipeline.bind_point, pipeline.layout, 0, 1, &context.resource_manager.descriptor_set, 0, nullptr);
			vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &dispatch.push);
			vkCmdDispatch(cmd, dispatch.groups_x, dispatch.groups_y, dispatch.groups_z);
		}
		context.compute_dispatches.clear();

		begin_rendering(cmd, context.swapchain);

		VkViewport viewport{};
		viewport.width = static_cast<float>(context.swapchain.extent.width);
		viewport.height = static_cast<float>(context.swapchain.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{ {0, 0}, context.swapchain.extent };

		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissor);

		// TODO: find a better way to do this, id hate to sort EVERY frame.
		std::sort(context.draw_list.begin(), context.draw_list.end(),
			[](const DrawCall& a, const DrawCall& b) {
				return a.pipeline.id < b.pipeline.id;
			});

		uint32_t current_pipeline_id = UINT32_MAX;

		for (const DrawCall& draw_call : context.draw_list) {
			if (draw_call.pipeline.id != current_pipeline_id) {
				current_pipeline_id = draw_call.pipeline.id;

				const PipelineContext& pipeline = context.pipelines[current_pipeline_id];
				vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.pipeline);
				vkCmdBindDescriptorSets(cmd, pipeline.bind_point, pipeline.layout, 0, 1, &context.resource_manager.descriptor_set, 0, nullptr);
			}

			MeshData& mesh = context.resource_manager.mesh_pool[draw_call.mesh.id];
				
			PushConstants push{};
			push.vertex_buffer_address = vulkan_get_buffer_address(context.resource_manager, mesh.vertex_buffer);
			vkCmdPushConstants(cmd, context.pipelines[draw_call.pipeline.id].layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &push);
				
			vkCmdDraw(cmd, mesh.vertex_count, 1, 0, 0);
		}

		context.draw_list.clear();

		end_frame_rendering(context.frame_sync, context.swapchain, context.device, context.surface, context.window, cmd);
	}

	void vulkan_context_destroy(VulkanContext& context)
	{
		vkDeviceWaitIdle(context.device.device);

		vulkan_frame_sync_destroy(context.frame_sync, context.device.device);
		vulkan_swapchain_destroy(context.swapchain, context.device.device);

		vulkan_resources_destroy(context.resource_manager);
		vmaDestroyAllocator(context.allocator);

		for (PipelineContext& pipeline : context.pipelines)
			vulkan_pipeline_destroy(pipeline, context.device.device);

		//vulkan_pipeline_destroy(context.pipeline, context.device.device);

		vulkan_device_destroy(&context.device);

		#ifdef NEVAREA_DEBUG
		destroy_debug_utils_messenger_ext(context.instance, context.debug_messenger, nullptr);
		#endif // NEVAREA_DEBUG

		vkDestroySurfaceKHR(context.instance, context.surface.surface, nullptr);
		vkDestroyInstance(context.instance, nullptr);
	}
}