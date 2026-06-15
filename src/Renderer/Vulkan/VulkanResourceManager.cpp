#include "VulkanResourceManager.hpp"
#include "Renderer/Vulkan/VulkanFrames.hpp"
#include "VulkanDebug.hpp"
#include "VulkanTranslate.hpp"

#include "lib/Core.hpp"
#include "lib/Rendering.hpp"
#include <cstdint>

namespace Nevarea::Renderer {
    static VkBufferUsageFlags to_vk_buffer_usage(uint32_t usage) {
		VkBufferUsageFlags flags = 0;
		if (usage & BufferUsage::STORAGE) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (usage & BufferUsage::UNIFORM) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (usage & BufferUsage::INDEX) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (usage & BufferUsage::INDIRECT) flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (usage & BufferUsage::TRANSFER_SRC) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (usage & BufferUsage::TRANSFER_DST) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		return flags;
	}

	static VmaMemoryUsage to_vma_memory(MemoryLocation location) {
		switch (location) {
			case MemoryLocation::GPU_ONLY: return VMA_MEMORY_USAGE_GPU_ONLY;
			case MemoryLocation::CPU_TO_GPU: return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case MemoryLocation::GPU_TO_CPU: return VMA_MEMORY_USAGE_GPU_TO_CPU;
		}
		return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}

	static_assert(sizeof(k_format_info) / sizeof(FormatInfo) == (size_t)Format::COUNT,
		"k_format_info is out of sync with the public Format enum");

	static const FormatInfo& format_info(Format format) {
		return k_format_info[static_cast<uint32_t>(format)];
	}

	VkFormat to_vk_format(Format format) {
		return format_info(format).vk;
	}

	static size_t format_size(Format format, uint32_t width, uint32_t height) {
		const FormatInfo& info = format_info(format);
		uint32_t bw = (width  + info.block_width  - 1) / info.block_width;
		uint32_t bh = (height + info.block_height - 1) / info.block_height;
		return static_cast<size_t>(bw) * bh * info.block_bytes;
	}

