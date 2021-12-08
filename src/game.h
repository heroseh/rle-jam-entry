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

#include "shaders/basic_shared.h"

#define GAME_WINDOW_TITLE "RLE"
#define GAME_WINDOW_WIDTH 1280
#define GAME_WINDOW_HEIGHT 720

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

typedef struct GameGfx GameGfx;
struct GameGfx {
	HeroLogicalDevice* ldev;

	HeroSwapchainId swapchain_id;
	HeroFrameBufferId* swapchain_frame_buffer_ids;
	U32 swapchain_frame_buffers_count;
	U32 swapchain_image_idx;
	U32 render_width;
	U32 render_height;

	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId render_pass_id;

	HeroBufferId vertex_buffer_id;
	HeroBufferId index_buffer_id;
	HeroVertexLayoutId vertex_layout_id;
	HeroBufferId uniform_buffer_id;
	HeroShaderId ui_shader_id;

	HeroDescriptorPoolId descriptor_pool_id;
	HeroMaterialId material_id;
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

	GameGfx gfx;
	fnl_state noise_state;
	GameBasicGlobalUBO global_ubo;
	F32 falloff_map_intensity;
};

extern Game game;

void game_init(void);

#endif // _RLE_H_


