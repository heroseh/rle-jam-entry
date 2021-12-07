#include "game.h"

#define FNL_IMPL
#include <deps/FastNoiseLite.h>

#define STB_IMAGE_IMPLEMENTATION
#include <deps/stb_image.h>

// ===========================================
//
//
// Logic
//
//
// ===========================================

HeroUIImageId game_ui_ascii_image(U8 byte) {
	return HERO_UI_IMAGE_ID(game.ui_ascii_image_atlas_id, byte);
}

void game_logic_init(void) {
	game.global_ubo.mountain_min = 0.69;
	game.global_ubo.sea_max = 0.3;
	game.falloff_map_smooth_size = 3.f;
	game.falloff_map_edge_offset = 2.2f;
}

Vec2 game_ui_text_size(HeroUIWindow* window, HeroString string, F32 wrap_at_width, HeroUIWidgetStyle* style) {
	const U32 GLYPH_WIDTH = 14;
	const U32 GLYPH_HEIGHT = 14;

	Uptr idx = 0;
	Codept codept;
	F32 width = 0;
	F32 max_width = 0;
	F32 height = style->text_line_height;
	bool wrap = style->text_wrap;
	while (hero_utf8_codept_iter(string, &idx, &codept)) {
		HERO_ASSERT(codept < 128, "only support ascii text right nowwwww");

		F32 advance = (F32)GLYPH_WIDTH;
		F32 new_width = width + advance;
		if (wrap && new_width > wrap_at_width) {
			max_width = HERO_MAX(max_width, width);
			width = advance;
			height += style->text_line_height;
		} else {
			width = new_width;
		}
	}
	max_width = HERO_MAX(max_width, width);

	F32 scale = style->text_line_height / (float)GLYPH_HEIGHT;
	return VEC2_INIT(max_width * scale, height);
}

HeroResult game_ui_text_render(HeroUIWindow* window, Vec2 top_left, HeroString string, F32 wrap_at_width, HeroUIWidgetStyle* style) {
	const U32 GLYPH_WIDTH = 14;
	const U32 GLYPH_HEIGHT = 14;
	F32 scale = style->text_line_height / (float)GLYPH_HEIGHT;

	Uptr idx = 0;
	Codept codept;
	F32 width = 0;
	F32 max_width = 0;
	F32 height = style->text_line_height;
	F32 start_x = top_left.x;
	Vec2 position = top_left;
	bool wrap = style->text_wrap;
	F32 glyph_height = GLYPH_HEIGHT * scale;
	while (hero_utf8_codept_iter(string, &idx, &codept)) {
		HERO_ASSERT(codept < 128, "only support ascii text right nowwwww");

		F32 glyph_width = (F32)GLYPH_WIDTH * scale;

		//
		// sub pixel positioning
		F32 shift_x = position.x - floorf(position.x);
		F32 shift_y = position.y - floorf(position.y);

		HeroAabb aabb;
		aabb.x = position.x - shift_x;
		aabb.y = position.y - shift_y;
		aabb.ex = aabb.x + glyph_width;
		aabb.ey = aabb.y + glyph_height;
		HeroResult result = hero_ui_widget_draw_image_grayscale_remap(window, &aabb, game_ui_ascii_image(codept), 0, style->text_color);
		if (result < 0) {
			return result;
		}

		F32 new_width = width + glyph_width;
		if (wrap && new_width > wrap_at_width) {
			max_width = HERO_MAX(max_width, width);
			width = glyph_width;
			position.x = start_x;
			position.y += style->text_line_height;
		} else {
			width = new_width;
			position.x += glyph_width;
		}
	}
	max_width = HERO_MAX(max_width, width);

	return HERO_SUCCESS;
}

void game_ui_value_adjuster_f(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroString name, float* value, float step) {
	hero_ui_box_start(window, sib_id, cut, HERO_UI_LEN_AUTO, hero_ui_ss.box);

		HeroColor fg_color = hero_color_init(0xff, 0, 0, 0xff);
		HeroUIWidgetId widget_id;

		widget_id = hero_ui_text_button(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("<"), hero_ui_ss.button);
		if (hero_ui_focus_state(window, widget_id) & HERO_UI_FOCUS_STATE_RELEASED) {
			*value -= step;
		}

		hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, 350.f, name, hero_ui_ss.text);

		widget_id = hero_ui_text_button(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit(">"), hero_ui_ss.button);
		if (hero_ui_focus_state(window, widget_id) & HERO_UI_FOCUS_STATE_RELEASED) {
			*value += step;
		}

	hero_ui_box_end(window);
}

