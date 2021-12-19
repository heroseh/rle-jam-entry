#ifndef _GAME_PLAY_SHARED_H_
#define _GAME_PLAY_SHARED_H_

#include "shared.h"

#define GAME_PLAY_GLOBAL_BINDING_GLOBAL_UBO 0
#define GAME_PLAY_GLOBAL_BINDING_VOXEL_HEIGHT_MAP 1
#define GAME_PLAY_GLOBAL_BINDING_BILLBOARD_IMAGE_ATLAS 2
#define GAME_PLAY_GLOBAL_BINDING_COLOR_ATTACHMENT_IMAGE 3
#define GAME_PLAY_GLOBAL_BINDING_DEPTH_ATTACHMENT_IMAGE 4

#define GAME_PLAY_VOXEL_FACE_TOP   0
#define GAME_PLAY_VOXEL_FACE_BACK  1
#define GAME_PLAY_VOXEL_FACE_LEFT  2
#define GAME_PLAY_VOXEL_FACE_RIGHT 3

#define GAME_PLAY_VOXEL_VERTICES_COUNT 16
#define GAME_PLAY_VOXEL_INDICES_COUNT 24
#define GAME_PLAY_TILE_AXIS_VOXELS_COUNT 2
#define GAME_PLAY_TILE_VOXELS_COUNT (GAME_PLAY_TILE_AXIS_VOXELS_COUNT * GAME_PLAY_TILE_AXIS_VOXELS_COUNT)
#define GAME_PLAY_TILE_VOXELS_VERTICES_COUNT (GAME_PLAY_TILE_VOXELS_COUNT * GAME_PLAY_VOXEL_VERTICES_COUNT)
#define GAME_PLAY_TILE_VOXELS_INDICES_COUNT (GAME_PLAY_TILE_VOXELS_COUNT * GAME_PLAY_VOXEL_INDICES_COUNT)
#define GAME_PLAY_TILE_VOXEL_STEP (1.0 / hero_cast(F32, GAME_PLAY_TILE_AXIS_VOXELS_COUNT))
#define GAME_PLAY_CELL_AXIS_VOXELS_COUNT (GAME_ISLAND_CELL_AXIS_TILES_COUNT * GAME_PLAY_TILE_AXIS_VOXELS_COUNT)

HERO_UNIFORM_BUFFER(GamePlayGlobalUBO, HERO_GFX_DESCRIPTOR_SET_GLOBAL, GAME_PLAY_GLOBAL_BINDING_GLOBAL_UBO) {
	Mat4x4 mvp;
};

#ifdef HERO_GLSL
layout(set = HERO_GFX_DESCRIPTOR_SET_GLOBAL, binding = GAME_PLAY_GLOBAL_BINDING_VOXEL_HEIGHT_MAP, r32f) uniform readonly image2D u_voxel_height_map;

layout(set = HERO_GFX_DESCRIPTOR_SET_GLOBAL, binding = GAME_PLAY_GLOBAL_BINDING_BILLBOARD_IMAGE_ATLAS) uniform sampler2D u_billboard_image_atlas;
#endif

#endif // _GAME_PLAY_SHARED_H_
