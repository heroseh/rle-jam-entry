#ifndef _GAME_ISLAND_GEN_DEBUG_SHARED_H_
#define _GAME_ISLAND_GEN_DEBUG_SHARED_H_

#include "shared.h"

#define GAME_ISLAND_GEN_DEBUG_GLOBAL_BINDING_GLOBAL_UBO 0
#define GAME_ISLAND_GEN_DEBUG_GLOBAL_BINDING_TILE_MAP 1

HERO_UNIFORM_BUFFER(GameIslandGenDebugGlobalUBO, HERO_GFX_DESCRIPTOR_SET_GLOBAL, GAME_ISLAND_GEN_DEBUG_GLOBAL_BINDING_GLOBAL_UBO) {
	Mat4x4 mvp;
	float deep_sea_max;
	float sea_max;
	float ground_max;
	int show_height_map;
	int show_grid;
};

#ifdef HERO_GLSL
layout(set = HERO_GFX_DESCRIPTOR_SET_GLOBAL, binding = GAME_ISLAND_GEN_DEBUG_GLOBAL_BINDING_TILE_MAP, r32ui) uniform readonly uimage2D u_tile_map;
#endif

#endif // _HERO_UI_GLSL_SHARED_H_

