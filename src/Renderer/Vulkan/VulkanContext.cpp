#include "VulkanContext.hpp"
#include "VulkanDebug.hpp"
#include "VulkanSpec.hpp"
#include "VulkanFrames.hpp"

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

		uint32_t extension_count = 0;
		VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));

		std::vector<VkExtensionProperties> avaliable_extensions(extension_count);
		VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, avaliable_extensions.data()));

		std::set<std::string> available;
		for (const auto& extension : avaliable_extensions)
			available.insert(extension.extensionName);

		std::vector<const char*> enabled_extensions;
		for (const char* extension : required_instance_extensions) {
			NEVAREA_ASSERT(available.count(extension) > 0, "VULKAN INSTANCE", "Required instance extension not available!");
			enabled_extensions.push_back(extension);
		}

		for (const char* extension : optional_instance_extensions) {
			if (available.count(extension) > 0) enabled_extensions.push_back(extension);
			else std::cerr << "[NEVAREA]: optional instance extension '" << extension << "' not available, skipping\n";
		}

		VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
		populate_debug_create_info(debug_create_info);

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledLayerCount = 0;
		instance_create_info.ppEnabledLayerNames = nullptr;
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
		instance_create_info.ppEnabledExtensionNames = enabled_extensions.data();

		#ifdef NEVAREA_DEBUG
			uint32_t available_count = 0;
			VK_CHECK(vkEnumerateInstanceLayerProperties(&available_count, nullptr));
			std::vector<VkLayerProperties> available_layers(available_count);
			VK_CHECK(vkEnumerateInstanceLayerProperties(&available_count, available_layers.data()));

			std::vector<const char*> enabled_layers;
			for (const char* requested : validation_layers) {
				bool found = false;
				for (const auto& layer : available_layers)
					if (strcmp(layer.layerName, requested) == 0) { found = true; break; }

				if (found) enabled_layers.push_back(requested);
				else std::cerr << "[NEVAREA]: validation layer '" << requested << "' not available, skipping\n";
			}

			instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
			instance_create_info.ppEnabledLayerNames = enabled_layers.data();
			if (!enabled_layers.empty())
				instance_create_info.pNext = &debug_create_info;
		#endif

		VK_ASSERT(vkCreateInstance(&instance_create_info, nullptr, &instance));
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

		VK_ASSERT(create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger));
	}

	static void vulkan_context_create_surface(WindowHandle window, VkInstance instance, VkSurfaceKHR& surface)
	{
		#ifdef NEVAREA_PLATFORM_WINDOWS
			VkWin32SurfaceCreateInfoKHR create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.hwnd = window_get_hwnd(window);
			create_info.hinstance = window_get_hinstance(window);

			VK_ASSERT(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));
		#endif
	}

	static void vulkan_context_create_allocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device, VmaAllocator& allocator, bool enable_memory_priority) {
		VmaAllocatorCreateInfo create_info{};
		create_info.instance = instance;
		create_info.device = device;
		create_info.physicalDevice = physical_device;
		create_info.vulkanApiVersion = NEVAREA_VULKAN_VERSION;
		create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | (enable_memory_priority ? VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT : 0);

		VK_ASSERT(vmaCreateAllocator(&create_info, &allocator));
	}

	void vulkan_context_init(VulkanContext& context, WindowHandle window) {
		context.window = window;

		vulkan_context_create_instance(context.instance);
		vulkan_context_debug_messenger(context.instance, context.debug_messenger);
		vulkan_context_create_surface(context.window, context.instance, context.surface.surface);
		vulkan_device_init(context.device, context.instance, context.surface.surface);
		vulkan_context_create_allocator(context.instance, context.device.physical_device, context.device.device, context.allocator, context.device.capabilities.memory_priority);
		vulkan_resources_init(context.resource_manager, context.allocator, context.device.device, context.device.graphics_queue, context.device.graphics_family_index);
		vulkan_swapchain_init(context.swapchain, context.device, context.surface, context.window);
		vulkan_frame_sync_init(context.frame_sync, context.device, static_cast<uint32_t>(context.swapchain.images.size()));
	}

	static void transition_tracked(VkCommandBuffer cmd, AllocatedImage& img, VkImageLayout new_layout,
		VkPipelineStageFlags2 src_stage, VkAccessFlags2 src_access,
		VkPipelineStageFlags2 dst_stage, VkAccessFlags2 dst_access)
	{
	    VkImageAspectFlags aspect = k_format_info[(uint32_t)img.format].aspect;
		transition_image(cmd, img.image, img.current_layout, new_layout, src_stage, src_access, dst_stage, dst_access, aspect);
		img.current_layout = new_layout;
	}

	void vulkan_context_draw(VulkanContext& context) {
	    if (context.device.device_lost) return;

	    VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.device.physical_device, context.surface.surface, &caps);
        if (caps.currentExtent.width == 0 || caps.currentExtent.height == 0) {
            context.compute_dispatches.clear();
            context.present_target = { UINT32_MAX, 0 };
            context.passes.clear();

            return;
        }

		VkCommandBuffer cmd = begin_frame(context.frame_sync, context.swapchain, context.device, context.surface, context.window);
		if (cmd == VK_NULL_HANDLE) return;

		bool present = context.present_target.index != UINT32_MAX;
		bool had_compute = !context.compute_dispatches.empty();

		if (present) {
			AllocatedImage& target = vulkan_get_image(context.resource_manager, context.present_target);
			transition_tracked(cmd, target, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
				VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
		}

		for (size_t i = 0; i < context.compute_dispatches.size(); i++) {
			const ComputeDispatch& dispatch = context.compute_dispatches[i];
			const PipelineContext& pipeline = vulkan_pipeline_get(context, dispatch.pipeline);

			uint32_t img = dispatch.target_image.index;
			if (img != UINT32_MAX && img != context.present_target.index
				&& img < context.resource_manager.image_pool.size()) {
				transition_tracked(cmd, context.resource_manager.image_pool[img], VK_IMAGE_LAYOUT_GENERAL,
					VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
					VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
			}

			vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.pipeline);
			vkCmdBindDescriptorSets(cmd, pipeline.bind_point, pipeline.layout, 0, 1, &context.resource_manager.descriptor_set, 0, nullptr);
			vkCmdPushConstants(cmd, pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, dispatch.push_size, dispatch.push_data);
			vkCmdDispatch(cmd, dispatch.groups_x, dispatch.groups_y, dispatch.groups_z);

			if (i + 1 < context.compute_dispatches.size()) {
				VkMemoryBarrier2 barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

				VkDependencyInfo dep{};
				dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				dep.memoryBarrierCount = 1;
				dep.pMemoryBarriers = &barrier;
				vkCmdPipelineBarrier2(cmd, &dep);
			}
		}
		context.compute_dispatches.clear();

		if (present) {
			AllocatedImage& target = vulkan_get_image(context.resource_manager, context.present_target);
			VkImage sc = context.swapchain.images[context.swapchain.current_image_index];

			transition_tracked(cmd, target, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
				VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);

			transition_image(cmd, sc,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
				VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

			blit_image(cmd, target.image, sc, target.extent, context.swapchain.extent);

			transition_image(cmd, sc,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_PIPELINE_STAGE_2_BLIT_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
				VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);

			context.present_target = { UINT32_MAX, 0 };
			context.passes.clear();

			end_frame_present(context.frame_sync, context.swapchain, context.device, context.surface, context.window, cmd);
			return;
		}

		if (had_compute) {
			VkMemoryBarrier2 barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
			barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
			barrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
			barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

			VkDependencyInfo dependency{};
			dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
			dependency.memoryBarrierCount = 1;
			dependency.pMemoryBarriers = &barrier;

			vkCmdPipelineBarrier2(cmd, &dependency);
		}

		VkViewport viewport{};
		viewport.width = static_cast<float>(context.swapchain.extent.width);
		viewport.height = static_cast<float>(context.swapchain.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{ {0, 0}, context.swapchain.extent };

		bool any_present = false;

		for (PassData& pass : context.passes) {
		    for (size_t i = 0; i < pass.color.size(); i++) {
                if (pass.present && i == 0) {
                    transition_image(cmd, context.swapchain.images[context.swapchain.current_image_index],
                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
                    any_present = true;
                } else {
                    transition_tracked(cmd, vulkan_get_image(context.resource_manager, { pass.color[i].image.id, pass.color[i].image.generation }),
                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
                }
            }

    		if (pass.depth.image.is_valid()) {
                transition_tracked(cmd, vulkan_get_image(context.resource_manager, { pass.depth.image.id, pass.depth.image.generation }),
                    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
                    VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
    		}

    		begin_rendering(cmd, pass, context.swapchain, context.resource_manager);
    		vkCmdSetViewport(cmd, 0, 1, &viewport);
    		vkCmdSetScissor(cmd, 0, 1, &scissor);

    		for (DrawBucket& bucket : pass.buckets) {
    			if (bucket.items.empty()) continue;

    			const PipelineContext& pipeline = vulkan_pipeline_get(context, bucket.pipeline);

    			vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.pipeline);
    			vkCmdBindDescriptorSets(cmd, pipeline.bind_point, pipeline.layout, 0, 1, &context.resource_manager.descriptor_set, 0, nullptr);

    			for (const DrawItem& item : bucket.items) {
                    MeshData& mesh = context.resource_manager.mesh_pool[item.mesh.id];

                    uint8_t push[NEVAREA_MAX_PUSH_CONSTANTS_SIZE];
                    memcpy(push, &mesh.vertex_address, sizeof(uint64_t)); // input our address directly
                    if (item.push_size) memcpy(push + sizeof(uint64_t), item.push_data, item.push_size);

                    uint32_t total = (uint32_t)sizeof(uint64_t) + item.push_size;
                    vkCmdPushConstants(cmd, pipeline.layout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0, total, push);

                    if (mesh.index_buffer.is_valid()) {
                        VkBuffer index_buffer = vulkan_get_buffer(context.resource_manager, mesh.index_buffer);
                        vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(cmd,
                            item.index_count, 1,
                            item.first_index, item.vertex_offset,
                            0);
                    } else {
                        vkCmdDraw(cmd, mesh.vertex_count,
                            1, 0, 0);
                    }
                }
    			bucket.items.clear();
    		}

            vkCmdEndRendering(cmd);

            for (size_t i = 0; i < pass.color.size(); i++) {
                if (pass.present && i == 0) continue;
                transition_tracked(cmd, vulkan_get_image(context.resource_manager, { pass.color[i].image.id, pass.color[i].image.generation }),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
            }
		}
		context.passes.clear();

		end_frame_rendering(context.frame_sync, context.swapchain, context.device,
	        context.surface, context.window, cmd, any_present);
	}

	void vulkan_context_destroy(VulkanContext& context)
	{
		vkDeviceWaitIdle(context.device.device);

		for (PipelineContext& pipeline : context.pipelines)
		    if (pipeline.pipeline != VK_NULL_HANDLE)
				vulkan_pipeline_destroy(pipeline, context.device.device, context.frame_sync);

		for (auto& queue : context.frame_sync.deletion_queues) vulkan_resources_flush_deletors(queue);

		vulkan_frame_sync_destroy(context.frame_sync, context.device.device);
		vulkan_swapchain_destroy(context.swapchain, context.device.device);

		vulkan_resources_destroy(context.resource_manager);
		vmaDestroyAllocator(context.allocator);

		vulkan_device_destroy(&context.device);

		#ifdef NEVAREA_DEBUG
		destroy_debug_utils_messenger_ext(context.instance, context.debug_messenger, nullptr);
		#endif // NEVAREA_DEBUG

		vkDestroySurfaceKHR(context.instance, context.surface.surface, nullptr);
		vkDestroyInstance(context.instance, nullptr);
	}

	static DrawBucket& get_bucket(VulkanContext& context, Pipeline pipeline) {
    	NEVAREA_ASSERT(context.current_pass_index >= 0, "RENDERER", "submit_mesh called outside begin_pass/end_pass");

        PassData& pass = context.passes[context.current_pass_index];
        if (pipeline.id >= pass.buckets.size()) pass.buckets.resize(pipeline.id + 1);

        DrawBucket& bucket = pass.buckets[pipeline.id];
        bucket.pipeline = pipeline;
        return bucket;
    }

	void vulkan_submit_mesh(VulkanContext& context, Mesh mesh, Pipeline pipeline, const void* push, size_t push_size) {
		NEVAREA_ASSERT(mesh.id < context.resource_manager.mesh_pool.size(),
			"RESOURCE MANAGER", "Mesh handle index out of range!");

		NEVAREA_ASSERT(mesh.generation == context.resource_manager.mesh_generation_pool[mesh.id],
			"RESOURCE MANAGER", "Stale Mesh handle (generation mismatch)!");

		MeshData& mesh_data = context.resource_manager.mesh_pool[mesh.id];

		NEVAREA_ASSERT(push_size <= NEVAREA_MAX_PUSH_CONSTANTS_SIZE - sizeof(uint64_t),
            "RENDERER", "push data too large (8 bytes reserved for the mesh vertex address)");

		DrawItem item{ mesh, 0, mesh_data.index_count, 0, {}, (uint32_t)push_size };
        if (push && push_size) memcpy(item.push_data, push, push_size);
        get_bucket(context, pipeline).items.push_back(item);
	}

	void vulkan_submit_mesh_range(VulkanContext& context, Mesh mesh, uint32_t first_index, uint32_t index_count, Pipeline pipeline, const void* push, size_t push_size) {
    	NEVAREA_ASSERT(mesh.id < context.resource_manager.mesh_pool.size(),
    		"RESOURCE MANAGER", "Mesh handle index out of range!");

    	NEVAREA_ASSERT(mesh.generation == context.resource_manager.mesh_generation_pool[mesh.id],
    		"RESOURCE MANAGER", "Stale Mesh handle (generation mismatch)!");

   		NEVAREA_ASSERT(push_size <= NEVAREA_MAX_PUSH_CONSTANTS_SIZE - sizeof(uint64_t),
               "RENDERER", "push data too large (8 bytes reserved for the mesh vertex address)");

        DrawItem item{mesh, first_index, index_count, 0, {}, (uint32_t)(push_size ? push_size : sizeof(uint64_t)) };
        if (push && push_size) memcpy(item.push_data, push, push_size);
        get_bucket(context, pipeline).items.push_back(item);
	}

	void vulkan_begin_pass(VulkanContext& context, PassData pass) {
	    context.passes.push_back(pass);
		context.current_pass_index = (int32_t)context.passes.size() - 1;
	}

	void vulkan_end_pass(VulkanContext& context) {
	    context.current_pass_index = -1;
	}

	Pipeline vulkan_pipeline_add(VulkanContext &context, const PipelineContext &pipeline) {
	    uint32_t index;

		if (!context.pipeline_free_list.empty()) {
		    index = context.pipeline_free_list.back();
		    context.pipeline_free_list.pop_back();
		    context.pipelines[index] = pipeline;
		} else {
		    index = static_cast<uint32_t>(context.pipelines.size());
		    context.pipelines.push_back(pipeline);
		    context.pipeline_generations.push_back(0);
		}

		return { index, context.pipeline_generations[index] };
	}

	PipelineContext& vulkan_pipeline_get(VulkanContext& context, Pipeline handle) {
		NEVAREA_ASSERT(handle.id < context.pipelines.size(), "PIPELINE", "Pipeline out of range!");
		NEVAREA_ASSERT(handle.generation == context.pipeline_generations[handle.id], "PIPELINE", "Stale Pipeline (generation mismatch)!");
		return context.pipelines[handle.id];
	}

	void vulkan_pipeline_remove(VulkanContext& context, Pipeline handle) {
		PipelineContext& pipeline = vulkan_pipeline_get(context, handle);
		vulkan_pipeline_destroy(pipeline, context.device.device, context.frame_sync);
		context.pipelines[handle.id] = {};
		context.pipeline_generations[handle.id]++;
		context.pipeline_free_list.push_back(handle.id);
	}
}