	static VkImageUsageFlags to_vk_image_usage(uint32_t usage) {
		VkImageUsageFlags flags = 0;
		if (usage & ImageUsage::STORAGE) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (usage & ImageUsage::SAMPLED) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (usage & ImageUsage::TRANSFER_SRC) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usage & ImageUsage::TRANSFER_DST) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (usage & ImageUsage::COLOR_TARGET) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (usage & ImageUsage::DEPTH_STENCIL_TARGET) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		return flags;
	}

	void vulkan_create_descriptor_pool(ResourceManager& manager) {
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, NEVAREA_BUFFER_IMAGE_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NEVAREA_BUFFER_STORAGE_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, NEVAREA_BUFFER_IMAGE_SIZE },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, NEVAREA_SAMPLER_SIZE }
		};

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
		create_info.maxSets = 1;
		create_info.poolSizeCount = 4;
		create_info.pPoolSizes = pool_sizes;

		VK_ASSERT(vkCreateDescriptorPool(manager.device, &create_info, nullptr, &manager.descriptor_pool));
		VK_NAME(manager.device, VK_OBJECT_TYPE_DESCRIPTOR_POOL, manager.descriptor_pool, "descriptor_pool");
	}

	void vulkan_create_descriptor_layout(ResourceManager& manager) {
		VkDescriptorSetLayoutBinding bindings[3]{};
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		bindings[0].descriptorCount = NEVAREA_BUFFER_IMAGE_SIZE;
		bindings[0].stageFlags = VK_SHADER_STAGE_ALL;

		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		bindings[1].descriptorCount = NEVAREA_BUFFER_IMAGE_SIZE;
		bindings[1].stageFlags = VK_SHADER_STAGE_ALL;

		bindings[2].binding = 2;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		bindings[2].descriptorCount = NEVAREA_SAMPLER_SIZE;
		bindings[2].stageFlags = VK_SHADER_STAGE_ALL;

		VkDescriptorBindingFlags flags[3] = {
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		};

		VkDescriptorSetLayoutBindingFlagsCreateInfo layout_flags{};
		layout_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		layout_flags.bindingCount = 3;
		layout_flags.pBindingFlags = flags;

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.pNext = &layout_flags;
		layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layout_info.bindingCount = 3;
		layout_info.pBindings = bindings;

		VK_ASSERT(vkCreateDescriptorSetLayout(manager.device, &layout_info, nullptr, &manager.descriptor_layout));
		VK_NAME(manager.device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, manager.descriptor_layout, "descriptor_set_layout");
	}

	void vulkan_init_descriptor_set(ResourceManager& manager) {
		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.descriptorPool = manager.descriptor_pool;
		info.descriptorSetCount = 1;
		info.pSetLayouts = &manager.descriptor_layout;

		VK_ASSERT(vkAllocateDescriptorSets(manager.device, &info, &manager.descriptor_set));
		VK_NAME(manager.device, VK_OBJECT_TYPE_DESCRIPTOR_SET, manager.descriptor_set, "descriptor_set");
	}

	void vulkan_create_command_pool(ResourceManager& manager, uint32_t queue_family_index) {
	    VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		create_info.queueFamilyIndex = queue_family_index;
		VK_ASSERT(vkCreateCommandPool(manager.device, &create_info, nullptr, &manager.upload_pool));

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = 0;
        VK_ASSERT(vkCreateFence(manager.device, &fence_info, nullptr, &manager.upload_fence));

        VkCommandBufferAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc.commandPool = manager.upload_pool;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = 1;
        VK_ASSERT(vkAllocateCommandBuffers(manager.device, &alloc, &manager.upload_cmd));
	}

	void vulkan_resources_init(ResourceManager& manager, VmaAllocator allocator, VkDevice device, VkQueue graphics_queue, uint32_t graphics_family_index)
	{
		manager.allocator = allocator;
		manager.device = device;
		manager.upload_queue = graphics_queue;

		vulkan_create_descriptor_pool(manager);
		vulkan_create_descriptor_layout(manager);
		vulkan_init_descriptor_set(manager);
		vulkan_create_command_pool(manager, graphics_family_index);
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

		for (auto& img : manager.image_pool) {
			if (img.image != VK_NULL_HANDLE) {
				std::cerr << "[NEVAREA]: [RESOURCE MANAGER] Leaked image" << std::endl;
				vkDestroyImageView(manager.device, img.view, nullptr);
				vmaDestroyImage(manager.allocator, img.image, img.allocation);
			}
		}

		manager.image_pool.clear();
		manager.image_generation_pool.clear();
		manager.image_free_list.clear();

		for (size_t i = 0; i < manager.sampler_pool.size(); ++i) {
		    if (manager.sampler_pool[i] != VK_NULL_HANDLE) {
				std::cerr << "[NEVAREA]: [RESOURCE MANAGER] Leaked sampler at slot " << i << std::endl;
				vkDestroySampler(manager.device, manager.sampler_pool[i], nullptr);
			}
		}

		manager.sampler_pool.clear();
		manager.sampler_generation_pool.clear();
		manager.sampler_free_list.clear();

		vkDestroyFence(manager.device, manager.upload_fence, nullptr);
		vkDestroyCommandPool(manager.device, manager.upload_pool, nullptr);

		vkDestroyDescriptorSetLayout(manager.device, manager.descriptor_layout, nullptr);
		vkDestroyDescriptorPool(manager.device, manager.descriptor_pool, nullptr);
	}

	void vulkan_immediate_submit(ResourceManager &manager, std::function<void (VkCommandBuffer)> &&record) {
	    vkResetFences(manager.device, 1, &manager.upload_fence);
	    vkResetCommandPool(manager.device, manager.upload_pool, 0);

		VkCommandBufferBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_ASSERT(vkBeginCommandBuffer(manager.upload_cmd, &begin));

        record(manager.upload_cmd);
        VK_ASSERT(vkEndCommandBuffer(manager.upload_cmd));

        VkCommandBufferSubmitInfo cmd_info{};
        cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmd_info.commandBuffer = manager.upload_cmd;

        VkSubmitInfo2 submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.commandBufferInfoCount = 1;
        submit.pCommandBufferInfos = &cmd_info;

        VK_ASSERT(vkQueueSubmit2(manager.upload_queue, 1, &submit, manager.upload_fence));
        VK_ASSERT(vkWaitForFences(manager.device, 1, &manager.upload_fence, VK_TRUE, UINT64_MAX));
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
		buffer_create_info.usage = to_vk_buffer_usage(buffer_description.usage) | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocation_create_info{};
		allocation_create_info.usage = to_vma_memory(buffer_description.memory);
		allocation_create_info.priority = buffer_description.priority;

		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;

		VK_ASSERT(vmaCreateBuffer(manager.allocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr));
		VK_NAME(manager.device, VK_OBJECT_TYPE_BUFFER, buffer, buffer_description.debug_name);

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

	void vulkan_upload_buffer(ResourceManager& manager, BufferHandle dst, const void* data, size_t size) {
	    VkBuffer dst_buffer = vulkan_get_buffer(manager, dst);

		VkBufferCreateInfo stg_info{};
		stg_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stg_info.size = size;
		stg_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stg_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VkBuffer staging_buffer;
		VmaAllocation staging_alloc;
		VK_ASSERT(vmaCreateBuffer(manager.allocator, &stg_info, &alloc_info, &staging_buffer, &staging_alloc, nullptr));

        void* mapped;
        VK_ASSERT(vmaMapMemory(manager.allocator, staging_alloc, &mapped));
        memcpy(mapped, data, size);
        vmaUnmapMemory(manager.allocator, staging_alloc);

        vulkan_immediate_submit(manager, [&](VkCommandBuffer cmd) {
            VkBufferCopy region{ 0, 0, size };
            vkCmdCopyBuffer(cmd, staging_buffer, dst_buffer, 1, &region);
        });

        vmaDestroyBuffer(manager.allocator, staging_buffer, staging_alloc);
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

	ImageHandle vulkan_create_image(ResourceManager& manager, const ImageDescription& description)
	{
	    VkExtent2D extent = { description.width, description.height };
	    VkFormat format = k_format_info[(uint32_t)description.format].vk;
	    VkImageUsageFlags usage = to_vk_image_usage(description.usage);

		AllocatedImage img{};
		img.extent = extent;
		img.format = description.format;
		img.usage = usage;

		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = format;
		image_info.extent = { extent.width, extent.height, 1 };
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = usage;

		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		alloc_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		alloc_info.priority = description.priority;

		VK_ASSERT(vmaCreateImage(manager.allocator, &image_info, &alloc_info, &img.image, &img.allocation, nullptr));
		VK_NAME(manager.device, VK_OBJECT_TYPE_IMAGE, img.image, "nevarea_image");

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = img.image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = format;
		view_info.subresourceRange = { k_format_info[(uint32_t)description.format].aspect, 0, 1, 0, 1 };
		VK_ASSERT(vkCreateImageView(manager.device, &view_info, nullptr, &img.view));

		uint32_t index;
		if (!manager.image_free_list.empty()) {
			index = manager.image_free_list.back();
			manager.image_free_list.pop_back();
			manager.image_pool[index] = img;
		}
		else {
			index = static_cast<uint32_t>(manager.image_pool.size());
			manager.image_pool.push_back(img);
			manager.image_generation_pool.push_back(0);
		}

		if (usage & VK_IMAGE_USAGE_STORAGE_BIT) {
			VkDescriptorImageInfo desc_image{};
			desc_image.imageView = img.view;
			desc_image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = manager.descriptor_set;
			write.dstBinding = 1;
			write.dstArrayElement = index;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write.pImageInfo = &desc_image;
			vkUpdateDescriptorSets(manager.device, 1, &write, 0, nullptr);
		}

		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
            VkDescriptorImageInfo desc_image{};
            desc_image.imageView = img.view;
            desc_image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet write{ };
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = manager.descriptor_set;
            write.dstBinding = 0;
            write.dstArrayElement = index;
            write.descriptorCount = 1;
            write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            write.pImageInfo = &desc_image;
            vkUpdateDescriptorSets(manager.device, 1, &write, 0, nullptr);
        }

		return { index, manager.image_generation_pool[index] };
	}

	AllocatedImage& vulkan_get_image(ResourceManager& manager, ImageHandle handle)
	{
		NEVAREA_ASSERT(handle.index < manager.image_pool.size(),
			"RESOURCE MANAGER", "ImageHandle index out of range!");

		NEVAREA_ASSERT(handle.generation == manager.image_generation_pool[handle.index],
			"RESOURCE MANAGER", "Stale ImageHandle (generation mismatch)!");

		return manager.image_pool[handle.index];
	}

	void vulkan_upload_image(ResourceManager& manager, ImageHandle handle, const void* pixels, size_t size) {
	    AllocatedImage& img = vulkan_get_image(manager, handle);

		NEVAREA_ASSERT((img.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0,
            "RESOURCE MANAGER", "upload_image: image must be created with ImageUsage::TRANSFER_DST!");

		size_t expected = format_size(img.format, img.extent.width, img.extent.height);

		NEVAREA_ASSERT(size >= expected, "RESOURCE MANAGER", "upload_image: pixel data smaller than image requires!");

		VkBufferCreateInfo buf_info{};
		buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.size = size;
        buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo alloc_ci{};
        alloc_ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        VkBuffer staging; VmaAllocation staging_alloc;
        VK_ASSERT(vmaCreateBuffer(manager.allocator, &buf_info, &alloc_ci, &staging, &staging_alloc, nullptr));

        void* data;
        VK_ASSERT(vmaMapMemory(manager.allocator, staging_alloc, &data));
        memcpy(data, pixels, size);
        vmaUnmapMemory(manager.allocator, staging_alloc);

        vulkan_immediate_submit(manager, [&](VkCommandBuffer cmd) {
            transition_image(cmd, img.image,
                img.current_layout,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, 0,
                VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = k_format_info[(uint32_t)img.format].aspect;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { img.extent.width, img.extent.height, 1 };

            vkCmdCopyBufferToImage(cmd, staging, img.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region);

            transition_image(cmd, img.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_COPY_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

            img.current_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        });

        vmaDestroyBuffer(manager.allocator, staging, staging_alloc);
	}

	void vulkan_destroy_image(ResourceManager& manager, ImageHandle handle, FrameContext& frame)
	{
		AllocatedImage& img = vulkan_get_image(manager, handle);

		VmaAllocator allocator = manager.allocator;
		VkDevice device = manager.device;
		VkImage image = img.image;
		VkImageView view = img.view;
		VmaAllocation allocation = img.allocation;

		vulkan_resources_push_deletor(frame.deletion_queues[frame.current_frame], [allocator, device, image, view, allocation]() {
			vkDestroyImageView(device, view, nullptr);
			vmaDestroyImage(allocator, image, allocation);
			});

		manager.image_pool[handle.index].image = VK_NULL_HANDLE;
		manager.image_pool[handle.index].view = VK_NULL_HANDLE;
		manager.image_generation_pool[handle.index]++;
		manager.image_free_list.push_back(handle.index);
	}

	Mesh vulkan_create_mesh(ResourceManager& manager, const void* vertex_data, uint32_t vertex_count, const VertexLayout& layout, uint32_t index_count, const uint32_t* indices) {
	    NEVAREA_ASSERT(layout.stride > 0 && vertex_data, "RESOURCE MANAGER",
			"you can't create a 0 stride mesh!");

	    size_t size = (size_t)layout.stride * vertex_count;

	    BufferDescription description{};
    	description.size = size;
    	description.usage = BufferUsage::STORAGE | BufferUsage::TRANSFER_DST;
    	description.memory = MemoryLocation::GPU_ONLY;
    	description.debug_name = "mesh_vertex_buffer";

		BufferHandle handle = vulkan_create_buffer(manager, description);
		vulkan_upload_buffer(manager, handle, vertex_data, size);

		MeshData mesh = {};
		mesh.vertex_buffer = handle;
		mesh.vertex_address = vulkan_get_buffer_address(manager, handle);
		mesh.vertex_count = vertex_count;

		if (index_count > 0) {
		    size_t index_size = index_count * sizeof(uint32_t);

		    BufferDescription index_description{};
			index_description.size = index_size;
			index_description.usage = BufferUsage::INDEX | BufferUsage::TRANSFER_DST;
			index_description.memory = MemoryLocation::GPU_ONLY;
			index_description.debug_name = "mesh_index_buffer";

			BufferHandle index_buffer = vulkan_create_buffer(manager, index_description);
			vulkan_upload_buffer(manager, index_buffer, indices, index_size);

			mesh.index_count = index_count;
			mesh.index_buffer = index_buffer;
		}

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

		if (mesh.index_buffer.is_valid()) {
		    BufferHandle index_buffer = mesh.index_buffer;
			uint32_t index_id = index_buffer.index;

			VkBuffer vk_index_buffer = manager.buffer_pool[index_id];
			VmaAllocation index_allocation = manager.allocation_pool[index_id];

			vulkan_resources_push_deletor(frame.deletion_queues[frame.current_frame], [allocator, vk_index_buffer, index_allocation]() {
				vmaDestroyBuffer(allocator, vk_index_buffer, index_allocation);
			});

			manager.buffer_pool[index_id] = VK_NULL_HANDLE;
			manager.generation_pool[index_id]++;
			manager.free_list.push_back(index_id);
		}

		manager.mesh_generation_pool[handle.id]++;
		manager.mesh_free_list.push_back(handle.id);
	}

	Sampler vulkan_create_sampler(ResourceManager& manager, const SamplerDescription& description) {
	    VkSamplerCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		create_info.pNext = nullptr;
		create_info.flags = 0;

		create_info.magFilter = to_vk_filter(description.mag_filter);
		create_info.minFilter = to_vk_filter(description.min_filter);

		create_info.addressModeU = to_vk_address_mode(description.address_u);
		create_info.addressModeV = to_vk_address_mode(description.address_v);
		create_info.addressModeW = to_vk_address_mode(description.address_w);

		create_info.anisotropyEnable = (description.max_anisotropy > 1);
		create_info.maxAnisotropy = description.max_anisotropy;

		create_info.borderColor = to_vk_border_color(description.border_color);

		create_info.unnormalizedCoordinates = VK_FALSE;

		create_info.compareEnable = (description.compare_op != CompareOp::NONE);
		create_info.compareOp = to_vk_compare_op(description.compare_op);

		create_info.mipmapMode = to_vk_mipmap_mode(description.mipmap_mode);
		create_info.mipLodBias = description.mip_lod_bias;
		create_info.minLod = description.min_lod;
		create_info.maxLod = description.max_lod >= 1000.0f ? VK_LOD_CLAMP_NONE : description.max_lod;

		VkSampler sampler;

		VK_ASSERT(vkCreateSampler(manager.device, &create_info, nullptr, &sampler));

		uint32_t index;
		if (!manager.sampler_free_list.empty()) {
			index = manager.sampler_free_list.back();
			manager.sampler_free_list.pop_back();
			manager.sampler_pool[index] = sampler;
		}
		else {
			index = static_cast<uint32_t>(manager.sampler_pool.size());
			manager.sampler_pool.push_back(sampler);
			manager.sampler_generation_pool.push_back(0);
		}

		VkDescriptorImageInfo image_info{};
		image_info.sampler = sampler;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = manager.descriptor_set;
		write.dstBinding = 2;
		write.dstArrayElement = index;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &image_info;

		vkUpdateDescriptorSets(manager.device, 1, &write, 0, nullptr);

		return { index, manager.sampler_generation_pool[index] };
	}

	void vulkan_destroy_sampler(ResourceManager& manager, Sampler sampler, FrameContext& frame) {
		NEVAREA_ASSERT(sampler.id < manager.sampler_pool.size(),
			"RESOURCE MANAGER", "Sampler handle index out of range!");

		NEVAREA_ASSERT(sampler.generation == manager.sampler_generation_pool[sampler.id],
			"RESOURCE MANAGER", "Stale Sampler handle (generation mismatch)!");

    	VkDevice device = manager.device;

        VkSampler raw_sampler = manager.sampler_pool[sampler.id];

        vulkan_resources_push_deletor(frame.deletion_queues[frame.current_frame], [device, raw_sampler]() {
            vkDestroySampler(device, raw_sampler, nullptr);
        });

       	manager.sampler_pool[sampler.id] = VK_NULL_HANDLE;
       	manager.sampler_generation_pool[sampler.id]++;
       	manager.sampler_free_list.push_back(sampler.id);
	}
}
