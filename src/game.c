#include "game.h"

// ===========================================
//
//
// Game
//
//
// ===========================================

Game game;

typedef struct GameVertex GameVertex;
struct GameVertex {
	Vec2 pos;
	HeroColor color;
};

static HeroVertexAttribInfo vertex_attribs[] = {
	{ // GameVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
	{ // GameVertex.color
		.location = 1,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_U8_F32_NORMALIZE,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_4,
	},
};

static HeroVertexBindingInfo vertex_bindings[] = {
	{
		.attribs = vertex_attribs,
		.attribs_count = HERO_ARRAY_COUNT(vertex_attribs),
		.size = sizeof(GameVertex),
	}
};

static HeroVertexLayout vertex_layout = {
	.bindings_count = HERO_ARRAY_COUNT(vertex_bindings),
	.bindings = vertex_bindings,
};

HeroResult game_gfx_swapchain_frame_buffers_reinit(HeroSwapchain* swapchain) {
	HeroResult result;

	for_range (i, 0, game.swapchain_frame_buffers_count) {
		result = hero_frame_buffer_deinit(game.ldev, game.swapchain_frame_buffer_ids[i]);
		HERO_RESULT_ASSERT(result);
	}

	HeroFrameBufferId* swapchain_frame_buffer_ids = hero_realloc_array(HeroFrameBufferId, hero_system_alctor, HERO_ALLOC_TAG_TODO, game.swapchain_frame_buffer_ids, game.swapchain_frame_buffers_count, swapchain->images_count);
	if (swapchain_frame_buffer_ids == NULL) {
		result = HERO_ERROR(ALLOCATION_FAILURE);
		HERO_RESULT_ASSERT(result);
	}
	game.swapchain_frame_buffer_ids = swapchain_frame_buffer_ids;
	game.swapchain_frame_buffers_count = swapchain->images_count;

	HeroImageId attachments[1];

	HeroFrameBufferSetup setup = {
		.attachments = attachments,
		.attachments_count = HERO_ARRAY_COUNT(attachments),
		.layers = swapchain->array_layers_count,
		.render_pass_layout_id = game.render_pass_layout_id,
		.width = swapchain->width,
		.height = swapchain->height,

	};

	for_range(i, 0, swapchain->images_count) {
		attachments[0] = swapchain->image_ids[i];

		result = hero_frame_buffer_init(game.ldev, &setup, &game.swapchain_frame_buffer_ids[i]);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

void game_init(void) {
	HeroSetup hero_setup = {
		.enum_group_strings = NULL
	};

	HeroResult result = hero_init(&hero_setup);
	HERO_RESULT_ASSERT(result);

	result = hero_virt_mem_page_size_get(&hero_virt_mem_page_size, &hero_virt_mem_reserve_align);
	HERO_RESULT_ASSERT(result);

	//hero.last_update_time = hero_time_now(HERO_TIME_MODE_MONOTONIC);

	HeroWindowSysSetup hero_window_sys_setup = {
		.backend_type = HERO_WINDOW_SYS_BACKEND_TYPE_X11,
		.windows_cap = 1,
		.alctor = hero_system_alctor,
	};

	result = hero_window_sys_init(&hero_window_sys_setup);
	HERO_RESULT_ASSERT(result);

	result = hero_window_open(GAME_WINDOW_TITLE, 0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, &game.window_id);
	HERO_RESULT_ASSERT(result);

	HeroGfxSysSetup hero_gfx_sys_setup = {
		.backend_type = HERO_GFX_BACKEND_TYPE_VULKAN,
		.display_manager_type = HERO_GFX_DISPLAY_MANAGER_XLIB,
		.application_name = "RLE",
		.vertex_layouts_cap = 4,
	};

	result = hero_gfx_sys_init(&hero_gfx_sys_setup);
	HERO_RESULT_ASSERT(result);

	HeroPhysicalDevice* physical_device;
	result = hero_physical_device_get(0, &physical_device);
	HERO_RESULT_ASSERT(result);

	HeroLogicalDeviceSetup setup = {
		.feature_flags = physical_device->feature_flags,
		.queue_support_flags = physical_device->queue_support_flags,
		.alctor = hero_system_alctor,
		.buffers_cap = 3,
		.images_cap = 32,
		.samplers_cap = 1,
		.shader_modules_cap = 1,
		.shaders_cap = 1,
		.descriptor_pools_cap = 1,
		.shader_globals_cap = 1,
		.render_pass_layouts_cap = 1,
		.render_passes_cap = 1,
		.frame_buffers_cap = 32,
		.pipeline_caches_cap = 1,
		.pipelines_cap = 1,
		.materials_cap = 1,
		.swapchains_cap = 1,
		.command_pools_cap = 1,
	};

	result = hero_logical_device_init(physical_device, &setup, &game.ldev);
	HERO_RESULT_ASSERT(result);

	result = hero_vertex_layout_register(&vertex_layout, true, &game.vertex_layout_id);
	HERO_RESULT_ASSERT(result);


	HeroShaderModuleId shader_module_id;
	{

		U8* code;
		Uptr code_size;
		result = hero_file_read_all("build/basic.spv", hero_system_alctor, 0, &code, &code_size);
		HERO_RESULT_ASSERT(result);

		HeroShaderModuleSetup setup = {
			.format = HERO_SHADER_FORMAT_SPIR_V,
			.code = code,
			.code_size = code_size,
		};

		result = hero_shader_module_init(game.ldev, &setup, &shader_module_id);
		HERO_RESULT_ASSERT(result);
	}

	HeroShaderStages shader_stages = {0};
	{
		shader_stages.type = HERO_SHADER_TYPE_GRAPHICS;
		shader_stages.data.graphics.vertex = (HeroShaderStage) {
			.backend.spir_v.entry_point_name = "vertex",
			.module_id = shader_module_id,
		};
		shader_stages.data.graphics.fragment = (HeroShaderStage) {
			.backend.spir_v.entry_point_name = "fragment",
			.module_id = shader_module_id,
		};
	}

	HeroShaderMetadata* shader_metadata;
	{
		HeroShaderMetadataSetup setup = {
			.stages = &shader_stages,
		};

		result = hero_shader_metadata_calculate(game.ldev, &setup, &shader_metadata);
		HERO_RESULT_ASSERT(result);
	}

	HeroShaderId shader_id;
	{
		HeroShaderSetup setup = {
			.metadata = shader_metadata,
			.stages = &shader_stages,
		};

		result = hero_shader_init(game.ldev, &setup, &shader_id);
		HERO_RESULT_ASSERT(result);
	}

	HeroDescriptorPoolId descriptor_pool_id;
	{
		HeroDescriptorShaderInfo shader_infos[] = {
			{
				.shader_id = shader_id,
				.advised_pool_counts = {
					[HERO_GFX_DESCRIPTOR_SET_GLOBAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_MATERIAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_DRAW_CMD] = 32,
				},
			},
		};
		HeroDescriptorPoolSetup setup = {
			.shader_infos = shader_infos,
			.shader_infos_count = HERO_ARRAY_COUNT(shader_infos),
		};
		result = hero_descriptor_pool_init(game.ldev, &setup, &descriptor_pool_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_UNIFORM,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 1,
			.elmt_size = sizeof(Mat4x4),
		};

		result = hero_buffer_init(game.ldev, &setup, &game.uniform_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	HeroShaderGlobalsId shader_globals_id;
	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = shader_id,
			.descriptor_pool_id = descriptor_pool_id,
		};

		result = hero_shader_globals_init(game.ldev, &setup, &shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(game.ldev, shader_globals_id, 0, 0, game.uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroAttachmentLayout attachments[] = {
			{
				.format = HERO_SURFACE_IMAGE_FORMAT,
				.samples_count = HERO_SAMPLE_COUNT_1,
				.present = true,
			},
		};
		HeroRenderPassLayoutSetup setup = {
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
		};

		result = hero_render_pass_layout_init(game.ldev, &setup, &game.render_pass_layout_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroAttachmentInfo attachments[] = {
			{
				.load_op = HERO_ATTACHMENT_LOAD_OP_CLEAR,
				.store_op = HERO_ATTACHMENT_STORE_OP_PRESERVE,
				.stencil_load_op = HERO_ATTACHMENT_LOAD_OP_UNINITIALIZED,
				.stencil_store_op = HERO_ATTACHMENT_STORE_OP_DISCARD,
			},
		};

		HeroRenderPassSetup setup = {
			.layout_id = game.render_pass_layout_id,
			.draw_cmd_descriptor_pool_id = descriptor_pool_id,
			.attachment_clear_values = NULL,
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
		};

		result = hero_render_pass_init(game.ldev, &setup, &game.render_pass_id);
		HERO_RESULT_ASSERT(result);
	}


	HeroSwapchain* swapchain;
	{
		HeroSwapchainSetup setup = {
			.window_id = game.window_id,
			.array_layers_count = 1,
			.vsync = true,
			.fifo = false,
		};

		result = hero_swapchain_init(game.ldev, &setup, &game.swapchain_id, &swapchain);
		HERO_RESULT_ASSERT(result);
	}

	result = game_gfx_swapchain_frame_buffers_reinit(swapchain);
	HERO_RESULT_ASSERT(result);

	HeroPipelineId pipeline_id;
	{
		HeroRenderState render_state = {0};
		render_state.topology = HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		render_state.viewports_count = 1;

		render_state.rasterization.polygon_mode = HERO_POLYGON_MODE_FILL;
		render_state.rasterization.front_face = HERO_FRONT_FACE_CLOCKWISE;
		render_state.rasterization.line_width = 1.f;

		render_state.multisample.rasterization_samples_count = HERO_SAMPLE_COUNT_1;

		static HeroRenderStateBlendAttachment blend_color_attachment = {0};
		blend_color_attachment.color_write_mask = 0xf;

		render_state.blend.attachments = &blend_color_attachment;
		render_state.blend.attachments_count = 1;

		render_state.depth_stencil.flags = HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_TEST | HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_WRITE;
		render_state.depth_stencil.depth_compare_op = HERO_COMPARE_OP_LESS_OR_EQUAL;
		render_state.depth_stencil.back.fail_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.back.pass_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.back.compare_op = HERO_COMPARE_OP_ALWAYS;
		render_state.depth_stencil.front.fail_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.front.pass_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.front.compare_op = HERO_COMPARE_OP_ALWAYS;

		HeroPipelineSetup setup = {
			.render_state = &render_state,
			.shader_id = shader_id,
			.render_pass_layout_id = game.render_pass_layout_id,
			.vertex_layout_id = game.vertex_layout_id,
			.cache_id.raw = 0,
		};

		result = hero_pipeline_init(game.ldev, &setup, &pipeline_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroMaterialSetup setup = {
			.pipeline_id = pipeline_id,
			.shader_globals_id = shader_globals_id,
			.descriptor_pool_id = descriptor_pool_id,
		};

		result = hero_material_init(game.ldev, &setup, &game.material_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroCommandPoolSetup setup = {
			.support_static = false,
			.command_buffers_cap = 1,
		};

		result = hero_command_pool_init(game.ldev, &setup, &game.command_pool_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 4,
			.typed.vertex.layout_id = game.vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(game.ldev, &setup, &game.vertex_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_INDEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 6,
			.typed.index_type = HERO_INDEX_TYPE_U32,
		};

		result = hero_buffer_init(game.ldev, &setup, &game.index_buffer_id);
		HERO_RESULT_ASSERT(result);

		U32* indices;
		result = hero_buffer_write(game.ldev, game.index_buffer_id, 0, 6, (void**)&indices);
		HERO_RESULT_ASSERT(result);
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 2;
		indices[4] = 3;
		indices[5] = 0;
	}

	/*

	_HeroSpirVInspect inspect = {0};
	result = _hero_vulkan_shader_metadata_spir_v_inspect((U32*)code, code_size, &inspect);
	HERO_RESULT_ASSERT(result);

	printf("push_constants_size = %u\n", inspect.push_constants_size);
	printf("descriptors_count = %lu\n", inspect.key_to_descriptor_map.count);
	HeroHashTableEntry(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor)* entry;
	Uptr entry_idx = 0;
	while ((result = hero_hash_table(_HeroSpirVDescriptorKey, _HeroSpirVDescriptor, iter_next)(&inspect.key_to_descriptor_map, &entry_idx, &entry)) != HERO_SUCCESS_FINISHED) {
		_HeroSpirVDescriptor d = entry->value;
		printf("DESCRIPTOR { set = %u, binding = %u, type = %u, count = %u, stage_flags = 0x%x, elmt_size = %u }\n", d.set, d.binding.binding, d.binding.descriptor_type, d.binding.descriptor_count, d.binding.stage_flags, d.binding.elmt_size);
	}

	for (U32 i = 0; i < inspect.vertex_attribs.count; i += 1) {
		HeroSpirVVertexAttribInfo* info = &inspect.vertex_attribs.data[i];
		printf("VERTEX_ATTRIB { location: %u, elmt_type: %u, elmts_count: %u }\n", info->location, info->elmt_type, info->vector_type + 1);
	}
	*/
}

void game_update(void) {
}

void game_render(void) {
	HeroResult result;

	result = hero_logical_device_frame_start(game.ldev);
	HERO_RESULT_ASSERT(result);

	U32 swapchain_image_idx;
	HeroSwapchain* swapchain;
	result = hero_swapchain_next_image(game.ldev, game.swapchain_id, &swapchain, &swapchain_image_idx);
	HERO_RESULT_ASSERT(result);
	if (result == HERO_SUCCESS_IS_NEW) {
		result = game_gfx_swapchain_frame_buffers_reinit(swapchain);
		HERO_RESULT_ASSERT(result);
	}

	Mat4x4* mvp;
	result = hero_buffer_write(game.ldev, game.uniform_buffer_id, 0, 1, (void**)&mvp);
	HERO_RESULT_ASSERT(result);

	/*
	static F32 angle = 0.f;
	angle += 1.f / 60.f * 0.25f;

	Mat4x4 view;
	mat4x4_identity(&view);
	mat4x4_translate(&view, VEC3_INIT(0.f, 0.f, 0.f));

	// rotate so increasing Z goes off into the distance.
	mat4x4_scale(&view, VEC3_INIT(1.f, 1.f, -1.f));

	mat4x4_rotate(&view, VEC3_INIT(1.f, 0.f, 0.f), sinf(angle) / 10.f);

	mat4x4_perspective(mvp, 0.785398f, (F32)swapchain->width / (F32)swapchain->height, 0.1f, 250.f);
	mat4x4_mul(mvp, &view, mvp);
	*/
	mat4x4_ortho(mvp, 0.f, swapchain->width, 0.f, swapchain->height, -1.f, 1.f);

	HeroAabb aabb = {
		.x = 80.f,
		.y = 80.f,
		.ex = swapchain->width - 80.f,
		.ey = swapchain->height - 80.f,
	};

	GameVertex* vertices;
	result = hero_buffer_write(game.ldev, game.vertex_buffer_id, 0, 4, (void**)&vertices);
	HERO_RESULT_ASSERT(result);
	vertices[0].pos = VEC2_INIT(aabb.x, aabb.y);
	vertices[0].color = hero_color_init(0xff, 0x00, 0x00, 0xff);
	vertices[1].pos = VEC2_INIT(aabb.ex, aabb.y);
	vertices[1].color = hero_color_init(0x00, 0xff, 0x00, 0xff);
	vertices[2].pos = VEC2_INIT(aabb.ex, aabb.ey);
	vertices[2].color = hero_color_init(0x00, 0x00, 0xff, 0xff);
	vertices[3].pos = VEC2_INIT(aabb.x, aabb.ey);
	vertices[3].color = hero_color_init(0x00, 0xff, 0xff, 0xff);

	result = hero_logical_device_submit_start(game.ldev);
	HERO_RESULT_ASSERT(result);

	HeroCommandRecorder* command_recorder;
	result = hero_command_recorder_start(game.ldev, game.command_pool_id, &command_recorder);
	HERO_RESULT_ASSERT(result);
	{
		result = hero_cmd_render_pass_start(command_recorder, game.render_pass_id, game.swapchain_frame_buffer_ids[swapchain_image_idx], NULL, NULL);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_start(command_recorder, game.material_id);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_set_vertex_buffer(command_recorder, game.vertex_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_end_indexed(command_recorder, game.index_buffer_id, 0, 6, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_render_pass_end(command_recorder);
		HERO_RESULT_ASSERT(result);

	}

	HeroCommandPoolBufferId command_buffer_id;
	result = hero_command_recorder_end(command_recorder, &command_buffer_id);
	HERO_RESULT_ASSERT(result);

	result = hero_logical_device_submit_command_buffers(game.ldev, game.command_pool_id, &command_buffer_id, 1);
	HERO_RESULT_ASSERT(result);

	result = hero_logical_device_submit_end(game.ldev, &game.swapchain_id, 1);
	HERO_RESULT_ASSERT(result);
}

int main(int argc, char** argv) {
	game_init();

	while (1) {
		HeroResult result = hero_events_poll();
		HERO_RESULT_ASSERT(result);

		U32 idx = 0;
		HeroEvent* e;
		while ((result = hero_events_iter_next(&e, &idx)) != HERO_SUCCESS_FINISHED) {
			HeroWindow* window;
			result = hero_window_get(e->any.window_id, &window);
			HERO_RESULT_ASSERT(result);

			switch (e->any.type) {
				case HERO_EVENT_TYPE_WINDOW_MOVED:
					break;
				case HERO_EVENT_TYPE_WINDOW_RESIZED:
					break;
				case HERO_EVENT_TYPE_WINDOW_CLOSE_REQUEST:
					exit(1);
					break;
				case HERO_EVENT_TYPE_WINDOW_CLOSED:
					break;
				case HERO_EVENT_TYPE_KEYBOARD_ENTER:
					break;
				case HERO_EVENT_TYPE_KEYBOARD_LEAVE:
					break;
				case HERO_EVENT_TYPE_KEYBOARD_KEY_UP:
					break;
				case HERO_EVENT_TYPE_KEYBOARD_KEY_DOWN:
					break;
				case HERO_EVENT_TYPE_KEYBOARD_KEY_INPUT_UTF32: {
					break;
				};
				case HERO_EVENT_TYPE_MOUSE_ENTER:
					break;
				case HERO_EVENT_TYPE_MOUSE_LEAVE:
					break;
				case HERO_EVENT_TYPE_MOUSE_MOVED:
					break;
				case HERO_EVENT_TYPE_MOUSE_BUTTON_UP:
					break;
				case HERO_EVENT_TYPE_MOUSE_BUTTON_DOWN:
					break;
				case HERO_EVENT_TYPE_MOUSE_WHEEL_MOVED:
					break;
			}
#if 0
			printf("event [%s]: window_id = %u", HeroEventType_strings[e->any.type], e->any.window_id.raw);
			switch (e->any.type) {
				case HERO_EVENT_TYPE_WINDOW_MOVED:
					printf(", x: %u, y: %u, rel_x: %d, rel_y: %d\n", e->moved.x, e->moved.y, e->moved.rel_x, e->moved.rel_y);
					break;
				case HERO_EVENT_TYPE_WINDOW_RESIZED:
					printf(", width: %u, height: %u, rel_width: %d, rel_height: %d\n", e->resized.width, e->resized.height, e->resized.rel_width, e->resized.rel_height);
					break;
				case HERO_EVENT_TYPE_WINDOW_CLOSE_REQUEST:
					printf("\n");
					exit(1);
					break;
				case HERO_EVENT_TYPE_WINDOW_CLOSED:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_KEYBOARD_ENTER:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_KEYBOARD_LEAVE:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_KEYBOARD_KEY_UP:
					printf(", key_code: %s, scan_code: %s, mod: %x, is_repeat: %u\n", HeroKeyCode_strings[e->key.code], HeroKeyCode_strings[e->key.scan_code], e->key.mod, e->key.is_repeat);
					break;
				case HERO_EVENT_TYPE_KEYBOARD_KEY_DOWN:
					printf(", key_code: %s, scan_code: %s, mod: %x, is_repeat: %u\n", HeroKeyCode_strings[e->key.code], HeroKeyCode_strings[e->key.scan_code], e->key.mod, e->key.is_repeat);
					break;
				case HERO_EVENT_TYPE_KEYBOARD_KEY_INPUT_UTF32: {
					printf(", codepoint: %.*s\n", e->key_input_utf32.size, e->key_input_utf32.bytes);

					HeroWindow* window;
					result = hero_window_get_ptr(hero.window_sys.keyboard.focused_window_id, &window);
					HERO_RESULT_ASSERT(result);

					result = hero_window_handle_input(window, hero_string((hero_u8*)e->key_input_utf32.bytes, e->key_input_utf32.size));
					HERO_RESULT_ASSERT(result);
					break;
				};
				case HERO_EVENT_TYPE_MOUSE_ENTER:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_MOUSE_LEAVE:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_MOUSE_MOVED:
					printf(", x: %u, y: %u, rel_x: %d, rel_y: %d\n", e->moved.x, e->moved.y, e->moved.rel_x, e->moved.rel_y);
					break;
				case HERO_EVENT_TYPE_MOUSE_BUTTON_UP:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_MOUSE_BUTTON_DOWN:
					printf("\n");
					break;
				case HERO_EVENT_TYPE_MOUSE_WHEEL_MOVED:
					printf("\n");
					break;
			}
#endif
		}

		game_update();

		game_render();
	}

	return 0;
}


