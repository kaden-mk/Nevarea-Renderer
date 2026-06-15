#pragma once

#include "Core/n_pch.hpp"
#include "lib/Rendering.hpp"

#include <vk_mem_alloc.h>
#include "VulkanSwapchain.hpp"

// For now its gonna be standalone, obviously this should be for every renderer but well.. its only vulkan for now so
namespace Nevarea::Renderer {
	#define NEVAREA_BUFFER_IMAGE_SIZE 1000
	#define NEVAREA_BUFFER_STORAGE_SIZE 1000
	#define NEVAREA_SAMPLER_SIZE 64

	struct ImageHandle {
		uint32_t index = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return index != UINT32_MAX; }
	};

	struct AllocatedImage {
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkExtent2D extent = {};
		Format format = Format::COUNT;
		VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageUsageFlags usage = 0;
	};

	struct BufferHandle {
		uint32_t index = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return index != UINT32_MAX; }
	};

	struct MeshData {
		BufferHandle vertex_buffer;
		uint64_t vertex_address;
		uint32_t vertex_count;

		BufferHandle index_buffer;
		uint32_t index_count = 0;

		uint32_t stride;
	};

	struct FormatInfo {
		VkFormat vk;
		uint32_t block_bytes;
		uint32_t block_width;
		uint32_t block_height;
		VkImageAspectFlags aspect;
	};

	static const FormatInfo k_format_info[] = {
	    { VK_FORMAT_R8_UNORM, 1, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R8G8_UNORM, 2, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R8G8B8A8_UNORM, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R8G8B8A8_SRGB, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_B8G8R8A8_UNORM, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_B8G8R8A8_SRGB, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R16_SFLOAT, 2, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R16G16_SFLOAT, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R16G16B16A16_SFLOAT, 8, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R32_SFLOAT, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R32G32_SFLOAT, 8, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R32G32B32A32_SFLOAT, 16, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_A2B10G10R10_UNORM_PACK32, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_B10G11R11_UFLOAT_PACK32, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_R32_UINT, 4, 1, 1, VK_IMAGE_ASPECT_COLOR_BIT },
		{ VK_FORMAT_D16_UNORM, 2, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT },
		{ VK_FORMAT_D32_SFLOAT, 4, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT },
		{ VK_FORMAT_D24_UNORM_S8_UINT, 4, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT },
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, 8, 1, 1, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT },
	};

	struct ResourceManager {
		std::vector<VkBuffer> buffer_pool;
		std::vector<VmaAllocation> allocation_pool;
		std::vector<uint32_t> generation_pool;
		std::vector<uint32_t> free_list;

		std::vector<MeshData> mesh_pool;
		std::vector<uint32_t> mesh_generation_pool;
		std::vector<uint32_t> mesh_free_list;

		std::vector<AllocatedImage> image_pool;
		std::vector<uint32_t> image_generation_pool;
		std::vector<uint32_t> image_free_list;

		std::vector<VkSampler> sampler_pool;
		std::vector<uint32_t>  sampler_generation_pool;
        std::vector<uint32_t>  sampler_free_list;

		VmaAllocator allocator;

		VkDescriptorSet descriptor_set;
		VkDescriptorSetLayout descriptor_layout;
		VkDescriptorPool descriptor_pool;

		VkCommandPool upload_pool;
		VkCommandBuffer upload_cmd;
		VkFence upload_fence;
		VkQueue upload_queue;

		VkDevice device;
	};

	void vulkan_resources_init(ResourceManager& manager, VmaAllocator allocator, VkDevice device, VkQueue graphics_queue, uint32_t graphics_family_index);
	void vulkan_resources_destroy(ResourceManager& manager);

	void vulkan_immediate_submit(ResourceManager& manager, std::function<void(VkCommandBuffer)>&& record);

	void vulkan_resources_push_deletor(DeletionQueue& deletion_queue, std::function<void()>&& fn);
	void vulkan_resources_flush_deletors(DeletionQueue& deletion_queue);

	BufferHandle vulkan_create_buffer(ResourceManager& manager, const BufferDescription& buffer_description);
	VkBuffer vulkan_get_buffer(const ResourceManager& manager, BufferHandle handle);
	uint64_t vulkan_get_buffer_address(const ResourceManager& manager, BufferHandle handle);
	void vulkan_upload_buffer(ResourceManager& manager, BufferHandle dst, const void* data, size_t size);
	void vulkan_destroy_buffer(ResourceManager&, BufferHandle handle);

	ImageHandle vulkan_create_image(ResourceManager& manager, const ImageDescription& description);
	AllocatedImage& vulkan_get_image(ResourceManager& manager, ImageHandle handle);
	void vulkan_upload_image(ResourceManager& manager, ImageHandle handle, const void* pixels, size_t size);
	void vulkan_destroy_image(ResourceManager& manager, ImageHandle handle, FrameContext& frame);

	Mesh vulkan_create_mesh(ResourceManager& manager, const void* vertex_data, uint32_t vertex_count, const VertexLayout& layout, uint32_t index_count, const uint32_t* indices);
	void vulkan_destroy_mesh(ResourceManager& manager, Mesh handle, FrameContext& frame);

	Sampler vulkan_create_sampler(ResourceManager& manager, const SamplerDescription& description);
	void vulkan_destroy_sampler(ResourceManager& manager, Sampler sampler, FrameContext& frame);
}
