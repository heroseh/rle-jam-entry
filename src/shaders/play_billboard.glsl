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

layout (location = 0) out smooth vec2 f_uv;
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
	f_uv = v_uv;
	f_color = v_color;
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

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	float gray = texture(u_billboard_image_atlas, f_uv).r;

	vec4 bg = vec4(0.0);
	vec4 fg = hero_color_to_glsl(f_color);
	out_frag_color = mix(bg, fg, gray);
}

#endif // FRAGMENT



