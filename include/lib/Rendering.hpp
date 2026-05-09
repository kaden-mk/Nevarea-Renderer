#pragma once

#include "WindowSystem.hpp"

namespace Nevarea {
	enum class RenderingAPI {
		NONE,
		VULKAN
	};

	enum class RenderContext : uint32_t { INVALID = 0 };
	enum class SwapchainHandle : uint32_t { INVALID = 0 };

	RenderContext renderer_create(RenderingAPI api);
	void renderer_destroy(RenderContext renderer);

	void renderer_hook_window(RenderContext renderer, WindowHandle window);
	void renderer_draw(RenderContext renderer);
}