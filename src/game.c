#include "game.h"

#define ENABLE_VOXEL_RAYTRACTER 0
#define GAME_ENTITIES_CAP 128
#define GAME_PLAYER_MOVE_SPEED 3.f
#define GAME_CAMERA_TARGET_OFFSET VEC3_INIT(0.5f, 8.f, -10.f)
#define GAME_MOVE_RADIUS_COLOR_VALID hero_color_init(0x20, 0xaa, 0x30, 0xff)
#define GAME_MOVE_RADIUS_COLOR_VALID_SPLASH hero_color_init(0x20, 0x70, 0xaa, 0xff)
#define GAME_MOVE_RADIUS_COLOR_INVALID hero_color_init(0xff, 0x20, 0x30, 0xff)
#define GAME_MOVE_RADIUS_COLOR_TARGET hero_color_init(0x00, 0x66, 0x10, 0xff)
#define GAME_MOVE_RADIUS_COLOR_TARGET_SPLASH hero_color_init(0x00, 0x20, 0x66, 0xff)
#define GAME_PLAYER_MOVE_ANIMATE_SECS 1.f
#define GAME_PLAYER_AIM_DIRECTION_ROTATE_SPEED_RADIUS_PER_SEC HERO_DEGREES_TO_RADIANS(180.f)

#define FNL_IMPL
#include <deps/FastNoiseLite.h>

#define STB_IMAGE_IMPLEMENTATION
#include <deps/stb_image.h>

// ===========================================
//
//
// UI
//
//
// ===========================================

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

void game_ui_value_adjuster_u(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroString name, U32* value, int step) {
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

// ===========================================
//
//
// Island Generator
//
//
// ===========================================

GameIslandGenConfig game_island_gen_configs[GAME_ISLAND_GEN_CONFIG_COUNT] = {
	{
		.seed = 1337,
		.frequency = 0.001000,
		.fractal_type = 1,
		.octaves = 7,
		.lacunarity = 2.160000,
		.deep_sea_max = 25.6,
		.sea_max = 51.2,
		.ground_max = 125.44,
		.falloff_map_intensity = 50.000000,
	},
	{
		.seed = 1337,
		.frequency = 0.001000,
		.fractal_type = 2,
		.octaves = 7,
		.lacunarity = 1.900000,
		.deep_sea_max = 25.6,
		.sea_max = 51.2,
		.ground_max = 138.24,
		.falloff_map_intensity = 10.0,
	},
	{
		.seed = 1337,
		.frequency = 0.001000,
		.fractal_type = 3,
		.octaves = 7,
		.lacunarity = 2.160000,
		.deep_sea_max = 25.6,
		.sea_max = 51.2,
		.ground_max = 125.44,
		.falloff_map_intensity = 50.000000,
	},
};

void game_island_gen(GameIslandGenConfig* config, F32* tile_height_map_out) {
	fnl_state noise_state = fnlCreateState();
	noise_state.seed = config->seed;
	noise_state.frequency = config->frequency;
	noise_state.fractal_type = config->fractal_type;
	noise_state.octaves = config->octaves;
	noise_state.lacunarity = config->lacunarity;

	F32 half_size = (F32)(GAME_ISLAND_AXIS_TILES_COUNT / 2);
	F32 map_to_norm_scale = 1.f / (F32)GAME_ISLAND_AXIS_TILES_COUNT;
	for_range(y, 0, GAME_ISLAND_AXIS_TILES_COUNT) {
		U32 row_start = y * GAME_ISLAND_AXIS_TILES_COUNT;
		for_range(x, 0, GAME_ISLAND_AXIS_TILES_COUNT) {
			Vec2 falloff_offset = VEC2_INIT(x - half_size, y - half_size);
			F32 falloff_height = (vec2_len(falloff_offset) + config->falloff_map_intensity) * map_to_norm_scale;
			F32 height = fnlGetNoise2D(&noise_state, x, y) * 0.5f + 0.5f; // convert to 0.0 - 1.0 from -1.0 - 1.0
			height -= falloff_height;
			height = HERO_MAX(height, 0.01f);
			height *= 256.f;

			tile_height_map_out[row_start + x] = height;
		}
	}
}

// ===========================================
//
//
// Scene: Island Generator Debug
//
//
// ===========================================

HeroResult game_scene_island_gen_debug_load(GameScene* s) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_ISLAND_GEN_DEBUG, "expected a island gen debug scene");
	GameSceneIslandGenDebug* scene = (GameSceneIslandGenDebug*)s;
	HeroResult result;

	scene->config = game_island_gen_configs[0];

	F32* tile_height_map = hero_alloc_array(F32, hero_system_alctor, 0, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	if (tile_height_map == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	scene->render_data.tile_height_map = tile_height_map;

	{
		HeroUIImageAtlasSetup setup = {
			.image_id = game.gfx.island_gen_debug.attachment_image_id,
			.sampler_id = game.gfx.clamp_nearest_sampler_id,
			.images_count = 1,
			.is_uniform = true,
			.data.uniform.cell_width = GAME_ISLAND_AXIS_TILES_COUNT,
			.data.uniform.cell_height = GAME_ISLAND_AXIS_TILES_COUNT,
			.data.uniform.cells_count_x = 1,
		};

		result = hero_ui_image_atlas_init(game.gfx.ldev, &setup, &scene->colored_height_map_image_atlas_id);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

HeroResult game_scene_island_gen_debug_unload(GameScene* s) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_ISLAND_GEN_DEBUG, "expected a island gen debug scene");
	GameSceneIslandGenDebug* scene = (GameSceneIslandGenDebug*)s;

	return HERO_SUCCESS;
}

HeroResult game_scene_island_gen_debug_update(GameScene* s, GameScene** replace_with_scene_out) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_ISLAND_GEN_DEBUG, "expected a island gen debug scene");
	GameSceneIslandGenDebug* scene = (GameSceneIslandGenDebug*)s;
	GameIslandGenConfig config = scene->config;
	HeroResult result;

	HeroUIWindow* window;
	result = hero_ui_window_get(game.ui_window_id, &window);
	if (result < 0) {
		return result;
	}

	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_FILL, NULL);
		hero_ui_box_start(window, __LINE__, HERO_UI_CUT_CENTER_HORIZONTAL, HERO_UI_LEN_AUTO, NULL);
			hero_ui_box_start(window, __LINE__, HERO_UI_CUT_CENTER_VERTICAL, HERO_UI_LEN_AUTO, NULL);
				hero_ui_image(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_IMAGE_ID(scene->colored_height_map_image_atlas_id, 0), -1, 1.f, HERO_UI_IMAGE_SCALE_MODE_NONE, HERO_UI_IMAGE_FLIP_Y, NULL);
			hero_ui_box_end(window);
		hero_ui_box_end(window);
	hero_ui_box_end(window);

	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_RIGHT, HERO_UI_LEN_AUTO, NULL);
		game_ui_value_adjuster_u(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("seed"), &config.seed, 1);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("frequency"), &config.frequency, 0.001f);
		game_ui_value_adjuster_u(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("fractal type"), &config.fractal_type, 1);
		game_ui_value_adjuster_u(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("octaves"), &config.octaves, 1);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("lacunarity"), &config.lacunarity, 0.01f);

		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("deep sea max"), &config.deep_sea_max, 0.01f);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("sea max"), &config.sea_max, 0.01f);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("ground_max"), &config.ground_max, 0.01f);
		game_ui_value_adjuster_f(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("falloff intensity"), &config.falloff_map_intensity, 10.f);

		HeroUIWidgetId widget_id;
		widget_id = hero_ui_text_toggle_button(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("show height map"), hero_ui_ss.button);
		scene->render_data.show_height_map = !!(hero_ui_widget_state(window, widget_id) & HERO_UI_WIDGET_STATE_ACTIVE);

		widget_id = hero_ui_text_toggle_button(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("show grid"), hero_ui_ss.button);
		scene->render_data.show_grid = !!(hero_ui_widget_state(window, widget_id) & HERO_UI_WIDGET_STATE_ACTIVE);

		U32 config_idx = scene->config_idx;
		game_ui_value_adjuster_u(window, __LINE__, HERO_UI_CUT_TOP, hero_string_lit("config"), &scene->config_idx, 1);
		scene->config_idx = HERO_MIN(scene->config_idx, HERO_ARRAY_COUNT(game_island_gen_configs) - 1);
		if (config_idx != scene->config_idx) {
			config = game_island_gen_configs[scene->config_idx];
		}
	hero_ui_box_end(window);

	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_RIGHT, HERO_UI_LEN_AUTO, NULL);
		widget_id = hero_ui_text_button(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("print"), hero_ui_ss.button);
		if (hero_ui_focus_state(window, widget_id) & HERO_UI_FOCUS_STATE_RELEASED) {
			printf(".seed = %u\n", config.seed);
			printf(".frequency = %f\n", config.frequency);
			printf(".fractal_type = %u\n", config.fractal_type);
			printf(".octaves = %u\n", config.octaves);
			printf(".lacunarity = %f\n", config.lacunarity);
			printf(".deep_sea_max = %f\n", config.deep_sea_max);
			printf(".sea_max = %f\n", config.sea_max);
			printf(".ground_max = %f\n", config.ground_max);
			printf(".falloff_map_intensity = %f\n", config.falloff_map_intensity);
			printf("\n");
		}
	hero_ui_box_end(window);

	scene->render_data.deep_sea_max = config.deep_sea_max;
	scene->render_data.sea_max = config.sea_max;
	scene->render_data.ground_max = config.ground_max;
	scene->render_data.has_been_updated = false;

	static int first_frame_init = 0;
	if (
		first_frame_init == 0 ||
		config.seed != scene->config.seed ||
		config.frequency != scene->config.frequency ||
		config.fractal_type != scene->config.fractal_type ||
		config.octaves != scene->config.octaves ||
		config.lacunarity != scene->config.lacunarity ||
		config.falloff_map_intensity != scene->config.falloff_map_intensity
	) {
		first_frame_init += 1;
		game_island_gen(&config, scene->render_data.tile_height_map);
		scene->render_data.has_been_updated = true;
	}

	scene->config = config;

	return HERO_SUCCESS;
}

