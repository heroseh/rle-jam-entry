#version 450
#extension GL_GOOGLE_include_directive: require

#include "../../deps/hero/core_glsl_shared.h"

#define BASIC_BINDING_GLOBAL_UBO 0
#define BASIC_BINDING_NOISE_TEXTURE 1

HERO_UNIFORM_BUFFER(GameGlobalUBO, 0, BASIC_BINDING_GLOBAL_UBO) {
	Mat4x4 mvp;
};

layout(set = 0, binding = BASIC_BINDING_NOISE_TEXTURE) uniform sampler2D u_noise_texture;

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
	if (height < 0.45) {
		color = vec3(0.0, 0.41, 0.58);
	} else if (height > 0.80) {
		color = vec3(0.7);
	} else {
		color = vec3(0.33, 0.49, 0.27);
	}

	out_frag_color = vec4(color, 1.0);
}

#endif // FRAGMENT


