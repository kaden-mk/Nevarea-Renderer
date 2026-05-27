#pragma once

#include "Core/n_pch.hpp"
#include "lib/Rendering.hpp"

#include <vk_mem_alloc.h>
#include "VulkanSwapchain.hpp"

// For now its gonna be standalone, obviously this should be for every renderer but well.. its only vulkan for now so
namespace Nevarea::Renderer {
	#define NEVAREA_BUFFER_IMAGE_SIZE 1000
	#define NEVAREA_BUFFER_STORAGE_SIZE 1000

	struct BufferHandle {
		uint32_t index;
		uint32_t generation;
	};

	struct BufferDescription {
		VkDeviceSize size;
		VkBufferUsageFlags usage;        
		VmaMemoryUsage memory_usage;
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

	Mesh vulkan_create_mesh(ResourceManager& manager, Vertex* vertices, uint32_t count);
	void vulkan_destroy_mesh(ResourceManager& manager, Mesh handle, FrameContext& frame);
}