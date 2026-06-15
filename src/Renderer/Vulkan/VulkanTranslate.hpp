#pragma once

#include "Core/n_pch.hpp"
#include "lib/Rendering.hpp"

namespace Nevarea::Renderer {
	VkSamplerAddressMode to_vk_address_mode(Nevarea::AddressMode mode);
	VkFilter to_vk_filter(Nevarea::Filter filter);
	VkSamplerMipmapMode to_vk_mipmap_mode(Nevarea::MipmapMode mode);
	VkCompareOp to_vk_compare_op(Nevarea::CompareOp op);
	VkBorderColor to_vk_border_color(Nevarea::BorderColor color);
	VkFormat to_vk_vertex_format(Nevarea::VertexFormat format);
	VkAttachmentLoadOp to_vk_load_op(LoadOp op);
	VkAttachmentStoreOp to_vk_store_op(StoreOp op);
	VkPrimitiveTopology to_vk_topology(PrimitiveTopology topology);
	VkPolygonMode to_vk_polygon_mode(PolygonMode mode);
	VkCullModeFlags to_vk_cull_mode(CullMode cull);
	VkFrontFace to_vk_front_face(FrontFace face);
}
