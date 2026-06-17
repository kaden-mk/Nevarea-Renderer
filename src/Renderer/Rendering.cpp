#include "lib/Rendering.hpp"
#include "RenderState.hpp"
#include "Renderer/Vulkan/VulkanContext.hpp"
#include "Renderer/Vulkan/VulkanFrames.hpp"
#include "Renderer/Vulkan/VulkanResourceManager.hpp"
#include "Renderer/Vulkan/VulkanTranslate.hpp"
#include "lib/Core.hpp"
#include "lib/WindowSystem.hpp"

namespace Nevarea {
	namespace {
		constexpr uint32_t MAX_RENDERERS = 4;
		RenderState g_renderers[MAX_RENDERERS];
	}

	RenderState* resolve(RenderContext context) {
		uint32_t id = static_cast<uint32_t>(context);
		NEVAREA_ASSERT(id != 0 && id <= MAX_RENDERERS, "RENDERER", "Invalid RenderContext!");

		RenderState* render_state = &g_renderers[id - 1];
		NEVAREA_ASSERT(render_state->is_active, "RENDERER", "RenderContext refers to a destroyed renderer!");

		return render_state;
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

	NvWinExtent renderer_get_swapchain_extent(RenderContext context) {
	    RenderState* render_state = resolve(context);

		switch (render_state->api) {
		    case RenderingAPI::VULKAN: {
				return { render_state->vulkan.swapchain.extent.width, render_state->vulkan.swapchain.extent.height };
			}

			case RenderingAPI::NONE: break;
		}

		return { 0, 0 };
	}

	void renderer_begin_pass(RenderContext context, const RenderPassDescription& description) {
	    RenderState* render_state = resolve(context);

		switch (render_state->api) {
		    case RenderingAPI::VULKAN: {
				Renderer::PassData pass{};
				pass.present = description.present;
				pass.depth = description.depth;
				pass.color.assign(description.color, description.color + description.color_count);
				Renderer::vulkan_begin_pass(render_state->vulkan, pass);
				break;
			}

			case RenderingAPI::NONE: break;
		}
	}

	void renderer_end_pass(RenderContext context) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                Renderer::vulkan_end_pass(render_state->vulkan);
                break;
            }

