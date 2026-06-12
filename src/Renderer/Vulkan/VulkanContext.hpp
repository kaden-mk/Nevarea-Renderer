#pragma once

#include "Platform/WindowSystemInternal.hpp"
#include "Core/n_pch.hpp"

#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanResourceManager.hpp"
#include "VulkanPipeline.hpp"

#include <vk_mem_alloc.h>

namespace Nevarea::Renderer {
	// ill keep this here for now
	struct DrawCall {
		Mesh mesh;
		PipelineHandle pipeline;
	};

	struct ComputeDispatch {
		PipelineHandle pipeline;
		uint32_t groups_x, groups_y, groups_z;
		PushConstants push;
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

		std::vector<DrawCall> draw_list;
		std::vector<ComputeDispatch> compute_dispatches;
	};

	void vulkan_context_init(VulkanContext& context, WindowHandle window);
	void vulkan_context_draw(VulkanContext& context);
	void vulkan_context_destroy(VulkanContext& context);

	PipelineHandle vulkan_pipeline_add(VulkanContext& context, const PipelineContext& pipeline);
	PipelineContext& vulkan_pipeline_get(VulkanContext& context, PipelineHandle handle);
	void vulkan_pipeline_remove(VulkanContext& context, PipelineHandle handle);
}
