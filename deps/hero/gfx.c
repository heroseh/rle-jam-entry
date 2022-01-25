#ifndef _HERO_GFX_H_
#include "gfx.h"
#endif

#if HERO_VULKAN_ENABLE
#ifndef _HERO_GFX_INTERNAL_VULKAN_H_
#include "gfx_internal_vulkan.h"
#endif
#endif

// ===========================================
//
//
// General
//
//
// ===========================================

U8 hero_index_type_sizes[HERO_INDEX_TYPE_COUNT] = {
	[HERO_INDEX_TYPE_U8] = sizeof(U8),
	[HERO_INDEX_TYPE_U16] = sizeof(U16),
	[HERO_INDEX_TYPE_U32] = sizeof(U32),
};

U8 hero_image_format_bytes_per_pixel[HERO_IMAGE_FORMAT_COUNT] = {
	[HERO_IMAGE_FORMAT_NONE] = 0,
	[HERO_IMAGE_FORMAT_R8_UNORM] = 1,
	[HERO_IMAGE_FORMAT_R8G8_UNORM] = 2,
	[HERO_IMAGE_FORMAT_R8G8B8_UNORM] = 3,
	[HERO_IMAGE_FORMAT_R8G8B8A8_UNORM] = 4,
	[HERO_IMAGE_FORMAT_B8G8R8_UNORM] = 3,
	[HERO_IMAGE_FORMAT_B8G8R8A8_UNORM] = 4,
	[HERO_IMAGE_FORMAT_R32_UINT] = 4,
	[HERO_IMAGE_FORMAT_R32_SINT] = 4,
	[HERO_IMAGE_FORMAT_R32_SFLOAT] = 4,
	[HERO_IMAGE_FORMAT_D16] = 2,
	[HERO_IMAGE_FORMAT_D32] = 3,
	[HERO_IMAGE_FORMAT_S8] = 1,
	[HERO_IMAGE_FORMAT_D16_S8] = 3,
	[HERO_IMAGE_FORMAT_D24_S8] = 4,
	[HERO_IMAGE_FORMAT_D32_S8] = 5,
};

// ===========================================
//
//
// Physical Device
//
//
// ===========================================


HeroResult hero_physical_device_get(U32 idx, HeroPhysicalDevice** physical_devices_out) {
	if (hero_gfx_sys.physical_devices == NULL) {
		return HERO_ERROR(NOT_STARTED);
	}

	if (idx >= hero_gfx_sys.physical_devices_count) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	*physical_devices_out = HERO_PTR_ADD(hero_gfx_sys.physical_devices, idx * hero_gfx_sys.physical_device_size);
	return HERO_SUCCESS;
}

HeroResult hero_physical_device_surface_image_formats_supported(HeroPhysicalDevice* physical_device, HeroSurface surface, HeroImageFormat* formats, U32 formats_count) {
	return hero_gfx_sys.backend_vtable.physical_device_surface_image_formats_supported(physical_device, surface, formats, formats_count);
}

// ===========================================
//
//
// Logical Device
//
//
// ===========================================

HeroResult hero_logical_device_init(HeroPhysicalDevice* physical_device, HeroLogicalDeviceSetup* setup, HeroLogicalDevice** out) {
	HeroLogicalDevice* ldev;
	HeroResult result = hero_gfx_sys.backend_vtable.logical_device_init(physical_device, setup, &ldev);
	if (result < 0) {
		return result;
	}

	ldev->physical_device = physical_device;
	ldev->alctor = setup->alctor;
	ldev->queue_support_flags = setup->queue_support_flags;
	ldev->last_completed_frame_idx = -1;

	*out = ldev;
	return HERO_SUCCESS;
}

HeroResult hero_logical_device_deinit(HeroLogicalDevice* ldev) {
	return hero_gfx_sys.backend_vtable.logical_device_deinit(ldev);
}

HeroResult hero_logical_device_frame_start(HeroLogicalDevice* ldev) {
	return hero_gfx_sys.backend_vtable.logical_device_frame_start(ldev);
}

HeroResult hero_logical_device_queue_transfer(HeroLogicalDevice* ldev) {
	return hero_gfx_sys.backend_vtable.logical_device_queue_transfer(ldev);
}

/*
HeroResult hero_logical_device_queue_command_buffers(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandPoolBufferId* command_pool_buffer_ids, U32 command_pool_buffers_count) {
	return hero_gfx_sys.backend_vtable.logical_device_queue_command_buffers(ldev, command_pool_id, command_pool_buffer_ids, command_pool_buffers_count);
}
*/

HeroResult hero_logical_device_submit(HeroLogicalDevice* ldev, HeroSwapchainId* swapchain_ids, U32 swapchains_count) {
	return hero_gfx_sys.backend_vtable.logical_device_submit(ldev, swapchain_ids, swapchains_count);
}

// ===========================================
//
//
// Vertex Layout
//
//
// ===========================================

U8 hero_vertex_elmt_type_sizes[HERO_VERTEX_ELMT_TYPE_COUNT] = {
	[HERO_VERTEX_ELMT_TYPE_U8] = sizeof(U8),
	[HERO_VERTEX_ELMT_TYPE_S8] = sizeof(S8),
	[HERO_VERTEX_ELMT_TYPE_U8_F32] = sizeof(U8),
	[HERO_VERTEX_ELMT_TYPE_S8_F32] = sizeof(S8),
	[HERO_VERTEX_ELMT_TYPE_U8_F32_NORMALIZE] = sizeof(U8),
	[HERO_VERTEX_ELMT_TYPE_S8_F32_NORMALIZE] = sizeof(S8),
	[HERO_VERTEX_ELMT_TYPE_U16] = sizeof(U16),
	[HERO_VERTEX_ELMT_TYPE_S16] = sizeof(S16),
	[HERO_VERTEX_ELMT_TYPE_U16_F32] = sizeof(U16),
	[HERO_VERTEX_ELMT_TYPE_S16_F32] = sizeof(S16),
	[HERO_VERTEX_ELMT_TYPE_U16_F32_NORMALIZE] = sizeof(U16),
	[HERO_VERTEX_ELMT_TYPE_S16_F32_NORMALIZE] = sizeof(S16),
	[HERO_VERTEX_ELMT_TYPE_U32] = sizeof(U32),
	[HERO_VERTEX_ELMT_TYPE_S32] = sizeof(S32),
	[HERO_VERTEX_ELMT_TYPE_U64] = sizeof(U64),
	[HERO_VERTEX_ELMT_TYPE_S64] = sizeof(S64),
	[HERO_VERTEX_ELMT_TYPE_F16] = sizeof(F16),
	[HERO_VERTEX_ELMT_TYPE_F32] = sizeof(F32),
	[HERO_VERTEX_ELMT_TYPE_F64] = sizeof(F64),
};

U8 hero_vertex_elmt_type_aligns[HERO_VERTEX_ELMT_TYPE_COUNT] = {
	[HERO_VERTEX_ELMT_TYPE_U8] = alignof(U8),
	[HERO_VERTEX_ELMT_TYPE_S8] = alignof(S8),
	[HERO_VERTEX_ELMT_TYPE_U8_F32] = alignof(U8),
	[HERO_VERTEX_ELMT_TYPE_S8_F32] = alignof(S8),
	[HERO_VERTEX_ELMT_TYPE_U8_F32_NORMALIZE] = alignof(U8),
	[HERO_VERTEX_ELMT_TYPE_S8_F32_NORMALIZE] = alignof(S8),
	[HERO_VERTEX_ELMT_TYPE_U16] = alignof(U16),
	[HERO_VERTEX_ELMT_TYPE_S16] = alignof(S16),
	[HERO_VERTEX_ELMT_TYPE_U16_F32] = alignof(U16),
	[HERO_VERTEX_ELMT_TYPE_S16_F32] = alignof(S16),
	[HERO_VERTEX_ELMT_TYPE_U16_F32_NORMALIZE] = alignof(U16),
	[HERO_VERTEX_ELMT_TYPE_S16_F32_NORMALIZE] = alignof(S16),
	[HERO_VERTEX_ELMT_TYPE_U32] = alignof(U32),
	[HERO_VERTEX_ELMT_TYPE_S32] = alignof(S32),
	[HERO_VERTEX_ELMT_TYPE_U64] = alignof(U64),
	[HERO_VERTEX_ELMT_TYPE_S64] = alignof(S64),
	[HERO_VERTEX_ELMT_TYPE_F16] = alignof(F16),
	[HERO_VERTEX_ELMT_TYPE_F32] = alignof(F32),
	[HERO_VERTEX_ELMT_TYPE_F64] = alignof(F64),
};

HeroResult hero_vertex_layout_register(HeroVertexLayout* vl, bool is_static, HeroVertexLayoutId* id_out) {
	HeroResult result;

	for_range(binding_idx, 0, vl->bindings_count) {
		HeroVertexBindingInfo* binding = &vl->bindings[binding_idx];
		if (binding->size == 0) {
			return HERO_ERROR(GFX_ELMT_SIZE_CANNOT_BE_ZERO);
		}

		U32 size = 0;
		U32 max_align = 0;
		for_range(attrib_idx, 0, binding->attribs_count) {
			HeroVertexAttribInfo* attrib = &binding->attribs[attrib_idx];
			U32 align = hero_vertex_elmt_type_aligns[attrib->elmt_type];
			size = HERO_INT_ROUND_UP_ALIGN(size, align);
			max_align = HERO_MAX(max_align, align);
			size += hero_vertex_elmt_type_sizes[attrib->elmt_type] * HERO_VERTEX_VECTOR_TYPE_ELMTS_COUNT(attrib->vector_type);
		}
		size = HERO_INT_ROUND_UP_ALIGN(size, max_align);

		if (binding->size != size) {
			return HERO_ERROR(GFX_BINDING_SIZE_MISMATCH);
		}
	}

	//
	// TODO deduplicate
	HeroVertexLayout* vertex_layout;
	result = hero_gfx_sys.backend_vtable.vertex_layout_register(vl, id_out, &vertex_layout);
	if (result < 0) {
		return result;
	}

	if (is_static) {
		*vertex_layout = *vl;
	} else {
		HERO_ABORT("unimplemented");
	}

	return HERO_SUCCESS;
}

HeroResult hero_vertex_layout_deregister(HeroVertexLayoutId id) {
	HeroVertexLayout* vertex_layout;
	HeroResult result = hero_vertex_layout_get(id, &vertex_layout);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.vertex_layout_deregister(id, vertex_layout);
}

HeroResult hero_vertex_layout_get(HeroVertexLayoutId id, HeroVertexLayout** out) {
	return hero_gfx_sys.backend_vtable.vertex_layout_get(id, out);
}

// ===========================================
//
//
// Buffer
//
//
// ===========================================

HeroResult hero_buffer_init(HeroLogicalDevice* ldev, HeroBufferSetup* setup, HeroBufferId* id_out) {
	HeroResult result;

	if (setup->queue_support_flags == 0 || !HERO_BITSET_CONTAINS_ALL(setup->queue_support_flags, ldev->queue_support_flags)) {
		return HERO_ERROR(GFX_QUEUE_SUPPORT_INCOMPATIBLE);
	}

	switch (setup->type) {
		case HERO_BUFFER_TYPE_VERTEX: {
			HeroVertexLayout* layout;
			result = hero_vertex_layout_get(setup->typed.vertex.layout_id, &layout);
			if (result < 0) {
				return result;
			}

			if (setup->typed.vertex.binding_idx >= layout->bindings_count) {
				return HERO_ERROR(GFX_VERTEX_BINDING_DOES_NOT_EXIST);
			}

			setup->elmt_size = layout->bindings[setup->typed.vertex.binding_idx].size;
			break;

	  	};
		case HERO_BUFFER_TYPE_INDEX:
			setup->elmt_size = hero_index_type_sizes[setup->typed.index_type];
			break;
		case HERO_BUFFER_TYPE_UNIFORM:
		case HERO_BUFFER_TYPE_STORAGE:
			if (setup->elmt_size == 0) {
				return HERO_ERROR(GFX_ELMT_SIZE_CANNOT_BE_ZERO);
			}
			break;
	}

	if (setup->elmts_count == 0) {
		return HERO_ERROR(GFX_BUFFER_ELMTS_COUNT_CANNOT_BE_ZERO);
	}

	HeroBuffer* buffer;
	result = hero_gfx_sys.backend_vtable.buffer_init(ldev, setup, id_out, &buffer);
	if (result < 0) {
		return result;
	}

	buffer->type = setup->type;
	buffer->flags = setup->flags;
	buffer->last_submitted_frame_idx = ldev->last_completed_frame_idx;
	buffer->elmts_count = setup->elmts_count;
	buffer->elmt_size = setup->elmt_size;

	switch (setup->type) {
		case HERO_BUFFER_TYPE_VERTEX:
			buffer->typed.vertex.layout_id = setup->typed.vertex.layout_id;
			buffer->typed.vertex.binding_idx = setup->typed.vertex.binding_idx;
			break;
		case HERO_BUFFER_TYPE_INDEX:
			buffer->typed.index_type = setup->typed.index_type;
			break;
		case HERO_BUFFER_TYPE_UNIFORM:
		case HERO_BUFFER_TYPE_STORAGE:
			break;
	}

	return HERO_SUCCESS;
}


HeroResult hero_buffer_deinit(HeroLogicalDevice* ldev, HeroBufferId id) {
	HeroBuffer* buffer;
	HeroResult result = hero_buffer_get(ldev, id, &buffer);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.buffer_deinit(ldev, id, buffer);
}


HeroResult hero_buffer_get(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer** out) {
	return hero_gfx_sys.backend_vtable.buffer_get(ldev, id, out);
}

HeroResult hero_buffer_resize(HeroLogicalDevice* ldev, HeroBufferId id, U64 elmts_count) {
	HeroBuffer* buffer;
	HeroResult result = hero_buffer_get(ldev, id, &buffer);
	if (result < 0) {
		return result;
	}

	if (buffer->elmts_count == elmts_count) {
		return HERO_SUCCESS;
	}

	result = hero_gfx_sys.backend_vtable.buffer_resize(ldev, buffer, elmts_count);
	if (result < 0) {
		return result;
	}

	buffer->elmts_count = elmts_count;

	return HERO_SUCCESS;
}

HeroResult hero_buffer_reserve(HeroLogicalDevice* ldev, HeroBufferId id, U64 elmts_count) {
	HeroBuffer* buffer;
	HeroResult result = hero_buffer_get(ldev, id, &buffer);
	if (result < 0) {
		return result;
	}

	if (elmts_count <= buffer->elmts_count) {
		return HERO_SUCCESS;
	}

	result = hero_gfx_sys.backend_vtable.buffer_resize(ldev, buffer, elmts_count);
	if (result < 0) {
		return result;
	}

	buffer->elmts_count = elmts_count;

	return HERO_SUCCESS;
}

HeroResult hero_buffer_map(HeroLogicalDevice* ldev, HeroBufferId id, void** addr_out) {
	HeroBuffer* buffer;
	HeroResult result = hero_buffer_get(ldev, id, &buffer);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.buffer_map(ldev, buffer, addr_out);
}

HeroResult hero_buffer_read(HeroLogicalDevice* ldev, HeroBufferId id, U64 start_idx, Uptr elmts_count, void* destination) {
	HeroBuffer* buffer;
	HeroResult result = hero_buffer_get(ldev, id, &buffer);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.buffer_read(ldev, buffer, start_idx, elmts_count, destination);
}

HeroResult hero_buffer_write(HeroLogicalDevice* ldev, HeroBufferId id, U64 start_idx, Uptr elmts_count, void** destination_out) {
	HeroBuffer* buffer;
	HeroResult result = hero_buffer_get(ldev, id, &buffer);
	if (result < 0) {
		return result;
	}

	if (elmts_count == 0) {
		*destination_out = NULL;
		return HERO_SUCCESS;
	}

	return hero_gfx_sys.backend_vtable.buffer_write(ldev, buffer, start_idx, elmts_count, destination_out);
}

// ===========================================
//
//
// Image
//
//
// ===========================================

HeroResult hero_image_init(HeroLogicalDevice* ldev, HeroImageSetup* setup, HeroImageId* id_out) {
	HeroImage* image;
	HeroResult result = hero_gfx_sys.backend_vtable.image_init(ldev, setup, id_out, &image);
	if (result < 0) {
		return result;
	}

	image->type = setup->type;
	image->format = setup->format;
	image->flags = setup->flags;
	image->samples = setup->samples;
	image->last_submitted_frame_idx = ldev->last_completed_frame_idx;
	image->flags = setup->flags;

	return HERO_SUCCESS;
}

HeroResult hero_image_deinit(HeroLogicalDevice* ldev, HeroImageId id) {
	HeroImage* image;
	HeroResult result = hero_image_get(ldev, id, &image);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.image_deinit(ldev, id, image);
}

HeroResult hero_image_get(HeroLogicalDevice* ldev, HeroImageId id, HeroImage** out) {
	return hero_gfx_sys.backend_vtable.image_get(ldev, id, out);
}


HeroResult hero_image_resize(HeroLogicalDevice* ldev, HeroImageId id, U32 width, U32 height, U32 depth, U32 mip_levels, U32 array_layers_count) {
	HeroImage* image;
	HeroResult result = hero_image_get(ldev, id, &image);
	if (result < 0) {
		return result;
	}

	image->width = width;
	image->height = height;
	image->depth = depth;
	image->mip_levels = mip_levels;
	image->array_layers_count = array_layers_count;
	return hero_gfx_sys.backend_vtable.image_resize(ldev, image, width, height, depth, mip_levels, array_layers_count);
}

HeroResult hero_image_map(HeroLogicalDevice* ldev, HeroImageId id, void** addr_out) {
	HeroImage* image;
	HeroResult result = hero_image_get(ldev, id, &image);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.image_map(ldev, image, addr_out);
}

HeroResult hero_image_read(HeroLogicalDevice* ldev, HeroImageId id, HeroImageArea* area, void* destination) {
	HeroImage* image;
	HeroResult result = hero_image_get(ldev, id, &image);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.image_read(ldev, image, area, destination);
}

HeroResult hero_image_write(HeroLogicalDevice* ldev, HeroImageId id, HeroImageArea* area, void** destination_out) {
	HeroImage* image;
	HeroResult result = hero_image_get(ldev, id, &image);
	if (result < 0) {
		return result;
	}

	HeroImageArea a;
	if (area == NULL) {
		a.offset_x = 0;
		a.offset_y = 0;
		a.offset_z = 0;
		a.width = image->width;
		a.height = image->height;
		a.depth = image->depth;
		a.mip_level = 0;
		a.array_layer = 0;
		a.array_layers_count = image->array_layers_count;
		area = &a;
	}

	return hero_gfx_sys.backend_vtable.image_write(ldev, image, area, destination_out);
}

// ===========================================
//
//
// Sampler
//
//
// ===========================================

HeroResult hero_sampler_init(HeroLogicalDevice* ldev, HeroSamplerSetup* setup, HeroSamplerId* id_out) {
	HeroSampler* sampler;
	HeroResult result = hero_gfx_sys.backend_vtable.sampler_init(ldev, setup, id_out, &sampler);
	if (result < 0) {
		return result;
	}

	sampler->mag_filter = setup->mag_filter;
	sampler->min_filter = setup->min_filter;
	sampler->mipmap_mode = setup->mipmap_mode;
	sampler->address_mode_u = setup->address_mode_u;
	sampler->address_mode_v = setup->address_mode_v;
	sampler->address_mode_w = setup->address_mode_w;
	sampler->mip_lod_bias = setup->mip_lod_bias;
	sampler->max_anisotropy = setup->max_anisotropy;
	sampler->min_lod = setup->min_lod;
	sampler->max_lod = setup->max_lod;
	sampler->compare_op = setup->compare_op;
	sampler->border_color = setup->border_color;

	return HERO_SUCCESS;
}

HeroResult hero_sampler_deinit(HeroLogicalDevice* ldev, HeroSamplerId id) {
	HeroSampler* sampler;
	HeroResult result = hero_sampler_get(ldev, id, &sampler);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.sampler_deinit(ldev, id, sampler);
}

HeroResult hero_sampler_get(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler** out) {
	return hero_gfx_sys.backend_vtable.sampler_get(ldev, id, out);
}

// ===========================================
//
//
// Shader Metadata
//
//
// ===========================================

U8 HERO_SHADER_TYPE_STAGES_COUNTS[HERO_SHADER_TYPE_COUNT] = {
	[HERO_SHADER_TYPE_GRAPHICS] = 5,
	[HERO_SHADER_TYPE_GRAPHICS_MESH] = 3,
	[HERO_SHADER_TYPE_COMPUTE] = 1,
};

HeroResult hero_shader_stages_validate(HeroShaderStages* stages) {
	switch (stages->type) {
		case HERO_SHADER_TYPE_GRAPHICS:
			if (stages->data.graphics.vertex.module_id.raw == 0) {
				return HERO_ERROR(GFX_SHADER_MISSING_VERTEX);
			}
			if (stages->data.graphics.fragment.module_id.raw == 0) {
				return HERO_ERROR(GFX_SHADER_MISSING_FRAGMENT);
			}
			break;
		case HERO_SHADER_TYPE_GRAPHICS_MESH:
			if (stages->data.graphics_mesh.mesh.module_id.raw == 0) {
				return HERO_ERROR(GFX_SHADER_MISSING_MESH);
			}
			if (stages->data.graphics_mesh.fragment.module_id.raw == 0) {
				return HERO_ERROR(GFX_SHADER_MISSING_FRAGMENT);
			}
			break;
		case HERO_SHADER_TYPE_COMPUTE:
			if (stages->data.compute.compute.module_id.raw == 0) {
				return HERO_ERROR(GFX_SHADER_MISSING_COMPUTE);
			}
			break;
	}

	return HERO_SUCCESS;
}

HeroShaderMetadata* hero_shader_metadata_alloc(HeroShaderMetadataAllocSetup* setup) {
	U16 descriptor_bindings_count = 0;
	for (U32 i = 0; i < HERO_GFX_DESCRIPTOR_SET_COUNT; i += 1) {
		descriptor_bindings_count += setup->spir_v.descriptor_bindings_counts[i];
	}

	Uptr size = sizeof(HeroShaderMetadata);

	U16 vertex_attribs_offset = 0;
	if (setup->spir_v.vertex_attribs_count) {
		size = HERO_INT_ROUND_UP_ALIGN(size, alignof(HeroVertexAttribInfo));
		vertex_attribs_offset = size;
		size += setup->spir_v.vertex_attribs_count * sizeof(HeroVertexAttribInfo);
	}

	U16 descriptor_bindings_offset = 0;
	if (descriptor_bindings_count) {
		size = HERO_INT_ROUND_UP_ALIGN(size, alignof(HeroSpirVDescriptorBinding));
		descriptor_bindings_offset = size;
		size += descriptor_bindings_count * sizeof(HeroSpirVDescriptorBinding);
	}

	HeroShaderMetadata* m = hero_alloc(hero_system_alctor, HERO_GFX_ALLOC_TAG_SHADER_METADATA, size, alignof(void*));
	if (m == NULL) {
		return NULL;
	}
	HERO_ZERO_ELMT(m);

	m->version = HERO_SHADER_METADATA_VERSION;
	m->spir_v.vertex_attribs_offset = vertex_attribs_offset;
	m->spir_v.vertex_attribs_count = setup->spir_v.vertex_attribs_count;
	m->spir_v.descriptor_bindings_offset = descriptor_bindings_offset;
	HERO_COPY_ARRAY(m->spir_v.descriptor_bindings_counts, setup->spir_v.descriptor_bindings_counts);

	return m;
}

HeroResult hero_shader_metadata_calculate(HeroLogicalDevice* ldev, HeroShaderMetadataSetup* setup, HeroShaderMetadata** out) {
	HeroResult result = hero_shader_stages_validate(setup->stages);
	if (result < 0) {
		return result;
	}

	result = hero_gfx_sys.backend_vtable.shader_metadata_calculate(ldev, setup, out);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Shader
//
//
// ===========================================

HeroResult hero_shader_module_init(HeroLogicalDevice* ldev, HeroShaderModuleSetup* setup, HeroShaderModuleId* id_out) {
	HeroShaderModule* shader_module;
	HeroResult result = hero_gfx_sys.backend_vtable.shader_module_init(ldev, setup, id_out, &shader_module);
	if (result < 0) {
		return result;
	}

	shader_module->code = setup->code;
	shader_module->code_size = setup->code_size;
	shader_module->format = setup->format;

	return HERO_SUCCESS;
}

HeroResult hero_shader_module_deinit(HeroLogicalDevice* ldev, HeroShaderModuleId id) {
	HeroShaderModule* shader_module;
	HeroResult result = hero_shader_module_get(ldev, id, &shader_module);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.shader_module_deinit(ldev, id, shader_module);
}

HeroResult hero_shader_module_get(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule** out) {
	return hero_gfx_sys.backend_vtable.shader_module_get(ldev, id, out);
}

HeroResult hero_shader_init(HeroLogicalDevice* ldev, HeroShaderSetup* setup, HeroShaderId* id_out) {
	HeroResult result = hero_shader_stages_validate(setup->stages);
	if (result < 0) {
		return result;
	}

	HeroShader* shader;
	result = hero_gfx_sys.backend_vtable.shader_init(ldev, setup, id_out, &shader);
	if (result < 0) {
		return result;
	}

	shader->metadata = setup->metadata;
	shader->stages = *setup->stages;
	return HERO_SUCCESS;
}

HeroResult hero_shader_deinit(HeroLogicalDevice* ldev, HeroShaderId id) {
	HeroShader* shader;
	HeroResult result = hero_shader_get(ldev, id, &shader);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.shader_deinit(ldev, id, shader);
}

HeroResult hero_shader_get(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader** out) {
	return hero_gfx_sys.backend_vtable.shader_get(ldev, id, out);
}

// ===========================================
//
//
// Descriptor Allocator
//
//
// ===========================================

HeroResult hero_descriptor_pool_init(HeroLogicalDevice* ldev, HeroDescriptorPoolSetup* setup, HeroDescriptorPoolId* id_out) {
	return hero_gfx_sys.backend_vtable.descriptor_pool_init(ldev, setup, id_out);
}

HeroResult hero_descriptor_pool_deinit(HeroLogicalDevice* ldev, HeroDescriptorPoolId id) {
	return hero_gfx_sys.backend_vtable.descriptor_pool_deinit(ldev, id);
}

HeroResult hero_descriptor_pool_reset(HeroLogicalDevice* ldev, HeroDescriptorPoolId id) {
	return hero_gfx_sys.backend_vtable.descriptor_pool_reset(ldev, id);
}


// ===========================================
//
//
// Shader Globals
//
//
// ===========================================

HeroResult hero_shader_globals_init(HeroLogicalDevice* ldev, HeroShaderGlobalsSetup* setup, HeroShaderGlobalsId* id_out) {
	HeroShaderGlobals* shader_globals;
	HeroResult result = hero_gfx_sys.backend_vtable.shader_globals_init(ldev, setup, id_out, &shader_globals);
	if (result < 0) {
		return result;
	}

	shader_globals->shader_id = setup->shader_id;
	shader_globals->descriptor_pool_id = setup->descriptor_pool_id;

	return HERO_SUCCESS;
}

HeroResult hero_shader_globals_deinit(HeroLogicalDevice* ldev, HeroShaderGlobalsId id) {
	HeroShaderGlobals* shader_globals;
	HeroResult result = hero_shader_globals_get(ldev, id, &shader_globals);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.shader_globals_deinit(ldev, id, shader_globals);
}

HeroResult hero_shader_globals_get(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals** out) {
	return hero_gfx_sys.backend_vtable.shader_globals_get(ldev, id, out);
}

HeroResult hero_shader_globals_set_descriptor(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data) {
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, type, data);
}

HeroResult hero_shader_globals_set_sampler(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroSamplerId sampler_id) {
	HeroDescriptorData data;
	data.image.sampler_id = sampler_id;
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_SAMPLER, &data);
}

HeroResult hero_shader_globals_set_image(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id) {
	HeroDescriptorData data;
	data.image.id = image_id;
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE, &data);
}

HeroResult hero_shader_globals_set_image_storage(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id) {
	HeroDescriptorData data;
	data.image.id = image_id;
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE, &data);
}

HeroResult hero_shader_globals_set_image_sampler(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id, HeroSamplerId sampler_id) {
	HeroDescriptorData data;
	data.image.id = image_id;
	data.image.sampler_id = sampler_id;
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &data);
}

HeroResult hero_shader_globals_set_uniform_buffer(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset) {
	HeroDescriptorData data;
	data.buffer.id = buffer_id;
	data.buffer.offset = buffer_offset;
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &data);
}

HeroResult hero_shader_globals_set_storage_buffer(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset) {
	HeroDescriptorData data;
	data.buffer.id = buffer_id;
	data.buffer.offset = buffer_offset;
	return hero_gfx_sys.backend_vtable.shader_globals_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER, &data);
}

HeroResult hero_shader_globals_update(HeroLogicalDevice* ldev, HeroShaderGlobalsId id) {
	HeroShaderGlobals* shader_globals;
	HeroResult result = hero_shader_globals_get(ldev, id, &shader_globals);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.shader_globals_update(ldev, shader_globals);
}

// ===========================================
//
//
// Render Pass Layout
//
//
// ===========================================

HeroResult hero_render_pass_layout_init(HeroLogicalDevice* ldev, HeroRenderPassLayoutSetup* setup, HeroRenderPassLayoutId* id_out) {
	if (setup->attachments_count == 0) {
		return HERO_ERROR(GFX_RENDER_PASS_LAYOUT_ATTACHMENTS_COUNT_CANNOT_BE_ZERO);
	}

	HeroRenderPassLayout* render_pass_layout;
	HeroResult result = hero_gfx_sys.backend_vtable.render_pass_layout_init(ldev, setup, id_out, &render_pass_layout);
	if (result < 0) {
		return result;
	}

	HeroAttachmentLayout* attachments = hero_alloc_array(HeroAttachmentLayout, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_LAYOUT_ATTACHMENTS, setup->attachments_count);
	if (attachments == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	HERO_COPY_ELMT_MANY(attachments, setup->attachments, setup->attachments_count);

	render_pass_layout->attachments = attachments;
	render_pass_layout->attachments_count = setup->attachments_count;

	return HERO_SUCCESS;
}

HeroResult hero_render_pass_layout_deinit(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id) {
	HeroRenderPassLayout* render_pass_layout;
	HeroResult result = hero_render_pass_layout_get(ldev, id, &render_pass_layout);
	if (result < 0) {
		return result;
	}

	hero_dealloc_array(HeroAttachmentLayout, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_LAYOUT_ATTACHMENTS, render_pass_layout->attachments, render_pass_layout->attachments_count);

	return hero_gfx_sys.backend_vtable.render_pass_layout_deinit(ldev, id, render_pass_layout);
}

HeroResult hero_render_pass_layout_get(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout** out) {
	return hero_gfx_sys.backend_vtable.render_pass_layout_get(ldev, id, out);
}

// ===========================================
//
//
// Render Pass
//
//
// ===========================================

HeroResult hero_render_pass_init(HeroLogicalDevice* ldev, HeroRenderPassSetup* setup, HeroRenderPassId* id_out) {
	HeroRenderPassLayout* render_pass_layout;
	HeroResult result = hero_render_pass_layout_get(ldev, setup->layout_id, &render_pass_layout);
	if (result < 0) {
		return result;
	}

	if (setup->attachments_count != render_pass_layout->attachments_count) {
		return HERO_ERROR(GFX_RENDER_PASS_ATTACHMENTS_MUST_MATCH_LAYOUT);
	}

	HeroRenderPass* render_pass;
	result = hero_gfx_sys.backend_vtable.render_pass_init(ldev, setup, render_pass_layout, id_out, &render_pass);
	if (result < 0) {
		return result;
	}

	HeroClearValue* clear_values = hero_alloc_array(HeroClearValue, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_CLEAR_VALUES, setup->attachments_count);
	if (clear_values == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	if (setup->attachment_clear_values) {
		HERO_COPY_ELMT_MANY(clear_values, setup->attachment_clear_values, setup->attachments_count);
	} else {
		HERO_ZERO_ELMT_MANY(clear_values, setup->attachments_count);
	}

	render_pass->layout_id = setup->layout_id;
	render_pass->attachment_clear_values = clear_values;
	render_pass->attachments_count = setup->attachments_count;

	return HERO_SUCCESS;
}

HeroResult hero_render_pass_deinit(HeroLogicalDevice* ldev, HeroRenderPassId id) {
	HeroRenderPass* render_pass;
	HeroResult result = hero_render_pass_get(ldev, id, &render_pass);
	if (result < 0) {
		return result;
	}

	hero_dealloc_array(HeroClearValue, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_CLEAR_VALUES, render_pass->attachment_clear_values, render_pass->attachments_count);

	return hero_gfx_sys.backend_vtable.render_pass_deinit(ldev, id, render_pass);
}

HeroResult hero_render_pass_get(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass** out) {
	return hero_gfx_sys.backend_vtable.render_pass_get(ldev, id, out);
}

// ===========================================
//
//
// Frame Buffer
//
//
// ===========================================

HeroResult hero_frame_buffer_init(HeroLogicalDevice* ldev, HeroFrameBufferSetup* setup, HeroFrameBufferId* id_out) {
	if (setup->attachments_count == 0) {
		return HERO_ERROR(GFX_FRAME_BUFFER_ATTACHMENTS_COUNT_CANNOT_BE_ZERO);
	}

	HeroFrameBuffer* frame_buffer;
	HeroResult result = hero_gfx_sys.backend_vtable.frame_buffer_init(ldev, setup, id_out, &frame_buffer);
	if (result < 0) {
		return result;
	}

	frame_buffer->render_pass_layout_id = setup->render_pass_layout_id;
	frame_buffer->width = setup->width;
	frame_buffer->height = setup->height;
	frame_buffer->layers = setup->layers;
	frame_buffer->attachments_count = setup->attachments_count;

	return HERO_SUCCESS;
}

HeroResult hero_frame_buffer_deinit(HeroLogicalDevice* ldev, HeroFrameBufferId id) {
	HeroFrameBuffer* frame_buffer;
	HeroResult result = hero_frame_buffer_get(ldev, id, &frame_buffer);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.frame_buffer_deinit(ldev, id, frame_buffer);
}

HeroResult hero_frame_buffer_get(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer** out) {
	return hero_gfx_sys.backend_vtable.frame_buffer_get(ldev, id, out);
}

// ===========================================
//
//
// Material Layout Cache
//
//
// ===========================================

HeroResult hero_pipeline_cache_init(HeroLogicalDevice* ldev, HeroPipelineCacheSetup* setup, HeroPipelineCacheId* id_out) {
	return hero_gfx_sys.backend_vtable.pipeline_cache_init(ldev, setup, id_out);
}

HeroResult hero_pipeline_cache_deinit(HeroLogicalDevice* ldev, HeroPipelineCacheId id) {
	return hero_gfx_sys.backend_vtable.pipeline_cache_deinit(ldev, id);
}

// ===========================================
//
//
// Material Layout
//
//
// ===========================================

HeroResult hero_pipeline_graphics_init(HeroLogicalDevice* ldev, HeroPipelineGraphicsSetup* setup, HeroPipelineId* id_out) {
	if (setup->shader_id.raw == 0) {
		return HERO_ERROR(GFX_PIPELINE_SHADER_ID_CANNOT_BE_NULL);
	}

	if (setup->render_pass_layout_id.raw == 0) {
		return HERO_ERROR(GFX_PIPELINE_RENDER_PASS_LAYOUT_ID_CANNOT_BE_NULL);
	}

	HeroPipeline* pipeline;
	HeroResult result = hero_gfx_sys.backend_vtable.pipeline_graphics_init(ldev, setup, id_out, &pipeline);
	if (result < 0) {
		return result;
	}

	pipeline->shader_id = setup->shader_id;
	pipeline->render_pass_layout_id = setup->render_pass_layout_id;
	pipeline->vertex_layout_id = setup->vertex_layout_id;

	return HERO_SUCCESS;
}

HeroResult hero_pipeline_deinit(HeroLogicalDevice* ldev, HeroPipelineId id) {
	HeroPipeline* pipeline;
	HeroResult result = hero_pipeline_get(ldev, id, &pipeline);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.pipeline_deinit(ldev, id, pipeline);
}

HeroResult hero_pipeline_get(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline** out) {
	return hero_gfx_sys.backend_vtable.pipeline_get(ldev, id, out);
}


// ===========================================
//
//
// Material
//
//
// ===========================================

HeroResult hero_material_init(HeroLogicalDevice* ldev, HeroMaterialSetup* setup, HeroMaterialId* id_out) {
	if (setup->pipeline_id.raw == 0) {
		return HERO_ERROR(GFX_MATERIAL_PIPELINE_ID_CANNOT_BE_NULL);
	}

	if (setup->shader_globals_id.raw == 0) {
		return HERO_ERROR(GFX_MATERIAL_SHADER_GLOBALS_ID_CANNOT_BE_NULL);
	}

	if (setup->descriptor_pool_id.raw == 0) {
		return HERO_ERROR(GFX_MATERIAL_DESCRIPTOR_POOL_ID_CANNOT_BE_NULL);
	}

	HeroMaterial* material;
	HeroResult result = hero_gfx_sys.backend_vtable.material_init(ldev, setup, id_out, &material);
	if (result < 0) {
		return result;
	}

	material->pipeline_id = setup->pipeline_id;
	material->shader_globals_id = setup->shader_globals_id;
	material->descriptor_pool_id = setup->descriptor_pool_id;

	return HERO_SUCCESS;
}

HeroResult hero_material_deinit(HeroLogicalDevice* ldev, HeroMaterialId id) {
	HeroMaterial* material;
	HeroResult result = hero_material_get(ldev, id, &material);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.material_deinit(ldev, id, material);
}

HeroResult hero_material_get(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial** out) {
	return hero_gfx_sys.backend_vtable.material_get(ldev, id, out);
}

HeroResult hero_material_set_descriptor(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data) {
	return hero_gfx_sys.backend_vtable.material_set_descriptor(ldev, id, binding_idx, elmt_idx, type, data);
}

HeroResult hero_material_set_sampler(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroSamplerId sampler_id) {
	HeroDescriptorData data;
	data.image.sampler_id = sampler_id;
	return hero_gfx_sys.backend_vtable.material_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_SAMPLER, &data);
}

HeroResult hero_material_set_image(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id) {
	HeroDescriptorData data;
	data.image.id = image_id;
	return hero_gfx_sys.backend_vtable.material_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE, &data);
}

HeroResult hero_material_set_image_sampler(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroImageId image_id, HeroSamplerId sampler_id) {
	HeroDescriptorData data;
	data.image.id = image_id;
	data.image.sampler_id = sampler_id;
	return hero_gfx_sys.backend_vtable.material_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &data);
}

HeroResult hero_material_set_uniform_buffer(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset) {
	HeroDescriptorData data;
	data.buffer.id = buffer_id;
	data.buffer.offset = buffer_offset;
	return hero_gfx_sys.backend_vtable.material_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &data);
}

HeroResult hero_material_set_storage_buffer(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset) {
	HeroDescriptorData data;
	data.buffer.id = buffer_id;
	data.buffer.offset = buffer_offset;
	return hero_gfx_sys.backend_vtable.material_set_descriptor(ldev, id, binding_idx, elmt_idx, HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER, &data);
}

HeroResult hero_material_update(HeroLogicalDevice* ldev, HeroMaterialId id) {
	HeroMaterial* material;
	HeroResult result = hero_material_get(ldev, id, &material);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.material_update(ldev, material);
}

// ===========================================
//
//
// Swapchain
//
//
// ===========================================

HeroResult hero_swapchain_init(HeroLogicalDevice* ldev, HeroSwapchainSetup* setup, HeroSwapchainId* id_out, HeroSwapchain** out) {
	return hero_gfx_sys.backend_vtable.swapchain_init(ldev, setup, id_out, out);
}

HeroResult hero_swapchain_deinit(HeroLogicalDevice* ldev, HeroSwapchainId id) {
	HeroSwapchain* swapchain;
	HeroResult result = hero_swapchain_get(ldev, id, &swapchain);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.swapchain_deinit(ldev, id, swapchain);
}

HeroResult hero_swapchain_get(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** out) {
	return hero_gfx_sys.backend_vtable.swapchain_get(ldev, id, out);
}

HeroResult hero_swapchain_next_image(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** swapchain_out, U32* next_image_idx_out) {
	HeroSwapchain* swapchain;
	HeroResult result = hero_swapchain_get(ldev, id, &swapchain);
	if (result < 0) {
		return result;
	}

	*swapchain_out = swapchain;
	return hero_gfx_sys.backend_vtable.swapchain_next_image(ldev, swapchain, next_image_idx_out);
}

// ===========================================
//
//
// Command Pool
//
//
// ===========================================
/*

HeroResult hero_command_pool_init(HeroLogicalDevice* ldev, HeroCommandPoolSetup* setup, HeroCommandPoolId* id_out) {
	return hero_gfx_sys.backend_vtable.command_pool_init(ldev, setup, id_out);
}

HeroResult hero_command_pool_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId id) {
	return hero_gfx_sys.backend_vtable.command_pool_deinit(ldev, id);
}

HeroResult hero_command_pool_reset(HeroLogicalDevice* ldev, HeroCommandPoolId id) {
	return hero_gfx_sys.backend_vtable.command_pool_reset(ldev, id);
}


// ===========================================
//
//
// Command Buffer
//
//
// ===========================================

HeroResult hero_command_buffer_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId pool_id, HeroCommandPoolBufferId buffer_id) {
	return hero_gfx_sys.backend_vtable.command_buffer_deinit(ldev, pool_id, buffer_id);
}

HeroResult hero_command_recorder_start(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandRecorder** out) {
	return hero_gfx_sys.backend_vtable.command_recorder_start(ldev, command_pool_id, false, out);
}

HeroResult hero_command_recorder_start_static(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandRecorder** out) {
	return hero_gfx_sys.backend_vtable.command_recorder_start(ldev, command_pool_id, true, out);
}

HeroResult hero_command_recorder_end(HeroCommandRecorder* command_recorder, HeroCommandPoolBufferId* id_out) {
	return hero_gfx_sys.backend_vtable.command_recorder_end(command_recorder, id_out);
}

HeroResult hero_cmd_render_pass_start(HeroCommandRecorder* command_recorder, HeroRenderPassId render_pass_id, HeroFrameBufferId frame_buffer_id, HeroViewport* viewport, HeroUAabb* scissor) {
	if (command_recorder->render_pass_id.raw) {
		return HERO_ERROR(ALREADY_STARTED);
	}

	HeroResult result = hero_gfx_sys.backend_vtable.cmd_render_pass_start(command_recorder, render_pass_id, frame_buffer_id, viewport, scissor);
	if (result < 0) {
		return result;
	}

	command_recorder->render_pass_id = render_pass_id;

	return HERO_SUCCESS;
}

HeroResult hero_cmd_render_pass_end(HeroCommandRecorder* command_recorder) {
	if (command_recorder->render_pass_id.raw == 0) {
		return HERO_ERROR(NOT_STARTED);
	}
	HeroResult result = hero_gfx_sys.backend_vtable.cmd_render_pass_end(command_recorder);
	if (result < 0) {
		return result;
	}

	command_recorder->render_pass_id.raw = 0;

	return HERO_SUCCESS;
}

HeroResult hero_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroMaterialId material_id) {
	if (command_recorder->material_id.raw) {
		return HERO_ERROR(ALREADY_STARTED);
	}

	HeroResult result = hero_gfx_sys.backend_vtable.cmd_draw_start(command_recorder, material_id);
	if (result < 0) {
		return result;
	}

	command_recorder->material_id = material_id;
	return HERO_SUCCESS;
}

HeroResult hero_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_start_idx, U32 vertices_count) {
	if (command_recorder->material_id.raw == 0) {
		return HERO_ERROR(NOT_STARTED);
	}

	HeroResult result = hero_gfx_sys.backend_vtable.cmd_draw_end_vertexed(command_recorder, vertices_start_idx, vertices_count);
	if (result < 0) {
		return result;
	}

	command_recorder->material_id.raw = 0;
	return HERO_SUCCESS;
}

HeroResult hero_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroBufferId index_buffer_id, U32 indices_start_idx, U32 indices_count, U32 vertices_start_idx) {
	if (command_recorder->material_id.raw == 0) {
		return HERO_ERROR(NOT_STARTED);
	}

	HeroResult result = hero_gfx_sys.backend_vtable.cmd_draw_end_indexed(command_recorder, index_buffer_id, indices_start_idx, indices_count, vertices_start_idx);
	if (result < 0) {
		return result;
	}

	command_recorder->material_id.raw = 0;
	return HERO_SUCCESS;
}

HeroResult hero_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroBufferId buffer_id, U32 binding, U64 offset) {
	if (command_recorder->material_id.raw == 0) {
		return HERO_ERROR(NOT_STARTED);
	}

	return hero_gfx_sys.backend_vtable.cmd_draw_set_vertex_buffer(command_recorder, buffer_id, binding, offset);
}

HeroResult hero_cmd_draw_set_push_constants(HeroCommandRecorder* command_recorder, void* data, U32 offset, U32 size) {
	if (command_recorder->material_id.raw == 0) {
		return HERO_ERROR(NOT_STARTED);
	}

	return hero_gfx_sys.backend_vtable.cmd_draw_set_push_constants(command_recorder, data, offset, size);
}

HeroResult hero_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count) {
	if (command_recorder->material_id.raw == 0) {
		return HERO_ERROR(NOT_STARTED);
	}

	return hero_gfx_sys.backend_vtable.cmd_draw_set_instances(command_recorder, instances_start_idx, instances_count);
}

HeroResult hero_cmd_draw_set_sampler(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroSamplerId sampler_id);
HeroResult hero_cmd_draw_set_texture(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroImageId texture_id);
HeroResult hero_cmd_draw_set_uniform_buffer(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_cmd_draw_set_storage_buffer(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, HeroBufferId buffer_id, U64 buffer_offset);
HeroResult hero_cmd_draw_set_dynamic_descriptor_offset(HeroCommandRecorder* command_recorder, U16 binding_idx, U16 elmt_idx, U32 dynamic_offset);
HeroResult hero_cmd_compute_dispatch(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, HeroShaderGlobalsId shader_globals_id, U32 group_count_x, U32 group_count_y, U32 group_count_z) {
	return hero_gfx_sys.backend_vtable.cmd_compute_dispatch(command_recorder, compute_shader_id, shader_globals_id, group_count_x, group_count_y, group_count_z);
}

*/
// ===========================================
//
//
// Image Info
//
//
// ===========================================

HeroResult hero_image_info_register(HeroLogicalDevice* ldev, HeroImageInfoFlags flags, HeroImageInfo* info, HeroImageInfoId* id_out) {
	HeroResult result;
	HeroImageInfo* dst;
	result = hero_object_pool(HeroImageInfo, alloc)(&ldev->image_info_pool, flags, &dst, id_out);
	if (result < 0) {
		return result;
	}

	*dst = *info;
	return HERO_SUCCESS;
}

HeroResult hero_image_info_deregister(HeroLogicalDevice* ldev, HeroImageInfoId id) {
	return hero_object_pool(HeroImageInfo, dealloc)(&ldev->image_info_pool, id);
}

HeroResult _hero_image_info_get(HeroLogicalDevice* ldev, HeroImageInfoId id, HeroImageInfo** out) {
	return hero_object_pool(HeroImageInfo, get)(&ldev->image_info_pool, id, out);
}

void hero_image_info_update_swapchain_sizes(HeroLogicalDevice* ldev) {
	HeroImageInfoId image_info_id = {0};
	HeroImageInfo* image_info;
	while (hero_object_pool(HeroImageInfo, iter_next)(&ldev->image_info_pool, &image_info_id, &image_info) != HERO_SUCCESS_FINISHED) {
CONTINUE: {}
		if (image_info->swapchain_id.raw) {
			HeroSwapchain* swapchain;
			HeroResult result = hero_swapchain_get(ldev, image_info->swapchain_id, &swapchain);
			if (result < 0) {
				//
				// swapchain no longer exists, so delete the image info and continue on.
				//

				HeroImageInfoId next_image_info_id = {0};
				HeroImageInfo* next_image_info;
				bool is_finished = hero_object_pool(HeroImageInfo, iter_next)(&ldev->image_info_pool, &next_image_info_id, &next_image_info) != HERO_SUCCESS_FINISHED;

				result = hero_object_pool(HeroImageInfo, dealloc)(&ldev->image_info_pool, image_info_id);
				HERO_RESULT_ASSERT(result);

				if (is_finished) {
					break;
				}

				image_info_id = next_image_info_id;
				image_info = next_image_info;
				goto CONTINUE;
			}

			//
			// we have a swapchain image so lets set the actual size
			//

			image_info->width = (U32)((F32)swapchain->width * image_info->swapchain_based_width_scale_ratio);
			image_info->height = (U32)((F32)swapchain->height * image_info->swapchain_based_height_scale_ratio);
		}
	}
}

// ===========================================
//
//
// Render Graph
//
//
// ===========================================

bool hero_pass_resource_type_is_compatible(HeroPassResourceType a, HeroPassResourceType b) {
	// TODO we might want so buffer types to be compatible with some image types
	return HERO_PASS_RESOURCE_TYPE_IS_BUFFER(a) == HERO_PASS_RESOURCE_TYPE_IS_BUFFER(b);
}

const char* hero_pass_resource_type_strings[HERO_PASS_RESOURCE_TYPE_COUNT] = {
	[HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT] = "INPUT_ATTACHMENT",
	[HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT] = "COLOR_OUTPUT_ATTACHMENT",
	[HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT] = "DEPTH_STENCIL_ATTACHMENT",
	[HERO_PASS_RESOURCE_TYPE_RO_IMAGE] = "RO_IMAGE",
	[HERO_PASS_RESOURCE_TYPE_RW_IMAGE] = "RW_IMAGE",
	[HERO_PASS_RESOURCE_TYPE_RO_BUFFER] = "RO_BUFFER",
	[HERO_PASS_RESOURCE_TYPE_RW_BUFFER] = "RW_BUFFER",
	[HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER] = "VERTEX_BUFFER",
	[HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER] = "INDEX_BUFFER",
};

void hero_image_info_init_swapchain_sized(HeroImageInfo* info, HeroImageFormat format, HeroSwapchainId swapchain_id);
void hero_image_info_init_swapchain_relative(HeroImageInfo* info, HeroImageFormat format, HeroSwapchainId swapchain_id, F32 width_multiple, F32 height_multiple);
void hero_image_info_init_2d(HeroImageInfo* info, HeroImageFormat format, U32 width, U32 height);
void hero_image_info_init_2d_array(HeroImageInfo* info, HeroImageFormat format, U32 width, U32 height, U16 array_layers_count);
void hero_image_info_init_3d(HeroImageInfo* info, HeroImageFormat format, U32 width, U32 height, U32 depth, U8 mip_levels_count);

HeroResult hero_render_graph_init(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out) {
	HeroRenderGraph* render_graph;
	HeroResult result = hero_gfx_sys.backend_vtable.render_graph_init(ldev, setup, id_out, &render_graph);
	if (result < 0) {
		return result;
	}

	//
	// allocate the resource pool
	U16 resources_cap = 0;
	for_range(resource_type, 0, HERO_PASS_RESOURCE_TYPE_COUNT) {
		HeroPassResourcePool* pool = &render_graph->resource_pools[resource_type];
		Uptr cap = setup->resource_pool_caps[resource_type];
		if (cap) {
			pool->data = hero_alloc(hero_system_alctor, 0, sizeof(HeroPassResource) * cap, alignof(HeroPassResource));
			if (!pool->data) {
				return HERO_ERROR(ALLOCATION_FAILURE);
			}
		}
		pool->count = 0;
		pool->cap = cap;
		resources_cap += cap;
	}

	//
	// allocate the passes
	{
		if (setup->passes_cap == 0) {
			return HERO_ERROR(GFX_GRAPH_CANNOT_HAVE_ZERO_PASSES);
		}

		render_graph->passes = hero_alloc_array(HeroPass, hero_system_alctor, 0, setup->passes_cap);
		if (!render_graph->passes) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		render_graph->passes_count = 0;
		render_graph->passes_cap = setup->passes_cap;
	}

	//
	// allocate the viewports
	{
		setup->viewports_cap = HERO_MAX(setup->viewports_cap, setup->passes_cap);
		render_graph->viewports = hero_alloc_array(HeroPassViewport, hero_system_alctor, 0, setup->viewports_cap);
		if (!render_graph->viewports) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(render_graph->viewports, setup->viewports_cap);

		render_graph->viewports_count = 0;
		render_graph->viewports_cap = setup->viewports_cap;
	}

	render_graph->name = setup->name;

	return HERO_SUCCESS;
}

HeroResult hero_render_graph_deinit(HeroLogicalDevice* ldev, HeroRenderGraphId id) {
	HeroRenderGraph* render_graph;
	HeroResult result = hero_render_graph_get(ldev, id, &render_graph);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.render_graph_deinit(ldev, id, render_graph);
}

HeroResult hero_render_graph_get(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out) {
	return hero_gfx_sys.backend_vtable.render_graph_get(ldev, id, out);
}

void hero_render_graph_clear(HeroRenderGraph* render_graph) {
	render_graph->passes_count = 0;
	render_graph->hash = 0;

	for_range(resource_type, 0, HERO_PASS_RESOURCE_TYPE_COUNT) {
		HeroPassResourcePool* pool = &render_graph->resource_pools[resource_type];
		pool->count = 0;
	}

	HERO_ZERO_ELMT_MANY(render_graph->viewports, render_graph->viewports_cap);
}

void hero_render_graph_pass_start(HeroRenderGraph* render_graph, HeroPassSetup* setup) {
	HERO_DEBUG_ASSERT(render_graph->building_pass_id == 0, "hero_render_graph_pass_end must be called before hero_render_graph_pass_start is called again");
	HERO_DEBUG_ASSERT(render_graph->passes_count >= render_graph->passes_cap, "we have exceed the maximum number of passes '%u' that render graph '%s' was allocated with", render_graph->passes_cap, render_graph->name);

#if HERO_DEBUG_ASSERTIONS
	for_range(idx, 0, render_graph->passes_count) {
		HeroPass* other_pass = &render_graph->passes[idx];
		HERO_ASSERT(other_pass->pass_enum != setup->pass_enum, "render graph '%s' pass '%s' has the same pass enum '%u' as pass '%s'", render_graph->name, setup->name, setup->pass_enum, other_pass->name);
	}
#endif

	HeroPass* pass = &render_graph->passes[render_graph->passes_count];
	HERO_DEBUG_ASSERT(pass->render_pass_layout_id.raw && setup->viewports_count > 0, "viewports_count must be a non zero value in render graph '%s' was allocated with", render_graph->name);

	//
	// create the ranges the resources in the resource pools
	for_range(resource_type, 0, HERO_PASS_RESOURCE_TYPE_COUNT) {
		HeroPassResourcePool* pool = &render_graph->resource_pools[resource_type];
		HeroRangeU16* range = &pass->resource_ranges_by_type[resource_type];
		range->start_idx = pool->count;
		range->end_idx = pool->count;
	}

	pass->name = setup->name;
	pass->pass_enum = setup->pass_enum;
	pass->render_pass_layout_id = setup->render_pass_layout_id;
	pass->userdata = setup->userdata;
	pass->execution_unit_idx = HERO_EXECUTION_UNIT_IDX_INVALID;
	pass->flags = 0;
	pass->viewports_range.start_idx = render_graph->viewports_count;
	pass->viewports_range.end_idx = render_graph->viewports_count + setup->viewports_count;
	render_graph->viewports_count += setup->viewports_count;

	render_graph->passes_count += 1;
	render_graph->building_pass_id = render_graph->passes_count;
}

void hero_render_graph_pass_end(HeroRenderGraph* render_graph) {
	HERO_DEBUG_ASSERT(render_graph->building_pass_id != 0, "hero_render_graph_pass_start must be called before hero_render_graph_pass_end");
	render_graph->building_pass_id = 0;
	render_graph->build_prev_pool_resource_id = 0;
}

HeroPassViewport* _hero_pass_get_viewport(HeroRenderGraph* render_graph, U16 viewport_idx) {
	HERO_DEBUG_ASSERT(render_graph->building_pass_id != 0, "hero_render_graph_pass_start must be called before adding a resource");

	HeroPass* pass = &render_graph->passes[render_graph->building_pass_id - 1];
	U16 viewports_count = pass->viewports_range.end_idx - pass->viewports_range.start_idx;
	HERO_ASSERT_ARRAY_BOUNDS(viewport_idx, viewports_count);

	return &render_graph->viewports[pass->viewports_range.start_idx + viewport_idx];
}

HeroViewport* hero_pass_set_viewport(HeroRenderGraph* render_graph, U16 viewport_idx) {
	HeroPassViewport* viewport = _hero_pass_get_viewport(render_graph, viewport_idx);
	viewport->has_set_viewport = true;
	return &viewport->viewport;
}

HeroUAabb* hero_pass_set_scissor(HeroRenderGraph* render_graph, U16 viewport_idx) {
	HeroPassViewport* viewport = _hero_pass_get_viewport(render_graph, viewport_idx);
	viewport->has_set_scissor = true;
	return &viewport->scissor;
}

void hero_pass_add_resource(HeroRenderGraph* render_graph, HeroPassResourceId id, void* data, HeroPassResourceType resource_type) {
	hero_pass_add_resource_linked(render_graph, id, HERO_PASS_ENUM_INVALID, 0, data, resource_type);
}

void hero_pass_add_input_attachment(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageInfoId image_info_id) {
	hero_pass_add_resource(render_graph, id, &image_info_id, HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT);
}

void hero_pass_add_color_output_attachment(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageInfoId image_info_id) {
	hero_pass_add_resource(render_graph, id, &image_info_id, HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT);
}

void hero_pass_add_depth_stencil_attachment(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageInfoId image_info_id) {
	hero_pass_add_resource(render_graph, id, &image_info_id, HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT);
}

void hero_pass_add_ro_image(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageInfoId image_info_id) {
	hero_pass_add_resource(render_graph, id, &image_info_id, HERO_PASS_RESOURCE_TYPE_RO_IMAGE);
}

void hero_pass_add_rw_image(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageInfoId image_info_id) {
	hero_pass_add_resource(render_graph, id, &image_info_id, HERO_PASS_RESOURCE_TYPE_RW_IMAGE);
}

void hero_pass_add_ro_buffer(HeroRenderGraph* render_graph, HeroPassResourceId id, U64 size) {
	hero_pass_add_resource(render_graph, id, &size, HERO_PASS_RESOURCE_TYPE_RO_BUFFER);
}

void hero_pass_add_rw_buffer(HeroRenderGraph* render_graph, HeroPassResourceId id, U64 size) {
	hero_pass_add_resource(render_graph, id, &size, HERO_PASS_RESOURCE_TYPE_RW_BUFFER);
}

void hero_pass_add_vertex_buffer(HeroRenderGraph* render_graph, HeroPassResourceId id, U64 size) {
	hero_pass_add_resource(render_graph, id, &size, HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER);
}

void hero_pass_add_index_buffer(HeroRenderGraph* render_graph, HeroPassResourceId id, U64 size) {
	hero_pass_add_resource(render_graph, id, &size, HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER);
}

void hero_pass_add_resource_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id, void* data, HeroPassResourceType resource_type) {
	HERO_DEBUG_ASSERT(render_graph->building_pass_id != 0, "hero_render_graph_pass_start must be called before adding a resource");

	HeroPassResourcePool* pool = &render_graph->resource_pools[resource_type];
	HERO_DEBUG_ASSERT(pool->count >= pool->cap, "we have exceed the maximum number of %s resources '%u' that render graph '%s' was allocated with", hero_pass_resource_type_strings[resource_type], pool->cap, render_graph->name);

	HeroPass* pass = &render_graph->passes[render_graph->building_pass_id - 1];
	HeroPassResource* resource = &pool->data[pool->count];
	resource->id = id;
	resource->type = resource_type;
	resource->flags = 0;
	resource->link_pass_enum = link_pass_enum;
	resource->link_id = link_id;
	resource->physical_resource_idx = HERO_PHYSICAL_RESOURCE_IDX_INVALID;
	resource->link_to_by_min_execution_unit_idx = HERO_EXECUTION_UNIT_IDX_INVALID;
	resource->link_chain_length = 0;
	resource->nearest_link_chain_mutation = HERO_NEAREST_LINK_CHAIN_MUTATION_INVALID;
	if (HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource_type)) {
		resource->data.buffer_size = *(U64*)data;
	} else {
		resource->data.image_info_id = *(HeroImageInfoId*)data;
		if (hero_object_id(HeroImageInfoId, user_bits)(resource->data.image_info_id) & HERO_IMAGE_INFO_FLAGS_IS_SWAPCHAIN) {
			pass->flags |= HERO_PASS_FLAGS_HAS_SWAPCHAIN;
		}
	}

	{
#if HERO_DEBUG_ASSERTIONS
		for_range(other_resource_type, 0, HERO_PASS_RESOURCE_TYPE_COUNT) {
			HeroPassResourcePool* other_resource_pool = &render_graph->resource_pools[other_resource_type];
			HeroRangeU16 range = pass->resource_ranges_by_type[other_resource_type];
			for_range(i, range.start_idx, range.end_idx) {
				HeroPassResourceId other_resource_id = other_resource_pool->data[i].id;
				HERO_ASSERT(other_resource_id != id, "render graph '%s' pass '%s' has a duplicate id", render_graph->name, pass->name, id);
			}
		}
#endif
	}

	pass->resource_ranges_by_type[resource_type].end_idx += 1;

	pool->count += 1;
	render_graph->build_prev_pool_resource_id = pool->count;
	render_graph->build_prev_resource_type = resource_type;
}

void hero_pass_add_input_attachment_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT);
}

void hero_pass_add_color_output_attachment_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT);
}

void hero_pass_add_depth_stencil_attachment_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT);
}

void hero_pass_add_ro_image_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_RO_IMAGE);
}

void hero_pass_add_rw_image_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_RW_IMAGE);
}

void hero_pass_add_ro_buffer_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_RO_BUFFER);
}

void hero_pass_add_rw_buffer_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_RW_BUFFER);
}

void hero_pass_add_vertex_buffer_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER);
}

void hero_pass_add_index_buffer_linked(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroPassEnum link_pass_enum, HeroPassResourceId link_id) {
	hero_pass_add_resource_linked(render_graph, id, link_pass_enum, link_id, NULL, HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER);
}

void hero_pass_and_make_flags(HeroRenderGraph* render_graph, HeroPassResourceFlags flags) {
	HERO_DEBUG_ASSERT(render_graph->building_pass_id != 0, "hero_render_graph_pass_start must be called before adding a resource");
	HERO_DEBUG_ASSERT(render_graph->build_prev_pool_resource_id != 0, "hero_pass_and_make_* functions can only be used directly after adding a resource with a hero_pass_add_* function");

	HeroPassResourcePool* pool = &render_graph->resource_pools[render_graph->build_prev_resource_type];
	HeroPassResource* resource = &pool->data[render_graph->build_prev_pool_resource_id - 1];
	resource->flags |= flags;

	if (flags & HERO_PASS_RESOURCE_FLAGS_IS_CPU_READABLE) {
		HeroPass* pass = &render_graph->passes[render_graph->building_pass_id - 1];
			pass->flags |= HERO_PASS_FLAGS_HAS_CPU_READABLE;
	}
}

void hero_pass_and_make_persistent(HeroRenderGraph* render_graph) {
	hero_pass_and_make_flags(render_graph, HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT);
}

void hero_pass_and_make_cpu_readable(HeroRenderGraph* render_graph) {
	hero_pass_and_make_flags(render_graph, HERO_PASS_RESOURCE_FLAGS_IS_CPU_READABLE);
}

void hero_pass_and_make_cpu_writeable(HeroRenderGraph* render_graph) {
	hero_pass_and_make_flags(render_graph, HERO_PASS_RESOURCE_FLAGS_IS_CPU_WRITEABLE);
}

void hero_pass_and_make_clear(HeroRenderGraph* render_graph, HeroClearValue clear_value) {
	hero_pass_and_make_flags(render_graph, HERO_PASS_RESOURCE_FLAGS_IS_CLEARED);

	HeroPassResourcePool* pool = &render_graph->resource_pools[render_graph->build_prev_resource_type];
	HeroPassResource* resource = &pool->data[render_graph->build_prev_pool_resource_id - 1];
	resource->clear_value = clear_value;
	HERO_DEBUG_ASSERT(!HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type) && HERO_PASS_RESOURCE_TYPE_IS_MUTABLE(resource->type), "we can only clear a mutable image resource and %s is not a mutable image", hero_pass_resource_type_strings[resource->type]);
}

void hero_pass_read_buffer(HeroRenderGraph* render_graph, HeroPassResourceId id, U64 start_idx, U64 end_idx, void* dst);
void hero_pass_write_buffer(HeroRenderGraph* render_graph, HeroPassResourceId id, U64 start_idx, U64 end_idx, void** dst_out);

void hero_pass_read_image(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageArea* area, void* dst);
void hero_pass_write_image(HeroRenderGraph* render_graph, HeroPassResourceId id, HeroImageArea* area, void** dst_out);

// ===========================================
//
//
// Frame Graph
//
//
// ===========================================

typedef struct _HeroCommandRecorder _HeroCommandRecorder;
struct _HeroCommandRecorder {
	HeroCommandRecorder public_;
	HeroAtomic(bool)    is_in_use;
	union {
		HeroCommandRecorderVulkan vulkan;
	};
};

thread_local static _HeroCommandRecorder _hero_command_recorder_thread_local;

HeroResult hero_frame_graph_init(HeroLogicalDevice* ldev, HeroFrameGraphSetup* setup, HeroFrameGraphId* id_out) {
	HeroFrameGraph* frame_graph;
	HeroResult result = hero_gfx_sys.backend_vtable.frame_graph_init(ldev, setup, id_out, &frame_graph);
	if (result < 0) {
		return result;
	}

	//
	// allocate the pass resource metadta
	{
		if (setup->resources_cap == 0) {
			return HERO_ERROR(GFX_GRAPH_CANNOT_HAVE_ZERO_RESOURCES);
		}

		result = hero_free_ranges_init(&frame_graph->persistent_images_memory_free_ranges, 0, hero_system_alctor, 0);
		if (result < 0) {
			return result;
		}

		result = hero_free_ranges_init(&frame_graph->persistent_buffers_memory_free_ranges, 0, hero_system_alctor, 0);
		if (result < 0) {
			return result;
		}

		result = hero_free_ranges_init(&frame_graph->images_memory_free_ranges, 0, hero_system_alctor, 0);
		if (result < 0) {
			return result;
		}

		result = hero_free_ranges_init(&frame_graph->buffers_memory_free_ranges, 0, hero_system_alctor, 0);
		if (result < 0) {
			return result;
		}

		Uptr bitset_words_count = DIV_ROUND_UP(setup->physical_resources_cap, 64);
		frame_graph->physical_resources_that_are_unused_bitset = hero_alloc_array(U64, hero_system_alctor, 0, bitset_words_count);
		if (!frame_graph->physical_resources_that_are_unused_bitset) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->physical_resources_that_are_unused_bitset, bitset_words_count);

		frame_graph->physical_resources_allocated_bitset = hero_alloc_array(U64, hero_system_alctor, 0, bitset_words_count);
		if (!frame_graph->physical_resources_allocated_bitset) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->physical_resources_allocated_bitset, bitset_words_count);

		frame_graph->physical_resources_newly_allocated_bitset = hero_alloc_array(U64, hero_system_alctor, 0, bitset_words_count);
		if (!frame_graph->physical_resources_newly_allocated_bitset) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->physical_resources_newly_allocated_bitset, bitset_words_count);

		frame_graph->physical_resource_ids = hero_alloc_array(HeroResourceId, hero_system_alctor, 0, setup->physical_resources_cap);
		if (!frame_graph->physical_resource_ids) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph->physical_resources = hero_alloc_array(HeroPassPhysicalResource, hero_system_alctor, 0, setup->physical_resources_cap);
		if (!frame_graph->physical_resources) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph->pass_resource_ids = hero_alloc_array(HeroPassResourceId, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph->pass_resource_ids) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph->pass_resources = hero_alloc_array(HeroPassResource, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph->pass_resources) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph->pass_resources_cap = setup->resources_cap;
		frame_graph->physical_resources_cap = setup->physical_resources_cap;
	}

	//
	// allocate the passes
	{
		if (setup->passes_cap == 0) {
			return HERO_ERROR(GFX_GRAPH_CANNOT_HAVE_ZERO_PASSES);
		}

		Uptr bitset_words_count = DIV_ROUND_UP(setup->passes_cap, 64);
		frame_graph->passes_that_are_unused_bitset = hero_alloc_array(U64, hero_system_alctor, 0, bitset_words_count);
		if (!frame_graph->passes_that_are_unused_bitset) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->passes_that_are_unused_bitset, bitset_words_count);

		frame_graph->passes_allocated_bitset = hero_alloc_array(U64, hero_system_alctor, 0, bitset_words_count);
		if (!frame_graph->passes_allocated_bitset) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->passes_allocated_bitset, bitset_words_count);

		frame_graph->passes_newly_allocated_bitset = hero_alloc_array(U64, hero_system_alctor, 0, bitset_words_count);
		if (!frame_graph->passes_newly_allocated_bitset) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->passes_newly_allocated_bitset, bitset_words_count);

		frame_graph->passes = hero_alloc_array(HeroPass, hero_system_alctor, 0, setup->passes_cap);
		if (!frame_graph->passes) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph->passes_cap = setup->passes_cap;
	}

	//
	// allocate the execution units
	{
		frame_graph->execution_units_pass_enums = hero_alloc_array(HeroPassEnum, hero_system_alctor, 0, setup->passes_cap);
		if (!frame_graph->execution_units_pass_enums) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph->execution_units_pass_ranges = hero_alloc_array(HeroRangeU16, hero_system_alctor, 0, setup->passes_cap);
		if (!frame_graph->execution_units_pass_ranges) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
	}

	//
	// allocate the viewports
	{
		setup->viewports_cap = HERO_MAX(setup->viewports_cap, setup->passes_cap);
		frame_graph->viewports = hero_alloc_array(HeroPassViewport, hero_system_alctor, 0, setup->viewports_cap);
		if (!frame_graph->viewports) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(frame_graph->viewports, setup->viewports_cap);

		frame_graph->viewports_count = 0;
		frame_graph->viewports_cap = setup->viewports_cap;
	}

	frame_graph->name = setup->name;
	frame_graph->persistent_images_memory_pool_size = setup->persistent_images_memory_pool_size;
	frame_graph->persistent_buffers_memory_pool_size = setup->persistent_buffers_memory_pool_size;
	frame_graph->images_memory_pool_size = setup->images_memory_pool_size;
	frame_graph->buffers_memory_pool_size = setup->buffers_memory_pool_size;

	return HERO_SUCCESS;
}

HeroResult hero_frame_graph_deinit(HeroLogicalDevice* ldev, HeroFrameGraphId id) {
	HeroFrameGraph* frame_graph;
	HeroResult result = hero_frame_graph_get(ldev, id, &frame_graph);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.frame_graph_deinit(ldev, id, frame_graph);
}

HeroResult hero_frame_graph_get(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroFrameGraph** out) {
	return hero_gfx_sys.backend_vtable.frame_graph_get(ldev, id, out);
}

HeroPassPhysicalResource* _hero_frame_graph_physical_resource_find_or_insert(HeroFrameGraph* frame_graph, HeroResourceId resource_id) {
	HeroPassPhysicalResource* physical_resource;
	for_range(physical_resource_idx, 0, frame_graph->physical_resources_count) {
		if (frame_graph->physical_resource_ids[physical_resource_idx] == resource_id) {
			physical_resource = &frame_graph->physical_resources[physical_resource_idx];
			goto END;
		}
	}

	//
	// find the next resource index by look through the allocated bitset
	Uptr physical_resource_idx = (Uptr)-1;
	HERO_DEBUG_ASSERT(
		hero_bitset_array_iter_next_zero64(frame_graph->physical_resources_allocated_bitset, frame_graph->physical_resources_cap, &physical_resource_idx),
		"frame graph '%s' physical resources capacity of '%u' has been exceeded", frame_graph->name, frame_graph->physical_resources_cap
	);
	frame_graph->physical_resources_count += 1;

	frame_graph->physical_resource_ids[physical_resource_idx] = resource_id;

	physical_resource = &frame_graph->physical_resources[physical_resource_idx];

	physical_resource->id = resource_id;
	physical_resource->parent_physical_resource_idx = HERO_PHYSICAL_RESOURCE_IDX_INVALID;
	physical_resource->mem_range = (HeroRange){0};

	//
	// set a bit to say it is allocated and what not
	hero_bitset_array_set64(frame_graph->physical_resources_allocated_bitset, physical_resource_idx);
	hero_bitset_array_set64(frame_graph->physical_resources_newly_allocated_bitset, physical_resource_idx);
	hero_bitset_array_unset64(frame_graph->physical_resources_that_are_unused_bitset, physical_resource_idx);

END:
	physical_resource->lifetime_execution_units_range = (HeroRangeU16){0};
	return physical_resource;
}

HeroPassResource* _hero_frame_graph_pass_resource_find(HeroFrameGraph* frame_graph, HeroPass* pass, HeroPassResourceId resource_id) {
	U32 pass_resource_idx = HERO_PASS_RESOURCE_IDX_INVALID;
	Uptr pass_resource_start_idx = pass->resource_ranges_by_type[0].start_idx;
	Uptr pass_resource_end_idx = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COUNT - 1].end_idx;
	for_range(i, pass_resource_start_idx, pass_resource_end_idx) {
		if (frame_graph->pass_resource_ids[i] == resource_id) {
			pass_resource_idx = i;
			break;
		}
	}

	//
	// ensure the resource exists
	HERO_DEBUG_ASSERT(pass_resource_idx != HERO_PASS_RESOURCE_IDX_INVALID, "frame graph '%s' pass '%s' does not have the resource id '%u'", frame_graph->name, pass->name, resource_id);

	return &frame_graph->pass_resources[pass_resource_idx];
}

U16 _hero_frame_graph_prepare_recursive(HeroFrameGraph* frame_graph, HeroPass* pass, HeroPassEnum pass_enum) {
	if (pass->execution_unit_idx != HERO_EXECUTION_UNIT_IDX_INVALID) {
		// pass has already been through this function
		return pass->execution_unit_idx;
	}

	//
	// now recursively ascend up the parents by look and what the pass's resources link to.
	// also find the max_execution_unit_idx of the parent passes so we can make ours + 1 that one, unless we have no parent passes.
	//

	U16 max_execution_unit_idx = 0;
	bool has_parent_pass = false;
	pass->execution_unit_idx = HERO_EXECUTION_UNIT_IDX_BEING_SET;
	{
		//
		// for every pass resource
		Uptr start_idx = pass->resource_ranges_by_type[0].start_idx;
		Uptr end_idx = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COUNT - 1].end_idx;
		for_range(idx, start_idx, end_idx) {
			//
			// extract the link from the buffer or image
			HeroPassResource* resource = &frame_graph->pass_resources[idx];
			HeroPassEnum link_pass_enum = resource->link_pass_enum;
			HeroPassResourceId link_id = resource->link_id;

			if (link_pass_enum == HERO_PASS_ENUM_INVALID) {
				//
				// this resource doesn't link to anything, you have discovered the birth of a physical resource.
				// mark the physical resource so that it still exists
				// or create an new allocation but don't give it an address range yet.
				//
				HeroPassPhysicalResource* physical_resource = _hero_frame_graph_physical_resource_find_or_insert(frame_graph, HERO_RESOURCE_ID(pass->pass_enum, resource->id));

				resource->physical_resource_idx = physical_resource - frame_graph->physical_resources;
				physical_resource->flags = resource->flags;
				physical_resource->is_buffer = HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type);
				continue;
			}

			//
			// ensure the linked pass exists
			HeroPass* parent_pass = &frame_graph->passes[link_pass_enum];
			HERO_DEBUG_ASSERT(parent_pass->pass_enum == link_pass_enum, "frame graph '%s' pass '%s' %s resource at index '%u' is linked to a pass enum '%u' that is not in the frame graph", frame_graph->name, pass->name, hero_pass_resource_type_strings[resource->type], idx - start_idx, link_pass_enum);


			//
			// find the linked resource exists in the linked pass
			HeroPassResource* link_resource = _hero_frame_graph_pass_resource_find(frame_graph, parent_pass, link_id);

			//
			// ensure the linked resource is a compatible type
			HERO_DEBUG_ASSERT(hero_pass_resource_type_is_compatible(resource->type, link_resource->type), "frame graph '%s' pass '%s' %s resource id '%u' is linked to a pass '%s' %s resource id '%u' are of incompatible types", frame_graph->name, pass->name, hero_pass_resource_type_strings[resource->type], resource->id, parent_pass->name, hero_pass_resource_type_strings[link_resource->type], link_id);

			if (parent_pass->execution_unit_idx == HERO_EXECUTION_UNIT_IDX_BEING_SET) {

				//
				// when a resource links to resource a head in the graph, it means it comes from the previous frame.
				// here we make a new physical resource so that when the previous frame is finished, it can copy it's resource to this frame's copy.
				HeroPassPhysicalResource* physical_resource = _hero_frame_graph_physical_resource_find_or_insert(frame_graph, HERO_RESOURCE_ID(pass->pass_enum, resource->id));
				resource->physical_resource_idx = physical_resource - frame_graph->physical_resources;

				HERO_DEBUG_ASSERT(link_resource->physical_resource_idx != HERO_PHYSICAL_RESOURCE_IDX_INVALID, "internal error: expected linked resource to have physical resource idx");
				physical_resource->parent_physical_resource_idx = link_resource->physical_resource_idx;
				physical_resource->flags = link_resource->flags;
				physical_resource->is_buffer = HERO_PASS_RESOURCE_TYPE_IS_BUFFER(link_resource->type);

				continue; // avoid a circular reference when a parent references a child that is already being processed
			}

			if (link_resource->physical_resource_idx != HERO_PHYSICAL_RESOURCE_IDX_INVALID) {
				if (HERO_PASS_RESOURCE_TYPE_IS_MUTABLE(resource->type) || resource->link_chain_length >= link_resource->nearest_link_chain_mutation) {
					//
					// when a resource links to another resource that has a physical resource already
					//     (this happens when recursively ascend up from another output image).
					// and when this resource mutates or there is a mutation that happens at the same execution_unit_idx or lower
					// then we have two versions of the resource existing at the same time. we canno have this without creating
					// a new physical resource. so we do that here, a copy will be made from the linked resource.
					HeroPassPhysicalResource* physical_resource = _hero_frame_graph_physical_resource_find_or_insert(frame_graph, HERO_RESOURCE_ID(pass->pass_enum, resource->id));
					resource->physical_resource_idx = physical_resource - frame_graph->physical_resources;

					physical_resource->parent_physical_resource_idx = link_resource->physical_resource_idx;
					physical_resource->flags = link_resource->flags;
					physical_resource->is_buffer = HERO_PASS_RESOURCE_TYPE_IS_BUFFER(link_resource->type);
				}
			}

			//
			// keep track of how many execution units the since the last mutation of this resource link chain
			if (HERO_PASS_RESOURCE_TYPE_IS_MUTABLE(resource->type)) {
				link_resource->nearest_link_chain_mutation = 0;
			} else if (resource->nearest_link_chain_mutation != HERO_NEAREST_LINK_CHAIN_MUTATION_INVALID) {
				link_resource->nearest_link_chain_mutation = resource->nearest_link_chain_mutation + 1;
			}

			link_resource->link_chain_length = HERO_MAX(link_resource->link_chain_length, resource->link_chain_length + 1);

			has_parent_pass = true;
			U16 parent_execution_unit_idx = _hero_frame_graph_prepare_recursive(frame_graph, parent_pass, link_pass_enum);
			max_execution_unit_idx = HERO_MAX(max_execution_unit_idx, parent_execution_unit_idx);

			//
			// bring the physical resource index, buffer size and image info id down the tree to the children
			resource->physical_resource_idx = link_resource->physical_resource_idx;
			if (HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type)) {
				resource->data.buffer_size = link_resource->data.buffer_size;
			} else {
				resource->data.image_info_id = link_resource->data.image_info_id;
			}
		}
	}

	if (has_parent_pass) {
		max_execution_unit_idx += 1;
	}

	pass->execution_unit_idx = max_execution_unit_idx;
	return max_execution_unit_idx;
}

HeroFreeRanges* _hero_frame_graph_resource_free_ranges(HeroFrameGraph* frame_graph, HeroPassResource* resource) {
	bool is_buffer = HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type);
	bool is_persistent = resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT;
	HeroFreeRanges* free_ranges;
	switch (is_buffer + (is_persistent * 2)) {

		// is_buffer && !is_persistent
		case 0: return &frame_graph->buffers_memory_free_ranges;

		// !is_buffer && !is_persistent
		case 1: return &frame_graph->images_memory_free_ranges;

		// is_buffer && is_persistent
		case 2: return &frame_graph->persistent_buffers_memory_free_ranges;

		// !is_buffer && is_persistent
		case 3: return &frame_graph->persistent_images_memory_free_ranges;
	}

	HERO_UNREACHABLE();
}

HeroResult hero_frame_graph_update(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroRenderGraphId* render_graph_ids, U32 render_graphs_count) {
	HeroFrameGraph* frame_graph;
	HeroResult result = hero_frame_graph_get(ldev, id, &frame_graph);

	//
	// clear the frame graph
	HERO_ONE_ELMT_MANY(frame_graph->passes, frame_graph->passes_count); // we one bits so that HeroPass.pass_enum is HERO_PASS_ENUM_INVALID
	frame_graph->passes_count = 0;
	frame_graph->viewports_count = 0;
	frame_graph->pass_resources_count = 0;

	//
	// reset some physical resource & passes bitsets so we can keep track of newly unused and newly allocated resources
	Uptr bitset_words_count = DIV_ROUND_UP(frame_graph->physical_resources_cap, 64);
	HERO_ONE_ELMT_MANY(frame_graph->physical_resources_that_are_unused_bitset, bitset_words_count);
	HERO_ZERO_ELMT_MANY(frame_graph->physical_resources_newly_allocated_bitset, bitset_words_count);
	HERO_ONE_ELMT_MANY(frame_graph->passes_that_are_unused_bitset, bitset_words_count);
	HERO_ZERO_ELMT_MANY(frame_graph->passes_newly_allocated_bitset, bitset_words_count);

	//
	// merge all the render graphs into the frame graph
	for_range(idx, 0, render_graphs_count) {
		HeroRenderGraph* render_graph;
		HeroResult result = hero_render_graph_get(ldev, render_graph_ids[idx], &render_graph);
		if (result < 0) {
			return result;
		}

		//
		// copy the passes into the frame graph from the render graph
		for_range(src_pass_idx, 0, render_graph->passes_count) {
			HeroPass* src_pass = &frame_graph->passes[src_pass_idx];
			HeroPassEnum pass_enum = src_pass->pass_enum;
			HERO_DEBUG_ASSERT(pass_enum < frame_graph->passes_cap, "frame graph '%s' has a capacity of '%u' passes and pass enum '%u' is out of bounds", frame_graph->name, frame_graph->passes_cap, pass_enum);

			HeroPass* dst_pass = &frame_graph->passes[pass_enum];

			HERO_DEBUG_ASSERT(dst_pass->pass_enum == 0, "frame graph '%s' pass '%s' has the same pass enum '%u' as pass '%s'", frame_graph->name, src_pass->name, dst_pass->pass_enum, dst_pass->name);

			*dst_pass = *src_pass;

			//
			// set a bit to say it is allocated and what not
			hero_bitset_array_set64(frame_graph->passes_allocated_bitset, pass_enum);
			hero_bitset_array_set64(frame_graph->passes_newly_allocated_bitset, pass_enum);
			hero_bitset_array_unset64(frame_graph->passes_that_are_unused_bitset, pass_enum);

			//
			// copy the resources from the pools in the render graph into a flat array in the frame graph.
			// this keeps the resources sorted by type but they are now in a flat array.
			for_range(resource_type, 0, HERO_PASS_RESOURCE_TYPE_COUNT) {
				HeroPassResourcePool* src_pool = &render_graph->resource_pools[resource_type];
				HeroRangeU16* range = &dst_pass->resource_ranges_by_type[resource_type];
				U16 src_elmts_count = range->end_idx - range->start_idx;

				U16 new_count = frame_graph->pass_resources_count + src_elmts_count;
				HERO_DEBUG_ASSERT(new_count > frame_graph->pass_resources_cap, "the frame graph '%s' resources capacity of '%u' has been exceed", frame_graph->name, frame_graph->pass_resources_cap);

				for_range(src_idx, 0, src_elmts_count) {
					HeroPassResource* src_resource = &src_pool->data[range->start_idx + src_idx];
					HeroPassResource* dst_resource = &frame_graph->pass_resources[frame_graph->pass_resources_count + src_idx];

					*dst_resource = *src_resource;
					frame_graph->pass_resource_ids[frame_graph->pass_resources_count + src_idx] = src_resource->id;
				}

				//
				// update the resource type range to reference the flat array
				range->start_idx = frame_graph->pass_resources_count;
				range->end_idx = new_count;

				frame_graph->pass_resources_count = new_count;
			}

			U16 viewports_count = dst_pass->viewports_range.end_idx - dst_pass->viewports_range.start_idx;
			HERO_COPY_ELMT_MANY(&frame_graph->viewports[frame_graph->viewports_count], &render_graph->viewports[dst_pass->viewports_range.start_idx], viewports_count);
			dst_pass->viewports_range.start_idx = frame_graph->viewports_count;
			dst_pass->viewports_range.end_idx = frame_graph->viewports_count + viewports_count;
			frame_graph->viewports_count += viewports_count;
		}
		frame_graph->passes_count += render_graph->passes_count;
	}

	//
	// validate and prepare the frame graph
	U16 max_execution_unit_idx = 0;
	{

		for_range(pass_enum, 0, frame_graph->passes_count) {
			HeroPass* pass = &frame_graph->passes[pass_enum];

			if (pass->flags & HERO_PASS_FLAGS_HAS_SWAPCHAIN_OR_CPU_READABLE) {
				U16 pass_max_execution_unit_idx = _hero_frame_graph_prepare_recursive(frame_graph, pass, pass_enum);
				max_execution_unit_idx = HERO_MAX(pass_max_execution_unit_idx, max_execution_unit_idx);
			}
		}

	}

	//
	// make the pass execution units
	U16 execution_units_count = max_execution_unit_idx + 1;
	frame_graph->execution_units_count = execution_units_count;
	{
		U16 collected_passes_count = 0;
		for_range(execution_unit_idx, 0, execution_units_count) {
			HeroRangeU16* range = &frame_graph->execution_units_pass_ranges[execution_unit_idx];
			range->start_idx = collected_passes_count;
			range->end_idx = collected_passes_count;
			for_range(pass_enum, 0, frame_graph->passes_count) {
				HeroPass* pass = &frame_graph->passes[pass_enum];
				if (pass->execution_unit_idx == execution_unit_idx) {
					frame_graph->execution_units_pass_enums[range->end_idx] = pass_enum;
					range->end_idx += 1;
					collected_passes_count += 1;
				}
			}
		}
	}

	{
		//
		// mask the passes allocated bitset onto the unused bitset.
		// this is so the backend can tell what passes need to be thrown away.
		for_range(word_idx, 0, bitset_words_count) {
			frame_graph->passes_that_are_unused_bitset[word_idx] &= frame_graph->passes_allocated_bitset[word_idx];
		}

		//
		// now remove the unused passes from the allocated bitset
		for_range(word_idx, 0, bitset_words_count) {
			frame_graph->passes_allocated_bitset[word_idx] &= ~frame_graph->passes_that_are_unused_bitset[word_idx];
		}
	}

	{
		//
		// mask the physical resources allocated bitset onto the unused bitset.
		// this is so the backend can tell what physical resources need to be thrown away.
		for_range(word_idx, 0, bitset_words_count) {
			frame_graph->physical_resources_that_are_unused_bitset[word_idx] &= frame_graph->physical_resources_allocated_bitset[word_idx];
		}

		//
		// now remove the unused physical resources from the allocated bitset
		for_range(word_idx, 0, bitset_words_count) {
			frame_graph->physical_resources_allocated_bitset[word_idx] &= ~frame_graph->physical_resources_that_are_unused_bitset[word_idx];
		}
	}

	/* GOES IN THE BACKEND
	Uptr physical_resource_idx = (Uptr)-1;
	while (hero_bitset_array_iter_next_one64(frame_graph->physical_resources_that_are_unused_bitset, frame_graph->physical_resources_cap, &physical_resource_idx)) {

	}
	*/

	//
	// go over the passes in execution unit order and work out the physical resource lifetime ranges
	{
		//
		// for every execution unit
		for_range(execution_unit_idx, 0, execution_units_count) {
			HeroRangeU16 range = frame_graph->execution_units_pass_ranges[execution_unit_idx];

			//
			// for every execution unit pass
			for_range(execution_units_pass_idx, range.start_idx, range.end_idx) {
				HeroPass* pass = &frame_graph->passes[frame_graph->execution_units_pass_enums[execution_units_pass_idx]];

				//
				// for every pass resource
				Uptr start_idx = pass->resource_ranges_by_type[0].start_idx;
				Uptr end_idx = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COUNT - 1].end_idx;
				for_range(resource_idx, start_idx, end_idx) {
					HeroPassResource* resource = &frame_graph->pass_resources[resource_idx];
					HeroPassPhysicalResource* physical_resource = &frame_graph->physical_resources[resource->physical_resource_idx];

					if (resource->link_pass_enum != HERO_PASS_ENUM_INVALID) {
						HeroPass* link_pass = &frame_graph->passes[resource->link_pass_enum];
						HeroPassResource* link_resource = _hero_frame_graph_pass_resource_find(frame_graph, link_pass, resource->link_id);
						link_resource->link_to_by_min_execution_unit_idx = HERO_MIN(link_resource->link_to_by_min_execution_unit_idx, execution_unit_idx);
					}

					if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT) {
						physical_resource->lifetime_execution_units_range.start_idx = 0;
						physical_resource->lifetime_execution_units_range.end_idx = execution_units_count;
					} else {
						if (physical_resource->lifetime_execution_units_range.start_idx == physical_resource->lifetime_execution_units_range.end_idx) {
							//
							// physical resource has not been started yet, so lets kick off the lifetime
							physical_resource->lifetime_execution_units_range.start_idx = execution_unit_idx;
						}

						physical_resource->lifetime_execution_units_range.end_idx = execution_unit_idx + 1;
					}
				}
			}
		}
	}

	//
	// go over the passes in execution unit order and assign the memory to the physical resources
	{

		//
		// setup the free ranges will to have all the memory available
		{
			frame_graph->images_memory_free_ranges.ranges.count = 0;
			frame_graph->buffers_memory_free_ranges.ranges.count = 0;

			HeroRange range = { .start_idx = 0, .end_idx = frame_graph->images_memory_pool_size };
			result = hero_free_ranges_give_range(&frame_graph->images_memory_free_ranges, range, hero_system_alctor, 0);
			if (result < 0) {
				return result;
			}

			range.end_idx = frame_graph->buffers_memory_pool_size;
			result = hero_free_ranges_give_range(&frame_graph->buffers_memory_free_ranges, range, hero_system_alctor, 0);
			if (result < 0) {
				return result;
			}
		}

		//
		// for every execution unit
		for_range(execution_unit_idx, 0, execution_units_count) {
			HeroRangeU16 range = frame_graph->execution_units_pass_ranges[execution_unit_idx];

			//
			// for every execution unit pass
			for_range(execution_units_pass_idx, range.start_idx, range.end_idx) {
				HeroPass* pass = &frame_graph->passes[frame_graph->execution_units_pass_enums[execution_units_pass_idx]];

				//
				// for every pass resource
				Uptr start_idx = pass->resource_ranges_by_type[0].start_idx;
				Uptr end_idx = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COUNT - 1].end_idx;

				//
				// loop over the physical resources with existing memory ranges and allocated/deallocate those.
				// this includes the deallocations for new resources that where added this frame as in
				// a previous loop of the execution unit they will be allocated and mem_range will be set.
				for_range(resource_idx, start_idx, end_idx) {
					HeroPassResource* resource = &frame_graph->pass_resources[resource_idx];
					HeroPassPhysicalResource* physical_resource = &frame_graph->physical_resources[resource->physical_resource_idx];

					if (physical_resource->mem_range.start_idx != physical_resource->mem_range.end_idx) {

						if (physical_resource->lifetime_execution_units_range.start_idx == execution_unit_idx) {
							HeroFreeRanges* free_ranges = _hero_frame_graph_resource_free_ranges(frame_graph, resource);

							result = hero_free_ranges_take_range_requested(free_ranges, physical_resource->mem_range, hero_system_alctor, 0);
							HERO_RESULT_ASSERT(result);
						} else if (physical_resource->lifetime_execution_units_range.end_idx == execution_unit_idx) {
							HeroFreeRanges* free_ranges = _hero_frame_graph_resource_free_ranges(frame_graph, resource);
							result = hero_free_ranges_give_range(free_ranges, physical_resource->mem_range, hero_system_alctor, 0);
							HERO_RESULT_ASSERT(result);
						}
					}
				}

				//
				// loop over the physical resources that are new and allocated them a memory range
				for_range(resource_idx, start_idx, end_idx) {
					HeroPassResource* resource = &frame_graph->pass_resources[resource_idx];
					HeroPassPhysicalResource* physical_resource = &frame_graph->physical_resources[resource->physical_resource_idx];

					if (physical_resource->mem_range.start_idx == physical_resource->mem_range.end_idx) {
						Uptr size;
						Uptr align;
						if (HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type)) {
							size = resource->data.buffer_size;
							align = 1; // TODO: get align from backend or something
						} else {
							HeroImageInfo* info;
							result = _hero_image_info_get(ldev, resource->data.image_info_id, &info);
							if (result < 0) {
								return result;
							}

							size = (Uptr)info->width * (Uptr)info->height * (Uptr)info->depth * (Uptr)info->array_layers_count * hero_image_format_bytes_per_pixel[info->format];
							if (info->mip_levels_count > 1) {
								HERO_ABORT("TODO: check if this is correct");
								size += info->width / 2;
							}
							align = hero_image_format_bytes_per_pixel[info->format]; // TODO: get align from backend or something
						}

						HeroFreeRanges* free_ranges = _hero_frame_graph_resource_free_ranges(frame_graph, resource);

						Uptr mem_start_idx;
						result = hero_free_ranges_take_range(free_ranges, size, align, hero_system_alctor, 0, &mem_start_idx);
						if (result < 0) {
							return result;
						}
						physical_resource->mem_range.start_idx = mem_start_idx;
						physical_resource->mem_range.end_idx = mem_start_idx + size;
					}
				}
			}
		}
	}

	return hero_gfx_sys.backend_vtable.frame_graph_update(ldev, frame_graph);
}

HeroResult hero_frame_graph_record_pass_start(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroPassEnum pass_enum, HeroCommandRecorder** command_recorder_out) {
	HeroFrameGraph* frame_graph;
	HeroResult result = hero_frame_graph_get(ldev, id, &frame_graph);

	_HeroCommandRecorder* command_recorder = &_hero_command_recorder_thread_local;
	bool expected_is_in_use = false;
	if (!atomic_compare_exchange_strong(&command_recorder->is_in_use, &expected_is_in_use, true)) {
		return HERO_ERROR(ALREADY_IN_USE);
	}

	HeroPass* pass = &frame_graph->passes[pass_enum];

	HeroPassRecordState expected_record_state = HERO_PASS_RECORD_STATE_NOT_STARTED;
	if (!atomic_compare_exchange_strong(&pass->record_state, &expected_record_state, HERO_PASS_RECORD_STATE_STARTED)) {
		if (expected_record_state == HERO_PASS_RECORD_STATE_FINISHED) {
			return HERO_ERROR(ALREADY_EXISTS);
		} else {
			return HERO_ERROR(ALREADY_STARTED);
		}
	}

	command_recorder->public_.ldev = ldev;
	command_recorder->public_.frame_graph = frame_graph;
	command_recorder->public_.pass = pass;
	*command_recorder_out = &command_recorder->public_;
	return hero_gfx_sys.backend_vtable.frame_graph_record_pass_start(ldev, frame_graph, pass, command_recorder);
}

HeroResult hero_frame_graph_record_pass_end(HeroCommandRecorder* command_recorder) {
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;
	HERO_DEBUG_ASSERT(command_recorder_ == &_hero_command_recorder_thread_local, "the command recorder is not from this thread");

	HeroResult result = hero_gfx_sys.backend_vtable.frame_graph_record_pass_end(command_recorder_);
	if (result < 0) {
		return result;
	}

	atomic_store(&command_recorder->pass->record_state, HERO_PASS_RECORD_STATE_FINISHED);
	atomic_store(&command_recorder_->is_in_use, false);
	return HERO_SUCCESS;
}

HeroResult hero_frame_graph_submit(HeroLogicalDevice* ldev, HeroFrameGraphId id) {
	HeroFrameGraph* frame_graph;
	HeroResult result = hero_frame_graph_get(ldev, id, &frame_graph);

	return hero_gfx_sys.backend_vtable.frame_graph_submit(ldev, frame_graph);
}

void hero_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroPipelineId pipeline_id) {
	HERO_DEBUG_ASSERT(!(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_OR_COMPUTE_DISPATCH_STARTED), "hero_cmd_draw_start cannot be called when a draw or compute dispatch has not finished");
	command_recorder->flags |= HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED;

	hero_gfx_sys.backend_vtable.cmd_draw_start(command_recorder, pipeline_id);
}

void hero_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_count) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED, "hero_cmd_draw_end_* cannot be called when a draw has not been started");

	HERO_DEBUG_ASSERT(vertices_count, "vertices_count must be a non zero value");

	hero_gfx_sys.backend_vtable.cmd_draw_end_vertexed(command_recorder, vertices_count);
}

void hero_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroPassResourceId index_buffer_resource_id, HeroIndexType index_type, U32 indices_count) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED, "hero_cmd_draw_end_* cannot be called when a draw has not been started");

	HERO_DEBUG_ASSERT(indices_count, "indices_count must be a non zero value");

	HeroPassResource* resource = _hero_frame_graph_pass_resource_find(command_recorder->frame_graph, command_recorder->pass, index_buffer_resource_id);
	HERO_DEBUG_ASSERT(resource->type == HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER, "expected index_buffer_resource_id to be of type HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER but got %s", hero_pass_resource_type_strings[resource->type]);

	hero_gfx_sys.backend_vtable.cmd_draw_end_indexed(command_recorder, resource, index_type, indices_count);
}


void hero_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroPassResourceId vertex_buffer_resource_id, U32 binding, U64 offset) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED, "hero_cmd_draw_set_vertex_buffer cannot be called when a draw has not been started");

	HeroPassResource* resource = _hero_frame_graph_pass_resource_find(command_recorder->frame_graph, command_recorder->pass, vertex_buffer_resource_id);
	HERO_DEBUG_ASSERT(resource->type == HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER, "expected vertex_buffer_resource_id to be of type HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER but got %s", hero_pass_resource_type_strings[resource->type]);

	hero_gfx_sys.backend_vtable.cmd_draw_set_vertex_buffer(command_recorder, resource, binding, offset);
}

void hero_cmd_draw_set_vertices_start_idx(HeroCommandRecorder* command_recorder, U32 vertices_start_idx) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED, "hero_cmd_draw_set_vertices_start_idx cannot be called when a draw has not been started");

	hero_gfx_sys.backend_vtable.cmd_draw_set_vertices_start_idx(command_recorder, vertices_start_idx);
}

void hero_cmd_draw_set_indices_start_idx(HeroCommandRecorder* command_recorder, U32 indices_start_idx) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED, "hero_cmd_draw_set_indices_start_idx cannot be called when a draw has not been started");

	hero_gfx_sys.backend_vtable.cmd_draw_set_indices_start_idx(command_recorder, indices_start_idx);
}

void hero_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_STARTED, "hero_cmd_draw_set_instances cannot be called when a draw has not been started");

	HERO_DEBUG_ASSERT(instances_count, "instances_count must be a non zero value");

	hero_gfx_sys.backend_vtable.cmd_draw_set_instances(command_recorder, instances_start_idx, instances_count);
}


void hero_cmd_compute_dispatch_start(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, U32 group_count_x, U32 group_count_y, U32 group_count_z) {
	HERO_DEBUG_ASSERT(!(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_OR_COMPUTE_DISPATCH_STARTED), "hero_cmd_compute_dispatch_start cannot be called when a draw or compute dispatch has not finished");
	command_recorder->flags |= HERO_COMMAND_RECORDER_FLAGS_COMPUTE_DISPATCH_STARTED;

	HERO_DEBUG_ASSERT(group_count_x, "group_count_x must be a non zero value");
	HERO_DEBUG_ASSERT(group_count_y, "group_count_y must be a non zero value");
	HERO_DEBUG_ASSERT(group_count_z, "group_count_z must be a non zero value");

#if HERO_DEBUG_ASSERTIONS
	HeroShader* shader;
	HeroResult result = hero_shader_get(command_recorder->ldev, compute_shader_id, &shader);
	HERO_RESULT_ASSERT(result);

	HERO_ASSERT(shader->stages.type == HERO_SHADER_TYPE_COMPUTE, "shader must be a compute shader");
#endif

	hero_gfx_sys.backend_vtable.cmd_compute_dispatch_start(command_recorder, compute_shader_id, group_count_x, group_count_y, group_count_z);
}

void hero_cmd_compute_dispatch_end(HeroCommandRecorder* command_recorder) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_COMPUTE_DISPATCH_STARTED, "hero_cmd_compute_dispatch_end cannot be called when a draw has not been started");

	hero_gfx_sys.backend_vtable.cmd_compute_dispatch_end(command_recorder);
}


void hero_cmd_add_sampler(HeroCommandRecorder* command_recorder, U32 binding, HeroSamplerId sampler_id) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_OR_COMPUTE_DISPATCH_STARTED, "hero_cmd_add_sampler cannot be called when a draw or compute dispatch has not been started");

	hero_gfx_sys.backend_vtable.cmd_add_sampler(command_recorder, binding, sampler_id);
}

void hero_cmd_add_image(HeroCommandRecorder* command_recorder, U32 binding, HeroPassResourceId resource_id) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_OR_COMPUTE_DISPATCH_STARTED, "hero_cmd_add_resource cannot be called when a draw or compute dispatch has not been started");

	HeroPassResource* resource = _hero_frame_graph_pass_resource_find(command_recorder->frame_graph, command_recorder->pass, resource_id);
#if HERO_DEBUG_ASSERTIONS
	switch (resource->type) {
		case HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT:
			HERO_ABORT("TODO image attachment resource!");
		case HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT:
		case HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT:
ERR: {}
			HERO_ABORT("%s resource is not allowed to be used as the command resource passed into the shader", hero_pass_resource_type_strings[resource->type]);
		default:
			if (HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type)) {
				goto ERR;
			}
			break;
	}
#endif

	hero_gfx_sys.backend_vtable.cmd_add_image(command_recorder, binding, resource);
}

void hero_cmd_add_buffer(HeroCommandRecorder* command_recorder, U32 binding, HeroPassResourceId resource_id) {
	HERO_DEBUG_ASSERT(command_recorder->flags & HERO_COMMAND_RECORDER_FLAGS_DRAW_OR_COMPUTE_DISPATCH_STARTED, "hero_cmd_add_resource cannot be called when a draw or compute dispatch has not been started");

	HeroPassResource* resource = _hero_frame_graph_pass_resource_find(command_recorder->frame_graph, command_recorder->pass, resource_id);
#if HERO_DEBUG_ASSERTIONS
	switch (resource->type) {
		case HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER:
		case HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER:
ERR: {}
			HERO_ABORT("%s resource is not allowed to be used as the command resource passed into the shader", hero_pass_resource_type_strings[resource->type]);
		default:
			if (!HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type)) {
				goto ERR;
			}
			break;
	}
#endif

	hero_gfx_sys.backend_vtable.cmd_add_buffer(command_recorder, binding, resource);
}

#if 0
const char* hero_render_graph_error_type_fmt_strings[HERO_RENDER_GRAPH_ERROR_TYPE_COUNT] = {
	[HERO_RENDER_GRAPH_ERROR_TYPE_NONE] = "HERO_RENDER_GRAPH_ERROR_TYPE_NONE\n",

	//
	// enums[0] = HeroPassEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_RECORDED_FN] = "HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_RECORDED_FN PASS.%s\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_CMD_ESTIMATE_FN] = "HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_CMD_ESTIMATE_FN PASS.%s\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_PASS_INVALID_RENDER_PASS_LAYOUT_ID] = "HERO_RENDER_GRAPH_ERROR_TYPE_PASS_INVALID_RENDER_PASS_LAYOUT_ID PASS.%s\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_PASS_UNUSED] = "HERO_RENDER_GRAPH_ERROR_TYPE_PASS_UNUSED PASS.%s\n",

	//
	// enums[0] = HeroImageInfoEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_UNUSED] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_UNUSED IMAGE.%s\n",

	//
	// enums[0] = HeroBufferInfoEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_UNUSED] = "HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_UNUSED BUFFER.%s\n",

	//
	// enums[0] = HeroPassEnum
	// enums[1] = expected_attachments_count
	// enums[2] = got_attachments_count
	[HERO_RENDER_GRAPH_ERROR_TYPE_PASS_ATTACHMENTS_COUNT_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_PASS_ATTACHMENTS_COUNT_MISMATCH PASS.%s expected '%u' attachments but got '%u'\n",

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageOutputEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_FORMAT_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_FORMAT_MISMATCH PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_SAMPLES_COUNT_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_SAMPLES_COUNT_MISMATCH PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_COLOR_IMAGE_FORMAT_FOR_COLOR_ATTACHMENT] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_COLOR_IMAGE_FORMAT_FOR_COLOR_ATTACHMENT PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_DEPTH_IMAGE_FORMAT_FOR_DEPTH_ATTACHMENT] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_DEPTH_IMAGE_FORMAT_FOR_DEPTH_ATTACHMENT PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_IMAGE_ENUM_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_IMAGE_ENUM_DOES_NOT_EXIST PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_DOES_NOT_EXIST PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_WIDTH_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_WIDTH_MISMATCH PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_HEIGHT_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_HEIGHT_MISMATCH PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ARRAY_LAYERS_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ARRAY_LAYERS_MISMATCH PASS.%s IMAGE_OUTPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_SWAPCHAIN_MISMATCH] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_SWAPCHAIN_MISMATCH PASS.%s IMAGE_OUTPUT.%u\n",

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassImageInputEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_ENUM_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_ENUM_DOES_NOT_EXIST PASS.%s IMAGE_INPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_IMAGE_OUTPUT_ENUM_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_IMAGE_OUTPUT_ENUM_DOES_NOT_EXIST PASS.%s IMAGE_INPUT.%u\n",

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferOutputEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_OUTPUT_BUFFER_ENUM_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_OUTPUT_BUFFER_ENUM_DOES_NOT_EXIST PASS.%s BUFFER_OUTPUT.%u\n",

	//
	// enums[0] = HeroPassEnum
	// enums[1] = HeroPassBufferInputEnum
	[HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_ENUM_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_ENUM_DOES_NOT_EXIST PASS.%s BUFFER_INPUT.%u\n",
	[HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_BUFFER_OUTPUT_ENUM_DOES_NOT_EXIST] = "HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_BUFFER_OUTPUT_ENUM_DOES_NOT_EXIST PASS.%s BUFFER_INPUT.%u\n",
};

HeroRenderGraphError* _hero_render_graph_error(HeroRenderGraphSetup* setup, HeroRenderGraphErrorType type) {
	if (setup->errors_count >= setup->errors_cap) {
		return NULL;
	}
	HeroRenderGraphError* error = &setup->errors_out[setup->errors_count];
	error->type = type;
	setup->errors_count += 1;
	return error;
}

void _hero_render_graph_error_1(HeroRenderGraphSetup* setup, HeroRenderGraphErrorType type, U16 enum_0) {
	HeroRenderGraphError* error = _hero_render_graph_error(setup, type);
	if (error) {
		error->enums[0] = enum_0;
	}
}

void _hero_render_graph_error_2(HeroRenderGraphSetup* setup, HeroRenderGraphErrorType type, U16 enum_0, U16 enum_1) {
	HeroRenderGraphError* error = _hero_render_graph_error(setup, type);
	if (error) {
		error->enums[0] = enum_0;
		error->enums[1] = enum_1;
	}
}

void _hero_render_graph_error_3(HeroRenderGraphSetup* setup, HeroRenderGraphErrorType type, U16 enum_0, U16 enum_1, U16 enum_2) {
	HeroRenderGraphError* error = _hero_render_graph_error(setup, type);
	if (error) {
		error->enums[0] = enum_0;
		error->enums[1] = enum_1;
		error->enums[2] = enum_2;
	}
}

void _hero_render_graph_validate_pass(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroPassEnum pass_enum, bool* has_swapchain_or_readback_output_out) {
	HeroPassInfo* pass_info = &setup->passes[pass_enum];

	//
	// validate pass's immediate data
	HeroRenderPassLayout* render_pass_layout = NULL;
	{
		if (pass_info->record_fn == NULL) {
			_hero_render_graph_error_1(setup, HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_RECORDED_FN, pass_enum);
		}
		if (pass_info->cmd_estimate_fn == NULL) {
			_hero_render_graph_error_1(setup, HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_CMD_ESTIMATE_FN, pass_enum);
		}

		if (pass_info->layout_id.raw) {
			HeroResult result = hero_render_pass_layout_get(ldev, pass_info->layout_id, &render_pass_layout);
			if (result < 0) {
				_hero_render_graph_error_1(setup, HERO_RENDER_GRAPH_ERROR_TYPE_PASS_INVALID_RENDER_PASS_LAYOUT_ID, pass_enum);
			}

			U32 attachments_count = pass_info->image_outputs_count + pass_info->image_inputs_count;
			if (pass_info->depth_stencil_image_output_enum != HERO_PASS_IMAGE_OUTPUT_ENUM_INVALID) {
				attachments_count += 1;
			}

			if (render_pass_layout->attachments_count != attachments_count) {
				_hero_render_graph_error_3(setup, HERO_RENDER_GRAPH_ERROR_TYPE_PASS_ATTACHMENTS_COUNT_MISMATCH, pass_enum, render_pass_layout->attachments_count, attachments_count);
			}
		}
	}

	//
	// validate image outputs
	for_range(image_output_enum, 0, pass_info->image_outputs_count) {
		HeroImageOutput* output = &pass_info->image_outputs[image_output_enum];

		if (output->image_info_enum >= setup->images_count) {
			_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_IMAGE_ENUM_DOES_NOT_EXIST, pass_enum, image_output_enum);
		} else {
			HeroImageInfo* info = &setup->images[output->image_info_enum];

			//
			// if render pass validate extra things
			if (pass_info->layout_id.raw && output->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
				if (output->attachment_idx >= render_pass_layout->attachments_count) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_DOES_NOT_EXIST, pass_enum, image_output_enum);
				}
				HeroAttachmentLayout* layout = &render_pass_layout->attachments[output->attachment_idx];

				if (layout->format != info->format) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_FORMAT_MISMATCH, pass_enum, image_output_enum);
				}
				if (layout->samples_count != (1 << info->samples_count_log2)) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_SAMPLES_COUNT_MISMATCH, pass_enum, image_output_enum);
				}

				HeroImageInfo* first_output = &setup->images[0];
				if (first_output->swapchain_id.raw != info->swapchain_id.raw) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_SWAPCHAIN_MISMATCH, pass_enum, image_output_enum);
				} else if (first_output->swapchain_id.raw || info->swapchain_id.raw) {
					if (first_output->width != info->width) {
						_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_WIDTH_MISMATCH, pass_enum, image_output_enum);
					}
					if (first_output->height != info->height) {
						_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_HEIGHT_MISMATCH, pass_enum, image_output_enum);
					}
					if (first_output->array_layers_count != info->array_layers_count) {
						_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ARRAY_LAYERS_MISMATCH, pass_enum, image_output_enum);
					}
				}
			}

			*has_swapchain_or_readback_output_out |= (!!info->swapchain_id.raw) | (!!(output->flags & HERO_IMAGE_OUTPUT_FLAGS_READBACK));

			bool is_depth = image_output_enum == pass_info->depth_stencil_image_output_enum;
			if (is_depth) {
				if (!HERO_IMAGE_FORMAT_IS_DEPTH(info->format)) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_DEPTH_IMAGE_FORMAT_FOR_DEPTH_ATTACHMENT, pass_enum, image_output_enum);
				}
			} else {
				if (HERO_IMAGE_FORMAT_IS_DEPTH(info->format)) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_COLOR_IMAGE_FORMAT_FOR_COLOR_ATTACHMENT, pass_enum, image_output_enum);
				}
			}
		}
	}

	//
	// validate image input
	for_range(image_input_enum, 0, pass_info->image_inputs_count) {
		HeroImageInput* input = &pass_info->image_inputs[image_input_enum];

		if (input->pass_enum != HERO_PASS_ENUM_INVALID) {
			if (input->pass_enum != HERO_PASS_ENUM_INVALID && input->pass_enum >= setup->passes_count) {
				_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_ENUM_DOES_NOT_EXIST, pass_enum, image_input_enum);
			} else {
				U32 count = input->pass_enum == HERO_PASS_ENUM_INVALID ? setup->cpu_image_outputs_count : setup->passes[input->pass_enum].image_outputs_count;
				if (input->pass_image_output_enum >= count) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_IMAGE_OUTPUT_ENUM_DOES_NOT_EXIST, pass_enum, image_input_enum);
				}
			}
		}
	}

	//
	// validate buffer output
	for_range(buffer_output_enum, 0, pass_info->buffer_outputs_count) {
		HeroBufferOutput* output = &pass_info->buffer_outputs[buffer_output_enum];

		if (output->buffer_info_enum >= setup->buffers_count) {
			_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_OUTPUT_BUFFER_ENUM_DOES_NOT_EXIST, pass_enum, buffer_output_enum);
		}
	}

	//
	// validate buffer input
	for_range(buffer_input_enum, 0, pass_info->buffer_inputs_count) {
		HeroBufferInput* input = &pass_info->buffer_inputs[buffer_input_enum];

		if (input->pass_enum != HERO_PASS_ENUM_INVALID) {
			if (input->pass_enum != HERO_PASS_ENUM_INVALID && input->pass_enum >= setup->passes_count) {
				_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_ENUM_DOES_NOT_EXIST, pass_enum, buffer_input_enum);
			} else {
				U32 count = input->pass_enum == HERO_PASS_ENUM_INVALID ? setup->cpu_buffer_outputs_count : setup->passes[input->pass_enum].buffer_outputs_count;
				if (input->pass_buffer_output_enum >= count) {
					_hero_render_graph_error_2(setup, HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_BUFFER_OUTPUT_ENUM_DOES_NOT_EXIST, pass_enum, buffer_input_enum);
				}
			}
		}
	}
}

U16 _hero_render_graph_recursively_mark_in_use(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroPassInfo* pass_info) {
	pass_info->flags |= HERO_PASS_INFO_FLAGS_IS_USED;

	//
	// mark out images that are outputs and are readback or swapchain that they are being used
	{
		for_range(output_idx, 0, pass_info->image_outputs_count) {
			HeroImageOutput* output = &pass_info->image_outputs[output_idx];
			HeroImageInfo* image_info = &setup->images[output->image_info_enum];
			if (image_info->swapchain_id.raw || output->flags & HERO_IMAGE_OUTPUT_FLAGS_READBACK) {
				output->flags |= HERO_IMAGE_OUTPUT_FLAGS_IS_USED;
				image_info->flags |= HERO_IMAGE_INFO_FLAGS_IS_USED;
			}
		}
		for_range(output_idx, 0, pass_info->buffer_outputs_count) {
			HeroBufferOutput* output = &pass_info->buffer_outputs[output_idx];
			HeroBufferInfo* buffer_info = &setup->buffers[output->buffer_info_enum];
			if (output->flags & HERO_BUFFER_OUTPUT_FLAGS_READBACK) {
				output->flags |= HERO_BUFFER_OUTPUT_FLAGS_IS_USED;
				buffer_info->flags |= HERO_BUFFER_INFO_FLAGS_IS_USED;
			}
		}
	}

	//
	// now recursively mark the parent nodes with the in use flags.
	// all find the max_execution_unit_idx of the parents so we can make ours + 1 that one, unless we have no parents.
	U16 max_execution_unit_idx = 0;
	bool has_parent_pass = false;
	pass_info->execution_unit_idx = HERO_EXECUTION_UNIT_IDX_INVALID;
	{
		for_range(input_idx, 0, pass_info->image_inputs_count) {
			HeroImageInput* image_input = &pass_info->image_inputs[input_idx];

			if (image_input->pass_enum == HERO_PASS_ENUM_INVALID) {
				continue;
			}

			HeroPassInfo* input_pass_info = &setup->passes[image_input->pass_enum];
			if (input_pass_info->execution_unit_idx == HERO_EXECUTION_UNIT_IDX_INVALID) {
				continue; // avoid a circular reference when a parent references a child that is already being processed
			}

			has_parent_pass = true;
			U16 parent_execution_unit_idx = _hero_render_graph_recursively_mark_in_use(ldev, setup, input_pass_info);
			max_execution_unit_idx = HERO_MAX(max_execution_unit_idx, parent_execution_unit_idx);
		}

		for_range(input_idx, 0, pass_info->buffer_inputs_count) {
			HeroBufferInput* buffer_input = &pass_info->buffer_inputs[input_idx];

			if (buffer_input->pass_enum == HERO_PASS_ENUM_INVALID) {
				continue;
			}

			HeroPassInfo* input_pass_info = &setup->passes[buffer_input->pass_enum];
			if (input_pass_info->execution_unit_idx == HERO_EXECUTION_UNIT_IDX_INVALID) {
				continue; // avoid a circular reference when a parent references a child that is already being processed
			}

			has_parent_pass = true;
			U16 parent_execution_unit_idx = _hero_render_graph_recursively_mark_in_use(ldev, setup, &setup->passes[buffer_input->pass_enum]);
			max_execution_unit_idx = HERO_MAX(max_execution_unit_idx, parent_execution_unit_idx);
		}
	}

	if (has_parent_pass) {
		max_execution_unit_idx += 1;
	}

	pass_info->execution_unit_idx = max_execution_unit_idx;
	return max_execution_unit_idx;
}

HeroResult hero_render_graph_init(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out) {
	setup->errors_count = 0;

	if (setup->passes_count == 0 || (setup->images_count == 0 && setup->buffers_count == 0)) {
		return HERO_ERROR(GFX_GRAPH_CANNOT_BE_EMPTY);
	}

	//
	// ensure a swapchain is only referenced once in the image infos
	for_range(image_info_enum_a, 0, setup->images_count) {
		HeroImageInfo* info_a = &setup->images[image_info_enum_a];
		if (info_a->swapchain_id.raw) {
			for_range(image_info_enum_b, 0, setup->images_count) {
				if (image_info_enum_a == image_info_enum_b) {
					continue;
				}

				HeroImageInfo* info_b = &setup->images[image_info_enum_b];
				if (info_b->swapchain_id.raw) {
					if (info_a->swapchain_id.raw == info_b->swapchain_id.raw && (info_a->flags & HERO_IMAGE_INFO_FLAGS_IS_SWAPCHAIN) && (info_b->flags & HERO_IMAGE_INFO_FLAGS_IS_SWAPCHAIN)) {
						return HERO_ERROR(GFX_DUPLICATE_SWAPCHAIN_IN_IMAGE_ATTACHMENTS);
					}
				}
			}
		}
	}

	bool has_swapchain_or_readback_output = false;
	for_range(pass_enum, 0, setup->passes_count) {
		_hero_render_graph_validate_pass(ldev, setup, pass_enum, &has_swapchain_or_readback_output);
	}

	if (!has_swapchain_or_readback_output) {
		return HERO_ERROR(GFX_NO_SWAPCHAIN_OR_READBACK_OUTPUT);
	}

	U16 max_execution_unit_idx = 0;
	for_range(pass_enum, 0, setup->passes_count) {
		HeroPassInfo* pass_info = &setup->passes[pass_enum];
		bool has_swapchain_or_readback_output = false;
		for_range(output_idx, 0, pass_info->image_outputs_count) {
			HeroImageOutput* output = &pass_info->image_outputs[output_idx];
			HeroImageInfo* image_info = &setup->images[output->image_info_enum];
			if (image_info->swapchain_id.raw || (output->flags & HERO_IMAGE_OUTPUT_FLAGS_READBACK)) {
				has_swapchain_or_readback_output = true;
				break;
			}
		}

		if (has_swapchain_or_readback_output) {
			U16 execution_unit_idx = _hero_render_graph_recursively_mark_in_use(ldev, setup, pass_info);
			max_execution_unit_idx = HERO_MAX(max_execution_unit_idx, execution_unit_idx);
		}
	}

	U16 execution_units_count = max_execution_unit_idx + 1;
	HeroPassEnum* execution_units_passes = hero_alloc_array(HeroPassEnum, hero_system_alctor, 0, setup->passes_count);
	HeroRangeU16* execution_units_ranges = hero_alloc_array(HeroRangeU16, hero_system_alctor, 0, execution_units_count);
	{
		if (!execution_units_passes || !execution_units_ranges) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		U16 collected_passes_count = 0;
		for_range(execution_unit_idx, 0, execution_units_count) {
			HeroRangeU16* range = &execution_units_ranges[execution_unit_idx];
			range->start_idx = collected_passes_count;
			range->end_idx = collected_passes_count;
			for_range(pass_enum, 0, setup->passes_count) {
				HeroPassInfo* pass_info = &setup->passes[pass_enum];
				if (pass_info->execution_unit_idx == execution_unit_idx) {
					execution_units_passes[range->end_idx] = pass_enum;
					range->end_idx += 1;
					collected_passes_count += 1;
				}
			}
		}
	}

	if (setup->report_unused_error) {
		for_range(pass_enum, 0, setup->passes_count) {
			if (!(setup->passes[pass_enum].flags & HERO_PASS_INFO_FLAGS_IS_USED)) {
				_hero_render_graph_error_1(setup, HERO_RENDER_GRAPH_ERROR_TYPE_PASS_UNUSED, pass_enum);
			}
		}
		for_range(image_info_enum, 0, setup->images_count) {
			if (!(setup->images[image_info_enum].flags & HERO_IMAGE_INFO_FLAGS_IS_USED)) {
				_hero_render_graph_error_1(setup, HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_UNUSED, image_info_enum);
			}
		}
		for_range(buffer_info_enum, 0, setup->buffers_count) {
			if (!(setup->buffers[buffer_info_enum].flags & HERO_BUFFER_INFO_FLAGS_IS_USED)) {
				_hero_render_graph_error_1(setup, HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_UNUSED, buffer_info_enum);
			}
		}
	}

	if (setup->errors_count) {
		return HERO_ERROR(GENERAL);
	}

	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	printf("GARGBAGE-===1==-=--=-=-=-==-=-=\n");
	HeroRenderGraph* render_graph;
	HeroResult result = hero_gfx_sys.backend_vtable.render_graph_init(ldev, setup, id_out, &render_graph);
	if (result < 0) {
		return result;
	}

	render_graph->debug_name = setup->debug_name;
	render_graph->images = setup->images;
	render_graph->buffers = setup->buffers;
	render_graph->passes = setup->passes;
	render_graph->cpu_image_outputs = setup->cpu_image_outputs;
	render_graph->cpu_buffer_outputs = setup->cpu_buffer_outputs;
	render_graph->images_count = setup->images_count;
	render_graph->buffers_count = setup->buffers_count;
	render_graph->passes_count = setup->passes_count;
	render_graph->cpu_image_outputs_count = setup->cpu_image_outputs_count;
	render_graph->cpu_buffer_outputs_count = setup->cpu_buffer_outputs_count;
	render_graph->execution_units_count = execution_units_count;
	render_graph->execution_units_passes = execution_units_passes;
	render_graph->execution_units_ranges = execution_units_ranges;
	exit(0);

	return HERO_SUCCESS;
}

HeroResult hero_render_graph_deinit(HeroLogicalDevice* ldev, HeroRenderGraphId id) {
	HeroRenderGraph* render_graph;
	HeroResult result = hero_render_graph_get(ldev, id, &render_graph);
	if (result < 0) {
		return result;
	}

	return hero_gfx_sys.backend_vtable.render_graph_deinit(ldev, id, render_graph);
}

HeroResult hero_render_graph_get(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out) {
	return hero_gfx_sys.backend_vtable.render_graph_get(ldev, id, out);
}

HeroResult hero_render_graph_deinit_gpu_resources(HeroLogicalDevice* ldev, HeroRenderGraphId id) {
	return hero_gfx_sys.backend_vtable.render_graph_deinit_gpu_resources(ldev, id);
}

HeroResult hero_render_graph_execute(HeroLogicalDevice* ldev, HeroRenderGraphId id) {
	return hero_gfx_sys.backend_vtable.render_graph_execute(ldev, id);
}

void hero_render_graph_print_errors(HeroRenderGraphSetup* setup) {
	for_range(i, 0, setup->errors_count) {
		HeroRenderGraphError* e = &setup->errors_out[i];
		const char* fmt = hero_render_graph_error_type_fmt_strings[e->type];
		switch (e->type) {
			case HERO_RENDER_GRAPH_ERROR_TYPE_NONE:
				printf("%s", fmt);
				break;

			//
			// enums[0] = HeroPassEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_RECORDED_FN:
			case HERO_RENDER_GRAPH_ERROR_TYPE_PASS_EXPECTED_CMD_ESTIMATE_FN:
			case HERO_RENDER_GRAPH_ERROR_TYPE_PASS_INVALID_RENDER_PASS_LAYOUT_ID:
			case HERO_RENDER_GRAPH_ERROR_TYPE_PASS_UNUSED: {
				HeroPassInfo* pass_info = &setup->passes[e->enums[0]];
				printf(fmt, pass_info->debug_name);
				break;
			};

			//
			// enums[0] = HeroImageInfoEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_UNUSED: {
				HeroImageInfo* image_info = &setup->images[e->enums[0]];
				printf(fmt, image_info->debug_name);
				break;
			};

			//
			// enums[0] = HeroBufferInfoEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_UNUSED: {
				HeroBufferInfo* buffer_info = &setup->buffers[e->enums[0]];
				printf(fmt, buffer_info->debug_name);
				break;
			};

			//
			// enums[0] = HeroPassEnum
			// enums[1] = expected_attachments_count
			// enums[2] = got_attachments_count
			case HERO_RENDER_GRAPH_ERROR_TYPE_PASS_ATTACHMENTS_COUNT_MISMATCH: {
				HeroPassInfo* pass_info = &setup->passes[e->enums[0]];
				printf(fmt, pass_info->debug_name, e->enums[1], e->enums[2]);
				break;
			};

			//
			// enums[0] = HeroPassEnum
			// enums[1] = HeroPassImageOutputEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_FORMAT_MISMATCH:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_SAMPLES_COUNT_MISMATCH:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_COLOR_IMAGE_FORMAT_FOR_COLOR_ATTACHMENT:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_EXPECTED_DEPTH_IMAGE_FORMAT_FOR_DEPTH_ATTACHMENT:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_IMAGE_ENUM_DOES_NOT_EXIST:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ATTACHMENT_DOES_NOT_EXIST:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_WIDTH_MISMATCH:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_HEIGHT_MISMATCH:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_ARRAY_LAYERS_MISMATCH:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_OUTPUT_SWAPCHAIN_MISMATCH: {
				HeroPassInfo* pass_info = &setup->passes[e->enums[0]];
				printf(fmt, pass_info->debug_name, e->enums[1]);
				break;
			};

			//
			// enums[0] = HeroPassEnum
			// enums[1] = HeroPassImageInputEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_ENUM_DOES_NOT_EXIST:
			case HERO_RENDER_GRAPH_ERROR_TYPE_IMAGE_INPUT_PASS_IMAGE_OUTPUT_ENUM_DOES_NOT_EXIST: {
				HeroPassInfo* pass_info = &setup->passes[e->enums[0]];
				printf(fmt, pass_info->debug_name, e->enums[1]);
				break;
			};

			//
			// enums[0] = HeroPassEnum
			// enums[1] = HeroPassBufferOutputEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_OUTPUT_BUFFER_ENUM_DOES_NOT_EXIST: {
				HeroPassInfo* pass_info = &setup->passes[e->enums[0]];
				printf(fmt, pass_info->debug_name, e->enums[1]);
				break;
			};

			//
			// enums[0] = HeroPassEnum
			// enums[1] = HeroPassBufferInputEnum
			case HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_ENUM_DOES_NOT_EXIST:
			case HERO_RENDER_GRAPH_ERROR_TYPE_BUFFER_INPUT_PASS_BUFFER_OUTPUT_ENUM_DOES_NOT_EXIST: {
				HeroPassInfo* pass_info = &setup->passes[e->enums[0]];
				printf(fmt, pass_info->debug_name, e->enums[1]);
				break;
			};
		}
	}
}

void _hero_render_graph_print_execution_unit_identation(U32 indent_count, FILE* f) {
	for_range(i, 0, indent_count) {
		fputs("    ", f);
	}
}

HeroImageOutput* _hero_render_graph_get_output_from_input_image(HeroRenderGraph* render_graph, HeroImageInput* input) {
	if (input->pass_enum == HERO_PASS_ENUM_INVALID) {
		return &render_graph->cpu_image_outputs[input->pass_image_output_enum];
	} else {
		HeroPassInfo* input_pass_info = &render_graph->passes[input->pass_enum];
		return &input_pass_info->image_outputs[input->pass_image_output_enum];
	}
}

HeroBufferOutput* _hero_render_graph_get_output_from_input_buffer(HeroRenderGraph* render_graph, HeroBufferInput* input) {
	if (input->pass_enum == HERO_PASS_ENUM_INVALID) {
		return &render_graph->cpu_buffer_outputs[input->pass_buffer_output_enum];
	} else {
		HeroPassInfo* input_pass_info = &render_graph->passes[input->pass_enum];
		return &input_pass_info->buffer_outputs[input->pass_buffer_output_enum];
	}
}

void hero_render_graph_print_execution_units(HeroLogicalDevice* ldev, HeroRenderGraphId id, FILE* f, bool color) {
	HeroRenderGraph* render_graph;
	HeroResult result = hero_render_graph_get(ldev, id, &render_graph);
	HERO_RESULT_ASSERT(result);

	for_range(execution_unit_idx, 0, render_graph->execution_units_count) {
		if (execution_unit_idx) {
			if (execution_unit_idx > 1) {
				_hero_render_graph_print_execution_unit_identation(execution_unit_idx - 1, f);
			}
			fputs("┗━━━┳", f);
		}
		fprintf(f, "EXE UNIT #%zu\n", execution_unit_idx);

		_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
		fputs("┃\n", f);

		HeroRangeU16 range = render_graph->execution_units_ranges[execution_unit_idx];
		for_range(pass_idx, range.start_idx, range.end_idx) {
			HeroPassEnum pass_enum = render_graph->execution_units_passes[pass_idx];
			HeroPassInfo* pass_info = &render_graph->passes[pass_enum];

			_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
			fprintf(f, "┣ PASS %s\n", pass_info->debug_name);

			if (pass_info->image_inputs_count || pass_info->buffer_inputs_count) {
				_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
				fputs("┃    ┣ INPUTS\n", f);
				for_range (image_input_enum, 0, pass_info->image_inputs_count) {
					HeroImageInput* image_input = &pass_info->image_inputs[image_input_enum];
					const char* pass_name = image_input->pass_enum == HERO_PASS_ENUM_INVALID ? "CPU" : render_graph->passes[image_input->pass_enum].debug_name;
					HeroImageOutput* image_output = _hero_render_graph_get_output_from_input_image(render_graph, image_input);

					_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
					if (image_input_enum + 1 == pass_info->image_inputs_count) {
						fprintf(f, "┃    ┃   ┗ PASS.%s.IMAGE.%s\n", pass_name, image_output->debug_name);
					} else {
						fprintf(f, "┃    ┃   ┣ PASS.%s.IMAGE.%s\n", pass_name, image_output->debug_name);
					}
				}
				for_range (buffer_input_enum, 0, pass_info->buffer_inputs_count) {
					HeroBufferInput* buffer_input = &pass_info->buffer_inputs[buffer_input_enum];
					const char* pass_name = buffer_input->pass_enum == HERO_PASS_ENUM_INVALID ? "CPU" : render_graph->passes[buffer_input->pass_enum].debug_name;
					HeroBufferOutput* buffer_output = _hero_render_graph_get_output_from_input_buffer(render_graph, buffer_input);

					_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
					if (buffer_input_enum + 1 == pass_info->buffer_inputs_count) {
						fprintf(f, "┃    ┃   ┗ PASS.%s.BUFFER.%s\n", pass_name, buffer_output->debug_name);
					} else {
						fprintf(f, "┃    ┃   ┣ PASS.%s.BUFFER.%s\n", pass_name, buffer_output->debug_name);
					}
				}
			}

			if (pass_info->image_outputs_count || pass_info->buffer_outputs_count) {
				_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
				fputs("┃    ┗ OUTPUTS\n", f);
				for_range (image_output_enum, 0, pass_info->image_outputs_count) {
					HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];

					_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
					if (image_output_enum + 1 == pass_info->image_outputs_count) {
						fprintf(f, "┃        ┗ IMAGE %s\n", image_output->debug_name);
					} else {
						fprintf(f, "┃        ┣ IMAGE %s\n", image_output->debug_name);
					}
				}
				for_range (buffer_output_enum, 0, pass_info->buffer_outputs_count) {
					HeroBufferOutput* buffer_output = &pass_info->buffer_outputs[buffer_output_enum];

					_hero_render_graph_print_execution_unit_identation(execution_unit_idx, f);
					if (buffer_output_enum + 1 == pass_info->buffer_outputs_count) {
						fprintf(f, "┃        ┗ BUFFER %s\n", buffer_output->debug_name);
					} else {
						fprintf(f, "┃        ┣ BUFFER %s\n", buffer_output->debug_name);
					}
				}
			}
		}
	}
}

void hero_render_graph_print_graphviz_dot(HeroLogicalDevice* ldev, HeroRenderGraphId id, FILE* f) {
	HeroRenderGraph* render_graph;
	HeroResult result = hero_render_graph_get(ldev, id, &render_graph);
	HERO_RESULT_ASSERT(result);

	fprintf(f, "digraph RenderGraph_%s {\n", render_graph->debug_name);
	fputs("\tgraph [\n", f);
	fputs("\t\tfontname=\"Liberation Mono bold\"\n", f);
	fputs("\t\tfontsize=18\n", f);
	fputs("\t]\n", f);
	fputs("\tnode [\n", f);
	fputs("\t\tfontname=\"Liberation Mono\"\n", f);
	fputs("\t]\n", f);
	fputs("\tedge [\n", f);
	fputs("\t\tfontname=\"Liberation Mono\"\n", f);
	fputs("\t]\n", f);
	fprintf(f, "\tlabel=\"RenderGraph: %s\"\n", render_graph->debug_name);

#if 0
		fprintf(f, "\tsubgraph cluster_1 {\n");
		fprintf(f, "\t\tlabel=\"Resources\"\n");
		for_range(image_info_enum, 0, render_graph->images_count) {
			HeroImageInfo* image_info = &render_graph->images[image_info_enum];
			fprintf(f, "\t\tIMAGE_%zu [label=\"{IMAGE.%s|{width|%u}|{height|%u}|{array_layers|%u}|{mip_levels|%u}|{samples_count|%u}|{format|%u}|{persistent|%u}|{readback|%u}}\", shape=\"record\"]\n", image_info_enum, image_info->debug_name, image_info->width, image_info->height, image_info->array_layers_count, image_info->mip_levels_count, (1 << image_info->samples_count_log2), image_info->format, !!(image_info->flags & HERO_IMAGE_INFO_FLAGS_PERSISTENT), !!(image_info->flags & HERO_IMAGE_INFO_FLAGS_READBACK));
		}

		for_range(buffer_info_enum, 0, render_graph->buffers_count) {
			HeroBufferInfo* buffer_info = &render_graph->buffers[buffer_info_enum];
			fprintf(f, "\t\tBUFFER_%zu [label=\"{BUFFER.%s|{size:%lu}|{persistent:%u}|{readback:%u}\", shape=\"record\"]\n", buffer_info_enum, buffer_info->debug_name, buffer_info->size, !!(buffer_info->flags & HERO_BUFFER_INFO_FLAGS_PERSISTENT), !!(buffer_info->flags & HERO_BUFFER_INFO_FLAGS_READBACK));
		}
		fprintf(f, "\t}\n");
#endif

	{
		fprintf(f, "\tsubgraph cluster_2 {\n");
		fprintf(f, "\t\tlabel=\"Graph\"\n");

		for_range(execution_unit_idx, 0, render_graph->execution_units_count) {
			fprintf(f, "\t\t{ rank=same ");
			for_range(pass_enum, 0, render_graph->passes_count) {
				HeroPassInfo* pass_info = &render_graph->passes[pass_enum];
				if (pass_info->execution_unit_idx == execution_unit_idx) {
					fprintf(f, "PASS_%zu\n", pass_enum);
				}
			}
			fprintf(f, "}\n");
		}

		//
		// CPU outputs
		{
			for_range(output_enum, 0, render_graph->cpu_image_outputs_count) {
				HeroImageOutput* image_output = &render_graph->cpu_image_outputs[output_enum];
				fprintf(f, "\t\tPASS_%u_IMAGE_OUTPUT_%zu [label=\"IMAGE.%s\", shape=\"box\"]\n", HERO_PASS_ENUM_INVALID, output_enum, image_output->debug_name);
			}

			for_range(output_enum, 0, render_graph->cpu_buffer_outputs_count) {
				HeroBufferOutput* buffer_output = &render_graph->cpu_buffer_outputs[output_enum];
				fprintf(f, "\t\tPASS_%u_BUFFER_OUTPUT_%zu [label=\"BUFFER.%s\", shape=\"cylinder\"]\n", HERO_PASS_ENUM_INVALID, output_enum, buffer_output->debug_name);
			}
		}

		for_range(pass_enum, 0, render_graph->passes_count) {
			HeroPassInfo* pass_info = &render_graph->passes[pass_enum];
			fprintf(f, "\t\tPASS_%zu [label=\"PASS.%s\", shape=\"circle\"]\n", pass_enum, pass_info->debug_name);

			for_range(output_enum, 0, pass_info->image_outputs_count) {
				HeroImageOutput* image_output = &pass_info->image_outputs[output_enum];
				fprintf(f, "\t\tPASS_%zu_IMAGE_OUTPUT_%zu [label=\"IMAGE.%s\", shape=\"box\"]\n", pass_enum, output_enum, image_output->debug_name);
				fprintf(f, "\t\tPASS_%zu -> PASS_%zu_IMAGE_OUTPUT_%zu [label=\"%zu\"]\n", pass_enum, pass_enum, output_enum, output_enum);
			}

			for_range(input_enum, 0, pass_info->image_inputs_count) {
				HeroImageInput* image_input = &pass_info->image_inputs[input_enum];
				U16 input_pass_execution_unit_idx = image_input->pass_enum == HERO_PASS_ENUM_INVALID ? 0 : render_graph->passes[image_input->pass_enum].execution_unit_idx;
				const char* style = input_pass_execution_unit_idx > pass_info->execution_unit_idx ? ", style=dashed" : "";
				fprintf(f, "\t\tPASS_%u_IMAGE_OUTPUT_%u -> PASS_%zu [label=\"%zu\"%s]", image_input->pass_enum, image_input->pass_image_output_enum, pass_enum, input_enum, style);
			}

			for_range(output_enum, 0, pass_info->buffer_outputs_count) {
				HeroBufferOutput* buffer_output = &pass_info->buffer_outputs[output_enum];
				fprintf(f, "\t\tPASS_%zu_BUFFER_OUTPUT_%zu [label=\"BUFFER.%s\", shape=\"cylinder\"]\n", pass_enum, output_enum, buffer_output->debug_name);
				fprintf(f, "\t\tPASS_%zu -> PASS_%zu_BUFFER_OUTPUT_%zu [label=\"%zu\"]\n", pass_enum, pass_enum, output_enum, output_enum);
			}

			for_range(input_enum, 0, pass_info->buffer_inputs_count) {
				HeroBufferInput* buffer_input = &pass_info->buffer_inputs[input_enum];
				U16 input_pass_execution_unit_idx = buffer_input->pass_enum == HERO_PASS_ENUM_INVALID ? 0 : render_graph->passes[buffer_input->pass_enum].execution_unit_idx;
				const char* style = input_pass_execution_unit_idx > pass_info->execution_unit_idx ? ", style=dashed" : "";
				fprintf(f, "\t\tPASS_%u_BUFFER_OUTPUT_%u -> PASS_%zu [label=\"%zu\"%s]", buffer_input->pass_enum, buffer_input->pass_buffer_output_enum, pass_enum, input_enum, style);
			}
		}
		fprintf(f, "\t}\n");
	}

	{
		fprintf(f, "\tsubgraph cluster_3 {\n");
		fprintf(f, "\t\tlabel=\"Execution Units\"\n");

		for_range(execution_unit_idx, 0, render_graph->execution_units_count) {
			fprintf(f, "\t\tEXE_UNIT_%zu [label=\"EXE_UNIT.%zu\", shape=\"house\"]\n", execution_unit_idx, execution_unit_idx);
			if (execution_unit_idx) {
				fprintf(f, "\t\tEXE_UNIT_%zu -> EXE_UNIT_%zu\n", execution_unit_idx - 1, execution_unit_idx);
			}
		}

		for_range(pass_enum, 0, render_graph->passes_count) {
			HeroPassInfo* pass_info = &render_graph->passes[pass_enum];
			fprintf(f, "\t\tEXE_UNIT_PASS_%zu [label=\"PASS.%s\", shape=\"circle\"]\n", pass_enum, pass_info->debug_name);
			fprintf(f, "\t\tEXE_UNIT_%u -> EXE_UNIT_PASS_%zu\n", pass_info->execution_unit_idx, pass_enum);
		}
		fprintf(f, "\t}\n");
	}

	fprintf(f, "}\n");
}
#endif

// ===========================================
//
//
// Backend: Vulkan
//
//
// ===========================================

#if HERO_VULKAN_ENABLE

#include <vulkan/vulkan_core.h>

#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif

#ifdef __APPLE__
#include <vulkan/vulkan_macos.h>
#include <vulkan/vulkan_ios.h>
#endif

#ifdef __unix__


#if HERO_X11_ENABLE
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif // HERO_X11_ENABLE

#ifdef HERO_WAYLAND_ENABLE
struct wl_display;
struct wl_surface;
#include <vulkan/vulkan_wayland.h>
#endif

#endif // __unix__

#ifdef __ANDROID__
#include <vulkan/vulkan_android.h>
#endif

#define HERO_VULKAN_TODO_ALLOCATOR NULL

#define HERO_VULKAN_FN(NAME) static PFN_##NAME NAME;
#define HERO_VULKAN_INSTANCE_FN(NAME) HERO_VULKAN_FN(NAME)
#define HERO_VULKAN_DEVICE_FN(NAME)
HERO_VULKAN_FN_LIST
#undef HERO_VULKAN_FN
#undef HERO_VULKAN_INSTANCE_FN
#undef HERO_VULKAN_DEVICE_FN

#ifdef _HERO_WINDOW_H_
VkResult _hero_window_vulkan_create_surface(HeroWindow* window, VkInstance instance, VkSurfaceKHR* surface_out) {
	switch (hero_window_sys.backend_type) {
#if HERO_X11_ENABLE
		case HERO_WINDOW_SYS_BACKEND_TYPE_X11: {
			VkXlibSurfaceCreateInfoKHR create_info = {
				.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
				.pNext = NULL,
				.flags = 0,
				.dpy = hero_window_sys.backend.x11.connection,
				.window = window->backend.x11.xwindow,
			};

			return vkCreateXlibSurfaceKHR(instance, &create_info, HERO_VULKAN_TODO_ALLOCATOR, surface_out);
	   };
#endif // HERO_X11_ENABLE
	}

	return VK_SUCCESS;
}

VkBool32 _hero_vulkan_queue_has_presentation_support(VkPhysicalDevice physical_device, U32 queue_family_idx) {
	switch (hero_window_sys.backend_type) {
#if HERO_X11_ENABLE
		case HERO_WINDOW_SYS_BACKEND_TYPE_X11: {
			XDisplay* connection = hero_window_sys.backend.x11.connection;
			XVisualID visual_id = hero_window_sys.backend.x11.visual_id;
			return vkGetPhysicalDeviceXlibPresentationSupportKHR(physical_device, queue_family_idx, connection, visual_id);
	   };
#endif // HERO_X11_ENABLE
	}
	return false;
}
#endif // _HERO_WINDOW_H_

// ===========================================
//
//
// Backend: Vulkan SPIR-V Parser
//
//
// ===========================================

typedef struct _HeroSpirVDescriptor _HeroSpirVDescriptor;
struct _HeroSpirVDescriptor {
	U8                 set;
	HeroSpirVDescriptorBinding binding;
};

typedef struct _HeroSpirVDescriptorType _HeroSpirVDescriptorType;
struct _HeroSpirVDescriptorType {
	VkDescriptorType type;
	U32 count;
};

typedef struct _HeroSpirVDescriptorKey _HeroSpirVDescriptorKey;
struct _HeroSpirVDescriptorKey {
	U32 set;
	U32 binding;
};

typedef union _HeroSpirVConstant _HeroSpirVConstant;
union _HeroSpirVConstant {
	U32 u32;
	S32 s32;
	F32 f32;
};

#define HERO_HASH_TABLE_KEY_TYPE U32
#define HERO_HASH_TABLE_VALUE_TYPE _HeroSpirVConstant
#include "hash_table_gen.inl"

#define HERO_HASH_TABLE_KEY_TYPE U32
#define HERO_HASH_TABLE_VALUE_TYPE HeroSpirVVertexAttribInfo
#include "hash_table_gen.inl"

#define HERO_HASH_TABLE_KEY_TYPE U32
#define HERO_HASH_TABLE_VALUE_TYPE U32
#include "hash_table_gen.inl"

#define HERO_HASH_TABLE_KEY_TYPE U32
#define HERO_HASH_TABLE_VALUE_TYPE _HeroSpirVDescriptor
#include "hash_table_gen.inl"

#define HERO_HASH_TABLE_KEY_TYPE _HeroSpirVDescriptorKey
#define HERO_HASH_TABLE_VALUE_TYPE _HeroSpirVDescriptor
#include "hash_table_gen.inl"

#define HERO_HASH_TABLE_KEY_TYPE U32
#define HERO_HASH_TABLE_VALUE_TYPE _HeroSpirVDescriptorType
#include "hash_table_gen.inl"

#define HERO_STACK_ELMT_TYPE HeroSpirVVertexAttribInfo
#include "stack_gen.inl"

typedef struct _HeroSpirVInspect _HeroSpirVInspect;
struct _HeroSpirVInspect {
	HeroHashTable(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor) key_to_descriptor_map;
	HeroStack(HeroSpirVVertexAttribInfo) vertex_attribs;
	U32 push_constants_size;
};

#define HERO_SPIR_V_MAGIC_NUMBER       0x07230203
#define HERO_SPIR_V_MAGIC_NUMBER_BSWAP 0x03022307

enum {
	_HERO_SPIR_V_OP_CODE_ENTRY_POINT        = 15,
	_HERO_SPIR_V_OP_CODE_TYPE_IMAGE         = 25,
	_HERO_SPIR_V_OP_CODE_TYPE_SAMPLER       = 26,
	_HERO_SPIR_V_OP_CODE_TYPE_SAMPLED_IMAGE = 27,
	_HERO_SPIR_V_OP_CODE_TYPE_BOOL          = 20,
	_HERO_SPIR_V_OP_CODE_TYPE_INT           = 21,
	_HERO_SPIR_V_OP_CODE_TYPE_FLOAT         = 22,
	_HERO_SPIR_V_OP_CODE_TYPE_VECTOR        = 23,
	_HERO_SPIR_V_OP_CODE_TYPE_MATRIX        = 24,
	_HERO_SPIR_V_OP_CODE_TYPE_ARRAY         = 28,
	_HERO_SPIR_V_OP_CODE_TYPE_STRUCT        = 30,
	_HERO_SPIR_V_OP_CODE_TYPE_POINTER       = 32,
	_HERO_SPIR_V_OP_CODE_CONSTANT           = 43,
	_HERO_SPIR_V_OP_CODE_FUNCTION           = 54,
	_HERO_SPIR_V_OP_CODE_VARIABLE           = 59,
	_HERO_SPIR_V_OP_CODE_LOAD               = 61,
	_HERO_SPIR_V_OP_CODE_ACCESS_CHAIN       = 65,
	_HERO_SPIR_V_OP_CODE_DECORATE           = 71,
};

enum {
	_HERO_SPIR_V_DECORATION_LOCATION       = 30,
	_HERO_SPIR_V_DECORATION_BINDING        = 33,
	_HERO_SPIR_V_DECORATION_DESCRIPTOR_SET = 34,
};

enum {
	_HERO_SPIR_V_DIM_BUFFER       = 5,
	_HERO_SPIR_V_DIM_SUBPASS_DATA = 6,
};

enum {
	_HERO_SPIR_V_STORAGE_CLASS_UNIFORM_CONSTANT = 0,
	_HERO_SPIR_V_STORAGE_CLASS_INPUT            = 1,
	_HERO_SPIR_V_STORAGE_CLASS_UNIFORM          = 2,
	_HERO_SPIR_V_STORAGE_CLASS_PUSH_CONSTANT    = 9,
	_HERO_SPIR_V_STORAGE_CLASS_STORAGE_BUFFER   = 12,
};

enum {
	_HERO_SPIR_V_EXE_MODEL_VERTEX                  = 0,
	_HERO_SPIR_V_EXE_MODEL_TESSELLATION_CONTROL    = 1,
	_HERO_SPIR_V_EXE_MODEL_TESSELLATION_EVALUATION = 2,
	_HERO_SPIR_V_EXE_MODEL_GEOMETRY                = 3,
	_HERO_SPIR_V_EXE_MODEL_FRAGMENT                = 4,
	_HERO_SPIR_V_EXE_MODEL_GL_COMPUTE              = 5,
};

typedef U32 (*_HeroVulkanSpirVDecodeWordFn)(U32* code, U32 code_op_count, U32* idx_mut);

U32 _hero_vulkan_spir_v_decode_word(U32* code, U32 code_op_count, U32* idx_mut) {
	HERO_ASSERT_ARRAY_BOUNDS(*idx_mut, code_op_count);
	U32 op_code = code[*idx_mut];
	*idx_mut += 1;
	return op_code;
}

U32 _hero_vulkan_spir_v_decode_word_bswap(U32* code, U32 code_op_count, U32* idx_mut) {
	HERO_ASSERT_ARRAY_BOUNDS(*idx_mut, code_op_count);
	U32 op_code = code[*idx_mut];
	*idx_mut += 1;
	return hero_bswap_32(op_code);
}

HeroResult _hero_vulkan_spir_v_descriptor_find_or_insert(HeroHashTable(U32, _HeroSpirVDescriptor)* id_to_descriptor_map, U32 target_id, HeroHashTableEntry(U32, _HeroSpirVDescriptor)** entry_out) {
	HeroResult result = hero_hash_table(U32, _HeroSpirVDescriptor, find_or_insert)(id_to_descriptor_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_MAP, entry_out);
	if (result < 0) {
		return result;
	}

	if (result == HERO_SUCCESS_IS_NEW) {
		HERO_ZERO_ELMT(&(*entry_out)->value);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_spir_v_insert_sizes(HeroHashTable(U32, U32)* id_to_size_std140_map, HeroHashTable(U32, U32)* id_to_size_std430_map, U32 target_id, U32 size_std140, U32 size_std430) {
	HeroHashTableEntry(U32, U32)* entry_std140;
	HeroResult result = hero_hash_table(U32, U32, find_or_insert)(id_to_size_std140_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_SIZE_STD140_MAP, &entry_std140);
	if (result < 0) {
		return result;
	}
	if (result == HERO_SUCCESS_IS_NEW) {
		entry_std140->value = size_std140;
	}

	HeroHashTableEntry(U32, U32)* entry_std430;
	result = hero_hash_table(U32, U32, find_or_insert)(id_to_size_std430_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_SIZE_STD430_MAP, &entry_std430);
	if (result < 0) {
		return result;
	}
	if (result == HERO_SUCCESS_IS_NEW) {
		entry_std430->value = size_std430;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_spir_v_insert_vertex_attrib_info(HeroHashTable(U32, HeroSpirVVertexAttribInfo)* id_to_vertex_attrib_info, U32 target_id, U8 location, HeroSpirVVertexElmtType elmt_type, HeroVertexVectorType vector_type) {
	HeroHashTableEntry(U32, HeroSpirVVertexAttribInfo)* entry;
	HeroResult result = hero_hash_table(U32, HeroSpirVVertexAttribInfo, find_or_insert)(id_to_vertex_attrib_info, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_VERTEX_ATTRIB_MAP, &entry);
	if (result < 0) {
		return result;
	}

	entry->value.location = location;
	entry->value.elmt_type = elmt_type;
	entry->value.vector_type = vector_type;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_metadata_spir_v_inspect(U32* code, U32 code_size, _HeroSpirVInspect* inspect) {
	HeroResult result;
	U32 code_op_count = code_size / sizeof(U32);
	if (code_op_count == 0) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	//
	// the SPIR-V spec can be found here: https://www.khronos.org/registry/SPIR-V/specs/1.0/SPIRV.html
	//

	//
	// parse the header
	//
	_HeroVulkanSpirVDecodeWordFn endian_decode_word_fn;
	U32 idx = 1;
	{
		if (code[0] == HERO_SPIR_V_MAGIC_NUMBER) {
			endian_decode_word_fn = _hero_vulkan_spir_v_decode_word;
		} else if (code[0] == HERO_SPIR_V_MAGIC_NUMBER_BSWAP) {
			endian_decode_word_fn = _hero_vulkan_spir_v_decode_word_bswap;
		} else {
			return HERO_ERROR(GFX_SPIR_V_BINARY_INVALID_FORMAT);
		}

		if (idx >= code_op_count) {
			return HERO_ERROR(GFX_SPIR_V_BINARY_CORRUPT);
		}

		U32 version = endian_decode_word_fn(code, code_op_count, &idx);
		U8 version_major = (version & 0x00ff0000) >> 16;
		U8 version_minor = (version & 0x0000ff00) >> 8;
		if (version_major == 0) {
			return HERO_ERROR(GFX_SPIR_V_BINARY_UNSUPPORTED_VERSION);
		}

		if (idx >= code_op_count) {
			return HERO_ERROR(GFX_SPIR_V_BINARY_CORRUPT);
		}
		U32 compiler_magic_number = endian_decode_word_fn(code, code_op_count, &idx);

		if (idx >= code_op_count) {
			return HERO_ERROR(GFX_SPIR_V_BINARY_CORRUPT);
		}
		U32 bound_aka_end_id = endian_decode_word_fn(code, code_op_count, &idx);

		if (idx >= code_op_count) {
			return HERO_ERROR(GFX_SPIR_V_BINARY_CORRUPT);
		}
		U32 instruction_schema = endian_decode_word_fn(code, code_op_count, &idx);
		if (instruction_schema != 0) {
			return HERO_ERROR(GFX_SPIR_V_BINARY_UNSUPPORTED_INSTRUCTION_SCHEMA);
		}
	}

	HeroHashTable(U32, _HeroSpirVDescriptor)      id_to_descriptor_map      = {0};
	HeroHashTable(U32, _HeroSpirVDescriptorType)  id_to_descriptor_type_map = {0};
	HeroHashTable(U32, _HeroSpirVConstant)        id_to_constant_map        = {0};
	HeroHashTable(U32, U32)                       id_to_size_std140_map     = {0};
	HeroHashTable(U32, U32)                       id_to_size_std430_map     = {0};
	HeroHashTable(U32, U32)                       id_to_location_map        = {0};
	HeroHashTable(U32, HeroSpirVVertexAttribInfo) id_to_vertex_attrib_info  = {0};
	HeroHashTable(U32, U32)                       id_to_non_vertex_input_hash_set_todo    = {0};
	U32 type_id_u32 = 0;
	U32 type_id_s32 = 0;
	U32 type_id_f32 = 0;

	U32 shader_stage_fn_id_vertex = 0;
	U32 shader_stage_fn_id_tessellation_control = 0;
	U32 shader_stage_fn_id_tessellation_evaluation = 0;
	U32 shader_stage_fn_id_geometry = 0;
	U32 shader_stage_fn_id_fragment = 0;
	U32 shader_stage_fn_id_gl_compute = 0;

	VkShaderStageFlags shader_stage = 0;

	while (idx < code_op_count) {
		U32 instruction_words_count_and_op_code = endian_decode_word_fn(code, code_op_count, &idx);
		U32 words_count = (instruction_words_count_and_op_code & 0xffff0000) >> 16;
		U32 op_code     = (instruction_words_count_and_op_code & 0x0000ffff) >> 0;

		U32 next_instruction_idx = idx + words_count - 1;

		switch (op_code) {
			case _HERO_SPIR_V_OP_CODE_ENTRY_POINT: {
				U32 exe_model = endian_decode_word_fn(code, code_op_count, &idx);
				U32 fn_id = endian_decode_word_fn(code, code_op_count, &idx);

				switch (exe_model) {
					case _HERO_SPIR_V_EXE_MODEL_VERTEX: shader_stage_fn_id_vertex = fn_id; break;
					case _HERO_SPIR_V_EXE_MODEL_TESSELLATION_CONTROL: shader_stage_fn_id_tessellation_control = fn_id; break;
					case _HERO_SPIR_V_EXE_MODEL_TESSELLATION_EVALUATION: shader_stage_fn_id_tessellation_evaluation = fn_id; break;
					case _HERO_SPIR_V_EXE_MODEL_GEOMETRY: shader_stage_fn_id_geometry = fn_id; break;
					case _HERO_SPIR_V_EXE_MODEL_FRAGMENT: shader_stage_fn_id_fragment = fn_id; break;
					case _HERO_SPIR_V_EXE_MODEL_GL_COMPUTE: shader_stage_fn_id_gl_compute = fn_id; break;
				}

				//
				// skip the name string
				while (1) {
					U32 word = endian_decode_word_fn(code, code_op_count, &idx);
					if (
						((word >> 0) & 0xff) == 0 ||
						((word >> 8) & 0xff) == 0 ||
						((word >> 16) & 0xff) == 0 ||
						((word >> 24) & 0xff) == 0
					) break;
				}

				//
				// we want to keep track of all inputs and outputs on non vertex shaders
				// so we can know which ones are vertex inputs.
				if (exe_model != _HERO_SPIR_V_EXE_MODEL_VERTEX) {
					U32 inputs_and_outputs_count = next_instruction_idx - idx;
					for_range(i, 0, inputs_and_outputs_count) {
						U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);

						HeroHashTableEntry(U32, U32)* entry;
						HeroResult result = hero_hash_table(U32, U32, find_or_insert)(&id_to_non_vertex_input_hash_set_todo, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_NON_VERTEX_INPUT, &entry);
						if (result < 0) {
							return result;
						}
					}
				}

				break;
			};
			case _HERO_SPIR_V_OP_CODE_FUNCTION: {
				U32 result_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 fn_id = endian_decode_word_fn(code, code_op_count, &idx);
				if (shader_stage_fn_id_vertex == fn_id) {
					shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
				} else if (shader_stage_fn_id_tessellation_control == fn_id) {
					shader_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				} else if (shader_stage_fn_id_tessellation_evaluation == fn_id) {
					shader_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				} else if (shader_stage_fn_id_geometry == fn_id) {
					shader_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				} else if (shader_stage_fn_id_fragment == fn_id) {
					shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				} else if (shader_stage_fn_id_gl_compute == fn_id) {
					shader_stage = VK_SHADER_STAGE_COMPUTE_BIT;
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_CONSTANT: {
				U32 type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 literal = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, _HeroSpirVConstant)* entry;
				result = hero_hash_table(U32, _HeroSpirVConstant, find_or_insert)(&id_to_constant_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_CONSTANT_MAP, &entry);
				if (result < 0) {
					return result;
				}

				if (type_id == type_id_u32 || type_id == type_id_s32 || type_id == type_id_f32) {
					entry->value.u32 = literal;
				}

				break;
			};
			case _HERO_SPIR_V_OP_CODE_DECORATE: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 decoration = endian_decode_word_fn(code, code_op_count, &idx);
				U32 literal = endian_decode_word_fn(code, code_op_count, &idx);

				switch (decoration) {
					case _HERO_SPIR_V_DECORATION_LOCATION: {
						HeroHashTableEntry(U32, U32)* entry;
						result = hero_hash_table(U32, U32, find_or_insert)(&id_to_location_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_LOCATION_MAP, &entry);
						if (result < 0) {
							return result;
						}

						entry->value = literal;
						break;
					};
					case _HERO_SPIR_V_DECORATION_BINDING: {
						HeroHashTableEntry(U32, _HeroSpirVDescriptor)* entry;
						result = _hero_vulkan_spir_v_descriptor_find_or_insert(&id_to_descriptor_map, target_id, &entry);
						if (result < 0) {
							return result;
						}
						entry->value.binding.binding = literal;
						break;
					};
					case _HERO_SPIR_V_DECORATION_DESCRIPTOR_SET: {
						HeroHashTableEntry(U32, _HeroSpirVDescriptor)* entry;
						result = _hero_vulkan_spir_v_descriptor_find_or_insert(&id_to_descriptor_map, target_id, &entry);
						if (result < 0) {
							return result;
						}
						entry->value.set = literal;
						break;
					};
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_IMAGE: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 sample_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 dim = endian_decode_word_fn(code, code_op_count, &idx);
				U32 depth = endian_decode_word_fn(code, code_op_count, &idx);
				U32 arrayed = endian_decode_word_fn(code, code_op_count, &idx);
				U32 ms = endian_decode_word_fn(code, code_op_count, &idx);
				U32 sampled = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find_or_insert)(&id_to_descriptor_type_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_TYPE_MAP, &entry);
				if (result < 0) {
					return result;
				}
				entry->value.count = 1;

				switch (dim) {
					case _HERO_SPIR_V_DIM_BUFFER:
						if (sampled == 1) {
							entry->value.type = HERO_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
						} else if (sampled == 2) {
							entry->value.type = HERO_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
						}
						break;
					case _HERO_SPIR_V_DIM_SUBPASS_DATA:
						entry->value.type = HERO_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
						break;
					default:
						if (sampled == 1) {
							entry->value.type = HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
						} else if (sampled == 2) {
							entry->value.type = HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE;
						}
						break;
				}

				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_SAMPLER: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find_or_insert)(&id_to_descriptor_type_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_TYPE_MAP, &entry);
				if (result < 0) {
					return result;
				}

				entry->value.count = 1;
				entry->value.type = HERO_DESCRIPTOR_TYPE_SAMPLER;
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_SAMPLED_IMAGE: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 image_type_id = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* image_entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find)(&id_to_descriptor_type_map, &image_type_id, &image_entry);
				HERO_RESULT_ASSERT(result);

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find_or_insert)(&id_to_descriptor_type_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_TYPE_MAP, &entry);
				if (result < 0) {
					return result;
				}

				if (image_entry->value.type == HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
					entry->value.type = HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					entry->value.count = 1;
				} else {
					entry->value = image_entry->value;
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_BOOL: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);

				result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, sizeof(U32), sizeof(U8));
				if (result < 0) {
					return result;
				}

				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_INT: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 width = endian_decode_word_fn(code, code_op_count, &idx);
				U32 signedness = endian_decode_word_fn(code, code_op_count, &idx);

				if (width == 32) {
					HeroSpirVVertexElmtType elmt_type;
					if (signedness == 0) {
						type_id_u32 = target_id;
						elmt_type = HERO_SHADER_VERTEX_ELMT_TYPE_UINT;
					} else {
						type_id_s32 = target_id;
						elmt_type = HERO_SHADER_VERTEX_ELMT_TYPE_SINT;
					}

					result = _hero_vulkan_spir_v_insert_vertex_attrib_info(&id_to_vertex_attrib_info, target_id, 0, elmt_type, HERO_VERTEX_VECTOR_TYPE_1);
					if (result < 0) {
						return result;
					}
				}

				result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, width / 8, width / 8);
				if (result < 0) {
					return result;
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_FLOAT: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 width = endian_decode_word_fn(code, code_op_count, &idx);

				if (width == 32) {
					type_id_f32 = target_id;

					result = _hero_vulkan_spir_v_insert_vertex_attrib_info(&id_to_vertex_attrib_info, target_id, 0, HERO_SHADER_VERTEX_ELMT_TYPE_FLOAT, HERO_VERTEX_VECTOR_TYPE_1);
					if (result < 0) {
						return result;
					}
				}

				result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, width / 8, width / 8);
				if (result < 0) {
					return result;
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_VECTOR: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 elmt_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 elmts_count = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, HeroSpirVVertexAttribInfo)* elmt_attrib_entry;
				result = hero_hash_table(U32, HeroSpirVVertexAttribInfo, find)(&id_to_vertex_attrib_info, &elmt_type_id, &elmt_attrib_entry);
				if (result >= 0) {
					result = _hero_vulkan_spir_v_insert_vertex_attrib_info(&id_to_vertex_attrib_info, target_id, elmt_attrib_entry->value.location, elmt_attrib_entry->value.elmt_type, HERO_VERTEX_VECTOR_TYPE_INIT(elmts_count));
					if (result < 0) {
						return result;
					}
				}

				HeroHashTableEntry(U32, U32)* elmt_entry_std140;
				result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &elmt_type_id, &elmt_entry_std140);
				HERO_RESULT_ASSERT(result);

				HeroHashTableEntry(U32, U32)* elmt_entry_std430;
				result = hero_hash_table(U32, U32, find)(&id_to_size_std430_map, &elmt_type_id, &elmt_entry_std430);
				HERO_RESULT_ASSERT(result);

				U32 elmts_count_std140 = elmts_count == 3 ? 4 : elmts_count;
				result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, elmt_entry_std140->value * elmts_count_std140, elmt_entry_std430->value * elmts_count);
				if (result < 0) {
					return result;
				}

				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_MATRIX: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 column_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 columns_count = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, U32)* column_entry_std140;
				result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &column_type_id, &column_entry_std140);
				HERO_RESULT_ASSERT(result);

				HeroHashTableEntry(U32, U32)* column_entry_std430;
				result = hero_hash_table(U32, U32, find)(&id_to_size_std430_map, &column_type_id, &column_entry_std430);
				HERO_RESULT_ASSERT(result);

				result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, column_entry_std140->value * columns_count, column_entry_std430->value * columns_count);
				if (result < 0) {
					return result;
				}

				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_ARRAY: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 elmt_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 length_constant_id = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, _HeroSpirVConstant)* constant_entry;
				result = hero_hash_table(U32, _HeroSpirVConstant, find)(&id_to_constant_map, &length_constant_id, &constant_entry);
				HERO_RESULT_ASSERT(result);

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* descriptor_entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find_or_insert)(&id_to_descriptor_type_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_TYPE_MAP, &descriptor_entry);
				if (result < 0) {
					return result;
				}

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* elmt_type_entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find)(&id_to_descriptor_type_map, &elmt_type_id, &elmt_type_entry);
				if (result >= 0) {
					descriptor_entry->value.type = elmt_type_entry->value.type;
				} else {
					descriptor_entry->value.type = HERO_DESCRIPTOR_TYPE_COUNT;
				}

				descriptor_entry->value.count = constant_entry->value.u32;

				if (result < 0) {
					HeroHashTableEntry(U32, U32)* elmt_entry_std140;
					result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &elmt_type_id, &elmt_entry_std140);
					HERO_RESULT_ASSERT(result);

					HeroHashTableEntry(U32, U32)* elmt_entry_std430;
					result = hero_hash_table(U32, U32, find)(&id_to_size_std430_map, &elmt_type_id, &elmt_entry_std430);
					HERO_RESULT_ASSERT(result);

					U32 std140_size = elmt_entry_std140->value * constant_entry->value.u32;
					result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, HERO_INT_ROUND_UP_ALIGN(std140_size, sizeof(Vec4)), elmt_entry_std430->value * constant_entry->value.u32);
					if (result < 0) {
						return result;
					}
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_STRUCT: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 fields_count = words_count - 2;

				U32 size_std140 = 0;
				U32 size_std430 = 0;
				for (U32 i = 0; i < fields_count; i += 1) {
					U32 field_type_id = endian_decode_word_fn(code, code_op_count, &idx);

					HeroHashTableEntry(U32, U32)* field_entry_std140;
					result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &field_type_id, &field_entry_std140);
					HERO_RESULT_ASSERT(result);
					size_std140 += field_entry_std140->value;

					HeroHashTableEntry(U32, U32)* field_entry_std430;
					result = hero_hash_table(U32, U32, find)(&id_to_size_std430_map, &field_type_id, &field_entry_std430);
					HERO_RESULT_ASSERT(result);
					size_std430 += field_entry_std430->value;
				}

				result = _hero_vulkan_spir_v_insert_sizes(&id_to_size_std140_map, &id_to_size_std430_map, target_id, size_std140, size_std430);
				if (result < 0) {
					return result;
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_TYPE_POINTER: {
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 storage_class = endian_decode_word_fn(code, code_op_count, &idx);
				U32 elmt_type_id = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, HeroSpirVVertexAttribInfo)* elmt_attrib_entry;
				result = hero_hash_table(U32, HeroSpirVVertexAttribInfo, find)(&id_to_vertex_attrib_info, &elmt_type_id, &elmt_attrib_entry);
				if (result >= 0) {
					HeroHashTableEntry(U32, HeroSpirVVertexAttribInfo)* entry;
					result = hero_hash_table(U32, HeroSpirVVertexAttribInfo, find_or_insert)(&id_to_vertex_attrib_info, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_VERTEX_ATTRIB_MAP, &entry);
					if (result < 0) {
						return result;
					}

					entry->value = elmt_attrib_entry->value;
				}

				HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* type_entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptorType, find)(&id_to_descriptor_type_map, &elmt_type_id, &type_entry);
				if (result >= 0) {
					HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
					result = hero_hash_table(U32, _HeroSpirVDescriptorType, find_or_insert)(&id_to_descriptor_type_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_DESCRIPTOR_TYPE_MAP, &entry);
					if (result < 0) {
						return result;
					}

					entry->value = type_entry->value;
				}

				HeroHashTableEntry(U32, U32)* elmt_entry_std140;
				result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &elmt_type_id, &elmt_entry_std140);
				if (result >= 0) {
					HeroHashTableEntry(U32, U32)* entry_std140;
					result = hero_hash_table(U32, U32, find_or_insert)(&id_to_size_std140_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_SIZE_STD140_MAP, &entry_std140);
					if (result == HERO_SUCCESS_IS_NEW) {
						entry_std140->value = elmt_entry_std140->value;
					}
				}

				HeroHashTableEntry(U32, U32)* elmt_entry_std430;
				result = hero_hash_table(U32, U32, find)(&id_to_size_std430_map, &elmt_type_id, &elmt_entry_std430);
				if (result >= 0) {
					HeroHashTableEntry(U32, U32)* entry_std430;
					result = hero_hash_table(U32, U32, find_or_insert)(&id_to_size_std430_map, &target_id, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_ID_TO_SIZE_STD430_MAP, &entry_std430);
					if (result == HERO_SUCCESS_IS_NEW) {
						entry_std430->value = elmt_entry_std430->value;
					}
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_VARIABLE: {
				U32 target_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 storage_class = endian_decode_word_fn(code, code_op_count, &idx);

				VkDescriptorType descriptor_type = HERO_DESCRIPTOR_TYPE_COUNT;
				U32 descriptor_count = 1;
				U32 descriptor_elmt_size = 0;

				switch (storage_class) {
					case _HERO_SPIR_V_STORAGE_CLASS_UNIFORM_CONSTANT: {
						HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
						result = hero_hash_table(U32, _HeroSpirVDescriptorType, find)(&id_to_descriptor_type_map, &target_type_id, &entry);
						HERO_RESULT_ASSERT(result);

						descriptor_type = entry->value.type;
						descriptor_count = entry->value.count;
						break;
					};
					case _HERO_SPIR_V_STORAGE_CLASS_INPUT: {
						HeroHashTableEntry(U32, U32)* non_vertex_input_entry;
						result = hero_hash_table(U32, U32, find)(&id_to_non_vertex_input_hash_set_todo, &target_id, &non_vertex_input_entry);
						if (result >= 0) {
							break;
						}

						HeroHashTableEntry(U32, HeroSpirVVertexAttribInfo)* attrib_entry;
						result = hero_hash_table(U32, HeroSpirVVertexAttribInfo, find)(&id_to_vertex_attrib_info, &target_type_id, &attrib_entry);
						if (result < 0) {
							break;
						}

						HeroHashTableEntry(U32, U32)* location_entry;
						result = hero_hash_table(U32, U32, find)(&id_to_location_map, &target_id, &location_entry);

						if (result >= 0) {
							HeroSpirVVertexAttribInfo* dst;
							result = hero_stack(HeroSpirVVertexAttribInfo, push)(&inspect->vertex_attribs, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_VERTEX_ATTRIBS, &dst);
							if (result < 0) {
								return result;
							}

							*dst = attrib_entry->value;
							dst->location = location_entry->value;
						}
						break;
					};
					case _HERO_SPIR_V_STORAGE_CLASS_UNIFORM: {
						descriptor_type = HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

						HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
						result = hero_hash_table(U32, _HeroSpirVDescriptorType, find)(&id_to_descriptor_type_map, &target_type_id, &entry);
						if (result >= 0) {
							descriptor_count = entry->value.count;
						}

						HeroHashTableEntry(U32, U32)* entry_std140;
						result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &target_type_id, &entry_std140);
						HERO_RESULT_ASSERT(result);
						descriptor_elmt_size = entry_std140->value;

						break;
					};
					case _HERO_SPIR_V_STORAGE_CLASS_PUSH_CONSTANT: {
						HeroHashTableEntry(U32, U32)* entry_std430;
						result = hero_hash_table(U32, U32, find)(&id_to_size_std430_map, &target_type_id, &entry_std430);
						HERO_RESULT_ASSERT(result);

						inspect->push_constants_size = entry_std430->value;
						break;
					};
					case _HERO_SPIR_V_STORAGE_CLASS_STORAGE_BUFFER: {
						descriptor_type = HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER;

						HeroHashTableEntry(U32, _HeroSpirVDescriptorType)* entry;
						result = hero_hash_table(U32, _HeroSpirVDescriptorType, find)(&id_to_descriptor_type_map, &target_type_id, &entry);
						if (result >= 0) {
							descriptor_count = entry->value.count;
						}

						HeroHashTableEntry(U32, U32)* entry_std140;
						result = hero_hash_table(U32, U32, find)(&id_to_size_std140_map, &target_type_id, &entry_std140);
						HERO_RESULT_ASSERT(result);
						descriptor_elmt_size = entry_std140->value;

						break;
					};
				}

				if (descriptor_type != HERO_DESCRIPTOR_TYPE_COUNT) {
					HeroHashTableEntry(U32, _HeroSpirVDescriptor)* entry;
					result = _hero_vulkan_spir_v_descriptor_find_or_insert(&id_to_descriptor_map, target_id, &entry);
					if (result < 0) {
						return result;
					}
					entry->value.binding.descriptor_type = descriptor_type;
					entry->value.binding.descriptor_count = descriptor_count;
					entry->value.binding.elmt_size = descriptor_elmt_size;

					_HeroSpirVDescriptorKey key = { .set = entry->value.set, .binding = entry->value.binding.binding };
					HeroHashTableEntry(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor)* key_entry;
					result = hero_hash_table(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor, find_or_insert)(&inspect->key_to_descriptor_map, &key, hero_system_alctor, HERO_GFX_ALLOC_TAG_SPIR_V_KEY_TO_DESCRIPTOR_MAP, &key_entry);
					if (result < 0) {
						return result;
					}

					if (result == HERO_SUCCESS_IS_NEW) {
						key_entry->value = entry->value;
					} else {
						if (key_entry->value.binding.descriptor_type != descriptor_type) {
							return HERO_ERROR(GFX_SPIR_V_BINARY_DESCRIPTOR_TYPE_MISMATCH);
						}
						if (key_entry->value.binding.descriptor_count != descriptor_count) {
							return HERO_ERROR(GFX_SPIR_V_BINARY_DESCRIPTOR_COUNT_MISMATCH);
						}
						if (key_entry->value.binding.elmt_size != descriptor_elmt_size) {
							return HERO_ERROR(GFX_SPIR_V_BINARY_DESCRIPTOR_ELMT_SIZE_MISMATCH);
						}
					}
				}
				break;
			};
			case _HERO_SPIR_V_OP_CODE_LOAD:
			case _HERO_SPIR_V_OP_CODE_ACCESS_CHAIN: {
				U32 target_type_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 target_id = endian_decode_word_fn(code, code_op_count, &idx);
				U32 base_id = endian_decode_word_fn(code, code_op_count, &idx);

				HeroHashTableEntry(U32, _HeroSpirVDescriptor)* entry;
				result = hero_hash_table(U32, _HeroSpirVDescriptor, find)(&id_to_descriptor_map, &base_id, &entry);
				if (result >= 0) {
					_HeroSpirVDescriptorKey key = { .set = entry->value.set, .binding = entry->value.binding.binding };
					HeroHashTableEntry(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor)* key_entry;
					result = hero_hash_table(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor, find)(&inspect->key_to_descriptor_map, &key, &key_entry);
					HERO_RESULT_ASSERT(result);

					key_entry->value.binding.stage_flags |= shader_stage;
				}

				break;
			};
			default:
				break;
		}

		idx = next_instruction_idx;
	}

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Backend: Vulkan Convertion
//
//
// ===========================================

static U32 _hero_vulkan_convert_to_physical_device_features_field_offsets_plus_one[] = {
	[HERO_PHYSICAL_DEVICE_FEATURE_ROBUST_BUFFER_ACCESS] = offsetof(VkPhysicalDeviceFeatures, robustBufferAccess) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_FULL_DRAW_INDEX_UINT32] = offsetof(VkPhysicalDeviceFeatures, fullDrawIndexUint32) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_IMAGE_CUBE_ARRAY] = offsetof(VkPhysicalDeviceFeatures, imageCubeArray) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_INDEPENDENT_BLEND] = offsetof(VkPhysicalDeviceFeatures, independentBlend) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_GEOMETRY_SHADER] = offsetof(VkPhysicalDeviceFeatures, geometryShader) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_TESSELLATION_SHADER] = offsetof(VkPhysicalDeviceFeatures, tessellationShader) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SAMPLE_RATE_SHADING] = offsetof(VkPhysicalDeviceFeatures, sampleRateShading) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_DUAL_SRC_BLEND] = offsetof(VkPhysicalDeviceFeatures, dualSrcBlend) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_LOGIC_OP] = offsetof(VkPhysicalDeviceFeatures, logicOp) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_MULTI_DRAW_INDIRECT] = offsetof(VkPhysicalDeviceFeatures, multiDrawIndirect) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_DRAW_INDIRECT_FIRST_INSTANCE] = offsetof(VkPhysicalDeviceFeatures, drawIndirectFirstInstance) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_DEPTH_CLAMP] = offsetof(VkPhysicalDeviceFeatures, depthClamp) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_DEPTH_BIAS_CLAMP] = offsetof(VkPhysicalDeviceFeatures, depthBiasClamp) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_FILL_MODE_NON_SOLID] = offsetof(VkPhysicalDeviceFeatures, fillModeNonSolid) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_DEPTH_BOUNDS] = offsetof(VkPhysicalDeviceFeatures, depthBounds) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_WIDE_LINES] = offsetof(VkPhysicalDeviceFeatures, wideLines) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_LARGE_POINTS] = offsetof(VkPhysicalDeviceFeatures, largePoints) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_ALPHA_TO_ONE] = offsetof(VkPhysicalDeviceFeatures, alphaToOne) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_MULTI_VIEWPORT] = offsetof(VkPhysicalDeviceFeatures, multiViewport) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SAMPLER_ANISOTROPY] = offsetof(VkPhysicalDeviceFeatures, samplerAnisotropy) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_TEXTURE_COMPRESSION_ETC2] = offsetof(VkPhysicalDeviceFeatures, textureCompressionETC2) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_TEXTURE_COMPRESSION_ASTC_LDR] = offsetof(VkPhysicalDeviceFeatures, textureCompressionASTC_LDR) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_TEXTURE_COMPRESSION_BC] = offsetof(VkPhysicalDeviceFeatures, textureCompressionBC) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_OCCLUSION_QUERY_PRECISE] = offsetof(VkPhysicalDeviceFeatures, occlusionQueryPrecise) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_PIPELINE_STATISTICS_QUERY] = offsetof(VkPhysicalDeviceFeatures, pipelineStatisticsQuery) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_VERTEX_PIPELINE_STORES_AND_ATOMICS] = offsetof(VkPhysicalDeviceFeatures, vertexPipelineStoresAndAtomics) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_FRAGMENT_STORES_AND_ATOMICS] = offsetof(VkPhysicalDeviceFeatures, fragmentStoresAndAtomics) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE] = offsetof(VkPhysicalDeviceFeatures, shaderTessellationAndGeometryPointSize) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_IMAGE_GATHER_EXTENDED] = offsetof(VkPhysicalDeviceFeatures, shaderImageGatherExtended) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_EXTENDED_FORMATS] = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageExtendedFormats) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_MULTISAMPLE] = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageMultisample) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_READ_WITHOUTFORMAT] = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageReadWithoutFormat) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT] = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageWriteWithoutFormat) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING] = offsetof(VkPhysicalDeviceFeatures, shaderUniformBufferArrayDynamicIndexing) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING] = offsetof(VkPhysicalDeviceFeatures, shaderSampledImageArrayDynamicIndexing) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING] = offsetof(VkPhysicalDeviceFeatures, shaderStorageBufferArrayDynamicIndexing) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING] = offsetof(VkPhysicalDeviceFeatures, shaderStorageImageArrayDynamicIndexing) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_CLIP_DISTANCE] = offsetof(VkPhysicalDeviceFeatures, shaderClipDistance) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_CULL_DISTANCE] = offsetof(VkPhysicalDeviceFeatures, shaderCullDistance) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_FLOAT64] = offsetof(VkPhysicalDeviceFeatures, shaderFloat64) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_INT64] = offsetof(VkPhysicalDeviceFeatures, shaderInt64) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_INT16] = offsetof(VkPhysicalDeviceFeatures, shaderInt16) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_RESOURCE_RESIDENCY] = offsetof(VkPhysicalDeviceFeatures, shaderResourceResidency) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SHADER_RESOURCE_MIN_LOD] = offsetof(VkPhysicalDeviceFeatures, shaderResourceMinLod) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_BINDING] = offsetof(VkPhysicalDeviceFeatures, sparseBinding) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_BUFFER] = offsetof(VkPhysicalDeviceFeatures, sparseResidencyBuffer) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE2_D] = offsetof(VkPhysicalDeviceFeatures, sparseResidencyImage2D) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE3_D] = offsetof(VkPhysicalDeviceFeatures, sparseResidencyImage3D) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY2_SAMPLES] = offsetof(VkPhysicalDeviceFeatures, sparseResidency2Samples) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY4_SAMPLES] = offsetof(VkPhysicalDeviceFeatures, sparseResidency4Samples) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY8_SAMPLES] = offsetof(VkPhysicalDeviceFeatures, sparseResidency8Samples) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY16_SAMPLES] = offsetof(VkPhysicalDeviceFeatures, sparseResidency16Samples) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_SPARSE_RESIDENCY_ALIASED] = offsetof(VkPhysicalDeviceFeatures, sparseResidencyAliased) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_VARIABLE_MULTISAMPLE_RATE] = offsetof(VkPhysicalDeviceFeatures, variableMultisampleRate) + 1,
	[HERO_PHYSICAL_DEVICE_FEATURE_INHERITED_QUERIES] = offsetof(VkPhysicalDeviceFeatures, inheritedQueries) + 1,
};

static HeroPhysicalDeviceType _hero_vulkan_convert_from_physical_device_type[] = {
	[VK_PHYSICAL_DEVICE_TYPE_OTHER] = HERO_PHYSICAL_DEVICE_TYPE_OTHER,
	[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = HERO_PHYSICAL_DEVICE_TYPE_GPU_INTEGRATED,
	[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU] = HERO_PHYSICAL_DEVICE_TYPE_GPU_DISCRETE,
	[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU] = HERO_PHYSICAL_DEVICE_TYPE_GPU_VIRTUAL,
	[VK_PHYSICAL_DEVICE_TYPE_CPU] = HERO_PHYSICAL_DEVICE_TYPE_CPU,
};

static VkFormat _hero_vulkan_convert_to_vertex_input_format[HERO_VERTEX_ELMT_TYPE_COUNT][HERO_VERTEX_VECTOR_TYPE_COUNT] = {
	[HERO_VERTEX_ELMT_TYPE_U8] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R8_UINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R8G8_UINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R8G8B8_UINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R8G8B8A8_UINT,
	},
	[HERO_VERTEX_ELMT_TYPE_S8] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R8_SINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R8G8_SINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R8G8B8_SINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R8G8B8A8_SINT,
	},
	[HERO_VERTEX_ELMT_TYPE_U8_F32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R8_USCALED,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R8G8_USCALED,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R8G8B8_USCALED,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R8G8B8A8_USCALED,
	},
	[HERO_VERTEX_ELMT_TYPE_S8_F32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R8_SSCALED,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R8G8_SSCALED,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R8G8B8_SSCALED,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R8G8B8A8_SSCALED,
	},
	[HERO_VERTEX_ELMT_TYPE_U8_F32_NORMALIZE] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R8_UNORM,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R8G8_UNORM,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R8G8B8_UNORM,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R8G8B8A8_UNORM,
	},
	[HERO_VERTEX_ELMT_TYPE_S8_F32_NORMALIZE] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R8_SNORM,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R8G8_SNORM,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R8G8B8_SNORM,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R8G8B8A8_SNORM,
	},
	[HERO_VERTEX_ELMT_TYPE_U16] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_UINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_UINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_UINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_UINT,
	},
	[HERO_VERTEX_ELMT_TYPE_S16] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_SINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_SINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_SINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_SINT,
	},
	[HERO_VERTEX_ELMT_TYPE_U16_F32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_USCALED,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_USCALED,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_USCALED,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_USCALED,
	},
	[HERO_VERTEX_ELMT_TYPE_S16_F32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_SSCALED,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_SSCALED,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_SSCALED,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_SSCALED,
	},
	[HERO_VERTEX_ELMT_TYPE_U16_F32_NORMALIZE] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_UNORM,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_UNORM,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_UNORM,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_UNORM,
	},
	[HERO_VERTEX_ELMT_TYPE_S16_F32_NORMALIZE] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_SNORM,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_SNORM,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_SNORM,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_SNORM,
	},
	[HERO_VERTEX_ELMT_TYPE_U32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R32_UINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R32G32_UINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R32G32B32_UINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R32G32B32A32_UINT,
	},
	[HERO_VERTEX_ELMT_TYPE_S32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R32_SINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R32G32_SINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R32G32B32_SINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R32G32B32A32_SINT,
	},
	[HERO_VERTEX_ELMT_TYPE_U64] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R64_UINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R64G64_UINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R64G64B64_UINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R64G64B64A64_UINT,
	},
	[HERO_VERTEX_ELMT_TYPE_S64] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R64_SINT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R64G64_SINT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R64G64B64_SINT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R64G64B64A64_SINT,
	},
	[HERO_VERTEX_ELMT_TYPE_F16] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R16_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R16G16_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R16G16B16_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R16G16B16A16_SFLOAT,
	},
	[HERO_VERTEX_ELMT_TYPE_F32] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R32_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R32G32_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R32G32B32_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R32G32B32A32_SFLOAT,
	},
	[HERO_VERTEX_ELMT_TYPE_F64] = {
		[HERO_VERTEX_VECTOR_TYPE_1] = VK_FORMAT_R64_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_2] = VK_FORMAT_R64G64_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_3] = VK_FORMAT_R64G64B64_SFLOAT,
		[HERO_VERTEX_VECTOR_TYPE_4] = VK_FORMAT_R64G64B64A64_SFLOAT,
	},
};

static VkIndexType _hero_vulkan_convert_to_index_type[] = {
	[HERO_INDEX_TYPE_U8] = VK_INDEX_TYPE_UINT8_EXT,
	[HERO_INDEX_TYPE_U16] = VK_INDEX_TYPE_UINT16,
	[HERO_INDEX_TYPE_U32] = VK_INDEX_TYPE_UINT32,
};

static VkImageType _hero_vulkan_convert_to_image_type[] = {
	[HERO_IMAGE_TYPE_1D] = VK_IMAGE_TYPE_1D,
	[HERO_IMAGE_TYPE_1D_ARRAY] = VK_IMAGE_TYPE_1D,
	[HERO_IMAGE_TYPE_2D] = VK_IMAGE_TYPE_2D,
	[HERO_IMAGE_TYPE_2D_ARRAY] = VK_IMAGE_TYPE_1D,
	[HERO_IMAGE_TYPE_3D] = VK_IMAGE_TYPE_3D,
	[HERO_IMAGE_TYPE_CUBE] = VK_IMAGE_TYPE_2D,
	[HERO_IMAGE_TYPE_CUBE_ARRAY] = VK_IMAGE_TYPE_2D,
};

static VkImageViewType _hero_vulkan_convert_to_image_view_type[] = {
	[HERO_IMAGE_TYPE_1D] = VK_IMAGE_VIEW_TYPE_1D,
	[HERO_IMAGE_TYPE_1D_ARRAY] = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
	[HERO_IMAGE_TYPE_2D] = VK_IMAGE_VIEW_TYPE_2D,
	[HERO_IMAGE_TYPE_2D_ARRAY] = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
	[HERO_IMAGE_TYPE_3D] = VK_IMAGE_VIEW_TYPE_3D,
	[HERO_IMAGE_TYPE_CUBE] = VK_IMAGE_VIEW_TYPE_CUBE,
	[HERO_IMAGE_TYPE_CUBE_ARRAY] = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
};

static VkFormat _hero_vulkan_convert_to_format[HERO_IMAGE_FORMAT_COUNT] = {
	[HERO_IMAGE_FORMAT_R8_UNORM] = VK_FORMAT_R8_UNORM,
	[HERO_IMAGE_FORMAT_R8G8_UNORM] = VK_FORMAT_R8G8_UNORM,
	[HERO_IMAGE_FORMAT_R8G8B8_UNORM] = VK_FORMAT_R8G8B8_UNORM,
	[HERO_IMAGE_FORMAT_R8G8B8A8_UNORM] = VK_FORMAT_R8G8B8A8_UNORM,
	[HERO_IMAGE_FORMAT_B8G8R8_UNORM] = VK_FORMAT_B8G8R8_UNORM,
	[HERO_IMAGE_FORMAT_B8G8R8A8_UNORM] = VK_FORMAT_B8G8R8A8_UNORM,
	[HERO_IMAGE_FORMAT_R32_UINT] = VK_FORMAT_R32_UINT,
	[HERO_IMAGE_FORMAT_R32_SINT] = VK_FORMAT_R32_SINT,
	[HERO_IMAGE_FORMAT_R32_SFLOAT] = VK_FORMAT_R32_SFLOAT,
	[HERO_IMAGE_FORMAT_D16] = VK_FORMAT_D16_UNORM,
	[HERO_IMAGE_FORMAT_D32] = VK_FORMAT_D32_SFLOAT,
	[HERO_IMAGE_FORMAT_S8] = VK_FORMAT_S8_UINT,
	[HERO_IMAGE_FORMAT_D16_S8] = VK_FORMAT_D16_UNORM_S8_UINT,
	[HERO_IMAGE_FORMAT_D24_S8] = VK_FORMAT_D24_UNORM_S8_UINT,
	[HERO_IMAGE_FORMAT_D32_S8] = VK_FORMAT_D32_SFLOAT_S8_UINT,
};

static HeroImageFormat _hero_vulkan_convert_from_format(VkFormat vk) {
	HeroImageFormat fmt;
	switch (vk) {
		case VK_FORMAT_R8_UNORM: fmt = HERO_IMAGE_FORMAT_R8_UNORM; break;
		case VK_FORMAT_R8G8_UNORM: fmt = HERO_IMAGE_FORMAT_R8G8_UNORM; break;
		case VK_FORMAT_R8G8B8_UNORM: fmt = HERO_IMAGE_FORMAT_R8G8B8_UNORM; break;
		case VK_FORMAT_R8G8B8A8_UNORM: fmt = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM; break;
		case VK_FORMAT_B8G8R8_UNORM: fmt = HERO_IMAGE_FORMAT_B8G8R8_UNORM; break;
		case VK_FORMAT_B8G8R8A8_UNORM: fmt = HERO_IMAGE_FORMAT_B8G8R8A8_UNORM; break;
		case VK_FORMAT_R32_UINT: fmt = HERO_IMAGE_FORMAT_R32_UINT; break;
		case VK_FORMAT_R32_SINT: fmt = HERO_IMAGE_FORMAT_R32_SINT; break;
		case VK_FORMAT_R32_SFLOAT: fmt = HERO_IMAGE_FORMAT_R32_SFLOAT; break;
		case VK_FORMAT_D16_UNORM: fmt = HERO_IMAGE_FORMAT_D16; break;
		case VK_FORMAT_D32_SFLOAT: fmt = HERO_IMAGE_FORMAT_D32; break;
		case VK_FORMAT_S8_UINT: fmt = HERO_IMAGE_FORMAT_S8; break;
		case VK_FORMAT_D16_UNORM_S8_UINT: fmt = HERO_IMAGE_FORMAT_D16_S8; break;
		case VK_FORMAT_D24_UNORM_S8_UINT: fmt = HERO_IMAGE_FORMAT_D24_S8; break;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: fmt = HERO_IMAGE_FORMAT_D32_S8; break;
		default: HERO_ABORT("unhandled vulkan format %u", vk); break;
	}

	return HERO_SUCCESS;
}

static VkFilter _hero_vulkan_convert_to_filter[] = {
	[HERO_FILTER_NEAREST] = VK_FILTER_NEAREST,
	[HERO_FILTER_LINEAR] = VK_FILTER_LINEAR,
	[HERO_FILTER_CUBIC_IMG] = VK_FILTER_CUBIC_IMG,
};

static VkSamplerMipmapMode _hero_vulkan_convert_to_sampler_mipmap_mode[] = {
	[HERO_SAMPLER_MIPMAP_MODE_NEAREST] = VK_SAMPLER_MIPMAP_MODE_NEAREST,
	[HERO_SAMPLER_MIPMAP_MODE_LINEAR] = VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

static VkSamplerAddressMode _hero_vulkan_convert_to_sampler_address_mode[] = {
	[HERO_SAMPLER_ADDRESS_MODE_REPEAT] = VK_SAMPLER_ADDRESS_MODE_REPEAT,
	[HERO_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT] = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	[HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE] = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
	[HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER] = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	[HERO_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE] = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

static VkCompareOp _hero_vulkan_convert_to_compare_op[] = {
	[HERO_COMPARE_OP_NEVER] = VK_COMPARE_OP_NEVER,
	[HERO_COMPARE_OP_LESS] = VK_COMPARE_OP_LESS,
	[HERO_COMPARE_OP_EQUAL] = VK_COMPARE_OP_EQUAL,
	[HERO_COMPARE_OP_LESS_OR_EQUAL] = VK_COMPARE_OP_LESS_OR_EQUAL,
	[HERO_COMPARE_OP_GREATER] = VK_COMPARE_OP_GREATER,
	[HERO_COMPARE_OP_NOT_EQUAL] = VK_COMPARE_OP_NOT_EQUAL,
	[HERO_COMPARE_OP_GREATER_OR_EQUAL] = VK_COMPARE_OP_GREATER_OR_EQUAL,
	[HERO_COMPARE_OP_ALWAYS] = VK_COMPARE_OP_ALWAYS,
};

static VkBorderColor _hero_vulkan_convert_to_border_color[] = {
	[HERO_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK] = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
	[HERO_BORDER_COLOR_INT_TRANSPARENT_BLACK] = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
	[HERO_BORDER_COLOR_FLOAT_OPAQUE_BLACK] = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
	[HERO_BORDER_COLOR_INT_OPAQUE_BLACK] = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
	[HERO_BORDER_COLOR_FLOAT_OPAQUE_WHITE] = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
	[HERO_BORDER_COLOR_INT_OPAQUE_WHITE] = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
};

static VkAttachmentLoadOp _hero_vulkan_convert_to_attachment_load_op[] = {
	[HERO_ATTACHMENT_LOAD_OP_PRESERVE] = VK_ATTACHMENT_LOAD_OP_LOAD,
	[HERO_ATTACHMENT_LOAD_OP_CLEAR] = VK_ATTACHMENT_LOAD_OP_CLEAR,
	[HERO_ATTACHMENT_LOAD_OP_UNINITIALIZED] = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
};

static VkAttachmentStoreOp _hero_vulkan_convert_to_attachment_store_op[] = {
	[HERO_ATTACHMENT_STORE_OP_PRESERVE] = VK_ATTACHMENT_STORE_OP_STORE,
	[HERO_ATTACHMENT_STORE_OP_DISCARD] = VK_ATTACHMENT_STORE_OP_DONT_CARE,
};

static VkDescriptorType _hero_vulkan_convert_to_descriptor_type[HERO_DESCRIPTOR_TYPE_COUNT] = {
	[HERO_DESCRIPTOR_TYPE_SAMPLER] = VK_DESCRIPTOR_TYPE_SAMPLER,
	[HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	[HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	[HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	[HERO_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER] = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
	[HERO_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER] = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
	[HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	[HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	[HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	[HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	[HERO_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
};

void _hero_vulkan_convert_to_shader_stage_flags(HeroShaderStageFlags hero, VkShaderStageFlags* vk_out) {
	VkShaderStageFlags vk = 0;

	if (hero & HERO_SHADER_STAGE_FLAGS_VERTEX) vk |= VK_SHADER_STAGE_VERTEX_BIT;
	if (hero & HERO_SHADER_STAGE_FLAGS_TESSELLATION_CONTROL) vk |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	if (hero & HERO_SHADER_STAGE_FLAGS_TESSELLATION_EVALUATION) vk |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	if (hero & HERO_SHADER_STAGE_FLAGS_GEOMETRY) vk |= VK_SHADER_STAGE_GEOMETRY_BIT;
	if (hero & HERO_SHADER_STAGE_FLAGS_FRAGMENT) vk |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (hero & HERO_SHADER_STAGE_FLAGS_COMPUTE) vk |= VK_SHADER_STAGE_COMPUTE_BIT;
	if (hero & HERO_SHADER_STAGE_FLAGS_TASK) vk |= VK_SHADER_STAGE_TASK_BIT_NV;
	if (hero & HERO_SHADER_STAGE_FLAGS_MESH) vk |= VK_SHADER_STAGE_MESH_BIT_NV;

	*vk_out = vk;
}

void _hero_vulkan_convert_to_cull_mode(HeroCullModeFlags hero, VkCullModeFlags* vk_out) {
	VkCullModeFlags vk = 0;

	if (hero & HERO_CULL_MODE_FLAGS_FRONT) vk |= VK_CULL_MODE_FRONT_BIT;
	if (hero & HERO_CULL_MODE_FLAGS_BACK) vk |= VK_CULL_MODE_BACK_BIT;

	*vk_out = vk;
}

void _hero_vulkan_convert_to_viewport(HeroViewport* hero, VkViewport* vk_out) {
	*vk_out = (VkViewport) {
		.x = hero->x,
		.y = hero->y,
		.width = hero->width,
		.height = hero->height,
		.minDepth = hero->min_depth,
		.maxDepth = hero->max_depth,
	};
}

void _hero_vulkan_convert_to_rect2d(HeroUAabb* hero, VkRect2D* vk_out) {
	*vk_out = (VkRect2D) {
		.offset = { .x = hero->x, .y = hero->y, },
		.extent = { .width = hero->ex - hero->x, .height = hero->ey - hero->y, },
	};
}

VkPrimitiveTopology _hero_vulkan_convert_to_topology[] = {
	[HERO_PRIMITIVE_TOPOLOGY_POINT_LIST] = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	[HERO_PRIMITIVE_TOPOLOGY_LINE_LIST] = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	[HERO_PRIMITIVE_TOPOLOGY_LINE_STRIP] = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	[HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	[HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	[HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
	[HERO_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY] = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	[HERO_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY] = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	[HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	[HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY] = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	[HERO_PRIMITIVE_TOPOLOGY_PATCH_LIST] = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};

VkStencilOp _hero_vulkan_convert_to_stencil_op[] = {
	[HERO_STENCIL_OP_KEEP] = VK_STENCIL_OP_KEEP,
	[HERO_STENCIL_OP_ZERO] = VK_STENCIL_OP_ZERO,
	[HERO_STENCIL_OP_REPLACE] = VK_STENCIL_OP_REPLACE,
	[HERO_STENCIL_OP_INCREMENT_AND_CLAMP] = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	[HERO_STENCIL_OP_DECREMENT_AND_CLAMP] = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	[HERO_STENCIL_OP_INVERT] = VK_STENCIL_OP_INVERT,
	[HERO_STENCIL_OP_INCREMENT_AND_WRAP] = VK_STENCIL_OP_INCREMENT_AND_WRAP,
	[HERO_STENCIL_OP_DECREMENT_AND_WRAP] = VK_STENCIL_OP_DECREMENT_AND_WRAP,
};

VkLogicOp _hero_vulkan_convert_to_logic_op[] = {
	[HERO_LOGIC_OP_CLEAR] = VK_LOGIC_OP_CLEAR,
	[HERO_LOGIC_OP_AND] = VK_LOGIC_OP_AND,
	[HERO_LOGIC_OP_AND_REVERSE] = VK_LOGIC_OP_AND_REVERSE,
	[HERO_LOGIC_OP_COPY] = VK_LOGIC_OP_COPY,
	[HERO_LOGIC_OP_AND_INVERTED] = VK_LOGIC_OP_AND_INVERTED,
	[HERO_LOGIC_OP_NO_OP] = VK_LOGIC_OP_NO_OP,
	[HERO_LOGIC_OP_XOR] = VK_LOGIC_OP_XOR,
	[HERO_LOGIC_OP_OR] = VK_LOGIC_OP_OR,
	[HERO_LOGIC_OP_NOR] = VK_LOGIC_OP_NOR,
	[HERO_LOGIC_OP_EQUIVALENT] = VK_LOGIC_OP_EQUIVALENT,
	[HERO_LOGIC_OP_INVERT] = VK_LOGIC_OP_INVERT,
	[HERO_LOGIC_OP_OR_REVERSE] = VK_LOGIC_OP_OR_REVERSE,
	[HERO_LOGIC_OP_COPY_INVERTED] = VK_LOGIC_OP_COPY_INVERTED,
	[HERO_LOGIC_OP_OR_INVERTED] = VK_LOGIC_OP_OR_INVERTED,
	[HERO_LOGIC_OP_NAND] = VK_LOGIC_OP_NAND,
	[HERO_LOGIC_OP_SET] = VK_LOGIC_OP_SET,
};

VkBlendFactor _hero_vulkan_convert_to_blend_factor[] = {
	[HERO_BLEND_FACTOR_ZERO] = VK_BLEND_FACTOR_ZERO,
	[HERO_BLEND_FACTOR_ONE] = VK_BLEND_FACTOR_ONE,
	[HERO_BLEND_FACTOR_SRC_COLOR] = VK_BLEND_FACTOR_SRC_COLOR,
	[HERO_BLEND_FACTOR_ONE_MINUS_SRC_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	[HERO_BLEND_FACTOR_DST_COLOR] = VK_BLEND_FACTOR_DST_COLOR,
	[HERO_BLEND_FACTOR_ONE_MINUS_DST_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	[HERO_BLEND_FACTOR_SRC_ALPHA] = VK_BLEND_FACTOR_SRC_ALPHA,
	[HERO_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	[HERO_BLEND_FACTOR_DST_ALPHA] = VK_BLEND_FACTOR_DST_ALPHA,
	[HERO_BLEND_FACTOR_ONE_MINUS_DST_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	[HERO_BLEND_FACTOR_CONSTANT_COLOR] = VK_BLEND_FACTOR_CONSTANT_COLOR,
	[HERO_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	[HERO_BLEND_FACTOR_CONSTANT_ALPHA] = VK_BLEND_FACTOR_CONSTANT_ALPHA,
	[HERO_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
	[HERO_BLEND_FACTOR_SRC_ALPHA_SATURATE] = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	[HERO_BLEND_FACTOR_SRC1_COLOR] = VK_BLEND_FACTOR_SRC1_COLOR,
	[HERO_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR] = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	[HERO_BLEND_FACTOR_SRC1_ALPHA] = VK_BLEND_FACTOR_SRC1_ALPHA,
	[HERO_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA] = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
};

VkBlendOp _hero_vulkan_convert_to_blend_op[] = {
	[HERO_BLEND_OP_ADD] = VK_BLEND_OP_ADD,
	[HERO_BLEND_OP_SUBTRACT] = VK_BLEND_OP_SUBTRACT,
	[HERO_BLEND_OP_REVERSE_SUBTRACT] = VK_BLEND_OP_REVERSE_SUBTRACT,
	[HERO_BLEND_OP_MIN] = VK_BLEND_OP_MIN,
	[HERO_BLEND_OP_MAX] = VK_BLEND_OP_MAX,
};

VkFrontFace _hero_vulkan_convert_to_front_face[] = {
	[HERO_FRONT_FACE_COUNTER_CLOCKWISE] = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	[HERO_FRONT_FACE_CLOCKWISE] = VK_FRONT_FACE_CLOCKWISE,
};

VkPolygonMode _hero_vulkan_convert_to_polygon_mode[] = {
	[HERO_POLYGON_MODE_FILL] = VK_POLYGON_MODE_FILL,
	[HERO_POLYGON_MODE_LINE] = VK_POLYGON_MODE_LINE,
	[HERO_POLYGON_MODE_POINT] = VK_POLYGON_MODE_POINT,
};

void _hero_vulkan_convert_to_stencil_op_state(HeroStencilOpState* hero, VkStencilOpState* vk_out) {
	*vk_out = (VkStencilOpState) {
		.failOp = _hero_vulkan_convert_to_stencil_op[hero->fail_op],
		.passOp = _hero_vulkan_convert_to_stencil_op[hero->pass_op],
		.depthFailOp = _hero_vulkan_convert_to_stencil_op[hero->depth_fail_op],
		.compareOp = _hero_vulkan_convert_to_compare_op[hero->depth_fail_op],
		.compareMask = hero->compare_mask,
		.writeMask = hero->write_mask,
		.reference = hero->reference,
	};

}

void _hero_vulkan_convert_to_pipeline_color_blend_attachement(HeroRenderStateBlendAttachment* hero, VkPipelineColorBlendAttachmentState* vk_out) {
	VkColorComponentFlags vk_write_mask = 0;
	if (hero->color_write_mask & HERO_COLOR_COMPONENT_FLAGS_R) vk_write_mask |= VK_COLOR_COMPONENT_R_BIT;
	if (hero->color_write_mask & HERO_COLOR_COMPONENT_FLAGS_G) vk_write_mask |= VK_COLOR_COMPONENT_G_BIT;
	if (hero->color_write_mask & HERO_COLOR_COMPONENT_FLAGS_B) vk_write_mask |= VK_COLOR_COMPONENT_B_BIT;
	if (hero->color_write_mask & HERO_COLOR_COMPONENT_FLAGS_A) vk_write_mask |= VK_COLOR_COMPONENT_A_BIT;

	*vk_out = (VkPipelineColorBlendAttachmentState) {
		.blendEnable = hero->blend_enable,
		.srcColorBlendFactor = _hero_vulkan_convert_to_blend_factor[hero->src_color_blend_factor],
		.dstColorBlendFactor = _hero_vulkan_convert_to_blend_factor[hero->dst_color_blend_factor],
		.colorBlendOp = _hero_vulkan_convert_to_blend_op[hero->color_blend_op],
		.srcAlphaBlendFactor = _hero_vulkan_convert_to_blend_factor[hero->src_alpha_blend_factor],
		.dstAlphaBlendFactor = _hero_vulkan_convert_to_blend_factor[hero->dst_alpha_blend_factor],
		.alphaBlendOp = _hero_vulkan_convert_to_blend_op[hero->alpha_blend_op],
		.colorWriteMask = vk_write_mask,
	};
}

static HeroResult _hero_vulkan_convert_from_result(VkResult result) {
	switch (result) {
		case VK_SUCCESS: return HERO_SUCCESS;
		case VK_ERROR_OUT_OF_HOST_MEMORY: return HERO_ERROR(ALLOCATION_FAILURE);
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: return HERO_ERROR(GFX_OUT_OF_MEMORY_DEVICE);
		default:
			HERO_ABORT("unhandled VkResult: %d", result);
	}
}

// ===========================================
//
//
// Backend: Vulkan Allocator
//
//
// ===========================================

typedef uint8_t _HeroVulkanAllocType;
enum _HeroVulkanAllocType {
	_HERO_VULKAN_ALLOC_TYPE_BUFFER,
	_HERO_VULKAN_ALLOC_TYPE_IMAGE,
	_HERO_VULKAN_ALLOC_TYPE_IMAGE_LINEAR,
	_HERO_VULKAN_ALLOC_TYPE_IMAGE_OPTIMAL,
	_HERO_VULKAN_ALLOC_TYPE_COUNT,
};

typedef struct _HeroVulkanAllocSetup _HeroVulkanAllocSetup;
struct _HeroVulkanAllocSetup {
	U64 size;
	U64 align;
	U32 memory_type_bits;
	HeroMemoryLocation memory_location;
	_HeroVulkanAllocType type;
};

HeroResult _hero_vulkan_device_memory_alloc(HeroLogicalDeviceVulkan* ldev_vulkan, _HeroVulkanAllocSetup* setup, VkDeviceMemory* out) {
	VkMemoryAllocateInfo vk_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = setup->size,
		.memoryTypeIndex = 0,
	};

	VkResult vk_result = ldev_vulkan->vkAllocateMemory(ldev_vulkan->handle, &vk_alloc_info, HERO_VULKAN_TODO_ALLOCATOR, out);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_device_memory_dealloc(HeroLogicalDeviceVulkan* ldev_vulkan, VkDeviceMemory device_memory) {
	ldev_vulkan->vkFreeMemory(ldev_vulkan->handle, device_memory, HERO_VULKAN_TODO_ALLOCATOR);

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Backend: Vulkan Functions
//
//
// ===========================================

HeroResult _hero_vulkan_physical_device_init(HeroPhysicalDeviceVulkan* physical_device_vulkan, VkPhysicalDevice vk_physical_device) {
	HeroResult result;
	VkResult vk_result;

	//
	// find the queues and the support this physical device has for them
	//
	HeroQueueSupportFlags queue_support_flags = 0;
	U32 queue_family_idx_uber = 0;
	U32 queue_family_idx_present = 0;
	U32 queue_family_idx_async_compute = 0;
	U32 queue_family_idx_async_transfer = 0;
	{
		U32 queue_families_count;
		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_families_count, NULL);

		VkQueueFamilyProperties* queue_family_properties = hero_alloc_array(VkQueueFamilyProperties, hero_system_alctor, HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES_VULKAN_QUEUE_FAMILY_PROPERTIES, queue_families_count);
		if (!queue_family_properties) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_families_count, queue_family_properties);

		//
		// the vulkan spec states the following:
		// - if the device has a queue with graphics support then one must exist with both graphics and compute
		// - a queue with graphics or compute has transfer support
		//
		for (U32 queue_family_idx = 0; queue_family_idx < queue_families_count; queue_family_idx += 1) {
			VkQueueFlags flags = queue_family_properties[queue_family_idx].queueFlags;

			if (flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
				queue_family_idx_uber = queue_family_idx;
				queue_support_flags |= HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS | HERO_QUEUE_SUPPORT_FLAGS_COMPUTE;

				//
				// present queue is usually on the uber queue
				if (_hero_vulkan_queue_has_presentation_support(vk_physical_device, queue_family_idx)) {
					queue_family_idx_present = queue_family_idx;
					queue_support_flags |= HERO_QUEUE_SUPPORT_FLAGS_PRESENT;
				}

				break;
			}
		}

		//
		// if we do not have graphics support, then make the uber queue the first compute queue
		if (!(queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS)) {
			for (U32 queue_family_idx = 0; queue_family_idx < queue_families_count; queue_family_idx += 1) {
				VkQueueFlags flags = queue_family_properties[queue_family_idx].queueFlags;

				if (flags & VK_QUEUE_COMPUTE_BIT) {
					queue_family_idx_uber = queue_family_idx;
					queue_support_flags |= HERO_QUEUE_SUPPORT_FLAGS_COMPUTE;
					break;
				}
			}
		}

		//
		// now look for the async compute & async transfer if they exist.
		// also get the present queue if we have graphics support and have not found a present queue yet.
		for (U32 queue_family_idx = 0; queue_family_idx < queue_families_count; queue_family_idx += 1) {
			VkQueueFlags flags = queue_family_properties[queue_family_idx].queueFlags;

			//
			// skip the uber queue to avoid the async queues using the same one
			if (queue_family_idx == queue_family_idx_uber) {
				continue;
			}

			//
			// present
			if ((queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS) && !(queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_PRESENT)) {
				if (_hero_vulkan_queue_has_presentation_support(vk_physical_device, queue_family_idx)) {
					queue_family_idx_present = queue_family_idx;
					queue_support_flags |= HERO_QUEUE_SUPPORT_FLAGS_PRESENT;
				}
			}

			//
			// async compute
			if (!(queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE)) {
				if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)) {
					queue_family_idx_async_transfer = queue_family_idx;
					queue_support_flags |= HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE;
				}
			}

			//
			// async transfer
			if (!(queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER)) {
				if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) {
					queue_family_idx_async_transfer = queue_family_idx;
					queue_support_flags |= HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER;
				}
			}
		}
	}

	//
	// query all the features this physical device has
	HeroPhysicalDeviceFeatureFlags feature_flags = {0};
	{
		VkPhysicalDeviceFeatures vk_features;
		vkGetPhysicalDeviceFeatures(vk_physical_device, &vk_features);

		//
		// convert from the vulkan structure to our flags bitset
		for (HeroPhysicalDeviceFeature feature = 0; feature < HERO_PHYSICAL_DEVICE_FEATURE_COUNT; feature += 1) {
			U32 offset = _hero_vulkan_convert_to_physical_device_features_field_offsets_plus_one[feature];
			if (offset == 0) {
				continue;
			}
			offset -= 1;

			if (*(VkBool32*)HERO_PTR_ADD(&vk_features, offset)) {
				HERO_PHYSICAL_DEVICE_FEATURE_FLAGS_INSERT(feature_flags, feature);
			}
		}
	}

	VkPhysicalDeviceProperties vk_properties;
	vkGetPhysicalDeviceProperties(vk_physical_device, &vk_properties);

	physical_device_vulkan->handle = vk_physical_device;
	physical_device_vulkan->api_version = vk_properties.apiVersion;
	physical_device_vulkan->queue_family_idx_uber = queue_family_idx_uber;
	physical_device_vulkan->queue_family_idx_present = queue_family_idx_present;
	physical_device_vulkan->queue_family_idx_async_compute = queue_family_idx_async_compute;
	physical_device_vulkan->queue_family_idx_async_transfer = queue_family_idx_async_transfer;

	HeroPhysicalDevice* physical_device = &physical_device_vulkan->public_;
	physical_device->device_name = vk_properties.deviceName;
	physical_device->feature_flags = feature_flags;
	physical_device->driver_version = vk_properties.driverVersion;
	physical_device->vendor_id = vk_properties.vendorID;
	physical_device->device_id = vk_properties.deviceID;
	physical_device->device_type = _hero_vulkan_convert_from_physical_device_type[vk_properties.deviceType];
	physical_device->queue_support_flags = queue_support_flags;

	return HERO_SUCCESS;
}

void _hero_vulkan_queue_family_indices_get(HeroLogicalDeviceVulkan* ldev_vulkan, HeroQueueSupportFlags queue_support_flags, U32* queue_family_indices_out, U16* queue_family_indices_count_out) {
	U32 count = 0;

	if (queue_support_flags & (HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS | HERO_QUEUE_SUPPORT_FLAGS_COMPUTE)) {
		queue_family_indices_out[count] = ldev_vulkan->queue_family_idx_uber;
		count += 1;
	}

	if (queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE) {
		queue_family_indices_out[count] = ldev_vulkan->queue_family_idx_async_compute;
		count += 1;
	}

	if (queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER) {
		queue_family_indices_out[count] = ldev_vulkan->queue_family_idx_async_transfer;
		count += 1;
	}

	*queue_family_indices_count_out = count;
}

HeroResult _hero_vulkan_init(HeroGfxSysSetup* setup) {
	VkResult vk_result;
	//
	// TODO: have this path configurable by the user some how
	const char* vulkan_dll_path = "libvulkan.so.1";

	HeroDLL* vulkan_dll = &hero_gfx_sys_vulkan.dll;
	HeroResult result = hero_dll_open(vulkan_dll, vulkan_dll_path);
	if (result < 0) return result;

	//
	// initialize all of the function pointers to that need to be called before creating a vulkan instance
	//
	{
#define HERO_VULKAN_FN(NAME) \
		result = hero_dll_resolve_symbol(vulkan_dll, #NAME, (void**)&NAME); \
		HERO_DEBUG_ASSERT(result >= 0, "could not find the '" #NAME "' symbol in '%s'", vulkan_dll_path); \
		/* end */
#define HERO_VULKAN_INSTANCE_FN(NAME) \
		/* end */
#define HERO_VULKAN_DEVICE_FN(NAME) \
		/* end */

		HERO_VULKAN_FN_LIST
#undef HERO_VULKAN_FN
#undef HERO_VULKAN_INSTANCE_FN
#undef HERO_VULKAN_DEVICE_FN
	}

	//
	// create the vulkan instance
	//
	VkInstance vk_instance;
	{
#if HERO_GFX_VULKAN_DEBUG
		const char* layers[] = {
			"VK_LAYER_LUNARG_api_dump",
			"VK_LAYER_KHRONOS_validation",
		};
		U32 layers_count = HERO_ARRAY_COUNT(layers);
#else
		const char** layers = NULL;
		U32 layers_count = 0;
#endif

		const char* display_manager_extension_name;
		switch (setup->display_manager_type) {
#ifdef _WIN32
			case HERO_GFX_DISPLAY_MANAGER_WINDOWS:
				display_manager_extension_name = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
				break;
#endif // _WIN32
#ifdef __APPLE__
			case HERO_GFX_DISPLAY_MANAGER_MACOS:
				display_manager_extension_name = VK_MVK_MACOS_SURFACE_EXTENSION_NAME;
				break;
			case HERO_GFX_DISPLAY_MANAGER_IOS:
				display_manager_extension_name = VK_MVK_IOS_SURFACE_EXTENSION_NAME;
				break;
#endif //  __APPLE__
#ifdef __unix__
#ifdef HERO_X11_ENABLE
			case HERO_GFX_DISPLAY_MANAGER_XLIB:
				display_manager_extension_name = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
				break;
#endif // HERO_XLIB_ENABLE
#ifdef HERO_WAYLAND_ENABLE
			case HERO_GFX_DISPLAY_MANAGER_WAYLAND:
				display_manager_extension_name = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
				break;
#endif // HERO_WAYLAND_ENABLE
#endif // __unix__
#ifdef __ANDROID__
			case HERO_GFX_DISPLAY_MANAGER_ANDROID:
				display_manager_extension_name =  VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
				break;
#endif // __ANDROID__
			default: HERO_ABORT("unhandled display_manager_type %u", setup->display_manager_type);
		}

		const char* extensions[] = {
			"VK_KHR_surface",
			display_manager_extension_name,
		};

		VkApplicationInfo application_info = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = NULL,
			.pApplicationName = setup->application_name,
			.applicationVersion = VK_MAKE_API_VERSION(0, HERO_VERSION_MAJOR, HERO_VERSION_MINOR, HERO_VERSION_PATCH),
			.pEngineName = HERO_ENGINE_NAME,
			.engineVersion = VK_MAKE_API_VERSION(0, HERO_VERSION_MAJOR, HERO_VERSION_MINOR, HERO_VERSION_PATCH),
			.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0),
		};

		VkInstanceCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.pApplicationInfo = &application_info,
			.enabledLayerCount = layers_count,
			.ppEnabledLayerNames = (const char* const*)layers,
			.enabledExtensionCount = HERO_ARRAY_COUNT(extensions),
			.ppEnabledExtensionNames = (const char* const*)extensions,
		};

		vk_result = vkCreateInstance(&create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_instance);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		hero_gfx_sys_vulkan.instance = vk_instance;
	}

	//
	// initialize all of the function pointers to that call directly into the vulkan driver
	// for functions that are called _before_ creating a logical device
	//
	{
#define HERO_VULKAN_FN(NAME) \
		/* end */
#define HERO_VULKAN_INSTANCE_FN(NAME) \
		NAME = (PFN_##NAME)vkGetInstanceProcAddr(vk_instance, #NAME); \
		HERO_DEBUG_ASSERT(NAME, "could not find the '" #NAME "' symbol for the vulkan instance"); \
		/* end */
#define HERO_VULKAN_DEVICE_FN(NAME) \
		/* end */

		HERO_VULKAN_FN_LIST
#undef HERO_VULKAN_FN
#undef HERO_VULKAN_INSTANCE_FN
#undef HERO_VULKAN_DEVICE_FN
	}

	//
	// create the physical devices.
	//
	{
		VkPhysicalDevice* vk_physical_devices = NULL;
		U32 physical_devices_count = 0;

		vk_result = vkEnumeratePhysicalDevices(vk_instance, &physical_devices_count, NULL);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		vk_physical_devices = hero_alloc_array(VkPhysicalDevice, hero_system_alctor, HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES_VULKAN, physical_devices_count);
		if (vk_physical_devices == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_result = vkEnumeratePhysicalDevices(vk_instance, &physical_devices_count, vk_physical_devices);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		HeroPhysicalDeviceVulkan* physical_devices = hero_alloc_array(HeroPhysicalDeviceVulkan, hero_system_alctor, HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES, physical_devices_count);
		if (physical_devices == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		HERO_ZERO_ELMT_MANY(physical_devices, physical_devices_count);

		for (U32 i = 0; i < physical_devices_count; i += 1) {
			HeroPhysicalDeviceVulkan* physical_device_vulkan = &physical_devices[i];
			VkPhysicalDevice vk_physical_device = vk_physical_devices[i];
			result = _hero_vulkan_physical_device_init(physical_device_vulkan, vk_physical_device);
			if (result < 0) {
				return result;
			}
		}

		hero_gfx_sys.physical_devices = physical_devices;
		hero_gfx_sys.physical_device_size = sizeof(HeroPhysicalDeviceVulkan);
		hero_gfx_sys.physical_devices_count = physical_devices_count;
	}

	result = hero_object_pool(HeroVertexLayoutVulkan, init)(&hero_gfx_sys_vulkan.vertex_layout_pool, setup->vertex_layouts_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_POOL);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_physical_device_surface_image_formats_supported(HeroPhysicalDevice* physical_device, HeroSurface surface, HeroImageFormat* formats, U32 formats_count) {
	HeroResult result;
	VkResult vk_result;
	HeroPhysicalDeviceVulkan* physical_device_vulkan = (HeroPhysicalDeviceVulkan*)physical_device;

	//
	// get the surfaces formats
	VkSurfaceFormatKHR* surface_formats;
	U32 surface_formats_count;
	{
		VkSurfaceKHR vk_surface = VK_NULL_HANDLE; // TODO
		vk_result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_vulkan->handle, vk_surface, &surface_formats_count, NULL);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		surface_formats = hero_alloc_array(VkSurfaceFormatKHR, hero_system_alctor, HERO_GFX_ALLOC_TAG_PHYSICAL_DEVICES_VULKAN_SURFACE_FORMATS, surface_formats_count);
		if (surface_formats == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_vulkan->handle, vk_surface, &physical_device_vulkan->surface_formats_count, surface_formats);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}


	for (int i = 0; i < surface_formats_count; i += 1) {
		HeroImageFormat format = _hero_vulkan_convert_from_format(surface_formats[i].format);

		for (int j = 0; j < formats_count; j += 1) {
			if (formats[j] == format) {
				return HERO_SUCCESS;
			}
		}
	}

	return HERO_ERROR(DOES_NOT_EXIST);
}

void _hero_vulkan_queue_create_info_init(VkDeviceQueueCreateInfo* create_info, U32 queue_family_idx) {
	static float queue_priorities = 1.0;
	create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	create_info->pNext = NULL;
	create_info->flags = 0;
	create_info->queueFamilyIndex = queue_family_idx;
	create_info->queueCount = 1;
	create_info->pQueuePriorities = &queue_priorities;
}

HeroResult _hero_vulkan_logical_device_init(HeroPhysicalDevice* physical_device, HeroLogicalDeviceSetup* setup, HeroLogicalDevice** out) {
	HeroResult result;
	VkResult vk_result;
	HeroPhysicalDeviceVulkan* physical_device_vulkan = (HeroPhysicalDeviceVulkan*)physical_device;

	HeroLogicalDeviceVulkan* ldev_vulkan = hero_alloc_elmt(HeroLogicalDeviceVulkan, setup->alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE);
	if (ldev_vulkan == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	HERO_ZERO_ELMT(ldev_vulkan);

	if (setup->queue_support_flags == 0 || !HERO_BITSET_CONTAINS_ALL(setup->queue_support_flags, physical_device->queue_support_flags)) {
		return HERO_ERROR(GFX_QUEUE_SUPPORT_INCOMPATIBLE);
	}

	U32 queues_count = 0;
	VkDeviceQueueCreateInfo queue_create_infos[HERO_PHYSICAL_DEVICE_VULKAN_QUEUES_CAP];

	_hero_vulkan_queue_create_info_init(&queue_create_infos[queues_count], physical_device_vulkan->queue_family_idx_uber);
	queues_count += 1;

	if (
		(setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_PRESENT) &&
		physical_device_vulkan->queue_family_idx_uber != physical_device_vulkan->queue_family_idx_present
	) {
		_hero_vulkan_queue_create_info_init(&queue_create_infos[queues_count], physical_device_vulkan->queue_family_idx_present);
		queues_count += 1;
	}

	if (setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE) {
		_hero_vulkan_queue_create_info_init(&queue_create_infos[queues_count], physical_device_vulkan->queue_family_idx_async_compute);
		queues_count += 1;
	}

	if (setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER) {
		_hero_vulkan_queue_create_info_init(&queue_create_infos[queues_count], physical_device_vulkan->queue_family_idx_async_transfer);
		queues_count += 1;
	}

	const char* extension_list[] = {
		"VK_KHR_swapchain",
	};

	const char* const* extensions = NULL;
	U32 extensions_count = 0;
	if (setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_PRESENT) {
		extensions = extension_list;
		extensions_count = 1;
	}

	//
	// translate our physical device features into vulkan ones
	VkPhysicalDeviceFeatures vk_features = {0};
	for_range(i, 0, HERO_ARRAY_COUNT(setup->feature_flags.bitset)) {
		HeroPhysicalDeviceFeature base_feature = i * 64;
		U64 bitset = setup->feature_flags.bitset[i];

		while (bitset) {
			// get the index of the least significant bit that is set.
			// this will be the next enumeration.
			HeroPhysicalDeviceFeature feature = base_feature + HERO_LEAST_SET_BIT_IDX_U64(bitset);

			U32 offset = _hero_vulkan_convert_to_physical_device_features_field_offsets_plus_one[feature];
			if (offset == 0) {
				continue;
			}
			offset -= 1;

			*(VkBool32*)HERO_PTR_ADD(&vk_features, offset) = VK_TRUE;

			bitset = HERO_LEAST_SET_BIT_REMOVE(bitset);
		}
	}

	VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features = {0};
	buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	buffer_device_address_features.bufferDeviceAddress = VK_TRUE;

	VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {0};
	descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	descriptor_indexing_features.pNext = &buffer_device_address_features;
    descriptor_indexing_features.shaderInputAttachmentArrayDynamicIndexing = VK_TRUE;
    descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    descriptor_indexing_features.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;

	VkDeviceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &descriptor_indexing_features,
		.flags = 0,
		.pQueueCreateInfos = queue_create_infos,
		.queueCreateInfoCount = queues_count,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = extensions_count,
		.ppEnabledExtensionNames = extensions,
		.pEnabledFeatures = &vk_features,
	};

	VkDevice vk_device;
	vk_result = vkCreateDevice(physical_device_vulkan->handle, &create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_device);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	{
		//
		// initialize all of the function pointers to that call directly into the vulkan driver
		// for functions that are called _after_ creating a logical device
		//
#define HERO_VULKAN_FN(NAME)
#define HERO_VULKAN_INSTANCE_FN(NAME)
#define HERO_VULKAN_DEVICE_FN(NAME) \
		ldev_vulkan->NAME = (PFN_##NAME)vkGetDeviceProcAddr(vk_device, #NAME); \
		HERO_DEBUG_ASSERT(ldev_vulkan->NAME, "could not find the '" #NAME "' symbol for the vulkan device"); \
		/* end */

		HERO_VULKAN_FN_LIST
#undef HERO_VULKAN_FN
#undef HERO_VULKAN_INSTANCE_FN
#undef HERO_VULKAN_DEVICE_FN
	}

	//
	// fetch the logical queues that were created with the logical device
	ldev_vulkan->vkGetDeviceQueue(vk_device, physical_device_vulkan->queue_family_idx_uber, 0, &ldev_vulkan->queue_uber);
	if (setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_PRESENT) {
		ldev_vulkan->vkGetDeviceQueue(vk_device, physical_device_vulkan->queue_family_idx_present, 0, &ldev_vulkan->queue_present);
	}
	if (setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_COMPUTE) {
		ldev_vulkan->vkGetDeviceQueue(vk_device, physical_device_vulkan->queue_family_idx_async_compute, 0, &ldev_vulkan->queue_async_compute);
	}
	if (setup->queue_support_flags & HERO_QUEUE_SUPPORT_FLAGS_ASYNC_TRANSFER) {
		ldev_vulkan->vkGetDeviceQueue(vk_device, physical_device_vulkan->queue_family_idx_async_transfer, 0, &ldev_vulkan->queue_async_transfer);
	}

	VkSemaphore vk_semaphore_present;
	{
		VkSemaphoreCreateInfo vk_semaphore_create_info = {0};
		vk_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vk_result = ldev_vulkan->vkCreateSemaphore(vk_device, &vk_semaphore_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_semaphore_present);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	VkDescriptorSetLayout null_descriptor_set_layout;
	{
		VkDescriptorSetLayoutCreateInfo vk_set_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.bindingCount = 0,
			.pBindings = NULL,
		};

		vk_result = ldev_vulkan->vkCreateDescriptorSetLayout(vk_device, &vk_set_layout_create_info, HERO_VULKAN_TODO_ALLOCATOR, &null_descriptor_set_layout);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	ldev_vulkan->handle = vk_device;
	ldev_vulkan->queue_family_idx_uber = physical_device_vulkan->queue_family_idx_uber;
	ldev_vulkan->queue_family_idx_present = physical_device_vulkan->queue_family_idx_present;
	ldev_vulkan->queue_family_idx_async_compute = physical_device_vulkan->queue_family_idx_async_compute;
	ldev_vulkan->queue_family_idx_async_transfer = physical_device_vulkan->queue_family_idx_async_transfer;
	ldev_vulkan->semaphore_present = vk_semaphore_present;
	ldev_vulkan->null_descriptor_set_layout = null_descriptor_set_layout;

	result = hero_object_pool(HeroBufferVulkan, init)(&ldev_vulkan->buffer_pool, setup->buffers_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_BUFFER_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroImageVulkan, init)(&ldev_vulkan->image_pool, setup->images_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_IMAGE_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroSamplerVulkan, init)(&ldev_vulkan->sampler_pool, setup->samplers_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_SAMPLER_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroShaderModuleVulkan, init)(&ldev_vulkan->shader_module_pool, setup->shader_modules_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_SHADER_MODULE_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroShaderVulkan, init)(&ldev_vulkan->shader_pool, setup->shaders_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_SHADER_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroDescriptorPoolVulkan, init)(&ldev_vulkan->descriptor_pool_pool, setup->descriptor_pools_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroShaderGlobalsVulkan, init)(&ldev_vulkan->shader_globals_pool, setup->shader_globals_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_SHADER_GLOBALS_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroRenderPassLayoutVulkan, init)(&ldev_vulkan->render_pass_layout_pool, setup->render_pass_layouts_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_LAYOUT_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroRenderPassVulkan, init)(&ldev_vulkan->render_pass_pool, setup->render_passes_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroFrameBufferVulkan, init)(&ldev_vulkan->frame_buffer_pool, setup->frame_buffers_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_FRAME_BUFFER_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroPipelineCacheVulkan, init)(&ldev_vulkan->pipeline_cache_pool, setup->pipeline_caches_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_PIPELINE_CACHE_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroPipelineVulkan, init)(&ldev_vulkan->pipeline_pool, setup->pipelines_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_PIPELINE_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroMaterialVulkan, init)(&ldev_vulkan->material_pool, setup->materials_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_MATERIAL_POOL);
	if (result < 0) {
		return result;
	}
	result = hero_object_pool(HeroSwapchainVulkan, init)(&ldev_vulkan->swapchain_pool, setup->swapchains_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_SWAPCHAIN_POOL);
	if (result < 0) {
		return result;
	}
	/*
	result = hero_object_pool(HeroCommandPoolVulkan, init)(&ldev_vulkan->command_pool_pool, setup->command_pools_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_POOL);
	if (result < 0) {
		return result;
	}
	*/
	result = hero_object_pool(HeroRenderGraphVulkan, init)(&ldev_vulkan->render_graph_pool, setup->render_graphs_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_GRAPH_POOL);
	if (result < 0) {
		return result;
	}

	//
	// initialize the stager
	{
		VkCommandPoolCreateInfo vk_command_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags =
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | // command buffers will be recycled using vkResetCommandBuffer
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,             // command buffers will short lived by being reset after every use
			.queueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
		};

		VkCommandPool vk_command_pool;
		vk_result = ldev_vulkan->vkCreateCommandPool(ldev_vulkan->handle, &vk_command_pool_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_command_pool);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		ldev_vulkan->staging_buffer_sys.command_pool = vk_command_pool;
	}

	//
	// create the null descriptor handles so we can initialize the descriptor sets with these.
	// this allows the users to not have to set them when they create there shader globals and materials.
	{
		{
			VkSamplerCreateInfo vk_create_info = {0};
			vk_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

			vk_result = ldev_vulkan->vkCreateSampler(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_sampler);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
		}

		{
			VkImageCreateInfo vk_image_create_info = {0};
			vk_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			vk_image_create_info.extent.width = 1;
			vk_image_create_info.extent.height = 1;
			vk_image_create_info.extent.depth = 1;
			vk_image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			vk_image_create_info.format = VK_FORMAT_R8_UNORM;
			vk_image_create_info.samples = 1;
			vk_image_create_info.mipLevels = 1;
			vk_image_create_info.arrayLayers = 1;

			vk_image_create_info.imageType = VK_IMAGE_TYPE_1D;
			vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_image_1d);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			vk_image_create_info.imageType = VK_IMAGE_TYPE_2D;
			vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_image_2d);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			vk_image_create_info.imageType = VK_IMAGE_TYPE_3D;
			vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_image_3d);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			VkDeviceMemory vk_device_memory;
			{
				VkMemoryRequirements mem_req;
				ldev_vulkan->vkGetImageMemoryRequirements(ldev_vulkan->handle, ldev_vulkan->null_image_2d, &mem_req);

				_HeroVulkanAllocSetup alloc_setup = {
					.size = mem_req.size,
					.align = mem_req.alignment,
					.memory_type_bits = mem_req.memoryTypeBits,
					.memory_location = HERO_MEMORY_LOCATION_SHARED,
					.type = _HERO_VULKAN_ALLOC_TYPE_IMAGE,
				};

				result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &vk_device_memory);
				if (result < 0) {
					return result;
				}
			}

			VkDeviceSize vk_device_memory_offset = 0;
			vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, ldev_vulkan->null_image_1d, vk_device_memory, vk_device_memory_offset);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, ldev_vulkan->null_image_2d, vk_device_memory, vk_device_memory_offset);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, ldev_vulkan->null_image_3d, vk_device_memory, vk_device_memory_offset);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}


			VkImageViewCreateInfo vk_image_view_create_info = {0};
			vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			vk_image_view_create_info.format = VK_FORMAT_R8_UNORM;
			vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
			vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
			vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
			vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
			vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			vk_image_view_create_info.subresourceRange.layerCount = 1;

			vk_image_view_create_info.image = ldev_vulkan->null_image_1d;
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
			vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_image_view_1d);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
			vk_image_view_create_info.image = ldev_vulkan->null_image_2d;
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_image_view_2d);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
			vk_image_view_create_info.image = ldev_vulkan->null_image_3d;
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_image_view_3d);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
		}

		{
			VkBufferCreateInfo vk_create_info = {0};
			vk_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			vk_create_info.size = 1;
			vk_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			vk_result = ldev_vulkan->vkCreateBuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &ldev_vulkan->null_buffer);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			VkDeviceMemory vk_device_memory;
			void* mapped_memory;
			{
				VkMemoryRequirements mem_req;
				ldev_vulkan->vkGetBufferMemoryRequirements(ldev_vulkan->handle, ldev_vulkan->null_buffer, &mem_req);

				_HeroVulkanAllocSetup alloc_setup = {
					.size = mem_req.size,
					.align = mem_req.alignment,
					.memory_type_bits = mem_req.memoryTypeBits,
					.memory_location = HERO_MEMORY_LOCATION_SHARED,
					.type = _HERO_VULKAN_ALLOC_TYPE_BUFFER,
				};

				result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &vk_device_memory);
				if (result < 0) {
					return result;
				}
			}

			VkDeviceSize vk_device_memory_offset = 0;
			vk_result = ldev_vulkan->vkBindBufferMemory(ldev_vulkan->handle, ldev_vulkan->null_buffer, vk_device_memory, vk_device_memory_offset);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
		}
	}

	*out = &ldev_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_logical_device_deinit(HeroLogicalDevice* ldev) {

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_pool_free_used_resources(HeroLogicalDevice* ldev, HeroDescriptorPoolVulkan* descriptor_pool_vulkan);

/*
HeroResult _hero_vulkan_command_pool_free_used_resources(HeroLogicalDevice* ldev, HeroCommandPoolVulkan* command_pool_vulkan);
*/

HeroResult _hero_vulkan_logical_device_frame_start(HeroLogicalDevice* ldev) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (ldev_vulkan->submits.count == 0) {
		return HERO_SUCCESS;
	}

	//
	// try to pop the submits from the queue if they are done.
	// if they are done, then clean up their resources.
	while (ldev_vulkan->submits.count) {
		_HeroGfxSubmitVulkan* submit = hero_stack(_HeroGfxSubmitVulkan, get)(&ldev_vulkan->submits, 0);
		vk_result = ldev_vulkan->vkGetFenceStatus(ldev_vulkan->handle, submit->fence_render);
		if (vk_result == VK_SUCCESS) {
			/*
			HeroCommandPoolId command_pool_id = {0};
			HeroCommandPoolVulkan* command_pool_vulkan;
			while (hero_object_pool(HeroCommandPoolVulkan, iter_next)(&ldev_vulkan->command_pool_pool, &command_pool_id, &command_pool_vulkan) != HERO_SUCCESS_FINISHED) {
				result = _hero_vulkan_command_pool_free_used_resources(ldev, command_pool_vulkan);
				if (result < 0) {
					return result;
				}
			}
			*/

			HeroDescriptorPoolId descriptor_pool_id = {0};
			HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
			while (hero_object_pool(HeroDescriptorPoolVulkan, iter_next)(&ldev_vulkan->descriptor_pool_pool, &descriptor_pool_id, &descriptor_pool_vulkan) != HERO_SUCCESS_FINISHED) {
				result = _hero_vulkan_descriptor_pool_free_used_resources(ldev, descriptor_pool_vulkan);
				if (result < 0) {
					return result;
				}
			}

			{
				U32 idx = 0;
				while (idx < ldev_vulkan->objects_to_deallocate.count) {
					HeroVulkanObjectDealloc* dealloc = &ldev_vulkan->objects_to_deallocate.data[idx];
					if (dealloc->image == VK_NULL_HANDLE) {
						idx += 1; // add 1 to remove this null entry from the array
						break;
					}

					HERO_DEBUG_ASSERT(dealloc->header & HERO_VULKAN_OBJECT_DEALLOC_IS_HEADER, "expected deallocation to start with the header");

					U32 entries_count = HERO_VULKAN_OBJECT_DEALLOC_HEADER_ENTRIES_COUNT(*dealloc);
					U32 objects_count = entries_count - 1; // minus 1 for the header itself
					switch (HERO_VULKAN_OBJECT_DEALLOC_HEADER_OBJECT_TYPE(*dealloc)) {
						case VK_OBJECT_TYPE_IMAGE:
							for_range(i, 0, objects_count) {
								ldev_vulkan->vkDestroyImage(ldev_vulkan->handle, dealloc[i].image, HERO_VULKAN_TODO_ALLOCATOR);

								i += 1;
								ldev_vulkan->vkDestroyImageView(ldev_vulkan->handle, dealloc[i].image_view, HERO_VULKAN_TODO_ALLOCATOR);
							}
							break;
						case VK_OBJECT_TYPE_BUFFER:
							for_range(i, 0, objects_count) {
								ldev_vulkan->vkDestroyBuffer(ldev_vulkan->handle, dealloc[i].buffer, HERO_VULKAN_TODO_ALLOCATOR);
							}
							break;
						case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
							for_range(i, 0, objects_count) {
								ldev_vulkan->vkDestroyDescriptorPool(ldev_vulkan->handle, dealloc[i].descriptor_pool, HERO_VULKAN_TODO_ALLOCATOR);
							}
							break;
						case VK_OBJECT_TYPE_COMMAND_POOL:
							for_range(i, 0, objects_count) {
								ldev_vulkan->vkDestroyCommandPool(ldev_vulkan->handle, dealloc[i].command_pool, HERO_VULKAN_TODO_ALLOCATOR);
							}
							break;
						case VK_OBJECT_TYPE_RENDER_PASS:
							for_range(i, 0, objects_count) {
								ldev_vulkan->vkDestroyRenderPass(ldev_vulkan->handle, dealloc[i].render_pass, HERO_VULKAN_TODO_ALLOCATOR);
							}
							break;
						case VK_OBJECT_TYPE_FRAMEBUFFER:
							for_range(i, 0, objects_count) {
								ldev_vulkan->vkDestroyFramebuffer(ldev_vulkan->handle, dealloc[i].frame_buffer, HERO_VULKAN_TODO_ALLOCATOR);
							}
							break;
					}

					idx += entries_count;
				}

				hero_stack(HeroVulkanObjectDealloc, remove_shift_range)(&ldev_vulkan->objects_to_deallocate, 0, idx);
			}

			{
				_HeroGfxStagingBufferSysVulkan* sys = &ldev_vulkan->staging_buffer_sys;

				if (sys->free_buffers_in_use.count) {
					//
					// count how many command buffers that are no longer being used by the GPU
					// that we can move into the free_buffers array
					U32 copy_count = 0;
					U32 remove_count = 0;
					for_range(j, 0, sys->free_buffers_in_use.count) {
						remove_count += 1;
						sys->free_buffers_in_use.data[j].size = 0;
						if (sys->free_buffers_in_use.data[j].buffer == VK_NULL_HANDLE) {
							break;
						}
						copy_count += 1; // avoid copying the VK_NULL_HANDLE
					}

					//
					// copy the elements over to the free_buffers array
					_HeroGfxStagingBufferVulkan* dst;
					result = hero_stack(_HeroGfxStagingBufferVulkan, push_many)(&sys->free_buffers, copy_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_BUFFERS, &dst);
					if (result < 0) {
						return result;
					}
					HERO_COPY_ELMT_MANY(dst, sys->free_buffers_in_use.data, copy_count);

					//
					// remove the from the free_buffers_in_use array
					hero_stack(_HeroGfxStagingBufferVulkan, remove_shift_range)(&sys->free_buffers_in_use, 0, remove_count);
				}

				if (sys->free_command_buffers_in_use.count) {
					//
					// count how many command buffers that are no longer being used by the GPU
					// that we can move into the free_command_buffers array
					U32 copy_count = 0;
					U32 remove_count = 0;
					for_range(j, 0, sys->free_command_buffers_in_use.count) {
						remove_count += 1;
						if (sys->free_command_buffers_in_use.data[j] == VK_NULL_HANDLE) {
							break;
						}
						copy_count += 1; // avoid copying the VK_NULL_HANDLE
					}

					//
					// copy the elements over to the free_command_buffers array
					VkCommandBuffer* dst;
					result = hero_stack(VkCommandBuffer, push_many)(&sys->free_command_buffers, copy_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_COMMAND_BUFFERS, &dst);
					if (result < 0) {
						return result;
					}
					HERO_COPY_ELMT_MANY(dst, sys->free_command_buffers_in_use.data, copy_count);

					//
					// remove the from the free_command_buffers_in_use array
					hero_stack(VkCommandBuffer, remove_shift_range)(&sys->free_command_buffers_in_use, 0, remove_count);
				}
			}

		} else if (vk_result != VK_NOT_READY) {
			return _hero_vulkan_convert_from_result(vk_result);
		} else {
			break;
		}


		ldev->last_completed_frame_idx += 1;
		hero_stack(_HeroGfxSubmitVulkan, remove_shift)(&ldev_vulkan->submits, 0);
	}

	//
	// push on a VK_NULL_HANDLE to seperate the previous frame resources from this frame's resources
	{
		/*
		HeroCommandPoolId command_pool_id = {0};
		HeroCommandPoolVulkan* command_pool_vulkan;
		while (hero_object_pool(HeroCommandPoolVulkan, iter_next)(&ldev_vulkan->command_pool_pool, &command_pool_id, &command_pool_vulkan) != HERO_SUCCESS_FINISHED) {
			if (command_pool_vulkan->free_buffers_in_use.count) {
				result = hero_stack(VkCommandBuffer, push_value)(&command_pool_vulkan->free_buffers_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS_IN_USE, VK_NULL_HANDLE);

				if (result < 0) {
					return result;
				}
			}
		}
		*/

		HeroDescriptorPoolId descriptor_pool_id = {0};
		HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
		while (hero_object_pool(HeroDescriptorPoolVulkan, iter_next)(&ldev_vulkan->descriptor_pool_pool, &descriptor_pool_id, &descriptor_pool_vulkan) != HERO_SUCCESS_FINISHED) {
			for (U32 i = 0; i < descriptor_pool_vulkan->layouts_count; i += 1) {
				_HeroDescriptorSetLayoutAuxVulkan* layout_aux = &descriptor_pool_vulkan->layout_auxs[i];
				if (layout_aux->free_sets_in_use.count) {
					result = hero_stack(VkDescriptorSet, push_value)(&layout_aux->free_sets_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS_IN_USE, VK_NULL_HANDLE);

					if (result < 0) {
						return result;
					}
				}
			}
		}

		//
		// if we have objects to deallocate, push on a null entry to seperate the frame.
		if (ldev_vulkan->objects_to_deallocate.count) {
			result = hero_stack(HeroVulkanObjectDealloc, push_value)(&ldev_vulkan->objects_to_deallocate, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_OBJECTS_TO_DEALLOCATE, ((HeroVulkanObjectDealloc) { .image = VK_NULL_HANDLE }));

			if (result < 0) {
				return result;
			}
		}
		ldev_vulkan->objects_to_deallocate_prev_object_type = VK_OBJECT_TYPE_UNKNOWN;

		{
			_HeroGfxStagingBufferSysVulkan* sys = &ldev_vulkan->staging_buffer_sys;
			if (sys->free_buffers_in_use.count) {
				_HeroGfxStagingBufferVulkan* dst;
				result = hero_stack(_HeroGfxStagingBufferVulkan, push)(&sys->free_buffers_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_BUFFERS_IN_USE, &dst);

				if (result < 0) {
					return result;
				}
				dst->buffer = VK_NULL_HANDLE;
			}

			if (sys->free_command_buffers_in_use.count) {
				result = hero_stack(VkCommandBuffer, push_value)(&sys->free_command_buffers_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_COMMAND_BUFFERS_IN_USE, VK_NULL_HANDLE);

				if (result < 0) {
					return result;
				}
			}
		}
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_logical_device_queue_transfer(HeroLogicalDevice* ldev) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;


	_HeroGfxStagingBufferSysVulkan* sys = &ldev_vulkan->staging_buffer_sys;
	if (sys->staged_updates.count) {
		//
		// get the command buffer, reuse one that is already allocated if we have any.
		VkCommandBuffer vk_command_buffer;
		if (sys->free_command_buffers.count) {
			vk_command_buffer = *hero_stack(VkCommandBuffer, get_last)(&sys->free_command_buffers);
			hero_stack(VkCommandBuffer, pop)(&sys->free_command_buffers);
		} else {
			VkCommandBufferAllocateInfo vk_command_buffer_alloc_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext = NULL,
				.commandPool = sys->command_pool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = 1, // maybe in the future we can allocate more and store the rest in free_command_buffers
			};

			vk_result = ldev_vulkan->vkAllocateCommandBuffers(ldev_vulkan->handle, &vk_command_buffer_alloc_info, &vk_command_buffer);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
		}

		//
		// begin command buffer recording
		{
			VkCommandBufferBeginInfo vk_command_buffer_begin_info = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.pNext = NULL,
				.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // command buffer is reset after a single submit
				.pInheritanceInfo = NULL,
			};

			vk_result = ldev_vulkan->vkBeginCommandBuffer(vk_command_buffer, &vk_command_buffer_begin_info);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
		}

		static bool init_null_images = false;
		if (!init_null_images) {
			init_null_images = true;
			VkImageMemoryBarrier memory_barrier = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.pNext = NULL,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.srcQueueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
				.dstQueueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
				.image = ldev_vulkan->null_image_2d,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};

			ldev_vulkan->vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, NULL, 0, NULL, 1, &memory_barrier);
		}

		for_range(i, 0, sys->staged_updates.count) {
			_HeroGfxStagedUpdate* update = &sys->staged_updates.data[i];

			if (update->is_image) {
				HeroImageArea* area = &update->data.image.area;

				VkBufferImageCopy copy_region = {
					.bufferOffset = update->src_offset,
					.bufferRowLength = 0,
					.bufferImageHeight = 0,
					.imageSubresource = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = area->mip_level,
						.baseArrayLayer = area->array_layer,
						.layerCount = area->array_layers_count,
					},
					.imageOffset = {
						.x = area->offset_x,
						.y = area->offset_y,
						.z = area->offset_z,
					},
					.imageExtent = {
						.width = area->width,
						.height = area->height,
						.depth = area->depth,
					},
				};

				VkImageMemoryBarrier memory_barrier = {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = NULL,
					.srcAccessMask = 0,
					.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.srcQueueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
					.dstQueueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
					.image = update->data.image.dst_image,
					.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = area->mip_level,
						.levelCount = 1,
						.baseArrayLayer = area->array_layer,
						.layerCount = area->array_layers_count,
					},
				};

				ldev_vulkan->vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &memory_barrier);

				ldev_vulkan->vkCmdCopyBufferToImage(vk_command_buffer, update->src_buffer, update->data.image.dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

				memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				memory_barrier.newLayout = update->data.image.layout;
				memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				ldev_vulkan->vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, NULL, 0, NULL, 1, &memory_barrier);
			} else {
				VkBufferCopy copy_region = {
					.srcOffset = update->src_offset,
					.dstOffset = update->data.buffer.dst_offset,
					.size = update->data.buffer.size,
				};

				ldev_vulkan->vkCmdCopyBuffer(vk_command_buffer, update->src_buffer, update->data.buffer.dst_buffer, 1, &copy_region);
			}
		}

		//
		// end the command buffer recording
		vk_result = ldev_vulkan->vkEndCommandBuffer(vk_command_buffer);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		//
		// queue the command buffer to submit
		VkCommandBuffer* dst;
		result = hero_stack(VkCommandBuffer, push)(&ldev_vulkan->submit_command_buffers, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_COMMAND_BUFFERS, &dst);
		if (result < 0) {
			return result;
		}
		*dst = vk_command_buffer;

		//
		// queue the command buffer to reuse later
		result = hero_stack(VkCommandBuffer, push)(&sys->free_command_buffers_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_COMMAND_BUFFERS_IN_USE, &dst);
		if (result < 0) {
			return result;
		}
		*dst = vk_command_buffer;

		//
		// move the staging buffers into the free_buffers_in_use
		{
			//
			// copy the elements over to the free_buffers array
			_HeroGfxStagingBufferVulkan* dst;
			result = hero_stack(_HeroGfxStagingBufferVulkan, push_many)(&sys->free_buffers_in_use, sys->used_buffers_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_BUFFERS_IN_USE, &dst);
			if (result < 0) {
				return result;
			}
			HERO_COPY_ELMT_MANY(dst, sys->free_buffers.data, sys->used_buffers_count);

			//
			// remove the from the free_buffers array
			hero_stack(_HeroGfxStagingBufferVulkan, remove_shift_range)(&sys->free_buffers, 0, sys->used_buffers_count);
			sys->used_buffers_count = 0;
		}

		sys->staged_updates.count = 0;
	}

	return HERO_SUCCESS;
}

/*
HeroResult _hero_vulkan_logical_device_queue_command_buffers(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, HeroCommandPoolBufferId* command_pool_buffer_ids, U32 command_pool_buffers_count) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkCommandBuffer* vk_command_buffers;
	result = hero_stack(VkCommandBuffer, push_many)(&ldev_vulkan->submit_command_buffers, command_pool_buffers_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_COMMAND_BUFFERS, &vk_command_buffers);
	if (result < 0) {
		return result;
	}

	HeroCommandPoolVulkan* command_pool_vulkan;
	result = hero_object_pool(HeroCommandPoolVulkan, get)(&ldev_vulkan->command_pool_pool, command_pool_id, &command_pool_vulkan);
	if (result < 0) {
		return result;
	}

	for_range(i, 0, command_pool_buffers_count) {
		HeroCommandPoolBufferVulkan* command_pool_buffer_vulkan;
		result = hero_object_pool(HeroCommandPoolBufferVulkan, get)(&command_pool_vulkan->command_buffer_pool, command_pool_buffer_ids[i], &command_pool_buffer_vulkan);
		if (result < 0) {
			return result;
		}

		VkCommandBuffer vk_command_buffer = command_pool_buffer_vulkan->command_buffer;
		vk_command_buffers[i] = vk_command_buffer;

		//
		// push the VkCommandBuffer on to the free_buffers_in_use so
		// that it is push on the free_buffers array when the next frame is finished
		result = hero_stack(VkCommandBuffer, push_value)(&command_pool_vulkan->free_buffers_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS_IN_USE, vk_command_buffer);

		result = hero_object_pool(HeroCommandPoolBufferVulkan, dealloc)(&command_pool_vulkan->command_buffer_pool, command_pool_buffer_ids[i]);
		if (result < 0) {
			return result;
		}
	}

	return HERO_SUCCESS;
}
*/

HeroResult _hero_vulkan_logical_device_submit(HeroLogicalDevice* ldev, HeroSwapchainId* swapchain_ids, U32 swapchains_count) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	{
		VkSemaphore* submit_render_semaphores;
		result = hero_stack(VkSemaphore, push_many)(&ldev_vulkan->submit_render_semaphores, swapchains_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_RENDER_SEMAPHORES, &submit_render_semaphores);
		if (result < 0) {
			return result;
		}

		VkSwapchainKHR* submit_swapchains;
		result = hero_stack(VkSwapchainKHR, push_many)(&ldev_vulkan->submit_swapchains, swapchains_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_SWAPCHAINS, &submit_swapchains);
		if (result < 0) {
			return result;
		}

		U32* submit_swapchain_image_indices;
		result = hero_stack(U32, push_many)(&ldev_vulkan->submit_swapchain_image_indices, swapchains_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMIT_SWAPCHAIN_IMAGE_INDICES, &submit_swapchain_image_indices);
		if (result < 0) {
			return result;
		}

		for_range(i, 0, swapchains_count) {
			HeroSwapchainVulkan* swapchain_vulkan;
			result = hero_object_pool(HeroSwapchainVulkan, get)(&ldev_vulkan->swapchain_pool, swapchain_ids[i], &swapchain_vulkan);
			if (result < 0) {
				return result;
			}

			submit_render_semaphores[i] = swapchain_vulkan->semaphore_render;
			submit_swapchains[i] = swapchain_vulkan->handle;
			submit_swapchain_image_indices[i] = swapchain_vulkan->image_idx;
		}
	}

	VkPipelineStageFlags wait_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo vk_submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = ldev_vulkan->submit_render_semaphores.count,
		.pWaitSemaphores = ldev_vulkan->submit_render_semaphores.data,
		.pWaitDstStageMask = &wait_flags,
		.commandBufferCount = ldev_vulkan->submit_command_buffers.count,
		.pCommandBuffers = ldev_vulkan->submit_command_buffers.data,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &ldev_vulkan->semaphore_present,
	};



	VkFence vk_fence = VK_NULL_HANDLE;
	if (ldev_vulkan->submit_free_fences.count) {
		vk_fence = *hero_stack(VkFence, get_last)(&ldev_vulkan->submit_free_fences);
		hero_stack(VkFence, pop)(&ldev_vulkan->submit_free_fences);

		vk_result = ldev_vulkan->vkResetFences(ldev_vulkan->handle, 1, &vk_fence);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	} else {
		VkFenceCreateInfo fence_create_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
		};

		vk_result = ldev_vulkan->vkCreateFence(ldev_vulkan->handle, &fence_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_fence);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	vk_result = ldev_vulkan->vkQueueSubmit(ldev_vulkan->queue_uber, 1, &vk_submit_info, vk_fence);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	//
	// TODO: find the present queue using platform function like: vkGetPhysicalDeviceXlibPresentationSupportKHR
	// so we can then call the vkQueuePresentKHR function a single time for all the swapchains
	//
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &ldev_vulkan->semaphore_present,
		.swapchainCount = ldev_vulkan->submit_swapchains.count,
		.pSwapchains = ldev_vulkan->submit_swapchains.data,
		.pImageIndices = ldev_vulkan->submit_swapchain_image_indices.data,
		.pResults = NULL,
	};
	vk_result = ldev_vulkan->vkQueuePresentKHR(ldev_vulkan->queue_present, &present_info);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	ldev_vulkan->submit_command_buffers.count = 0;
	ldev_vulkan->submit_render_semaphores.count = 0;
	ldev_vulkan->submit_swapchains.count = 0;
	ldev_vulkan->submit_swapchain_image_indices.count = 0;

	_HeroGfxSubmitVulkan* submit;
	result = hero_stack(_HeroGfxSubmitVulkan, push)(&ldev_vulkan->submits, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_SUBMITS, &submit);
	if (result < 0) {
		return result;
	}
	submit->fence_render = vk_fence;

	ldev->last_submitted_frame_idx += 1;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_dealloc_object_once_next_frame_has_executed(HeroLogicalDeviceVulkan* ldev_vulkan, VkObjectType object_type, U32 count, HeroVulkanObjectDealloc** out) {
	VkObjectType objects_to_deallocate_prev_object_type = ldev_vulkan->objects_to_deallocate_prev_object_type;

	// if we are an image, make enough room to have an image view be store after every image
	count *= (object_type == VK_OBJECT_TYPE_IMAGE) * 2;

	// add another count for the header if this is a different type from the last
	count += objects_to_deallocate_prev_object_type == VK_OBJECT_TYPE_UNKNOWN;

	HeroVulkanObjectDealloc* deallocs;
	HeroResult result = hero_stack(HeroVulkanObjectDealloc, push_many)(&ldev_vulkan->objects_to_deallocate, count, hero_system_alctor, HERO_GFX_ALLOC_TAG_LOGICAL_DEVICE_VULKAN_OBJECTS_TO_DEALLOCATE, &deallocs);
	if (result < 0) {
		return result;
	}

	if (objects_to_deallocate_prev_object_type != VK_OBJECT_TYPE_UNKNOWN) {
		//
		// here we are adding objects of the same type
		HeroVulkanObjectDealloc* existing_header = &ldev_vulkan->objects_to_deallocate.data[ldev_vulkan->objects_to_deallocate_prev_idx];
		U32 existing_count = HERO_VULKAN_OBJECT_DEALLOC_HEADER_ENTRIES_COUNT(*existing_header);
		existing_header->header = HERO_VULKAN_OBJECT_DEALLOC_HEADER(object_type, existing_count + count);
		*out = &deallocs[0];
	} else {
		//
		// new type so setup the new header
		deallocs[0].header = HERO_VULKAN_OBJECT_DEALLOC_HEADER(object_type, count);
		*out = &deallocs[1];
	}

	ldev_vulkan->objects_to_deallocate_prev_idx = deallocs - ldev_vulkan->objects_to_deallocate.data;
	ldev_vulkan->objects_to_deallocate_prev_object_type = object_type;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_stage_buffer_update(HeroLogicalDeviceVulkan* ldev_vulkan, bool is_image, _HeroGfxStagedUpdateData* data, void** destination_out) {
	HeroResult result;
	VkResult vk_result;
	U64 align = 1; // TODO
	U64 size;
	if (is_image) {
		HeroImageArea* area = &data->image.area;
		align = hero_image_format_bytes_per_pixel[data->image.format];
		size = (U64)area->width *
			(U64)area->height *
			(U64)area->depth *
			(U64)area->array_layers_count *
			(U64)hero_image_format_bytes_per_pixel[data->image.format];
	} else {
		size = data->buffer.size;
	}

	//
	// try to find a buffer that can fit the request size and align constraints
	_HeroGfxStagingBufferSysVulkan* sys = &ldev_vulkan->staging_buffer_sys;
	_HeroGfxStagingBufferVulkan* buffer = NULL;
	for_range(i, 0, sys->free_buffers.count) {
		_HeroGfxStagingBufferVulkan* b = &sys->free_buffers.data[i];
		U64 pos = u64_round_up_to_multiple(b->size, align);
		U64 end_pos = pos + size;
		if (end_pos > b->cap) {
			continue;
		}

		buffer = b;
		break;
	}

	//
	// if we cannot find a buffer with enough room, make another buffer
	// and push it on the end of buffers array.
	if (!buffer) {
		U64 buffer_size = HERO_MAX(HERO_VULKAN_STAGING_BUFFER_MIN_SIZE, size);

		U32 vk_queue_family_indices[HERO_PHYSICAL_DEVICE_VULKAN_QUEUES_CAP];
		U16 vk_queue_family_indices_count;
		_hero_vulkan_queue_family_indices_get(ldev_vulkan, ldev_vulkan->public_.queue_support_flags, vk_queue_family_indices, &vk_queue_family_indices_count);

		VkSharingMode sharing_mode;
		if (vk_queue_family_indices_count > 1) {
			sharing_mode = VK_SHARING_MODE_CONCURRENT;
		} else {
			sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
			vk_queue_family_indices_count = 0;
		}

		VkBufferCreateInfo vk_create_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.size = buffer_size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = sharing_mode,
			.queueFamilyIndexCount = vk_queue_family_indices_count,
			.pQueueFamilyIndices = vk_queue_family_indices,
		};

		VkBuffer vk_buffer;
		vk_result = ldev_vulkan->vkCreateBuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_buffer);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		VkDeviceMemory vk_device_memory;
		void* mapped_memory;
		{
			VkMemoryRequirements mem_req;
			ldev_vulkan->vkGetBufferMemoryRequirements(ldev_vulkan->handle, vk_buffer, &mem_req);

			_HeroVulkanAllocSetup alloc_setup = {
				.size = mem_req.size,
				.align = mem_req.alignment,
				.memory_type_bits = mem_req.memoryTypeBits,
				.memory_location = HERO_MEMORY_LOCATION_SHARED,
				.type = _HERO_VULKAN_ALLOC_TYPE_BUFFER,
			};

			result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &vk_device_memory);
			if (result < 0) {
				return result;
			}

			//
			// TODO: let the custom GPU memory allocator call this internally
			vk_result = ldev_vulkan->vkMapMemory(
				ldev_vulkan->handle,
				vk_device_memory,
				0,
				buffer_size,
				0,
				&mapped_memory);
		}

		VkDeviceSize vk_device_memory_offset = 0;
		vk_result = ldev_vulkan->vkBindBufferMemory(ldev_vulkan->handle, vk_buffer, vk_device_memory, vk_device_memory_offset);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		result = hero_stack(_HeroGfxStagingBufferVulkan, push)(&sys->free_buffers, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGING_FREE_BUFFERS, &buffer);
		if (result < 0) {
			return result;
		}

		buffer->buffer = vk_buffer;
		buffer->device_memory = vk_device_memory;
		buffer->size = 0;
		buffer->cap = buffer_size;
		buffer->mapped_memory = mapped_memory;
	}

	U64 src_offset = u64_round_up_to_multiple(buffer->size, align);
	void* dst = HERO_PTR_ADD(buffer->mapped_memory, src_offset);
	buffer->size += size;

	_HeroGfxStagedUpdate* update;
	result = hero_stack(_HeroGfxStagedUpdate, push)(&sys->staged_updates, hero_system_alctor, HERO_GFX_ALLOC_TAG_VULKAN_STAGED_UPDATES, &update);
	if (result < 0) {
		return result;
	}
	update->is_image = is_image;
	update->src_buffer = buffer->buffer;
	update->src_offset = src_offset;
	update->data = *data;

	U32 buffer_idx = buffer - sys->free_buffers.data;
	sys->used_buffers_count = HERO_MAX(sys->used_buffers_count, buffer_idx + 1);

	*destination_out = dst;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_vertex_layout_register(HeroVertexLayout* vl, HeroVertexLayoutId* id_out, HeroVertexLayout** out) {
	HeroVertexLayoutVulkan* vertex_layout_vulkan;
	HeroResult result = hero_object_pool(HeroVertexLayoutVulkan, alloc)(&hero_gfx_sys_vulkan.vertex_layout_pool, 0, &vertex_layout_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	VkVertexInputBindingDescription* vk_bindings = hero_alloc_array(VkVertexInputBindingDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_VULKAN_BINDINGS, vl->bindings_count);
	if (vk_bindings == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	U32 vk_attribs_count = 0;
	for (U32 i = 0; i < vl->bindings_count; i += 1) {
		HeroVertexBindingInfo* binding = &vl->bindings[i];
		vk_attribs_count += binding->attribs_count;
	}

	VkVertexInputAttributeDescription* vk_attribs = hero_alloc_array(VkVertexInputAttributeDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_VULKAN_ATTRIBUTES, vk_attribs_count);
	if (vk_attribs == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	U32 vk_attrib_idx = 0;
	for (U32 i = 0; i < vl->bindings_count; i += 1) {
		HeroVertexBindingInfo* binding = &vl->bindings[i];
		VkVertexInputBindingDescription* vk_desc = &vk_bindings[i];

		vk_desc->binding = i;
		vk_desc->stride = binding->size;
		vk_desc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		U32 offset = 0;
		for (U32 j = 0; j < binding->attribs_count; j += 1) {
			HeroVertexAttribInfo* attrib = &binding->attribs[j];
			VkVertexInputAttributeDescription* vk_attrib = &vk_attribs[vk_attrib_idx];

			vk_attrib->location = attrib->location;
			vk_attrib->binding = i;
			vk_attrib->format = _hero_vulkan_convert_to_vertex_input_format[attrib->elmt_type][attrib->vector_type];
			vk_attrib->offset = offset;

			offset = HERO_INT_ROUND_UP_ALIGN(offset, hero_vertex_elmt_type_aligns[attrib->elmt_type]);
			offset += (U32)hero_vertex_elmt_type_sizes[attrib->elmt_type] * (U32)HERO_VERTEX_VECTOR_TYPE_ELMTS_COUNT(attrib->vector_type);
			vk_attrib_idx += 1;
		}
	}

	vertex_layout_vulkan->bindings = vk_bindings;
	vertex_layout_vulkan->attribs = vk_attribs;
	vertex_layout_vulkan->attribs_count = vk_attribs_count;
	*out = &vertex_layout_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_vertex_layout_deregister(HeroVertexLayoutId id, HeroVertexLayout* vertex_layout) {
	HeroResult result;
	VkResult vk_result;
	HeroVertexLayoutVulkan* vertex_layout_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroVertexLayoutVulkan, vertex_layout);

	hero_dealloc_array(VkVertexInputBindingDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_VULKAN_BINDINGS, vertex_layout_vulkan->bindings, vertex_layout->bindings_count);
	hero_dealloc_array(VkVertexInputAttributeDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_VERTEX_LAYOUT_VULKAN_ATTRIBUTES, vertex_layout_vulkan->attribs, vertex_layout_vulkan->attribs_count);

	return hero_object_pool(HeroVertexLayoutVulkan, dealloc)(&hero_gfx_sys_vulkan.vertex_layout_pool, id);
}

HeroResult _hero_vulkan_vertex_layout_get(HeroVertexLayoutId id, HeroVertexLayout** out) {
	HeroResult result;
	VkResult vk_result;

	HeroVertexLayoutVulkan* vertex_layout_vulkan;
	result = hero_object_pool(HeroVertexLayoutVulkan, get)(&hero_gfx_sys_vulkan.vertex_layout_pool, id, &vertex_layout_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &vertex_layout_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_buffer_reinit(HeroLogicalDevice* ldev, HeroBufferVulkan* buffer_vulkan, U64 new_size) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (buffer_vulkan->handle != VK_NULL_HANDLE) {
		ldev_vulkan->vkDestroyBuffer(ldev_vulkan->handle, buffer_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);
		result = _hero_vulkan_device_memory_dealloc(ldev_vulkan, buffer_vulkan->device_memory);
		if (result < 0) {
			return result;
		}

		buffer_vulkan->handle = VK_NULL_HANDLE;
		buffer_vulkan->device_memory = VK_NULL_HANDLE;
	}

	VkBufferCreateFlags vk_create_flags = 0;

	VkBufferUsageFlags vk_usage_flags = 0;
	{
		vk_usage_flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vk_usage_flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		switch (buffer_vulkan->public_.type) {
			case HERO_BUFFER_TYPE_VERTEX:  vk_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
			case HERO_BUFFER_TYPE_INDEX:   vk_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
			case HERO_BUFFER_TYPE_UNIFORM: vk_usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
			case HERO_BUFFER_TYPE_STORAGE: vk_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
		}

		if (buffer_vulkan->public_.flags & HERO_BUFFER_FLAGS_INDIRECT_DRAW) {
			vk_usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		}
	}

	U32 vk_queue_family_indices[HERO_PHYSICAL_DEVICE_VULKAN_QUEUES_CAP];
	U16 vk_queue_family_indices_count;
	_hero_vulkan_queue_family_indices_get(ldev_vulkan, buffer_vulkan->public_.queue_support_flags, vk_queue_family_indices, &vk_queue_family_indices_count);

	VkSharingMode sharing_mode;
	if (vk_queue_family_indices_count > 1) {
		sharing_mode = VK_SHARING_MODE_CONCURRENT;
	} else {
		sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
		vk_queue_family_indices_count = 0;
	}

	VkBufferCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = vk_create_flags,
		.size = new_size,
		.usage = vk_usage_flags,
		.sharingMode = sharing_mode,
		.queueFamilyIndexCount = vk_queue_family_indices_count,
		.pQueueFamilyIndices = vk_queue_family_indices,
	};

	VkBuffer vk_buffer;
	vk_result = ldev_vulkan->vkCreateBuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkDeviceMemory vk_device_memory;
	HeroMemoryLocation memory_location = buffer_vulkan->public_.memory_location;
	{
		VkMemoryRequirements mem_req;
		ldev_vulkan->vkGetBufferMemoryRequirements(ldev_vulkan->handle, vk_buffer, &mem_req);

		_HeroVulkanAllocSetup alloc_setup = {
			.size = mem_req.size,
			.align = mem_req.alignment,
			.memory_type_bits = mem_req.memoryTypeBits,
			.memory_location = memory_location,
			.type = _HERO_VULKAN_ALLOC_TYPE_BUFFER,
		};

		result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &vk_device_memory);
		if (result < 0) {
			return result;
		}

		memory_location = alloc_setup.memory_location;
	}

	VkDeviceSize vk_device_memory_offset = 0;
	vk_result = ldev_vulkan->vkBindBufferMemory(ldev_vulkan->handle, vk_buffer, vk_device_memory, vk_device_memory_offset);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	buffer_vulkan->handle = vk_buffer;
	buffer_vulkan->device_memory = vk_device_memory;
	buffer_vulkan->public_.memory_location = memory_location;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_buffer_init(HeroLogicalDevice* ldev, HeroBufferSetup* setup, HeroBufferId* id_out, HeroBuffer** out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroBufferVulkan* buffer_vulkan;
	result = hero_object_pool(HeroBufferVulkan, alloc)(&ldev_vulkan->buffer_pool, 0, &buffer_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	buffer_vulkan->public_.type = setup->type;
	buffer_vulkan->public_.flags = setup->flags;
	buffer_vulkan->public_.memory_location = setup->memory_location;
	buffer_vulkan->public_.queue_support_flags = setup->queue_support_flags;
	buffer_vulkan->public_.elmts_count = setup->elmts_count;
	buffer_vulkan->public_.elmt_size = setup->elmt_size;

	VkDeviceSize new_size = setup->elmts_count * setup->elmt_size;
	result = _hero_vulkan_buffer_reinit(ldev, buffer_vulkan, new_size);
	if (result < 0) {
		return result;
	}

	*out = &buffer_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_buffer_deinit(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer* buffer) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroBufferVulkan* buffer_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroBufferVulkan, buffer);

	ldev_vulkan->vkDestroyBuffer(ldev_vulkan->handle, buffer_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);
	result = _hero_vulkan_device_memory_dealloc(ldev_vulkan, buffer_vulkan->device_memory);
	if (result < 0) {
		return result;
	}

	return hero_object_pool(HeroBufferVulkan, dealloc)(&ldev_vulkan->buffer_pool, id);
}

HeroResult _hero_vulkan_buffer_get(HeroLogicalDevice* ldev, HeroBufferId id, HeroBuffer** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroBufferVulkan* buffer_vulkan;
	result = hero_object_pool(HeroBufferVulkan, get)(&ldev_vulkan->buffer_pool, id, &buffer_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &buffer_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_buffer_resize(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 elmts_count) {
	HeroResult result;
	HeroBufferVulkan* buffer_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroBufferVulkan, buffer);

	VkDeviceSize new_size = elmts_count * buffer->elmt_size;
	return _hero_vulkan_buffer_reinit(ldev, buffer_vulkan, new_size);
}

HeroResult _hero_vulkan_buffer_map(HeroLogicalDevice* ldev, HeroBuffer* buffer, void** addr_out) {
	HERO_ABORT("unimplemented");
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_buffer_read(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 start_idx, Uptr elmts_count, void* destination) {
	HERO_ABORT("unimplemented");
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_buffer_write(HeroLogicalDevice* ldev, HeroBuffer* buffer, U64 start_idx, Uptr elmts_count, void** destination_out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroBufferVulkan* buffer_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroBufferVulkan, buffer);

	_HeroGfxStagedUpdateData data;
	data.buffer.dst_buffer = buffer_vulkan->handle;
	data.buffer.dst_offset = start_idx * (U64)buffer_vulkan->public_.elmt_size;
	data.buffer.size = (U64)elmts_count * (U64)buffer_vulkan->public_.elmt_size;

	result = _hero_vulkan_stage_buffer_update(ldev_vulkan, false, &data, destination_out);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_image_reinit(HeroLogicalDevice* ldev, HeroImageVulkan* image_vulkan) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (image_vulkan->handle != VK_NULL_HANDLE) {
		result = _hero_vulkan_device_memory_dealloc(ldev_vulkan, image_vulkan->device_memory);
		if (result < 0) {
			return result;
		}

		ldev_vulkan->vkDestroyImageView(ldev_vulkan->handle, image_vulkan->view_handle, HERO_VULKAN_TODO_ALLOCATOR);
		ldev_vulkan->vkDestroyImage(ldev_vulkan->handle, image_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

		image_vulkan->handle = VK_NULL_HANDLE;
		image_vulkan->view_handle = VK_NULL_HANDLE;
	}

	HeroImageFlags flags = image_vulkan->public_.flags;

	VkImageCreateFlags vk_create_flags = 0;
	if (image_vulkan->public_.type == HERO_IMAGE_TYPE_CUBE || image_vulkan->public_.type == HERO_IMAGE_TYPE_CUBE_ARRAY) {
		vk_create_flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImageUsageFlags vk_usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (flags & HERO_IMAGE_FLAGS_SAMPLED) vk_usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (flags & HERO_IMAGE_FLAGS_STORAGE) vk_usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (flags & HERO_IMAGE_FLAGS_COLOR_ATTACHMENT) vk_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (flags & HERO_IMAGE_FLAGS_DEPTH_STENCIL_ATTACHMENT) vk_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (flags & HERO_IMAGE_FLAGS_INPUT_ATTACHMENT) vk_usage_flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

	VkQueueFlags queue_flags = VK_QUEUE_TRANSFER_BIT;
	if (flags & HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS) queue_flags |= VK_QUEUE_GRAPHICS_BIT;
	if (flags & HERO_IMAGE_FLAGS_USED_FOR_COMPUTE) queue_flags |= VK_QUEUE_COMPUTE_BIT;

	U32 vk_queue_family_indices[HERO_PHYSICAL_DEVICE_VULKAN_QUEUES_CAP];
	U16 vk_queue_family_indices_count;
	_hero_vulkan_queue_family_indices_get(ldev_vulkan, image_vulkan->public_.queue_support_flags, vk_queue_family_indices, &vk_queue_family_indices_count);

	VkSharingMode sharing_mode;
	if (vk_queue_family_indices_count > 1) {
		sharing_mode = VK_SHARING_MODE_CONCURRENT;
	} else {
		sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
		vk_queue_family_indices_count = 0;
	}

	VkImageCreateInfo vk_image_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = vk_create_flags,
		.imageType = _hero_vulkan_convert_to_image_type[image_vulkan->public_.type],
		.format = _hero_vulkan_convert_to_format[image_vulkan->public_.internal_format],
		.extent = (VkExtent3D) { image_vulkan->public_.width, image_vulkan->public_.height, image_vulkan->public_.depth },
		.mipLevels = image_vulkan->public_.mip_levels,
		.arrayLayers = image_vulkan->public_.array_layers_count,
		.samples = image_vulkan->public_.samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = vk_usage_flags,
		.sharingMode = sharing_mode,
		.queueFamilyIndexCount = vk_queue_family_indices_count,
		.pQueueFamilyIndices = vk_queue_family_indices,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VkImage vk_image;
	vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_image);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkDeviceMemory vk_device_memory;
	HeroMemoryLocation memory_location = image_vulkan->public_.memory_location;
	{
		VkMemoryRequirements mem_req;
		ldev_vulkan->vkGetImageMemoryRequirements(ldev_vulkan->handle, vk_image, &mem_req);

		_HeroVulkanAllocSetup alloc_setup = {
			.size = mem_req.size,
			.align = mem_req.alignment,
			.memory_type_bits = mem_req.memoryTypeBits,
			.memory_location = memory_location,
			.type = _HERO_VULKAN_ALLOC_TYPE_IMAGE,
		};

		result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &vk_device_memory);
		if (result < 0) {
			return result;
		}

		memory_location = alloc_setup.memory_location;
	}

	VkDeviceSize vk_device_memory_offset = 0;
	vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, vk_image, vk_device_memory, vk_device_memory_offset);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkImageAspectFlags aspect_mask = 0;
	switch (image_vulkan->public_.format) {
		case HERO_IMAGE_FORMAT_S8:
			aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case HERO_IMAGE_FORMAT_D16_S8:
		case HERO_IMAGE_FORMAT_D24_S8:
		case HERO_IMAGE_FORMAT_D32_S8:
			aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		case HERO_IMAGE_FORMAT_D16:
		case HERO_IMAGE_FORMAT_D32:
			aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		default:
			aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;
	}

	VkImageViewCreateInfo vk_image_view_create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = vk_image,
		.viewType = _hero_vulkan_convert_to_image_view_type[image_vulkan->public_.type],
		.format = _hero_vulkan_convert_to_format[image_vulkan->public_.format],
		.components = {
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A,
		},
		.subresourceRange = {
			.aspectMask = aspect_mask,
			.baseMipLevel = 0,
			.levelCount = image_vulkan->public_.mip_levels,
			.baseArrayLayer = 0,
			.layerCount = image_vulkan->public_.array_layers_count,
		},
	};

	VkImageView vk_image_view;
	vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_image_view);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}


	image_vulkan->handle = vk_image;
	image_vulkan->view_handle = vk_image_view;
	image_vulkan->device_memory = vk_device_memory;
	image_vulkan->public_.memory_location = memory_location;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_image_init(HeroLogicalDevice* ldev, HeroImageSetup* setup, HeroImageId* id_out, HeroImage** out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroImageVulkan* image_vulkan;
	result = hero_object_pool(HeroImageVulkan, alloc)(&ldev_vulkan->image_pool, 0, &image_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	image_vulkan->public_.type = setup->type;
	image_vulkan->public_.format = setup->format;
	image_vulkan->public_.internal_format = setup->internal_format;
	image_vulkan->public_.flags = setup->flags;
	image_vulkan->public_.samples = setup->samples;
	image_vulkan->public_.memory_location = setup->memory_location;
	image_vulkan->public_.queue_support_flags = setup->queue_support_flags;
	image_vulkan->public_.width = setup->width;
	image_vulkan->public_.height = setup->height;
	image_vulkan->public_.depth = setup->depth;
	image_vulkan->public_.mip_levels = setup->mip_levels;
	image_vulkan->public_.array_layers_count = setup->array_layers_count;

	result = _hero_vulkan_image_reinit(ldev, image_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &image_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_image_deinit(HeroLogicalDevice* ldev, HeroImageId id, HeroImage* image) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroImageVulkan* image_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroImageVulkan, image);

	ldev_vulkan->vkDestroyImage(ldev_vulkan->handle, image_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);
	result = _hero_vulkan_device_memory_dealloc(ldev_vulkan, image_vulkan->device_memory);
	if (result < 0) {
		return result;
	}

	return hero_object_pool(HeroImageVulkan, dealloc)(&ldev_vulkan->image_pool, id);
}

HeroResult _hero_vulkan_image_get(HeroLogicalDevice* ldev, HeroImageId id, HeroImage** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroImageVulkan* image_vulkan;
	result = hero_object_pool(HeroImageVulkan, get)(&ldev_vulkan->image_pool, id, &image_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &image_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_image_resize(HeroLogicalDevice* ldev, HeroImage* image, U32 width, U32 height, U32 depth, U32 mip_levels, U32 array_layers_count) {
	HeroResult result;
	HeroImageVulkan* image_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroImageVulkan, image);

	return _hero_vulkan_image_reinit(ldev, image_vulkan);
}

HeroResult _hero_vulkan_image_map(HeroLogicalDevice* ldev, HeroImage* image, void** addr_out) {
	HERO_ABORT("unimplemented");

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_image_read(HeroLogicalDevice* ldev, HeroImage* image, HeroImageArea* area, void* destination) {
	HERO_ABORT("unimplemented");

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_image_write(HeroLogicalDevice* ldev, HeroImage* image, HeroImageArea* area, void** destination_out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroImageVulkan* image_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroImageVulkan, image);

	_HeroGfxStagedUpdateData data;
	data.image.dst_image = image_vulkan->handle;
	data.image.format = image_vulkan->public_.internal_format;
	data.image.area = *area;
	data.image.layout = (image_vulkan->public_.flags & HERO_IMAGE_FLAGS_STORAGE)
		? VK_IMAGE_LAYOUT_GENERAL
		: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	result = _hero_vulkan_stage_buffer_update(ldev_vulkan, true, &data, destination_out);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_sampler_init(HeroLogicalDevice* ldev, HeroSamplerSetup* setup, HeroSamplerId* id_out, HeroSampler** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroSamplerVulkan* sampler_vulkan;
	result = hero_object_pool(HeroSamplerVulkan, alloc)(&ldev_vulkan->sampler_pool, 0, &sampler_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	VkSamplerCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = _hero_vulkan_convert_to_filter[setup->mag_filter],
		.minFilter = _hero_vulkan_convert_to_filter[setup->min_filter],
		.mipmapMode = _hero_vulkan_convert_to_sampler_mipmap_mode[setup->mipmap_mode],
		.addressModeU = _hero_vulkan_convert_to_sampler_address_mode[setup->address_mode_u],
		.addressModeV = _hero_vulkan_convert_to_sampler_address_mode[setup->address_mode_v],
		.addressModeW = _hero_vulkan_convert_to_sampler_address_mode[setup->address_mode_w],
		.mipLodBias = setup->mip_lod_bias,
		.anisotropyEnable = (setup->flags & HERO_SAMPLER_SETUP_FLAGS_ANISOTROPY_ENABLE) != 0,
		.maxAnisotropy = setup->max_anisotropy,
		.compareEnable = (setup->flags & HERO_SAMPLER_SETUP_FLAGS_COMPARE_ENABLE) != 0,
		.compareOp = _hero_vulkan_convert_to_compare_op[setup->compare_op],
		.minLod = setup->min_lod,
		.maxLod = setup->max_lod,
		.borderColor = _hero_vulkan_convert_to_border_color[setup->border_color],
		.unnormalizedCoordinates = (setup->flags & HERO_SAMPLER_SETUP_FLAGS_UNNORMALIZED_COORDINATES) != 0,
	};

	VkSampler vk_sampler;
	vk_result = ldev_vulkan->vkCreateSampler(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_sampler);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	sampler_vulkan->handle = vk_sampler;
	*out = &sampler_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_sampler_deinit(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler* sampler) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroSamplerVulkan* sampler_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroSamplerVulkan, sampler);

	ldev_vulkan->vkDestroySampler(ldev_vulkan->handle, sampler_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroSamplerVulkan, dealloc)(&ldev_vulkan->sampler_pool, id);
}

HeroResult _hero_vulkan_sampler_get(HeroLogicalDevice* ldev, HeroSamplerId id, HeroSampler** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroSamplerVulkan* sampler_vulkan;
	result = hero_object_pool(HeroSamplerVulkan, get)(&ldev_vulkan->sampler_pool, id, &sampler_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &sampler_vulkan->public_;
	return HERO_SUCCESS;
}


HeroResult _hero_vulkan_shader_module_init(HeroLogicalDevice* ldev, HeroShaderModuleSetup* setup, HeroShaderModuleId* id_out, HeroShaderModule** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (setup->format != HERO_SHADER_FORMAT_SPIR_V) {
		return HERO_ERROR(GFX_SHADER_FORMAT_INCOMPATIBLE);
	}

	VkShaderModuleCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = setup->code_size,
		.pCode = (U32*)setup->code,
	};

	VkShaderModule vk_shader_module;
	vk_result = ldev_vulkan->vkCreateShaderModule(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_shader_module);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroShaderModuleVulkan* shader_module_vulkan;
	result = hero_object_pool(HeroShaderModuleVulkan, alloc)(&ldev_vulkan->shader_module_pool, 0, &shader_module_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	shader_module_vulkan->handle = vk_shader_module;
	*out = &shader_module_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_module_deinit(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule* shader_module) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroShaderModuleVulkan* shader_module_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroShaderModuleVulkan, shader_module);

	ldev_vulkan->vkDestroyShaderModule(ldev_vulkan->handle, shader_module_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroShaderModuleVulkan, dealloc)(&ldev_vulkan->shader_module_pool, id);
}

HeroResult _hero_vulkan_shader_module_get(HeroLogicalDevice* ldev, HeroShaderModuleId id, HeroShaderModule** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroShaderModuleVulkan* shader_module_vulkan;
	result = hero_object_pool(HeroShaderModuleVulkan, get)(&ldev_vulkan->shader_module_pool, id, &shader_module_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &shader_module_vulkan->public_;
	return HERO_SUCCESS;
}

int descriptor_binding_sort_fn(const void* a, const void* b) {
	const HeroSpirVDescriptorBinding* a_ = a;
	const HeroSpirVDescriptorBinding* b_ = b;

	if (a_->set < b_->set) {
		return -1;
	} else if (a_->set > b_->set) {
		return 1;
	} else {
		if (a_->binding < b_->binding) {
			return -1;
		} else if (a_->binding > b_->binding) {
			return 1;
		} else {
			return 0;
		}
	}
}

HeroResult _hero_vulkan_shader_metadata_calculate(HeroLogicalDevice* ldev, HeroShaderMetadataSetup* setup, HeroShaderMetadata** out) {
	HeroResult result;
	_HeroSpirVInspect inspect = {0};

	HeroShaderModuleId unique_module_ids[HERO_SHADER_STAGES_COUNT] = {0};
	U32 unique_module_ids_count = 0;

	HeroShaderStages* stages = setup->stages;
	U32 shader_stages_count = HERO_SHADER_TYPE_STAGES_COUNTS[stages->type];
	for (U32 i = 0; i < shader_stages_count; i += 1) {
		HeroShaderModuleId shader_module_id = stages->data.array[i].module_id;
		if (shader_module_id.raw == 0) {
			continue;
		}

		Uptr module_idx = 0;
		for (; module_idx < unique_module_ids_count; module_idx += 1) {
			if (unique_module_ids[module_idx].raw == shader_module_id.raw) {
				break;
			}
		}

		if (module_idx != unique_module_ids_count) {
			continue;
		}

		unique_module_ids[unique_module_ids_count] = shader_module_id;
		unique_module_ids_count += 1;

		HeroShaderModule* shader_module;
		result = hero_shader_module_get(ldev, shader_module_id, &shader_module);
		HERO_RESULT_ASSERT(result);

		result = _hero_vulkan_shader_metadata_spir_v_inspect((U32*)shader_module->code, shader_module->code_size, &inspect);
		HERO_RESULT_ASSERT(result);
	}

	Uptr metadata_size = 0;

	HeroShaderMetadataAllocSetup alloc_setup = {
		.spir_v = {
			.vertex_attribs_count = inspect.vertex_attribs.count,
			.descriptor_bindings_counts = {0},
		},
	};

	Uptr hash_table_idx = 0;
	HeroHashTableEntry(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor)* entry;
	while (hero_hash_table(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor, iter_next)(&inspect.key_to_descriptor_map, &hash_table_idx, &entry) != HERO_SUCCESS_FINISHED) {
		alloc_setup.spir_v.descriptor_bindings_counts[entry->key.set] += 1;
	}

	HeroShaderMetadata* metadata = hero_shader_metadata_alloc(&alloc_setup);
	if (metadata == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	HeroSpirVVertexAttribInfo* vertex_attribs = hero_shader_metadata_spir_v_vertex_attribs(metadata);
	HERO_COPY_ELMT_MANY(vertex_attribs, inspect.vertex_attribs.data, inspect.vertex_attribs.count);

	HeroSpirVDescriptorBinding* descriptor_bindings = hero_shader_metadata_spir_v_descriptor_bindings(metadata);
	U32 i = 0;
	hash_table_idx = 0;
	while (hero_hash_table(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor, iter_next)(&inspect.key_to_descriptor_map, &hash_table_idx, &entry) != HERO_SUCCESS_FINISHED) {
		descriptor_bindings[i] = entry->value.binding;
		descriptor_bindings[i].set = entry->key.set;
		i += 1;
	}

	qsort(descriptor_bindings, inspect.key_to_descriptor_map.count, sizeof(HeroSpirVDescriptorBinding), descriptor_binding_sort_fn);

	metadata->spir_v.push_constants_size = inspect.push_constants_size;

	*out = metadata;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_init(HeroLogicalDevice* ldev, HeroShaderSetup* setup, HeroShaderId* id_out, HeroShader** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroShaderStages* stages = setup->stages;
	U8 stages_count = HERO_SHADER_TYPE_STAGES_COUNTS[stages->type];
	for (U32 i = 0; i < stages_count; i += 1) {
		HeroShaderModuleId shader_module_id = stages->data.array[i].module_id;
		if (shader_module_id.raw == 0) {
			continue;
		}

		HeroShaderModule* shader_module;
		result = hero_shader_module_get(ldev, shader_module_id, &shader_module);
		if (result < 0) {
			return result;
		}

		if (shader_module->format != HERO_SHADER_FORMAT_NONE && shader_module->format != HERO_SHADER_FORMAT_SPIR_V) {
			return HERO_ERROR(GFX_SHADER_FORMAT_INCOMPATIBLE);
		}
	}

	HeroShaderMetadata* metadata = setup->metadata;

	VkDescriptorSetLayout descriptor_set_layouts[HERO_GFX_DESCRIPTOR_SET_COUNT];
	HeroSpirVDescriptorBinding* descriptor_bindings = hero_shader_metadata_spir_v_descriptor_bindings(metadata);
	U32 descriptor_bindings_count = hero_shader_metadata_spir_v_descriptor_bindings_count(metadata);
	VkDescriptorUpdateTemplateEntry* update_template_entries = hero_alloc_array(VkDescriptorUpdateTemplateEntry, hero_system_alctor, 0, descriptor_bindings_count);
	if (!update_template_entries) {
		return HERO_ERROR(GFX_OUT_OF_MEMORY_DEVICE);
	}
	U32* binding_update_data_indices = hero_alloc_array(U32, hero_system_alctor, 0, descriptor_bindings_count);
	if (!update_template_entries) {
		return HERO_ERROR(GFX_OUT_OF_MEMORY_DEVICE);
	}
	U32 descriptors_counts[HERO_GFX_DESCRIPTOR_SET_COUNT] = {0};
	U32 descriptors_count = 0;
	if (descriptor_bindings_count == 0) {
		for_range(i, 0, HERO_GFX_DESCRIPTOR_SET_COUNT) {
			descriptor_set_layouts[i] = ldev_vulkan->null_descriptor_set_layout;
		}
	} else {
		VkDescriptorSetLayoutBinding* vk_bindings = hero_alloc_array(VkDescriptorSetLayoutBinding, hero_system_alctor, 0, descriptor_bindings_count);
		if (!vk_bindings) {
			return HERO_ERROR(GFX_OUT_OF_MEMORY_DEVICE);
		}

		U32 offset = 0;
		U32 prev_set = 0;
		for (U32 i = 0; i < descriptor_bindings_count; i += 1) {
			HeroSpirVDescriptorBinding* binding = &descriptor_bindings[i];
			VkDescriptorSetLayoutBinding* vk_binding = &vk_bindings[i];
			vk_binding->binding = binding->binding;
			vk_binding->descriptorType = _hero_vulkan_convert_to_descriptor_type[binding->descriptor_type];
			vk_binding->descriptorCount = binding->descriptor_count;
			vk_binding->pImmutableSamplers = NULL;
			vk_binding->stageFlags = binding->stage_flags;

			if (binding->set != prev_set) {
				offset = 0;
				prev_set = binding->set;
			}

			VkDescriptorUpdateTemplateEntry* update_template_entry = &update_template_entries[i];
			update_template_entry->dstBinding = binding->binding;
			update_template_entry->dstArrayElement = 0;
			update_template_entry->descriptorCount = binding->descriptor_count;
			update_template_entry->descriptorType = _hero_vulkan_convert_to_descriptor_type[binding->descriptor_type];
			update_template_entry->offset = offset;
			update_template_entry->stride = sizeof(HeroDescriptorUpdateDataVulkan);

			descriptors_counts[binding->set] += binding->descriptor_count;
			descriptors_count += binding->descriptor_count;
			binding_update_data_indices[i] = offset / sizeof(HeroDescriptorUpdateDataVulkan);
			offset += sizeof(HeroDescriptorUpdateDataVulkan) * binding->descriptor_count;
		}

		VkDescriptorSetLayoutCreateInfo vk_set_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.pBindings = vk_bindings,
		};

		for (U32 i = 0; i < HERO_GFX_DESCRIPTOR_SET_COUNT; i += 1) {
			U32 count = metadata->spir_v.descriptor_bindings_counts[i];
			VkDescriptorSetLayout vk_descriptor_set_layout;
			if (count == 0) {
				vk_descriptor_set_layout = ldev_vulkan->null_descriptor_set_layout;
			} else {
				vk_set_layout_create_info.bindingCount = count;
				vk_result = ldev_vulkan->vkCreateDescriptorSetLayout(ldev_vulkan->handle, &vk_set_layout_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_descriptor_set_layout);
				if (vk_result < 0) {
					return _hero_vulkan_convert_from_result(vk_result);
				}

				vk_set_layout_create_info.pBindings += count;
			}

			descriptor_set_layouts[i] = vk_descriptor_set_layout;
		}
	}

	VkPipelineLayout vk_pipeline_layout;
	{
		VkPushConstantRange vk_push_constant_range = {
			.offset = 0,
			.size = metadata->spir_v.push_constants_size,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS, // TODO get from shader metadata
		};

		VkPipelineLayoutCreateInfo vk_create_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.setLayoutCount = HERO_GFX_DESCRIPTOR_SET_COUNT,
			.pSetLayouts = descriptor_set_layouts,
			.pushConstantRangeCount = metadata->spir_v.push_constants_size ? 1 : 0,
			.pPushConstantRanges = metadata->spir_v.push_constants_size ? &vk_push_constant_range : NULL,
		};

		vk_result = ldev_vulkan->vkCreatePipelineLayout(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_pipeline_layout);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	VkDescriptorUpdateTemplate descriptor_update_templates[HERO_GFX_DESCRIPTOR_SET_COUNT] = {0};
	if (descriptor_bindings_count) {
		VkDescriptorUpdateTemplateCreateInfo update_template_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.pDescriptorUpdateEntries = update_template_entries,
			.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.pipelineLayout = vk_pipeline_layout,
		};

		for (U32 i = 0; i < HERO_GFX_DESCRIPTOR_SET_COUNT; i += 1) {
			U32 count = metadata->spir_v.descriptor_bindings_counts[i];
			if (count == 0) {
				continue;
			}
			VkDescriptorSetLayout vk_descriptor_set_layout = descriptor_set_layouts[i];

			update_template_create_info.descriptorUpdateEntryCount = count;
			update_template_create_info.descriptorSetLayout = vk_descriptor_set_layout;
			update_template_create_info.set = i;

			VkDescriptorUpdateTemplate vk_descriptor_update_template;
			vk_result = ldev_vulkan->vkCreateDescriptorUpdateTemplate(ldev_vulkan->handle, &update_template_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_descriptor_update_template);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			update_template_create_info.pDescriptorUpdateEntries += count;
			descriptor_update_templates[i] = vk_descriptor_update_template;
		}
	}

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	if (stages->type == HERO_SHADER_TYPE_COMPUTE) {
		HeroShaderModuleVulkan* shader_module_vulkan;
		result = hero_object_pool(HeroShaderModuleVulkan, get)(&ldev_vulkan->shader_module_pool, stages->data.compute.compute.module_id, &shader_module_vulkan);
		if (result < 0) {
			return result;
		}

		VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
		if (stages->data.compute.cache_id.raw) {
			HeroPipelineCacheVulkan* pipeline_cache_vulkan;
			result = hero_object_pool(HeroPipelineCacheVulkan, get)(&ldev_vulkan->pipeline_cache_pool, stages->data.compute.cache_id, &pipeline_cache_vulkan);
			if (result < 0) {
				return result;
			}

			vk_pipeline_cache = pipeline_cache_vulkan->handle;
		}

		VkComputePipelineCreateInfo vk_create_info = {
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.stage = {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.stage = VK_SHADER_STAGE_COMPUTE_BIT,
				.module = shader_module_vulkan->handle,
				.pName = stages->data.compute.compute.backend.spir_v.entry_point_name,
				.pSpecializationInfo = NULL,
			},
			.layout = vk_pipeline_layout,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = 0,
		};

		vk_result = ldev_vulkan->vkCreateComputePipelines(ldev_vulkan->handle, vk_pipeline_cache, 1, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_pipeline);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, alloc)(&ldev_vulkan->shader_pool, 0, &shader_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	shader_vulkan->pipeline_layout = vk_pipeline_layout;
	shader_vulkan->compute_pipeline = vk_pipeline;
	HERO_COPY_ARRAY(shader_vulkan->descriptor_set_layouts, descriptor_set_layouts);
	HERO_COPY_ARRAY(shader_vulkan->descriptor_update_templates, descriptor_update_templates);
	HERO_COPY_ARRAY(shader_vulkan->descriptors_counts, descriptors_counts);
	shader_vulkan->descriptor_binding_update_data_indices = binding_update_data_indices;
	*out = &shader_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_deinit(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader* shader) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroShaderVulkan* shader_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroShaderVulkan, shader);

	ldev_vulkan->vkDestroyPipelineLayout(ldev_vulkan->handle, shader_vulkan->pipeline_layout, HERO_VULKAN_TODO_ALLOCATOR);
	for (U32 i = 0; i < HERO_GFX_DESCRIPTOR_SET_COUNT; i += 1) {
		ldev_vulkan->vkDestroyDescriptorSetLayout(ldev_vulkan->handle, shader_vulkan->descriptor_set_layouts[i], HERO_VULKAN_TODO_ALLOCATOR);
	}

	return hero_object_pool(HeroShaderVulkan, dealloc)(&ldev_vulkan->shader_pool, id);
}

HeroResult _hero_vulkan_shader_get(HeroLogicalDevice* ldev, HeroShaderId id, HeroShader** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, id, &shader_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &shader_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_pool_init(HeroLogicalDevice* ldev, HeroDescriptorPoolSetup* setup, HeroDescriptorPoolId* id_out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	U32 layouts_count = setup->shader_infos_count * HERO_GFX_DESCRIPTOR_SET_COUNT; // TODO deduplicate VkDescriptorSetLayout
	_HeroDescriptorSetLayoutAuxVulkan* layout_auxs = hero_alloc_array(_HeroDescriptorSetLayoutAuxVulkan, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS, layouts_count);
	if (layout_auxs == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	HERO_ZERO_ELMT_MANY(layout_auxs, layouts_count);

	VkDescriptorSetLayout* layouts = hero_alloc_array(VkDescriptorSetLayout, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUTS, layouts_count);
	if (layouts == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}


	//
	// loop over all shaders and do the following:
	// - use their metadata to know how many descriptors we want per VkDescriptorType
	// - fill up the layouts array with VkDescriptorSetLayout
	// - initialize the layout_auxs array
	// - accumulate the pool_max_sets
	U32 type_sizes[HERO_DESCRIPTOR_TYPE_COUNT] = {0};
	U32 vk_pool_sizes_count = 0;
	U32 layout_idx = 0;
	U32 pool_max_sets = 0;
	for (U32 i = 0; i < setup->shader_infos_count; i += 1) {
		HeroDescriptorShaderInfo* shader_info = &setup->shader_infos[i];

		HeroShaderVulkan* shader_vulkan;
		result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, shader_info->shader_id, &shader_vulkan);
		if (result < 0) {
			return result;
		}
		HeroShaderMetadata* metadata = shader_vulkan->public_.metadata;

		U32 binding_idx = 0;
		HeroSpirVDescriptorBinding* descriptor_bindings = hero_shader_metadata_spir_v_descriptor_bindings(metadata);
		for (U32 set_idx = 0; set_idx < HERO_GFX_DESCRIPTOR_SET_COUNT; set_idx += 1) {
			U32 advised_pool_count = shader_info->advised_pool_counts[set_idx];
			if (advised_pool_count == 0) {
				return HERO_ERROR(GFX_DESCRIPTOR_POOL_ADVISED_POOL_COUNT_CANNOT_BE_ZERO);
			}

			layout_auxs[layout_idx].cap_per_pool = advised_pool_count;
			layout_auxs[layout_idx].last_frame_idx_for_free_sets_in_use = ldev->last_submitted_frame_idx;
			layouts[layout_idx] = shader_vulkan->descriptor_set_layouts[set_idx];
			pool_max_sets += advised_pool_count;

			//
			// use the metadata to know how many descriptors we want per VkDescriptorType
			U32 set_bindings_count = metadata->spir_v.descriptor_bindings_counts[set_idx];
			for (U32 j = 0; j < set_bindings_count; j += 1) {
				HeroSpirVDescriptorBinding* binding = &descriptor_bindings[binding_idx];
				HERO_ASSERT_ARRAY_BOUNDS(binding->descriptor_type, HERO_DESCRIPTOR_TYPE_COUNT);
				if (type_sizes[binding->descriptor_type] == 0) {
					vk_pool_sizes_count += 1;
				}
				type_sizes[binding->descriptor_type] += binding->descriptor_count * advised_pool_count;
			}
			layout_idx += 1;
			binding_idx += 1;
		}
	}

	VkDescriptorPoolSize* vk_pool_sizes = hero_alloc_array(VkDescriptorPoolSize, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOL_SIZES, vk_pool_sizes_count);
	for (VkDescriptorType type = 0, dst_idx = 0; type < HERO_DESCRIPTOR_TYPE_COUNT; type += 1) {
		U32 count = type_sizes[type];
		if (count) {
			vk_pool_sizes[dst_idx].type = _hero_vulkan_convert_to_descriptor_type[type];
			vk_pool_sizes[dst_idx].descriptorCount = count;
			dst_idx += 1;
		}
	}

	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	result = hero_object_pool(HeroDescriptorPoolVulkan, alloc)(&ldev_vulkan->descriptor_pool_pool, 0, &descriptor_pool_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	descriptor_pool_vulkan->layout_auxs = layout_auxs;
	descriptor_pool_vulkan->layouts = layouts;
	descriptor_pool_vulkan->layouts_count = layouts_count;
	descriptor_pool_vulkan->pool_sizes = vk_pool_sizes;
	descriptor_pool_vulkan->pool_sizes_count = vk_pool_sizes_count;
	descriptor_pool_vulkan->pool_max_sets = pool_max_sets;

	return HERO_SUCCESS;

}

HeroResult _hero_vulkan_descriptor_pool_reset_now(HeroLogicalDeviceVulkan* ldev_vulkan, HeroDescriptorPoolVulkan* descriptor_pool_vulkan) {
	HeroResult result;

	//
	// deallocate all of the pools we have made
	for (U32 i = 0; i < descriptor_pool_vulkan->pools.count; i += 1) {
		_HeroDescriptorPoolVulkan* pool = &descriptor_pool_vulkan->pools.data[i];

		hero_dealloc_array(U32, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOL_FREE_COUNTS, pool->layout_free_counts, descriptor_pool_vulkan->layouts_count);

		HeroVulkanObjectDealloc* dealloc;
		result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, VK_OBJECT_TYPE_DESCRIPTOR_POOL, 1, &dealloc);
		if (result < 0) {
			return result;
		}
		dealloc->descriptor_pool = pool->handle;
	}
	descriptor_pool_vulkan->pools.count = 0;

	//
	// reset all of the auxillary data for the layouts
	for (U32 i = 0; i < descriptor_pool_vulkan->layouts_count; i += 1) {
		_HeroDescriptorSetLayoutAuxVulkan* layout_aux = &descriptor_pool_vulkan->layout_auxs[i];
		layout_aux->free_sets.count = 0;
		layout_aux->free_sets_in_use.count = 0;
		layout_aux->free_pool_idx = 0;
	}

	//
	// increment the reset counter so we can throw away old data
	descriptor_pool_vulkan->reset_counter += 1;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_pool_find_layout_idx(HeroDescriptorPoolVulkan* descriptor_pool_vulkan, VkDescriptorSetLayout layout, U32* layout_idx_out) {
	for_range(i, 0, descriptor_pool_vulkan->layouts_count) {
		if (descriptor_pool_vulkan->layouts[i] == layout) {
			*layout_idx_out = i;
			return HERO_SUCCESS;
		}
	}

	return HERO_ERROR(DOES_NOT_EXIST);
}

HeroResult _hero_vulkan_descriptor_pool_deinit(HeroLogicalDevice* ldev, HeroDescriptorPoolId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, id, &descriptor_pool_vulkan);
	if (result < 0) {
		return result;
	}

	//
	// reset the allocator to see most of the memory
	result = _hero_vulkan_descriptor_pool_reset_now(ldev_vulkan, descriptor_pool_vulkan);
	if (result < 0) {
		return result;
	}

	//
	// free the dynamic arrays in the auxillary layout data
	for (U32 i = 0; i < descriptor_pool_vulkan->layouts_count; i += 1) {
		_HeroDescriptorSetLayoutAuxVulkan* layout_aux = &descriptor_pool_vulkan->layout_auxs[i];

		hero_stack(VkDescriptorSet, deinit)(&layout_aux->free_sets, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS);
		hero_stack(VkDescriptorSet, deinit)(&layout_aux->free_sets_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS_IN_USE);
	}

	//
	// free the dynamic arrays in the pool
	hero_stack(_HeroDescriptorPoolVulkan, deinit)(&descriptor_pool_vulkan->pools, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOLS);
	hero_dealloc_array(_HeroDescriptorSetLayoutAuxVulkan, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS, descriptor_pool_vulkan->layout_auxs, descriptor_pool_vulkan->layouts_count);
	hero_dealloc_array(VkDescriptorSetLayout, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUTS, descriptor_pool_vulkan->layouts, descriptor_pool_vulkan->layouts_count);

	return hero_object_pool(HeroDescriptorPoolVulkan, dealloc)(&ldev_vulkan->descriptor_pool_pool, id);
}

HeroResult _hero_vulkan_descriptor_pool_reset(HeroLogicalDevice* ldev, HeroDescriptorPoolId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, id, &descriptor_pool_vulkan);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_descriptor_pool_reset_now(ldev_vulkan, descriptor_pool_vulkan);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_pool_alloc(HeroLogicalDevice* ldev, HeroDescriptorPoolVulkan* descriptor_pool_vulkan, VkDescriptorSetLayout layout, VkDescriptorSet* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (HERO_UNLIKELY(layout == ldev_vulkan->null_descriptor_set_layout)) {
		*out = VK_NULL_HANDLE;
		return HERO_SUCCESS;
	}

	//
	// get the auxillary layout data for this descriptor set layout.
	U32 layout_idx;
	result = _hero_vulkan_descriptor_pool_find_layout_idx(descriptor_pool_vulkan, layout, &layout_idx);
	if (result < 0) {
		return result;
	}
	_HeroDescriptorSetLayoutAuxVulkan* layout_aux = &descriptor_pool_vulkan->layout_auxs[layout_idx];

	//
	// if we have a free set, pop it off the stack and return that.
	if (layout_aux->free_sets.count) {
		layout_aux->free_sets.count -= 1;

		VkDescriptorSet vk_descriptor_set = layout_aux->free_sets.data[layout_aux->free_sets.count];
		*out = vk_descriptor_set;
		return HERO_SUCCESS;
	}

	//
	// create a new pool (that is used by all layouts) if we have no more sets available
	// for this layout in the existing pools
	if (layout_aux->free_pool_idx == descriptor_pool_vulkan->pools.count) {
		_HeroDescriptorPoolVulkan* pool;
		result = hero_stack(_HeroDescriptorPoolVulkan, push)(&descriptor_pool_vulkan->pools, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOLS, &pool);
		if (result < 0) {
			return result;
		}

		//
		// allocate the pool's free counts for each layout and initialize them
		U32* layout_free_counts = hero_alloc_array(U32, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_POOL_FREE_COUNTS, descriptor_pool_vulkan->layouts_count);
		if (layout_free_counts == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		for (U32 i = 0; i < descriptor_pool_vulkan->layouts_count; i += 1) {
			layout_free_counts[i] = descriptor_pool_vulkan->layout_auxs[i].cap_per_pool;
		}

		VkDescriptorPoolCreateInfo vk_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.maxSets = descriptor_pool_vulkan->pool_max_sets,
			.poolSizeCount = descriptor_pool_vulkan->pool_sizes_count,
			.pPoolSizes = descriptor_pool_vulkan->pool_sizes,
		};

		VkDescriptorPool vk_descriptor_pool;
		vk_result = ldev_vulkan->vkCreateDescriptorPool(ldev_vulkan->handle, &vk_pool_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_descriptor_pool);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		pool->handle = vk_descriptor_pool;
		pool->layout_free_counts = layout_free_counts;
	}


	_HeroDescriptorPoolVulkan* pool = hero_stack(_HeroDescriptorPoolVulkan, get)(&descriptor_pool_vulkan->pools, layout_aux->free_pool_idx);

	//
	// remove the free entry we are about to take and advance the free_pool_idx if we exhaust this pool.
	pool->layout_free_counts[layout_idx] -= 1;
	if (pool->layout_free_counts[layout_idx] == 0) {
		layout_aux->free_pool_idx += 1;
	}

	VkDescriptorSetAllocateInfo vk_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = pool->handle,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout,
	};

	VkDescriptorSet vk_descriptor_set;
	vk_result = ldev_vulkan->vkAllocateDescriptorSets(ldev_vulkan->handle, &vk_alloc_info, &vk_descriptor_set);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	*out = vk_descriptor_set;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_pool_dealloc(HeroLogicalDevice* ldev, HeroDescriptorPoolVulkan* descriptor_pool_vulkan, VkDescriptorSetLayout layout, VkDescriptorSet vk_descriptor_set, HeroGfxFrameIdx last_submitted_frame_idx) {
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (HERO_UNLIKELY(layout == ldev_vulkan->null_descriptor_set_layout)) {
		return HERO_SUCCESS;
	}

	//
	// get the auxillary layout data for this descriptor set layout.
	U32 layout_idx;
	HeroResult result = _hero_vulkan_descriptor_pool_find_layout_idx(descriptor_pool_vulkan, layout, &layout_idx);
	if (result < 0) {
		return result;
	}
	_HeroDescriptorSetLayoutAuxVulkan* layout_aux = &descriptor_pool_vulkan->layout_auxs[layout_idx];

	//
	// if the descriptor set is still in use the it goes in a different array.
	HeroStack(VkDescriptorSet)* free_sets;
	HeroAllocTag alloc_tag;
	if (HERO_GFX_FRAME_IDX_LESS_THAN_OR_EQUAL(last_submitted_frame_idx, ldev->last_completed_frame_idx)) {
		free_sets = &layout_aux->free_sets;
		alloc_tag = HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS;
	} else {
		free_sets = &layout_aux->free_sets_in_use;
		alloc_tag = HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS_IN_USE;
	}

	result = hero_stack(VkDescriptorSet, push_value)(free_sets, hero_system_alctor, alloc_tag, vk_descriptor_set);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_pool_free_used_resources(HeroLogicalDevice* ldev, HeroDescriptorPoolVulkan* descriptor_pool_vulkan) {
	HeroResult result;
	for (U32 i = 0; i < descriptor_pool_vulkan->layouts_count; i += 1) {
		_HeroDescriptorSetLayoutAuxVulkan* layout_aux = &descriptor_pool_vulkan->layout_auxs[i];
		if (layout_aux->free_sets_in_use.count == 0) {
			continue;
		}

		//
		// count how many sets that are no longer being used by the GPU
		// that we can move into the free_sets array
		U32 copy_count = 0;
		U32 remove_count = 0;
		for_range(j, 0, layout_aux->free_sets_in_use.count) {
			remove_count += 1;
			if (layout_aux->free_sets_in_use.data[j] == VK_NULL_HANDLE) {
				break;
			}
			copy_count += 1; // avoid copying the VK_NULL_HANDLE
		}

		//
		// copy the elements over to the free_sets array
		VkDescriptorSet* dst;
		result = hero_stack(VkDescriptorSet, push_many)(&layout_aux->free_sets, copy_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_DESCRIPTOR_POOL_VULKAN_LAYOUT_AUXS_FREE_SETS, &dst);
		if (result < 0) {
			return result;
		}
		HERO_COPY_ELMT_MANY(dst, &layout_aux->free_sets_in_use.data[0], copy_count);

		//
		// remove the from the free_sets_in_use array
		hero_stack(VkDescriptorSet, remove_shift_range)(&layout_aux->free_sets_in_use, 0, remove_count);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_set_init(HeroLogicalDevice* ldev, HeroShaderVulkan* shader_vulkan, U32 set, HeroDescriptorPoolVulkan* descriptor_pool_vulkan, HeroDescriptorSetVulkan* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkDescriptorSet vk_descriptor_set;
	VkDescriptorSetLayout vk_descriptor_set_layout = shader_vulkan->descriptor_set_layouts[set];
	result = _hero_vulkan_descriptor_pool_alloc(ldev, descriptor_pool_vulkan, vk_descriptor_set_layout, &vk_descriptor_set);
	if (result < 0) {
		return result;
	}

	HeroShaderMetadata* metadata = shader_vulkan->public_.metadata;
	U32 descriptors_count = shader_vulkan->descriptors_counts[set];
	HeroDescriptorUpdateDataVulkan* descriptor_update_data = hero_alloc_array(HeroDescriptorUpdateDataVulkan, hero_system_alctor, 0, descriptors_count);
	if (descriptors_count && !descriptor_update_data) {
		return HERO_ERROR(GFX_OUT_OF_MEMORY_DEVICE);
	}

	HeroSpirVDescriptorBinding* bindings = hero_shader_metadata_spir_v_descriptor_bindings(metadata);
	U32 bindings_start_idx = 0;
	for_range(i, 0, set) {
		bindings_start_idx += metadata->spir_v.descriptor_bindings_counts[i];
	}
	bindings = &bindings[bindings_start_idx];
	U32 data_idx = 0;
	U32 bindings_count = metadata->spir_v.descriptor_bindings_counts[set];
	for_range(i, 0, bindings_count) {
		switch (bindings[i].descriptor_type) {
			case HERO_DESCRIPTOR_TYPE_SAMPLER:
			case HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				bool is_storage = bindings[i].descriptor_type == HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				for_range(j, data_idx, data_idx + bindings[i].descriptor_count) {
					descriptor_update_data[j].image.sampler = ldev_vulkan->null_sampler;
					descriptor_update_data[j].image.imageView = ldev_vulkan->null_image_view_2d;
					descriptor_update_data[j].image.imageLayout = is_storage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				break;
			};
			case HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				for_range(j, data_idx, data_idx + bindings[i].descriptor_count) {
					descriptor_update_data[j].buffer.buffer = ldev_vulkan->null_buffer;
					descriptor_update_data[j].buffer.offset = 0;
					descriptor_update_data[j].buffer.range = 1;
				}
				break;
			case HERO_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
			case HERO_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			case HERO_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				HERO_ABORT("TODO");
		}
		data_idx += bindings[i].descriptor_count;
	}

	out->handle = vk_descriptor_set;
	out->layout = vk_descriptor_set_layout;
	out->update_template = shader_vulkan->descriptor_update_templates[set];
	out->update_data = descriptor_update_data;
	out->binding_update_data_indices = &shader_vulkan->descriptor_binding_update_data_indices[bindings_start_idx];
	out->descriptors_count = descriptors_count;
	out->last_submitted_frame_idx = ldev->last_completed_frame_idx - 1; // start one less, so we can queue the data the frame it is created

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_set_deinit(HeroLogicalDevice* ldev, HeroShaderVulkan* shader_vulkan, HeroDescriptorPoolVulkan* descriptor_pool_vulkan, HeroDescriptorSetVulkan* descriptor_set) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (descriptor_set->descriptors_count) {
		hero_dealloc_array(HeroDescriptorUpdateDataVulkan, hero_system_alctor, 0, descriptor_set->update_data, descriptor_set->descriptors_count);
	}

	result = _hero_vulkan_descriptor_pool_dealloc(ldev, descriptor_pool_vulkan, descriptor_set->layout, descriptor_set->handle, descriptor_set->last_submitted_frame_idx);
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_set_set_descriptor(HeroLogicalDevice* ldev, HeroDescriptorPoolId descriptor_pool_id, HeroDescriptorSetVulkan* set, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (HERO_GFX_FRAME_IDX_LESS_THAN(set->last_submitted_frame_idx, ldev->last_submitted_frame_idx)) {
		HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
		result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, descriptor_pool_id, &descriptor_pool_vulkan);
		if (result < 0) {
			return result;
		}

		result = _hero_vulkan_descriptor_pool_dealloc(ldev, descriptor_pool_vulkan, set->layout, set->handle, set->last_submitted_frame_idx);
		if (result < 0) {
			return result;
		}

		result = _hero_vulkan_descriptor_pool_alloc(ldev, descriptor_pool_vulkan, set->layout, &set->handle);
		if (result < 0) {
			return result;
		}

		set->last_submitted_frame_idx = ldev->last_submitted_frame_idx;
	}

	U32 data_idx = set->binding_update_data_indices[binding_idx];
	HeroDescriptorUpdateDataVulkan* vulkan_data = &set->update_data[data_idx + elmt_idx];
	switch (type) {
		case HERO_DESCRIPTOR_TYPE_SAMPLER:
		case HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
			HeroSamplerVulkan* sampler_vulkan;
			result = hero_object_pool(HeroSamplerVulkan, get)(&ldev_vulkan->sampler_pool, data->image.sampler_id, &sampler_vulkan);
			if (result < 0) {
				return result;
			}

			vulkan_data->image.sampler = sampler_vulkan->handle;
			if (type != HERO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				break;
			}
			// fallthrough
		};
		case HERO_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		case HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
			HeroImageVulkan* image_vulkan;
			result = hero_object_pool(HeroImageVulkan, get)(&ldev_vulkan->image_pool, data->image.id, &image_vulkan);
			if (result < 0) {
				return result;
			}

			bool is_storage = type == HERO_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			vulkan_data->image.imageView = image_vulkan->view_handle;
			vulkan_data->image.imageLayout = is_storage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		};
		case HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		case HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		case HERO_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		case HERO_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
			HeroBufferVulkan* buffer_vulkan;
			result = hero_object_pool(HeroBufferVulkan, get)(&ldev_vulkan->buffer_pool, data->buffer.id, &buffer_vulkan);
			if (result < 0) {
				return result;
			}

			vulkan_data->buffer.buffer = buffer_vulkan->handle;
			vulkan_data->buffer.offset = data->buffer.offset;
			vulkan_data->buffer.range = (U64)buffer_vulkan->public_.elmts_count * (U64)buffer_vulkan->public_.elmt_size;
			break;
		};
		case HERO_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		case HERO_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		case HERO_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
			HERO_ABORT("TODO");
	};

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_descriptor_set_update(HeroLogicalDevice* ldev, HeroDescriptorSetVulkan* descriptor_set) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (descriptor_set->descriptors_count) {
		ldev_vulkan->vkUpdateDescriptorSetWithTemplate(ldev_vulkan->handle, descriptor_set->handle, descriptor_set->update_template, descriptor_set->update_data);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_globals_init(HeroLogicalDevice* ldev, HeroShaderGlobalsSetup* setup, HeroShaderGlobalsId* id_out, HeroShaderGlobals** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroShaderVulkan* shader_vulkan;
	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	{
		result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, setup->shader_id, &shader_vulkan);
		if (result < 0) {
			return result;
		}

		result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, setup->descriptor_pool_id, &descriptor_pool_vulkan);
		if (result < 0) {
			return result;
		}
	}

	HeroShaderGlobalsVulkan* shader_globals_vulkan;
	result = hero_object_pool(HeroShaderGlobalsVulkan, alloc)(&ldev_vulkan->shader_globals_pool, 0, &shader_globals_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_descriptor_set_init(ldev, shader_vulkan, HERO_GFX_DESCRIPTOR_SET_GLOBAL, descriptor_pool_vulkan, &shader_globals_vulkan->descriptor_set);
	if (result < 0) {
		return result;
	}

	*out = &shader_globals_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_globals_deinit(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals* shader_globals) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroShaderGlobalsVulkan* shader_globals_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroShaderGlobalsVulkan, shader_globals);

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, shader_globals->shader_id, &shader_vulkan);
	if (result < 0) {
		return result;
	}

	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, shader_globals->descriptor_pool_id, &descriptor_pool_vulkan);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_descriptor_set_deinit(ldev, shader_vulkan, descriptor_pool_vulkan, &shader_globals_vulkan->descriptor_set);
	if (result < 0) {
		return result;
	}

	return hero_object_pool(HeroShaderGlobalsVulkan, dealloc)(&ldev_vulkan->shader_globals_pool, id);
}

HeroResult _hero_vulkan_shader_globals_get(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, HeroShaderGlobals** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroShaderGlobalsVulkan* shader_globals_vulkan;
	result = hero_object_pool(HeroShaderGlobalsVulkan, get)(&ldev_vulkan->shader_globals_pool, id, &shader_globals_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &shader_globals_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_shader_globals_set_descriptor(HeroLogicalDevice* ldev, HeroShaderGlobalsId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroShaderGlobalsVulkan* shader_globals_vulkan;
	result = hero_object_pool(HeroShaderGlobalsVulkan, get)(&ldev_vulkan->shader_globals_pool, id, &shader_globals_vulkan);
	if (result < 0) {
		return result;
	}

	return _hero_vulkan_descriptor_set_set_descriptor(ldev, shader_globals_vulkan->public_.descriptor_pool_id, &shader_globals_vulkan->descriptor_set, binding_idx, elmt_idx, type, data);
}

HeroResult _hero_vulkan_shader_globals_update(HeroLogicalDevice* ldev, HeroShaderGlobals* shader_globals) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroShaderGlobalsVulkan* shader_globals_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroShaderGlobalsVulkan, shader_globals);

	return _hero_vulkan_descriptor_set_update(ldev, &shader_globals_vulkan->descriptor_set);
}

HeroResult _hero_vulkan_render_pass_init_handle(HeroLogicalDevice* ldev, HeroAttachmentLayout* attachment_layouts, HeroAttachmentInfo* attachment_infos, U32 attachments_count, VkRenderPass* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	bool is_render_pass_layout = attachment_infos == NULL;
	bool is_render_pass = attachment_infos != NULL;

	VkAttachmentDescription* vk_attachments;
	VkAttachmentReference* vk_attachment_refs;
	VkSubpassDescription vk_subpass = {0};
	{
		vk_attachments = hero_alloc_array(VkAttachmentDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_DESCRIPTION, attachments_count);
		if (vk_attachments == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_attachment_refs = hero_alloc_array(VkAttachmentReference, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_REF, attachments_count);
		if (vk_attachment_refs == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		if (is_render_pass_layout) {
			// render pass layouts does not need to set most of VkAttachmentDescription.
			// only the fields in the for loop below need to be set for them.
			HERO_ZERO_ELMT_MANY(vk_attachments, attachments_count);
		}

		VkAttachmentReference* vk_depth_ref = NULL;
		U32 color_attachment_idx = 0;
		for (U32 i = 0; i < attachments_count; i += 1) {
			HeroAttachmentLayout* hero = &attachment_layouts[i];
			bool is_depth = HERO_IMAGE_FORMAT_IS_DEPTH(hero->format);

			VkAttachmentDescription* vk = &vk_attachments[i];
			vk->format = _hero_vulkan_convert_to_format[hero->format];
			vk->samples = hero->samples_count;

			if (is_depth) {
				vk->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			} else {
				switch (hero->post_usage) {
					case HERO_ATTACHEMENT_POST_USAGE_PRESENT:
						vk->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						break;
					case HERO_ATTACHEMENT_POST_USAGE_SAMPLED:
						vk->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						break;
					case HERO_ATTACHEMENT_POST_USAGE_NONE:
						vk->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						break;
				}
			}

			VkAttachmentReference* ref;
			if (is_depth) {
				HERO_DEBUG_ASSERT(vk_depth_ref == NULL, "front end should have checked to ensure we have 1 depth attachment at most");
				ref = &vk_attachment_refs[attachments_count - 1];
				vk_depth_ref = ref;
			} else {
				ref = &vk_attachment_refs[color_attachment_idx];
				color_attachment_idx += 1;
			}
			ref->attachment = i;
			ref->layout = is_depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		vk_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vk_subpass.colorAttachmentCount = attachments_count - (vk_depth_ref ? 1 : 0);
		vk_subpass.pColorAttachments = vk_attachment_refs;
		vk_subpass.pDepthStencilAttachment = vk_depth_ref;
	}

	if (is_render_pass) {
		for (U32 i = 0; i < attachments_count; i += 1) {
			HeroAttachmentInfo* hero = &attachment_infos[i];
			VkAttachmentDescription* vk = &vk_attachments[i];
			vk->flags = 0;
			vk->loadOp = _hero_vulkan_convert_to_attachment_load_op[hero->load_op];
			vk->storeOp = _hero_vulkan_convert_to_attachment_store_op[hero->store_op];
			vk->stencilLoadOp = _hero_vulkan_convert_to_attachment_load_op[hero->stencil_load_op];
			vk->stencilStoreOp = _hero_vulkan_convert_to_attachment_store_op[hero->stencil_store_op];
			vk->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	static VkSubpassDependency vk_dependencies[] = {
		{
			.srcSubpass      = VK_SUBPASS_EXTERNAL,
			.dstSubpass      = 0,
			.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		},
		{
			.srcSubpass      = 0,
			.dstSubpass      = VK_SUBPASS_EXTERNAL,
			.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		}
	};

	VkRenderPassCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pAttachments = vk_attachments,
		.attachmentCount = attachments_count,
		.pSubpasses = &vk_subpass,
		.subpassCount = 1,
		.pDependencies = vk_dependencies,
		.dependencyCount = HERO_ARRAY_COUNT(vk_dependencies),
	};

	VkRenderPass vk_render_pass;
	vk_result = ldev_vulkan->vkCreateRenderPass(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_render_pass);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	*out = vk_render_pass;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_pass_layout_init(HeroLogicalDevice* ldev, HeroRenderPassLayoutSetup* setup, HeroRenderPassLayoutId* id_out, HeroRenderPassLayout** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan;
	result = hero_object_pool(HeroRenderPassLayoutVulkan, alloc)(&ldev_vulkan->render_pass_layout_pool, 0, &render_pass_layout_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	VkRenderPass vk_render_pass;
	result = _hero_vulkan_render_pass_init_handle(ldev, setup->attachments, NULL, setup->attachments_count, &vk_render_pass);
	if (result < 0) {
		return result;
	}

	render_pass_layout_vulkan->handle = vk_render_pass;
	*out = &render_pass_layout_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_pass_layout_deinit(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout* render_pass_layout) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroRenderPassLayoutVulkan, render_pass_layout);

	ldev_vulkan->vkDestroyRenderPass(ldev_vulkan->handle, render_pass_layout_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroRenderPassLayoutVulkan, dealloc)(&ldev_vulkan->render_pass_layout_pool, id);
}

HeroResult _hero_vulkan_render_pass_layout_get(HeroLogicalDevice* ldev, HeroRenderPassLayoutId id, HeroRenderPassLayout** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan;
	result = hero_object_pool(HeroRenderPassLayoutVulkan, get)(&ldev_vulkan->render_pass_layout_pool, id, &render_pass_layout_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &render_pass_layout_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_pass_init(HeroLogicalDevice* ldev, HeroRenderPassSetup* setup, HeroRenderPassLayout* render_pass_layout, HeroRenderPassId* id_out, HeroRenderPass** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkRenderPass vk_render_pass;
	result = _hero_vulkan_render_pass_init_handle(ldev, render_pass_layout->attachments, setup->attachments, setup->attachments_count, &vk_render_pass);
	if (result < 0) {
		return result;
	}

	HeroRenderPassVulkan* render_pass_vulkan;
	result = hero_object_pool(HeroRenderPassVulkan, alloc)(&ldev_vulkan->render_pass_pool, 0, &render_pass_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	render_pass_vulkan->handle = vk_render_pass;
	*out = &render_pass_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_pass_deinit(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass* render_pass) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroRenderPassVulkan* render_pass_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroRenderPassVulkan, render_pass);

	ldev_vulkan->vkDestroyRenderPass(ldev_vulkan->handle, render_pass_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroRenderPassVulkan, dealloc)(&ldev_vulkan->render_pass_pool, id);
}

HeroResult _hero_vulkan_render_pass_get(HeroLogicalDevice* ldev, HeroRenderPassId id, HeroRenderPass** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderPassVulkan* render_pass_vulkan;
	result = hero_object_pool(HeroRenderPassVulkan, get)(&ldev_vulkan->render_pass_pool, id, &render_pass_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &render_pass_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_buffer_init(HeroLogicalDevice* ldev, HeroFrameBufferSetup* setup, HeroFrameBufferId* id_out, HeroFrameBuffer** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderPassLayout* render_pass_layout;
	result = hero_render_pass_layout_get(ldev, setup->render_pass_layout_id, &render_pass_layout);
	if (result < 0) {
		return result;
	}
	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroRenderPassLayoutVulkan, render_pass_layout);

	VkImageView* vk_attachments = hero_alloc_array(VkImageView, hero_system_alctor, HERO_ERROR_GFX_FRAME_BUFFER_VULKAN_ATTACHMENTS, setup->attachments_count);
	for (U32 i = 0; i < setup->attachments_count; i += 1) {
		HeroImage* image;
		result = hero_image_get(ldev, setup->attachments[i], &image);
		if (result < 0) {
			return result;
		}
		HeroImageVulkan* image_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroImageVulkan, image);

		vk_attachments[i] = image_vulkan->view_handle;
	}

	VkFramebufferCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.renderPass = render_pass_layout_vulkan->handle,
		.attachmentCount = setup->attachments_count,
		.pAttachments = vk_attachments,
		.width = setup->width,
		.height = setup->height,
		.layers = setup->layers,
	};

	VkFramebuffer vk_frame_buffer;
	vk_result = ldev_vulkan->vkCreateFramebuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_frame_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroFrameBufferVulkan* frame_buffer_vulkan;
	result = hero_object_pool(HeroFrameBufferVulkan, alloc)(&ldev_vulkan->frame_buffer_pool, 0, &frame_buffer_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	frame_buffer_vulkan->handle = vk_frame_buffer;
	*out = &frame_buffer_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_buffer_deinit(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer* frame_buffer) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameBufferVulkan* frame_buffer_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameBufferVulkan, frame_buffer);

	ldev_vulkan->vkDestroyFramebuffer(ldev_vulkan->handle, frame_buffer_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroFrameBufferVulkan, dealloc)(&ldev_vulkan->frame_buffer_pool, id);
}

HeroResult _hero_vulkan_frame_buffer_get(HeroLogicalDevice* ldev, HeroFrameBufferId id, HeroFrameBuffer** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroFrameBufferVulkan* frame_buffer_vulkan;
	result = hero_object_pool(HeroFrameBufferVulkan, get)(&ldev_vulkan->frame_buffer_pool, id, &frame_buffer_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &frame_buffer_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_pipeline_cache_init(HeroLogicalDevice* ldev, HeroPipelineCacheSetup* setup, HeroPipelineCacheId* id_out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkPipelineCacheCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.initialDataSize = setup->size,
		.pInitialData = setup->data,
	};

	VkPipelineCache vk_pipeline_cache;
	vk_result = ldev_vulkan->vkCreatePipelineCache(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_pipeline_cache);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroPipelineCacheVulkan* pipeline_cache_vulkan;
	result = hero_object_pool(HeroPipelineCacheVulkan, alloc)(&ldev_vulkan->pipeline_cache_pool, 0, &pipeline_cache_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	pipeline_cache_vulkan->handle = vk_pipeline_cache;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_pipeline_cache_deinit(HeroLogicalDevice* ldev, HeroPipelineCacheId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroPipelineCacheVulkan* pipeline_cache_vulkan;
	result = hero_object_pool(HeroPipelineCacheVulkan, get)(&ldev_vulkan->pipeline_cache_pool, id, &pipeline_cache_vulkan);
	if (result < 0) {
		return result;
	}

	ldev_vulkan->vkDestroyPipelineCache(ldev_vulkan->handle, pipeline_cache_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroPipelineCacheVulkan, dealloc)(&ldev_vulkan->pipeline_cache_pool, id);
}

HeroResult _hero_vulkan_pipeline_graphics_init(HeroLogicalDevice* ldev, HeroPipelineGraphicsSetup* setup, HeroPipelineId* id_out, HeroPipeline** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;


	HeroRenderState* render_state = setup->render_state;

	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan;
	result = hero_object_pool(HeroRenderPassLayoutVulkan, get)(&ldev_vulkan->render_pass_layout_pool, setup->render_pass_layout_id, &render_pass_layout_vulkan);
	if (result < 0) {
		return result;
	}

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, setup->shader_id, &shader_vulkan);
	if (result < 0) {
		return result;
	}
	HeroShader* shader = &shader_vulkan->public_;

	VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
	if (setup->cache_id.raw) {
		HeroPipelineCacheVulkan* pipeline_cache_vulkan;
		result = hero_object_pool(HeroPipelineCacheVulkan, get)(&ldev_vulkan->pipeline_cache_pool, setup->cache_id, &pipeline_cache_vulkan);
		if (result < 0) {
			return result;
		}

		vk_pipeline_cache = pipeline_cache_vulkan->handle;
	}

	VkPipelineShaderStageCreateInfo vk_shader_stages[HERO_SHADER_STAGES_COUNT] = {0};
	U32 vk_shader_stages_count;
	{
		static VkShaderStageFlags vk_shader_stages_stages_graphics[] = {
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		static VkShaderStageFlags vk_shader_stages_stages_graphics_mesh[] = {
			VK_SHADER_STAGE_TASK_BIT_NV,
			VK_SHADER_STAGE_MESH_BIT_NV,
			VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		U32 vk_max_shader_stages_count = HERO_SHADER_TYPE_STAGES_COUNTS[HERO_SHADER_TYPE_GRAPHICS];
		VkShaderStageFlags* vk_shader_stages_stages = vk_shader_stages_stages_graphics;
		if (shader->stages.type == HERO_SHADER_TYPE_GRAPHICS_MESH) {
			vk_max_shader_stages_count = HERO_SHADER_TYPE_STAGES_COUNTS[HERO_SHADER_TYPE_GRAPHICS_MESH];
			vk_shader_stages_stages = vk_shader_stages_stages_graphics_mesh;
		}

		vk_shader_stages_count = vk_max_shader_stages_count;
		for (U32 dst_idx = 0, src_idx = 0; src_idx < vk_max_shader_stages_count; dst_idx += 1, src_idx += 1) {
			HeroShaderStage* shader_stage = &shader->stages.data.array[src_idx];

			HeroShaderModule* shader_module;

			//
			// skip over the empty shader stages on the source end.
			// but keep the destination vulkan index the same.
			while (1) {
				if (shader_stage->module_id.raw) {
					result = hero_shader_module_get(ldev, shader_stage->module_id, &shader_module);
					if (result < 0) {
						return result;
					}

					if (shader_module->format != HERO_SHADER_FORMAT_NONE) {
						break;
					}
				}

				src_idx += 1;
				vk_shader_stages_count -= 1;
				if (src_idx == vk_max_shader_stages_count) {
					goto VK_PIPELINE_INIT_SHADER_END;
				}
				shader_stage = &shader->stages.data.array[src_idx];
			}

			HeroShaderModuleVulkan* shader_module_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroShaderModuleVulkan, shader_module);

			vk_shader_stages[dst_idx] = (VkPipelineShaderStageCreateInfo) {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = NULL,
				.flags = 0,
				.stage = vk_shader_stages_stages[src_idx],
				.module = shader_module_vulkan->handle,
				.pName = shader_stage->backend.spir_v.entry_point_name,
				.pSpecializationInfo = NULL,
			};
		}
VK_PIPELINE_INIT_SHADER_END: {}
	}

	VkPipelineVertexInputStateCreateInfo vk_vertex_input = (VkPipelineVertexInputStateCreateInfo) {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = 0,
		.vertexAttributeDescriptionCount = 0,
		.pVertexBindingDescriptions = NULL,
		.pVertexAttributeDescriptions = NULL,
	};

	if (setup->vertex_layout_id.raw) {
		HeroVertexLayoutVulkan* vertex_layout_vulkan;
		result = hero_object_pool(HeroVertexLayoutVulkan, get)(&hero_gfx_sys_vulkan.vertex_layout_pool, setup->vertex_layout_id, &vertex_layout_vulkan);
		if (result < 0) {
			return result;
		}

		vk_vertex_input.vertexBindingDescriptionCount = vertex_layout_vulkan->public_.bindings_count;
		vk_vertex_input.vertexAttributeDescriptionCount = vertex_layout_vulkan->attribs_count;
		vk_vertex_input.pVertexBindingDescriptions = vertex_layout_vulkan->bindings;
		vk_vertex_input.pVertexAttributeDescriptions = vertex_layout_vulkan->attribs;
	}

	VkPipelineInputAssemblyStateCreateInfo vk_input_assembly = (VkPipelineInputAssemblyStateCreateInfo) {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = _hero_vulkan_convert_to_topology[render_state->topology],
		.primitiveRestartEnable = render_state->enable_primitive_restart,
	};

	VkPipelineTessellationStateCreateInfo vk_tessellation = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.patchControlPoints = render_state->tessellation_patch_control_points,
	};

	VkPipelineViewportStateCreateInfo vk_viewport = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = render_state->viewports_count,
		.pViewports = NULL,
		.scissorCount = render_state->viewports_count, // vulkan spec says that viewportCount & scissorCount have to be the same number.
		.pScissors = NULL,
	};

	VkPipelineRasterizationStateCreateInfo vk_rasterization;
	{
		HeroRenderStateRasterization* hero_info = &render_state->rasterization;

		vk_rasterization = (VkPipelineRasterizationStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.depthClampEnable = (hero_info->flags & HERO_RASTERIZATION_FLAGS_ENABLE_DEPTH_CLAMP) != 0,
			.rasterizerDiscardEnable = (hero_info->flags & HERO_RASTERIZATION_FLAGS_ENABLE_DISCARD) != 0,
			.polygonMode = _hero_vulkan_convert_to_polygon_mode[hero_info->polygon_mode],
			.frontFace = _hero_vulkan_convert_to_front_face[hero_info->front_face],
			.depthBiasEnable = (hero_info->flags & HERO_RASTERIZATION_FLAGS_ENABLE_DEPTH_BIAS) != 0,
			.depthBiasConstantFactor = hero_info->depth_bias_constant_factor,
			.depthBiasClamp =  hero_info->depth_bias_clamp,
			.depthBiasSlopeFactor = hero_info->depth_bias_slope_factor,
			.lineWidth = hero_info->line_width,
		};

		_hero_vulkan_convert_to_cull_mode(hero_info->cull_mode_flags, &vk_rasterization.cullMode);
	}

	VkPipelineMultisampleStateCreateInfo vk_multisample;
	{
		HeroRenderStateMultisample* hero_info = &render_state->multisample;

		vk_multisample = (VkPipelineMultisampleStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.rasterizationSamples = hero_info->rasterization_samples_count,
			.sampleShadingEnable = (hero_info->flags & HERO_MULTISAMPLE_FLAGS_ENABLE_SAMPLE_SHADING) != 0,
			.minSampleShading = hero_info->min_sample_shading,
			.pSampleMask = hero_info->sample_mask,
			.alphaToCoverageEnable = (hero_info->flags & HERO_MULTISAMPLE_FLAGS_ENABLE_ALPHA_TO_COVERAGE) != 0,
			.alphaToOneEnable = (hero_info->flags & HERO_MULTISAMPLE_FLAGS_ENABLE_ALPHA_TO_ONE) != 0,
		};
	};

	VkPipelineDepthStencilStateCreateInfo vk_depth_stencil;
	{
		HeroRenderStateDepthStencil* hero_info = &render_state->depth_stencil;

		vk_depth_stencil = (VkPipelineDepthStencilStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.depthTestEnable = (hero_info->flags & HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_TEST) != 0,
			.depthWriteEnable = (hero_info->flags & HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_WRITE) != 0,
			.depthCompareOp = _hero_vulkan_convert_to_compare_op[hero_info->depth_compare_op],
			.depthBoundsTestEnable = (hero_info->flags & HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_BOUNDS_TEST) != 0,
			.stencilTestEnable = (hero_info->flags & HERO_DEPTH_STENCIL_FLAGS_ENABLE_STENCIL_TEST) != 0,
			.minDepthBounds = 0.f,
			.maxDepthBounds = 0.f,
		};
		_hero_vulkan_convert_to_stencil_op_state(&hero_info->front, &vk_depth_stencil.front);
		_hero_vulkan_convert_to_stencil_op_state(&hero_info->back, &vk_depth_stencil.back);
	};

	VkPipelineColorBlendStateCreateInfo vk_color_blend;
	{
		HeroRenderStateBlend* hero_info = &render_state->blend;

		VkPipelineColorBlendAttachmentState* vk_attachments = hero_alloc_array(VkPipelineColorBlendAttachmentState, hero_system_alctor, HERO_GFX_ALLOC_TAG_PIPELINE_VULKAN_ATTACHMENTS, hero_info->attachments_count);

		for (U32 i = 0; i < hero_info->attachments_count; i += 1) {
			_hero_vulkan_convert_to_pipeline_color_blend_attachement(&hero_info->attachments[i], &vk_attachments[i]);
		}

		vk_color_blend = (VkPipelineColorBlendStateCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.logicOpEnable = hero_info->enable_logic_op,
			.logicOp = _hero_vulkan_convert_to_logic_op[hero_info->logic_op],
			.attachmentCount = hero_info->attachments_count,
			.pAttachments = vk_attachments,
		};

		HERO_COPY_ARRAY(vk_color_blend.blendConstants, hero_info->blend_constants);
	}

	static VkDynamicState vk_dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo vk_dynamic = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = sizeof(vk_dynamic_states) / sizeof(*vk_dynamic_states),
		.pDynamicStates = vk_dynamic_states,
	};

	VkGraphicsPipelineCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = vk_shader_stages_count,
		.pStages = vk_shader_stages,
		.pVertexInputState = &vk_vertex_input,
		.pInputAssemblyState = &vk_input_assembly,
		.pTessellationState = &vk_tessellation,
		.pViewportState = &vk_viewport,
		.pRasterizationState = &vk_rasterization,
		.pMultisampleState = &vk_multisample,
		.pDepthStencilState = &vk_depth_stencil,
		.pColorBlendState = &vk_color_blend,
		.pDynamicState = &vk_dynamic,
		.layout = shader_vulkan->pipeline_layout,
		.renderPass = render_pass_layout_vulkan->handle,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0,
	};

	VkPipeline vk_pipeline;
	vk_result = ldev_vulkan->vkCreateGraphicsPipelines(ldev_vulkan->handle, vk_pipeline_cache, 1, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_pipeline);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroPipelineVulkan* pipeline_vulkan;
	result = hero_object_pool(HeroPipelineVulkan, alloc)(&ldev_vulkan->pipeline_pool, 0, &pipeline_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	pipeline_vulkan->handle = vk_pipeline;
	pipeline_vulkan->public_.type = HERO_PIPELINE_TYPE_GRAPHICS;
	*out = &pipeline_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_pipeline_deinit(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline* pipeline) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroPipelineVulkan* pipeline_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroPipelineVulkan, pipeline);

	ldev_vulkan->vkDestroyPipeline(ldev_vulkan->handle, pipeline_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroPipelineVulkan, dealloc)(&ldev_vulkan->pipeline_pool, id);
}

HeroResult _hero_vulkan_pipeline_get(HeroLogicalDevice* ldev, HeroPipelineId id, HeroPipeline** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroPipelineVulkan* pipeline_vulkan;
	result = hero_object_pool(HeroPipelineVulkan, get)(&ldev_vulkan->pipeline_pool, id, &pipeline_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &pipeline_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_material_init(HeroLogicalDevice* ldev, HeroMaterialSetup* setup, HeroMaterialId* id_out, HeroMaterial** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroPipelineVulkan* pipeline_vulkan;
	HeroShaderGlobalsVulkan* shader_globals_vulkan;
	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	HeroShaderVulkan* shader_vulkan;
	{
		result = hero_object_pool(HeroPipelineVulkan, get)(&ldev_vulkan->pipeline_pool, setup->pipeline_id, &pipeline_vulkan);
		if (result < 0) {
			return result;
		}

		result = hero_object_pool(HeroShaderGlobalsVulkan, get)(&ldev_vulkan->shader_globals_pool, setup->shader_globals_id, &shader_globals_vulkan);
		if (result < 0) {
			return result;
		}

		result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, setup->descriptor_pool_id, &descriptor_pool_vulkan);
		if (result < 0) {
			return result;
		}

		if (shader_globals_vulkan->public_.shader_id.raw != pipeline_vulkan->public_.shader_id.raw) {
			return HERO_ERROR(GFX_MATERIAL_PIPELINE_AND_SHADER_GLOBALS_SHADER_ID_MISMATCH);
		}

		result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, pipeline_vulkan->public_.shader_id, &shader_vulkan);
		if (result < 0) {
			return result;
		}
	}

	HeroMaterialVulkan* material_vulkan;
	result = hero_object_pool(HeroMaterialVulkan, alloc)(&ldev_vulkan->material_pool, 0, &material_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_descriptor_set_init(ldev, shader_vulkan, HERO_GFX_DESCRIPTOR_SET_MATERIAL, descriptor_pool_vulkan, &material_vulkan->descriptor_set);
	if (result < 0) {
		return result;
	}
	material_vulkan->public_.shader_id = pipeline_vulkan->public_.shader_id;
	*out = &material_vulkan->public_;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_material_deinit(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial* material) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroMaterialVulkan* material_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroMaterialVulkan, material);

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, material->shader_id, &shader_vulkan);
	if (result < 0) {
		return result;
	}

	HeroDescriptorPoolVulkan* descriptor_pool_vulkan;
	result = hero_object_pool(HeroDescriptorPoolVulkan, get)(&ldev_vulkan->descriptor_pool_pool, material->descriptor_pool_id, &descriptor_pool_vulkan);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_descriptor_set_deinit(ldev, shader_vulkan, descriptor_pool_vulkan, &material_vulkan->descriptor_set);
	if (result < 0) {
		return result;
	}

	return hero_object_pool(HeroMaterialVulkan, dealloc)(&ldev_vulkan->material_pool, id);
}

HeroResult _hero_vulkan_material_get(HeroLogicalDevice* ldev, HeroMaterialId id, HeroMaterial** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroMaterialVulkan* material_vulkan;
	result = hero_object_pool(HeroMaterialVulkan, get)(&ldev_vulkan->material_pool, id, &material_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &material_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_material_set_descriptor(HeroLogicalDevice* ldev, HeroMaterialId id, U16 binding_idx, U16 elmt_idx, HeroDescriptorType type, HeroDescriptorData* data) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroMaterialVulkan* material_vulkan;
	result = hero_object_pool(HeroMaterialVulkan, get)(&ldev_vulkan->material_pool, id, &material_vulkan);
	if (result < 0) {
		return result;
	}

	return _hero_vulkan_descriptor_set_set_descriptor(ldev, material_vulkan->public_.descriptor_pool_id, &material_vulkan->descriptor_set, binding_idx, elmt_idx, type, data);
}

HeroResult _hero_vulkan_material_update(HeroLogicalDevice* ldev, HeroMaterial* material) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroMaterialVulkan* material_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroMaterialVulkan, material);

	return _hero_vulkan_descriptor_set_update(ldev, &material_vulkan->descriptor_set);
}

HeroResult _hero_vulkan_swapchain_reinit(HeroLogicalDevice* ldev, HeroSwapchainVulkan* swapchain_vulkan) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroPhysicalDevice* physical_device = ldev->physical_device;
	HeroPhysicalDeviceVulkan* physical_device_vulkan = (HeroPhysicalDeviceVulkan*)physical_device;
	HeroSwapchain* swapchain = &swapchain_vulkan->public_;

	//
	// TODO: on initialization and in debug, ensure this format is supported
	HeroImageFormat image_format = HERO_SURFACE_IMAGE_FORMAT;

	//
	// on initialization
	if (swapchain_vulkan->handle == VK_NULL_HANDLE) {
		//
		// get the surface from the window
		{
			HeroWindow* window;
			result = hero_window_get(swapchain->window_id, &window);
			if (result < 0) {
				return result;
			}

			vk_result = _hero_window_vulkan_create_surface(window, hero_gfx_sys_vulkan.instance, &swapchain_vulkan->surface);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			VkBool32 is_supported = VK_FALSE;
			vk_result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_vulkan->handle, physical_device_vulkan->queue_family_idx_present, swapchain_vulkan->surface, &is_supported);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
			HERO_ASSERT(is_supported, "in _hero_vulkan_init we found the queue_family_idx_present which should be okay for all surfaces");
		}

		{
			VkPresentModeKHR* present_modes;
			U32 present_modes_count;
			{
				vk_result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_vulkan->handle, swapchain_vulkan->surface, &present_modes_count, NULL);
				if (vk_result < 0) {
					return _hero_vulkan_convert_from_result(vk_result);
				}

				present_modes = hero_alloc_array(VkPresentModeKHR, hero_system_alctor, HERO_GFX_ALLOC_TAG_SWAPCHAIN_VULKAN_PRESENT_MODES, present_modes_count);
				if (present_modes == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				vk_result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_vulkan->handle, swapchain_vulkan->surface, &present_modes_count, present_modes);
				if (vk_result < 0) {
					return _hero_vulkan_convert_from_result(vk_result);
				}
			}

			VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // spec says this is always present so default to fifo
			if (!swapchain->vsync || !swapchain->fifo) {
				VkPresentModeKHR desired_present_mode;
				if (swapchain->vsync) {
					desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
				} else {
					desired_present_mode = swapchain->fifo ? VK_PRESENT_MODE_FIFO_RELAXED_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
				for (U32 i = 0; i < present_modes_count; i += 1) {
					if (present_modes[i] == desired_present_mode) {
						present_mode = desired_present_mode;
						break;
					}
				}
			}

			swapchain_vulkan->present_mode = present_mode;
		}
	}

	VkSurfaceCapabilitiesKHR surface_capabilities;
	vk_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_vulkan->handle, swapchain_vulkan->surface, &surface_capabilities);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	U32 min_images_count = surface_capabilities.minImageCount;

	VkExtent2D image_extent = surface_capabilities.currentExtent;
	HERO_ASSERT(image_extent.width != -1 && image_extent.height != -1, "TODO handle this special case");

	U32 array_layers_count = HERO_MAX(swapchain->array_layers_count, 1);
	array_layers_count = HERO_MIN(array_layers_count, surface_capabilities.maxImageArrayLayers);

	swapchain->width = image_extent.width;
	swapchain->height = image_extent.height;

	VkCompositeAlphaFlagBitsKHR alpha_fmt = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	{
		VkCompositeAlphaFlagBitsKHR alpha_list[] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (int i = 0; i < sizeof(alpha_list) / sizeof(VkCompositeAlphaFlagBitsKHR); i += 1) {
			if (surface_capabilities.supportedCompositeAlpha & alpha_list[i]) {
				alpha_fmt = alpha_list[i];
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = swapchain_vulkan->surface,
		.minImageCount = min_images_count,
		.imageFormat = _hero_vulkan_convert_to_format[image_format],
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, // TODO: make this configurable
		.imageExtent = image_extent,
		.imageArrayLayers = array_layers_count,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &ldev_vulkan->queue_family_idx_uber,
		.preTransform = surface_capabilities.currentTransform,
		.compositeAlpha = alpha_fmt,
		.presentMode = swapchain_vulkan->present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = swapchain_vulkan->handle,
	};

	if (swapchain_vulkan->handle != VK_NULL_HANDLE) {
		//
		// we are waiting for the old swapchain & it's images to not be used anymore.
		// TODO: optimized this so that we only wait for those thing and not stall the whole workload
		vk_result = ldev_vulkan->vkDeviceWaitIdle(ldev_vulkan->handle);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	VkSwapchainKHR vk_swapchain;
	vk_result = ldev_vulkan->vkCreateSwapchainKHR(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_swapchain);
	if (vk_result) return _hero_vulkan_convert_from_result(vk_result);


	U32 images_count;
	VkImage* vk_images;
	{
		vk_result = ldev_vulkan->vkGetSwapchainImagesKHR(ldev_vulkan->handle, vk_swapchain, &images_count, NULL);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		vk_images = hero_alloc_array(VkImage, hero_system_alctor, HERO_GFX_ALLOC_TAG_SWAPCHAIN_VULKAN_IMAGES, images_count);
		if (vk_images == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_result = ldev_vulkan->vkGetSwapchainImagesKHR(ldev_vulkan->handle, vk_swapchain, &images_count, vk_images);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	//
	// deallocate the all old images
	for (U32 i = 0; i < swapchain->images_count; i += 1) {
		HeroImageId image_id = swapchain->image_ids[i];
		HeroImageVulkan* image_vulkan;
		result = hero_object_pool(HeroImageVulkan, get)(&ldev_vulkan->image_pool, image_id, &image_vulkan);
		if (result < 0) {
			return result;
		}

		//
		// TODO verify that we do not need to destroy this later. I don't think anything references this at this point in time.
		ldev_vulkan->vkDestroyImageView(ldev_vulkan->handle, image_vulkan->view_handle, HERO_VULKAN_TODO_ALLOCATOR);

		result = hero_object_pool(HeroImageVulkan, dealloc)(&ldev_vulkan->image_pool, image_id);
		if (result < 0) {
			return result;
		}
	}

	if (swapchain->images_count != images_count) {
		swapchain->image_ids = hero_realloc_array(HeroImageId, hero_system_alctor, HERO_GFX_ALLOC_TAG_SWAPCHAIN_IMAGE_IDS, swapchain->image_ids, swapchain->images_count, images_count);
		if (swapchain->image_ids == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
	}

	for (U32 i = 0; i < images_count; i += 1) {
		VkImageViewCreateInfo vk_image_view_create_info = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.image = vk_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = _hero_vulkan_convert_to_format[image_format],
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = array_layers_count,
			},
		};

		VkImageView vk_image_view;
		vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_image_view);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		HeroImageId image_id;
		HeroImageVulkan* image_vulkan;
		result = hero_object_pool(HeroImageVulkan, alloc)(&ldev_vulkan->image_pool, 0, &image_vulkan, &image_id);
		if (result < 0) {
			return result;
		}

		image_vulkan->handle = vk_images[i];
		image_vulkan->view_handle = vk_image_view;
		image_vulkan->public_.type = HERO_IMAGE_TYPE_2D;
		image_vulkan->public_.format = image_format;
		image_vulkan->public_.flags = HERO_IMAGE_FLAGS_SWAPCHAIN;
		image_vulkan->public_.samples = HERO_SAMPLE_COUNT_1;
		image_vulkan->public_.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS;
		image_vulkan->public_.width = image_extent.width;
		image_vulkan->public_.height = image_extent.height;
		image_vulkan->public_.depth = 1;
		image_vulkan->public_.mip_levels = 1;
		image_vulkan->public_.array_layers_count = array_layers_count;

		swapchain->image_ids[i] = image_id;
	}

	swapchain_vulkan->handle = vk_swapchain;
	swapchain->width = image_extent.width;
	swapchain->height = image_extent.height;
	swapchain->images_count = images_count;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_swapchain_init(HeroLogicalDevice* ldev, HeroSwapchainSetup* setup, HeroSwapchainId* id_out, HeroSwapchain** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (setup->array_layers_count == 0) {
		return HERO_ERROR(GFX_SWAPCHAIN_ARRAY_LAYERS_CANNOT_BE_ZERO);
	}

	HeroSwapchainVulkan* swapchain_vulkan;
	result = hero_object_pool(HeroSwapchainVulkan, alloc)(&ldev_vulkan->swapchain_pool, 0, &swapchain_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	swapchain_vulkan->public_.window_id = setup->window_id;
	swapchain_vulkan->public_.array_layers_count = setup->array_layers_count;
	swapchain_vulkan->public_.vsync = setup->vsync;
	swapchain_vulkan->public_.fifo = setup->fifo;

	result = _hero_vulkan_swapchain_reinit(ldev, swapchain_vulkan);
	if (result < 0) {
		return result;
	}

	VkSemaphore vk_semaphore_render;
	{
		VkSemaphoreCreateInfo vk_semaphore_create_info = {0};
		vk_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vk_result = ldev_vulkan->vkCreateSemaphore(ldev_vulkan->handle, &vk_semaphore_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_semaphore_render);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	swapchain_vulkan->semaphore_render = vk_semaphore_render;
	*out = &swapchain_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_swapchain_deinit(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain* swapchain) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroSwapchainVulkan* swapchain_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroSwapchainVulkan, swapchain);

	ldev_vulkan->vkDestroySwapchainKHR(ldev_vulkan->handle, swapchain_vulkan->handle, HERO_VULKAN_TODO_ALLOCATOR);

	return hero_object_pool(HeroSwapchainVulkan, dealloc)(&ldev_vulkan->swapchain_pool, id);
}

HeroResult _hero_vulkan_swapchain_get(HeroLogicalDevice* ldev, HeroSwapchainId id, HeroSwapchain** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroSwapchainVulkan* swapchain_vulkan;
	result = hero_object_pool(HeroSwapchainVulkan, get)(&ldev_vulkan->swapchain_pool, id, &swapchain_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &swapchain_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_swapchain_next_image(HeroLogicalDevice* ldev, HeroSwapchain* swapchain, U32* next_image_idx_out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroSwapchainVulkan* swapchain_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroSwapchainVulkan, swapchain);

	U32 image_idx = 0;
	bool has_created_new_swapchain = false;
	while (1) {
		vk_result = ldev_vulkan->vkAcquireNextImageKHR(ldev_vulkan->handle, swapchain_vulkan->handle, UINT64_MAX, swapchain_vulkan->semaphore_render, VK_NULL_HANDLE, &image_idx);
		if (vk_result == VK_ERROR_OUT_OF_DATE_KHR || vk_result == VK_SUBOPTIMAL_KHR) {
			result = _hero_vulkan_swapchain_reinit(ldev, swapchain_vulkan);
			if (result < 0) {
				return result;
			}
			has_created_new_swapchain = true;
			continue;
		}
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
		break;
	}

	swapchain_vulkan->image_idx = image_idx;
	if (next_image_idx_out) {
		*next_image_idx_out = image_idx;
	}

	swapchain->has_been_resized = has_created_new_swapchain;
	return has_created_new_swapchain ? HERO_SUCCESS_IS_NEW : HERO_SUCCESS;
}

/*
HeroResult _hero_vulkan_command_pool_reset_now(HeroLogicalDeviceVulkan* ldev_vulkan, HeroCommandPoolVulkan* command_pool_vulkan) {
	HeroResult result;

	HeroVulkanObjectDealloc* dealloc;
	result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, VK_OBJECT_TYPE_COMMAND_POOL, 1, &dealloc);
	if (result < 0) {
		return result;
	}
	dealloc->command_pool = command_pool_vulkan->command_pool;

	//
	// push the static VkCommandPool on this array to be freed once the next frame has finish executing
	if (command_pool_vulkan->command_pool_static != VK_NULL_HANDLE) {
		result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, VK_OBJECT_TYPE_COMMAND_POOL, 1, &dealloc);
		if (result < 0) {
			return result;
		}
		dealloc->command_pool = command_pool_vulkan->command_pool_static;
	}

	//
	// remove all free buffers as they will no longer be available
	command_pool_vulkan->free_buffers.count = 0;
	command_pool_vulkan->free_buffers_in_use.count = 0;
	hero_object_pool(HeroCommandPoolBufferVulkan, clear)(&command_pool_vulkan->command_buffer_pool);

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_pool_init(HeroLogicalDevice* ldev, HeroCommandPoolSetup* setup, HeroCommandPoolId* id_out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkCommandPoolCreateInfo vk_command_pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags =
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | // command buffers will be recycled using vkResetCommandBuffer
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,             // command buffers will short lived by being reset after every use
		.queueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
	};

	VkCommandPool vk_command_pool;
	vk_result = ldev_vulkan->vkCreateCommandPool(ldev_vulkan->handle, &vk_command_pool_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_command_pool);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkCommandPool vk_command_pool_static = VK_NULL_HANDLE;
	if (setup->support_static) {
		vk_command_pool_create_info.flags = 0; // our buffers will be long lived and not recycled by being reset
		vk_result = ldev_vulkan->vkCreateCommandPool(ldev_vulkan->handle, &vk_command_pool_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_command_pool_static);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	HeroCommandPoolVulkan* command_pool_vulkan;
	result = hero_object_pool(HeroCommandPoolVulkan, alloc)(&ldev_vulkan->command_pool_pool, 0, &command_pool_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	result = hero_object_pool(HeroCommandPoolBufferVulkan, init)(&command_pool_vulkan->command_buffer_pool, setup->command_buffers_cap, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_COMMAND_POOL_BUFFERS_POOL);
	if (result < 0) {
		return result;
	}

	command_pool_vulkan->command_pool = vk_command_pool;
	command_pool_vulkan->command_pool_static = vk_command_pool_static;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_pool_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroCommandPoolVulkan* command_pool_vulkan;
	result = hero_object_pool(HeroCommandPoolVulkan, get)(&ldev_vulkan->command_pool_pool, id, &command_pool_vulkan);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_command_pool_reset_now(ldev_vulkan, command_pool_vulkan);
	if (result < 0) {
		return result;
	}

	hero_stack(VkCommandBuffer, deinit)(&command_pool_vulkan->free_buffers, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS);
	hero_stack(VkCommandBuffer, deinit)(&command_pool_vulkan->free_buffers_in_use, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS_IN_USE);
	hero_object_pool(HeroCommandPoolBufferVulkan, deinit)(&command_pool_vulkan->command_buffer_pool, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_COMMAND_POOL_BUFFERS_POOL);

	return hero_object_pool(HeroCommandPoolVulkan, dealloc)(&ldev_vulkan->command_pool_pool, id);
}

HeroResult _hero_vulkan_command_pool_reset(HeroLogicalDevice* ldev, HeroCommandPoolId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroCommandPoolVulkan* command_pool_vulkan;
	result = hero_object_pool(HeroCommandPoolVulkan, get)(&ldev_vulkan->command_pool_pool, id, &command_pool_vulkan);
	if (result < 0) {
		return result;
	}

	result = _hero_vulkan_command_pool_reset_now(ldev_vulkan, command_pool_vulkan);
	if (result < 0) {
		return result;
	}


	return hero_object_pool(HeroCommandPoolVulkan, dealloc)(&ldev_vulkan->command_pool_pool, id);
}

HeroResult _hero_vulkan_command_pool_alloc(HeroLogicalDeviceVulkan* ldev_vulkan, HeroCommandPoolVulkan* command_pool_vulkan, bool is_static, VkCommandBuffer* out) {
	HeroResult result;
	VkResult vk_result;

	if (is_static && command_pool_vulkan->command_pool_static == VK_NULL_HANDLE) {
		return HERO_ERROR(GFX_COMMAND_POOL_STATIC_SUPPORT_NOT_ENABLED);
	}

	//
	// if we have a free buffer, pop it off the stack and return that.
	if (!is_static && command_pool_vulkan->free_buffers.count) {
		command_pool_vulkan->free_buffers.count -= 1;
		VkCommandBuffer vk_command_buffer = command_pool_vulkan->free_buffers.data[command_pool_vulkan->free_buffers.count];

		*out = vk_command_buffer;
		return HERO_SUCCESS;
	}

	VkCommandBufferAllocateInfo vk_command_buffer_alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = is_static ? command_pool_vulkan->command_pool_static : command_pool_vulkan->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1, // maybe in the future for non-static buffers we can allocate more and store the rest in free_buffers
	};

	VkCommandBuffer vk_command_buffer;
	vk_result = ldev_vulkan->vkAllocateCommandBuffers(ldev_vulkan->handle, &vk_command_buffer_alloc_info, &vk_command_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	*out = vk_command_buffer;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_pool_free_used_resources(HeroLogicalDevice* ldev, HeroCommandPoolVulkan* command_pool_vulkan) {
	HeroResult result;

	if (command_pool_vulkan->free_buffers_in_use.count == 0) {
		return HERO_SUCCESS;
	}

	//
	// count how many buffers that are no longer being used by the GPU
	// that we can move into the free_buffers array
	U32 copy_count = 0;
	U32 remove_count = 0;
	for_range(j, 0, command_pool_vulkan->free_buffers_in_use.count) {
		remove_count += 1;
		if (command_pool_vulkan->free_buffers_in_use.data[j] == VK_NULL_HANDLE) {
			break;
		}
		copy_count += 1; // avoid copying the VK_NULL_HANDLE
	}

	//
	// copy the elements over to the free_buffers array
	VkCommandBuffer* dst;
	result = hero_stack(VkCommandBuffer, push_many)(&command_pool_vulkan->free_buffers, copy_count, hero_system_alctor, HERO_GFX_ALLOC_TAG_COMMAND_POOL_VULKAN_FREE_BUFFERS, &dst);
	if (result < 0) {
		return result;
	}
	HERO_COPY_ELMT_MANY(dst, command_pool_vulkan->free_buffers_in_use.data, copy_count);

	//
	// remove the from the free_buffers_in_use array
	hero_stack(VkCommandBuffer, remove_shift_range)(&command_pool_vulkan->free_buffers_in_use, 0, remove_count);

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_recorder_get(HeroLogicalDeviceVulkan* ldev_vulkan, HeroCommandPoolId command_pool_id, bool expected_in_use, HeroCommandPoolVulkan** command_pool_vulkan_out, HeroCommandRecorderVulkan** command_recorder_vulkan_out) {
	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroResult result = hero_object_pool(HeroCommandPoolVulkan, get)(&ldev_vulkan->command_pool_pool, command_pool_id, &command_pool_vulkan);
	if (result < 0) {
		return result;
	}

	HeroCommandRecorderVulkan* command_recorder_vulkan = &command_pool_vulkan->command_recorder;

	if (expected_in_use != (command_recorder_vulkan->command_buffer != VK_NULL_HANDLE)) {
		return expected_in_use ? HERO_ERROR(NOT_STARTED) : HERO_ERROR(ALREADY_STARTED);
	}

	*command_pool_vulkan_out = command_pool_vulkan;
	*command_recorder_vulkan_out = command_recorder_vulkan;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_buffer_get(HeroLogicalDeviceVulkan* ldev_vulkan, HeroCommandPoolId pool_id, HeroCommandPoolBufferId buffer_id, HeroCommandPoolVulkan** command_pool_vulkan_out, HeroCommandPoolBufferVulkan** command_pool_buffer_vulkan_out) {
	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroResult result = hero_object_pool(HeroCommandPoolVulkan, get)(&ldev_vulkan->command_pool_pool, pool_id, &command_pool_vulkan);
	if (result < 0) {
		return result;
	}

	HeroCommandPoolBufferVulkan* command_pool_buffer_vulkan;
	result = hero_object_pool(HeroCommandPoolBufferVulkan, get)(&command_pool_vulkan->command_buffer_pool, buffer_id, &command_pool_buffer_vulkan);
	if (result < 0) {
		return result;
	}

	*command_pool_vulkan_out = command_pool_vulkan;
	*command_pool_buffer_vulkan_out = command_pool_buffer_vulkan;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_buffer_deinit(HeroLogicalDevice* ldev, HeroCommandPoolId pool_id, HeroCommandPoolBufferId buffer_id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroCommandPoolBufferVulkan* command_buffer_vulkan;
	result = _hero_vulkan_command_buffer_get(ldev_vulkan, pool_id, buffer_id, &command_pool_vulkan, &command_buffer_vulkan);
	if (result < 0) {
		return result;
	}

	//
	//TODO finish meeeeeeee


	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_recorder_start(HeroLogicalDevice* ldev, HeroCommandPoolId command_pool_id, bool is_static, HeroCommandRecorder** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	//
	// get the command pool and buffer structures
	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroCommandRecorderVulkan* command_recorder_vulkan;
	result = _hero_vulkan_command_recorder_get(ldev_vulkan, command_pool_id, false, &command_pool_vulkan, &command_recorder_vulkan);
	if (result < 0) {
		return result;
	}
	HeroCommandRecorder* command_recorder = &command_recorder_vulkan->public_;
	HERO_ZERO_ELMT(command_recorder_vulkan);

	//
	// allocate a vulkan command buffer
	VkCommandBuffer vk_command_buffer;
	result = _hero_vulkan_command_pool_alloc(ldev_vulkan, command_pool_vulkan, is_static, &vk_command_buffer);
	if (result < 0) {
		return result;
	}

	//
	// start command buffer recording
	{
		VkCommandBufferBeginInfo vk_command_buffer_begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // command buffer is reset after a single submit
			.pInheritanceInfo = NULL,
		};

		vk_result = ldev_vulkan->vkBeginCommandBuffer(vk_command_buffer, &vk_command_buffer_begin_info);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	command_recorder_vulkan->command_buffer = vk_command_buffer;
	command_recorder->ldev = ldev;
	command_recorder->command_pool_id = command_pool_id;
	command_recorder->is_static = is_static;
	*out = command_recorder;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_command_recorder_end(HeroCommandRecorder* command_recorder, HeroCommandPoolBufferId* id_out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDevice* ldev = command_recorder->ldev;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	//
	// get the command pool and buffer structures
	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroCommandRecorderVulkan* command_recorder_vulkan;
	result = _hero_vulkan_command_recorder_get(ldev_vulkan, command_recorder->command_pool_id, true, &command_pool_vulkan, &command_recorder_vulkan);
	if (result < 0) {
		return result;
	}
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;

	vk_result = ldev_vulkan->vkEndCommandBuffer(vk_command_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroCommandPoolBufferId command_pool_buffer_id;
	HeroCommandPoolBufferVulkan* command_pool_buffer_vulkan;
	result = hero_object_pool(HeroCommandPoolBufferVulkan, alloc)(&command_pool_vulkan->command_buffer_pool, 0, &command_pool_buffer_vulkan, &command_pool_buffer_id);
	if (result < 0) {
		return result;
	}

	*id_out = command_pool_buffer_id;

	command_pool_buffer_vulkan->command_buffer = command_recorder_vulkan->command_buffer;
	command_pool_buffer_vulkan->is_static = command_recorder_vulkan->public_.is_static;
	command_recorder_vulkan->command_buffer = VK_NULL_HANDLE;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_render_pass_start(HeroCommandRecorder* command_recorder, HeroRenderPassId render_pass_id, HeroFrameBufferId frame_buffer_id, HeroViewport* viewport, HeroUAabb* scissor) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDevice* ldev = command_recorder->ldev;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	//
	// get the command pool and buffer structures
	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroCommandRecorderVulkan* command_recorder_vulkan;
	result = _hero_vulkan_command_recorder_get(ldev_vulkan, command_recorder->command_pool_id, true, &command_pool_vulkan, &command_recorder_vulkan);
	if (result < 0) {
		return result;
	}

	HeroRenderPassVulkan* render_pass_vulkan;
	result = hero_object_pool(HeroRenderPassVulkan, get)(&ldev_vulkan->render_pass_pool, render_pass_id, &render_pass_vulkan);
	if (result < 0) {
		return result;
	}

	HeroFrameBufferVulkan* frame_buffer_vulkan;
	result = hero_object_pool(HeroFrameBufferVulkan, get)(&ldev_vulkan->frame_buffer_pool, frame_buffer_id, &frame_buffer_vulkan);
	if (result < 0) {
		return result;
	}

	if (render_pass_vulkan->public_.layout_id.raw != frame_buffer_vulkan->public_.render_pass_layout_id.raw) {
		return HERO_ERROR(GFX_RENDER_PASS_LAYOUT_MISMATCH);
	}

	//
	// TODO: if we keep the same design as the VkClearValue then document this by HeroClearValue.
	VkClearValue* vk_clear_values = (VkClearValue*)render_pass_vulkan->public_.attachment_clear_values;
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;

	{
		HeroViewport default_viewport;
		if (viewport == NULL) {
			default_viewport.x = 0.f;
			default_viewport.y = 0.f;
			default_viewport.width = frame_buffer_vulkan->public_.width;
			default_viewport.height = frame_buffer_vulkan->public_.height;
			default_viewport.min_depth = 0.f;
			default_viewport.max_depth = 1.f;
			viewport = &default_viewport;
		}

		HeroUAabb default_scissor;
		if (scissor == NULL) {
			default_scissor.x = 0.f;
			default_scissor.y = 0.f;
			default_scissor.ex = frame_buffer_vulkan->public_.width;
			default_scissor.ey = frame_buffer_vulkan->public_.height;
			scissor = &default_scissor;
		}

		VkViewport vk_viewport;
		_hero_vulkan_convert_to_viewport(viewport, &vk_viewport);

		VkRect2D vk_scissor;
		_hero_vulkan_convert_to_rect2d(scissor, &vk_scissor);

		ldev_vulkan->vkCmdSetViewport(vk_command_buffer, 0, 1, &vk_viewport);
		ldev_vulkan->vkCmdSetScissor(vk_command_buffer, 0, 1, &vk_scissor);
	}

	VkRenderPassBeginInfo render_pass_being_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = render_pass_vulkan->handle,
		.framebuffer = frame_buffer_vulkan->handle,
		.renderArea = { .offset.x = 0, .offset.y = 0, .extent.width = frame_buffer_vulkan->public_.width, .extent.height = frame_buffer_vulkan->public_.height },
		.clearValueCount = render_pass_vulkan->public_.attachments_count,
		.pClearValues = vk_clear_values,
	};

	ldev_vulkan->vkCmdBeginRenderPass(vk_command_buffer, &render_pass_being_info, VK_SUBPASS_CONTENTS_INLINE);

	command_recorder_vulkan->bound_pipeline_graphics = VK_NULL_HANDLE;
	for_range(i, 0, HERO_GFX_DESCRIPTOR_SET_COUNT) {
		command_recorder_vulkan->bound_graphics_descriptor_sets[i] = VK_NULL_HANDLE;
	}
	for_range(i, 0, HERO_BUFFER_BINDINGS_CAP) {
		command_recorder_vulkan->bound_vertex_buffers[i] = VK_NULL_HANDLE;
		command_recorder_vulkan->bound_vertex_buffer_offsets[i] = 0;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_render_pass_end(HeroCommandRecorder* command_recorder) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDevice* ldev = command_recorder->ldev;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	//
	// get the command pool and buffer structures
	HeroCommandPoolVulkan* command_pool_vulkan;
	HeroCommandRecorderVulkan* command_recorder_vulkan;
	result = _hero_vulkan_command_recorder_get(ldev_vulkan, command_recorder->command_pool_id, true, &command_pool_vulkan, &command_recorder_vulkan);
	if (result < 0) {
		return result;
	}

	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;
	ldev_vulkan->vkCmdEndRenderPass(vk_command_buffer);

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroMaterialId material_id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;

	HeroMaterialVulkan* material_vulkan;
	result = hero_object_pool(HeroMaterialVulkan, get)(&ldev_vulkan->material_pool, material_id, &material_vulkan);
	if (result < 0) {
		return result;
	}

	HeroPipelineVulkan* pipeline_vulkan;
	result = hero_object_pool(HeroPipelineVulkan, get)(&ldev_vulkan->pipeline_pool, material_vulkan->public_.pipeline_id, &pipeline_vulkan);
	if (result < 0) {
		return result;
	}

	if (pipeline_vulkan->public_.type != HERO_PIPELINE_TYPE_GRAPHICS) {
		return HERO_ERROR(GFX_PIPELINE_MUST_BE_GRAPHICS);
	}

	HeroRenderPassVulkan* render_pass_vulkan;
	result = hero_object_pool(HeroRenderPassVulkan, get)(&ldev_vulkan->render_pass_pool, command_recorder->render_pass_id, &render_pass_vulkan);
	if (result < 0) {
		return result;
	}

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, pipeline_vulkan->public_.shader_id, &shader_vulkan);
	if (result < 0) {
		return result;
	}

	HeroShaderGlobalsVulkan* shader_globals_vulkan;
	result = hero_object_pool(HeroShaderGlobalsVulkan, get)(&ldev_vulkan->shader_globals_pool, material_vulkan->public_.shader_globals_id, &shader_globals_vulkan);
	if (result < 0) {
		return result;
	}

	if (render_pass_vulkan->public_.layout_id.raw != pipeline_vulkan->public_.render_pass_layout_id.raw) {
		return HERO_ERROR(GFX_RENDER_PASS_LAYOUT_MISMATCH);
	}

	if (pipeline_vulkan->handle != command_recorder_vulkan->bound_pipeline_graphics) {
		ldev_vulkan->vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_vulkan->handle);
		command_recorder_vulkan->bound_pipeline_graphics = pipeline_vulkan->handle;
	}

	if (command_recorder_vulkan->bound_graphics_descriptor_sets[HERO_GFX_DESCRIPTOR_SET_GLOBAL] != shader_globals_vulkan->descriptor_set.handle) {
		command_recorder_vulkan->bound_graphics_descriptor_sets[HERO_GFX_DESCRIPTOR_SET_GLOBAL] = shader_globals_vulkan->descriptor_set.handle;

		//
		// TODO
		U32 dynamic_descriptors_count = 0;
		U32* dynamic_descriptor_offsets = NULL;

		ldev_vulkan->vkCmdBindDescriptorSets(
			vk_command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			shader_vulkan->pipeline_layout,
			HERO_GFX_DESCRIPTOR_SET_GLOBAL,
			1,
			&shader_globals_vulkan->descriptor_set.handle,
			dynamic_descriptors_count,
			dynamic_descriptor_offsets
		);
	}

	if (command_recorder_vulkan->bound_graphics_descriptor_sets[HERO_GFX_DESCRIPTOR_SET_MATERIAL] != material_vulkan->descriptor_set.handle) {
		command_recorder_vulkan->bound_graphics_descriptor_sets[HERO_GFX_DESCRIPTOR_SET_MATERIAL] = material_vulkan->descriptor_set.handle;

		//
		// TODO
		U32 dynamic_descriptors_count = 0;
		U32* dynamic_descriptor_offsets = NULL;

		ldev_vulkan->vkCmdBindDescriptorSets(
			vk_command_buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			shader_vulkan->pipeline_layout,
			HERO_GFX_DESCRIPTOR_SET_MATERIAL,
			1,
			&material_vulkan->descriptor_set.handle,
			dynamic_descriptors_count,
			dynamic_descriptor_offsets
		);
	}

	command_recorder_vulkan->vertex_buffer_binding_start = -1;
	command_recorder_vulkan->vertex_buffer_binding_end = 0;
	command_recorder_vulkan->instances_start_idx = 0;
	command_recorder_vulkan->instances_count = 1;
	command_recorder_vulkan->pipeline_layout = shader_vulkan->pipeline_layout;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_end(HeroLogicalDeviceVulkan* ldev_vulkan, HeroCommandRecorderVulkan* command_recorder_vulkan, VkCommandBuffer vk_command_buffer) {
	//
	// if any of the vertex buffers have changed, lets bind them.
	if (command_recorder_vulkan->vertex_buffer_binding_start < command_recorder_vulkan->vertex_buffer_binding_end) {
		U32 buffers_count = command_recorder_vulkan->vertex_buffer_binding_end - command_recorder_vulkan->vertex_buffer_binding_start;
		ldev_vulkan->vkCmdBindVertexBuffers(vk_command_buffer,
			command_recorder_vulkan->vertex_buffer_binding_start,
			buffers_count,
			&command_recorder_vulkan->bound_vertex_buffers[command_recorder_vulkan->vertex_buffer_binding_start],
			&command_recorder_vulkan->bound_vertex_buffer_offsets[command_recorder_vulkan->vertex_buffer_binding_start]);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_start_idx, U32 vertices_count) {
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;

	HeroResult result = _hero_vulkan_cmd_draw_end(ldev_vulkan, command_recorder_vulkan, vk_command_buffer);
	if (result < 0) {
		return result;
	}

	ldev_vulkan->vkCmdDraw(vk_command_buffer,
		vertices_count,
		command_recorder_vulkan->instances_count,
		vertices_start_idx,
		command_recorder_vulkan->instances_start_idx);

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroBufferId index_buffer_id, U32 indices_start_idx, U32 indices_count, U32 vertices_start_idx) {
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;

	HeroResult result = _hero_vulkan_cmd_draw_end(ldev_vulkan, command_recorder_vulkan, vk_command_buffer);
	if (result < 0) {
		return result;
	}

	HeroBufferVulkan* buffer_vulkan;
	result = hero_object_pool(HeroBufferVulkan, get)(&ldev_vulkan->buffer_pool, index_buffer_id, &buffer_vulkan);
	if (result < 0) {
		return result;
	}

	if (buffer_vulkan->public_.type != HERO_BUFFER_TYPE_INDEX) {
		return HERO_ERROR(GFX_EXPECTED_INDEX_BUFFER);
	}

	ldev_vulkan->vkCmdBindIndexBuffer(vk_command_buffer,
		buffer_vulkan->handle,
		0,
		_hero_vulkan_convert_to_index_type[buffer_vulkan->public_.typed.index_type]);

	ldev_vulkan->vkCmdDrawIndexed(vk_command_buffer,
		indices_count,
		command_recorder_vulkan->instances_count,
		indices_start_idx,
		vertices_start_idx,
		command_recorder_vulkan->instances_start_idx);

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroBufferId buffer_id, U32 binding, U64 offset) {
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;

	HeroBufferVulkan* buffer_vulkan;
	HeroResult result = hero_object_pool(HeroBufferVulkan, get)(&ldev_vulkan->buffer_pool, buffer_id, &buffer_vulkan);
	if (result < 0) {
		return result;
	}

	if (buffer_vulkan->public_.type != HERO_BUFFER_TYPE_VERTEX) {
		return HERO_ERROR(GFX_EXPECTED_VERTEX_BUFFER);
	}

	offset *= buffer_vulkan->public_.elmt_size;

	HERO_ASSERT_ARRAY_BOUNDS(binding, HERO_BUFFER_BINDINGS_CAP);

	if (
		command_recorder_vulkan->bound_vertex_buffers[binding] != buffer_vulkan->handle ||
		command_recorder_vulkan->bound_vertex_buffer_offsets[binding] != offset
	) {
		command_recorder_vulkan->bound_vertex_buffers[binding] = buffer_vulkan->handle;
		command_recorder_vulkan->bound_vertex_buffer_offsets[binding] = offset;
		command_recorder_vulkan->vertex_buffer_binding_start = HERO_MIN(command_recorder_vulkan->vertex_buffer_binding_start, binding);
		command_recorder_vulkan->vertex_buffer_binding_end = HERO_MAX(command_recorder_vulkan->vertex_buffer_binding_end, binding + 1);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_set_push_constants(HeroCommandRecorder* command_recorder, void* data, U32 offset, U32 size) {
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;

	VkShaderStageFlags stage_flags = VK_SHADER_STAGE_ALL_GRAPHICS; // TODO get from shader metadata
	VkPipelineLayout pipeline_layout = command_recorder_vulkan->pipeline_layout;
	ldev_vulkan->vkCmdPushConstants(vk_command_buffer, pipeline_layout, stage_flags, offset, size, data);

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count) {
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;

	command_recorder_vulkan->instances_start_idx = instances_start_idx;
	command_recorder_vulkan->instances_count = instances_count;

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_cmd_compute_dispatch(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, HeroShaderGlobalsId shader_globals_id, U32 group_count_x, U32 group_count_y, U32 group_count_z) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	HeroCommandRecorderVulkan* command_recorder_vulkan = (HeroCommandRecorderVulkan*)command_recorder;
	VkCommandBuffer vk_command_buffer = command_recorder_vulkan->command_buffer;

	HeroShaderVulkan* shader_vulkan;
	result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, compute_shader_id, &shader_vulkan);
	if (result < 0) {
		return result;
	}

	HeroShaderGlobalsVulkan* shader_globals_vulkan;
	result = hero_object_pool(HeroShaderGlobalsVulkan, get)(&ldev_vulkan->shader_globals_pool, shader_globals_id, &shader_globals_vulkan);
	if (result < 0) {
		return result;
	}

	if (shader_vulkan->public_.stages.type != HERO_SHADER_TYPE_COMPUTE) {
		return HERO_ERROR(GFX_SHADER_MUST_BE_COMPUTE);
	}

	if (command_recorder_vulkan->bound_pipeline_compute != shader_vulkan->compute_pipeline) {
		ldev_vulkan->vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader_vulkan->compute_pipeline);
		command_recorder_vulkan->bound_pipeline_compute = shader_vulkan->compute_pipeline;
	}

	if (command_recorder_vulkan->bound_compute_descriptor_set != shader_globals_vulkan->descriptor_set.handle) {
		command_recorder_vulkan->bound_compute_descriptor_set = shader_globals_vulkan->descriptor_set.handle;

		//
		// TODO
		U32 dynamic_descriptors_count = 0;
		U32* dynamic_descriptor_offsets = NULL;

		ldev_vulkan->vkCmdBindDescriptorSets(
			vk_command_buffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			shader_vulkan->pipeline_layout,
			HERO_GFX_DESCRIPTOR_SET_GLOBAL,
			1,
			&shader_globals_vulkan->descriptor_set.handle,
			dynamic_descriptors_count,
			dynamic_descriptor_offsets
		);
	}

	ldev_vulkan->vkCmdDispatch(vk_command_buffer, group_count_x, group_count_y, group_count_z);

	return HERO_SUCCESS;
}
*/

HeroResult _hero_vulkan_render_graph_init(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out, HeroRenderGraph** out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderGraphVulkan* render_graph_vulkan;
	result = hero_object_pool(HeroRenderGraphVulkan, alloc)(&ldev_vulkan->render_graph_pool, 0, &render_graph_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	*out = &render_graph_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_deinit(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph* render_graph) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroRenderGraphVulkan* render_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroRenderGraphVulkan, render_graph);

	return hero_object_pool(HeroRenderGraphVulkan, dealloc)(&ldev_vulkan->render_graph_pool, id);
}

HeroResult _hero_vulkan_render_graph_get(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderGraphVulkan* render_graph_vulkan;
	result = hero_object_pool(HeroRenderGraphVulkan, get)(&ldev_vulkan->render_graph_pool, id, &render_graph_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &render_graph_vulkan->public_;
	return HERO_SUCCESS;
}

static VkDescriptorType hero_vulkan_descriptor_binding_types[HERO_VULKAN_DESCRIPTOR_BINDING_COUNT] = {
	[HERO_VULKAN_DESCRIPTOR_BINDING_SAMPLERS] = VK_DESCRIPTOR_TYPE_SAMPLER,
	[HERO_VULKAN_DESCRIPTOR_BINDING_RO_IMAGE_1D] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	[HERO_VULKAN_DESCRIPTOR_BINDING_RO_IMAGE_2D] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	[HERO_VULKAN_DESCRIPTOR_BINDING_RO_IMAGE_3D] = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
	[HERO_VULKAN_DESCRIPTOR_BINDING_RW_IMAGE_1D] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	[HERO_VULKAN_DESCRIPTOR_BINDING_RW_IMAGE_2D] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
	[HERO_VULKAN_DESCRIPTOR_BINDING_RW_IMAGE_3D] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
};

HeroResult _hero_vulkan_frame_graph_init(HeroLogicalDevice* ldev, HeroFrameGraphSetup* setup, HeroFrameGraphId* id_out, HeroFrameGraph** out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroFrameGraphVulkan* frame_graph_vulkan;
	result = hero_object_pool(HeroFrameGraphVulkan, alloc)(&ldev_vulkan->frame_graph_pool, 0, &frame_graph_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	{
		frame_graph_vulkan->passes = hero_alloc_array(HeroPassVulkan, hero_system_alctor, 0, setup->passes_cap);
		if (!frame_graph_vulkan->passes) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
	}

	//
	// allocate resource device memory pools
	{
		VkMemoryAllocateInfo vk_alloc_info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.memoryTypeIndex = 0, // TODO: find GPU_LOCAL memory
		};

		//
		// allocate persistent resource memory pools
		{
			vk_alloc_info.allocationSize = setup->persistent_images_memory_pool_size;
			VkResult vk_result = ldev_vulkan->vkAllocateMemory(ldev_vulkan->handle, &vk_alloc_info, HERO_VULKAN_TODO_ALLOCATOR, &frame_graph_vulkan->persistent_images_device_memory);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			vk_alloc_info.allocationSize = setup->persistent_buffers_memory_pool_size;
			vk_result = ldev_vulkan->vkAllocateMemory(ldev_vulkan->handle, &vk_alloc_info, HERO_VULKAN_TODO_ALLOCATOR, &frame_graph_vulkan->persistent_buffers_device_memory);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}

			frame_graph_vulkan->persistent_vulkan_resources = hero_alloc_array(HeroVulkanResource, hero_system_alctor, 0, setup->physical_resources_cap);
			if (!frame_graph_vulkan->persistent_vulkan_resources) {
				return HERO_ERROR(ALLOCATION_FAILURE);
			}
		}

		//
		// allocate the active frames resource memory pools
		//
		{
			frame_graph_vulkan->active_frames = hero_alloc_array(HeroFrameGraphVulkanActiveFrame, hero_system_alctor, 0, setup->active_frames_count);
			if (!frame_graph_vulkan->active_frames) {
				return HERO_ERROR(ALLOCATION_FAILURE);
			}

			for_range(i, 0, setup->active_frames_count) {
				HeroFrameGraphVulkanActiveFrame* active_frame = &frame_graph_vulkan->active_frames[i];

				vk_alloc_info.allocationSize = setup->images_memory_pool_size;
				VkResult vk_result = ldev_vulkan->vkAllocateMemory(ldev_vulkan->handle, &vk_alloc_info, HERO_VULKAN_TODO_ALLOCATOR, &active_frame->images_device_memory);
				if (vk_result < 0) {
					return _hero_vulkan_convert_from_result(vk_result);
				}

				vk_alloc_info.allocationSize = setup->buffers_memory_pool_size;
				vk_result = ldev_vulkan->vkAllocateMemory(ldev_vulkan->handle, &vk_alloc_info, HERO_VULKAN_TODO_ALLOCATOR, &active_frame->buffers_device_memory);
				if (vk_result < 0) {
					return _hero_vulkan_convert_from_result(vk_result);
				}
			}

			frame_graph_vulkan->active_frames_vulkan_resources = hero_alloc_array(HeroVulkanResource, hero_system_alctor, 0, (U32)setup->active_frames_count * (U32)setup->physical_resources_cap);
			if (!frame_graph_vulkan->active_frames_vulkan_resources) {
				return HERO_ERROR(ALLOCATION_FAILURE);
			}
		}
	}

	{
		frame_graph_vulkan->active_frames_frame_buffers = hero_alloc_array(VkFramebuffer, hero_system_alctor, 0, (U32)setup->active_frames_count * (U32)setup->passes_cap);
		if (!frame_graph_vulkan->active_frames_frame_buffers) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph_vulkan->active_frames_command_buffers = hero_alloc_array(VkCommandBuffer, hero_system_alctor, 0, (U32)setup->active_frames_count * (U32)setup->passes_cap);
		if (!frame_graph_vulkan->active_frames_command_buffers) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
	}

	{
		frame_graph_vulkan->execution_units_image_barrier_physical_resource_indices = hero_alloc_array(U16, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph_vulkan->execution_units_image_barrier_physical_resource_indices) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph_vulkan->execution_units_image_barriers = hero_alloc_array(VkImageMemoryBarrier, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph_vulkan->execution_units_image_barriers) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph_vulkan->execution_units_buffer_barrier_physical_resource_indices = hero_alloc_array(U16, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph_vulkan->execution_units_buffer_barrier_physical_resource_indices) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph_vulkan->execution_units_buffer_barriers = hero_alloc_array(VkBufferMemoryBarrier, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph_vulkan->execution_units_buffer_barriers) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		frame_graph_vulkan->execution_units = hero_alloc_array(HeroExecutionUnitVulkan, hero_system_alctor, 0, setup->resources_cap);
		if (!frame_graph_vulkan->execution_units) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
	}

	//
	// create the command pool and allocate all of the command buffers for all passes
	{
		VkCommandPoolCreateInfo vk_command_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags =
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | // command buffers will be recycled using vkResetCommandBuffer
				VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,             // command buffers will short lived by being reset after every use
			.queueFamilyIndex = ldev_vulkan->queue_family_idx_uber,
		};

		VkCommandPool vk_command_pool_transient_reset;
		VkResult vk_result = ldev_vulkan->vkCreateCommandPool(ldev_vulkan->handle, &vk_command_pool_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_command_pool_transient_reset);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		VkCommandBufferAllocateInfo vk_command_buffer_alloc_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = vk_command_pool_transient_reset,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = (U32)setup->passes_cap * (U32)setup->active_frames_count,
		};

		vk_result = ldev_vulkan->vkAllocateCommandBuffers(ldev_vulkan->handle, &vk_command_buffer_alloc_info, frame_graph_vulkan->active_frames_command_buffers);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		frame_graph_vulkan->command_pool_transient_reset = vk_command_pool_transient_reset;
	}

	{
		VkDescriptorSetLayoutBinding bindings[HERO_VULKAN_DESCRIPTOR_BINDING_COUNT];

		for_range(binding, 0, HERO_VULKAN_DESCRIPTOR_BINDING_COUNT) {
			VkDescriptorSetLayoutBinding* b = &bindings[binding];
			b->binding = binding;
			b->descriptorType = hero_vulkan_descriptor_binding_types[binding];
			b->descriptorCount = setup->resources_cap;
			b->stageFlags= VK_SHADER_STAGE_ALL;
			b->pImmutableSamplers = NULL;
		};
		bindings[HERO_VULKAN_DESCRIPTOR_BINDING_SAMPLERS].descriptorCount = ldev_vulkan->sampler_pool.cap;

		VkDescriptorSetLayoutCreateInfo vk_set_layout_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.bindingCount = HERO_VULKAN_DESCRIPTOR_BINDING_COUNT,
			.pBindings = bindings,
		};

		VkResult vk_result = ldev_vulkan->vkCreateDescriptorSetLayout(ldev_vulkan->handle, &vk_set_layout_create_info, HERO_VULKAN_TODO_ALLOCATOR, &frame_graph_vulkan->descriptor_set_layout);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	{
		VkDescriptorPoolSize pool_sizes[HERO_VULKAN_DESCRIPTOR_BINDING_COUNT];
		for_range(binding, 0, HERO_VULKAN_DESCRIPTOR_BINDING_COUNT) {
			pool_sizes[binding].type = hero_vulkan_descriptor_binding_types[binding];
			pool_sizes[binding].descriptorCount = setup->resources_cap;
		}
		pool_sizes[HERO_VULKAN_DESCRIPTOR_BINDING_SAMPLERS].descriptorCount = ldev_vulkan->sampler_pool.cap;

		VkDescriptorPoolCreateInfo vk_pool_create_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.maxSets = setup->active_frames_count,
			.poolSizeCount = HERO_VULKAN_DESCRIPTOR_BINDING_COUNT,
			.pPoolSizes = pool_sizes,
		};

		VkResult vk_result = ldev_vulkan->vkCreateDescriptorPool(ldev_vulkan->handle, &vk_pool_create_info, HERO_VULKAN_TODO_ALLOCATOR, &frame_graph_vulkan->descriptor_pool);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}

		VkDescriptorSetAllocateInfo vk_alloc_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.pNext = NULL,
			.descriptorPool = frame_graph_vulkan->descriptor_pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &frame_graph_vulkan->descriptor_set_layout,
		};

		for_range(active_frame_idx, 0, setup->active_frames_count) {
			VkDescriptorSet* dst = &frame_graph_vulkan->active_frames[active_frame_idx].descriptor_set;
			vk_result = ldev_vulkan->vkAllocateDescriptorSets(ldev_vulkan->handle, &vk_alloc_info, dst);
			if (vk_result < 0) {
				return _hero_vulkan_convert_from_result(vk_result);
			}
		}
	}

	*out = &frame_graph_vulkan->public_;
	return HERO_SUCCESS;

}

HeroResult _hero_vulkan_frame_graph_deinit(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroFrameGraph* frame_graph) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameGraphVulkan* frame_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameGraphVulkan, frame_graph);

	return hero_object_pool(HeroFrameGraphVulkan, dealloc)(&ldev_vulkan->frame_graph_pool, id);
}

HeroResult _hero_vulkan_frame_graph_get(HeroLogicalDevice* ldev, HeroFrameGraphId id, HeroFrameGraph** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroFrameGraphVulkan* frame_graph_vulkan;
	result = hero_object_pool(HeroFrameGraphVulkan, get)(&ldev_vulkan->frame_graph_pool, id, &frame_graph_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &frame_graph_vulkan->public_;
	return HERO_SUCCESS;

}

HeroResult _hero_vulkan_frame_graph_init_image(HeroLogicalDevice* ldev, HeroFrameGraphVulkan* frame_graph_vulkan, HeroPassPhysicalResource* physical_resource) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameGraph* frame_graph = &frame_graph_vulkan->public_;

	HeroImageInfo* image_info;
	{
		HeroPassEnum pass_enum = HERO_RESOURCE_ID_PASS_ENUM(physical_resource->id);
		HeroPassResourceId pass_resource_id = HERO_RESOURCE_ID_PASS_RESOURCE_ID(physical_resource->id);
		HeroPass* pass = &frame_graph->passes[pass_enum];
		HeroPassResource* resource = _hero_frame_graph_pass_resource_find(frame_graph, pass, pass_resource_id);
		result = _hero_image_info_get(ldev, resource->data.image_info_id, &image_info);
		if (result < 0) {
			return result;
		}
	}

	bool is_persistent = !!(physical_resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT);

	U32 active_frame_idx = 0;
	U32 physical_resource_idx = physical_resource - frame_graph_vulkan->public_.physical_resources;
NEXT_ACTIVE_FRAME: {}

	VkImageCreateInfo vk_image_create_info = {0};
	vk_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	vk_image_create_info.extent.width = image_info->width;
	vk_image_create_info.extent.height = image_info->height;
	vk_image_create_info.extent.depth = image_info->depth;
	vk_image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT; // TODO: collect usage data earlier and assign only what is needed
	if (HERO_IMAGE_FORMAT_IS_DEPTH(image_info->format)) {
		vk_image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	} else {
		vk_image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	vk_image_create_info.format = _hero_vulkan_convert_to_format[image_info->format];
	vk_image_create_info.imageType = image_info->type;
	vk_image_create_info.samples = 1 << image_info->samples_count_log2;
	vk_image_create_info.mipLevels = image_info->mip_levels_count;
	vk_image_create_info.arrayLayers = image_info->array_layers_count;

	VkImage image;
	vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &image);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkDeviceMemory device_memory = is_persistent
		? frame_graph_vulkan->persistent_images_device_memory
		: frame_graph_vulkan->active_frames[active_frame_idx].images_device_memory;
	vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, image, device_memory, physical_resource->mem_range.start_idx);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkImageViewCreateInfo vk_image_view_create_info = {0};
	vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vk_image_view_create_info.image = image;
	vk_image_view_create_info.format = _hero_vulkan_convert_to_format[image_info->format];
	vk_image_view_create_info.viewType = _hero_vulkan_convert_to_image_view_type[image_info->format];
	vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vk_image_view_create_info.subresourceRange.levelCount = image_info->mip_levels_count;
	vk_image_view_create_info.subresourceRange.layerCount = image_info->array_layers_count;

	VkImageView image_view;
	vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &image_view);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroVulkanResource* vulkan_resource;
	if (is_persistent) {
		vulkan_resource = &frame_graph_vulkan->persistent_vulkan_resources[physical_resource_idx];
	} else {
		vulkan_resource = &frame_graph_vulkan->active_frames_vulkan_resources[active_frame_idx * frame_graph_vulkan->public_.physical_resources_cap + physical_resource_idx];
	}
	vulkan_resource->image = image;
	vulkan_resource->image_view = image_view;

	if (!is_persistent && active_frame_idx < frame_graph_vulkan->public_.active_frames_count) {
		active_frame_idx += 1;
		goto NEXT_ACTIVE_FRAME;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_graph_init_buffer(HeroLogicalDevice* ldev, HeroFrameGraphVulkan* frame_graph_vulkan, HeroPassPhysicalResource* physical_resource) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	bool is_persistent = !!(physical_resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT);

	U32 active_frame_idx = 0;
	U32 physical_resource_idx = physical_resource - frame_graph_vulkan->public_.physical_resources;
NEXT_ACTIVE_FRAME: {}

	VkBufferCreateInfo vk_create_info = {0};
	vk_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vk_create_info.size = physical_resource->mem_range.end_idx - physical_resource->mem_range.start_idx;
	vk_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // TODO: collect usage data earlier and assign only what is needed

	VkBuffer buffer;
	vk_result = ldev_vulkan->vkCreateBuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkDeviceMemory device_memory = is_persistent
		? frame_graph_vulkan->persistent_buffers_device_memory
		: frame_graph_vulkan->active_frames[active_frame_idx].buffers_device_memory;
	vk_result = ldev_vulkan->vkBindBufferMemory(ldev_vulkan->handle, buffer, device_memory, physical_resource->mem_range.start_idx);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	HeroVulkanResource* vulkan_resource;
	if (is_persistent) {
		vulkan_resource = &frame_graph_vulkan->persistent_vulkan_resources[physical_resource_idx];
	} else {
		vulkan_resource = &frame_graph_vulkan->active_frames_vulkan_resources[active_frame_idx * frame_graph_vulkan->public_.physical_resources_cap + physical_resource_idx];
	}
	vulkan_resource->buffer = buffer;

	if (!is_persistent && active_frame_idx < frame_graph_vulkan->public_.active_frames_count) {
		active_frame_idx += 1;
		goto NEXT_ACTIVE_FRAME;
	}

	return HERO_SUCCESS;
}

HeroVulkanResource* _hero_vulkan_frame_graph_vulkan_resource_get(HeroFrameGraphVulkan* frame_graph_vulkan, HeroPassResource* resource, U32 active_frame_idx) {
	if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT) {
		return &frame_graph_vulkan->persistent_vulkan_resources[resource->physical_resource_idx];
	} else {
		return &frame_graph_vulkan->active_frames_vulkan_resources[active_frame_idx * frame_graph_vulkan->public_.physical_resources_cap + resource->physical_resource_idx];
	}
}

HeroResult _hero_vulkan_frame_graph_init_frame_buffer(HeroLogicalDevice* ldev, HeroFrameGraphVulkan* frame_graph_vulkan, HeroPass* pass, HeroPassVulkan* pass_vulkan) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	U32 active_frame_idx = 0;
	U32 swapchain_image_idx = 0;
	U32 swapchain_images_count = 0;

	HeroRangeU16 color_output_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT];
	HeroRangeU16 depth_stencil_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT];
	U32 color_output_attachments_count = color_output_attachments_range.end_idx - color_output_attachments_range.start_idx;
	U32 attachments_count = color_output_attachments_count + depth_stencil_attachments_range.end_idx - depth_stencil_attachments_range.start_idx;
	HeroPassResource* pass_resources = frame_graph_vulkan->public_.pass_resources;
	VkImageView* vk_attachments = hero_alloc_array(VkImageView, hero_system_alctor, 0, attachments_count);

NEXT_ACTIVE_FRAME: {}
NEXT_SWAPCHAIN_IMAGE: {}
	for_range(idx, 0, color_output_attachments_count) {
		HeroPassResource* resource = &pass_resources[color_output_attachments_range.start_idx + idx];
		HERO_DEBUG_ASSERT(!HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type), "internal error: expected an image resource here but got a buffer\n");
		if (hero_object_id(HeroImageInfoId, user_bits)(resource->data.image_info_id) & HERO_IMAGE_INFO_FLAGS_IS_SWAPCHAIN) {
			HeroImageInfo* image_info;
			result = _hero_image_info_get(ldev, pass_resources[color_output_attachments_range.start_idx].data.image_info_id, &image_info);
			if (result < 0) {
				return result;
			}

			pass_vulkan->swapchain_id = image_info->swapchain_id;
			HeroSwapchainVulkan* swapchain_vulkan;
			result = hero_object_pool(HeroSwapchainVulkan, get)(&ldev_vulkan->swapchain_pool, image_info->swapchain_id, &swapchain_vulkan);
			if (result < 0) {
				return result;
			}
			HERO_DEBUG_ASSERT(swapchain_images_count == 0 || swapchain_images_count == swapchain_vulkan->public_.images_count, "internal error: multiple swapchains used in a single pass");
			swapchain_images_count = swapchain_vulkan->public_.images_count;

			vk_attachments[idx] = swapchain_vulkan->image_views[swapchain_image_idx];
		} else {
			HeroVulkanResource* vulkan_resource = _hero_vulkan_frame_graph_vulkan_resource_get(frame_graph_vulkan, resource, active_frame_idx);

			vk_attachments[idx] = vulkan_resource->image_view;
		}
	}

	if (depth_stencil_attachments_range.start_idx != depth_stencil_attachments_range.end_idx) {
		HeroPassResource* resource = &pass_resources[depth_stencil_attachments_range.start_idx];
		HERO_DEBUG_ASSERT(!HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type), "internal error: expected an image resource here but got a buffer\n");
		HeroVulkanResource* vulkan_resource = _hero_vulkan_frame_graph_vulkan_resource_get(frame_graph_vulkan, resource, active_frame_idx);

		vk_attachments[color_output_attachments_count] = vulkan_resource->image_view;
	}

	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan;
	result = hero_object_pool(HeroRenderPassLayoutVulkan, get)(&ldev_vulkan->render_pass_layout_pool, pass->render_pass_layout_id, &render_pass_layout_vulkan);
	if (result < 0) {
		return result;
	}

	HeroImageInfo* image_info;
	result = _hero_image_info_get(ldev, pass_resources[color_output_attachments_range.start_idx].data.image_info_id, &image_info);
	if (result < 0) {
		return result;
	}

	pass_vulkan->frame_buffer_image_width = image_info->width;
	pass_vulkan->frame_buffer_image_height = image_info->height;

	VkFramebufferCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.renderPass = render_pass_layout_vulkan->handle,
		.attachmentCount = attachments_count,
		.pAttachments = vk_attachments,
		.width = image_info->width,
		.height = image_info->height,
		.layers = image_info->array_layers_count,
	};

	VkFramebuffer vk_frame_buffer;
	vk_result = ldev_vulkan->vkCreateFramebuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_frame_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkFramebuffer* frame_buffer_ptr = &frame_graph_vulkan->active_frames_frame_buffers[active_frame_idx * frame_graph_vulkan->public_.physical_resources_cap + pass->pass_enum];

	if (swapchain_images_count) {
		VkFramebuffer* swapchain_frame_buffers = *(VkFramebuffer**)frame_buffer_ptr;
		if (active_frame_idx == 0) {
			swapchain_frame_buffers = hero_alloc_array(VkFramebuffer, hero_system_alctor, 0, swapchain_images_count);
			if (!swapchain_frame_buffers) {
				return HERO_ERROR(ALLOCATION_FAILURE);
			}
			*(VkFramebuffer**)frame_buffer_ptr = swapchain_frame_buffers;
		}
		swapchain_frame_buffers[swapchain_image_idx] = vk_frame_buffer;
	} else {
		*frame_buffer_ptr = vk_frame_buffer;
	}

	if (swapchain_image_idx < swapchain_images_count) {
		swapchain_image_idx += 1;
		goto NEXT_SWAPCHAIN_IMAGE;
	}

	if (active_frame_idx < frame_graph_vulkan->public_.active_frames_count) {
		active_frame_idx += 1;
		swapchain_images_count = 0;
		goto NEXT_ACTIVE_FRAME;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_graph_init_render_pass(HeroLogicalDevice* ldev, HeroFrameGraphVulkan* frame_graph_vulkan, HeroPass* pass, HeroPassVulkan* pass_vulkan) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameGraph* frame_graph = &frame_graph_vulkan->public_;

	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan;
	result = hero_object_pool(HeroRenderPassLayoutVulkan, get)(&ldev_vulkan->render_pass_layout_pool, pass->render_pass_layout_id, &render_pass_layout_vulkan);
	if (result < 0) {
		return result;
	}

	pass_vulkan->clear_count = 0;

	HeroRangeU16 color_output_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT];
	HeroRangeU16 depth_stencil_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT];
	HeroRangeU16 input_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT];
	U32 color_output_attachments_count = color_output_attachments_range.end_idx - color_output_attachments_range.start_idx;
	U32 input_attachments_count = input_attachments_range.end_idx - input_attachments_range.start_idx;
	U32 depth_stencil_attachments_count = depth_stencil_attachments_range.end_idx - depth_stencil_attachments_range.start_idx;
	HERO_DEBUG_ASSERT(depth_stencil_attachments_count <= 1, "internal error: we should have a maximum of 1 depth stencil attachment");
	U32 attachments_count = color_output_attachments_count + input_attachments_count + depth_stencil_attachments_count;

	VkAttachmentDescription* vk_attachments;
	VkAttachmentReference* vk_attachment_refs;
	VkAttachmentReference* vk_color_output_attachment_refs = NULL;
	VkAttachmentReference* vk_input_attachment_refs = NULL;
	VkAttachmentReference* vk_depth_stencil_attachment_ref = NULL;
	VkSubpassDescription vk_subpass = {0};
	{
		vk_attachments = hero_alloc_array(VkAttachmentDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_DESCRIPTION, attachments_count);
		if (vk_attachments == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_attachment_refs = hero_alloc_array(VkAttachmentReference, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_DESCRIPTION, attachments_count);
		if (vk_attachment_refs == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		if (color_output_attachments_count) {
			vk_color_output_attachment_refs = vk_attachment_refs;
		}

		if (input_attachments_count) {
			vk_input_attachment_refs = &vk_attachment_refs[color_output_attachments_count];
		}

		if (depth_stencil_attachments_count) {
			vk_depth_stencil_attachment_ref = &vk_attachment_refs[color_output_attachments_count + input_attachments_count];
		}

		//
		// color outputs attachments
		for_range(color_output_idx, color_output_attachments_range.start_idx, color_output_attachments_range.end_idx) {
			HeroPassResource* resource = &frame_graph->pass_resources[color_output_idx];

			HeroImageInfo* image_info;
			result = _hero_image_info_get(ldev, resource->data.image_info_id, &image_info);
			if (result < 0) {
				return result;
			}

			U32 attachment_idx = color_output_idx - color_output_attachments_range.start_idx;

			VkImageLayout image_layout = image_info->swapchain_id.raw ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentLoadOp load_op;
			if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT || resource->link_pass_enum != HERO_PASS_ENUM_INVALID) {
				load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
			} else if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_CLEARED) {
				load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
				pass_vulkan->clear_count += 1;
			} else {
				load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}

			VkAttachmentDescription* attachment = &vk_attachments[attachment_idx];
			*attachment = (VkAttachmentDescription) {
				.flags = 0,
				.format = _hero_vulkan_convert_to_format[image_info->format],
				.samples = 1 << image_info->samples_count_log2,
				.loadOp = load_op,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = image_layout,
			};

			VkAttachmentReference* ref = &vk_attachment_refs[attachment_idx];
			ref->attachment = attachment_idx;
			ref->layout = image_layout;
		}

		//
		// input attachments
		for_range(input_idx, input_attachments_range.start_idx, input_attachments_range.end_idx) {
			HeroPassResource* resource = &frame_graph->pass_resources[input_idx];

			HeroImageInfo* image_info;
			result = _hero_image_info_get(ldev, resource->data.image_info_id, &image_info);
			if (result < 0) {
				return result;
			}

			U32 attachment_idx = input_idx - input_attachments_range.start_idx;
			attachment_idx += color_output_attachments_count; // input attachments start after the color output attachments

			VkAttachmentDescription* attachment = &vk_attachments[attachment_idx];
			*attachment = (VkAttachmentDescription) {
				.flags = 0,
				.format = _hero_vulkan_convert_to_format[image_info->format],
				.samples = 1 << image_info->samples_count_log2,
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};

			VkAttachmentReference* ref = &vk_input_attachment_refs[attachment_idx];
			ref->attachment = attachment_idx;
			ref->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		//
		// depth stencil attachment
		if (depth_stencil_attachments_count) {
			HeroPassResource* resource = &frame_graph->pass_resources[depth_stencil_attachments_range.start_idx];

			HeroImageInfo* image_info;
			result = _hero_image_info_get(ldev, resource->data.image_info_id, &image_info);
			if (result < 0) {
				return result;
			}

			U32 attachment_idx = depth_stencil_attachments_range.start_idx;
			attachment_idx += color_output_attachments_count + input_attachments_count; // input attachments start after the color output & input attachments

			VkAttachmentLoadOp load_op;
			if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT || resource->link_pass_enum != HERO_PASS_ENUM_INVALID) {
				load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
			} else if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_CLEARED) {
				load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
			} else {
				load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}

			VkAttachmentDescription* attachment = &vk_attachments[attachment_idx];
			*attachment = (VkAttachmentDescription) {
				.flags = 0,
				.format = _hero_vulkan_convert_to_format[image_info->format],
				.samples = 1 << image_info->samples_count_log2,
				.loadOp = load_op,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			};

			VkAttachmentReference* ref = &vk_attachment_refs[attachment_idx];
			ref->attachment = attachment_idx;
			ref->layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}

		vk_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vk_subpass.inputAttachmentCount = input_attachments_count;
		vk_subpass.pInputAttachments = vk_input_attachment_refs;
		vk_subpass.colorAttachmentCount = color_output_attachments_count;
		vk_subpass.pColorAttachments = vk_color_output_attachment_refs;
		vk_subpass.pDepthStencilAttachment = vk_depth_stencil_attachment_ref;
	}

	VkRenderPassCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pAttachments = vk_attachments,
		.attachmentCount = attachments_count,
		.pSubpasses = &vk_subpass,
		.subpassCount = 1,
		.pDependencies = NULL,
		.dependencyCount = 0,
	};

	VkRenderPass vk_render_pass;
	vk_result = ldev_vulkan->vkCreateRenderPass(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_render_pass);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	pass_vulkan->render_pass = vk_render_pass;
	return HERO_SUCCESS;
}

VkImageMemoryBarrier* _hero_vulkan_frame_graph_image_barrier_find_or_insert(HeroFrameGraphVulkan* frame_graph_vulkan, HeroExecutionUnitVulkan* execution_unit, U16 physical_resource_idx) {
	U16* image_barrier_physical_resource_indices = frame_graph_vulkan->execution_units_image_barrier_physical_resource_indices;
	VkImageMemoryBarrier* image_barriers = frame_graph_vulkan->execution_units_image_barriers;
	U16 end_idx = execution_unit->resource_barriers_start_idx + execution_unit->image_barriers_count;
	for_range(idx, execution_unit->resource_barriers_start_idx, end_idx) {
		if (image_barrier_physical_resource_indices[idx] == physical_resource_idx) {
			return &image_barriers[idx];
		}
	}

	image_barrier_physical_resource_indices[end_idx] = physical_resource_idx;
	VkImageMemoryBarrier* image_barrier = &image_barriers[end_idx];
	execution_unit->image_barriers_count += 1;
	return image_barrier;
}

VkBufferMemoryBarrier* _hero_vulkan_frame_graph_buffer_barrier_find_or_insert(HeroFrameGraphVulkan* frame_graph_vulkan, HeroExecutionUnitVulkan* execution_unit, U16 physical_resource_idx) {
	U16* buffer_barrier_physical_resource_indices = frame_graph_vulkan->execution_units_buffer_barrier_physical_resource_indices;
	VkBufferMemoryBarrier* buffer_barriers = frame_graph_vulkan->execution_units_buffer_barriers;
	U16 end_idx = execution_unit->resource_barriers_start_idx + execution_unit->buffer_barriers_count;
	for_range(idx, execution_unit->resource_barriers_start_idx, end_idx) {
		if (buffer_barrier_physical_resource_indices[idx] == physical_resource_idx) {
			return &buffer_barriers[idx];
		}
	}

	buffer_barrier_physical_resource_indices[end_idx] = physical_resource_idx;
	VkBufferMemoryBarrier* buffer_barrier = &buffer_barriers[end_idx];
	execution_unit->buffer_barriers_count += 1;
	return buffer_barrier;
}

void _hero_vulkan_frame_graph_update_resource_barrier(HeroFrameGraphVulkan* frame_graph_vulkan, HeroExecutionUnitVulkan* execution_unit, HeroPassResource* resource, bool is_compute, bool is_destination) {
	VkPipelineStageFlags* stage_flags;
	VkAccessFlags*        access_flags;
	VkImageLayout*        layout;
	if (HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type)) {
		VkBufferMemoryBarrier* buffer_barrier = _hero_vulkan_frame_graph_buffer_barrier_find_or_insert(frame_graph_vulkan, execution_unit, resource->physical_resource_idx);

		if (is_destination) {
			stage_flags = &execution_unit->dst_stage_flags;
			access_flags = &buffer_barrier->dstAccessMask;
			layout = NULL;
		} else {
			stage_flags = &execution_unit->src_stage_flags;
			access_flags = &buffer_barrier->srcAccessMask;
			layout = NULL;
		}
	} else {
		VkImageMemoryBarrier* image_barrier = _hero_vulkan_frame_graph_image_barrier_find_or_insert(frame_graph_vulkan, execution_unit, resource->physical_resource_idx);

		if (is_destination) {
			stage_flags = &execution_unit->dst_stage_flags;
			access_flags = &image_barrier->dstAccessMask;
			layout = &image_barrier->newLayout;
		} else {
			stage_flags = &execution_unit->src_stage_flags;
			access_flags = &image_barrier->srcAccessMask;
			layout = &image_barrier->oldLayout;
		}
	}

	HERO_ASSERT(*layout == VK_IMAGE_LAYOUT_UNDEFINED, "TODO: work out how to handle this instance where the image layout has been set twice to something different");

	//
	// TODO some of the resource can be done later but we need to keep
	// track of when the resource is being using in the pipeline.
	switch (resource->type) {
		case HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT:
			*stage_flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			*access_flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			*layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT:
			*stage_flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			*access_flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			*layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT:
			*stage_flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			*access_flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			*layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			break;
		case HERO_PASS_RESOURCE_TYPE_RO_IMAGE:
		case HERO_PASS_RESOURCE_TYPE_RO_BUFFER:
			*stage_flags |= is_compute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			*access_flags |= VK_ACCESS_SHADER_READ_BIT;
			*layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case HERO_PASS_RESOURCE_TYPE_RW_IMAGE:
		case HERO_PASS_RESOURCE_TYPE_RW_BUFFER:
			*stage_flags |= is_compute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			*access_flags |= VK_ACCESS_SHADER_WRITE_BIT;
			*layout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		case HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER:
			*stage_flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			*access_flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			break;
		case HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER:
			*stage_flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			*access_flags |= VK_ACCESS_INDEX_READ_BIT;
			break;
	}
}

HeroResult _hero_vulkan_frame_graph_update(HeroLogicalDevice* ldev, HeroFrameGraph* frame_graph) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameGraphVulkan* frame_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameGraphVulkan, frame_graph);

	//
	// deallocate the old physical resources that are no longer used
	Uptr physical_resource_idx = -1;
	while (hero_bitset_array_iter_next_one64(frame_graph->physical_resources_that_are_unused_bitset, frame_graph->physical_resources_cap, &physical_resource_idx)) {
		HeroPassPhysicalResource* physical_resource = &frame_graph->physical_resources[physical_resource_idx];
		VkObjectType object_type = physical_resource->is_buffer ? VK_OBJECT_TYPE_BUFFER : VK_OBJECT_TYPE_IMAGE;

		if (physical_resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_PERSISTENT) {
			HeroVulkanResource* vulkan_resource = &frame_graph_vulkan->persistent_vulkan_resources[physical_resource_idx];
			HeroVulkanObjectDealloc* dealloc;
			result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, object_type, 1, &dealloc);
			if (result < 0) {
				return result;
			}
			if (physical_resource->is_buffer) {
				dealloc->buffer = vulkan_resource->buffer;
			} else {
				dealloc[0].image = vulkan_resource->image;
				dealloc[1].image_view = vulkan_resource->image_view;
			}
		} else {
			HeroVulkanObjectDealloc* dealloc;
			result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, object_type, frame_graph->active_frames_count, &dealloc);
			if (result < 0) {
				return result;
			}
			for_range(active_frame_idx, 0, frame_graph->active_frames_count) {
				HeroVulkanResource* vulkan_resource = &frame_graph_vulkan->active_frames_vulkan_resources[active_frame_idx * frame_graph->physical_resources_cap + physical_resource_idx];
				if (physical_resource->is_buffer) {
					dealloc->buffer = vulkan_resource->buffer;
				} else {
					dealloc[0].image = vulkan_resource->image;
					dealloc[1].image_view = vulkan_resource->image_view;
				}
				dealloc += 1;
			}
		}
	}

	//
	// initialize any new resources that have been newly been added to the frame graph this frame
	physical_resource_idx = -1;
	while (hero_bitset_array_iter_next_one64(frame_graph->physical_resources_newly_allocated_bitset, frame_graph->physical_resources_cap, &physical_resource_idx)) {
		HeroPassPhysicalResource* physical_resource = &frame_graph->physical_resources[physical_resource_idx];
		if (physical_resource->is_buffer) {
			_hero_vulkan_frame_graph_init_buffer(ldev, frame_graph_vulkan, physical_resource);
		} else {
			_hero_vulkan_frame_graph_init_image(ldev, frame_graph_vulkan, physical_resource);
		}
	}

	//
	// deallocate the old passes that are no longer used
	Uptr pass_enum = -1;
	while (hero_bitset_array_iter_next_one64(frame_graph->passes_that_are_unused_bitset, frame_graph->passes_cap, &pass_enum)) {
		HeroPass* pass = &frame_graph->passes[pass_enum];

		if (pass->render_pass_layout_id.raw == 0) {
			// compute passes have no frame buffer or render passes so skip
			continue;
		}

		HeroPassVulkan* pass_vulkan = &frame_graph_vulkan->passes[pass_enum];

		HeroVulkanObjectDealloc* dealloc;
		result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, VK_OBJECT_TYPE_RENDER_PASS, 1, &dealloc);
		if (result < 0) {
			return result;
		}
		dealloc->render_pass = pass_vulkan->render_pass;

		U32 swapchain_images_count = 1;
		if (pass->flags & HERO_PASS_FLAGS_HAS_SWAPCHAIN) {
			HeroSwapchain* swapchain = NULL;
			result = hero_swapchain_get(ldev, pass_vulkan->swapchain_id, &swapchain);
			if (result < 0) {
				return result;
			}
			swapchain_images_count = swapchain->images_count;
		}

		result = _hero_vulkan_dealloc_object_once_next_frame_has_executed(ldev_vulkan, VK_OBJECT_TYPE_FRAMEBUFFER, frame_graph->active_frames_count * swapchain_images_count, &dealloc);
		if (result < 0) {
			return result;
		}

		for_range(active_frame_idx, 0, frame_graph->active_frames_count) {
			VkFramebuffer* frame_buffer = &frame_graph_vulkan->active_frames_frame_buffers[active_frame_idx * frame_graph->passes_cap + pass_enum];
			if (pass->flags & HERO_PASS_FLAGS_HAS_SWAPCHAIN) {
				VkFramebuffer* frame_buffers = *(VkFramebuffer**)frame_buffer;
				for_range(image_idx, 0, swapchain_images_count) {
					dealloc[image_idx].frame_buffer = frame_buffers[image_idx];
				}
				dealloc += swapchain_images_count;
			} else {
				dealloc->frame_buffer = *frame_buffer;
				dealloc += 1;
			}
		}
	}

	//
	// initialize any new passes that have been newly been added to the frame graph this frame
	pass_enum = -1;
	while (hero_bitset_array_iter_next_one64(frame_graph->passes_newly_allocated_bitset, frame_graph->passes_cap, &pass_enum)) {
		HeroPass* pass = &frame_graph->passes[pass_enum];
		HeroPassVulkan* pass_vulkan = &frame_graph_vulkan->passes[pass_enum];

		if (pass->render_pass_layout_id.raw == 0) {
			// compute passes have no frame buffer or render passes so skip
			continue;
		}

		result = _hero_vulkan_frame_graph_init_frame_buffer(ldev, frame_graph_vulkan, pass, pass_vulkan);
		if (result < 0) {
			return result;
		}

		result = _hero_vulkan_frame_graph_init_render_pass(ldev, frame_graph_vulkan, pass, pass_vulkan);
		if (result < 0) {
			return result;
		}
	}

	//
	// work out all of the synchronization that needs to happen at each execution unit
	HERO_DEBUG_ASSERT(frame_graph->execution_units_count, "internal error: expected to have atleast 1 execution unit");
	{
		HERO_ZERO_ELMT_MANY(frame_graph_vulkan->execution_units_image_barriers, frame_graph->pass_resources_cap);
		HERO_ZERO_ELMT_MANY(frame_graph_vulkan->execution_units_buffer_barriers, frame_graph->pass_resources_cap);

		{
			HeroExecutionUnitVulkan* execution_unit = &frame_graph_vulkan->execution_units[0];
			execution_unit->resource_barriers_start_idx = 0;
		}

		//
		// for every execution unit
		// set all the _destination_ flags for the resource barriers in the _current_ execution unit.
		// this is how the resource is being used after the barrier.
		Uptr resource_barriers_start_idx = 0;
		for_range(execution_unit_idx, 0, frame_graph->execution_units_count) {
			HeroExecutionUnitVulkan* execution_unit = &frame_graph_vulkan->execution_units[execution_unit_idx];

			HeroRangeU16 execution_unit_passes_range = frame_graph->execution_units_pass_ranges[execution_unit_idx];

			//
			// start by setting up the resource barrier start index
			execution_unit->resource_barriers_start_idx = resource_barriers_start_idx;

			//
			// now lets offset the resource barrier start index by the total number
			// of resources in the execution unit's passes.
			//
			// essentially we are reserving the maximum space for if all resources
			// need resource barrier (which they won't). as this makes it easier
			// for the code after this.
			//
			// for every execution unit pass
			for_range(execution_units_pass_idx, execution_unit_passes_range.start_idx, execution_unit_passes_range.end_idx) {
				HeroPass* pass = &frame_graph->passes[frame_graph->execution_units_pass_enums[execution_units_pass_idx]];

				Uptr start_idx = pass->resource_ranges_by_type[0].start_idx;
				Uptr end_idx = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COUNT - 1].end_idx;
				Uptr resources_count = end_idx - start_idx;

				resource_barriers_start_idx += resources_count;
			}

			//
			// build the resource barriers for this execution unit and the ones the resources link too.
			//
			// for every execution unit pass
			for_range(execution_units_pass_idx, execution_unit_passes_range.start_idx, execution_unit_passes_range.end_idx) {
				HeroPass* pass = &frame_graph->passes[frame_graph->execution_units_pass_enums[execution_units_pass_idx]];
				HeroPassVulkan* pass_vulkan = &frame_graph_vulkan->passes[pass->pass_enum];
				bool is_compute = pass->render_pass_layout_id.raw;

				//
				// for every pass resource
				Uptr start_idx = pass->resource_ranges_by_type[0].start_idx;
				Uptr end_idx = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COUNT - 1].end_idx;
				for_range(resource_idx, start_idx, end_idx) {
					HeroPassResource* resource = &frame_graph->pass_resources[resource_idx];
					HeroPassPhysicalResource* physical_resource = &frame_graph->physical_resources[resource->physical_resource_idx];

					bool links_to_prev_execution_unit = resource->link_pass_enum != HERO_PASS_ENUM_INVALID;
					if (links_to_prev_execution_unit) {
						_hero_vulkan_frame_graph_update_resource_barrier(frame_graph_vulkan, execution_unit, resource, is_compute, true);
					}

					bool links_to_next_execution_unit = resource->link_chain_length > 0;
					if (links_to_next_execution_unit) {
						if (resource->link_to_by_min_execution_unit_idx <= execution_unit_idx) {
							HERO_ABORT("TODO links to previous frame, use a semaphore to sync");
						} else {
							HeroExecutionUnitVulkan* next_execution_unit = &frame_graph_vulkan->execution_units[resource->link_to_by_min_execution_unit_idx];

							_hero_vulkan_frame_graph_update_resource_barrier(frame_graph_vulkan, next_execution_unit, resource, is_compute, false);
						}
					}
				}
			}
		}
	}

	frame_graph_vulkan->num_frames_to_descriptor_sets = frame_graph->active_frames_count;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_graph_record_pass_start(HeroLogicalDevice* ldev, HeroFrameGraph* frame_graph, HeroPass* pass, _HeroCommandRecorder* command_recorder) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameGraphVulkan* frame_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameGraphVulkan, frame_graph);

	VkCommandBuffer vk_command_buffer = frame_graph_vulkan->active_frames_command_buffers[frame_graph->active_frame_idx * frame_graph->passes_cap + pass->pass_enum];
	command_recorder->vulkan.command_buffer = vk_command_buffer;
	{
		//
		// TODO: this currently resets the command buffer too implicitly.
		// I have heard that we should explicitly reset these buffers together
		// on the same thread at some point when they are no longer used by the GPU.
		// maybe think about having a command pool per active frame and just resetting the whole pool but we might have to realloc the buffers aggain.

		VkCommandBufferBeginInfo vk_command_buffer_begin_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // command buffer is reset after a single submit
			.pInheritanceInfo = NULL,
		};

		vk_result = ldev_vulkan->vkBeginCommandBuffer(vk_command_buffer, &vk_command_buffer_begin_info);
		if (vk_result < 0) {
			return _hero_vulkan_convert_from_result(vk_result);
		}
	}

	HeroPassEnum first_execution_unit_pass_enum = frame_graph->execution_units_pass_enums[frame_graph->execution_units_pass_ranges[pass->execution_unit_idx].start_idx];
	if (first_execution_unit_pass_enum == pass->pass_enum) {

		HeroExecutionUnitVulkan* execution_unit = &frame_graph_vulkan->execution_units[pass->execution_unit_idx];

		ldev_vulkan->vkCmdPipelineBarrier(
				vk_command_buffer,
				execution_unit->src_stage_flags,
				execution_unit->dst_stage_flags,
				VK_DEPENDENCY_BY_REGION_BIT,
				0,    // memoryBarrierCount
				NULL, // pMemoryBarriers
				execution_unit->buffer_barriers_count,
				&frame_graph_vulkan->execution_units_buffer_barriers[execution_unit->resource_barriers_start_idx],
				execution_unit->image_barriers_count,
				&frame_graph_vulkan->execution_units_image_barriers[execution_unit->resource_barriers_start_idx]
			);
	}

	//
	// if we are a render pass, then begin & setup the render pass
	if (pass->render_pass_layout_id.raw) {
		HeroPassVulkan* pass_vulkan = &frame_graph_vulkan->passes[pass->pass_enum];

		VkFramebuffer vk_frame_buffer = frame_graph_vulkan->active_frames_frame_buffers[frame_graph->active_frame_idx * frame_graph->passes_cap + pass->pass_enum];
		if (pass_vulkan->swapchain_id.raw) {
			HeroSwapchainVulkan* swapchain_vulkan;
			result = hero_object_pool(HeroSwapchainVulkan, get)(&ldev_vulkan->swapchain_pool, pass_vulkan->swapchain_id, &swapchain_vulkan);
			if (result < 0) {
				return result;
			}

			VkFramebuffer* swapchain_frame_buffers = (VkFramebuffer*)vk_frame_buffer;
			vk_frame_buffer = swapchain_frame_buffers[swapchain_vulkan->image_idx];
		}

		//
		// convert the clear values into VkClearValue
		VkClearValue* vk_clear_values = hero_alloc_array(VkClearValue, hero_system_alctor, 0, pass_vulkan->clear_count);
		{
			U32 next_clear_value_idx = 0;
			HeroRangeU16 color_output_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT];
			HeroRangeU16 depth_stencil_attachments_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT];
			for_range(i, color_output_attachments_range.start_idx, color_output_attachments_range.end_idx) {
				HeroPassResource* resource = &frame_graph->pass_resources[i];
				if (next_clear_value_idx >= pass_vulkan->clear_count) {
					break;
				}

				if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_CLEARED) {
					VkClearValue* vk_clear = &vk_clear_values[next_clear_value_idx];
					vk_clear->color.float32[0] = resource->clear_value.color.f32[0];
					vk_clear->color.float32[1] = resource->clear_value.color.f32[1];
					vk_clear->color.float32[2] = resource->clear_value.color.f32[2];
					vk_clear->color.float32[3] = resource->clear_value.color.f32[3];
					next_clear_value_idx += 1;
				}
			}

			for_range(i, depth_stencil_attachments_range.start_idx, depth_stencil_attachments_range.end_idx) {
				HeroPassResource* resource = &frame_graph->pass_resources[i];
				if (next_clear_value_idx >= pass_vulkan->clear_count) {
					break;
				}

				if (resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_CLEARED) {
					VkClearValue* vk_clear = &vk_clear_values[next_clear_value_idx];
					vk_clear->depthStencil.depth = resource->clear_value.depth;
					vk_clear->depthStencil.stencil = resource->clear_value.stencil;
					next_clear_value_idx += 1;
				}
			}
		}

		HeroUAabb render_area = {
			.x = UINT32_MAX,
			.y = UINT32_MAX,
			.ex = 0,
			.ey = 0,
		};
		U16 viewports_count = pass->viewports_range.end_idx - pass->viewports_range.start_idx;
		VkViewport* vk_viewports = hero_alloc_array(VkViewport, hero_system_alctor, 0, viewports_count);
		VkRect2D* vk_scissors = hero_alloc_array(VkRect2D, hero_system_alctor, 0, viewports_count);
		for_range(i, pass->viewports_range.start_idx, pass->viewports_range.end_idx) {
			HeroPassViewport* pass_viewport = &frame_graph->viewports[i];

			HeroViewport* viewport = &pass_viewport->viewport;
			HeroViewport default_viewport;
			if (!pass_viewport->has_set_viewport) {
				default_viewport.x = 0.f;
				default_viewport.y = 0.f;
				default_viewport.width = pass_vulkan->frame_buffer_image_width;
				default_viewport.height = pass_vulkan->frame_buffer_image_height;
				default_viewport.min_depth = 0.f;
				default_viewport.max_depth = 1.f;
				viewport = &default_viewport;
			}

			HeroUAabb* scissor = &pass_viewport->scissor;
			HeroUAabb default_scissor;
			if (!pass_viewport->has_set_scissor) {
				default_scissor.x = 0.f;
				default_scissor.y = 0.f;
				default_scissor.ex = pass_vulkan->frame_buffer_image_width;
				default_scissor.ey = pass_vulkan->frame_buffer_image_height;
				scissor = &default_scissor;
			}

			VkViewport* vk_viewport = &vk_viewports[i];
			_hero_vulkan_convert_to_viewport(viewport, vk_viewport);
			_hero_vulkan_convert_to_rect2d(scissor, &vk_scissors[i]);

			render_area.x = HERO_MIN(render_area.x, (S32)vk_viewport->x);
			render_area.y = HERO_MIN(render_area.y, (S32)vk_viewport->y);

			S32 ex = (S32)vk_viewport->x + (S32)vk_viewport->width;
			S32 ey = (S32)vk_viewport->y + (S32)vk_viewport->height;
			render_area.ex = HERO_MAX(render_area.ex, ex);
			render_area.ey = HERO_MAX(render_area.ey, ey);
		}

		VkRect2D vk_render_area = {
			.offset.x = render_area.x,
			.offset.y = render_area.y,
			.extent.width = render_area.ex - render_area.x,
			.extent.height = render_area.ey - render_area.y,
		};

		VkRenderPassBeginInfo render_pass_being_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = NULL,
			.renderPass = pass_vulkan->render_pass,
			.framebuffer = vk_frame_buffer,
			.renderArea = vk_render_area,
			.clearValueCount = pass_vulkan->clear_count,
			.pClearValues = vk_clear_values,
		};

		ldev_vulkan->vkCmdBeginRenderPass(vk_command_buffer, &render_pass_being_info, VK_SUBPASS_CONTENTS_INLINE);

		ldev_vulkan->vkCmdSetViewport(vk_command_buffer, 0, viewports_count, vk_viewports);
		ldev_vulkan->vkCmdSetScissor(vk_command_buffer, 0, viewports_count, vk_scissors);
	}

	//
	// look at all of the read write image resources for this pass and clear them if they requested to be cleared
	HeroRangeU16 rw_image_resource_range = pass->resource_ranges_by_type[HERO_PASS_RESOURCE_TYPE_RW_IMAGE];
	for_range(i, rw_image_resource_range.start_idx, rw_image_resource_range.end_idx) {
		HeroPassResource* resource = &frame_graph->pass_resources[i];
		HERO_DEBUG_ASSERT(resource->type == HERO_PASS_RESOURCE_TYPE_RW_IMAGE, "internal error: expected the resource to be HERO_PASS_RESOURCE_TYPE_RW_IMAGE");

		if (!(resource->flags & HERO_PASS_RESOURCE_FLAGS_IS_CLEARED)) {
			continue;
		}

		HeroImageInfo* info;
		result = _hero_image_info_get(ldev, resource->data.image_info_id, &info);
		if (result < 0) {
			return result;
		}

		HeroVulkanResource* vulkan_resource = _hero_vulkan_frame_graph_vulkan_resource_get(frame_graph_vulkan, resource, frame_graph->active_frame_idx);
		if (HERO_IMAGE_FORMAT_IS_DEPTH(info->format)) {
			VkClearDepthStencilValue vk_clear_value = {
				.depth = resource->clear_value.depth,
				.stencil = resource->clear_value.stencil,
			};

			ldev_vulkan->vkCmdClearDepthStencilImage(
				vk_command_buffer,
				vulkan_resource->image,
				VK_IMAGE_LAYOUT_GENERAL,
				&vk_clear_value,
				0,
				NULL);
		} else {
			VkClearColorValue vk_clear_value;
			vk_clear_value.float32[0] = resource->clear_value.color.f32[0];
			vk_clear_value.float32[1] = resource->clear_value.color.f32[1];
			vk_clear_value.float32[2] = resource->clear_value.color.f32[2];
			vk_clear_value.float32[3] = resource->clear_value.color.f32[3];

			ldev_vulkan->vkCmdClearColorImage(
				vk_command_buffer,
				vulkan_resource->image,
				VK_IMAGE_LAYOUT_GENERAL,
				&vk_clear_value,
				0,
				NULL);
		}
	}

	command_recorder->vulkan.bound_pipeline_graphics.raw = 0;
	command_recorder->vulkan.bound_pipeline_compute.raw = 0;
	for_range(i, 0, HERO_BUFFER_BINDINGS_CAP) {
		command_recorder->vulkan.bound_vertex_buffers[i] = VK_NULL_HANDLE;
		command_recorder->vulkan.bound_vertex_buffer_offsets[i] = 0;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_graph_record_pass_end(_HeroCommandRecorder* command_recorder) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->public_.ldev;

	VkCommandBuffer vk_command_buffer = command_recorder->vulkan.command_buffer;

	//
	// if we are a render pass, then begin & setup the render pass
	if (command_recorder->public_.pass->render_pass_layout_id.raw) {
		ldev_vulkan->vkCmdEndRenderPass(vk_command_buffer);
	}

	vk_result = ldev_vulkan->vkEndCommandBuffer(vk_command_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_frame_graph_submit(HeroLogicalDevice* ldev, HeroFrameGraph* frame_graph) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroFrameGraphVulkan* frame_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameGraphVulkan, frame_graph);

	if (frame_graph_vulkan->num_frames_to_descriptor_sets) {
		frame_graph_vulkan->num_frames_to_descriptor_sets -= 1;
		VkDescriptorSet vk_descriptor_set = frame_graph_vulkan->active_frames[frame_graph->active_frame_idx].descriptor_set;

		VkDescriptorImageInfo* samplers;
		VkDescriptorImageInfo* ro_image_1ds;
		VkDescriptorImageInfo* ro_image_2ds;
		VkDescriptorImageInfo* ro_image_3ds;
		VkDescriptorImageInfo* rw_image_1ds;
		VkDescriptorImageInfo* rw_image_2ds;
		VkDescriptorImageInfo* rw_image_3ds;

		//
		// allocate the update arrays for the resources and set them all to the null resource
		{
			{
				samplers = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, ldev_vulkan->sampler_pool.cap);
				if (samplers == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, ldev_vulkan->sampler_pool.cap) {
					VkDescriptorImageInfo* info = &samplers[i];
					info->sampler = ldev_vulkan->null_sampler;
				}
			}

			{
				ro_image_1ds = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, frame_graph->pass_resources_cap);
				if (ro_image_1ds == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, frame_graph->pass_resources_cap) {
					VkDescriptorImageInfo* info = &ro_image_1ds[i];
					info->imageView = ldev_vulkan->null_image_view_1d;
					info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}

			{
				ro_image_2ds = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, frame_graph->pass_resources_cap);
				if (ro_image_2ds == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, frame_graph->pass_resources_cap) {
					VkDescriptorImageInfo* info = &ro_image_2ds[i];
					info->imageView = ldev_vulkan->null_image_view_2d;
					info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}

			{
				ro_image_3ds = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, frame_graph->pass_resources_cap);
				if (ro_image_3ds == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, frame_graph->pass_resources_cap) {
					VkDescriptorImageInfo* info = &ro_image_3ds[i];
					info->imageView = ldev_vulkan->null_image_view_3d;
					info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}

			{
				rw_image_1ds = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, frame_graph->pass_resources_cap);
				if (rw_image_1ds == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, frame_graph->pass_resources_cap) {
					VkDescriptorImageInfo* info = &rw_image_1ds[i];
					info->imageView = ldev_vulkan->null_image_view_1d;
					info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}

			{
				rw_image_2ds = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, frame_graph->pass_resources_cap);
				if (rw_image_2ds == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, frame_graph->pass_resources_cap) {
					VkDescriptorImageInfo* info = &rw_image_2ds[i];
					info->imageView = ldev_vulkan->null_image_view_2d;
					info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}

			{
				rw_image_3ds = hero_alloc_array(VkDescriptorImageInfo, hero_system_alctor, 0, frame_graph->pass_resources_cap);
				if (rw_image_3ds == NULL) {
					return HERO_ERROR(ALLOCATION_FAILURE);
				}

				for_range(i, 0, frame_graph->pass_resources_cap) {
					VkDescriptorImageInfo* info = &rw_image_2ds[i];
					info->imageView = ldev_vulkan->null_image_view_3d;
					info->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
			}
		}

		//
		// now set all of the resources in the update arrays
		{
			HeroSamplerVulkan* sampler = NULL;
			HeroSamplerId sampler_id = {0};
			while (hero_object_pool(HeroSamplerVulkan, iter_next)(&ldev_vulkan->sampler_pool, &sampler_id, &sampler) != HERO_SUCCESS_FINISHED) {
				U32 idx = hero_object_id(HeroSamplerId, idx)(sampler_id);
				VkDescriptorImageInfo* info = &samplers[idx];
				info->sampler = sampler->handle;
			}

			for_range(resource_idx, 0, frame_graph->pass_resources_cap) {
				HeroPassResource* resource = &frame_graph->pass_resources[resource_idx];
				if (HERO_PASS_RESOURCE_TYPE_IS_BUFFER(resource->type)) {
					continue;
				}

				bool is_rw = HERO_PASS_RESOURCE_TYPE_IS_MUTABLE(resource->type);

				HeroImageInfo* info;
				result = _hero_image_info_get(ldev, resource->data.image_info_id, &info);
				if (result < 0) {
					return result;
				}

				VkDescriptorImageInfo* infos;
				switch (info->type) {
					case HERO_IMAGE_TYPE_1D:
						if (is_rw) infos = rw_image_1ds;
						else       infos = ro_image_1ds;
						break;
					case HERO_IMAGE_TYPE_2D:
						if (is_rw) infos = rw_image_2ds;
						else       infos = ro_image_2ds;
						break;
					case HERO_IMAGE_TYPE_3D:
						if (is_rw) infos = rw_image_3ds;
						else       infos = ro_image_3ds;
						break;
				}

				HeroVulkanResource* vulkan_resource = _hero_vulkan_frame_graph_vulkan_resource_get(frame_graph_vulkan, resource, frame_graph->active_frame_idx);

				VkImageLayout image_layout = is_rw ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				infos[resource_idx].imageView = vulkan_resource->image_view;
				infos[resource_idx].imageLayout = image_layout;
			}
		}

		VkWriteDescriptorSet vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_COUNT];
		for_range(binding, 0, HERO_VULKAN_DESCRIPTOR_BINDING_COUNT) {
			VkWriteDescriptorSet* write = &vk_write_descriptor_sets[binding];
			write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write->pNext = NULL;
			write->dstSet = vk_descriptor_set;
			write->dstBinding = binding;
			write->dstArrayElement = 0;
			write->descriptorCount = binding == HERO_VULKAN_DESCRIPTOR_BINDING_SAMPLERS ? ldev_vulkan->sampler_pool.cap : frame_graph->pass_resources_cap;
			write->descriptorType = hero_vulkan_descriptor_binding_types[binding];
			write->pBufferInfo = NULL;
			write->pTexelBufferView = NULL;
		}

		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_SAMPLERS].pImageInfo = samplers;
		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_RO_IMAGE_1D].pImageInfo = ro_image_1ds;
		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_RO_IMAGE_2D].pImageInfo = ro_image_2ds;
		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_RO_IMAGE_3D].pImageInfo = ro_image_3ds;
		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_RW_IMAGE_1D].pImageInfo = rw_image_1ds;
		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_RW_IMAGE_2D].pImageInfo = rw_image_2ds;
		vk_write_descriptor_sets[HERO_VULKAN_DESCRIPTOR_BINDING_RW_IMAGE_3D].pImageInfo = rw_image_3ds;

		ldev_vulkan->vkUpdateDescriptorSets(
			ldev_vulkan->handle,
			HERO_VULKAN_DESCRIPTOR_BINDING_COUNT,
			vk_write_descriptor_sets,
			0,
			NULL);
	}

	return HERO_SUCCESS;
}

void _hero_vulkan_cmd_draw_start(HeroCommandRecorder* command_recorder, HeroPipelineId pipeline_id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	VkCommandBuffer vk_command_buffer = command_recorder_->vulkan.command_buffer;
	if (pipeline_id.raw != command_recorder_->vulkan.bound_pipeline_graphics.raw) {
		HeroPipelineVulkan* pipeline_vulkan;
		result = hero_object_pool(HeroPipelineVulkan, get)(&ldev_vulkan->pipeline_pool, pipeline_id, &pipeline_vulkan);
		HERO_RESULT_ASSERT(result);

		ldev_vulkan->vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_vulkan->handle);
		command_recorder_->vulkan.bound_pipeline_graphics.raw = pipeline_id.raw;
	}

	command_recorder_->vulkan.vertex_buffer_binding_start = -1;
	command_recorder_->vulkan.vertex_buffer_binding_end = 0;
	command_recorder_->vulkan.vertices_start_idx = 0;
	command_recorder_->vulkan.indices_start_idx = 0;
	command_recorder_->vulkan.instances_start_idx = 0;
	command_recorder_->vulkan.instances_count = 1;
}

void _hero_vulkan_cmd_draw_end_prepare(_HeroCommandRecorder* command_recorder) {
	//
	// if any of the vertex buffers have changed, lets bind them.
	if (command_recorder->vulkan.vertex_buffer_binding_start < command_recorder->vulkan.vertex_buffer_binding_end) {
		HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->public_.ldev;
		U32 buffers_count = command_recorder->vulkan.vertex_buffer_binding_end - command_recorder->vulkan.vertex_buffer_binding_start;
		ldev_vulkan->vkCmdBindVertexBuffers(command_recorder->vulkan.command_buffer,
			command_recorder->vulkan.vertex_buffer_binding_start,
			buffers_count,
			&command_recorder->vulkan.bound_vertex_buffers[command_recorder->vulkan.vertex_buffer_binding_start],
			&command_recorder->vulkan.bound_vertex_buffer_offsets[command_recorder->vulkan.vertex_buffer_binding_start]);
	}
}

void _hero_vulkan_cmd_draw_end_vertexed(HeroCommandRecorder* command_recorder, U32 vertices_count) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	_hero_vulkan_cmd_draw_end_prepare(command_recorder_);

	ldev_vulkan->vkCmdDraw(command_recorder_->vulkan.command_buffer,
		vertices_count,
		command_recorder_->vulkan.instances_count,
		command_recorder_->vulkan.vertices_start_idx,
		command_recorder_->vulkan.instances_start_idx);
}

void _hero_vulkan_cmd_draw_end_indexed(HeroCommandRecorder* command_recorder, HeroPassResource* index_buffer_resource, HeroIndexType index_type, U32 indices_count) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	_hero_vulkan_cmd_draw_end_prepare(command_recorder_);

	HeroFrameGraph* frame_graph = command_recorder->frame_graph;
	HeroFrameGraphVulkan* frame_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameGraphVulkan, frame_graph);
	HeroVulkanResource* vulkan_resource = _hero_vulkan_frame_graph_vulkan_resource_get(frame_graph_vulkan, index_buffer_resource, frame_graph->active_frame_idx);

	VkCommandBuffer vk_command_buffer = command_recorder_->vulkan.command_buffer;
	ldev_vulkan->vkCmdBindIndexBuffer(vk_command_buffer,
		vulkan_resource->buffer,
		0,
		_hero_vulkan_convert_to_index_type[index_type]);

	ldev_vulkan->vkCmdDrawIndexed(vk_command_buffer,
		indices_count,
		command_recorder_->vulkan.instances_count,
		command_recorder_->vulkan.indices_start_idx,
		command_recorder_->vulkan.vertices_start_idx,
		command_recorder_->vulkan.instances_start_idx);
}

void _hero_vulkan_cmd_draw_set_vertex_buffer(HeroCommandRecorder* command_recorder, HeroPassResource* vertex_buffer_resource, U32 binding, U64 offset) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	HeroFrameGraph* frame_graph = command_recorder->frame_graph;
	HeroFrameGraphVulkan* frame_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroFrameGraphVulkan, frame_graph);
	HeroVulkanResource* vulkan_resource = _hero_vulkan_frame_graph_vulkan_resource_get(frame_graph_vulkan, vertex_buffer_resource, frame_graph->active_frame_idx);

	HERO_ASSERT_ARRAY_BOUNDS(binding, HERO_BUFFER_BINDINGS_CAP);

	command_recorder_->vulkan.bound_vertex_buffers[binding] = vulkan_resource->buffer;
	command_recorder_->vulkan.bound_vertex_buffer_offsets[binding] = offset;
	command_recorder_->vulkan.vertex_buffer_binding_start = HERO_MIN(command_recorder_->vulkan.vertex_buffer_binding_start, binding);
	command_recorder_->vulkan.vertex_buffer_binding_end = HERO_MAX(command_recorder_->vulkan.vertex_buffer_binding_end, binding + 1);
}

void _hero_vulkan_cmd_draw_set_vertices_start_idx(HeroCommandRecorder* command_recorder, U32 vertices_start_idx) {
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;
	command_recorder_->vulkan.vertices_start_idx = vertices_start_idx;
}

void _hero_vulkan_cmd_draw_set_indices_start_idx(HeroCommandRecorder* command_recorder, U32 indices_start_idx) {
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;
	command_recorder_->vulkan.indices_start_idx = indices_start_idx;
}

void _hero_vulkan_cmd_draw_set_instances(HeroCommandRecorder* command_recorder, U32 instances_start_idx, U32 instances_count) {
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;
	command_recorder_->vulkan.instances_start_idx = instances_start_idx;
	command_recorder_->vulkan.instances_count = instances_count;
}

void _hero_vulkan_cmd_compute_dispatch_start(HeroCommandRecorder* command_recorder, HeroShaderId compute_shader_id, U32 group_count_x, U32 group_count_y, U32 group_count_z) {
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;
	command_recorder_->vulkan.group_count_x = group_count_x;
	command_recorder_->vulkan.group_count_y = group_count_y;
	command_recorder_->vulkan.group_count_z = group_count_z;

	if (command_recorder_->vulkan.bound_pipeline_compute.raw != compute_shader_id.raw) {
		HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
		VkCommandBuffer vk_command_buffer = command_recorder_->vulkan.command_buffer;

		HeroShaderVulkan* shader_vulkan;
		HeroResult result = hero_object_pool(HeroShaderVulkan, get)(&ldev_vulkan->shader_pool, compute_shader_id, &shader_vulkan);
		HERO_RESULT_ASSERT(result);

		ldev_vulkan->vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader_vulkan->compute_pipeline);
		command_recorder_->vulkan.bound_pipeline_compute = compute_shader_id;
	}
}

void _hero_vulkan_cmd_compute_dispatch_end(HeroCommandRecorder* command_recorder) {
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;
	VkCommandBuffer vk_command_buffer = command_recorder_->vulkan.command_buffer;
	ldev_vulkan->vkCmdDispatch(vk_command_buffer, command_recorder_->vulkan.group_count_x, command_recorder_->vulkan.group_count_y, command_recorder_->vulkan.group_count_z);
}

typedef struct HeroShaderInfo HeroShaderInfo;
struct HeroShaderInfo {
	U32 samplers_start_idx;
	U32 images_start_idx;
	U32 buffers_start_idx;
	U32 samplers_count;
	U32 images_count;
	U32 buffers_count;
};

typedef struct HeroShaderBinary HeroShaderBinary;
struct HeroShaderBinary {
	U8* code;
	U32 code_size;

	HeroShaderInfo* info;
	U32 info_count;

	union {
		struct {
			U8* sampler_push_constant_offsets;
			U8* image_push_constant_offsets;
			U8* buffer_push_constant_offsets;
		} vulkan;
	};
};

HeroShaderBinary TODO_GENERATE_THIS_SHADER_BINARY_SHIZ;

void _hero_vulkan_cmd_add_sampler(HeroCommandRecorder* command_recorder, U32 binding, HeroSamplerId sampler_id) {
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	HeroShaderBinary* binary = &TODO_GENERATE_THIS_SHADER_BINARY_SHIZ;
	HeroShaderInfo* info = &binary->info[/*TODO*/0];

	HERO_DEBUG_ASSERT(binding < info->samplers_count, "binding '%u' is out of bounds for the samplers with a count of '%u'", binding, info->samplers_count);

	U32 dst_offset = binary->vulkan.sampler_push_constant_offsets[info->samplers_start_idx + binding];
	command_recorder_->vulkan.push_constants[dst_offset] = hero_object_id(HeroSamplerId, idx)(sampler_id);
}

void _hero_vulkan_cmd_add_image(HeroCommandRecorder* command_recorder, U32 binding, HeroPassResource* resource) {
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	HeroShaderBinary* binary = &TODO_GENERATE_THIS_SHADER_BINARY_SHIZ;
	HeroShaderInfo* info = &binary->info[/*TODO*/0];

	HERO_DEBUG_ASSERT(binding < info->images_count, "binding '%u' is out of bounds for the images with a count of '%u'", binding, info->images_count);

	U32 dst_offset = binary->vulkan.image_push_constant_offsets[info->images_start_idx + binding];
	U32 resource_idx = resource - command_recorder->frame_graph->pass_resources;
	command_recorder_->vulkan.push_constants[dst_offset] = resource_idx;
}

void _hero_vulkan_cmd_add_buffer(HeroCommandRecorder* command_recorder, U32 binding, HeroPassResource* resource) {
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)command_recorder->ldev;
	_HeroCommandRecorder* command_recorder_ = (_HeroCommandRecorder*)command_recorder;

	HeroShaderBinary* binary = &TODO_GENERATE_THIS_SHADER_BINARY_SHIZ;
	HeroShaderInfo* info = &binary->info[/*TODO*/0];

	HERO_DEBUG_ASSERT(binding < info->buffers_count, "binding '%u' is out of bounds for the buffers with a count of '%u'", binding, info->buffers_count);

	U32 dst_offset = binary->vulkan.buffer_push_constant_offsets[info->buffers_start_idx + binding];
	U32 resource_idx = resource - command_recorder->frame_graph->pass_resources;
	command_recorder_->vulkan.push_constants[dst_offset] = resource_idx;
}

#if 0

HeroResult _hero_vulkan_render_graph_reinit_output_image_swapchain(HeroLogicalDevice* ldev, HeroImageOutput* image_output, HeroImageInfo* image_info, HeroSwapchain* swapchain, HeroImageOutputVulkan* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroSwapchainVulkan* swapchain_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroSwapchainVulkan, swapchain);

	if (image_info->flags & HERO_IMAGE_INFO_FLAGS_IS_SWAPCHAIN) {
		out->image_view = swapchain_vulkan->image_views[swapchain_vulkan->image_idx];
		return HERO_SUCCESS;
	}

	VkImageCreateInfo vk_image_create_info = {0};
	vk_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	vk_image_create_info.extent.width = (U32)(roundf(image_info->width * (F32)swapchain->width));
	vk_image_create_info.extent.height = (U32)(roundf(image_info->height * (F32)swapchain->height));
	vk_image_create_info.extent.depth = 1;
	vk_image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	vk_image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	vk_image_create_info.format = _hero_vulkan_convert_to_format[image_info->format];
	vk_image_create_info.imageType = VK_IMAGE_TYPE_2D;
	vk_image_create_info.samples = 1;
	vk_image_create_info.mipLevels = 1;
	vk_image_create_info.arrayLayers = swapchain->array_layers_count;

	vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &out->image);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	{
		VkMemoryRequirements mem_req;
		ldev_vulkan->vkGetImageMemoryRequirements(ldev_vulkan->handle, out->image, &mem_req);

		_HeroVulkanAllocSetup alloc_setup = {
			.size = mem_req.size,
			.align = mem_req.alignment,
			.memory_type_bits = mem_req.memoryTypeBits,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.type = _HERO_VULKAN_ALLOC_TYPE_IMAGE,
		};

		result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &out->device_memory);
		if (result < 0) {
			return result;
		}
	}

	VkDeviceSize vk_device_memory_offset = 0;
	vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, out->image, out->device_memory, vk_device_memory_offset);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkImageViewCreateInfo vk_image_view_create_info = {0};
	vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vk_image_view_create_info.image = out->image;
	vk_image_view_create_info.format = _hero_vulkan_convert_to_format[image_info->format];
	vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vk_image_view_create_info.subresourceRange.levelCount = image_info->mip_levels_count;
	vk_image_view_create_info.subresourceRange.layerCount = image_info->array_layers_count;

	vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &out->image_view);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_init_output_image(HeroLogicalDevice* ldev, HeroImageOutput* image_output, HeroImageInfo* image_info, HeroImageOutputVulkan* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkImageCreateInfo vk_image_create_info = {0};
	vk_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	vk_image_create_info.extent.width = (U32)roundf(image_info->width);
	vk_image_create_info.extent.height = (U32)roundf(image_info->height);
	vk_image_create_info.extent.depth = image_info->depth;
	vk_image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	if (HERO_IMAGE_FORMAT_IS_DEPTH(image_info->format)) {
		vk_image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	} else {
		vk_image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	vk_image_create_info.format = _hero_vulkan_convert_to_format[image_info->format];
	vk_image_create_info.imageType = image_info->type;
	vk_image_create_info.samples = 1 << image_info->samples_count_log2;
	vk_image_create_info.mipLevels = image_info->mip_levels_count;
	vk_image_create_info.arrayLayers = image_info->array_layers_count;

	vk_result = ldev_vulkan->vkCreateImage(ldev_vulkan->handle, &vk_image_create_info, HERO_VULKAN_TODO_ALLOCATOR, &out->image);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	{
		VkMemoryRequirements mem_req;
		ldev_vulkan->vkGetImageMemoryRequirements(ldev_vulkan->handle, out->image, &mem_req);

		_HeroVulkanAllocSetup alloc_setup = {
			.size = mem_req.size,
			.align = mem_req.alignment,
			.memory_type_bits = mem_req.memoryTypeBits,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.type = _HERO_VULKAN_ALLOC_TYPE_IMAGE,
		};

		result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &out->device_memory);
		if (result < 0) {
			return result;
		}
	}

	VkDeviceSize vk_device_memory_offset = 0;
	vk_result = ldev_vulkan->vkBindImageMemory(ldev_vulkan->handle, out->image, out->device_memory, vk_device_memory_offset);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	VkImageViewCreateInfo vk_image_view_create_info = {0};
	vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vk_image_view_create_info.image = out->image;
	vk_image_view_create_info.format = _hero_vulkan_convert_to_format[image_info->format];
	vk_image_view_create_info.viewType = _hero_vulkan_convert_to_image_view_type[image_info->format];
	vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
	vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
	vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
	vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
	vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vk_image_view_create_info.subresourceRange.levelCount = image_info->mip_levels_count;
	vk_image_view_create_info.subresourceRange.layerCount = image_info->array_layers_count;

	vk_result = ldev_vulkan->vkCreateImageView(ldev_vulkan->handle, &vk_image_view_create_info, HERO_VULKAN_TODO_ALLOCATOR, &out->image_view);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_init_output_buffer(HeroLogicalDevice* ldev, HeroBufferOutput* buffer_output, HeroBufferInfo* buffer_info, HeroBufferOutputVulkan* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	VkBufferCreateInfo vk_create_info = {0};
	vk_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vk_create_info.size = buffer_info->size;
	vk_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

	vk_result = ldev_vulkan->vkCreateBuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &out->buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	void* mapped_memory;
	{
		VkMemoryRequirements mem_req;
		ldev_vulkan->vkGetBufferMemoryRequirements(ldev_vulkan->handle, out->buffer, &mem_req);

		_HeroVulkanAllocSetup alloc_setup = {
			.size = mem_req.size,
			.align = mem_req.alignment,
			.memory_type_bits = mem_req.memoryTypeBits,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.type = _HERO_VULKAN_ALLOC_TYPE_BUFFER,
		};

		result = _hero_vulkan_device_memory_alloc(ldev_vulkan, &alloc_setup, &out->device_memory);
		if (result < 0) {
			return result;
		}
	}

	VkDeviceSize vk_device_memory_offset = 0;
	vk_result = ldev_vulkan->vkBindBufferMemory(ldev_vulkan->handle, out->buffer, out->device_memory, vk_device_memory_offset);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_reinit_frame_buffer(HeroLogicalDevice* ldev, HeroRenderGraphVulkan* render_graph_vulkan, HeroSwapchain* swapchain, HeroPassInfo* pass_info, HeroPassInfoVulkan* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	if (out->frame_buffer) {
		ldev_vulkan->vkDestroyFramebuffer(ldev_vulkan->handle, out->frame_buffer, HERO_VULKAN_TODO_ALLOCATOR);
	}

	U32 output_attachments_count = 0;
	for_range(image_output_enum, 0, pass_info->image_outputs_count) {
		HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
		if (image_output->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
			output_attachments_count += 1;
		}
	}

	VkImageView* vk_attachments = hero_alloc_array(VkImageView, hero_system_alctor, 0, output_attachments_count);
	U32 output_attachment_idx = 0;
	for_range(image_output_enum, 0, pass_info->image_outputs_count) {
		HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
		HeroImageOutputVulkan* image_output_vulkan = &render_graph_vulkan->image_outputs[image_output->backend_image_output_enum];
		if (image_output->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
			vk_attachments[output_attachment_idx] = image_output_vulkan->image_view;
			output_attachment_idx += 1;
		}
	}

	HeroImageOutput* image_output = &pass_info->image_outputs[0];
	HeroImageInfo* image_info = &render_graph_vulkan->public_.images[image_output->image_info_enum];

	VkFramebufferCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.renderPass = out->render_pass,
		.attachmentCount = output_attachments_count,
		.pAttachments = vk_attachments,
		.width = swapchain ? (U32)roundf(image_info->width * (F32)swapchain->width) : (U32)roundf(image_info->width),
		.height = swapchain ? (U32)roundf(image_info->height * (F32)swapchain->height) : (U32)roundf(image_info->height),
		.layers = image_info->array_layers_count,
	};

	VkFramebuffer vk_frame_buffer;
	vk_result = ldev_vulkan->vkCreateFramebuffer(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &out->frame_buffer);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_init_render_pass(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroPassInfo* pass_info, HeroPassInfoVulkan* out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderPassLayoutVulkan* render_pass_layout_vulkan;
	result = hero_object_pool(HeroRenderPassLayoutVulkan, get)(&ldev_vulkan->render_pass_layout_pool, pass_info->layout_id, &render_pass_layout_vulkan);
	if (result < 0) {
		return result;
	}

	U32 attachments_count = 0;
	U32 output_attachments_count = 0;
	U32 input_attachments_count = 0;
	for_range(image_output_enum, 0, pass_info->image_outputs_count) {
		HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
		if (image_output->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
			attachments_count += 1;
			output_attachments_count += 1;
		}
	}
	for_range(image_input_enum, 0, pass_info->image_inputs_count) {
		HeroImageInput* image_input = &pass_info->image_inputs[image_input_enum];
		if (image_input->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
			attachments_count += 1;
			input_attachments_count += 1;
		}
	}

	VkAttachmentDescription* vk_attachments;
	VkAttachmentReference* vk_attachment_output_refs;
	VkAttachmentReference* vk_attachment_input_refs;
	VkSubpassDescription vk_subpass = {0};
	{
		vk_attachments = hero_alloc_array(VkAttachmentDescription, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_DESCRIPTION, attachments_count);
		if (vk_attachments == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_attachment_output_refs = hero_alloc_array(VkAttachmentReference, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_REF, output_attachments_count);
		if (output_attachments_count && vk_attachment_output_refs == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		vk_attachment_input_refs = hero_alloc_array(VkAttachmentReference, hero_system_alctor, HERO_GFX_ALLOC_TAG_RENDER_PASS_VULKAN_ATTACHMENT_REF, input_attachments_count);
		if (input_attachments_count && vk_attachment_input_refs == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}

		U32 attachment_idx = 0;
		U32 output_attachment_idx = 0;
		U32 input_attachment_idx = 0;
		for_range(image_output_enum, 0, pass_info->image_outputs_count) {
			HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
			HeroImageInfo* image_info = &setup->images[image_output->image_info_enum];
			if (image_output->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
				U32 idx = attachment_idx;
				U32 output_idx = output_attachment_idx;
				if (image_output_enum == pass_info->depth_stencil_image_output_enum) {
					idx = attachments_count - 1;
					output_idx = output_attachments_count - 1;
				}

				VkAttachmentDescription* attachment = &vk_attachments[idx];
				*attachment = (VkAttachmentDescription) {
					.flags = 0,
					.format = _hero_vulkan_convert_to_format[image_info->format],
					.samples = 1 << image_info->samples_count_log2,
					.loadOp = image_output->flags & HERO_IMAGE_INFO_FLAGS_PERSISTENT ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = image_info->swapchain_id.raw ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL,
				};

				VkAttachmentReference* ref = &vk_attachment_output_refs[output_idx];
				ref->attachment = idx;
				ref->layout = image_info->swapchain_id.raw ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_GENERAL;

				if (image_output_enum != pass_info->depth_stencil_image_output_enum) {
					attachment_idx += 1;
					output_attachment_idx += 1;
				}
			}
		}
		for_range(image_input_enum, 0, pass_info->image_inputs_count) {
			HeroImageInput* image_input = &pass_info->image_inputs[image_input_enum];
			HeroImageOutput* image_output;
			if (image_input->pass_enum == HERO_PASS_ENUM_INVALID) {
				image_output = &setup->cpu_image_outputs[image_input->pass_image_output_enum];
			} else {
				HeroPassInfo* input_pass_info = &setup->passes[image_input->pass_enum];
				image_output = &input_pass_info->image_outputs[image_input->pass_image_output_enum];
			}

			HeroImageInfo* image_info = &setup->images[image_output->image_info_enum];
			if (image_input->attachment_idx != HERO_ATTACHMENT_IDX_INVALID) {
				VkAttachmentDescription* attachment = &vk_attachments[attachment_idx];
				*attachment = (VkAttachmentDescription) {
					.flags = 0,
					.format = _hero_vulkan_convert_to_format[image_info->format],
					.samples = 1 << image_info->samples_count_log2,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_GENERAL,
					.finalLayout = VK_IMAGE_LAYOUT_GENERAL,
				};

				VkAttachmentReference* ref = &vk_attachment_input_refs[input_attachment_idx];
				ref->attachment = attachment_idx;
				ref->layout = VK_IMAGE_LAYOUT_GENERAL;

				attachment_idx += 1;
				input_attachment_idx += 1;
			}
		}

		vk_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vk_subpass.inputAttachmentCount = input_attachments_count;
		vk_subpass.pInputAttachments = vk_attachment_input_refs;
		vk_subpass.colorAttachmentCount = pass_info->depth_stencil_image_output_enum != HERO_PASS_IMAGE_OUTPUT_ENUM_INVALID ? output_attachments_count - 1 : output_attachments_count;
		vk_subpass.pColorAttachments = vk_attachment_output_refs;
		vk_subpass.pDepthStencilAttachment = pass_info->depth_stencil_image_output_enum != HERO_PASS_IMAGE_OUTPUT_ENUM_INVALID ? &vk_attachment_output_refs[output_attachments_count - 1] : NULL;
	}

	static VkSubpassDependency vk_dependencies[] = {
		{
			.srcSubpass      = VK_SUBPASS_EXTERNAL,
			.dstSubpass      = 0,
			.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		},
		{
			.srcSubpass      = 0,
			.dstSubpass      = VK_SUBPASS_EXTERNAL,
			.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		}
	};

	VkRenderPassCreateInfo vk_create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pAttachments = vk_attachments,
		.attachmentCount = attachments_count,
		.pSubpasses = &vk_subpass,
		.subpassCount = 1,
		.pDependencies = vk_dependencies,
		.dependencyCount = HERO_ARRAY_COUNT(vk_dependencies),
	};

	VkRenderPass vk_render_pass;
	vk_result = ldev_vulkan->vkCreateRenderPass(ldev_vulkan->handle, &vk_create_info, HERO_VULKAN_TODO_ALLOCATOR, &vk_render_pass);
	if (vk_result < 0) {
		return _hero_vulkan_convert_from_result(vk_result);
	}

	out->render_pass = vk_render_pass;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_init(HeroLogicalDevice* ldev, HeroRenderGraphSetup* setup, HeroRenderGraphId* id_out, HeroRenderGraph** out) {
	HeroResult result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderGraphVulkan* render_graph_vulkan;
	result = hero_object_pool(HeroRenderGraphVulkan, alloc)(&ldev_vulkan->render_graph_pool, 0, &render_graph_vulkan, id_out);
	if (result < 0) {
		return result;
	}

	U32 image_outputs_count = 0;
	U32 buffer_outputs_count = 0;
	for_range(image_output_enum, 0, setup->cpu_image_outputs_count) {
		HeroImageOutput* image_output = &setup->cpu_image_outputs[image_output_enum];
		image_output->backend_image_output_enum = image_outputs_count;
		image_outputs_count += 1;
	}
	for_range(buffer_output_enum, 0, setup->cpu_buffer_outputs_count) {
		HeroBufferOutput* buffer_output = &setup->cpu_buffer_outputs[buffer_output_enum];
		buffer_output->backend_buffer_output_enum = buffer_outputs_count;
		buffer_outputs_count += 1;
	}
	for_range(pass_enum, 0, setup->passes_count) {
		HeroPassInfo* pass_info = &setup->passes[pass_enum];
		for_range(image_output_enum, 0, pass_info->image_outputs_count) {
			HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
			image_output->backend_image_output_enum = image_outputs_count;
			image_outputs_count += 1;
		}
		for_range(buffer_output_enum, 0, pass_info->buffer_outputs_count) {
			HeroBufferOutput* buffer_output = &pass_info->buffer_outputs[buffer_output_enum];
			buffer_output->backend_buffer_output_enum = buffer_outputs_count;
			buffer_outputs_count += 1;
		}
	}

	HeroPassInfoVulkan* passes_vulkan = hero_alloc_array(HeroPassInfoVulkan, hero_system_alctor, 0, setup->passes_count);
	HeroImageOutputVulkan* image_outputs_vulkan = hero_alloc_array(HeroImageOutputVulkan, hero_system_alctor, 0, image_outputs_count);
	HeroBufferOutputVulkan* buffer_outputs_vulkan = hero_alloc_array(HeroBufferOutputVulkan, hero_system_alctor, 0, buffer_outputs_count);
	if ((setup->passes_count && !passes_vulkan) || (image_outputs_count && !image_outputs_vulkan) || (buffer_outputs_count && !buffer_outputs_vulkan)) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	HERO_ZERO_ELMT_MANY(passes_vulkan, setup->passes_count);
	HERO_ZERO_ELMT_MANY(image_outputs_vulkan, image_outputs_count);
	HERO_ZERO_ELMT_MANY(buffer_outputs_vulkan, buffer_outputs_count);

	render_graph_vulkan->passes = passes_vulkan;
	render_graph_vulkan->image_outputs = image_outputs_vulkan;
	render_graph_vulkan->buffer_outputs = buffer_outputs_vulkan;

	for_range(image_output_enum, 0, setup->cpu_image_outputs_count) {
		HeroImageOutput* image_output = &setup->cpu_image_outputs[image_output_enum];
		HeroImageInfo* image_info = &setup->images[image_output->image_info_enum];

		result = _hero_vulkan_render_graph_init_output_image(ldev, image_output, image_info, &image_outputs_vulkan[image_output->backend_image_output_enum]);
		if (result < 0) {
			return result;
		}
	}
	for_range(buffer_output_enum, 0, setup->cpu_buffer_outputs_count) {
		HeroBufferOutput* buffer_output = &setup->cpu_buffer_outputs[buffer_output_enum];
		HeroBufferInfo* buffer_info = &setup->buffers[buffer_output->buffer_info_enum];

		result = _hero_vulkan_render_graph_init_output_buffer(ldev, buffer_output, buffer_info, &buffer_outputs_vulkan[buffer_output->backend_buffer_output_enum]);
		if (result < 0) {
			return result;
		}
	}
	for_range(pass_enum, 0, setup->passes_count) {
		HeroPassInfo* pass_info = &setup->passes[pass_enum];

		HeroSwapchain* swapchain = NULL;
		HeroImageInfo* image_info = &setup->images[pass_info->image_outputs[0].image_info_enum];
		if (image_info->swapchain_id.raw) {
			result = hero_swapchain_get(ldev, image_info->swapchain_id, &swapchain);
			if (result < 0) {
				return result;
			}
		}


		for_range(image_output_enum, 0, pass_info->image_outputs_count) {
			HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
			HeroImageInfo* image_info = &setup->images[image_output->image_info_enum];

			if (swapchain) {
				result = _hero_vulkan_render_graph_reinit_output_image_swapchain(ldev, image_output, image_info, swapchain, &image_outputs_vulkan[image_output->backend_image_output_enum]);
			} else {
				result = _hero_vulkan_render_graph_init_output_image(ldev, image_output, image_info, &image_outputs_vulkan[image_output->backend_image_output_enum]);
			}
			if (result < 0) {
				return result;
			}
		}

		for_range(buffer_output_enum, 0, pass_info->buffer_outputs_count) {
			HeroBufferOutput* buffer_output = &pass_info->buffer_outputs[buffer_output_enum];
			HeroBufferInfo* buffer_info = &setup->buffers[buffer_output->buffer_info_enum];

			result = _hero_vulkan_render_graph_init_output_buffer(ldev, buffer_output, buffer_info, &buffer_outputs_vulkan[buffer_output->backend_buffer_output_enum]);
			if (result < 0) {
				return result;
			}
		}

		if (pass_info->layout_id.raw) {
			result = _hero_vulkan_render_graph_init_render_pass(ldev, setup, pass_info, &passes_vulkan[pass_enum]);
			if (result < 0) {
				return result;
			}

			result = _hero_vulkan_render_graph_reinit_frame_buffer(ldev, render_graph_vulkan, swapchain, pass_info, &passes_vulkan[pass_enum]);
			if (result < 0) {
				return result;
			}
		}
	}

	*out = &render_graph_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_deinit(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph* render_graph) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;
	HeroRenderGraphVulkan* render_graph_vulkan = HERO_GFX_INTERNAL_OBJECT(HeroRenderGraphVulkan, render_graph);

	result = _hero_vulkan_render_graph_deinit_gpu_resources(ldev, id);
	if (result < 0) {
		return result;
	}

	/*
	hero_dealloc_array(HeroImageInfoVulkan, hero_system_alctor, 0, render_graph_vulkan->images, render_graph->images_count);
	hero_dealloc_array(HeroBufferInfoVulkan, hero_system_alctor, 0, render_graph_vulkan->buffers, render_graph->buffers_count);
	*/
	hero_dealloc_array(HeroPassInfoVulkan, hero_system_alctor, 0, render_graph_vulkan->passes, render_graph->passes_count);
	return hero_object_pool(HeroRenderGraphVulkan, dealloc)(&ldev_vulkan->render_graph_pool, id);
}

HeroResult _hero_vulkan_render_graph_get(HeroLogicalDevice* ldev, HeroRenderGraphId id, HeroRenderGraph** out) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderGraphVulkan* render_graph_vulkan;
	result = hero_object_pool(HeroRenderGraphVulkan, get)(&ldev_vulkan->render_graph_pool, id, &render_graph_vulkan);
	if (result < 0) {
		return result;
	}

	*out = &render_graph_vulkan->public_;
	return HERO_SUCCESS;
}

HeroResult _hero_vulkan_render_graph_deinit_gpu_resources(HeroLogicalDevice* ldev, HeroRenderGraphId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderGraphVulkan* render_graph_vulkan;
	result = hero_object_pool(HeroRenderGraphVulkan, get)(&ldev_vulkan->render_graph_pool, id, &render_graph_vulkan);
	if (result < 0) {
		return result;
	}
	HeroRenderGraph* render_graph = &render_graph_vulkan->public_;
	if (!render_graph_vulkan->initialized_gpu_resources) {
		return HERO_SUCCESS;
	}

	HERO_ABORT("UNIMPLEMENTED");

	for_range(image_info_enum, 0, render_graph->images_count) {

	}

	for_range(buffer_info_enum, 0, render_graph->buffers_count) {

	}

	for_range(pass_enum, 0, render_graph->passes_count) {

	}
}

HeroResult _hero_vulkan_render_graph_execute(HeroLogicalDevice* ldev, HeroRenderGraphId id) {
	HeroResult result;
	VkResult vk_result;
	HeroLogicalDeviceVulkan* ldev_vulkan = (HeroLogicalDeviceVulkan*)ldev;

	HeroRenderGraphVulkan* render_graph_vulkan;
	result = hero_object_pool(HeroRenderGraphVulkan, get)(&ldev_vulkan->render_graph_pool, id, &render_graph_vulkan);
	if (result < 0) {
		return result;
	}
	HeroRenderGraph* render_graph = &render_graph_vulkan->public_;

	for_range(pass_enum, 0, render_graph->passes_count) {
		HeroPassInfo* pass_info = &render_graph->passes[pass_enum];

		HeroImageInfo* image_info = &render_graph->images[pass_info->image_outputs[0].image_info_enum];
		if (image_info->swapchain_id.raw == 0) {
			continue;
		}

		HeroSwapchain* swapchain = NULL;
		result = hero_swapchain_get(ldev, image_info->swapchain_id, &swapchain);
		if (result < 0) {
			return result;
		}

		if (!swapchain->has_been_resized) {
			continue;
		}

		for_range(image_output_enum, 0, pass_info->image_outputs_count) {
			HeroImageOutput* image_output = &pass_info->image_outputs[image_output_enum];
			result = _hero_vulkan_render_graph_reinit_output_image_swapchain(ldev, image_output, image_info, swapchain, &render_graph_vulkan->image_outputs[image_output->backend_image_output_enum]);
		}

		if (pass_info->layout_id.raw) {
			result = _hero_vulkan_render_graph_reinit_frame_buffer(ldev, render_graph_vulkan, swapchain, pass_info, &render_graph_vulkan->passes[pass_enum]);
			if (result < 0) {
				return result;
			}
		}
	}

	//
	// allocate all the graphics resources if we have not execute at all since the resource where last destroyed
	//
	if (!render_graph_vulkan->initialized_gpu_resources) {
		for_range(image_info_enum, 0, render_graph->images_count) {

		}

		for_range(buffer_info_enum, 0, render_graph->buffers_count) {

		}

		for_range(pass_enum, 0, render_graph->passes_count) {

		}
	}


	return HERO_SUCCESS;
}

#endif

HeroGfxSysVulkan hero_gfx_sys_vulkan;

#endif // HERO_VULKAN_ENABLE

// ===========================================
//
//
// Gfx System
//
//
// ===========================================

HeroGfxSys hero_gfx_sys;

HeroResult hero_gfx_sys_init(HeroGfxSysSetup* setup) {
	HeroResult result;

	hero_gfx_sys.backend_type = setup->backend_type;

	result = HERO_SUCCESS;
	switch (setup->backend_type) {
#if HERO_VULKAN_ENABLE
		case HERO_GFX_BACKEND_TYPE_VULKAN: {
			hero_gfx_sys.backend_vtable.physical_device_surface_image_formats_supported = _hero_vulkan_physical_device_surface_image_formats_supported;
			hero_gfx_sys.backend_vtable.logical_device_init = _hero_vulkan_logical_device_init;
			hero_gfx_sys.backend_vtable.logical_device_deinit = _hero_vulkan_logical_device_deinit;
			hero_gfx_sys.backend_vtable.logical_device_frame_start = _hero_vulkan_logical_device_frame_start;
			hero_gfx_sys.backend_vtable.logical_device_queue_transfer = _hero_vulkan_logical_device_queue_transfer;
			/*
			hero_gfx_sys.backend_vtable.logical_device_queue_command_buffers = _hero_vulkan_logical_device_queue_command_buffers;
			*/
			hero_gfx_sys.backend_vtable.logical_device_submit = _hero_vulkan_logical_device_submit;
			hero_gfx_sys.backend_vtable.vertex_layout_register = _hero_vulkan_vertex_layout_register;
			hero_gfx_sys.backend_vtable.vertex_layout_deregister = _hero_vulkan_vertex_layout_deregister;
			hero_gfx_sys.backend_vtable.vertex_layout_get = _hero_vulkan_vertex_layout_get;
			hero_gfx_sys.backend_vtable.buffer_init = _hero_vulkan_buffer_init;
			hero_gfx_sys.backend_vtable.buffer_deinit = _hero_vulkan_buffer_deinit;
			hero_gfx_sys.backend_vtable.buffer_get = _hero_vulkan_buffer_get;
			hero_gfx_sys.backend_vtable.buffer_resize = _hero_vulkan_buffer_resize;
			hero_gfx_sys.backend_vtable.buffer_map = _hero_vulkan_buffer_map;
			hero_gfx_sys.backend_vtable.buffer_read = _hero_vulkan_buffer_read;
			hero_gfx_sys.backend_vtable.buffer_write = _hero_vulkan_buffer_write;
			hero_gfx_sys.backend_vtable.image_init = _hero_vulkan_image_init;
			hero_gfx_sys.backend_vtable.image_deinit = _hero_vulkan_image_deinit;
			hero_gfx_sys.backend_vtable.image_get = _hero_vulkan_image_get;
			hero_gfx_sys.backend_vtable.image_resize = _hero_vulkan_image_resize;
			hero_gfx_sys.backend_vtable.image_map = _hero_vulkan_image_map;
			hero_gfx_sys.backend_vtable.image_read = _hero_vulkan_image_read;
			hero_gfx_sys.backend_vtable.image_write = _hero_vulkan_image_write;
			hero_gfx_sys.backend_vtable.sampler_init = _hero_vulkan_sampler_init;
			hero_gfx_sys.backend_vtable.sampler_deinit = _hero_vulkan_sampler_deinit;
			hero_gfx_sys.backend_vtable.sampler_get = _hero_vulkan_sampler_get;
			hero_gfx_sys.backend_vtable.shader_module_init = _hero_vulkan_shader_module_init;
			hero_gfx_sys.backend_vtable.shader_module_deinit = _hero_vulkan_shader_module_deinit;
			hero_gfx_sys.backend_vtable.shader_module_get = _hero_vulkan_shader_module_get;
			hero_gfx_sys.backend_vtable.shader_metadata_calculate = _hero_vulkan_shader_metadata_calculate;
			hero_gfx_sys.backend_vtable.shader_init = _hero_vulkan_shader_init;
			hero_gfx_sys.backend_vtable.shader_deinit = _hero_vulkan_shader_deinit;
			hero_gfx_sys.backend_vtable.shader_get = _hero_vulkan_shader_get;
			hero_gfx_sys.backend_vtable.descriptor_pool_init = _hero_vulkan_descriptor_pool_init;
			hero_gfx_sys.backend_vtable.descriptor_pool_deinit = _hero_vulkan_descriptor_pool_deinit;
			hero_gfx_sys.backend_vtable.descriptor_pool_reset = _hero_vulkan_descriptor_pool_reset;
			hero_gfx_sys.backend_vtable.shader_globals_init = _hero_vulkan_shader_globals_init;
			hero_gfx_sys.backend_vtable.shader_globals_deinit = _hero_vulkan_shader_globals_deinit;
			hero_gfx_sys.backend_vtable.shader_globals_get = _hero_vulkan_shader_globals_get;
			hero_gfx_sys.backend_vtable.shader_globals_set_descriptor = _hero_vulkan_shader_globals_set_descriptor;
			hero_gfx_sys.backend_vtable.shader_globals_update = _hero_vulkan_shader_globals_update;
			hero_gfx_sys.backend_vtable.render_pass_layout_init = _hero_vulkan_render_pass_layout_init;
			hero_gfx_sys.backend_vtable.render_pass_layout_deinit = _hero_vulkan_render_pass_layout_deinit;
			hero_gfx_sys.backend_vtable.render_pass_layout_get = _hero_vulkan_render_pass_layout_get;
			hero_gfx_sys.backend_vtable.render_pass_init = _hero_vulkan_render_pass_init;
			hero_gfx_sys.backend_vtable.render_pass_deinit = _hero_vulkan_render_pass_deinit;
			hero_gfx_sys.backend_vtable.render_pass_get = _hero_vulkan_render_pass_get;
			hero_gfx_sys.backend_vtable.frame_buffer_init = _hero_vulkan_frame_buffer_init;
			hero_gfx_sys.backend_vtable.frame_buffer_deinit = _hero_vulkan_frame_buffer_deinit;
			hero_gfx_sys.backend_vtable.frame_buffer_get = _hero_vulkan_frame_buffer_get;
			hero_gfx_sys.backend_vtable.pipeline_cache_init = _hero_vulkan_pipeline_cache_init;
			hero_gfx_sys.backend_vtable.pipeline_cache_deinit = _hero_vulkan_pipeline_cache_deinit;
			hero_gfx_sys.backend_vtable.pipeline_graphics_init = _hero_vulkan_pipeline_graphics_init;
			hero_gfx_sys.backend_vtable.pipeline_deinit = _hero_vulkan_pipeline_deinit;
			hero_gfx_sys.backend_vtable.pipeline_get = _hero_vulkan_pipeline_get;
			hero_gfx_sys.backend_vtable.material_init = _hero_vulkan_material_init;
			hero_gfx_sys.backend_vtable.material_deinit = _hero_vulkan_material_deinit;
			hero_gfx_sys.backend_vtable.material_get = _hero_vulkan_material_get;
			hero_gfx_sys.backend_vtable.material_set_descriptor = _hero_vulkan_material_set_descriptor;
			hero_gfx_sys.backend_vtable.material_update = _hero_vulkan_material_update;
			hero_gfx_sys.backend_vtable.swapchain_init = _hero_vulkan_swapchain_init;
			hero_gfx_sys.backend_vtable.swapchain_deinit = _hero_vulkan_swapchain_deinit;
			hero_gfx_sys.backend_vtable.swapchain_get = _hero_vulkan_swapchain_get;
			hero_gfx_sys.backend_vtable.swapchain_next_image = _hero_vulkan_swapchain_next_image;
			/*
			hero_gfx_sys.backend_vtable.command_pool_init = _hero_vulkan_command_pool_init;
			hero_gfx_sys.backend_vtable.command_pool_deinit = _hero_vulkan_command_pool_deinit;
			hero_gfx_sys.backend_vtable.command_pool_reset = _hero_vulkan_command_pool_reset;
			hero_gfx_sys.backend_vtable.command_buffer_deinit = _hero_vulkan_command_buffer_deinit;
			hero_gfx_sys.backend_vtable.command_recorder_start = _hero_vulkan_command_recorder_start;
			hero_gfx_sys.backend_vtable.command_recorder_end = _hero_vulkan_command_recorder_end;
			hero_gfx_sys.backend_vtable.cmd_render_pass_start = _hero_vulkan_cmd_render_pass_start;
			hero_gfx_sys.backend_vtable.cmd_render_pass_end = _hero_vulkan_cmd_render_pass_end;
			hero_gfx_sys.backend_vtable.cmd_draw_start = _hero_vulkan_cmd_draw_start;
			hero_gfx_sys.backend_vtable.cmd_draw_end_vertexed = _hero_vulkan_cmd_draw_end_vertexed;
			hero_gfx_sys.backend_vtable.cmd_draw_end_indexed = _hero_vulkan_cmd_draw_end_indexed;
			hero_gfx_sys.backend_vtable.cmd_draw_set_vertex_buffer = _hero_vulkan_cmd_draw_set_vertex_buffer;
			hero_gfx_sys.backend_vtable.cmd_draw_set_push_constants = _hero_vulkan_cmd_draw_set_push_constants;
			hero_gfx_sys.backend_vtable.cmd_draw_set_instances = _hero_vulkan_cmd_draw_set_instances;
			hero_gfx_sys.backend_vtable.cmd_compute_dispatch = _hero_vulkan_cmd_compute_dispatch;
			*/
			hero_gfx_sys.backend_vtable.render_graph_init = _hero_vulkan_render_graph_init;
			hero_gfx_sys.backend_vtable.render_graph_deinit = _hero_vulkan_render_graph_deinit;
			hero_gfx_sys.backend_vtable.render_graph_get = _hero_vulkan_render_graph_get;
			hero_gfx_sys.backend_vtable.frame_graph_init = _hero_vulkan_frame_graph_init;
			hero_gfx_sys.backend_vtable.frame_graph_deinit = _hero_vulkan_frame_graph_deinit;
			hero_gfx_sys.backend_vtable.frame_graph_get = _hero_vulkan_frame_graph_get;
			hero_gfx_sys.backend_vtable.frame_graph_update = _hero_vulkan_frame_graph_update;
			hero_gfx_sys.backend_vtable.frame_graph_record_pass_start = _hero_vulkan_frame_graph_record_pass_start;
			hero_gfx_sys.backend_vtable.frame_graph_record_pass_end = _hero_vulkan_frame_graph_record_pass_end;
			hero_gfx_sys.backend_vtable.frame_graph_submit = _hero_vulkan_frame_graph_submit;
			hero_gfx_sys.backend_vtable.cmd_draw_start = _hero_vulkan_cmd_draw_start;
			hero_gfx_sys.backend_vtable.cmd_draw_end_vertexed = _hero_vulkan_cmd_draw_end_vertexed;
			hero_gfx_sys.backend_vtable.cmd_draw_end_indexed = _hero_vulkan_cmd_draw_end_indexed;
			hero_gfx_sys.backend_vtable.cmd_draw_set_vertex_buffer = _hero_vulkan_cmd_draw_set_vertex_buffer;
			hero_gfx_sys.backend_vtable.cmd_draw_set_vertices_start_idx = _hero_vulkan_cmd_draw_set_vertices_start_idx;
			hero_gfx_sys.backend_vtable.cmd_draw_set_indices_start_idx = _hero_vulkan_cmd_draw_set_indices_start_idx;
			hero_gfx_sys.backend_vtable.cmd_draw_set_instances = _hero_vulkan_cmd_draw_set_instances;
			hero_gfx_sys.backend_vtable.cmd_compute_dispatch_start = _hero_vulkan_cmd_compute_dispatch_start;
			hero_gfx_sys.backend_vtable.cmd_compute_dispatch_end = _hero_vulkan_cmd_compute_dispatch_end;
			hero_gfx_sys.backend_vtable.cmd_add_sampler = _hero_vulkan_cmd_add_sampler;
			hero_gfx_sys.backend_vtable.cmd_add_image = _hero_vulkan_cmd_add_image;
			hero_gfx_sys.backend_vtable.cmd_add_buffer = _hero_vulkan_cmd_add_buffer;
#if 0
			hero_gfx_sys.backend_vtable.render_graph_deinit_gpu_resources = _hero_vulkan_render_graph_deinit_gpu_resources;
			hero_gfx_sys.backend_vtable.render_graph_execute = _hero_vulkan_render_graph_execute;
#endif
			result = _hero_vulkan_init(setup);
			break;
		};
#endif // HERO_VULKAN_ENABLE
	}
	if (result < 0) {
		return result;
	}

	return HERO_SUCCESS;
}

