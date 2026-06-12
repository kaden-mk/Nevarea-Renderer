#include "RenderState.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"

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

		static VkFormat to_vk_format(Format format) {
			switch (format) {
				case Format::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
				case Format::RGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
			}
			return VK_FORMAT_UNDEFINED;
		}

		static VkImageUsageFlags to_vk_image_usage(uint32_t usage) {
			VkImageUsageFlags flags = 0;
			if (usage & ImageUsage::STORAGE) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
			if (usage & ImageUsage::SAMPLED) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if (usage & ImageUsage::TRANSFER_SRC) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (usage & ImageUsage::TRANSFER_DST) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if (usage & ImageUsage::COLOR_TARGET) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			return flags;
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

				return Renderer::vulkan_pipeline_add(render_state->vulkan, pipeline);
			}

			case RenderingAPI::NONE: {
				return {};
			}
		}

		return {};
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

				return Renderer::vulkan_pipeline_add(render_state->vulkan, pipeline);
			}

			case RenderingAPI::NONE: {
				return {};
			}
		}

		return {};
	}

	void renderer_destroy_pipeline(RenderContext context, PipelineHandle pipeline) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				auto& vulkan_renderer = render_state->vulkan;
				Renderer::vulkan_pipeline_remove(vulkan_renderer, pipeline);
				break;
			}

			case RenderingAPI::NONE:
				break;
		}
	}

	Image renderer_create_image(RenderContext context, const ImageDescription& description) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				VkExtent2D extent = { description.width, description.height };
				Renderer::ImageHandle handle = Renderer::vulkan_create_image(render_state->vulkan.resource_manager, extent, to_vk_format(description.format), to_vk_image_usage(description.usage));

				return { handle.index, handle.generation };
			}

			case RenderingAPI::NONE:
				break;
		}

		return {};
	}

	void renderer_destroy_image(RenderContext context, Image handle) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				Renderer::vulkan_destroy_image(render_state->vulkan.resource_manager, { handle.id, handle.generation }, render_state->vulkan.frame_sync);
				break;
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

		return {};
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

	void renderer_dispatch_compute(RenderContext context, PipelineHandle pipeline, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z, uint64_t buffer_address, Image target_image) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
    		case RenderingAPI::VULKAN:
    			render_state->vulkan.compute_dispatches.push_back({ pipeline, groups_x, groups_y, groups_z, { buffer_address, target_image.id } });
    			break;

    		case RenderingAPI::NONE:
    			break;
		}
	}

	void renderer_present_image(RenderContext context, Image handle) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				render_state->vulkan.present_target = { handle.id, handle.generation };
				break;

			case RenderingAPI::NONE:
				break;
		}
	}

	Buffer renderer_create_buffer(RenderContext context, size_t size, const char* debug_name) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                Renderer::BufferDescription description{};
                description.size = size;
                description.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                description.memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                description.name = debug_name;

                Renderer::BufferHandle handle = Renderer::vulkan_create_buffer(render_state->vulkan.resource_manager, description);
                return { handle.index, handle.generation };
            }

            case RenderingAPI::NONE:
                break;
        }

        return { UINT32_MAX, 0 };
    }

    void renderer_destroy_buffer(RenderContext context, Buffer handle) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN:
                Renderer::vulkan_destroy_buffer(render_state->vulkan.resource_manager, { handle.id, handle.generation });
                break;

            case RenderingAPI::NONE:
                break;
        }
    }

    void renderer_update_buffer(RenderContext context, Buffer handle, const void* data, size_t size) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                auto& manager = render_state->vulkan.resource_manager;

                NEVAREA_ASSERT(handle.id < manager.buffer_pool.size(), "RESOURCE MANAGER", "Buffer handle out of range!");
                NEVAREA_ASSERT(handle.generation == manager.generation_pool[handle.id], "RESOURCE MANAGER", "Stale Buffer handle!");

                VmaAllocationInfo alloc_info{};
                vmaGetAllocationInfo(manager.allocator, manager.allocation_pool[handle.id], &alloc_info);
                NEVAREA_ASSERT(size <= alloc_info.size, "RESOURCE MANAGER", "renderer_update_buffer: size exceeds buffer allocation!");

                void* mapped_memory = nullptr;
                VK_ASSERT(vmaMapMemory(manager.allocator, manager.allocation_pool[handle.id], &mapped_memory));
                memcpy(mapped_memory, data, size);
                vmaUnmapMemory(manager.allocator, manager.allocation_pool[handle.id]);
                break;
            }

            case RenderingAPI::NONE:
                break;
        }
    }

    uint64_t renderer_get_buffer_address(RenderContext context, Buffer handle) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN:
                return Renderer::vulkan_get_buffer_address(render_state->vulkan.resource_manager, { handle.id, handle.generation });

                case RenderingAPI::NONE:
                break;
        }

        return 0;
    }
}
