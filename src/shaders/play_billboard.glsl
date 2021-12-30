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
layout (location = 1) in vec2 v_uv;
layout (location = 2) in U32 v_color;
layout (location = 3) in U32 v_tint;

layout (location = 0) out smooth vec2 f_uv;
layout (location = 1) out flat U32 f_color;
layout (location = 2) out flat U32 f_tint;

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
	f_color = v_color;
	f_tint = v_tint;
	gl_Position = mvp * vec4(v_position, 1.0);
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
layout (location = 1) in flat U32 f_color;
layout (location = 2) in flat U32 f_tint;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	float gray = texture(u_billboard_image_atlas, f_uv).r;
	vec4 color = vec4(0.0);
	if (gray == 0.0) {
		//
		// pixel thick border
		float thickness = 1.f / (14 * 16);
		float left = texture(u_billboard_image_atlas, vec2(f_uv.x - thickness, f_uv.y)).r;
		float right = texture(u_billboard_image_atlas, vec2(f_uv.x + thickness, f_uv.y)).r;
		float bottom = texture(u_billboard_image_atlas, vec2(f_uv.x, f_uv.y - thickness)).r;
		float top = texture(u_billboard_image_atlas, vec2(f_uv.x, f_uv.y + thickness)).r;
		if (left != 0.0 || right != 0.0 || bottom != 0.0 || top != 0.0) {
			color.a = 1.0;
		}
	} else {
		vec4 bg = vec4(0.0);
		vec4 fg = hero_color_to_glsl(f_color);
		color = mix(bg, fg, gray);
	}
	vec4 tint = hero_color_to_glsl(f_tint);
	out_frag_color = vec4(mix(color.rgb, tint.rgb, tint.a), color.a);
}

#endif // FRAGMENT