HeroResult game_scene_island_gen_debug_update_render_data(GameScene* s) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_ISLAND_GEN_DEBUG, "expected a island gen debug scene");
	GameSceneIslandGenDebug* scene = (GameSceneIslandGenDebug*)s;
	GameSceneIslandGenDebugRenderData* dst = &game.gfx.island_gen_debug.scene_render_data;

	if (dst->tile_height_map == NULL) {
		F32* tile_height_map = hero_alloc_array(F32, hero_system_alctor, 0, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
		if (tile_height_map == NULL) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		dst->tile_height_map = tile_height_map;
	}

	if (scene->render_data.has_been_updated) {
		// copy the generated map into the new buffer we swap with every frame.
		HERO_COPY_ELMT_MANY(dst->tile_height_map, scene->render_data.tile_height_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	}

	F32* tile_height_map = dst->tile_height_map;
	dst->tile_height_map = scene->render_data.tile_height_map;
	scene->render_data.tile_height_map = tile_height_map;

	dst->deep_sea_max = scene->render_data.deep_sea_max;
	dst->sea_max = scene->render_data.sea_max;
	dst->ground_max = scene->render_data.ground_max;
	dst->deep_sea_max = scene->render_data.deep_sea_max;
	dst->show_grid = scene->render_data.show_grid;
	dst->show_height_map = scene->render_data.show_height_map;
	dst->has_been_updated = scene->render_data.has_been_updated;

	return HERO_SUCCESS;
}

HeroResult game_scene_island_gen_debug_init(GameScene** scene_out) {
	GameSceneIslandGenDebug* scene = hero_alloc_elmt(GameSceneIslandGenDebug, hero_system_alctor, 0);
	if (scene == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	scene->scene.type = GAME_SCENE_TYPE_ISLAND_GEN_DEBUG;
	scene->scene.state = GAME_SCENE_STATE_UNLOADED;
	scene->scene.load_fn = game_scene_island_gen_debug_load;
	scene->scene.unload_fn = game_scene_island_gen_debug_unload;
	scene->scene.update_fn = game_scene_island_gen_debug_update;
	scene->scene.update_render_data_fn = game_scene_island_gen_debug_update_render_data;

	*scene_out = (GameScene*)scene;
	return HERO_SUCCESS;
}

// ===========================================
//
//
// Scene: Play
//
//
// ===========================================

Vec3 game_direction_vectors[GAME_DIRECTION_COUNT] = {
	[GAME_DIRECTION_NONE] = {0},
	[GAME_DIRECTION_UP] = VEC3_FORWARD,
	[GAME_DIRECTION_DOWN] = VEC3_BACKWARD,
	[GAME_DIRECTION_LEFT] = VEC3_LEFT,
	[GAME_DIRECTION_RIGHT] = VEC3_RIGHT,
};

HeroResult _game_scene_play_init_voxel_height_map(GameScenePlay* scene) {
	F32* voxel_height_map = hero_alloc_array(F32, hero_system_alctor, 0, GAME_ISLAND_AXIS_VOXELS_COUNT * GAME_ISLAND_AXIS_VOXELS_COUNT);
	if (voxel_height_map == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	U32 row_start = 0;
	U32 next_row_start = GAME_ISLAND_AXIS_TILES_COUNT;

	F32 tl = scene->tile_height_map[row_start];
	F32 tr = scene->tile_height_map[row_start + 1];
	F32 bl = scene->tile_height_map[next_row_start];
	F32 br = scene->tile_height_map[next_row_start + 1];
	U32 voxel_start = 0;
	for_range(y, 0, GAME_ISLAND_AXIS_TILES_COUNT) {
		for_range(x, 0, GAME_ISLAND_AXIS_TILES_COUNT) {

			F32 tvy = 0.f;
			F32 tstep = 1.f / (F32)GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;
			U32 voxel_idx = voxel_start;
			for_range(vy, 0, GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) {
				F32 tvx = 0.f;
				for_range(vx, 0, GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) {
					F32 voxel_height = hero_bilerp(tl, tr, bl, br, tvx, tvy);
					voxel_height_map[voxel_idx] = voxel_height;
					tvx += tstep;
					voxel_idx += 1;
				}

				tvy += tstep;

				// increment to next row
				voxel_idx += GAME_ISLAND_AXIS_VOXELS_COUNT - GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;
			}

			// increment to next column
			voxel_start += GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;

			if (x + 2 < GAME_ISLAND_AXIS_TILES_COUNT) {
				tl = tr;
				bl = br;
				tr = scene->tile_height_map[row_start + x + 1];
				br = scene->tile_height_map[next_row_start + x + 1];
			}
		}

		if (y + 2 < GAME_ISLAND_AXIS_TILES_COUNT) {
			row_start += GAME_ISLAND_AXIS_TILES_COUNT;
			tl = scene->tile_height_map[row_start];
			tr = scene->tile_height_map[row_start + 1];

			next_row_start += GAME_ISLAND_AXIS_TILES_COUNT;
			bl = scene->tile_height_map[next_row_start];
			br = scene->tile_height_map[next_row_start + 1];

			// go to the next row
			voxel_start = ((y + 1) * GAME_ISLAND_AXIS_VOXELS_COUNT) * GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;
		}
	}

	scene->voxel_height_map = voxel_height_map;

	return HERO_SUCCESS;
}

HeroResult game_scene_play_load(GameScene* s) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_PLAY, "expected a play scene");
	GameScenePlay* scene = (GameScenePlay*)s;
	HeroResult result;

	scene->camera.forward = VEC3_FORWARD;
	scene->camera.position = VEC3_INIT(0.f, 118.f, -8.f);
	scene->camera.rotation = QUAT_IDENTITY;
	scene->player_move_aim_direction = VEC2_UP;

	F32* tile_height_map = hero_alloc_array(F32, hero_system_alctor, 0, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	if (tile_height_map == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	scene->tile_height_map = tile_height_map;

	GameIslandGenConfig config = game_island_gen_configs[0];
	game_island_gen(&config, scene->tile_height_map);

	result = _game_scene_play_init_voxel_height_map(scene);
	HERO_RESULT_ASSERT(result);

	U32* terrain_tile_tint_map = hero_alloc_array(U32, hero_system_alctor, 0, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	if (terrain_tile_tint_map == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	scene->terrain_tile_tint_map = terrain_tile_tint_map;
	HERO_ZERO_ELMT_MANY(terrain_tile_tint_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);

	{
		HeroUIImageAtlasSetup setup = {
			.image_id = game.gfx.play.attachment_image_id,
			.sampler_id = game.gfx.clamp_nearest_sampler_id,
			.images_count = 1,
			.is_uniform = true,
			.data.uniform.cell_width = 1920,
			.data.uniform.cell_height = 1080,
			.data.uniform.cells_count_x = 1,
		};

		result = hero_ui_image_atlas_init(game.gfx.ldev, &setup, &scene->ui_image_atlas_id);
		HERO_RESULT_ASSERT(result);
	}

	hero_object_pool(GameEntity, init)(&scene->entity_pool, GAME_ENTITIES_CAP, hero_system_alctor, 0);

	U32 cell_x = 7;
	U32 cell_y = 15;
	{
		GameEntity* player;
		result = game_scene_play_entity_add(scene, GAME_ENTITY_TYPE_HUMAN, &player, &scene->player_entity_id);
		HERO_RESULT_ASSERT(result);
		player->flags |= GAME_ENTITY_FLAGS_PLAYER;

		player->position = VEC3_INIT(cell_x * GAME_ISLAND_CELL_AXIS_TILES_COUNT, 0.f, cell_y * GAME_ISLAND_CELL_AXIS_TILES_COUNT);
		strncpy(player->name, "player", sizeof(player->name));
		game_scene_play_camera_set_target_entity(scene, scene->player_entity_id);
	}

	{
		GameEntity* enemy;
		result = game_scene_play_entity_add(scene, GAME_ENTITY_TYPE_HUMAN, &enemy, &scene->enemy_entity_id);
		HERO_RESULT_ASSERT(result);
		enemy->position = VEC3_INIT(cell_x * GAME_ISLAND_CELL_AXIS_TILES_COUNT + 3.f, 0.f, cell_y * GAME_ISLAND_CELL_AXIS_TILES_COUNT + 0.f);

		result = game_scene_play_entity_add(scene, GAME_ENTITY_TYPE_HUMAN, &enemy, NULL);
		HERO_RESULT_ASSERT(result);
		enemy->position = VEC3_INIT(cell_x * GAME_ISLAND_CELL_AXIS_TILES_COUNT + 2.f, 0.f, cell_y * GAME_ISLAND_CELL_AXIS_TILES_COUNT + 0.f);

		result = game_scene_play_entity_add(scene, GAME_ENTITY_TYPE_HUMAN, &enemy, NULL);
		HERO_RESULT_ASSERT(result);
		enemy->position = VEC3_INIT(cell_x * GAME_ISLAND_CELL_AXIS_TILES_COUNT + 2.f, 0.f, cell_y * GAME_ISLAND_CELL_AXIS_TILES_COUNT + 1.f);
	}

	return HERO_SUCCESS;
}

HeroResult game_scene_play_unload(GameScene* s) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_PLAY, "expected a play scene");
	GameScenePlay* scene = (GameScenePlay*)s;

	return HERO_SUCCESS;
}

HeroResult game_scene_play_update(GameScene* s, GameScene** replace_with_scene_out) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_PLAY, "expected a play scene");
	GameScenePlay* scene = (GameScenePlay*)s;
	HeroResult result;

	scene->render_data.entity_billboards.count = 0;
	scene->render_data.terrain_tile_tint_map = NULL;

	HeroUIWindow* window;
	result = hero_ui_window_get(game.ui_window_id, &window);
	if (result < 0) {
		return result;
	}

	game_scene_play_camera_update(scene);

	hero_ui_image(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_IMAGE_ID(scene->ui_image_atlas_id, 0), -1, 1.f, HERO_UI_IMAGE_SCALE_MODE_NONE, HERO_UI_IMAGE_FLIP_NONE, NULL);

	hero_ui_widget_next_layer_start(window, __LINE__);

	//hero_ui_box(window, __LINE__, HERO_UI_CUT_LEFT, 100.f, hero_ui_ss.box);

	{
		GameInputState input_state = {0};
		for_range(input, 0, GAME_INPUT_COUNT) {
			HeroScanCode scan_code = game_input_default_keyboard_map[input];
			if (hero_keyboard_scan_code_is_pressed(scan_code)) {
				input_state.is_pressed |= ENUM_FLAG(input);
			}

			if (hero_keyboard_scan_code_has_been_pressed(scan_code)) {
				input_state.has_been_pressed |= ENUM_FLAG(input);
			}

			if (hero_keyboard_scan_code_has_been_released(scan_code)) {
				input_state.has_been_released |= ENUM_FLAG(input);
			}
		}

		result = game_scene_play_player_handle_input(scene, input_state);
		HERO_RESULT_ASSERT(result);
	}
#if 0
	if (hero_window_sys.mouse.rel_x || hero_window_sys.mouse.rel_y) {
		scene->camera_yaw += 0.01 * (F32)hero_window_sys.mouse.rel_x;
		scene->camera_pitch += 0.01 * (F32)hero_window_sys.mouse.rel_y;
		Quat x_rotation = quat_rotate(scene->camera_yaw, VEC3_INIT(0.f, 1.f, 0.f));
		Quat y_rotation = quat_rotate(scene->camera_pitch, VEC3_INIT(1.f, 0.f, 0.f));
		scene->camera_rotation = quat_norm(quat_mul(x_rotation, y_rotation));
		scene->camera_forward = quat_mul_vec3(scene->camera_rotation, VEC3_FORWARD);
	}
		printf("camera_forward = { %f, %f, %f }\n", scene->camera_forward.x, scene->camera_forward.y, scene->camera_forward.z);

	if (hero_keyboard_scan_code_is_pressed(HERO_KEY_CODE_W)) {
		Vec3 offset = vec3_mul_scalar(scene->camera_forward, 0.1);
		scene->camera_position = vec3_add(scene->camera_position, offset);
	}

	if (hero_keyboard_scan_code_is_pressed(HERO_KEY_CODE_A)) {
		Vec3 dir_vec = vec3_perp_z_left(scene->camera_forward);
		dir_vec.y = 0.f;
		Vec3 offset = vec3_mul_scalar(dir_vec, 0.1);
		scene->camera_position = vec3_add(scene->camera_position, offset);
	}

	if (hero_keyboard_scan_code_is_pressed(HERO_KEY_CODE_S)) {
		Vec3 offset = vec3_mul_scalar(scene->camera_forward, 0.1);
		scene->camera_position = vec3_sub(scene->camera_position, offset);
	}

	if (hero_keyboard_scan_code_is_pressed(HERO_KEY_CODE_D)) {
		Vec3 dir_vec = vec3_perp_z_right(scene->camera_forward);
		dir_vec.y = 0.f;
		Vec3 offset = vec3_mul_scalar(dir_vec, 0.1);
		scene->camera_position = vec3_add(scene->camera_position, offset);
	}

	if (hero_keyboard_scan_code_is_pressed(HERO_KEY_CODE_SPACE)) {
		scene->camera_position.y += 0.1;
	}
#endif

	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_FILL, NULL);
	{
		if (scene->npc_move_time != 0.f) {
			scene->npc_move_time += game.dt * GAME_PLAYER_MOVE_SPEED;
		}

		{
			//
			// update the player first so all of the npc entities can start moving in the same frame
			GameEntity* player;
			HeroResult result = game_scene_play_entity_get(scene, scene->player_entity_id, &player);
			if (result < 0) {
				return result;
			}
			result = game_scene_play_entity_update(scene, scene->player_entity_id, player);
			if (result < 0) {
				return result;
			}
		}

#if 0
						GameInputState state = {0};
						state.is_pressed |= ENUM_FLAG(GAME_INPUT_DOWN);

						result = game_scene_play_entity_handle_input(scene, scene->enemy_entity_id, state);
						HERO_RESULT_ASSERT(result);
#endif

		GameEntity* entity = NULL;
		GameEntityId entity_id = {0};
		while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
			if (entity_id.raw != scene->player_entity_id.raw) {
				result = game_scene_play_entity_update(scene, entity_id, entity);
				if (result < 0) {
					return result;
				}
			}
		}

		{
			GameEntity* player;
			HeroResult result = game_scene_play_entity_get(scene, scene->player_entity_id, &player);
			if (result < 0) {
				return result;
			}

			// TODO: we have this next layer so it fixes being centered
			// atm there is only health bars that have a custom_offset so this dialog should be able to be a sibling of those bars right?
			hero_ui_widget_next_layer_start(window, __LINE__);

			hero_ui_box_start(window, __LINE__, HERO_UI_CUT_CENTER_HORIZONTAL, HERO_UI_LEN_AUTO, NULL);

			switch (scene->player_action_state) {
				case GAME_PLAYER_ACTION_STATE_MOVE_DIALOG: {
					hero_ui_text(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("Select a move"), game_ui_ss.dialog_text);
					hero_ui_box_start(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_ui_ss.box);

					for_range(move_idx, 0, 4) {
						HeroUIWidgetState widget_state = HERO_UI_WIDGET_STATE_DEFAULT;
						if (move_idx == scene->player_selected_move_idx) {
							widget_state = HERO_UI_WIDGET_STATE_FOCUSED;
						}
						hero_ui_widget_next_forced_state(window, widget_state);

						hero_ui_text(window, move_idx, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, game_moves[move_idx].name, game_ui_ss.move_text);
					}
					hero_ui_box_end(window);
					break;
				};
				case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY: {
					hero_ui_text(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("Target an entity"), game_ui_ss.dialog_text);
					break;
				};
				case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE: {
					hero_ui_text(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_lit("Target a tile"), game_ui_ss.dialog_text);
					break;
				};
			}

			hero_ui_box_end(window);
			hero_ui_widget_next_layer_end(window);
		}

		if (scene->npc_move_time > 1.f) {
			scene->npc_move_time = 0.f;
		}
	}
	hero_ui_box_end(window);

	hero_ui_widget_next_layer_end(window);

	hero_ui_widget_next_layer_start(window, __LINE__);

	scene->glow_intensity += game.dt * 0.8f;
	if (scene->glow_intensity > 1.f) scene->glow_intensity = 0.1f;

	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, NULL);
	hero_ui_box_start(window, __LINE__, HERO_UI_CUT_RIGHT, HERO_UI_LEN_AUTO, hero_ui_ss.box);
	switch (scene->player_action_state) {
		case GAME_PLAYER_ACTION_STATE_MOVING: {
			hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("X: Moves"), game_ui_ss.controls_hint_text);
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_DIALOG: {
			if (scene->player_target_entity_id.raw == 0) {
				hero_ui_widget_next_forced_state(window, HERO_UI_WIDGET_STATE_DISABLED);
			}
			hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("A: Accept"), game_ui_ss.controls_hint_text);
			hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("B: Close"), game_ui_ss.controls_hint_text);
			if (game_moves[scene->player_selected_move_idx].target_mode != GAME_MOVE_TARGET_MODE_DIRECTION) {
				hero_ui_widget_next_forced_state(window, HERO_UI_WIDGET_STATE_DISABLED);
			}
			hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("L/R: Aim"), game_ui_ss.controls_hint_text);
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY:
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE: {
			hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("A: Confirm"), game_ui_ss.controls_hint_text);
			hero_ui_text(window, __LINE__, HERO_UI_CUT_LEFT, HERO_UI_LEN_AUTO, hero_string_lit("B: Back"), game_ui_ss.controls_hint_text);
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE: {
			scene->player_move_animate_time += game.dt;
			if (scene->player_move_animate_time >= GAME_PLAYER_MOVE_ANIMATE_SECS) {
				scene->player_action_state = GAME_PLAYER_ACTION_STATE_MOVING;
				scene->flags &= ~GAME_SCENE_PLAY_GLOW;
				game_scene_play_camera_set_target_entity(scene, scene->player_entity_id);

				GameEntity* entity = NULL;
				GameEntityId entity_id = {0};
				while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
					entity->animate_og_health = entity->health;
				}

				HERO_ZERO_ELMT_MANY(scene->terrain_tile_tint_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
				scene->render_data.terrain_tile_tint_map = scene->terrain_tile_tint_map;
			}
			break;
		};
	}
	hero_ui_box_end(window);
	hero_ui_box_end(window);

	hero_ui_widget_next_layer_end(window);

	return HERO_SUCCESS;
}

HeroResult game_scene_play_update_render_data(GameScene* s) {
	HERO_ASSERT(s->type == GAME_SCENE_TYPE_PLAY, "expected a play scene");
	GameScenePlay* scene = (GameScenePlay*)s;
	GameScenePlayRenderData* dst = &game.gfx.play.scene_render_data;

	dst->tile_height_map = scene->tile_height_map;
	dst->voxel_height_map = NULL;
	dst->camera_forward = scene->camera.forward;
	dst->camera_position = scene->camera.position;
	dst->camera_rotation = scene->camera.rotation;
	dst->terrain_tile_tint_map = scene->render_data.terrain_tile_tint_map;

	static bool init = false;
	if (!init) {
		dst->voxel_height_map = scene->voxel_height_map;
		dst->terrain_tile_tint_map = scene->terrain_tile_tint_map;
		init = true;
	}

	dst->entity_billboards.count = 0;
	GameRenderEntityBillboard* dst_renders;
	HeroResult result = hero_stack(GameRenderEntityBillboard, push_many)(&dst->entity_billboards, scene->render_data.entity_billboards.count, hero_system_alctor, 0, &dst_renders);
	if (result < 0) {
		return result;
	}
	HERO_COPY_ELMT_MANY(dst->entity_billboards.data, scene->render_data.entity_billboards.data, scene->render_data.entity_billboards.count);

	return HERO_SUCCESS;
}

HeroResult game_scene_play_init(GameScene** scene_out) {
	GameScenePlay* scene = hero_alloc_elmt(GameScenePlay, hero_system_alctor, 0);
	if (scene == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}
	HERO_ZERO_ELMT(scene);
	scene->scene.type = GAME_SCENE_TYPE_PLAY;
	scene->scene.state = GAME_SCENE_STATE_UNLOADED;
	scene->scene.load_fn = game_scene_play_load;
	scene->scene.unload_fn = game_scene_play_unload;
	scene->scene.update_fn = game_scene_play_update;
	scene->scene.update_render_data_fn = game_scene_play_update_render_data;

	*scene_out = (GameScene*)scene;
	return HERO_SUCCESS;
}

HeroResult game_scene_play_entity_add(GameScenePlay* scene, GameEntityType type, GameEntity** entity_out, GameEntityId* id_out) {
	HeroResult result;
	GameEntity* entity;
	GameEntityId id;
	result = hero_object_pool(GameEntity, alloc)(&scene->entity_pool, 0, &entity, &id);
	if (result < 0) {
		return result;
	}
	if (entity_out) *entity_out = entity;
	if (id_out) *id_out = id;
	entity->type = type;
	entity->health = 100;
	entity->animate_og_health = entity->health;

	switch (type) {
		case GAME_ENTITY_TYPE_HUMAN: {
			entity->flags |=
				GAME_ENTITY_FLAGS_AUTO_Y_FROM_HEIGHT_MAP |
				GAME_ENTITY_FLAGS_BILLBOARD              ;
			entity->steps_per_move = 2;
			strncpy(entity->name, "human", sizeof(entity->name));
			break;
		};
		default: HERO_ABORT("unhandled entity type %u\n", type);
	}

	return HERO_SUCCESS;
}

