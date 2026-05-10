#pragma once

#include "RenderState.hpp"
#include "Core/InternalState.hpp"
#include "Core/n_pch.hpp"

namespace Nevarea {
	namespace {
		constexpr uint32_t MAX_RENDERERS = 4;
		RenderState g_renderers[MAX_RENDERERS];

		RenderState* resolve(RenderContext context) {
			uint32_t id = static_cast<uint32_t>(context);
			NEVAREA_ASSERT(id != 0 && id <= MAX_RENDERERS, "RENDERER", "Invalid RenderContext!");

			RenderState* render_state = &g_renderers[id - 1];
			NEVAREA_ASSERT(render_state->is_active, "RENDERER", "RenderContext refers to a destroyed renderer!");

			return render_state;
		}
	}

	RenderContext renderer_create(RenderingAPI api)
	{
		for (uint32_t i = 0; i < MAX_RENDERERS; ++i) {
			if (g_renderers[i].is_active) continue;

			RenderState& render_state = g_renderers[i];
			render_state.is_active = true;
			render_state.api = api;
			
			switch (api) {
				case RenderingAPI::VULKAN:
					new (&render_state.vulkan) Renderer::VulkanContext {};
					break;

				case RenderingAPI::NONE:
			 		break;
			}

			return static_cast<RenderContext>(i + 1);
		}

		NEVAREA_ASSERT(false, "RENDERER", "MAX_RENDERERS exceeded!");
		return RenderContext::INVALID;
	}

	void renderer_destroy(RenderContext context)
	{
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				Renderer::vulkan_context_destroy(render_state->vulkan);
				break;

			case RenderingAPI::NONE:
				break;
		}

		render_state->is_active = false;
	}

	void renderer_hook_window(RenderContext context, WindowHandle window)
	{
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				Renderer::vulkan_context_init(render_state->vulkan, window);
				break;

			case RenderingAPI::NONE:
				break;
		}
	}

	void renderer_draw(RenderContext context)
	{
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				Renderer::vulkan_context_draw(render_state->vulkan);
				break;

			case RenderingAPI::NONE:
				break;
		}
	}
}