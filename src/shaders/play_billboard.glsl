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
	out_frag_color = vec4(1.0);
}

#endif // FRAGMENT