HeroResult game_scene_play_entity_get(GameScenePlay* scene, GameEntityId id, GameEntity** out) {
	return hero_object_pool(GameEntity, get)(&scene->entity_pool, id, out);
}

F32 game_scene_play_tile_max_voxel_height(GameScenePlay* scene, U32 tile_x, U32 tile_y) {
	F32 height = 0.f;
	U32 voxel_start_x = tile_x * GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;
	U32 voxel_start_y = tile_y * GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;

	for_range(voxel_y, voxel_start_y, voxel_start_y + GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) {
		for_range(voxel_x, voxel_start_x, voxel_start_x + GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) {
			F32 voxel_height = scene->voxel_height_map[voxel_y * GAME_ISLAND_AXIS_VOXELS_COUNT + voxel_x];
			height = HERO_MAX(height, voxel_height);
		}
	}

	return height;
}

HeroResult game_scene_play_entity_update(GameScenePlay* scene, GameEntityId id, GameEntity* entity) {
	HeroResult result;
	switch (entity->type) {
		case GAME_ENTITY_TYPE_HUMAN: {
			break;
		};
		default: HERO_ABORT("unhandled entity type %u\n", entity->type);
	}

	//
	// tell the npc to begin stop moving so it will stop at the next tile.
	/*
	if (scene->npc_move_time >= 1.f) {
		GameInputState state = {0};
		result = game_scene_play_entity_handle_input(scene, id, state);
		HERO_RESULT_ASSERT(result);
	}
	*/

	//
	// move the entity
	F32 move_speed = (entity->flags & GAME_ENTITY_FLAGS_PLAYER) ? GAME_PLAYER_MOVE_SPEED : (F32)entity->steps_per_move * GAME_PLAYER_MOVE_SPEED;
	Vec3 move_vec = vec3_mul_scalar(game_direction_vectors[entity->direction], move_speed * game.dt);
	Vec2 prev_xy = VEC2_INIT(entity->position.x, entity->position.z);
	entity->position = vec3_add(entity->position, move_vec);

	//
	// stop the entity if we are stopping and we have passed the next tile.
	if (entity->flags & GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE) {
		F32 diff = 1.f;
		switch (entity->direction) {
			case GAME_DIRECTION_UP:
			case GAME_DIRECTION_DOWN:
				diff = fabsf(entity->stop_on_next_tile_start - entity->position.z);
				break;
			case GAME_DIRECTION_LEFT:
			case GAME_DIRECTION_RIGHT:
				diff = fabsf(entity->stop_on_next_tile_start - entity->position.x);
				break;
		}
		if (diff >= 1.f) {
			Vec3 align_back = vec3_mul_scalar(game_direction_vectors[entity->direction], diff - 1.f);
			entity->position = vec3_sub(entity->position, align_back);

			entity->direction = GAME_DIRECTION_NONE;
			entity->flags &= ~GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE;
		}
	}

	//
	// automatically set he y axis position to the voxel height map and interpolate between tiles when moving
	if (entity->flags & GAME_ENTITY_FLAGS_AUTO_Y_FROM_HEIGHT_MAP) {
		U32 tile_x = (U32)floorf(entity->position.x);
		U32 tile_y = (U32)floorf(entity->position.z);

		F32 height = game_scene_play_tile_max_voxel_height(scene, tile_x, tile_y);

		F32 next_height = 0.f;
		F32 interp_ratio = 0.f;
		if (entity->direction != GAME_DIRECTION_NONE) {
			switch (entity->direction) {
				case GAME_DIRECTION_UP:
				case GAME_DIRECTION_DOWN:
					interp_ratio = fabsf(entity->position.z - floorf(entity->position.z));
					tile_y += 1;
					break;
				case GAME_DIRECTION_LEFT:
				case GAME_DIRECTION_RIGHT:
					interp_ratio = fabsf(entity->position.x - floorf(entity->position.x));
					tile_x += 1;
					break;
			}

			next_height = game_scene_play_tile_max_voxel_height(scene, tile_x, tile_y);
		}

		switch (entity->direction) {
			case GAME_DIRECTION_NONE:
				entity->position.y = height;
				break;
			case GAME_DIRECTION_UP:
			case GAME_DIRECTION_RIGHT:
				entity->position.y = hero_lerp(height, next_height, interp_ratio);
				break;
			case GAME_DIRECTION_DOWN:
			case GAME_DIRECTION_LEFT:
				entity->position.y = hero_lerp(height, next_height, interp_ratio);
				break;
		}
	}

	//
	// rendering
	{
		if (entity->flags & GAME_ENTITY_FLAGS_BILLBOARD) {
			GameRenderEntityBillboard* render;
			result = hero_stack(GameRenderEntityBillboard, push)(&scene->render_data.entity_billboards, hero_system_alctor, 0, &render);
			if (result < 0) {
				return result;
			}

			render->type = entity->type;
			render->position = entity->position;
			render->ascii = game_scene_play_entity_ascii(entity);
			render->color = hero_color_init(0xff, 0x66, 0xcc, 0xff);
			if (!(scene->flags & GAME_SCENE_PLAY_GLOW) || entity->flags & GAME_ENTITY_FLAGS_PLAYER) {
				render->tint = 0;
			} else {
				F32 glow_intensity = scene->glow_intensity;
				if ((scene->player_action_state == GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY || scene->player_action_state == GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE) && entity->flags & GAME_ENTITY_FLAGS_VALID_SPLASH) {
					render->tint = GAME_MOVE_RADIUS_COLOR_TARGET_SPLASH;
					glow_intensity = 1.f - glow_intensity;
				} else if (scene->player_action_state == GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY && id.raw == scene->player_target_entity_id.raw) {
					render->tint = GAME_MOVE_RADIUS_COLOR_TARGET;
					glow_intensity = 1.f - glow_intensity;
				} else if (entity->flags & GAME_ENTITY_FLAGS_INVALID_RANGE) {
					render->tint = GAME_MOVE_RADIUS_COLOR_INVALID;
				} else if (entity->flags & GAME_ENTITY_FLAGS_VALID_SPLASH) {
					render->tint = GAME_MOVE_RADIUS_COLOR_VALID_SPLASH;
				} else {
					render->tint = GAME_MOVE_RADIUS_COLOR_VALID;
				}
				render->tint = hero_color_chan_set(render->tint, HERO_COLOR_CHAN_A, (U8)(glow_intensity * 255.f));
			}
		}
	}

	//
	// player specific logic
	if (entity->flags & GAME_ENTITY_FLAGS_PLAYER) {
		if (floorf(prev_xy.x) != floorf(entity->position.x) || floorf(prev_xy.y) != floorf(entity->position.z)) {
			entity->step_idx += 1;
			if (entity->step_idx == entity->steps_per_move) {
				entity->step_idx = 0;
				scene->npc_move_time = game.dt * GAME_PLAYER_MOVE_SPEED;
			}
		}

		static int init = 0;
		if (!init) {
			scene->camera.position = entity->position;
			init = 1;
		}
	}

	{
		HeroUIWindow* window;
		result = hero_ui_window_get(game.ui_window_id, &window);
		if (result < 0) {
			return result;
		}
		hero_ui_box_start(window, id.raw, HERO_UI_CUT_LEFT, HERO_UI_LEN_FILL, NULL);

		Vec3 pos = entity->position;
		pos.x += 0.5f;
		Vec2 screen_pos = game_scene_play_world_to_screen_space_point(scene, pos);

		F32 width = 150.f;
		hero_ui_widget_custom_offset(window, screen_pos, width, HERO_UI_ALIGN_X_CENTER | HERO_UI_ALIGN_Y_TOP);
		hero_ui_box_start(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, NULL);

			hero_ui_text(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_c(entity->name), game_ui_ss.health_bar_text);

			U32 health = entity->health;
			if (scene->player_action_state == GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE && entity->health != entity->animate_og_health) {
				F32 interp_ratio = 1.f - (scene->player_move_animate_time / GAME_PLAYER_MOVE_ANIMATE_SECS);
				health = (U32)hero_lerp((F32)health, (F32)entity->animate_og_health, interp_ratio);
			}

			hero_ui_box_start(window, __LINE__, HERO_UI_CUT_TOP, width / 5.f, game_ui_ss.health_bar);
				F32 inner_width = width - game_ui_ss.health_bar[0].padding.left - game_ui_ss.health_bar[0].padding.right - (game_ui_ss.health_bar[0].border_width * 2.f);
				hero_ui_box(window, __LINE__, HERO_UI_CUT_LEFT, ((F32)health / 100.f) * inner_width, game_ui_ss.health_bar_inner);
			hero_ui_box_end(window);

			snprintf(entity->health_string, sizeof(entity->health_string), "%u/%u", health, 100);
			hero_ui_text(window, __LINE__, HERO_UI_CUT_TOP, HERO_UI_LEN_AUTO, hero_string_c(entity->health_string), game_ui_ss.health_bar_text);
		hero_ui_box_end(window);

		hero_ui_box_end(window);
	}

	return HERO_SUCCESS;
}

