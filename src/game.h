#ifndef _RLE_H_
#define _RLE_H_

#include <deps/hero/core.h>
#include <deps/hero/window.h>
#include <deps/hero/gfx.h>

#include <deps/hero/core.c>
#include <deps/hero/window.c>
#include <deps/hero/gfx.c>

#include <deps/FastNoiseLite.h>

#define GAME_WINDOW_TITLE "RLE"
#define GAME_WINDOW_WIDTH 1280
#define GAME_WINDOW_HEIGHT 720

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

	HeroLogicalDevice* ldev;
	HeroSwapchainId swapchain_id;
	HeroFrameBufferId* swapchain_frame_buffer_ids;
	U32 swapchain_frame_buffers_count;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId render_pass_id;
	HeroMaterialId material_id;
	HeroCommandPoolId command_pool_id;
	HeroBufferId vertex_buffer_id;
	HeroBufferId index_buffer_id;
	HeroVertexLayoutId vertex_layout_id;
	HeroBufferId uniform_buffer_id;
	HeroImageId noise_image_id;
	HeroSamplerId clamp_nearest_sampler_id;
	fnl_state noise_state;
};

extern Game game;

void game_init(void);
void game_update(void);
void game_render(void);

#endif // _RLE_H_


