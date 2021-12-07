#version 450
#extension GL_GOOGLE_include_directive: require

#include "../../deps/hero/core_glsl_shared.h"

#define BASIC_BINDING_GLOBAL_UBO 0
#define BASIC_BINDING_GLYPH_ATLAS 1

HERO_UNIFORM_BUFFER(GameGlobalUBO, 0, BASIC_BINDING_GLOBAL_UBO) {
	Mat4x4 mvp;
};

layout(set = 0, binding = BASIC_BINDING_GLYPH_ATLAS) uniform sampler2D u_glyph_atlas;

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
	out_frag_color = texture(u_glyph_atlas, f_uv).r;
}

#endif // FRAGMENT



