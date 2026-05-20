#include "VulkanResourceManager.hpp"

#include "lib/Core.hpp"
#include "lib/Rendering.hpp"

namespace Nevarea::Renderer {
	void vulkan_create_descriptor_pool(ResourceManager& manager) {
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, NEVAREA_BUFFER_IMAGE_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NEVAREA_BUFFER_STORAGE_SIZE }
		};

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		create_info.maxSets = 1;
		create_info.poolSizeCount = 2;
		create_info.pPoolSizes = pool_sizes;

		VK_ASSERT(vkCreateDescriptorPool(manager.device, &create_info, nullptr, &manager.descriptor_pool));
	}

	void vulkan_create_descriptor_layout(ResourceManager& manager) {
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = NEVAREA_BUFFER_IMAGE_SIZE;
		binding.stageFlags = VK_SHADER_STAGE_ALL;

		VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo layout_flags{};
		layout_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layout_flags.bindingCount = 1;
		layout_flags.pBindingFlags = &flags;

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = &layout_flags;
		layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layout_info.bindingCount = 1;
		layout_info.pBindings = &binding;

		VK_ASSERT(vkCreateDescriptorSetLayout(manager.device, &layout_info, nullptr, &manager.descriptor_layout));
	}

	void vulkan_init_descriptor_set(ResourceManager& manager) {
		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorPool = manager.descriptor_pool;
		info.descriptorSetCount = 1;
		info.pSetLayouts = &manager.descriptor_layout;

		VK_ASSERT(vkAllocateDescriptorSets(manager.device, &info, &manager.descriptor_set));
	}

	void vulkan_resources_init(ResourceManager& manager, VmaAllocator allocator, VkDevice device)
	{
		manager.allocator = allocator;
		manager.device = device;

		vulkan_create_descriptor_pool(manager);
		vulkan_create_descriptor_layout(manager);
		vulkan_init_descriptor_set(manager);
	}

	void vulkan_resources_destroy(ResourceManager& manager)
	{
		for (size_t i = 0; i < manager.mesh_pool.size(); ++i) {
			MeshData& mesh = manager.mesh_pool[i];
			BufferHandle& buffer = mesh.vertex_buffer;

			if (buffer.index >= manager.generation_pool.size()) continue;
			if (buffer.generation != manager.generation_pool[buffer.index]) continue;
			if (manager.buffer_pool[buffer.index] == VK_NULL_HANDLE) continue;

			std::cerr << "[NEVAREA]: [RESOURCE MANAGER] Leaked mesh at slot " << i << std::endl;
			vulkan_destroy_buffer(manager, buffer);
		}

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
	
		vkDestroyDescriptorSetLayout(manager.device, manager.descriptor_layout, nullptr);
		vkDestroyDescriptorPool(manager.device, manager.descriptor_pool, nullptr);
	}

	void vulkan_resources_push_deletor(DeletionQueue& deletion_queue, std::function<void()>&& fn)
	{
		deletion_queue.deletors.push_back(std::move(fn));
	}

	void vulkan_resources_flush_deletors(DeletionQueue& deletion_queue)
	{
		for (auto& fn : deletion_queue.deletors) fn();
		deletion_queue.deletors.clear();
	}

	BufferHandle vulkan_create_buffer(ResourceManager& manager, const BufferDescription& buffer_description)
	{
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = buffer_description.size;
		buffer_create_info.usage = buffer_description.usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocation_create_info{};
		allocation_create_info.usage = buffer_description.memory_usage;

		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;

		VK_ASSERT(vmaCreateBuffer(manager.allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr));

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

	uint64_t vulkan_get_buffer_address(const ResourceManager& manager, BufferHandle handle) {
		VkBuffer buffer = vulkan_get_buffer(manager, handle);

		VkBufferDeviceAddressInfo address_info{};
		address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		address_info.buffer = buffer;

		return vkGetBufferDeviceAddress(manager.device, &address_info);
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

	Mesh vulkan_create_mesh(ResourceManager& manager, Vertex* vertices, uint32_t count) {
		BufferDescription description{};
		description.size = sizeof(Vertex) * count;
		description.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		description.memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		BufferHandle handle = vulkan_create_buffer(manager, description);

		void* data;
		vmaMapMemory(manager.allocator, manager.allocation_pool[handle.index], &data);
		memcpy(data, vertices, description.size);
		vmaUnmapMemory(manager.allocator, manager.allocation_pool[handle.index]);

		MeshData mesh = { handle, count };

		uint32_t index;
		if (!manager.mesh_free_list.empty()) {
			index = manager.mesh_free_list.back();
			manager.mesh_free_list.pop_back();
			manager.mesh_pool[index] = mesh;
		}
		else {
			index = static_cast<uint32_t>(manager.mesh_pool.size());
			manager.mesh_pool.push_back(mesh);
			manager.mesh_generation_pool.push_back(0);
		}

		return { index, manager.mesh_generation_pool[index] };
	}

	void vulkan_destroy_mesh(ResourceManager& manager, Mesh handle, FrameContext& frame) {
		NEVAREA_ASSERT(handle.id < manager.mesh_pool.size(),
			"RESOURCE MANAGER", "Mesh handle index out of range!");

		NEVAREA_ASSERT(handle.generation == manager.mesh_generation_pool[handle.id],
			"RESOURCE MANAGER", "Stale Mesh handle (generation mismatch)!");

		MeshData& mesh = manager.mesh_pool[handle.id];
		BufferHandle vertex_buffer = mesh.vertex_buffer;
		uint32_t buffer_id = vertex_buffer.index;

		VmaAllocator allocator = manager.allocator;
		VkBuffer buffer = manager.buffer_pool[buffer_id];
		VmaAllocation allocation = manager.allocation_pool[buffer_id];

		vulkan_resources_push_deletor(frame.deletion_queues[frame.current_frame], [allocator, buffer, allocation]() {
			vmaDestroyBuffer(allocator, buffer, allocation);
		});
	
		manager.buffer_pool[buffer_id] = VK_NULL_HANDLE;
		manager.generation_pool[buffer_id]++;
		manager.free_list.push_back(buffer_id);

		manager.mesh_generation_pool[handle.id]++;
		manager.mesh_free_list.push_back(handle.id);
	}
}