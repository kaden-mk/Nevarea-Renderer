#include "VulkanTranslate.hpp"

namespace Nevarea::Renderer {
	VkSamplerAddressMode to_vk_address_mode(Nevarea::AddressMode mode) {
	    switch (mode) {
			case AddressMode::REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case AddressMode::MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case AddressMode::CLAMP_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case AddressMode::CLAMP_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}

		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkFilter to_vk_filter(Nevarea::Filter filter) {
	    switch (filter) {
			case Filter::NEAREST: return VK_FILTER_NEAREST;
			case Filter::LINEAR: return VK_FILTER_LINEAR;
		}

		return VK_FILTER_LINEAR;
	}

	VkSamplerMipmapMode to_vk_mipmap_mode(Nevarea::MipmapMode mode) {
	    switch (mode) {
			case MipmapMode::LINEAR: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			case MipmapMode::NEAREST: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}

		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	VkCompareOp to_vk_compare_op(Nevarea::CompareOp op) {
		switch (op) {
			case CompareOp::NONE: return VK_COMPARE_OP_ALWAYS;
			case CompareOp::LESS: return VK_COMPARE_OP_LESS;
			case CompareOp::LESS_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOp::GREATER: return VK_COMPARE_OP_GREATER;
			case CompareOp::GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOp::EQUAL: return VK_COMPARE_OP_EQUAL;
			case CompareOp::ALWAYS: return VK_COMPARE_OP_ALWAYS;
		}

		return VK_COMPARE_OP_ALWAYS;
	}

	VkBorderColor to_vk_border_color(Nevarea::BorderColor color) {
		switch (color) {
			case BorderColor::TRANSPARENT_BLACK: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case BorderColor::OPAQUE_BLACK: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case BorderColor::OPAQUE_WHITE: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		}

		return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	}

	VkFormat to_vk_vertex_format(Nevarea::VertexFormat format) {
	    switch (format) {
			case VertexFormat::FLOAT: return VK_FORMAT_R32_SFLOAT;
			case VertexFormat::FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
			case VertexFormat::FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexFormat::FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case VertexFormat::UNORM8x4: return VK_FORMAT_R8G8B8A8_UNORM;
			case VertexFormat::UINT: return VK_FORMAT_R32_UINT;
			case VertexFormat::UINT2: return VK_FORMAT_R32G32_UINT;
			case VertexFormat::UINT4: return VK_FORMAT_R32G32B32A32_UINT;
		}

		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	VkAttachmentLoadOp to_vk_load_op(LoadOp op) {
	    switch (op) {
			case LoadOp::LOAD: return VK_ATTACHMENT_LOAD_OP_LOAD;
			case LoadOp::CLEAR: return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case LoadOp::DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		return VK_ATTACHMENT_LOAD_OP_NONE;
	}

	VkAttachmentStoreOp to_vk_store_op(StoreOp op) {
	    switch (op) {
			case StoreOp::STORE: return VK_ATTACHMENT_STORE_OP_STORE;
			case StoreOp::DONT_CARE: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

	    return VK_ATTACHMENT_STORE_OP_NONE;
	}

	VkPrimitiveTopology to_vk_topology(PrimitiveTopology topology) {
		switch (topology) {
			case PrimitiveTopology::TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PrimitiveTopology::LINE_LIST: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::POINT_LIST: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}

		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	VkPolygonMode to_vk_polygon_mode(PolygonMode mode) {
		switch (mode) {
			case PolygonMode::FILL: return VK_POLYGON_MODE_FILL;
			case PolygonMode::LINE: return VK_POLYGON_MODE_LINE;
			case PolygonMode::POINT: return VK_POLYGON_MODE_POINT;
		}

		return VK_POLYGON_MODE_FILL;
	}

	VkCullModeFlags to_vk_cull_mode(CullMode cull) {
		switch (cull) {
			case CullMode::NONE: return VK_CULL_MODE_NONE;
			case CullMode::FRONT: return VK_CULL_MODE_FRONT_BIT;
			case CullMode::BACK: return VK_CULL_MODE_BACK_BIT;
		}

		return VK_CULL_MODE_NONE;
	}

	VkFrontFace to_vk_front_face(FrontFace face) {
		switch (face) {
			case FrontFace::CLOCKWISE: return VK_FRONT_FACE_CLOCKWISE;
			case FrontFace::COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}

		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	VkPresentModeKHR to_vk_present_mode(PresentMode mode) {
		switch (mode) {
			case PresentMode::VSYNC: return VK_PRESENT_MODE_FIFO_KHR;
			case PresentMode::MAILBOX: return VK_PRESENT_MODE_MAILBOX_KHR;
			case PresentMode::IMMEDIATE: return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkImageType to_vk_image_type(ImageType image_type) {
	    switch (image_type) {
			case ImageType::D3: return VK_IMAGE_TYPE_3D;
            case ImageType::D1: case ImageType::D1_ARRAY: return VK_IMAGE_TYPE_1D;
            default: return VK_IMAGE_TYPE_2D;
		}
	}

	VkImageViewType to_vk_image_view_type(ImageType image_type) {
        switch (image_type) {
            case ImageType::D1: return VK_IMAGE_VIEW_TYPE_1D;
            case ImageType::D2: return VK_IMAGE_VIEW_TYPE_2D;
            case ImageType::D3: return VK_IMAGE_VIEW_TYPE_3D;
            case ImageType::D1_ARRAY: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            case ImageType::D2_ARRAY: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            case ImageType::CUBE: return VK_IMAGE_VIEW_TYPE_CUBE;
            case ImageType::CUBE_ARRAY: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        }
        return VK_IMAGE_VIEW_TYPE_2D;
	}

	VkImageUsageFlags to_vk_image_usage(uint32_t usage) {
    	VkImageUsageFlags flags = 0;
    	if (usage & ImageUsage::STORAGE) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    	if (usage & ImageUsage::SAMPLED) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    	if (usage & ImageUsage::TRANSFER_SRC) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    	if (usage & ImageUsage::TRANSFER_DST) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    	if (usage & ImageUsage::COLOR_TARGET) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    	if (usage & ImageUsage::DEPTH_STENCIL_TARGET) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    	return flags;
	}

	VkBufferUsageFlags to_vk_buffer_usage(uint32_t usage) {
        VkBufferUsageFlags flags = 0;
        if (usage & BufferUsage::STORAGE) flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (usage & BufferUsage::UNIFORM) flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (usage & BufferUsage::INDEX) flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (usage & BufferUsage::INDIRECT) flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        if (usage & BufferUsage::TRANSFER_SRC) flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (usage & BufferUsage::TRANSFER_DST) flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (usage & BufferUsage::ACCEL_STORAGE) flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
        if (usage & BufferUsage::ACCEL_INPUT) flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        if (usage & BufferUsage::SHADER_BINDING_TABLE) flags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        return flags;
    }

    VmaMemoryUsage to_vma_memory(MemoryLocation location) {
		switch (location) {
			case MemoryLocation::GPU_ONLY: return VMA_MEMORY_USAGE_GPU_ONLY;
			case MemoryLocation::CPU_TO_GPU: return VMA_MEMORY_USAGE_CPU_TO_GPU;
			case MemoryLocation::GPU_TO_CPU: return VMA_MEMORY_USAGE_GPU_TO_CPU;
		}
		return VMA_MEMORY_USAGE_CPU_TO_GPU;
	}

	VkImageCreateFlags to_vk_image_create_flags(uint32_t flags, ImageType type) {
        VkImageCreateFlags out = 0;

        if (flags & ImageFlags::MUTABLE_FORMAT) out |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        if (flags & ImageFlags::BLOCK_TEXEL_VIEW) out |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
        if (flags & ImageFlags::ARRAY_2D_COMPATIBLE) out |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        if (type == ImageType::CUBE || type == ImageType::CUBE_ARRAY)
            out |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        return out;
    }
}
