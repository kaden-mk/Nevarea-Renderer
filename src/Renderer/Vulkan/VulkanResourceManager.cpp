#include "VulkanResourceManager.hpp"

#include "lib/Core.hpp"

namespace Nevarea::Renderer {
	void vulkan_resources_init(ResourceManager& manager, VmaAllocator allocator)
	{
		manager.allocator = allocator;
	}

	void vulkan_resources_destroy(ResourceManager& manager)
	{
		for (size_t i = 0; i < manager.buffer_pool.size(); ++i) {
			if (manager.buffer_pool[i] != VK_NULL_HANDLE) {
				std::cerr << "[NEVAREA]: [RESOURCE MANAGER] Leaked buffer at slot " << i << std::endl;
				vmaDestroyBuffer(manager.allocator, manager.buffer_pool[i], manager.allocation_pool[i]);
			}
		}

		manager.buffer_pool.clear();
		manager.allocation_pool.clear();
		manager.generation_pool.clear();
		manager.free_list.clear();
	}

	BufferHandle vulkan_create_buffer(ResourceManager& manager, const BufferDescription& buffer_description)
	{
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_description.size;
		buffer_create_info.usage = buffer_description.usage;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocation_create_info{};
		allocation_create_info.usage = buffer_description.memory_usage;

		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;

		NEVAREA_ASSERT(vmaCreateBuffer(manager.allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr) == VK_SUCCESS,
			"RESOURCE MANAGER", "vmaCreateBuffer failed!");

		uint32_t index;
		if (!manager.free_list.empty()) {
			index = manager.free_list.back();
			manager.free_list.pop_back();
			manager.buffer_pool[index] = buffer;
			manager.allocation_pool[index] = allocation;
		} else {
			index = static_cast<uint32_t>(manager.buffer_pool.size());
			manager.buffer_pool.push_back(buffer);
			manager.allocation_pool.push_back(allocation);
			manager.generation_pool.push_back(0);
		}

		return { index, manager.generation_pool[index] };
	}

	VkBuffer vulkan_get_buffer(const ResourceManager& manager, BufferHandle handle)
	{
		NEVAREA_ASSERT(handle.index < manager.buffer_pool.size(),
			"RESOURCE MANAGER", "BufferHandle index out of range!");

		NEVAREA_ASSERT(handle.generation == manager.generation_pool[handle.index],
			"RESOURCE MANAGER", "Stale BufferHandle (generation mismatch)!");

		return manager.buffer_pool[handle.index];
	}

	void vulkan_destroy_buffer(ResourceManager& manager, BufferHandle handle)
	{
		NEVAREA_ASSERT(handle.index < manager.buffer_pool.size(),
			"RESOURCE MANAGER", "BufferHandle index out of range!");

		NEVAREA_ASSERT(handle.generation == manager.generation_pool[handle.index],
			"RESOURCE MANAGER", "Stale BufferHandle (generation mismatch)!");

		vmaDestroyBuffer(manager.allocator, manager.buffer_pool[handle.index], manager.allocation_pool[handle.index]);

		manager.buffer_pool[handle.index] = VK_NULL_HANDLE;
		manager.allocation_pool[handle.index] = VK_NULL_HANDLE;
		manager.generation_pool[handle.index]++;
		manager.free_list.push_back(handle.index);
	}
}
