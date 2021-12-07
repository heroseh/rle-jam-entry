#version 450
#extension GL_GOOGLE_include_directive: require

#include "ui_glsl_shared.h"

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
layout (location = 1) in U32 v_aux;
layout (location = 2) in vec2 v_uv;

layout (location = 0) out smooth vec2 f_uv;
layout (location = 1) out flat U32 f_aux;
layout (location = 2) out smooth vec2 f_position;
layout (location = 3) out flat U32 f_color_idx;

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
	f_aux = v_aux;
	f_color_idx = gl_VertexIndex / 4;

	f_position = v_position;

	gl_Position = global_ubo.mvp * vec4(v_position, 0.0, 1.0);
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
layout (location = 1) in flat U32 f_aux;
layout (location = 2) in smooth vec2 f_position;
layout (location = 3) in flat U32 f_color_idx;

layout (location = 0) out vec4 out_frag_color;

// ===========================================
//
// Main
//
// ===========================================

float signed_distance_circle(vec2 coord, float radius) {
	return length(coord) - radius;
}

float signed_distance_aabb(vec2 coord, vec2 half_size, float radius) {
	vec2 diff = abs(coord) - (half_size - radius);
	return length(max(diff, 0.0)) + min(max(diff.x, diff.y), 0.0) - radius;
}

float signed_distance_op_onion(float d, float radius) {
	return abs(d) - radius;
}

void main() {
	U32 draw_type = HERO_UI_DRAW_AUX_TYPE(f_aux);
	vec4 color = hero_color_to_glsl(colors[f_color_idx / 4][f_color_idx % 4]);
	switch (draw_type) {
		case HERO_UI_DRAW_CMD_TYPE_AABB:
		case HERO_UI_DRAW_CMD_TYPE_AABB_BORDER: {
			U32 shape_idx = HERO_UI_DRAW_AUX_SHAPE_IDX(f_aux);
			Vec4 shape = aabbs[shape_idx];

			vec2 center_pos = shape.xy;
			vec2 half_size = shape.zw;
			vec2 relative_position = f_position - center_pos;

			F32 radius = f_uv.x;
			F32 border_width = f_uv.y;
			float d = signed_distance_aabb(relative_position, half_size - border_width, radius);
			if (draw_type == HERO_UI_DRAW_CMD_TYPE_AABB_BORDER) {
				d = signed_distance_op_onion(d, border_width);
			}
			if (d >= 0.0) {
				discard;
			}
			break;
		};
		case HERO_UI_DRAW_CMD_TYPE_IMAGE: {
			U32 texture_idx = HERO_UI_DRAW_AUX_TEXTURE_IDX(f_aux);
			color *= texture(u_textures[texture_idx], f_uv);
			break;
		};
		case HERO_UI_DRAW_CMD_TYPE_IMAGE_GRAYSCALE_REMAP:
			U32 texture_idx = HERO_UI_DRAW_AUX_TEXTURE_IDX(f_aux);
			float gray = texture(u_textures[texture_idx], f_uv).r;

			vec4 bg = color;
			vec4 fg = hero_color_to_glsl(HERO_UI_DRAW_AUX_COLOR_FG(f_aux));
			fg.a = 1.0; // we don't have alpha support on foreground since we only have 24 bits
			color = mix(bg, fg, gray);
			break;
		case HERO_UI_DRAW_CMD_TYPE_CIRCLE:
		case HERO_UI_DRAW_CMD_TYPE_CIRCLE_BORDER: {
			U32 shape_idx = HERO_UI_DRAW_AUX_SHAPE_IDX(f_aux);

			Vec4 group = circle_positions[shape_idx / 2];
			U32 group_offset = (shape_idx % 2) * 2;
			Vec2 center_pos = vec2(group[group_offset], group[group_offset + 1]);

			vec2 relative_position = f_position - center_pos;

			F32 radius = f_uv.x;
			float d = signed_distance_circle(relative_position, radius);
			if (draw_type == HERO_UI_DRAW_CMD_TYPE_CIRCLE_BORDER) {
				F32 border_width = f_uv.y;
				d = signed_distance_op_onion(d, border_width);
			}
			if (d >= 0.0) {
				discard;
			}
			break;
		};
	}

	out_frag_color = color;
}

#endif // FRAGMENT