void game_scene_play_splash_tile_origin(GameScenePlay* scene, U32* tile_x_out, U32* tile_y_out) {
	U32 splash_tile_x;
	U32 splash_tile_y;
	switch (scene->player_action_state) {
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY: {
			GameEntity* target;
			HeroResult result = game_scene_play_entity_get(scene, scene->player_target_entity_id, &target);
			HERO_RESULT_ASSERT(result);
			printf("scene->player_target_entity_id.raw = %u\n", scene->player_target_entity_id.raw);

			splash_tile_x = (U32)floorf(target->position.x);
			splash_tile_y = (U32)floorf(target->position.z);
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE: {
			splash_tile_x = scene->player_target_tile_x;
			splash_tile_y = scene->player_target_tile_y;
			break;
		};
		default: {
			GameEntity* player;
			HeroResult result = game_scene_play_entity_get(scene, scene->player_entity_id, &player);
			HERO_RESULT_ASSERT(result);

			splash_tile_x = (U32)floorf(player->position.x);
			splash_tile_y = (U32)floorf(player->position.z);
			break;
		};
	}

	*tile_x_out = splash_tile_x;
	*tile_y_out = splash_tile_y;
}

void game_scene_play_combat_enemy_entity_filter(GameScenePlay* scene, GameEntityFilterArgs* args_out) {
	HeroResult result;
	GameMove* move = &game_moves[scene->player_selected_move_idx];

	GameEntity* player;
	result = game_scene_play_entity_get(scene, scene->player_entity_id, &player);
	HERO_RESULT_ASSERT(result);

	U32 splash_origin_tile_x;
	U32 splash_origin_tile_y;
	game_scene_play_splash_tile_origin(scene, &splash_origin_tile_x, &splash_origin_tile_y);

	F32 spread_angle = move->target_mode == GAME_MOVE_TARGET_MODE_DIRECTION ? move->spread_angle : 0.f;

	args_out->origin_tile_x = (U32)floorf(player->position.x);
	args_out->origin_tile_y = (U32)floorf(player->position.z);
	args_out->radius = move->range_radius;
	args_out->spread_angle = spread_angle;
	args_out->aim_direction = scene->player_move_aim_direction;
	args_out->splash_origin_tile_x = splash_origin_tile_x;
	args_out->splash_origin_tile_y = splash_origin_tile_y;
	args_out->splash_radius = move->splash_radius;
	args_out->ignore_entity_id = scene->player_entity_id;
}

void game_scene_play_combat_update_tints(GameScenePlay* scene, GameEntityFilterArgs* filter_args) {
	HeroResult result;
	//
	// get the entity filter if the user does not provide one
	GameEntityFilterArgs _args;
	if (filter_args == NULL) {
		game_scene_play_combat_enemy_entity_filter(scene, &_args);
		filter_args = &_args;
	}

	//
	// update the entity tints
	{
		game_scene_play_entity_filter_start(scene, filter_args);

		GameEntity* entity = NULL;
		GameEntityId entity_id = {0};
		while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
			game_scene_play_entity_filter_check(scene, entity_id, entity, filter_args);
			entity->flags &= ~(GAME_ENTITY_FLAGS_INVALID_RANGE | GAME_ENTITY_FLAGS_VALID_SPLASH);

			if (!filter_args->_valid) {
				entity->flags |= GAME_ENTITY_FLAGS_INVALID_RANGE;
			} else if (filter_args->_splash) {
				entity->flags |= GAME_ENTITY_FLAGS_VALID_SPLASH;
			}
		}
		HERO_RESULT_ASSERT(result);
	}

	//
	// now update the tile map tints
	//

	GameMove* move = &game_moves[scene->player_selected_move_idx];

	U32 color;
	if (scene->player_target_entity_id.raw) {
		color = GAME_MOVE_RADIUS_COLOR_VALID;
	} else {
		color = GAME_MOVE_RADIUS_COLOR_INVALID;
	}
	color = hero_color_chan_set(color, HERO_COLOR_CHAN_A, (U8)(scene->glow_intensity * 255.f));

	HERO_ZERO_ELMT_MANY(scene->terrain_tile_tint_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	switch (move->target_mode) {
		case GAME_MOVE_TARGET_MODE_DIRECTION: {
			Vec2 cone_dir = scene->player_move_aim_direction;
			F32 half_angle_cos = cosf(move->spread_angle / 2.f);
			F32 squared_half_angle_cos = half_angle_cos * fabsf(half_angle_cos);

			for_range_s(y, -(move->range_radius + 1), move->range_radius + 1) {
				for_range_s(x, -(move->range_radius + 1), move->range_radius + 1) {
					Vec2 offset = VEC2_INIT((F32)x, (F32)y);
					F32 squared_distance = vec2_dot(offset, offset);
					if (squared_distance > filter_args->_squared_radius) {
						continue;
					}

					F32 tile_angle_cos = vec2_dot(offset, cone_dir);
					F32 squared_tile_angle_cos = tile_angle_cos * fabsf(tile_angle_cos);
					F32 result = squared_half_angle_cos * squared_distance;
					if (result > squared_tile_angle_cos) {
						continue;
					}

					scene->terrain_tile_tint_map[(filter_args->origin_tile_y + y) * GAME_ISLAND_AXIS_TILES_COUNT + (filter_args->origin_tile_x + x)] = color;
				}
			}
			break;
		};
		default: {
			S32 radius = move->range_radius + move->splash_radius + 1;
			for_range_s(y, -radius, radius) {
				for_range_s(x, -radius, radius) {
					Vec2 offset = VEC2_INIT((F32)x, (F32)y);
					F32 squared_distance = vec2_dot(offset, offset);

					bool is_in_spash_radius = false;
					HeroColor tile_color = color;
					if (filter_args->_squared_splash_radius > 0.f) {
						Vec2 tile_coord = vec2_add(VEC2_INIT((F32)filter_args->origin_tile_x, (F32)filter_args->origin_tile_y), VEC2_INIT((F32)x, (F32)y));
						Vec2 splash_offset = vec2_sub(tile_coord, VEC2_INIT((F32)filter_args->splash_origin_tile_x, (F32)filter_args->splash_origin_tile_y));
						F32 squared_splash_distance = vec2_dot(splash_offset, splash_offset);
						if (squared_splash_distance <= filter_args->_squared_splash_radius) {
							tile_color = GAME_MOVE_RADIUS_COLOR_VALID_SPLASH;
							tile_color = hero_color_chan_set(tile_color, HERO_COLOR_CHAN_A, (U8)(scene->glow_intensity * 255.f));

							is_in_spash_radius = true;
						}
					}

					if (!is_in_spash_radius && squared_distance > filter_args->_squared_radius) {
						continue;
					}

					scene->terrain_tile_tint_map[(filter_args->origin_tile_y + y) * GAME_ISLAND_AXIS_TILES_COUNT + (filter_args->origin_tile_x + x)] = tile_color;
				}
			}
			break;
		};
	}

	scene->render_data.terrain_tile_tint_map = scene->terrain_tile_tint_map;
}

void game_scene_play_combat_select_move(GameScenePlay* scene, U32 move_idx) {
	scene->player_selected_move_idx = move_idx;

	GameEntityFilterArgs filter_args;
	game_scene_play_combat_enemy_entity_filter(scene, &filter_args);

	GameEntityId entity_id = {0};
	HeroResult result = game_scene_play_entity_find_closest_in_range(scene, &filter_args, &entity_id);
	HERO_RESULT_ASSERT(result);
	scene->player_target_entity_id = entity_id;

	game_scene_play_combat_update_tints(scene, &filter_args);
}

void game_scene_play_entity_perform_turn(GameScenePlay* scene, GameEntityId id, GameEntityTurn* turn) {
	GameEntity* entity;
	HeroResult result = game_scene_play_entity_get(scene, id, &entity);
	HERO_RESULT_ASSERT(result);

	switch (turn->type) {
	case GAME_ENTITY_TURN_TYPE_NONE:
		break;
	case GAME_ENTITY_TURN_TYPE_MOVE_TILE: {
		GameDirection direction = turn->data.move_tile.direction;

		if (direction != entity->direction) {
			switch (entity->direction) {
			case GAME_DIRECTION_UP:
				entity->stop_on_next_tile_start = floorf(entity->position.z);
				entity->flags |= GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE;
				break;
			case GAME_DIRECTION_DOWN:
				entity->stop_on_next_tile_start = ceilf(entity->position.z);
				entity->flags |= GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE;
				break;
			case GAME_DIRECTION_LEFT:
				entity->stop_on_next_tile_start = ceilf(entity->position.x);
				entity->flags |= GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE;
				break;
			case GAME_DIRECTION_RIGHT:
				entity->stop_on_next_tile_start = floorf(entity->position.x);
				entity->flags |= GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE;
				break;
			}
		}

		if (entity->direction == GAME_DIRECTION_NONE) {
			entity->direction = direction;
		}
		break;
	};
	case GAME_ENTITY_TURN_TYPE_USE_MOVE: {
		GameMove* move = turn->data.use_move.move;

		switch (move->target_mode) {
			case GAME_MOVE_TARGET_MODE_ENTITY:
				if (move->splash_radius > 0.f) {
					GameEntity* target;
					HeroResult result = game_scene_play_entity_get(scene, turn->data.use_move.data.target_entity_id, &target);
					HERO_RESULT_ASSERT(result);

					GameEntityFilterArgs filter_args = {0};
					filter_args.origin_tile_x = (U32)floorf(entity->position.x);
					filter_args.origin_tile_y = (U32)floorf(entity->position.z);
					filter_args.radius = move->range_radius;
					filter_args.splash_origin_tile_x = (U32)floorf(target->position.x);
					filter_args.splash_origin_tile_y = (U32)floorf(target->position.z);
					filter_args.splash_radius = move->splash_radius;
					filter_args.ignore_entity_id = id;
					game_scene_play_entity_filter_start(scene, &filter_args);

					GameEntity* entity = NULL;
					GameEntityId entity_id = {0};
					while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
						game_scene_play_entity_filter_check(scene, entity_id, entity, &filter_args);

						if (filter_args._valid && filter_args._splash) {
							game_scene_play_entity_apply_damage(scene, entity_id);
						}
					}
					HERO_RESULT_ASSERT(result);
				} else {
					game_scene_play_entity_apply_damage(scene, turn->data.use_move.data.target_entity_id);
				}
				break;
			case GAME_MOVE_TARGET_MODE_DIRECTION: {
				GameEntityFilterArgs filter_args = {0};
				filter_args.origin_tile_x = (U32)floorf(entity->position.x);
				filter_args.origin_tile_y = (U32)floorf(entity->position.z);
				filter_args.radius = move->range_radius;
				filter_args.aim_direction = turn->data.use_move.data.target_direction;
				filter_args.spread_angle = move->spread_angle;
				filter_args.ignore_entity_id = id;
				game_scene_play_entity_filter_start(scene, &filter_args);

				GameEntity* entity = NULL;
				GameEntityId entity_id = {0};
				while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
					game_scene_play_entity_filter_check(scene, entity_id, entity, &filter_args);
					if (filter_args._valid) {
						game_scene_play_entity_apply_damage(scene, entity_id);
					}
				}
				HERO_RESULT_ASSERT(result);
				break;
			};
			case GAME_MOVE_TARGET_MODE_TILE: {
				GameEntityFilterArgs filter_args = {0};
				filter_args.origin_tile_x = (U32)floorf(entity->position.x);
				filter_args.origin_tile_y = (U32)floorf(entity->position.z);
				filter_args.radius = move->range_radius;
				filter_args.splash_origin_tile_x = turn->data.use_move.data.target_tile.x;
				filter_args.splash_origin_tile_y = turn->data.use_move.data.target_tile.y;
				filter_args.splash_radius = move->splash_radius;
				filter_args.ignore_entity_id = id;
				game_scene_play_entity_filter_start(scene, &filter_args);

				GameEntity* entity = NULL;
				GameEntityId entity_id = {0};
				while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
					game_scene_play_entity_filter_check(scene, entity_id, entity, &filter_args);

					if (filter_args._valid && filter_args._splash) {
						game_scene_play_entity_apply_damage(scene, entity_id);
					}
				}
				HERO_RESULT_ASSERT(result);
				break;
			};
			case GAME_MOVE_TARGET_MODE_SELF: {
				GameEntityFilterArgs filter_args = {0};
				filter_args.origin_tile_x = (U32)floorf(entity->position.x);
				filter_args.origin_tile_y = (U32)floorf(entity->position.z);
				filter_args.radius = move->range_radius;
				filter_args.ignore_entity_id = id;
				game_scene_play_entity_filter_start(scene, &filter_args);

				GameEntity* entity = NULL;
				GameEntityId entity_id = {0};
				while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
					game_scene_play_entity_filter_check(scene, entity_id, entity, &filter_args);

					if (filter_args._valid) {
						game_scene_play_entity_apply_damage(scene, entity_id);
					}
				}
				HERO_RESULT_ASSERT(result);
				break;
			};
		}

		break;
	};
	case GAME_ENTITY_TURN_TYPE_USE_ABILITY:
		HERO_ABORT("UNIMPLEMENTED");
		break;
	case GAME_ENTITY_TURN_TYPE_USE_ITEM:
		HERO_ABORT("UNIMPLEMENTED");
		break;
	}
}

HeroResult game_scene_play_player_handle_input(GameScenePlay* scene, GameInputState state) {
	HeroResult result;

	GameEntity* player;
	result = game_scene_play_entity_get(scene, scene->player_entity_id, &player);
	HERO_RESULT_ASSERT(result);

	GameEntityTurn* entity_turn = &scene->player_entity_turn;

	GamePlayerActionState prev_state = scene->player_action_state;
	GamePlayerActionState next_state = scene->player_action_state;
	switch (scene->player_action_state) {
		case GAME_PLAYER_ACTION_STATE_MOVING:
			if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_A)) {
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_B)) {
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_X)) {
				next_state = GAME_PLAYER_ACTION_STATE_MOVE_DIALOG;
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_Y)) {
			} else {
				GameDirection direction = GAME_DIRECTION_NONE;
				if (state.is_pressed & ENUM_FLAG(GAME_INPUT_UP)) {
					direction = GAME_DIRECTION_UP;
				}
				if (state.is_pressed & ENUM_FLAG(GAME_INPUT_LEFT)) {
					direction = GAME_DIRECTION_LEFT;
				}
				if (state.is_pressed & ENUM_FLAG(GAME_INPUT_DOWN)) {
					direction = GAME_DIRECTION_DOWN;
				}
				if (state.is_pressed & ENUM_FLAG(GAME_INPUT_RIGHT)) {
					direction = GAME_DIRECTION_RIGHT;
				}
				entity_turn->type = GAME_ENTITY_TURN_TYPE_MOVE_TILE;
				entity_turn->data.move_tile.direction = direction;
				game_scene_play_entity_perform_turn(scene, scene->player_entity_id, &scene->player_entity_turn);
			}
			break;
		case GAME_PLAYER_ACTION_STATE_INTERACT_DIALOG:
			break;
		case GAME_PLAYER_ACTION_STATE_MOVE_DIALOG:
			if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_A)) {
				if (scene->player_target_entity_id.raw) {
					GameMove* move = &game_moves[scene->player_selected_move_idx];
					entity_turn->data.use_move.move = move;
					switch (move->target_mode) {
						case GAME_MOVE_TARGET_MODE_ENTITY: {
							next_state = GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY;
							break;
						};
						case GAME_MOVE_TARGET_MODE_DIRECTION:
							entity_turn->data.use_move.data.target_direction = scene->player_move_aim_direction;
							// fallthrough
						case GAME_MOVE_TARGET_MODE_SELF:
							game_scene_play_entity_perform_turn(scene, scene->player_entity_id, &scene->player_entity_turn);
							next_state = GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE;
							break;
						case GAME_MOVE_TARGET_MODE_TILE:
							next_state = GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE;
							break;
					}
				}
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_B)) {
				next_state = GAME_PLAYER_ACTION_STATE_MOVING;
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_UP)) {
				U32 move_idx = scene->player_selected_move_idx == 0 ? 3 : scene->player_selected_move_idx - 1;
				game_scene_play_combat_select_move(scene, move_idx);
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_DOWN)) {
				U32 move_idx = scene->player_selected_move_idx == 3 ? 0 : scene->player_selected_move_idx + 1;
				game_scene_play_combat_select_move(scene, move_idx);
			} else if (state.is_pressed & ENUM_FLAG(GAME_INPUT_LEFT)) {
				scene->player_move_aim_direction = vec2_rotate(scene->player_move_aim_direction, HERO_DEGREES_TO_RADIANS(90.f) + (-GAME_PLAYER_AIM_DIRECTION_ROTATE_SPEED_RADIUS_PER_SEC * game.dt));
				game_scene_play_combat_select_move(scene, scene->player_selected_move_idx);
			} else if (state.is_pressed & ENUM_FLAG(GAME_INPUT_RIGHT)) {
				scene->player_move_aim_direction = vec2_rotate(scene->player_move_aim_direction, HERO_DEGREES_TO_RADIANS(90.f) + (GAME_PLAYER_AIM_DIRECTION_ROTATE_SPEED_RADIUS_PER_SEC * game.dt));
				game_scene_play_combat_select_move(scene, scene->player_selected_move_idx);
			}
			break;
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY: {
			if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_A)) {
				entity_turn->data.use_move.data.target_entity_id = scene->player_target_entity_id;
				game_scene_play_entity_perform_turn(scene, scene->player_entity_id, &scene->player_entity_turn);
				next_state = GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE;
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_B)) {
				next_state = GAME_PLAYER_ACTION_STATE_MOVE_DIALOG;
			} else {
				bool has_been_pressed = false;
				bool is_backwards = false;

				if (state.has_been_pressed & (ENUM_FLAG(GAME_INPUT_UP) | ENUM_FLAG(GAME_INPUT_LEFT))) {
					has_been_pressed = true;
					is_backwards = false;
				} else if (state.has_been_pressed & (ENUM_FLAG(GAME_INPUT_DOWN) | ENUM_FLAG(GAME_INPUT_RIGHT))) {
					has_been_pressed = true;
					is_backwards = true;
				}

				if (has_been_pressed) {
					GameMove* move = &game_moves[scene->player_selected_move_idx];
					GameEntityFilterArgs filter_args = {0};
					filter_args.origin_tile_x = (U32)floorf(player->position.x);
					filter_args.origin_tile_y = (U32)floorf(player->position.z);
					filter_args.radius = move->range_radius;
					filter_args.ignore_entity_id = scene->player_entity_id;

					game_scene_play_entity_iter_next_in_range(scene, &scene->player_target_entity_id, &filter_args, is_backwards);
					game_scene_play_camera_set_target_entity(scene, scene->player_target_entity_id);
					if (move->splash_radius > 0.f) {
						game_scene_play_combat_update_tints(scene, NULL);
					}
				}
			}
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE: {
			if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_A)) {
				entity_turn->data.use_move.data.target_tile.x = scene->player_target_tile_x;
				entity_turn->data.use_move.data.target_tile.y = scene->player_target_tile_y;
				game_scene_play_entity_perform_turn(scene, scene->player_entity_id, &scene->player_entity_turn);
				next_state = GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE;
			} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_B)) {
				next_state = GAME_PLAYER_ACTION_STATE_MOVE_DIALOG;
			} else {
				bool has_been_pressed = false;
				bool is_backwards = false;

				U32 og_target_tile_x = scene->player_target_tile_x;
				U32 og_target_tile_y = scene->player_target_tile_y;

				if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_UP)) {
					scene->player_target_tile_y += 1;
				} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_DOWN)) {
					scene->player_target_tile_y -= 1;
				} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_LEFT)) {
					scene->player_target_tile_x -= 1;
				} else if (state.has_been_pressed & ENUM_FLAG(GAME_INPUT_RIGHT)) {
					scene->player_target_tile_x += 1;
				}

				GameMove* move = &game_moves[scene->player_selected_move_idx];
				F32 squared_radius = (F32)move->range_radius * (F32)move->range_radius;
				if (og_target_tile_x != scene->player_target_tile_x || og_target_tile_y != scene->player_target_tile_y) {
					Vec2 offset = vec2_sub(VEC2_INIT((F32)scene->player_target_tile_x, (F32)scene->player_target_tile_y), VEC2_INIT(floorf(player->position.x), floorf(player->position.z)));
					F32 squared_distance = vec2_dot(offset, offset);

					if (squared_distance <= squared_radius) {
						game_scene_play_combat_update_tints(scene, NULL);
						game_scene_play_camera_set_target_tile(scene, scene->player_target_tile_x, scene->player_target_tile_y);
					} else {
						scene->player_target_tile_x = og_target_tile_x;
						scene->player_target_tile_y = og_target_tile_y;
					}
				}
			}
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE:
			break;
		case GAME_PLAYER_ACTION_STATE_ITEM:
			break;
		case GAME_PLAYER_ACTION_STATE_ABILITY_DIALOG:
			break;
		case GAME_PLAYER_ACTION_STATE_ABILITY_TARGET_ENTITY:
			break;
		case GAME_PLAYER_ACTION_STATE_ABILITY_TARGET_DIRECTION:
			break;
	}

	//
	// if the state has changed then setup the next state
	if (prev_state != next_state) {
		scene->player_action_state = next_state;
		switch (next_state) {
		case GAME_PLAYER_ACTION_STATE_MOVING: {
			scene->flags &= ~GAME_SCENE_PLAY_GLOW;
			game_scene_play_camera_set_target_entity(scene, scene->player_entity_id);

			if (prev_state == GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE) {
				GameEntity* entity = NULL;
				GameEntityId entity_id = {0};
				while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
					entity->animate_og_health = entity->health;
				}
			}

			HERO_ZERO_ELMT_MANY(scene->terrain_tile_tint_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
			scene->render_data.terrain_tile_tint_map = scene->terrain_tile_tint_map;
			break;
		};
		case GAME_PLAYER_ACTION_STATE_INTERACT_DIALOG:
			break;
		case GAME_PLAYER_ACTION_STATE_MOVE_DIALOG:
			scene->flags |= GAME_SCENE_PLAY_GLOW;
			entity_turn->type = GAME_ENTITY_TURN_TYPE_USE_MOVE;
			game_scene_play_camera_set_target_entity(scene, scene->player_entity_id);
			if (prev_state == GAME_PLAYER_ACTION_STATE_MOVING) {
				game_scene_play_combat_select_move(scene, 0);
			} else {
				game_scene_play_combat_update_tints(scene, NULL);
			}
			break;
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY: {
			game_scene_play_camera_set_target_entity(scene, scene->player_target_entity_id);
			if (game_moves[scene->player_selected_move_idx].splash_radius > 0.f) {
				game_scene_play_combat_update_tints(scene, NULL);
			}
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE: {
			scene->player_target_tile_x = (U32)floorf(player->position.x);
			scene->player_target_tile_y = (U32)floorf(player->position.z);
			break;
		};
		case GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE: {
			scene->player_move_animate_time = 0.f;
			break;
		};
		}
	}

	return HERO_SUCCESS;
}

