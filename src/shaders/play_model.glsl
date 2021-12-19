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
	//gl_Position = mvp; * vec4(position, 1.0);
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



