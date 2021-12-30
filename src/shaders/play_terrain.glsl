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
layout (location = 1) out smooth vec3 f_color;
layout (location = 2) out flat vec4 f_tint;

out gl_PerVertex {
	vec4 gl_Position;
};

layout (push_constant) uniform constants {
	U32 view_tile_left_start;
	U32 view_tile_bottom_start;
	U32 view_tile_width;
};

// ===========================================
//
// Main
//
// ===========================================

void main() {
	f_normal = v_normal;
	f_color = vec3(0.5, 0.5, 0.5);

	U32 tile_x = view_tile_left_start + (gl_InstanceIndex % view_tile_width);
	U32 tile_z = view_tile_bottom_start + (gl_InstanceIndex / view_tile_width);

	vec3 pos = v_position + vec3(F32(tile_x), 0.0, F32(tile_z));

	U32 voxel_idx = (gl_VertexIndex / GAME_ISLAND_VOXEL_VERTICES_COUNT);
	U32 voxel_offset_x = voxel_idx % GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;
	U32 voxel_offset_z = voxel_idx / GAME_ISLAND_TILE_AXIS_VOXELS_COUNT;
	U32 voxel_x = (tile_x * GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) + voxel_offset_x;
	U32 voxel_z = (tile_z * GAME_ISLAND_TILE_AXIS_VOXELS_COUNT) + voxel_offset_z;
	F32 y = imageLoad(u_voxel_height_map, ivec2(voxel_x, voxel_z)).r;
	f_tint = imageLoad(u_terrain_tile_tint_map, ivec2(tile_x, tile_z));

	if (f_tint.a > 0.0 && (voxel_offset_x == 2 || voxel_offset_z == 2)) {
		f_tint.rgb *= 0.6;
		f_tint.a = 1.0;
	}

	if (pos.y != 0.0) {
		pos.y = y;
	} else {
		pos.y = y - 4.f; // TODO this will cause a the bottom half of the voxel to be missing if the neighbouring has a difference > 4.f
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
layout (location = 1) in smooth vec3 f_color;
layout (location = 2) in flat vec4 f_tint;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	vec3 color = f_color;
	vec3 normal = normalize(f_normal);

	float brightness = 1.0;
	if (abs(normal.x) > abs(normal.y) && abs(normal.x) > abs(normal.z)) {
		brightness = 0.3;
	} else if (abs(normal.z) > abs(normal.y) && abs(normal.z) > abs(normal.x)) {
		brightness = 0.15;
	}

	out_frag_color = vec4(mix(color, f_tint.rgb, f_tint.a) * brightness, 1.0);
}

#endif // FRAGMENT



