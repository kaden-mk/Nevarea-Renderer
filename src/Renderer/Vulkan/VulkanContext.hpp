#pragma once

#include "Platform/WindowSystemInternal.hpp"
#include "Core/n_pch.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanResourceManager.hpp"
#include "VulkanPipeline.hpp"

#include <vk_mem_alloc.h>

namespace Nevarea::Renderer {
    struct MeshPush { uint64_t vertex_buffer_address; }; // temporary for now

	struct DrawBucket {
		PipelineHandle pipeline;
		std::vector<Mesh> meshes;
	};

	struct ComputeDispatch {
		PipelineHandle pipeline;
		uint32_t groups_x, groups_y, groups_z;

		uint8_t push_data[NEVAREA_MAX_PUSH_CONSTANTS_SIZE];
		uint32_t push_size;
		ImageHandle target_image = { UINT32_MAX, 0 };
	};

	struct VulkanContext {
		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		VmaAllocator allocator;

		ResourceManager resource_manager;
		WindowHandle window;
		DeviceContext device;
		SurfaceContext surface;
		SwapchainContext swapchain;
		FrameContext frame_sync;
		ImageHandle present_target = { UINT32_MAX, 0 };

		std::vector<PipelineContext> pipelines;
		std::vector<uint32_t> pipeline_generations;
		std::vector<uint32_t> pipeline_free_list;

		std::vector<DrawBucket> draw_buckets;
		std::vector<ComputeDispatch> compute_dispatches;
	};

	void vulkan_context_init(VulkanContext& context, WindowHandle window);
	void vulkan_context_draw(VulkanContext& context);
	void vulkan_context_destroy(VulkanContext& context);

	void vulkan_submit_mesh(VulkanContext& context, Mesh mesh, PipelineHandle pipeline);

	PipelineHandle vulkan_pipeline_add(VulkanContext& context, const PipelineContext& pipeline);
	PipelineContext& vulkan_pipeline_get(VulkanContext& context, PipelineHandle handle);
	void vulkan_pipeline_remove(VulkanContext& context, PipelineHandle handle);
}
