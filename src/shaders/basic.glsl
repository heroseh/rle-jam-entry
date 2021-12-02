#version 450
#extension GL_GOOGLE_include_directive: require

#include "../../deps/hero/core_glsl_shared.h"

#define BASIC_BINDING_GLOBAL_UBO 0

HERO_UNIFORM_BUFFER(GameGlobalUBO, 0, BASIC_BINDING_GLOBAL_UBO) {
	Mat4x4 mvp;
};

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
layout (location = 1) in vec4 v_color;

layout (location = 0) out smooth vec4 f_color;

out gl_PerVertex {
	vec4 gl_Position;
};

// ===========================================
//
// Main
//
// ===========================================

void main() {
	f_color = v_color;
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
layout (location = 0) in smooth vec4 f_color;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

void main() {
	out_frag_color = f_color;
}

#endif // FRAGMENT