void game_ui_value_adjuster_i(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroString name, int* value, int step) {
	hero_ui_box_start(window, sib_id, cut, HERO_UI_LEN_AUTO, hero_ui_ss.box);

		HeroColor fg_color = hero_color_init(0xff, 0, 0, 0xff);
		HeroUIWidgetId widget_id;

		widget_id = hero_ui_text_button(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("<"), hero_ui_ss.button);
		if (hero_ui_focus_state(window, widget_id) & HERO_UI_FOCUS_STATE_RELEASED) {
			*value -= step;
		}

		hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, 350.f, name, hero_ui_ss.text);

		widget_id = hero_ui_text_button(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit(">"), hero_ui_ss.button);
		if (hero_ui_focus_state(window, widget_id) & HERO_UI_FOCUS_STATE_RELEASED) {
			*value += step;
		}

	hero_ui_box_end(window);
}

//
// max must be in the range of 0.f to 1.f
F32 game_falloff_map_generate_at(F32 max) {
	F32 edge = game.falloff_map_smooth_size;
	F32 offset = game.falloff_map_edge_offset;

	F32 pow_edge = powf(max, edge);

	return pow_edge / (pow_edge + powf(offset - (offset * max), edge));
}

void game_logic_update(void) {
	HeroResult result;

	HeroUIWindow* window = hero_ui_window_start(game.ui_window_id, game.gfx.render_width, game.gfx.render_height);

	fnl_state noise_state = game.noise_state;
	F32 falloff_map_smooth_size = game.falloff_map_smooth_size;
	F32 falloff_map_edge_offset = game.falloff_map_edge_offset;
	int show_falloff_map = game.global_ubo.show_falloff_map;

	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_RIGHT, HERO_UI_LEN_AUTO, NULL);

		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("frequency"), &game.noise_state.frequency, 0.001f);
		game_ui_value_adjuster_i(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("fractal type"), (int*)&game.noise_state.fractal_type, 1);
		game_ui_value_adjuster_i(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("octaves"), &game.noise_state.octaves, 1);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("lacunarity"), &game.noise_state.lacunarity, 0.1f);

		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("mountain min"), &game.global_ubo.mountain_min, 0.1f);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("sea max"), &game.global_ubo.sea_max, 0.1f);

		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("falloff smooth size"), &game.falloff_map_smooth_size, 0.25f);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("falloff edge offset"), &game.falloff_map_edge_offset, 0.25f);

		HeroUIWidgetId widget_id;
		widget_id = hero_ui_text_toggle_button(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("show falloff map"), hero_ui_ss.button);
		game.global_ubo.show_falloff_map = !!(hero_ui_widget_state(window, widget_id) & HERO_UI_WIDGET_STATE_ACTIVE);
	hero_ui_box_end(window);

	hero_ui_window_end(window);

	static int init = 0;

	//noise_state.octaves = game.octaves;
	//noise_state.lacunarity = game.lacunarity;
	if (
		!init ||
		!HERO_CMP_ELMT(&noise_state, &game.noise_state) ||
		falloff_map_smooth_size != game.falloff_map_smooth_size ||
		falloff_map_edge_offset != game.falloff_map_edge_offset ||
		show_falloff_map != game.global_ubo.show_falloff_map
	) {
		init += 1;
		F32* pixels;
		result = hero_image_write(game.gfx.ldev, game.gfx.noise_image_id, NULL, (void**)&pixels);

		for_range(y, 0, 960) {
			for_range(x, 0, 960) {
				F32 height = 0.f;
				F32 falloff_x = (((F32)x / 960.f) * 2.f) - 1.f;
				F32 falloff_y = (((F32)y / 960.f) * 2.f) - 1.f;
				if (game.global_ubo.show_falloff_map) {
					height = game_falloff_map_generate_at(HERO_MAX(fabsf(falloff_x), fabsf(falloff_y)));
				} else {
					height = fnlGetNoise2D(&game.noise_state, x, y) * 0.5f + 0.5f; // convert to 0.0 - 1.0 from -1.0 - 1.0
					height -= game_falloff_map_generate_at(HERO_MAX(fabsf(falloff_x), fabsf(falloff_y)));
				}

				pixels[y * 960 + x] = height;
			}
		}
	}
}

// ===========================================
//
//
// Graphics: Common
//
//
// ===========================================

