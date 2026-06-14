#pragma once

#include "WindowSystem.hpp"
#include "lib/Core.hpp"

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
		float priority = 0.5f;
	};

	struct Image {
		uint32_t id = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return id != UINT32_MAX; }
	};

	struct Mesh {
		uint32_t id = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return id != UINT32_MAX; }
	};

	struct PipelineHandle {
		uint32_t id = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return id != UINT32_MAX; }
	};

	struct Buffer {
        uint32_t id = UINT32_MAX;
        uint32_t generation = 0;
        bool is_valid() const { return id != UINT32_MAX; }
    };

   	namespace BufferUsage {
		enum : uint32_t {
			STORAGE = 1u << 0, UNIFORM = 1u << 1, INDEX = 1u << 2,
			INDIRECT = 1u << 3, TRANSFER_SRC = 1u << 4, TRANSFER_DST = 1u << 5,
		};
	};

	enum class MemoryLocation : uint32_t { GPU_ONLY, CPU_TO_GPU, GPU_TO_CPU };

	enum class PresentMode : uint32_t {
		VSYNC,
		MAILBOX,
		IMMEDIATE,
	};

	struct BufferDescription {
		size_t size = 0;
		uint32_t usage = 0;
		MemoryLocation memory = MemoryLocation::CPU_TO_GPU;
		const char* debug_name = "nevarea_buffer";
		float priority = 0.5f;
	};

	enum class Filter : uint32_t { NEAREST, LINEAR };
	enum class MipmapMode : uint32_t { NEAREST, LINEAR };
	enum class AddressMode : uint32_t { REPEAT, MIRRORED_REPEAT, CLAMP_EDGE, CLAMP_BORDER };
	enum class CompareOp : uint32_t { NONE, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL, EQUAL, ALWAYS };
	enum class BorderColor : uint32_t { TRANSPARENT_BLACK, OPAQUE_BLACK, OPAQUE_WHITE };

	enum class VertexFormat : uint32_t {
	    FLOAT, FLOAT2, FLOAT3, FLOAT4,
		UNORM8x4,
		UINT, UINT2, UINT4
	};

	struct VertexAttribute {
   	    uint32_t location;
        VertexFormat format;
        uint32_t offset;
   	};

    struct VertexLayout {
        const VertexAttribute* attributes;
        uint32_t attribute_count;
        uint32_t stride;
    };

	struct SamplerDescription {
        Filter min_filter = Filter::LINEAR;
        Filter mag_filter = Filter::LINEAR;
        MipmapMode  mipmap_mode = MipmapMode::LINEAR;
        AddressMode address_u = AddressMode::REPEAT;
        AddressMode address_v = AddressMode::REPEAT;
        AddressMode address_w = AddressMode::REPEAT;

        float mip_lod_bias = 0.0f;
        float min_lod = 0.0f;
        float max_lod = 1000.0f; // ~VK_LOD_CLAMP_NONE meaning that large lods wont get clamped
        float max_anisotropy = 1.0f;

        CompareOp compare_op = CompareOp::NONE;
        BorderColor border_color = BorderColor::OPAQUE_BLACK;
	};

	struct Sampler {
	    uint32_t id = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() { return id != UINT32_MAX; };
	};

	struct RendererCapabilities {
		bool memory_priority = false;
		bool pageable_memory = false;

		bool present_id = false;
		bool present_wait = false;
		bool present_id2 = false;
		bool present_wait2 = false;
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

	NEVAREA_API RenderContext renderer_create(RenderingAPI api);
	NEVAREA_API void renderer_destroy(RenderContext renderer);

	NEVAREA_API void renderer_hook_window(RenderContext renderer, WindowHandle window);
	NEVAREA_API void renderer_draw(RenderContext renderer);

	NEVAREA_API NvWinExtent renderer_get_swapchain_extent(RenderContext renderer);
	NEVAREA_API bool renderer_swapchain_resized(RenderContext renderer);
	NEVAREA_API void renderer_set_present_mode(RenderContext renderer, PresentMode mode);

	NEVAREA_API const RendererCapabilities& renderer_get_capabilities(RenderContext renderer);

	NEVAREA_API PipelineHandle renderer_create_pipeline(RenderContext renderer, const char* vert, const char* frag);
	NEVAREA_API PipelineHandle renderer_create_compute_pipeline(RenderContext renderer, const char* compute);
	NEVAREA_API void renderer_destroy_pipeline(RenderContext renderer, PipelineHandle pipeline);

	NEVAREA_API Image renderer_create_image(RenderContext renderer, const ImageDescription& description);
	NEVAREA_API void renderer_destroy_image(RenderContext renderer, Image handle);

	NEVAREA_API Mesh renderer_create_mesh(RenderContext renderer, const void* vertex_data, uint32_t vertex_count, const VertexLayout& layout, const uint32_t* indices = nullptr, uint32_t index_count = 0);
	NEVAREA_API void renderer_destroy_mesh(RenderContext renderer, Mesh handle);

	NEVAREA_API Sampler renderer_create_sampler(RenderContext renderer, const SamplerDescription& description);
	NEVAREA_API void renderer_destroy_sampler(RenderContext renderer, Sampler sampler);

	NEVAREA_API void renderer_submit_mesh(RenderContext renderer, Mesh mesh, PipelineHandle pipeline);
	NEVAREA_API void renderer_dispatch_compute(RenderContext renderer, PipelineHandle pipeline, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z, const void* push, size_t size, Image storage_target);
	NEVAREA_API void renderer_present_image(RenderContext renderer, Image handle);
	NEVAREA_API void renderer_upload_image(RenderContext renderer, Image handle, const void* pixels, size_t size);

	NEVAREA_API Buffer renderer_create_buffer(RenderContext context, const BufferDescription& description);
	NEVAREA_API void renderer_destroy_buffer(RenderContext context, Buffer handle);
	NEVAREA_API void renderer_update_buffer(RenderContext context, Buffer handle, const void* data, size_t size);
	NEVAREA_API uint64_t renderer_get_buffer_address(RenderContext context, Buffer handle);

	NEVAREA_API bool renderer_device_lost(RenderContext renderer);

	NEVAREA_API uint64_t renderer_last_present_id(RenderContext renderer);
	NEVAREA_API bool renderer_wait_for_present(RenderContext renderer, uint64_t present_id, uint64_t timeout_ns);
}
