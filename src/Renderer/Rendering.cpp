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
				render_state->vulkan.~VulkanContext();
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

	const RendererCapabilities& renderer_get_capabilities(RenderContext context) {
		static const RendererCapabilities empty{};
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				return render_state->vulkan.device.capabilities;

			case RenderingAPI::NONE:
				return empty;
		}
		return empty;
	}

	PipelineHandle renderer_create_compute_pipeline(RenderContext context, const char* compute) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				Renderer::PipelineContext pipeline;

				Renderer::vulkan_compute_pipeline_init(pipeline,
					render_state->vulkan.device.device,
					render_state->vulkan.resource_manager.descriptor_layout,
					compute
				);

				render_state->vulkan.pipelines.push_back(pipeline);
				return { static_cast<uint32_t>(render_state->vulkan.pipelines.size() - 1) };
			}

			case RenderingAPI::NONE: {
				return {};
			}
		}
	}

	PipelineHandle renderer_create_pipeline(RenderContext context, const char* vert, const char* frag) {
		RenderState* render_state = resolve(context);
		
		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				Renderer::PipelineContext pipeline;

				Renderer::vulkan_pipeline_init(pipeline,
					render_state->vulkan.device.device,
					render_state->vulkan.swapchain.image_format,
					render_state->vulkan.resource_manager.descriptor_layout,
					vert,
					frag
				);

				render_state->vulkan.pipelines.push_back(pipeline);
				return { static_cast<uint32_t>(render_state->vulkan.pipelines.size() - 1) };
			}

			case RenderingAPI::NONE: {
				return {};
			}
		}
	}

	void renderer_destroy_pipeline(RenderContext context, PipelineHandle pipeline) {
		RenderState* render_state = resolve(context);
		
		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				auto& vulkan_renderer = render_state->vulkan;
				Renderer::vulkan_pipeline_destroy(vulkan_renderer.pipelines[pipeline.id], vulkan_renderer.device.device, vulkan_renderer.frame_sync);
				break;
			}

			case RenderingAPI::NONE:
				break;
		}
	}

	Mesh renderer_create_mesh(RenderContext context, Vertex* vertices, uint32_t count) {
		RenderState* render_state = resolve(context);
		
		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				return Renderer::vulkan_create_mesh(render_state->vulkan.resource_manager, vertices, count);
			}

			case RenderingAPI::NONE: {
				return {};
			}
		}
	}

	void renderer_destroy_mesh(RenderContext context, Mesh handle) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				Renderer::vulkan_destroy_mesh(render_state->vulkan.resource_manager, handle, render_state->vulkan.frame_sync);
				break;

			case RenderingAPI::NONE:
				break;
		}
	}

	void renderer_submit_mesh(RenderContext context, Mesh mesh, PipelineHandle pipeline) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				render_state->vulkan.draw_list.push_back({ mesh, pipeline });
				break;

			case RenderingAPI::NONE:
				break;
		}
	}

	void renderer_dispatch_compute(RenderContext context, PipelineHandle pipeline, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z, uint64_t buffer_address) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				render_state->vulkan.compute_dispatches.push_back({ pipeline, groups_x, groups_y, groups_z, { buffer_address } });
				break;

			case RenderingAPI::NONE:
				break;
		}
	}
}