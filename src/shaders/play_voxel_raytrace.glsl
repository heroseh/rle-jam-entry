#version 450
#extension GL_GOOGLE_include_directive: require

#include "play_shared.h"

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(set = HERO_GFX_DESCRIPTOR_SET_GLOBAL, binding = GAME_PLAY_GLOBAL_BINDING_COLOR_ATTACHMENT_IMAGE, rgba8_snorm) uniform image2D u_color_attachment_image;
layout(set = HERO_GFX_DESCRIPTOR_SET_GLOBAL, binding = GAME_PLAY_GLOBAL_BINDING_DEPTH_ATTACHMENT_IMAGE, rgba8_snorm) uniform image2D u_depth_attachment_image;

void compute() {
	vec4 color = vec4(1.0, 1.0, 0.0, 1.0);

	ivec2 coord = ivec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	imageStore(u_color_attachment_image, coord, color);
}

