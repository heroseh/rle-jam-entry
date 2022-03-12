
typedef _Bool Bool;
typedef uint32_t U32;
typedef int32_t S32;
typedef float F32;
typedef vec2_t Vec2;
typedef vec3_t Vec3;
typedef vec4_t Vec4;

vertex Vec4 billboard_shader_vertex(U32 vertex_idx, U32 instance_idx) {
	return vec4(0.f, 0.f, 0.f, 0.f);
}

typedef enum NamedEnum TypedefNamedEnum;
enum NamedEnum {
	NAMED_ENUM_VALUE,
};

enum {
	ZERO,
	ONE,
	TWO = 2,
	THREE,
	TEN = 10,
	ELEVEN,

	MINUS_FIVE = -5,
	MINUS_FOUR,
};

S32 global = 7;

struct {
	U32 omg;
} global_struct;

typedef S32 signed_int;
typedef struct Named { S32 i[1]; } named_wrapped_signed_int;
typedef struct { F32 i[2]; } wrapped_float2;
typedef struct { F32 i[4]; } wrapped_float4;
fragment Vec4 billboard_shader_fragment(Vec4 state) {
	typedef struct Struct TypedefStruct;
	struct Struct {
		U32 a;
		struct {
			U32 k;
			S32 d;
		};
		union {
			U32 k;
			wrapped_float2 d;
		} named;
		union {
			U32 another;
			struct {
				U32 something;
				U32 something_else;
				union {
					wrapped_float2 f2[22];
					wrapped_float4 f4[11];
				};
			};
		};
	};

	struct Struct test;
	U32 nnnn;
	TypedefStruct typedef_test;
	struct Inline { S32 a; struct Struct b[3]; } testa;
	struct Inline reuse;

	struct EnumInStruct {
		enum EnumInStruct {
			VALUE_IN_STRUCT,
		} enum_in_struct;
		U32 t;
	};

	TypedefStruct board[4][4] = {
		[0][1].named.k = 7u,

		[0][2] = {
			22u,
			.named = { 88u }
		},

		[1] = {
			{
				22u
			},
			[44].something_else = 12u,
			[99].f2 = { [12].i[2] = 34.f },
			[99] = { .another = 299u }
		}
	};

	TypedefStruct st = {
		66u,
		{
			.k = 44u,
			.d = -22,
		},
		{
			77u,
			.d = { .i = { [1] = 33.f, [0] = 99.f } }
		},
		.something_else = 123u,
		.f2 = { { .i = { 123.f } }, [1] = { { 12.f } } },
		.f2[0].i[1] = 512.f
	};

	st = (TypedefStruct){ 12u };

	enum Named named_enum = NAMED_ENUM_VALUE;
	named_enum = 7;

	enum EnumInStruct enum_in_struct = VALUE_IN_STRUCT;

	struct EnumInStruct wrapped_enum_in_struct;
	wrapped_enum_in_struct.enum_in_struct = VALUE_IN_STRUCT;

	test.f2[0].i[0] = 1.f;
	test.f2[1].i[1] = 0.3f;

	F32 red = test.f4[0].i[0];
	F32 blue = test.f4[0].i[3];

	static const F32 static_red = 0.5f;

	Bool bool_ = 79798789;
	U32 uint = 0;
	S32 sint = 0;
	F32 float_ = 0;

	bool_ = uint;
	bool_ = uint;
	bool_ = sint;
	bool_ = float_;

	uint = uint;
	uint = bool_;
	uint = sint;
	uint = float_;

	sint = bool_;
	sint = uint;
	sint = float_;

	float_ = bool_;
	float_ = uint;
	float_ = sint;

	uint = uint * bool_ ? sint ? sint - 1 : sint + 1 : uint;

	return vec4(static_red, (F32)(global - 6), blue * (F32)_Alignof(struct EnumInStruct) - 0.6f, 1.f);
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

