#ifndef _RLE_H_
#define _RLE_H_

#include <deps/hero/core.h>
#include <deps/hero/window.h>
#include <deps/hero/gfx.h>
#include <deps/hero/ui.h>

#include <deps/hero/core.c>
#include <deps/hero/window.c>
#include <deps/hero/gfx.c>
#include <deps/hero/ui.c>

#include <deps/FastNoiseLite.h>

#include "shaders/island_gen_debug_shared.h"

#define GAME_WINDOW_TITLE "RLE"
#define GAME_WINDOW_WIDTH 1280
#define GAME_WINDOW_HEIGHT 720

// ===========================================
//
//
// Island Generator
//
//
// ===========================================

typedef struct GameIslandGenConfig GameIslandGenConfig;
struct GameIslandGenConfig {
	U32 seed;
	F32 frequency;
	U32 fractal_type;
	U32 octaves;
	F32 lacunarity;
	F32 deep_sea_max;
	F32 sea_max;
	F32 ground_max;
	F32 falloff_map_intensity;
};

enum {
	GAME_ISLAND_GEN_CONFIG_SIMPLE,
	GAME_ISLAND_GEN_CONFIG_NOT_SO_SIMPLE,
	GAME_ISLAND_GEN_CONFIG_NOT_SIMPLE,
	GAME_ISLAND_GEN_CONFIG_COUNT,
};

extern GameIslandGenConfig game_island_gen_configs[GAME_ISLAND_GEN_CONFIG_COUNT];

void game_island_gen(GameIslandGenConfig* config, GameTile* tile_map_out);

// ===========================================
//
//
// Scene
//
//
// ===========================================

typedef U8 GameSceneType;
enum {
	GAME_SCENE_TYPE_ISLAND_GEN_DEBUG,
};

#define HERO_STACK_ELMT_TYPE GameSceneType
#include "deps/hero/stack_gen.inl"

typedef U8 GameSceneState;
enum {
	GAME_SCENE_STATE_UNLOADED,
	GAME_SCENE_STATE_LOADING,
	GAME_SCENE_STATE_PAUSED,
	GAME_SCENE_STATE_RUNNING,
};

typedef struct GameScene GameScene;

typedef HeroResult (*GameSceneLoadFn)(GameScene* scene);
typedef HeroResult (*GameSceneUnloadFn)(GameScene* scene);
typedef HeroResult (*GameSceneUpdateFn)(GameScene* scene, GameScene** replace_with_scene_out);
typedef HeroResult (*GameSceneUpdateRenderDataFn)(GameScene* scene);

struct GameScene {
	GameSceneType               type;
	GameSceneState              state;
	GameSceneLoadFn             load_fn;
	GameSceneUnloadFn           unload_fn;
	GameSceneUpdateFn           update_fn;
	GameSceneUpdateRenderDataFn update_render_data_fn;
};

static inline HeroResult game_scene_load(GameScene* scene) {
	HERO_ASSERT(scene->state == GAME_SCENE_STATE_UNLOADED, "can only load a scene that is unloaded");
	scene->state = GAME_SCENE_STATE_LOADING;
	return scene->load_fn(scene);
}

static inline HeroResult game_scene_unload(GameScene* scene) {
	HERO_ASSERT(scene->state != GAME_SCENE_STATE_UNLOADED, "can only unload a scene that is loaded");
	HeroResult result = scene->unload_fn(scene);
	if (result < 0) {
		return result;
	}

	scene->state = GAME_SCENE_STATE_UNLOADED;
	return HERO_SUCCESS;
}

static inline HeroResult game_scene_update(GameScene* scene, GameScene** replace_with_scene_out) {
	HERO_ASSERT(scene->state != GAME_SCENE_STATE_UNLOADED, "can only update a scene that is loaded");
	return scene->update_fn(scene, replace_with_scene_out);
}

