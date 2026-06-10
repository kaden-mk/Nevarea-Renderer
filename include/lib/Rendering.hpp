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

	enum class Format : uint32_t {
		RGBA8_UNORM,
		RGBA16_SFLOAT,
	};

	namespace ImageUsage {
		enum : uint32_t {
			STORAGE = 1u << 0,
			SAMPLED = 1u << 1,
			TRANSFER_SRC = 1u << 2,
			TRANSFER_DST = 1u << 3,
			COLOR_TARGET = 1u << 4,
		};
	}

	struct ImageDescription {
		uint32_t width = 0;
		uint32_t height = 0;
		Format format = Format::RGBA16_SFLOAT;
		uint32_t usage = 0;
	};

	struct Image {
		uint32_t id = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return id != UINT32_MAX; }
	};

	struct Vertex {
		float pos[2];
	};

	struct Mesh {
		uint32_t id = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return id != UINT32_MAX; }
	};

	struct PipelineHandle {
		uint32_t id = UINT32_MAX;
		bool is_valid() const { return id != UINT32_MAX; }
	};

	struct Buffer {
        uint32_t id = UINT32_MAX;
        uint32_t generation = 0;
        bool is_valid() const { return id != UINT32_MAX; }
    };

	struct RendererCapabilities {
		bool memory_priority = false;
		bool pageable_memory = false;

		bool present_id = false;
		bool present_wait = false;
		bool swapchain_maintenance1 = false;

		bool descriptor_buffer = false;
		bool descriptor_heap = false;
		bool mutable_descriptor_type = false;
		bool shader_object = false;
		bool extended_dynamic_state3 = false;

		bool calibrated_timestamps = false;
		bool shader_module_identifier = false;

		bool ray_query = false;
		bool ray_tracing_pipeline = false;
		bool acceleration_structure = false;
		bool ray_tracing_position_fetch = false;
		bool opacity_micromap = false;

		bool mesh_shader = false;
		bool variable_rate_shading = false;
		bool cooperative_matrix = false;
		bool device_generated_commands = false;
	};

	RenderContext renderer_create(RenderingAPI api);
	void renderer_destroy(RenderContext renderer);

	void renderer_hook_window(RenderContext renderer, WindowHandle window);
	void renderer_draw(RenderContext renderer);

	const RendererCapabilities& renderer_get_capabilities(RenderContext renderer);

	PipelineHandle renderer_create_pipeline(RenderContext renderer, const char* vert, const char* frag);
	PipelineHandle renderer_create_compute_pipeline(RenderContext renderer, const char* compute);
	void renderer_destroy_pipeline(RenderContext renderer, PipelineHandle pipeline);

	Image renderer_create_image(RenderContext renderer, const ImageDescription& description);
	void renderer_destroy_image(RenderContext renderer, Image handle);

	Mesh renderer_create_mesh(RenderContext renderer, Vertex* vertices, uint32_t count);
	void renderer_destroy_mesh(RenderContext renderer, Mesh handle);

	void renderer_submit_mesh(RenderContext renderer, Mesh mesh, PipelineHandle pipeline);
	void renderer_dispatch_compute(RenderContext renderer, PipelineHandle pipeline, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z, uint64_t buffer_address = 0, Image target_image = {});
	void renderer_present_image(RenderContext renderer, Image handle);

	Buffer renderer_create_buffer(RenderContext context, size_t size, const char* debug_name = "nevarea_buffer");
    void renderer_destroy_buffer(RenderContext context, Buffer handle);
    void renderer_update_buffer(RenderContext context, Buffer handle, const void* data, size_t size);
    uint64_t renderer_get_buffer_address(RenderContext context, Buffer handle);
}