U8 game_scene_play_entity_ascii(GameEntity* entity) {
	if (entity->flags & GAME_ENTITY_FLAGS_PLAYER) {
		return '@';
	}

	switch (entity->type) {
		case GAME_ENTITY_TYPE_HUMAN:
			return 'H';
	}

	return '\0';
}

void game_scene_play_entity_filter_start(GameScenePlay* scene, GameEntityFilterArgs* args) {
	args->_squared_radius = args->radius * args->radius;
	args->_prev_squared_distance = 0.f;
	args->_valid = false;
	args->_splash = false;
	args->_half_angle_cos = cosf(args->spread_angle / 2.f);
	args->_squared_half_angle_cos = args->_half_angle_cos * fabsf(args->_half_angle_cos);
	args->_squared_splash_radius = args->splash_radius * args->splash_radius;
}

void game_scene_play_entity_filter_check(GameScenePlay* scene, GameEntityId entity_id, GameEntity* entity, GameEntityFilterArgs* args) {
	args->_valid = false;
	args->_splash = false;

	if (entity_id.raw == args->ignore_entity_id.raw) {
		return;
	}

	S32 offset_x = (S32)floorf(entity->position.x) - args->origin_tile_x;
	S32 offset_y = (S32)floorf(entity->position.z) - args->origin_tile_y;

	U32 squared_distance = (offset_x * offset_x) + (offset_y * offset_y);
	args->_prev_squared_distance = squared_distance;
	if (squared_distance > args->_squared_radius) {
		return;
	}
	if (args->spread_angle > 0.f) {
		Vec2 offset = VEC2_INIT((F32)offset_x, (F32)offset_y);
		F32 entity_angle_cos = vec2_dot(offset, args->aim_direction);
		F32 squared_tile_angle_cos = entity_angle_cos * fabsf(entity_angle_cos);
		F32 result = args->_squared_half_angle_cos * squared_distance;
		if (result > squared_tile_angle_cos) {
			return;
		}
	}
	args->_valid = true;

	if (args->_squared_splash_radius > 0.f) {
		S32 splash_offset_x = (S32)floorf(entity->position.x) - args->splash_origin_tile_x;
		S32 splash_offset_y = (S32)floorf(entity->position.z) - args->splash_origin_tile_y;
		U32 squared_splash_distance = (splash_offset_x * splash_offset_x) + (splash_offset_y * splash_offset_y);
		if (squared_splash_distance <= args->_squared_splash_radius) {
			args->_splash = true;
		}
	}
}

