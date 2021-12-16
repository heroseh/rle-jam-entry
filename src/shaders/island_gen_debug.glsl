#version 450
#extension GL_GOOGLE_include_directive: require

#include "island_gen_debug_shared.h"

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

layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_uv;

layout (location = 0) out smooth vec2 f_uv;

out gl_PerVertex {
	vec4 gl_Position;
};

// ===========================================
//
// Main
//
// ===========================================

void main() {
	f_uv = v_uv;
	gl_Position = mvp * vec4(v_position, 0.0, 1.0);
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
layout (location = 0) in smooth vec2 f_uv;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	if (show_grid == 1) {
		float grid_thickness = 4.f;
		float grid_cell_size = float(GAME_ISLAND_AXIS_TILES_COUNT) / 32.f;
		vec2 v = f_uv * float(GAME_ISLAND_AXIS_TILES_COUNT);
		v /= grid_cell_size;
		float half_grid_thickness = grid_thickness / 2.f;
		vec2 diff = (v - floor(v)) * grid_cell_size;
		if (any(greaterThan(diff, vec2(-half_grid_thickness))) && any(lessThan(diff, vec2(half_grid_thickness)))) {
			out_frag_color = vec4(0.0,0.0,0.0, 1.0);
			return;
		}
	}

	F32 height = texture(u_tile_height_map, f_uv).r;
	vec3 color;
	if (show_height_map == 1) {
		color = vec3(height);
	} else {
		if (height <= deep_sea_max) {
			vec3 low = vec3(0.0, 0.20, 0.34);
			vec3 high = vec3(0.0, 0.26, 0.38);
			color = mix(low, high, smoothstep(0.f, deep_sea_max, height));
		} else if (height <= sea_max) {
			vec3 low = vec3(0.0, 0.24, 0.42);
			vec3 high = vec3(0.0, 0.51, 0.68);
			color = mix(low, high, smoothstep(deep_sea_max, sea_max, height));
		} else if (height <= ground_max) {
			vec3 low = vec3(0.23, 0.39, 0.17);
			vec3 high = vec3(0.33, 0.49, 0.27);
			color = mix(low, high, smoothstep(sea_max, ground_max, height));
		} else {
			vec3 low = vec3(0.5);
			vec3 high = vec3(0.7);
			color = mix(low, high, smoothstep(ground_max, 1.0, height));
		}
	}

	out_frag_color = vec4(color, 1.0);
}

#endif // FRAGMENT


