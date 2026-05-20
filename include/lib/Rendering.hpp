#pragma once

#include "WindowSystem.hpp"

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

namespace Nevarea {
	enum class RenderingAPI {
		NONE,
		VULKAN
	};

	enum class RenderContext : uint32_t { INVALID = 0 };
	enum class SwapchainHandle : uint32_t { INVALID = 0 };

	struct Vertex {
		float pos[2];
	};

	struct Mesh {
		uint32_t id;
		uint32_t generation;
	};

	struct PipelineHandle {
		uint32_t id;
	};

	RenderContext renderer_create(RenderingAPI api);
	void renderer_destroy(RenderContext renderer);

	void renderer_hook_window(RenderContext renderer, WindowHandle window);
	void renderer_draw(RenderContext renderer);

	PipelineHandle renderer_create_pipeline(RenderContext renderer, const char* vert, const char* frag);
	PipelineHandle renderer_create_compute_pipeline(RenderContext renderer, const char* compute);
	void renderer_destroy_pipeline(RenderContext renderer, PipelineHandle pipeline);

	Mesh renderer_create_mesh(RenderContext renderer, Vertex* vertices, uint32_t count);
	void renderer_destroy_mesh(RenderContext renderer, Mesh handle);

	void renderer_submit_mesh(RenderContext renderer, Mesh mesh, PipelineHandle pipeline);
	void renderer_dispatch_compute(RenderContext renderer, PipelineHandle pipeline, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z, uint64_t buffer_address = 0);
}