static inline HeroResult game_scene_update_render_data(GameScene* scene) {
	HERO_ASSERT(scene->state != GAME_SCENE_STATE_UNLOADED, "can only render a scene that is loaded");
	return scene->update_render_data_fn(scene);
}

// ===========================================
//
//
// Scene: Island Generator Debug
//
//
// ===========================================

typedef struct GameSceneIslandGenDebugRenderData GameSceneIslandGenDebugRenderData;
struct GameSceneIslandGenDebugRenderData {
	GameTile* tile_map;
	F32 deep_sea_max;
	F32 sea_max;
	F32 ground_max;
	bool show_grid;
	bool show_height_map;
	bool has_been_updated;
};

typedef struct GameSceneIslandGenDebug GameSceneIslandGenDebug;
struct GameSceneIslandGenDebug {
	GameScene scene;
	GameIslandGenConfig config;
	U32 config_idx;
	U32 seed;
	GameSceneIslandGenDebugRenderData render_data;
	HeroUIImageAtlasId colored_height_map_image_atlas_id;
};

HeroResult game_scene_island_gen_debug_init(GameScene** scene_out);

// ===========================================
//
//
// Logic
//
//
// ===========================================

void game_logic_init(void);
void game_logic_update(void);

// ===========================================
//
//
// Graphics: Common
//
//
// ===========================================

HeroResult game_gfx_shader_init(const char* name, HeroShaderId* id_out);
HeroResult game_gfx_swapchain_frame_buffers_reinit(HeroSwapchain* swapchain);

// ===========================================
//
//
// Graphics Scene: Island Generator Debug
//
//
// ===========================================

typedef struct GameGfxIslandGenDebug GameGfxIslandGenDebug;
struct GameGfxIslandGenDebug {
	GameSceneIslandGenDebugRenderData scene_render_data;

	HeroVertexLayoutId     vertex_layout_id;
	HeroBufferId           vertex_buffer_id;
	HeroBufferId           index_buffer_id;
	HeroBufferId           uniform_buffer_id;
	HeroShaderId           shader_id;
	HeroDescriptorPoolId   descriptor_pool_id;
	HeroShaderGlobalsId    shader_globals_id;
	HeroPipelineId         pipeline_id;
	HeroMaterialId         material_id;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId       render_pass_id;
	HeroFrameBufferId      frame_buffer_id;
	HeroImageId            attachment_image_id;
	HeroImageId            tile_map_image_id;
};

HeroResult game_gfx_island_gen_debug_init();
HeroResult game_gfx_island_gen_debug_deinit();
HeroResult game_gfx_island_gen_debug_render(HeroCommandRecorder* command_recorder);

// ===========================================
//
//
// Graphics
//
//
// ===========================================

typedef struct GameGfx GameGfx;
struct GameGfx {
	HeroLogicalDevice* ldev;
	HeroStack(GameSceneType) scene_queue;

	GameGfxIslandGenDebug island_gen_debug;

	HeroSwapchainId swapchain_id;
	HeroFrameBufferId* swapchain_frame_buffer_ids;
	U32 swapchain_frame_buffers_count;
	U32 swapchain_image_idx;
	U32 render_width;
	U32 render_height;

	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId render_pass_id;

	HeroShaderId ui_shader_id;

	HeroDescriptorPoolId descriptor_pool_id;
	HeroCommandPoolId command_pool_id;
	HeroImageId noise_image_id;
	HeroImageId font_ascii_image_id;
	HeroSamplerId clamp_nearest_sampler_id;
};

void game_gfx_init(void);
void game_gfx_render(void);

// ===========================================
//
//
// Game
//
//
// ===========================================

typedef struct Game Game;
struct Game {
	HeroWindowSys window_sys;
	HeroWindowId window_id;
	HeroUIWindowId ui_window_id;
	HeroUIImageAtlasId ui_ascii_image_atlas_id;
	GameScene* scene;

	GameGfx gfx;
};

extern Game game;

void game_init(void);

#endif // _RLE_H_


