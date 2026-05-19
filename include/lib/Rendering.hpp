#pragma once

#include "WindowSystem.hpp"

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

	RenderContext renderer_create(RenderingAPI api);
	void renderer_destroy(RenderContext renderer);

	void renderer_hook_window(RenderContext renderer, WindowHandle window);
	void renderer_draw(RenderContext renderer);

	Mesh renderer_create_mesh(RenderContext renderer, Vertex* vertices, uint32_t count);
	void renderer_destroy_mesh(RenderContext renderer, Mesh handle);
	void renderer_submit_mesh(RenderContext renderer, Mesh mesh);
}