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

layout (location = 0) in vec2 v_position;

layout (location = 0) out smooth vec3 f_normal;
layout (location = 1) out flat U32 f_color;

out gl_PerVertex {
	vec4 gl_Position;
};

// ===========================================
//
// Main
//
// ===========================================

void main() {
	vec2 uv = v_position / float(GAME_ISLAND_AXIS_TILES_COUNT);
	float height = texture(u_tile_height_map, uv).r;
	vec3 position = vec3(v_position, height);

	{
		const float NORMAL_SAMPLE_STEP = 1.f / float(GAME_ISLAND_AXIS_TILES_COUNT);
		vec2 offset = vec2(NORMAL_SAMPLE_STEP, 0.0);
		float height_l = texture(u_tile_height_map, uv - offset.xy).r;
		float height_r = texture(u_tile_height_map, uv + offset.xy).r;
		float height_u = texture(u_tile_height_map, uv + offset.yx).r;
		float height_d = texture(u_tile_height_map, uv - offset.yx).r;

		f_normal = normalize(
			vec3(
				height_l - height_r,
				height_d - height_u,
				2.0
			)
		);
	}

	gl_Position = mvp * vec4(position, 1.0);
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
layout (location = 1) in flat U32 f_color;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	vec4 color = hero_color_to_glsl(f_color);

	vec3 light_dir = vec3(0.0, -1.0, 0.0);

	float NdotL = max(0.0, -dot(f_normal, light_dir));

	out_frag_color = vec4(0.0, 1.0, 0.0, 1.0) * NdotL;
}

#endif // FRAGMENT