HeroResult game_scene_play_entity_find_closest_in_range(GameScenePlay* scene, GameEntityFilterArgs* args, GameEntityId* id_out) {
	HeroResult result;
	game_scene_play_entity_filter_start(scene, args);

	GameEntity* entity = NULL;
	GameEntityId entity_id = {0};
	F32 closest_squared_distance = INFINITY;
	GameEntityId closest_entity_id = {0};
	while ((result = hero_object_pool(GameEntity, iter_next)(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
		game_scene_play_entity_filter_check(scene, entity_id, entity, args);
		if (!args->_valid) {
			continue;
		}

		if (args->_prev_squared_distance < closest_squared_distance && args->_prev_squared_distance <= args->_squared_radius) {
			closest_entity_id = entity_id;
			closest_squared_distance = args->_prev_squared_distance;
		}
	}
	HERO_RESULT_ASSERT(result);

	if (closest_entity_id.raw) {
		*id_out = closest_entity_id;
		return HERO_SUCCESS;
	} else {
		return HERO_ERROR_DOES_NOT_EXIST;
	}
}

HeroResult game_scene_play_entity_iter_next_in_range(GameScenePlay* scene, GameEntityId* id_mut, GameEntityFilterArgs* args, bool backwards) {
	HeroResult result;
	game_scene_play_entity_filter_start(scene, args);

	GameEntity* entity = NULL;
	GameEntityId entity_id = {0};

START_AGAIN: {}

	HeroResult (*iter_fn)(HeroObjectPool(GameEntity)* entity_pool, GameEntityId* id_mut, GameEntity** out) =
		backwards
			? hero_object_pool(GameEntity, iter_prev)
			: hero_object_pool(GameEntity, iter_next);

	while ((result = iter_fn(&scene->entity_pool, &entity_id, &entity)) != HERO_SUCCESS_FINISHED) {
		game_scene_play_entity_filter_check(scene, entity_id, entity, args);
		if (!args->_valid) {
			continue;
		}

		if (args->_prev_squared_distance <= args->_squared_radius) {
			*id_mut = entity_id;
			return HERO_SUCCESS;
		}
	}
	HERO_RESULT_ASSERT(result);

	//
	// go round again if we didn't start from the beginning and found nothing.
	if (id_mut->raw != 0 && entity_id.raw == 0) {
		goto START_AGAIN;
	}

	return HERO_ERROR_DOES_NOT_EXIST;
}

void game_scene_play_entity_apply_damage(GameScenePlay* scene, GameEntityId target_id) {
	GameEntity* target;
	HeroResult result = game_scene_play_entity_get(scene, target_id, &target);
	HERO_RESULT_ASSERT(result);

	target->animate_og_health = target->health;

	GameMove* move = &game_moves[scene->player_selected_move_idx];
	U32 damage = move->damage;
	if (target->health <= damage) {
		target->health = 0;
	} else {
		target->health -= damage;
	}
}

void game_scene_play_camera_set_target_tile(GameScenePlay* scene, U32 tile_x, U32 tile_y) {
	GameCamera* camera = &scene->camera;
	camera->target_entity_id.raw = 0;

	F32 y = game_scene_play_tile_max_voxel_height(scene, tile_x, tile_y);
	camera->target_position = VEC3_INIT(tile_x, y, tile_y);
}

void game_scene_play_camera_set_target_entity(GameScenePlay* scene, GameEntityId entity_id) {
	GameCamera* camera = &scene->camera;
	if (camera->target_entity_id.raw == entity_id.raw) {
		return;
	}

	camera->target_entity_id = entity_id;
}

void game_scene_play_camera_update(GameScenePlay* scene) {
	GameCamera* camera = &scene->camera;
	Vec3 position;
	if (camera->target_entity_id.raw) {
		GameEntity* entity;
		HeroResult result = game_scene_play_entity_get(scene, camera->target_entity_id, &entity);
		HERO_RESULT_ASSERT(result);

		position = entity->position;
	} else {
		position = camera->target_position;
	}

	Vec3 target_vec = vec3_sub(vec3_add(position, GAME_CAMERA_TARGET_OFFSET), camera->position);
	F32 distance = vec3_len(target_vec);

	F32 move_distance = HERO_MAX(distance, GAME_PLAYER_MOVE_SPEED);
	move_distance *= game.dt;
	move_distance = HERO_MIN(move_distance, distance);

	Vec3 target_dir = vec3_norm(target_vec);
	Vec3 move_vec = vec3_mul_scalar(target_dir, move_distance);

	camera->position = vec3_add(camera->position, move_vec);

	static int init = 0;
	if (!init) {
		Vec3 camera_target_pos = vec3_sub(camera->position, GAME_CAMERA_TARGET_OFFSET);
		camera->forward = vec3_norm(vec3_sub(vec3_add(camera_target_pos, VEC3_INIT(0.5f, 0.f, 0.5f)), camera->position));
		F32 pitch = asinf(-camera->forward.y);
		F32 yaw = atan2f(camera->forward.x, camera->forward.z);
		Quat x_rotation = quat_rotate(yaw, VEC3_INIT(0.f, 1.f, 0.f));
		Quat y_rotation = quat_rotate(pitch, VEC3_INIT(1.f, 0.f, 0.f));
		camera->rotation = quat_norm(quat_mul(x_rotation, y_rotation));
		init = 1;
	}

	game_scene_play_update_mvp(scene);
}

void game_scene_play_update_mvp(GameScenePlay* scene) {
	Mat4x4 view_to_ndc;
	mat4x4_perspective(&view_to_ndc, HERO_DEGREES_TO_RADIANS(45.f), (float)1920 / (float)1080, 0.1f, 100.f);

	Mat4x4 world_to_view;
	mat4x4_identity(&world_to_view);

	mat4x4_mul_quat(&world_to_view, &world_to_view, quat_conj(scene->camera.rotation));

	mat4x4_translate(&world_to_view, vec3_neg(scene->camera.position));

	mat4x4_mul(&scene->mvp, &world_to_view, &view_to_ndc);
}

Vec2 game_scene_play_world_to_screen_space_point(GameScenePlay* scene, Vec3 position) {
	Vec4 screen_pos = mat4x4_mul_vec4(&scene->mvp, VEC4_INIT(position.x, position.y, position.z, 1.f));
	screen_pos.x = screen_pos.x / fabsf(screen_pos.w) * 960.f + 960.f;
	screen_pos.y = screen_pos.y / fabsf(screen_pos.w) * 540.f + 540.f;
	screen_pos.y = 1080.f - screen_pos.y;

	return VEC2_INIT(screen_pos.x, screen_pos.y);
}

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
	HeroResult result;

	GameSceneType scene_type = GAME_SCENE_TYPE_PLAY;
	switch (scene_type) {
		case GAME_SCENE_TYPE_ISLAND_GEN_DEBUG:
			result = game_scene_island_gen_debug_init(&game.scene);
			break;
		case GAME_SCENE_TYPE_PLAY:
			result = game_scene_play_init(&game.scene);
			break;
	}
	HERO_RESULT_ASSERT(result);

	result = game_scene_load(game.scene);
	HERO_RESULT_ASSERT(result);
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

void game_logic_update(void) {
	HeroResult result;

	HeroUIWindow* window = hero_ui_window_start(game.ui_window_id, game.gfx.render_width, game.gfx.render_height);

	result = game_scene_update(game.scene, &game.scene);
	HERO_RESULT_ASSERT(result);

	hero_ui_window_end(window);

	result = hero_ui_window_update(game.ui_window_id);
	HERO_RESULT_ASSERT(result);
}

// ===========================================
//
//
// Graphics: Common
//
//
// ===========================================

HeroResult game_gfx_shader_init(const char* name, HeroShaderType type, HeroShaderId* id_out) {
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
	shader_stages.type = type;
	switch (type) {
		case HERO_SHADER_TYPE_GRAPHICS: {
			shader_stages.data.graphics.vertex = (HeroShaderStage) {
				.backend.spir_v.entry_point_name = "vertex",
				.module_id = shader_module_id,
			};
			shader_stages.data.graphics.fragment = (HeroShaderStage) {
				.backend.spir_v.entry_point_name = "fragment",
				.module_id = shader_module_id,
			};
			break;
		};
		case HERO_SHADER_TYPE_COMPUTE: {
			shader_stages.data.compute.compute = (HeroShaderStage) {
				.backend.spir_v.entry_point_name = "compute",
				.module_id = shader_module_id,
			};
			break;
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
// Graphics Scene: Island Generator Debug
//
//
// ===========================================

typedef struct GameIslandGenDebugVertex GameIslandGenDebugVertex;
struct GameIslandGenDebugVertex {
	Vec2 pos;
	Vec2 uv;
};

static HeroVertexAttribInfo game_gfx_island_gen_debug_vertex_attribs[] = {
	{ // GameIslandGenDebugVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
	{ // GameIslandGenDebugVertex.uv
		.location = 1,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
};

static HeroVertexBindingInfo game_gfx_island_gen_debug_vertex_bindings[] = {
	{
		.attribs = game_gfx_island_gen_debug_vertex_attribs,
		.attribs_count = HERO_ARRAY_COUNT(game_gfx_island_gen_debug_vertex_attribs),
		.size = sizeof(GameIslandGenDebugVertex),
	}
};

static HeroVertexLayout game_gfx_island_gen_debug_vertex_layout = {
	.bindings_count = HERO_ARRAY_COUNT(game_gfx_island_gen_debug_vertex_bindings),
	.bindings = game_gfx_island_gen_debug_vertex_bindings,
};

HeroResult game_gfx_island_gen_debug_init() {
	GameGfxIslandGenDebug* gfx = &game.gfx.island_gen_debug;
	HeroLogicalDevice* ldev = game.gfx.ldev;
	HeroResult result;

	result = hero_vertex_layout_register(&game_gfx_island_gen_debug_vertex_layout, true, &gfx->vertex_layout_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 4,
			.typed.vertex.layout_id = gfx->vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->vertex_buffer_id);
		HERO_RESULT_ASSERT(result);

		{
			GameIslandGenDebugVertex* vertices;
			result = hero_buffer_write(ldev, gfx->vertex_buffer_id, 0, 4, (void**)&vertices);
			HERO_RESULT_ASSERT(result);
			vertices[0].pos = VEC2_INIT(0.f, 0.f);
			vertices[0].uv = VEC2_INIT(0.f, 0.f);
			vertices[1].pos = VEC2_INIT(GAME_ISLAND_AXIS_TILES_COUNT, 0.f);
			vertices[1].uv = VEC2_INIT(1.f, 0.f);
			vertices[2].pos = VEC2_INIT(GAME_ISLAND_AXIS_TILES_COUNT, GAME_ISLAND_AXIS_TILES_COUNT);
			vertices[2].uv = VEC2_INIT(1.f, 1.f);
			vertices[3].pos = VEC2_INIT(0.f, GAME_ISLAND_AXIS_TILES_COUNT);
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

		result = hero_buffer_init(ldev, &setup, &gfx->index_buffer_id);
		HERO_RESULT_ASSERT(result);

		U32* indices;
		result = hero_buffer_write(ldev, gfx->index_buffer_id, 0, 6, (void**)&indices);
		HERO_RESULT_ASSERT(result);
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 2;
		indices[4] = 3;
		indices[5] = 0;
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_UNIFORM,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 1,
			.elmt_size = sizeof(GameIslandGenDebugGlobalUBO),
		};

		result = hero_buffer_init(ldev, &setup, &gfx->uniform_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	result = game_gfx_shader_init("island_gen_debug", HERO_SHADER_TYPE_GRAPHICS, &gfx->shader_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroDescriptorShaderInfo shader_infos[] = {
			{
				.shader_id = gfx->shader_id,
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
		result = hero_descriptor_pool_init(ldev, &setup, &gfx->descriptor_pool_id);
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
			.width = GAME_ISLAND_AXIS_TILES_COUNT,
			.height = GAME_ISLAND_AXIS_TILES_COUNT,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(ldev, &setup, &gfx->tile_height_map_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_SAMPLED | HERO_IMAGE_FLAGS_COLOR_ATTACHMENT,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = GAME_ISLAND_AXIS_TILES_COUNT,
			.height = GAME_ISLAND_AXIS_TILES_COUNT,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(ldev, &setup, &gfx->attachment_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = gfx->shader_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_shader_globals_init(ldev, &setup, &gfx->shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(ldev, gfx->shader_globals_id, GAME_ISLAND_GEN_DEBUG_GLOBAL_BINDING_GLOBAL_UBO, 0, gfx->uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_image_sampler(ldev, gfx->shader_globals_id, GAME_ISLAND_GEN_DEBUG_GLOBAL_BINDING_TILE_MAP, 0, gfx->tile_height_map_image_id, game.gfx.clamp_linear_sampler_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(ldev, gfx->shader_globals_id);
		HERO_RESULT_ASSERT(result);
	}

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

		HeroPipelineGraphicsSetup setup = {
			.render_state = &render_state,
			.shader_id = gfx->shader_id,
			.render_pass_layout_id = gfx->render_pass_layout_id,
			.vertex_layout_id = gfx->vertex_layout_id,
			.cache_id.raw = 0,
		};

		result = hero_pipeline_graphics_init(ldev, &setup, &gfx->pipeline_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroMaterialSetup setup = {
			.pipeline_id = gfx->pipeline_id,
			.shader_globals_id = gfx->shader_globals_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_material_init(ldev, &setup, &gfx->material_id);
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
			.layout_id = gfx->render_pass_layout_id,
			.draw_cmd_descriptor_pool_id = gfx->descriptor_pool_id,
			.attachment_clear_values = NULL,
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
		};

		result = hero_render_pass_init(ldev, &setup, &gfx->render_pass_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroFrameBufferSetup setup = {
			.attachments = &gfx->attachment_image_id,
			.attachments_count = 1,
			.layers = 1,
			.render_pass_layout_id = gfx->render_pass_layout_id,
			.width = GAME_ISLAND_AXIS_TILES_COUNT,
			.height = GAME_ISLAND_AXIS_TILES_COUNT,

		};

		result = hero_frame_buffer_init(ldev, &setup, &gfx->frame_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

HeroResult game_gfx_island_gen_debug_deinit() {
	HERO_ABORT("unimplemented");
	return HERO_SUCCESS;
}

HeroResult game_gfx_island_gen_debug_render(HeroCommandRecorder* command_recorder) {
	HeroResult result;
	/*
	GameGfxIslandGenDebug* gfx = &game.gfx.island_gen_debug;

	if (gfx->scene_render_data.has_been_updated) {
		F32* tile_height_map;
		result = hero_image_write(game.gfx.ldev, gfx->tile_height_map_image_id, NULL, (void**)&tile_height_map);
		HERO_RESULT_ASSERT(result);
		HERO_COPY_ELMT_MANY(tile_height_map, gfx->scene_render_data.tile_height_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	}

	GameIslandGenDebugGlobalUBO* global_ubo;
	result = hero_buffer_write(game.gfx.ldev, gfx->uniform_buffer_id, 0, 1, (void**)&global_ubo);
	HERO_RESULT_ASSERT(result);
	mat4x4_ortho(&global_ubo->mvp, 0.f, (F32)GAME_ISLAND_AXIS_TILES_COUNT, (F32)GAME_ISLAND_AXIS_TILES_COUNT, 0.f, -1.f, 1.f);
	global_ubo->deep_sea_max = gfx->scene_render_data.deep_sea_max;
	global_ubo->sea_max = gfx->scene_render_data.sea_max;
	global_ubo->ground_max = gfx->scene_render_data.ground_max;
	global_ubo->show_grid = gfx->scene_render_data.show_grid;
	global_ubo->show_height_map = gfx->scene_render_data.show_height_map;

	result = hero_cmd_render_pass_start(command_recorder, gfx->render_pass_id, gfx->frame_buffer_id, NULL, NULL);
	HERO_RESULT_ASSERT(result);

	{
		result = hero_cmd_draw_start(command_recorder, gfx->material_id);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_set_vertex_buffer(command_recorder, gfx->vertex_buffer_id, 0, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_end_indexed(command_recorder, gfx->index_buffer_id, 0, 6, 0);
		HERO_RESULT_ASSERT(result);
	}

	result = hero_cmd_render_pass_end(command_recorder);
	HERO_RESULT_ASSERT(result);

	*/
	return HERO_SUCCESS;
}

// ===========================================
//
//
// Graphics Scene: Play
//
//
// ===========================================

typedef struct GameTerrainTileVertex GameTerrainTileVertex;
struct GameTerrainTileVertex {
	Vec3 pos;
	Vec3 normal;
};

static HeroVertexAttribInfo game_gfx_terrain_tile_attribs[] = {
	{ // GameTerrainTileVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_3,
	},
	{ // GameTerrainTileVertex.normal
		.location = 1,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_3,
	},
};

static HeroVertexBindingInfo game_gfx_terrain_tile_bindings[] = {
	{
		.attribs = game_gfx_terrain_tile_attribs,
		.attribs_count = HERO_ARRAY_COUNT(game_gfx_terrain_tile_attribs),
		.size = sizeof(GameTerrainTileVertex),
	}
};

static HeroVertexLayout game_gfx_terrain_tile_layout = {
	.bindings_count = HERO_ARRAY_COUNT(game_gfx_terrain_tile_bindings),
	.bindings = game_gfx_terrain_tile_bindings,
};

typedef struct GameModelVertex GameModelVertex;
struct GameModelVertex {
	Vec2 pos;
};

static HeroVertexAttribInfo game_gfx_model_attribs[] = {
	{ // GameModelVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
};

static HeroVertexBindingInfo game_gfx_model_bindings[] = {
	{
		.attribs = game_gfx_model_attribs,
		.attribs_count = HERO_ARRAY_COUNT(game_gfx_model_attribs),
		.size = sizeof(GameModelVertex),
	}
};

static HeroVertexLayout game_gfx_model_layout = {
	.bindings_count = HERO_ARRAY_COUNT(game_gfx_model_bindings),
	.bindings = game_gfx_model_bindings,
};

typedef struct GameBillboardVertex GameBillboardVertex;
struct GameBillboardVertex {
	Vec3 pos;
	Vec2 uv;
	U32 color;
	U32 tint;
};

static HeroVertexAttribInfo game_gfx_billboard_attribs[] = {
	{ // GameBillboardVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_3,
	},
	{ // GameBillboardVertex.uv
		.location = 1,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
	{ // GameBillboardVertex.color
		.location = 2,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_U32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_1,
	},
	{ // GameBillboardVertex.tint
		.location = 3,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_U32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_1,
	},
};

static HeroVertexBindingInfo game_gfx_billboard_bindings[] = {
	{
		.attribs = game_gfx_billboard_attribs,
		.attribs_count = HERO_ARRAY_COUNT(game_gfx_billboard_attribs),
		.size = sizeof(GameBillboardVertex),
	}
};

static HeroVertexLayout game_gfx_billboard_layout = {
	.bindings_count = HERO_ARRAY_COUNT(game_gfx_billboard_bindings),
	.bindings = game_gfx_billboard_bindings,
};

HeroResult _game_gfx_play_terrain_tile_init() {
	GameGfxPlay* gfx = &game.gfx.play;
	HeroLogicalDevice* ldev = game.gfx.ldev;
	HeroResult result;

	result = hero_vertex_layout_register(&game_gfx_terrain_tile_layout, true, &gfx->terrain_tile_vertex_layout_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = GAME_ISLAND_TILE_VOXELS_VERTICES_COUNT,
			.typed.vertex.layout_id = gfx->terrain_tile_vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->terrain_tile_vertex_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_INDEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = GAME_ISLAND_TILE_VOXELS_INDICES_COUNT,
			.typed.index_type = HERO_INDEX_TYPE_U32,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->terrain_tile_index_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	GameTerrainTileVertex* vertices;
	result = hero_buffer_write(game.gfx.ldev, gfx->terrain_tile_vertex_buffer_id, 0, GAME_ISLAND_TILE_VOXELS_VERTICES_COUNT, (void**)&vertices);
	HERO_RESULT_ASSERT(result);

	U32* indices;
	result = hero_buffer_write(game.gfx.ldev, gfx->terrain_tile_index_buffer_id, 0, GAME_ISLAND_TILE_VOXELS_INDICES_COUNT, (void**)&indices);
	HERO_RESULT_ASSERT(result);

	F32 pos_y = 0.f;
	U32 vertices_start_idx = 0;
	U32 indices_start_idx = 0;
	for_range(vy, 0, GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) {
		F32 pos_x = 0.f;

		for_range(vx, 0, GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) {
			//
			// top
			//
			vertices[vertices_start_idx + 0] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 1.f, pos_y),
				.normal = VEC3_UP,
			};
			vertices[vertices_start_idx + 1] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 1.f, pos_y + GAME_ISLAND_TILE_VOXEL_STEP),
				.normal = VEC3_UP,
			};
			vertices[vertices_start_idx + 2] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 1.f, pos_y + GAME_ISLAND_TILE_VOXEL_STEP),
				.normal = VEC3_UP,
			};
			vertices[vertices_start_idx + 3] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 1.f, pos_y),
				.normal = VEC3_UP,
			};

			//
			// backward
			//
			vertices[vertices_start_idx + 4] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 0.f, pos_y),
				.normal = VEC3_BACKWARD,
			};
			vertices[vertices_start_idx + 5] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 1.f, pos_y),
				.normal = VEC3_BACKWARD,
			};
			vertices[vertices_start_idx + 6] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 1.f, pos_y),
				.normal = VEC3_BACKWARD,
			};
			vertices[vertices_start_idx + 7] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 0.f, pos_y),
				.normal = VEC3_BACKWARD,
			};

			//
			// left
			//
			vertices[vertices_start_idx + 8] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 0.f, pos_y + GAME_ISLAND_TILE_VOXEL_STEP),
				.normal = VEC3_LEFT,
			};
			vertices[vertices_start_idx + 9] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 1.f, pos_y + GAME_ISLAND_TILE_VOXEL_STEP),
				.normal = VEC3_LEFT,
			};
			vertices[vertices_start_idx + 10] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 1.f, pos_y),
				.normal = VEC3_LEFT,
			};
			vertices[vertices_start_idx + 11] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x, 0.f, pos_y),
				.normal = VEC3_LEFT,
			};


			//
			// right
			//
			vertices[vertices_start_idx + 12] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 0.f, pos_y + GAME_ISLAND_TILE_VOXEL_STEP),
				.normal = VEC3_LEFT,
			};
			vertices[vertices_start_idx + 13] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 1.f, pos_y + GAME_ISLAND_TILE_VOXEL_STEP),
				.normal = VEC3_LEFT,
			};
			vertices[vertices_start_idx + 14] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 1.f, pos_y),
				.normal = VEC3_LEFT,
			};
			vertices[vertices_start_idx + 15] = (GameTerrainTileVertex) {
				.pos = VEC3_INIT(pos_x + GAME_ISLAND_TILE_VOXEL_STEP, 0.f, pos_y),
				.normal = VEC3_LEFT,
			};


			//
			// top
			//
			indices[indices_start_idx + 0] = vertices_start_idx + 0;
			indices[indices_start_idx + 1] = vertices_start_idx + 1;
			indices[indices_start_idx + 2] = vertices_start_idx + 2;
			indices[indices_start_idx + 3] = vertices_start_idx + 2;
			indices[indices_start_idx + 4] = vertices_start_idx + 3;
			indices[indices_start_idx + 5] = vertices_start_idx + 0;

			//
			// backward
			//
			indices[indices_start_idx + 6]  = vertices_start_idx + 4;
			indices[indices_start_idx + 7]  = vertices_start_idx + 5;
			indices[indices_start_idx + 8]  = vertices_start_idx + 6;
			indices[indices_start_idx + 9]  = vertices_start_idx + 6;
			indices[indices_start_idx + 10] = vertices_start_idx + 7;
			indices[indices_start_idx + 11] = vertices_start_idx + 4;

			//
			// left
			//
			indices[indices_start_idx + 12] = vertices_start_idx + 8;
			indices[indices_start_idx + 13] = vertices_start_idx + 9;
			indices[indices_start_idx + 14] = vertices_start_idx + 10;
			indices[indices_start_idx + 15] = vertices_start_idx + 10;
			indices[indices_start_idx + 16] = vertices_start_idx + 11;
			indices[indices_start_idx + 17] = vertices_start_idx + 8;

			//
			// right
			//
			indices[indices_start_idx + 18] = vertices_start_idx + 12;
			indices[indices_start_idx + 19] = vertices_start_idx + 15;
			indices[indices_start_idx + 20] = vertices_start_idx + 14;
			indices[indices_start_idx + 21] = vertices_start_idx + 14;
			indices[indices_start_idx + 22] = vertices_start_idx + 13;
			indices[indices_start_idx + 23] = vertices_start_idx + 12;

			vertices_start_idx += GAME_ISLAND_VOXEL_VERTICES_COUNT;
			indices_start_idx += GAME_ISLAND_VOXEL_INDICES_COUNT;
			pos_x += GAME_ISLAND_TILE_VOXEL_STEP;
		}

		pos_y += GAME_ISLAND_TILE_VOXEL_STEP;
	}

	return HERO_SUCCESS;
}

