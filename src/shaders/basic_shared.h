#ifndef _GAME_BASIC_SHARED_H_
#define _GAME_BASIC_SHARED_H_

#include "../../deps/hero/core_glsl_shared.h"

#define GAME_BASIC_GLOBAL_BINDING_GLOBAL_UBO 0
#define GAME_BASIC_GLOBAL_BINDING_NOISE_TEXTURE 1

HERO_UNIFORM_BUFFER(GameBasicGlobalUBO, HERO_GFX_DESCRIPTOR_SET_GLOBAL, GAME_BASIC_GLOBAL_BINDING_GLOBAL_UBO) {
	Mat4x4 mvp;
	float deep_sea_max;
	float sea_max;
	float ground_max;
	int show_height_map;
	int show_grid;
};

#ifdef HERO_GLSL
layout(set = HERO_GFX_DESCRIPTOR_SET_GLOBAL, binding = GAME_BASIC_GLOBAL_BINDING_NOISE_TEXTURE) uniform sampler2D u_noise_texture;
#endif

#endif // _HERO_UI_GLSL_SHARED_H_

