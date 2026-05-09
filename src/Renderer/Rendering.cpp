#include "Rendering.hpp"
#pragma once

namespace Nevarea {
	struct RenderState {
		void* context;
		RenderingAPI api = RenderingAPI::NONE;
	};

	RenderContext Nevarea::renderer_create(RenderingAPI api)
	{
		RenderState* render_context{};

		return render_context;
	}

	void renderer_destroy(RenderContext renderer)
	{

	}

	void renderer_hook_window(RenderContext renderer, WindowHandle window)
	{

	}

	void renderer_draw(RenderContext renderer)
	{

	}
}