HeroResult game_gfx_play_init() {
	GameGfxPlay* gfx = &game.gfx.play;
	HeroLogicalDevice* ldev = game.gfx.ldev;
	HeroResult result;

	result = game_gfx_shader_init("play_terrain", HERO_SHADER_TYPE_GRAPHICS, &gfx->terrain_tile_shader_id);
	HERO_RESULT_ASSERT(result);

	result = game_gfx_shader_init("play_model", HERO_SHADER_TYPE_GRAPHICS, &gfx->model_shader_id);
	HERO_RESULT_ASSERT(result);

	result = game_gfx_shader_init("play_billboard", HERO_SHADER_TYPE_GRAPHICS, &gfx->billboard_shader_id);
	HERO_RESULT_ASSERT(result);

	result = game_gfx_shader_init("play_voxel_raytrace", HERO_SHADER_TYPE_COMPUTE, &gfx->voxel_raytrace_shader_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroDescriptorShaderInfo shader_infos[] = {
			{
				.shader_id = gfx->voxel_raytrace_shader_id,
				.advised_pool_counts = {
					[HERO_GFX_DESCRIPTOR_SET_GLOBAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_MATERIAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_DRAW_CMD] = 32,
				},
			},
			{
				.shader_id = gfx->terrain_tile_shader_id,
				.advised_pool_counts = {
					[HERO_GFX_DESCRIPTOR_SET_GLOBAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_MATERIAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_DRAW_CMD] = 32,
				},
			},
			{
				.shader_id = gfx->model_shader_id,
				.advised_pool_counts = {
					[HERO_GFX_DESCRIPTOR_SET_GLOBAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_MATERIAL] = 32,
					[HERO_GFX_DESCRIPTOR_SET_DRAW_CMD] = 32,
				},
			},
			{
				.shader_id = gfx->billboard_shader_id,
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
		result = hero_descriptor_pool_init(ldev, &setup, &gfx->descriptor_pool_id);
		HERO_RESULT_ASSERT(result);
	}

	result = _game_gfx_play_terrain_tile_init();
	HERO_RESULT_ASSERT(result);

	result = hero_vertex_layout_register(&game_gfx_model_layout, true, &gfx->model_vertex_layout_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 24,
			.typed.vertex.layout_id = gfx->model_vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->model_vertex_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_INDEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 36,
			.typed.index_type = HERO_INDEX_TYPE_U32,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->model_index_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	result = hero_vertex_layout_register(&game_gfx_billboard_layout, true, &gfx->billboard_vertex_layout_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 24,
			.typed.vertex.layout_id = gfx->billboard_vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->billboard_vertex_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_INDEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 24,
			.typed.index_type = HERO_INDEX_TYPE_U32,
		};

		result = hero_buffer_init(ldev, &setup, &gfx->billboard_index_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_UNIFORM,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 1,
			.elmt_size = sizeof(GamePlayGlobalUBO),
		};

		result = hero_buffer_init(ldev, &setup, &gfx->uniform_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_COMPUTE | HERO_IMAGE_FLAGS_STORAGE,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = 1920,
			.height = 1088,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(ldev, &setup, &gfx->voxel_raytrace_color_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R32_SFLOAT,
			.format = HERO_IMAGE_FORMAT_R32_SFLOAT,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_COMPUTE | HERO_IMAGE_FLAGS_STORAGE,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = 1920,
			.height = 1088,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(ldev, &setup, &gfx->voxel_raytrace_depth_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_COLOR_ATTACHMENT | HERO_IMAGE_FLAGS_SAMPLED,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = 1920,
			.height = 1080,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(ldev, &setup, &gfx->attachment_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_D32,
			.format = HERO_IMAGE_FORMAT_D32,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_DEPTH_STENCIL_ATTACHMENT,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = 1920,
			.height = 1080,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(ldev, &setup, &gfx->depth_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_STORAGE,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = GAME_ISLAND_AXIS_TILES_COUNT,
			.height = GAME_ISLAND_AXIS_TILES_COUNT,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(game.gfx.ldev, &setup, &gfx->terrain_tile_tint_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageSetup setup = {
			.type = HERO_IMAGE_TYPE_2D,
			.internal_format = HERO_IMAGE_FORMAT_R32_SFLOAT,
			.format = HERO_IMAGE_FORMAT_R32_SFLOAT,
			.flags = HERO_IMAGE_FLAGS_USED_FOR_GRAPHICS | HERO_IMAGE_FLAGS_STORAGE,
			.samples = HERO_SAMPLE_COUNT_1,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.width = GAME_ISLAND_AXIS_VOXELS_COUNT,
			.height = GAME_ISLAND_AXIS_VOXELS_COUNT,
			.depth = 1,
			.mip_levels = 1,
			.array_layers_count = 1,
		};

		result = hero_image_init(game.gfx.ldev, &setup, &gfx->voxel_height_image_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = gfx->voxel_raytrace_shader_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_shader_globals_init(ldev, &setup, &gfx->voxel_raytrace_shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(ldev, gfx->voxel_raytrace_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_GLOBAL_UBO, 0, gfx->uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_image_storage(ldev, gfx->voxel_raytrace_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_COLOR_ATTACHMENT_IMAGE, 0, gfx->voxel_raytrace_color_image_id);

		result = hero_shader_globals_set_image_storage(ldev, gfx->voxel_raytrace_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_DEPTH_ATTACHMENT_IMAGE, 0, gfx->voxel_raytrace_depth_image_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(ldev, gfx->voxel_raytrace_shader_globals_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = gfx->terrain_tile_shader_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_shader_globals_init(ldev, &setup, &gfx->terrain_tile_shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(ldev, gfx->terrain_tile_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_GLOBAL_UBO, 0, gfx->uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_image_storage(ldev, gfx->terrain_tile_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_VOXEL_HEIGHT_MAP, 0, gfx->voxel_height_image_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_image_storage(ldev, gfx->terrain_tile_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_TERRAIN_TILE_TINT_MAP, 0, gfx->terrain_tile_tint_image_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(ldev, gfx->terrain_tile_shader_globals_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = gfx->model_shader_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_shader_globals_init(ldev, &setup, &gfx->model_shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(ldev, gfx->model_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_GLOBAL_UBO, 0, gfx->uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(ldev, gfx->model_shader_globals_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroShaderGlobalsSetup setup = {
			.shader_id = gfx->billboard_shader_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_shader_globals_init(ldev, &setup, &gfx->billboard_shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(ldev, gfx->billboard_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_GLOBAL_UBO, 0, gfx->uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_image_sampler(ldev, gfx->billboard_shader_globals_id, GAME_PLAY_GLOBAL_BINDING_BILLBOARD_IMAGE_ATLAS, 0, game.gfx.font_ascii_image_id, game.gfx.clamp_nearest_sampler_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(ldev, gfx->billboard_shader_globals_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroAttachmentLayout attachments[] = {
			{
				.format = HERO_IMAGE_FORMAT_R8G8B8A8_UNORM,
				.samples_count = HERO_SAMPLE_COUNT_1,
				.post_usage = HERO_ATTACHEMENT_POST_USAGE_SAMPLED,
			},
			{
				.format = HERO_IMAGE_FORMAT_D32,
				.samples_count = HERO_SAMPLE_COUNT_1,
				.post_usage = HERO_ATTACHEMENT_POST_USAGE_NONE,
			},
		};

		HeroRenderPassLayoutSetup setup = {
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
		};

		result = hero_render_pass_layout_init(ldev, &setup, &gfx->render_pass_layout_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroRenderState render_state = {0};
		render_state.topology = HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		render_state.viewports_count = 1;

		render_state.rasterization.polygon_mode = HERO_POLYGON_MODE_FILL;
		render_state.rasterization.cull_mode_flags = HERO_CULL_MODE_FLAGS_BACK;
		render_state.rasterization.front_face = HERO_FRONT_FACE_CLOCKWISE;
		render_state.rasterization.line_width = 1.f;

		render_state.multisample.rasterization_samples_count = HERO_SAMPLE_COUNT_1;

		static HeroRenderStateBlendAttachment blend_color_attachment = {0};
		blend_color_attachment.blend_enable = true;
		blend_color_attachment.color_write_mask = 0xf;
		blend_color_attachment.src_color_blend_factor = HERO_BLEND_FACTOR_SRC_ALPHA;
		blend_color_attachment.dst_color_blend_factor = HERO_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_color_attachment.color_blend_op = HERO_BLEND_OP_ADD;
		blend_color_attachment.src_alpha_blend_factor = HERO_BLEND_FACTOR_ONE;
		blend_color_attachment.dst_alpha_blend_factor = HERO_BLEND_FACTOR_ONE;
		blend_color_attachment.alpha_blend_op = HERO_BLEND_OP_ADD;

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

		HeroPipelineGraphicsSetup setup = {
			.render_state = &render_state,
			.shader_id = gfx->model_shader_id,
			.render_pass_layout_id = gfx->render_pass_layout_id,
			.vertex_layout_id = gfx->model_vertex_layout_id,
			.cache_id.raw = 0,
		};

		result = hero_pipeline_graphics_init(ldev, &setup, &gfx->model_pipeline_id);
		HERO_RESULT_ASSERT(result);

		setup.shader_id = gfx->terrain_tile_shader_id;
		setup.vertex_layout_id = gfx->terrain_tile_vertex_layout_id;
		result = hero_pipeline_graphics_init(ldev, &setup, &gfx->terrain_tile_pipeline_id);
		HERO_RESULT_ASSERT(result);

		setup.shader_id = gfx->billboard_shader_id;
		setup.vertex_layout_id = gfx->billboard_vertex_layout_id;
		result = hero_pipeline_graphics_init(ldev, &setup, &gfx->billboard_pipeline_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroMaterialSetup setup = {
			.pipeline_id = gfx->terrain_tile_pipeline_id,
			.shader_globals_id = gfx->terrain_tile_shader_globals_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_material_init(ldev, &setup, &gfx->terrain_tile_material_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroMaterialSetup setup = {
			.pipeline_id = gfx->model_pipeline_id,
			.shader_globals_id = gfx->model_shader_globals_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_material_init(ldev, &setup, &gfx->model_material_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroMaterialSetup setup = {
			.pipeline_id = gfx->billboard_pipeline_id,
			.shader_globals_id = gfx->billboard_shader_globals_id,
			.descriptor_pool_id = gfx->descriptor_pool_id,
		};

		result = hero_material_init(ldev, &setup, &gfx->billboard_material_id);
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
			{
				.load_op = HERO_ATTACHMENT_LOAD_OP_CLEAR,
				.store_op = HERO_ATTACHMENT_STORE_OP_PRESERVE,
				.stencil_load_op = HERO_ATTACHMENT_LOAD_OP_UNINITIALIZED,
				.stencil_store_op = HERO_ATTACHMENT_STORE_OP_DISCARD,
			},
		};

		HeroClearValue clear_values[] = {
			{ .color.f32 = { 1.f, 0.f, 0.f, 1.f } },
			{
				.depth = 1.f,
				.stencil = 0,
			}
		};

		HeroRenderPassSetup setup = {
			.layout_id = gfx->render_pass_layout_id,
			.draw_cmd_descriptor_pool_id = gfx->descriptor_pool_id,
			.attachment_clear_values = clear_values,
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
		};

		result = hero_render_pass_init(ldev, &setup, &gfx->render_pass_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroImageId attachments[] = {
			gfx->attachment_image_id,
			gfx->depth_image_id,
		};

		HeroFrameBufferSetup setup = {
			.attachments = attachments,
			.attachments_count = HERO_ARRAY_COUNT(attachments),
			.layers = 1,
			.render_pass_layout_id = gfx->render_pass_layout_id,
			.width = 1920,
			.height = 1080,

		};

		result = hero_frame_buffer_init(ldev, &setup, &gfx->frame_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

HeroResult game_gfx_play_deinit() {
	HERO_ABORT("unimplemented");
	return HERO_SUCCESS;
}

typedef struct GameGfxPlayTerrianPushConstants GameGfxPlayTerrianPushConstants;
struct GameGfxPlayTerrianPushConstants {
	U32 view_tile_left_start;
	U32 view_tile_bottom_start;
	U32 view_tile_width;
};

int game_render_entity_billboard_sort_fn(const void* a, const void* b) {
	const GameRenderEntityBillboard* a_ = a;
	const GameRenderEntityBillboard* b_ = b;
	if (a_->position.z < b_->position.z) {
		return 1;
	} else {
		return -1;
	}
}

HeroResult game_gfx_play_render(HeroCommandRecorder* command_recorder) {
	GameGfxPlay* gfx = &game.gfx.play;
	HeroResult result;
	/*

	if (gfx->scene_render_data.terrain_tile_tint_map) {
		// TODO support sub updates
		U32* tile_tint_map;
		HeroResult result = hero_image_write(game.gfx.ldev, gfx->terrain_tile_tint_image_id, NULL, (void**)&tile_tint_map);
		HERO_RESULT_ASSERT(result);
		HERO_COPY_ELMT_MANY(tile_tint_map, gfx->scene_render_data.terrain_tile_tint_map, GAME_ISLAND_AXIS_TILES_COUNT * GAME_ISLAND_AXIS_TILES_COUNT);
	}

	if (gfx->scene_render_data.voxel_height_map) {
		F32* voxel_height_map;
		HeroResult result = hero_image_write(game.gfx.ldev, gfx->voxel_height_image_id, NULL, (void**)&voxel_height_map);
		HERO_RESULT_ASSERT(result);
		HERO_COPY_ELMT_MANY(voxel_height_map, gfx->scene_render_data.voxel_height_map, GAME_ISLAND_AXIS_VOXELS_COUNT * GAME_ISLAND_AXIS_VOXELS_COUNT);
	}

	GamePlayGlobalUBO* global_ubo;
	result = hero_buffer_write(game.gfx.ldev, gfx->uniform_buffer_id, 0, 1, (void**)&global_ubo);
	HERO_RESULT_ASSERT(result);

	{
		Mat4x4 view_to_ndc;
		mat4x4_perspective(&view_to_ndc, HERO_DEGREES_TO_RADIANS(45.f), (float)1920 / (float)1080, 0.1f, 100.f);

		// inverse Y & Z to make +Y go up and +Z go forward
		mat4x4_scale(&view_to_ndc, VEC3_INIT(1.f, -1.f, -1.f));

		Mat4x4 world_to_view;
		mat4x4_identity(&world_to_view);

		mat4x4_mul_quat(&world_to_view, &world_to_view, quat_conj(gfx->scene_render_data.camera_rotation));

		mat4x4_translate(&world_to_view, vec3_neg(gfx->scene_render_data.camera_position));

		mat4x4_mul(&global_ubo->mvp, &world_to_view, &view_to_ndc);
	}

#if ENABLE_VOXEL_RAYTRACTER
	result = hero_cmd_compute_dispatch(command_recorder, gfx->voxel_raytrace_shader_id, gfx->voxel_raytrace_shader_globals_id, 1920 / 32, 1088 / 32, 1);
	HERO_RESULT_ASSERT(result);
#endif


	result = hero_cmd_render_pass_start(command_recorder, gfx->render_pass_id, gfx->frame_buffer_id, NULL, NULL);
	HERO_RESULT_ASSERT(result);

	U32 view_width = 38;
	U32 view_height = 28;
	F32 view_x_offset = (F32)view_width / 2.f;
	F32 view_y_offset = (F32)view_height / 2.f;
	Vec3 camera_target = vec3_sub(gfx->scene_render_data.camera_position, GAME_CAMERA_TARGET_OFFSET);
	U32 view_tile_left_start = (U32)(camera_target.x - view_x_offset);
	U32 view_tile_bottom_start = (U32)(camera_target.z - view_y_offset + 4.f);

	GameGfxPlayTerrianPushConstants pc = {
		.view_tile_left_start = view_tile_left_start,
		.view_tile_bottom_start = view_tile_bottom_start,
		.view_tile_width = view_width,
	};

	result = hero_cmd_draw_start(command_recorder, gfx->terrain_tile_material_id);
	HERO_RESULT_ASSERT(result);

	result = hero_cmd_draw_set_vertex_buffer(command_recorder, gfx->terrain_tile_vertex_buffer_id, 0, 0);
	HERO_RESULT_ASSERT(result);

	result = hero_cmd_draw_set_instances(command_recorder, 0, view_width * view_height);
	HERO_RESULT_ASSERT(result);

	result = hero_cmd_draw_set_push_constants(command_recorder, &pc, 0, sizeof(pc));
	HERO_RESULT_ASSERT(result);

	result = hero_cmd_draw_end_indexed(command_recorder, gfx->terrain_tile_index_buffer_id, 0, GAME_ISLAND_TILE_VOXELS_INDICES_COUNT, 0);
	HERO_RESULT_ASSERT(result);

	{
		U32 vertices_count = gfx->scene_render_data.entity_billboards.count * 4;
		U32 indices_count = gfx->scene_render_data.entity_billboards.count * 6;

		result = hero_buffer_reserve(game.gfx.ldev, gfx->billboard_vertex_buffer_id, vertices_count);
		HERO_RESULT_ASSERT(result);

		result = hero_buffer_reserve(game.gfx.ldev, gfx->billboard_index_buffer_id, indices_count);
		HERO_RESULT_ASSERT(result);

		GameBillboardVertex* vertices;
		result = hero_buffer_write(game.gfx.ldev, gfx->billboard_vertex_buffer_id, 0, vertices_count, (void**)&vertices);
		HERO_RESULT_ASSERT(result);

		U32* indices;
		result = hero_buffer_write(game.gfx.ldev, gfx->billboard_index_buffer_id, 0, indices_count, (void**)&indices);
		HERO_RESULT_ASSERT(result);

		Vec3 top_left;
		Vec3 top_right;
		Vec3 bottom_right;
		Vec3 bottom_left;
		{
			Vec3 camera_right = vec3_norm(vec3_cross(VEC3_UP, gfx->scene_render_data.camera_forward));
			Vec3 camera_up = vec3_norm(vec3_cross(gfx->scene_render_data.camera_forward, camera_right));

			F32 billboard_width = 1.f;
			F32 billboard_height = 1.f;

			Vec3 top = vec3_mul_scalar(camera_up, billboard_height);
			Vec3 x_offset = vec3_mul_scalar(camera_right, billboard_width);

			top_left = top;
			top_right = vec3_add(top, x_offset);
			bottom_right = x_offset;
			bottom_left = VEC3_ZERO;
		}

		U32 vertices_start_idx = 0;
		U32 indices_start_idx = 0;
		qsort(gfx->scene_render_data.entity_billboards.data, gfx->scene_render_data.entity_billboards.count, sizeof(GameRenderEntityBillboard), game_render_entity_billboard_sort_fn);
		for_range(i, 0, gfx->scene_render_data.entity_billboards.count) {
			GameRenderEntityBillboard* b = &gfx->scene_render_data.entity_billboards.data[i];

			U32 color = hero_color_init(0xff, 0x66, 0xcc, 0xff);
			//U32 color = hero_color_init(0xff, 0x66, 0xcc, 0xff);
			F32 pixel_to_norm_ratio = 1.f / 16.f;
			F32 uv_x = (F32)(b->ascii & 0xf) * pixel_to_norm_ratio;
			F32 uv_y = (F32)((b->ascii & 0xf0) >> 4) * pixel_to_norm_ratio;
			F32 uv_ex = uv_x + pixel_to_norm_ratio;
			F32 uv_ey = uv_y + pixel_to_norm_ratio;

			vertices[vertices_start_idx + 0] = (GameBillboardVertex) {
				.pos = vec3_add(top_left, b->position),
				.uv = VEC2_INIT(uv_x, uv_y),
				.color = b->color,
				.tint = b->tint,
			};

			vertices[vertices_start_idx + 1] = (GameBillboardVertex) {
				.pos = vec3_add(top_right, b->position),
				.uv = VEC2_INIT(uv_ex, uv_y),
				.color = b->color,
				.tint = b->tint,
			};

			vertices[vertices_start_idx + 2] = (GameBillboardVertex) {
				.pos = vec3_add(bottom_right, b->position),
				.uv = VEC2_INIT(uv_ex, uv_ey),
				.color = b->color,
				.tint = b->tint,
			};

			vertices[vertices_start_idx + 3] = (GameBillboardVertex) {
				.pos = vec3_add(bottom_left, b->position),
				.uv = VEC2_INIT(uv_x, uv_ey),
				.color = b->color,
				.tint = b->tint,
			};

			indices[indices_start_idx + 0] = vertices_start_idx + 0;
			indices[indices_start_idx + 1] = vertices_start_idx + 1;
			indices[indices_start_idx + 2] = vertices_start_idx + 2;
			indices[indices_start_idx + 3] = vertices_start_idx + 2;
			indices[indices_start_idx + 4] = vertices_start_idx + 3;
			indices[indices_start_idx + 5] = vertices_start_idx + 0;

			vertices_start_idx += 4;
			indices_start_idx += 6;
		}

		result = hero_cmd_draw_start(command_recorder, gfx->billboard_material_id);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_set_vertex_buffer(command_recorder, gfx->billboard_vertex_buffer_id, 0, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_draw_end_indexed(command_recorder, gfx->billboard_index_buffer_id, 0, indices_count, 0);
		HERO_RESULT_ASSERT(result);
	}

	result = hero_cmd_render_pass_end(command_recorder);
	HERO_RESULT_ASSERT(result);
	*/

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Graphics
//
//
// ===========================================

enum {
	GAME_GFX_PLAY_IMAGE_SWAPCHAIN_SIZED_RGBA8,
	GAME_GFX_PLAY_IMAGE_SWAPCHAIN_SIZED_D32,
	GAME_GFX_PLAY_IMAGE_SWAPCHAIN,

	GAME_GFX_PLAY_IMAGE_COUNT,
};

enum {
	GAME_GFX_PLAY_PASS_VOXEL_MAP_RAY_TRACER,
	GAME_GFX_PLAY_PASS_SCENE,
	GAME_GFX_PLAY_PASS_UI,

	GAME_GFX_PLAY_PASS_COUNT,
};

enum {
	GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_HEIGHT_MAP,
	GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_COLOR_OUTPUT,
	GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_DEPTH,
	GAME_GFX_PLAY_SCENE_IMAGE_COLOR_OUTPUT,
	GAME_GFX_PLAY_SCENE_IMAGE_DEPTH,
	GAME_GFX_PLAY_SCENE_ENTITY_BILLBOARDS_VERTEX_BUFFER,
	GAME_GFX_PLAY_SCENE_ENTITY_BILLBOARDS_INDEX_BUFFER,
	GAME_GFX_PLAY_SCENE_ENTITY_BILLBOARDS_RO_BUFFER,
};

HeroResult game_gfx_play_pass_voxel_map_raytracer_record(HeroCommandRecorder* command_recorder) {
	return HERO_SUCCESS;
}

HeroResult game_gfx_play_pass_scene_record(HeroCommandRecorder* command_recorder) {
	return HERO_SUCCESS;
}

void _game_gfx_init_render_graph(void) {
	HeroResult result;
	HeroRenderGraphSetup setup = {
		.resource_pool_caps = {
			[HERO_PASS_RESOURCE_TYPE_INPUT_ATTACHMENT] =         256,
			[HERO_PASS_RESOURCE_TYPE_COLOR_OUTPUT_ATTACHMENT] =  256,
			[HERO_PASS_RESOURCE_TYPE_DEPTH_STENCIL_ATTACHMENT] = 256,
			[HERO_PASS_RESOURCE_TYPE_RO_IMAGE] =                 256,
			[HERO_PASS_RESOURCE_TYPE_RW_IMAGE] =                 256,
			[HERO_PASS_RESOURCE_TYPE_RO_BUFFER] =                256,
			[HERO_PASS_RESOURCE_TYPE_RW_BUFFER] =                256,
			[HERO_PASS_RESOURCE_TYPE_VERTEX_BUFFER] =            256,
			[HERO_PASS_RESOURCE_TYPE_INDEX_BUFFER] =             256,
		},
		.passes_cap = 8,
		.viewports_cap = 8,
	};

	result = hero_render_graph_init(game.gfx.ldev, &setup, &game.gfx.render_graph_id);
	HERO_RESULT_ASSERT(result);

	HeroImageInfo image_info_swapchain_sized_rgba8;
	hero_image_info_init_swapchain_sized(&image_info_swapchain_sized_rgba8, HERO_IMAGE_FORMAT_R8G8B8A8_UNORM, game.gfx.swapchain_id);
	result = hero_image_info_register(game.gfx.ldev, 0, &image_info_swapchain_sized_rgba8, &game.gfx.image_info_swapchain_sized_rgba8_id);
	HERO_RESULT_ASSERT(result);

	HeroImageInfo image_info_swapchain_sized_d32;
	hero_image_info_init_swapchain_sized(&image_info_swapchain_sized_d32, HERO_IMAGE_FORMAT_D32, game.gfx.swapchain_id);
	result = hero_image_info_register(game.gfx.ldev, 0, &image_info_swapchain_sized_d32, &game.gfx.image_info_swapchain_sized_rgba8_id);
	HERO_RESULT_ASSERT(result);

	HeroImageInfo image_info_voxel_height_map;
	hero_image_info_init_2d(&image_info_voxel_height_map, HERO_IMAGE_FORMAT_R32_SFLOAT, 1024, 1024);
	result = hero_image_info_register(game.gfx.ldev, 0, &image_info_voxel_height_map, &game.gfx.image_info_voxel_height_map_id);
	HERO_RESULT_ASSERT(result);

}

void game_gfx_init(void) {
	HeroResult result;

	HeroGfxSysSetup hero_gfx_sys_setup = {
		.backend_type = HERO_GFX_BACKEND_TYPE_VULKAN,
		.display_manager_type = HERO_GFX_DISPLAY_MANAGER_XLIB,
		.application_name = "RLE",
		.vertex_layouts_cap = 32,
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
		.render_graphs_cap = 32,
	};

	result = hero_logical_device_init(physical_device, &setup, &game.gfx.ldev);
	HERO_RESULT_ASSERT(result);

	result = game_gfx_shader_init("ui", HERO_SHADER_TYPE_GRAPHICS, &game.gfx.ui_shader_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroDescriptorShaderInfo shader_infos[] = {
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

	{
		HeroSamplerSetup setup = {0};
		setup.mag_filter = HERO_FILTER_LINEAR,
		setup.min_filter = HERO_FILTER_LINEAR,
		setup.address_mode_u = HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		setup.address_mode_v = HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		setup.address_mode_w = HERO_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,

		result = hero_sampler_init(game.gfx.ldev, &setup, &game.gfx.clamp_linear_sampler_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroAttachmentLayout attachments[] = {
			{
				.format = HERO_SURFACE_IMAGE_FORMAT,
				.samples_count = HERO_SAMPLE_COUNT_1,
				.post_usage = HERO_ATTACHEMENT_POST_USAGE_PRESENT,
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

	/*
	{
		HeroCommandPoolSetup setup = {
			.support_static = false,
			.command_buffers_cap = 1,
		};

		result = hero_command_pool_init(game.gfx.ldev, &setup, &game.gfx.command_pool_id);
		HERO_RESULT_ASSERT(result);
	}
	*/

	result = game_gfx_island_gen_debug_init();
	HERO_RESULT_ASSERT(result);

	result = game_gfx_play_init();
	HERO_RESULT_ASSERT(result);

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

	_game_gfx_init_render_graph();
}

void _game_gfx_update_render_graph(void) {
	HeroResult result;
	HeroRenderGraph* render_graph;
	result = hero_render_graph_get(game.gfx.ldev, game.gfx.render_graph_id, &render_graph);
	HERO_RESULT_ASSERT(result);

	{
		HeroPassSetup setup = {
			.name = "voxel map raytracer",
			.userdata = NULL,
			.render_pass_layout_id.raw = 0,
			.pass_enum = GAME_GFX_PLAY_PASS_VOXEL_MAP_RAY_TRACER,
			.viewports_count = 0,
		};
		hero_render_graph_pass_start(render_graph, &setup);

		{
			hero_pass_add_ro_image(render_graph, GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_HEIGHT_MAP, game.gfx.image_info_voxel_height_map_id);
			hero_pass_and_make_persistent(render_graph);
			hero_pass_and_make_cpu_writeable(render_graph);
		}

		hero_pass_add_rw_image(render_graph, GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_COLOR_OUTPUT, game.gfx.image_info_swapchain_sized_rgba8_id);
		hero_pass_add_rw_image(render_graph, GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_DEPTH, game.gfx.image_info_swapchain_sized_d32_id);

		hero_render_graph_pass_end(render_graph);
	}

	{
		HeroPassSetup setup = {
			.name = "scene",
			.userdata = NULL,
			.render_pass_layout_id = game.gfx.play.render_pass_layout_id,
			.pass_enum = GAME_GFX_PLAY_PASS_SCENE,
			.viewports_count = 1,
		};
		hero_render_graph_pass_start(render_graph, &setup);

		hero_pass_add_color_output_attachment(render_graph, GAME_GFX_PLAY_SCENE_IMAGE_COLOR_OUTPUT, game.gfx.image_info_swapchain_sized_rgba8_id);

		hero_pass_add_depth_stencil_attachment_linked(render_graph, GAME_GFX_PLAY_SCENE_IMAGE_DEPTH, GAME_GFX_PLAY_PASS_VOXEL_MAP_RAY_TRACER, GAME_GFX_PLAY_VOXEL_RAY_TRACER_IMAGE_DEPTH);

		{
			hero_pass_add_vertex_buffer(render_graph, GAME_GFX_PLAY_SCENE_ENTITY_BILLBOARDS_VERTEX_BUFFER, 1024 * sizeof(GameBillboardVertex));
			hero_pass_and_make_cpu_writeable(render_graph);
		}

		{
			hero_pass_add_index_buffer(render_graph, GAME_GFX_PLAY_SCENE_ENTITY_BILLBOARDS_INDEX_BUFFER, 1024 * sizeof(U16));
			hero_pass_and_make_cpu_writeable(render_graph);
		}

		{
			hero_pass_add_ro_buffer(render_graph, GAME_GFX_PLAY_SCENE_ENTITY_BILLBOARDS_RO_BUFFER, sizeof(GamePlayGlobalUBO));
			hero_pass_and_make_cpu_writeable(render_graph);
		}

		hero_render_graph_pass_end(render_graph);
	}

	result = hero_ui_window_update_render_graph(game.ui_window_id, render_graph, GAME_GFX_PLAY_PASS_UI, game.gfx.image_info_swapchain_sized_rgba8_id);
	HERO_RESULT_ASSERT(result);
}

void game_gfx_render(void) {
	HeroResult result;

	{
		result = hero_ui_window_update_render_data(game.ui_window_id);
		HERO_RESULT_ASSERT(result);

		result = game_scene_update_render_data(game.scene);
		HERO_RESULT_ASSERT(result);
	}

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

	_game_gfx_update_render_graph();

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

	/*
	HeroCommandRecorder* command_recorder;
	result = hero_command_recorder_start(game.gfx.ldev, game.gfx.command_pool_id, &command_recorder);
	HERO_RESULT_ASSERT(result);

	switch (game.scene->type) {
		case GAME_SCENE_TYPE_ISLAND_GEN_DEBUG:
			result = game_gfx_island_gen_debug_render(command_recorder);
			break;
		case GAME_SCENE_TYPE_PLAY:
			result = game_gfx_play_render(command_recorder);
			break;
	}

	{
		result = hero_cmd_render_pass_start(command_recorder, game.gfx.render_pass_id, game.gfx.swapchain_frame_buffer_ids[game.gfx.swapchain_image_idx], NULL, NULL);
		HERO_RESULT_ASSERT(result);

		result = hero_ui_window_render(game.ui_window_id, game.gfx.ldev, command_recorder);
		HERO_RESULT_ASSERT(result);

		result = hero_cmd_render_pass_end(command_recorder);
		HERO_RESULT_ASSERT(result);
	}

	HeroCommandPoolBufferId command_buffer_id;
	result = hero_command_recorder_end(command_recorder, &command_buffer_id);
	HERO_RESULT_ASSERT(result);

	result = hero_logical_device_queue_transfer(game.gfx.ldev);

	result = hero_logical_device_queue_command_buffers(game.gfx.ldev, game.gfx.command_pool_id, &command_buffer_id, 1);
	HERO_RESULT_ASSERT(result);

	result = hero_logical_device_submit(game.gfx.ldev, &game.gfx.swapchain_id, 1);
	*/
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
}

int main(int argc, char** argv) {
	game_init();

	HeroTime last_frame_start_time = hero_time_now(HERO_TIME_MODE_MONOTONIC);

	F32 dt_acc_for_one_sec = 0.f;
	U32 fps = 0;
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

		HeroTime frame_start_time = hero_time_now(HERO_TIME_MODE_MONOTONIC);
		HeroDuration frame_duration = hero_time_diff(frame_start_time, last_frame_start_time);

		game.dt = hero_duration_to_f32_secs(frame_duration);
		dt_acc_for_one_sec += game.dt;
		if (dt_acc_for_one_sec >= 1.f) {
			dt_acc_for_one_sec -= 1.f;
			printf("fps = %u, ms_per_frame = %f\n", fps, 1.f / fps);
			fps = 0;
		}

		game_logic_update();

		game_gfx_render();

		last_frame_start_time = frame_start_time;
		fps += 1;
	}

	return 0;
}