HeroResult game_gfx_shader_init(const char* name, HeroShaderId* id_out) {
	HeroResult result;

	HeroShaderModuleId shader_module_id;
	{
		char file_path[128];
		snprintf(file_path, sizeof(file_path), "build/%s.spv", name);

		U8* code;
		Uptr code_size;
		result = hero_file_read_all(file_path, hero_system_alctor, 0, &code, &code_size);
		HERO_RESULT_ASSERT(result);

		HeroShaderModuleSetup setup = {
			.format = HERO_SHADER_FORMAT_SPIR_V,
			.code = code,
			.code_size = code_size,
		};

		result = hero_shader_module_init(game.gfx.ldev, &setup, &shader_module_id);
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

		result = hero_shader_metadata_calculate(game.gfx.ldev, &setup, &shader_metadata);
		HERO_RESULT_ASSERT(result);
	}

	HeroShaderId shader_id;
	{
		HeroShaderSetup setup = {
			.metadata = shader_metadata,
			.stages = &shader_stages,
		};

		result = hero_shader_init(game.gfx.ldev, &setup, &shader_id);
		HERO_RESULT_ASSERT(result);
	}

	*id_out = shader_id;

	return HERO_SUCCESS;
}

HeroResult game_gfx_swapchain_frame_buffers_reinit(HeroSwapchain* swapchain) {
	HeroResult result;

	for_range (i, 0, game.gfx.swapchain_frame_buffers_count) {
		result = hero_frame_buffer_deinit(game.gfx.ldev, game.gfx.swapchain_frame_buffer_ids[i]);
		HERO_RESULT_ASSERT(result);
	}

	HeroFrameBufferId* swapchain_frame_buffer_ids = hero_realloc_array(HeroFrameBufferId, hero_system_alctor, HERO_ALLOC_TAG_TODO, game.gfx.swapchain_frame_buffer_ids, game.gfx.swapchain_frame_buffers_count, swapchain->images_count);
	if (swapchain_frame_buffer_ids == NULL) {
		result = HERO_ERROR(ALLOCATION_FAILURE);
		HERO_RESULT_ASSERT(result);
	}
	game.gfx.swapchain_frame_buffer_ids = swapchain_frame_buffer_ids;
	game.gfx.swapchain_frame_buffers_count = swapchain->images_count;

	HeroImageId attachments[1];

	HeroFrameBufferSetup setup = {
		.attachments = attachments,
		.attachments_count = HERO_ARRAY_COUNT(attachments),
		.layers = swapchain->array_layers_count,
		.render_pass_layout_id = game.gfx.render_pass_layout_id,
		.width = swapchain->width,
		.height = swapchain->height,

	};

	for_range(i, 0, swapchain->images_count) {
		attachments[0] = swapchain->image_ids[i];

		result = hero_frame_buffer_init(game.gfx.ldev, &setup, &game.gfx.swapchain_frame_buffer_ids[i]);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Graphics: World Map
//
//
// ===========================================


// ===========================================
//
//
// Graphics
//
//
// ===========================================

typedef struct GameVertex GameVertex;
struct GameVertex {
	Vec2 pos;
	Vec2 uv;
};

static HeroVertexAttribInfo vertex_attribs[] = {
	{ // GameVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
	{ // GameVertex.uv
		.location = 1,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
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

void game_gfx_init(void) {
	HeroResult result;

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
		.buffers_cap = 32,
		.images_cap = 32,
		.samplers_cap = 32,
		.shader_modules_cap = 32,
		.shaders_cap = 32,
		.descriptor_pools_cap = 32,
		.shader_globals_cap = 32,
		.render_pass_layouts_cap = 32,
		.render_passes_cap = 32,
		.frame_buffers_cap = 32,
		.pipeline_caches_cap = 32,
		.pipelines_cap = 32,
		.materials_cap = 32,
		.swapchains_cap = 32,
		.command_pools_cap = 32,
	};

	result = hero_logical_device_init(physical_device, &setup, &game.gfx.ldev);
	HERO_RESULT_ASSERT(result);

	result = hero_vertex_layout_register(&vertex_layout, true, &game.gfx.vertex_layout_id);
	HERO_RESULT_ASSERT(result);


	HeroShaderId shader_id;
	result = game_gfx_shader_init("basic", &shader_id);
	HERO_RESULT_ASSERT(result);

	result = game_gfx_shader_init("ui", &game.gfx.ui_shader_id);
	HERO_RESULT_ASSERT(result);

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
			{
				.shader_id = game.gfx.ui_shader_id,
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
		result = hero_descriptor_pool_init(game.gfx.ldev, &setup, &game.gfx.descriptor_pool_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_UNIFORM,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 1,
			.elmt_size = sizeof(GameBasicGlobalUBO),
		};

		result = hero_buffer_init(game.gfx.ldev, &setup, &game.gfx.uniform_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R32_SFLOAT,
			.format = HERO_IMAGE_FORMAT_R32_SFLOAT,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_SAMPLED,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = 960,
			.height = 960,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(game.gfx.ldev, &setup, &game.gfx.noise_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		int width, height, comp;
		U8* src_pixels = stbi_load("assets/font-ascii.png", &width, &height, &comp, 1);
		if (src_pixels == NULL) {
			result = HERO_ERROR(ALLOCATION_FAILURE);
			HERO_RESULT_ASSERT(result);
		}

		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R8_UNORM,
			.format = HERO_IMAGE_FORMAT_R8_UNORM,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_SAMPLED,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = width,
			.height = height,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(game.gfx.ldev, &setup, &game.gfx.font_ascii_image_id);
		HERO_RESULT_ASSERT(result);

		U8* dst_pixels;
		result = hero_image_write(game.gfx.ldev, game.gfx.font_ascii_image_id, NULL, (void**)&dst_pixels);

		memcpy(dst_pixels, src_pixels, width * height);
	}

	{
		HeroSamplerSetup setup = {0};
		setup.mag_filter = HERO_FILTER_NEAREST,
		setup.min_filter = HERO_FILTER_NEAREST,
		setup.address_mode_u = HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		setup.address_mode_v = HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		setup.address_mode_w = HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,

		result = hero_sampler_init(game.gfx.ldev, &setup, &game.gfx.clamp_nearest_sampler_id);
		HERO_RESULT_ASSERT(result);
	}

	HeroShaderGlobalsId shader_globals_id;
	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = shader_id,
			.descriptor_pool_id = game.gfx.descriptor_pool_id,
		};

		result = hero_shader_globals_init(game.gfx.ldev, &setup, &shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(game.gfx.ldev, shader_globals_id, 0, 0, game.gfx.uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_image_sampler(game.gfx.ldev, shader_globals_id, 1, 0, game.gfx.noise_image_id, game.gfx.clamp_nearest_sampler_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(game.gfx.ldev, shader_globals_id);
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

		result = hero_render_pass_layout_init(game.gfx.ldev, &setup, &game.gfx.render_pass_layout_id);
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
			.layout_id = game.gfx.render_pass_layout_id,
			.draw_cmd_descriptor_pool_id = game.gfx.descriptor_pool_id,
			.attachment_clear_values = NULL,
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
		};

		result = hero_render_pass_init(game.gfx.ldev, &setup, &game.gfx.render_pass_id);
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

		result = hero_swapchain_init(game.gfx.ldev, &setup, &game.gfx.swapchain_id, &swapchain);
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
			.render_pass_layout_id = game.gfx.render_pass_layout_id,
			.vertex_layout_id = game.gfx.vertex_layout_id,
			.cache_id.raw = 0,
		};

		result = hero_pipeline_init(game.gfx.ldev, &setup, &pipeline_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroMaterialSetup setup = {
			.pipeline_id = pipeline_id,
			.shader_globals_id = shader_globals_id,
			.descriptor_pool_id = game.gfx.descriptor_pool_id,
		};

		result = hero_material_init(game.gfx.ldev, &setup, &game.gfx.material_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroCommandPoolSetup setup = {
			.support_static = false,
			.command_buffers_cap = 1,
		};

		result = hero_command_pool_init(game.gfx.ldev, &setup, &game.gfx.command_pool_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 4,
			.typed.vertex.layout_id = game.gfx.vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(game.gfx.ldev, &setup, &game.gfx.vertex_buffer_id);
		HERO_RESULT_ASSERT(result);

		HeroAabb aabb = {
			.x = 80.f,
			.y = 80.f,
			.ex = 960.f + 80.f,
			.ey = 960.f + 80.f,
		};

		{
			GameVertex* vertices;
			result = hero_buffer_write(game.gfx.ldev, game.gfx.vertex_buffer_id, 0, 4, (void**)&vertices);
			HERO_RESULT_ASSERT(result);
			vertices[0].pos = VEC2_INIT(aabb.x, aabb.y);
			vertices[0].uv = VEC2_INIT(0.f, 0.f);
			vertices[1].pos = VEC2_INIT(aabb.ex, aabb.y);
			vertices[1].uv = VEC2_INIT(1.f, 0.f);
			vertices[2].pos = VEC2_INIT(aabb.ex, aabb.ey);
			vertices[2].uv = VEC2_INIT(1.f, 1.f);
			vertices[3].pos = VEC2_INIT(aabb.x, aabb.ey);
			vertices[3].uv = VEC2_INIT(0.f, 1.f);
		}
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

		result = hero_buffer_init(game.gfx.ldev, &setup, &game.gfx.index_buffer_id);
		HERO_RESULT_ASSERT(result);

		U32* indices;
		result = hero_buffer_write(game.gfx.ldev, game.gfx.index_buffer_id, 0, 6, (void**)&indices);
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

void game_gfx_render(void) {
	HeroResult result;

	result = hero_logical_device_frame_start(game.gfx.ldev);
	HERO_RESULT_ASSERT(result);

	HeroSwapchain* swapchain;
	result = hero_swapchain_next_image(game.gfx.ldev, game.gfx.swapchain_id, &swapchain, &game.gfx.swapchain_image_idx);
	HERO_RESULT_ASSERT(result);
	if (result == HERO_SUCCESS_IS_NEW) {
		result = game_gfx_swapchain_frame_buffers_reinit(swapchain);
		HERO_RESULT_ASSERT(result);
	}
	game.gfx.render_width = swapchain->width;
	game.gfx.render_height = swapchain->height;

	GameBasicGlobalUBO* global_ubo;
	result = hero_buffer_write(game.gfx.ldev, game.gfx.uniform_buffer_id, 0, 1, (void**)&global_ubo);
	HERO_RESULT_ASSERT(result);
	*global_ubo = game.global_ubo;

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
	mat4x4_ortho(&global_ubo->mvp, 0.f, swapchain->width, swapchain->height, 0.f, -1.f, 1.f);

	HeroCommandRecorder* command_recorder;
	result = hero_command_recorder_start(game.gfx.ldev, game.gfx.command_pool_id, &command_recorder);
	HERO_RESULT_ASSERT(result);

	result = hero_cmd_render_pass_start(command_recorder, game.gfx.render_pass_id, game.gfx.swapchain_frame_buffer_ids[game.gfx.swapchain_image_idx], NULL, NULL);
	HERO_RESULT_ASSERT(result);

	{
		result = hero_cmd_draw_start(command_recorder, game.gfx.material_id);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_set_vertex_buffer(command_recorder, game.gfx.vertex_buffer_id, 0, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_end_indexed(command_recorder, game.gfx.index_buffer_id, 0, 6, 0);
		HERO_RESULT_ASSERT(result);
	}

	result = hero_ui_window_render(game.ui_window_id, game.gfx.ldev, command_recorder);
	HERO_RESULT_ASSERT(result);

	result = hero_cmd_render_pass_end(command_recorder);
	HERO_RESULT_ASSERT(result);

	HeroCommandPoolBufferId command_buffer_id;
	result = hero_command_recorder_end(command_recorder, &command_buffer_id);
	HERO_RESULT_ASSERT(result);

	result = hero_logical_device_queue_transfer(game.gfx.ldev);

	result = hero_logical_device_queue_command_buffers(game.gfx.ldev, game.gfx.command_pool_id, &command_buffer_id, 1);
	HERO_RESULT_ASSERT(result);

	result = hero_logical_device_submit(game.gfx.ldev, &game.gfx.swapchain_id, 1);
	HERO_RESULT_ASSERT(result);
}

// ===========================================
//
//
// Game
//
//
// ===========================================

Game game;

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

	game_gfx_init();

	{
		HeroUISysSetup hero_ui_sys_setup = {
			.ldev = game.gfx.ldev,
			.windows_cap = 1,
			.image_atlases_cap = 16,
			.shader_id = game.gfx.ui_shader_id,
			.descriptor_pool_id = game.gfx.descriptor_pool_id,
			.render_pass_layout_id = game.gfx.render_pass_layout_id,
			.text_size_fn = game_ui_text_size,
			.text_render_fn = game_ui_text_render,
		};

		result = hero_ui_sys_init(&hero_ui_sys_setup);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroUIWindowSetup setup = {
			.widgets_cap = 1024,
			.ldev = game.gfx.ldev,
			.window_id = game.window_id,
		};

		result = hero_ui_window_init(&setup, &game.ui_window_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroUIImageAtlasSetup setup = {
			.image_id = game.gfx.font_ascii_image_id,
			.sampler_id = game.gfx.clamp_nearest_sampler_id,
			.images_count = 16 * 16,
			.is_uniform = true,
			.data.uniform.cell_width = 14,
			.data.uniform.cell_height = 14,
			.data.uniform.cells_count_x = 16,
		};

		result = hero_ui_image_atlas_init(game.gfx.ldev, &setup, &game.ui_ascii_image_atlas_id);
		HERO_RESULT_ASSERT(result);
	}

	game_logic_init();

	game.noise_state = fnlCreateState();
	game.noise_state.fractal_type = 1;
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

		game_logic_update();

		game_gfx_render();
	}

	return 0;
}
