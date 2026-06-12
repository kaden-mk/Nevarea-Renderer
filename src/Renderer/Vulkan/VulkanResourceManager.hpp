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
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	struct BufferHandle {
		uint32_t index = UINT32_MAX;
		uint32_t generation = 0;
		bool is_valid() const { return index != UINT32_MAX; }
	};

	struct MeshData {
		BufferHandle vertex_buffer;
		uint32_t vertex_count;
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

		VkDevice device;
	};

	void vulkan_resources_init(ResourceManager& manager, VmaAllocator allocator, VkDevice device);
	void vulkan_resources_destroy(ResourceManager& manager);

	void vulkan_resources_push_deletor(DeletionQueue& deletion_queue, std::function<void()>&& fn);
	void vulkan_resources_flush_deletors(DeletionQueue& deletion_queue);

	BufferHandle vulkan_create_buffer(ResourceManager& manager, const BufferDescription& buffer_description);
	VkBuffer vulkan_get_buffer(const ResourceManager& manager, BufferHandle handle);
	uint64_t vulkan_get_buffer_address(const ResourceManager& manager, BufferHandle handle);
	void vulkan_destroy_buffer(ResourceManager&, BufferHandle handle);

	ImageHandle vulkan_create_image(ResourceManager& manager, const ImageDescription& description);
	AllocatedImage& vulkan_get_image(ResourceManager& manager, ImageHandle handle);
	void vulkan_destroy_image(ResourceManager& manager, ImageHandle handle, FrameContext& frame);

	Mesh vulkan_create_mesh(ResourceManager& manager, Vertex* vertices, uint32_t count);
	void vulkan_destroy_mesh(ResourceManager& manager, Mesh handle, FrameContext& frame);

	Sampler vulkan_create_sampler(ResourceManager& manager, const SamplerDescription& description);
	void vulkan_destroy_sampler(ResourceManager& manager, Sampler sampler, FrameContext& frame);
}
