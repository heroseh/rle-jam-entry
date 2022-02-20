
vertex Vec4 billboard_shader_vertex(U32 vertex_idx, U32 instance_idx) {
	return vec4(0.f);
}

fragment Vec4 billboard_shader_fragment(Vec4 state) {
	U32 test = 0u;
	U32 testa;
	F32 rgb[3][3] = {0};
	return vec4(rgb[0][0], rgb[1][1], rgb[2][2], 1.f);
}

/*
fragment Vec4 billboard_shader_fragment(Vec4 state) {
	Vec4 color;
	color = vec4(1.f, 0.f, 0.f, 1.f);
	%0 = vec4(...);
	if (1) { BLOCK_0:
		color = vec4(1.f, 0.f, 0.f, 1.f);
		%1 = vec4(...);
	} else if (0) {BLOCK_1:
		color = vec4(0.f, 0.f, 1.f, 1.f);
		%2 = vec4(...);
	} else if (true) {
		color = vec4(1.f, 0.f, 1.f, 1.f);
		%3 = vec4(...);
	} else {
		color = vec4(0.f, 1.f, 1.f, 1.f);
	}
	%5 = PHI BLOCK_0 %1, BLOCK_1 %2

	return color;
}
*/

/*
struct Globals {
	U32 variable;
};

struct BillboardShaderState {
	Vec4 position; [[position]]
	Vec2 uv;
	Vec4 color; [[nointerp]]
};

struct BillboardResources {
	ro_buffer(Globals) globals;
	image2d(r8)        height_map;
	sampler            clamp_linear_sampler;
};

vertex BillboardShaderState billboard_shader_vertex(BillboardResources resources, U32 vertex_idx, U32 instance_idx) {

	BillboardShaderState state;
	state.position = vec4(0.f, 0.f, 0.f, 0.f);
	state.uv = vec2(0.f, 0.f);
	return state;
}

struct BillboardFragment {
	Vec4 color;
};

fragment BillboardFragment billboard_shader_fragment(BillboardResources resources, BillboardShaderState state) {
	BillboardFragment fragment;
	fragment.color = vec4(1.f);
	return fragment;
}
*/

