#pragma once

#include "Core/n_pch.hpp"
#include <vk_mem_alloc.h>

// For now its gonna be standalone, obviously this should be for every renderer but well.. its only vulkan for now so
namespace Nevarea::Renderer {
	struct BufferHandle {
		uint32_t index;
		uint32_t generation;
	};

	struct BufferDescription {
		VkDeviceSize size;
		VkBufferUsageFlags usage;        
		VmaMemoryUsage memory_usage;
	};

	struct ResourceManager {
		std::vector<VkBuffer> buffer_pool;
		std::vector<VmaAllocation> allocation_pool;
		std::vector<uint32_t> generation_pool;
		std::vector<uint32_t> free_list;

		VmaAllocator allocator;
	};

	void vulkan_resources_init(ResourceManager& manager, VmaAllocator allocator);
	void vulkan_resources_destroy(ResourceManager& manager);

	BufferHandle vulkan_create_buffer(ResourceManager& manager, const BufferDescription& buffer_description);
	VkBuffer vulkan_get_buffer(const ResourceManager& manager, BufferHandle handle);
	void vulkan_destroy_buffer(ResourceManager&, BufferHandle handle);
}