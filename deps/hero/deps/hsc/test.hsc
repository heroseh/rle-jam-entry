
vertex Vec4 billboard_shader_vertex(U32 vertex_idx, U32 instance_idx) {
	return vec4(0.f);
}

fragment Vec4 billboard_shader_fragment(Vec4 state) {
	U32 test = 4u;
	test += test + 1u;
	if (test == 9 && 1) {
		return vec4(1.f, 0.f, 0.f, 1.f);
	} else if (0) {
		return vec4(0.f, 0.f, 1.f, 1.f);
	} else if (true) {
		return vec4(1.f, 0.f, 1.f, 1.f);
	} else {
		return vec4(0.f, 1.f, 1.f, 1.f);
	}
}

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