            case RenderingAPI::NONE: break;
        }
	}

	void renderer_request_device_extensions(RenderContext context, const char* const* names, uint32_t count) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                for (size_t i = 0; i < (size_t)count; i++)
                    render_state->vulkan.device.requested_extensions.push_back(names[i]);

                break;
            }

            case RenderingAPI::NONE: break;
        }
	}

	bool renderer_extension_supported(RenderContext renderer, const char* name) {
        RenderState* render_state = resolve(renderer);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                VkPhysicalDevice device = render_state->vulkan.device.physical_device;

                uint32_t count = 0;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
                std::vector<VkExtensionProperties> props(count);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &count, props.data());

                for (const VkExtensionProperties& extension : props) if (strcmp(extension.extensionName, name) == 0) return true;

                return false;
            }

            case RenderingAPI::NONE: break;
        }

        return false;
	}

	bool renderer_extension_enabled(RenderContext renderer, const char* name) {
        RenderState* render_state = resolve(renderer);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                for (const char* extension : render_state->vulkan.device.enabled_extensions)
                    if (strcmp(extension, name) == 0) return true;

                return false;
            }

            case RenderingAPI::NONE: break;
        }

        return false;
	}

	bool renderer_swapchain_resized(RenderContext context) {
	    RenderState* render_state = resolve(context);

		switch (render_state->api) {
		    case RenderingAPI::VULKAN: {
				bool was = render_state->vulkan.swapchain.resized;
				render_state->vulkan.swapchain.resized = false;
				return was;
			}

			case RenderingAPI::NONE: break;
		}

		return false;
	}

	void renderer_set_present_mode(RenderContext context, PresentMode mode) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				auto& vk = render_state->vulkan;
				vk.swapchain.desired_present_mode = Renderer::to_vk_present_mode(mode);
				Renderer::recreate_swapchain(vk.swapchain, vk.device, vk.surface, vk.window);
				Renderer::vulkan_frame_sync_ensure_present_semaphores(vk.frame_sync, vk.device.device, static_cast<uint32_t>(vk.swapchain.images.size()));
				break;
			}
			case RenderingAPI::NONE:
				break;
		}
	}

	Pipeline renderer_create_compute_pipeline(RenderContext context, const char* compute) {
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

	Pipeline renderer_create_pipeline(RenderContext context, const PipelineDescription& description) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				Renderer::PipelineContext pipeline;

				Renderer::vulkan_pipeline_init(pipeline,
					render_state->vulkan.device.device,
					render_state->vulkan.swapchain.image_format,
					render_state->vulkan.resource_manager.descriptor_layout,
					description);

				return Renderer::vulkan_pipeline_add(render_state->vulkan, pipeline);
			}

			case RenderingAPI::NONE: {
				return {};
			}
		}

		return {};
	}

	void renderer_destroy_pipeline(RenderContext context, Pipeline pipeline) {
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
				Renderer::ImageHandle handle = Renderer::vulkan_create_image(render_state->vulkan.resource_manager, description);
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

	Mesh renderer_create_mesh(RenderContext context, const void* vertex_data, uint32_t vertex_count, const VertexLayout& layout, const uint32_t* indices, uint32_t index_count) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: {
				return Renderer::vulkan_create_mesh(render_state->vulkan.resource_manager, vertex_data, vertex_count, layout, index_count, indices);
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

	Sampler renderer_create_sampler(RenderContext context, const SamplerDescription& description) {
    	RenderState* render_state = resolve(context);

    	switch (render_state->api) {
            case RenderingAPI::VULKAN: return Renderer::vulkan_create_sampler(render_state->vulkan.resource_manager, description);
            case RenderingAPI::NONE: break;
    	}

        return {};
	}

	void renderer_destroy_sampler(RenderContext context, Sampler sampler) {
	    RenderState* render_state = resolve(context);

		switch (render_state->api) {
		    case RenderingAPI::VULKAN: Renderer::vulkan_destroy_sampler(render_state->vulkan.resource_manager, sampler, render_state->vulkan.frame_sync);
			case RenderingAPI::NONE: break;
		};
	}

	void renderer_submit_mesh(RenderContext context, Mesh mesh, Pipeline pipeline, const void* push, size_t push_size) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN:
				Renderer::vulkan_submit_mesh(render_state->vulkan, mesh, pipeline, push, push_size);
				break;

			case RenderingAPI::NONE:
				break;
		}
	}

	void renderer_submit_mesh_range(RenderContext context, Mesh mesh, uint32_t first_index, uint32_t index_count, Pipeline pipeline, const void* push, size_t push_size) {
    	RenderState* render_state = resolve(context);

    	switch (render_state->api) {
    		case RenderingAPI::VULKAN:
    			Renderer::vulkan_submit_mesh_range(render_state->vulkan, mesh, first_index, index_count, pipeline, push, push_size);
    			break;

    		case RenderingAPI::NONE:
    			break;
    	}
	}

	void renderer_dispatch_compute(RenderContext context, Pipeline pipeline, uint32_t groups_x, uint32_t groups_y, uint32_t groups_z, const void* push, size_t size, Image storage_target) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
    		case RenderingAPI::VULKAN: {
                Renderer::ComputeDispatch dispatch{ pipeline, groups_x, groups_y, groups_z };
                NEVAREA_ASSERT(size <= sizeof(dispatch.push_data), "RENDERER", "push data exceeds 128 bytes");
                memcpy(dispatch.push_data, push, size);
                dispatch.push_size = (uint32_t)size;
                dispatch.target_image = { storage_target.id, storage_target.generation };
                render_state->vulkan.compute_dispatches.push_back(dispatch);

   			    break;
            }

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

	void renderer_upload_image(RenderContext context, Image handle, const void* pixels, size_t size) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
                Renderer::vulkan_upload_image(render_state->vulkan.resource_manager, { handle.id, handle.generation }, pixels, size);
                break;
            }

            case RenderingAPI::NONE:
                break;
        }
	}

	Buffer renderer_create_buffer(RenderContext context, const BufferDescription& description) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case RenderingAPI::VULKAN: {
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

                VkMemoryPropertyFlags mem_flags;
                vmaGetAllocationMemoryProperties(manager.allocator, manager.allocation_pool[handle.id], &mem_flags);

                if (mem_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                    void* mapped_memory = nullptr;
                    VK_ASSERT(vmaMapMemory(manager.allocator, manager.allocation_pool[handle.id], &mapped_memory));
                    memcpy(mapped_memory, data, size);
                    vmaUnmapMemory(manager.allocator, manager.allocation_pool[handle.id]);
                } else
                    Renderer::vulkan_upload_buffer(manager, { handle.id, handle.generation }, data, size);

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

   	bool renderer_device_lost(RenderContext context) {
		RenderState* render_state = resolve(context);

		switch (render_state->api) {
			case RenderingAPI::VULKAN: return render_state->vulkan.device.device_lost;
			case RenderingAPI::NONE: break;
		}

		return false;
	}

	uint64_t renderer_last_present_id(RenderContext context) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case Nevarea::RenderingAPI::VULKAN: return render_state->vulkan.frame_sync.present_id;
            case Nevarea::RenderingAPI::NONE: break;
        }

        return 0;
	}

	bool renderer_wait_for_present(RenderContext context, uint64_t present_id, uint64_t timeout_ns) {
        RenderState* render_state = resolve(context);

        switch (render_state->api) {
            case Nevarea::RenderingAPI::VULKAN: {
                auto& vk = render_state->vulkan;

                return Renderer::vulkan_wait_for_present(vk.device, vk.swapchain, present_id, timeout_ns);
            }
            case Nevarea::RenderingAPI::NONE: break;
        }

        return false;
	}
}
