#version 450
#extension GL_GOOGLE_include_directive: require

#include "play_shared.h"

// ===========================================
//
//
// Vertex
//
//
// ===========================================
#ifdef VERTEX

// ===========================================
//
// Inputs & Outputs
//
// ===========================================

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;

layout (location = 0) out smooth vec3 f_normal;
layout (location = 1) out smooth vec4 f_color;

out gl_PerVertex {
	vec4 gl_Position;
};

// ===========================================
//
// Main
//
// ===========================================

void main() {
	f_normal = v_normal;
	f_color = vec4(0.5, 0.5, 0.5, 1.0);

	U32 tile_offset_x = gl_InstanceIndex % GAME_ISLAND_CELL_AXIS_TILES_COUNT;
	U32 tile_offset_z = gl_InstanceIndex / GAME_ISLAND_CELL_AXIS_TILES_COUNT;

	vec3 pos = v_position + vec3(F32(tile_offset_x), 0.0, F32(tile_offset_z));

	if (pos.y != 0.0) {
		U32 voxel_idx = (gl_VertexIndex / GAME_PLAY_VOXEL_VERTICES_COUNT);
		U32 voxel_offset_x = (tile_offset_x * GAME_PLAY_TILE_AXIS_VOXELS_COUNT) + (voxel_idx % GAME_PLAY_TILE_AXIS_VOXELS_COUNT);
		U32 voxel_offset_z = (tile_offset_z * GAME_PLAY_TILE_AXIS_VOXELS_COUNT) + (voxel_idx / GAME_PLAY_TILE_AXIS_VOXELS_COUNT);
		pos.y = imageLoad(u_voxel_height_map, ivec2(voxel_offset_x, voxel_offset_z)).r;
	}

	gl_Position = mvp * vec4(pos, 1.0);
}

#endif // VERTEX

// ===========================================
//
//
// Fragment
//
//
// ===========================================
#ifdef FRAGMENT

// ===========================================
//
// Inputs & Outputs
//
// ===========================================
layout (location = 0) in smooth vec3 f_normal;
layout (location = 1) in smooth vec4 f_color;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	vec4 color = f_color;
	vec3 normal = normalize(f_normal);

	vec3 light_dir = vec3(0.0, -1.0, 0.0);

	float NdotL = dot(normal, light_dir);
	float intensity = max(0.0, -NdotL);

	out_frag_color = vec4(color.rgb * intensity, 1.0);
}

#endif // FRAGMENT



