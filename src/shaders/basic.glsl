#version 450
#extension GL_GOOGLE_include_directive: require

#include "basic_shared.h"

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
	float height = texture(u_noise_texture, f_uv).r;
	vec3 color;
	if (show_falloff_map == 1) {
		color = vec3(height);
	} else {
		if (height <= sea_max) {
			color = vec3(0.0, 0.41, 0.58);
		} else if (height >= mountain_min) {
			color = vec3(0.7);
		} else {
			color = vec3(0.33, 0.49, 0.27);
		}
	}

	out_frag_color = vec4(color, 1.0);
}

#endif // FRAGMENT


