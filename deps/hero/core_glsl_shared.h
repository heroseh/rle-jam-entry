#ifndef _HERO_CORE_GLSL_SHARED_H_
#define _HERO_CORE_GLSL_SHARED_H_

// ===========================================
//
//
// GLSL Interop
//
//
// ===========================================

#ifdef HERO_GLSL
#define U32 uint
#define S32 int
#define F32 float
#define hero_mod mod
#define Vec2 vec2
#define Vec3 vec3
#define Vec4 vec4
#define UVec4 uvec4
#define Mat4x4 mat4x4
#define HERO_UNIFORM_BUFFER(NAME, set_, binding_) layout(std140, set = set_, binding = binding_) uniform NAME
#define hero_cast(T, expr) (T(expr))
#define INFINITY (1.0 / 0.0)
#define HERO_INLINE
#else
#define hero_mod fmodf
#define HERO_UNIFORM_BUFFER(NAME, set_, binding_) \
	typedef struct NAME NAME; \
	struct NAME \
	/* end */

#define hero_cast(T, expr) ((T)(expr))
#define HERO_INLINE static inline
#endif // HERO_GLSL

// ===========================================
//
//
// Number Utilities
//
//
// ===========================================

HERO_INLINE U32 u32_round_to_multiple(U32 v, U32 multiple) {
	v += multiple / 2;
	U32 rem = v % multiple;
	if (v > 0.0f) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

HERO_INLINE U32 u32_round_up_to_multiple(U32 v, U32 multiple) {
	U32 rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v + multiple - rem;
	} else {
		return v - rem;
	}
}

HERO_INLINE U32 u32_round_down_to_multiple(U32 v, U32 multiple) {
	U32 rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

#ifndef HERO_GLSL // U64 is not in GLSL

HERO_INLINE U64 u64_round_to_multiple(U64 v, U64 multiple) {
	v += multiple / 2;
	U64 rem = v % multiple;
	if (v > 0.0f) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

HERO_INLINE U64 u64_round_up_to_multiple(U64 v, U64 multiple) {
	U64 rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v + multiple - rem;
	} else {
		return v - rem;
	}
}

HERO_INLINE U64 u64_round_down_to_multiple(U64 v, U64 multiple) {
	U64 rem = v % multiple;
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

#endif //  HERO_GLSL

HERO_INLINE F32 f32_round_to_multiple(F32 v, F32 multiple) {
	v += multiple * 0.5f;
	F32 rem = hero_mod(v, multiple);
	if (v > 0.0f) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

HERO_INLINE F32 f32_round_up_to_multiple(F32 v, F32 multiple) {
	F32 rem = hero_mod(v, multiple);
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v + multiple - rem;
	} else {
		return v - rem;
	}
}

HERO_INLINE F32 f32_round_down_to_multiple(F32 v, F32 multiple) {
	F32 rem = hero_mod(v, multiple);
	if (rem == 0.0) return v;
	if (v > 0.0) {
		return v - rem;
	} else {
		return v - rem - multiple;
	}
}

// ===========================================
//
//
// Color
//
//
// ===========================================

#define HERO_COLOR_CHAN_R 0
#define HERO_COLOR_CHAN_G 8
#define HERO_COLOR_CHAN_B 16
#define HERO_COLOR_CHAN_A 24

#define HeroColor U32

#define hero_color_init(r, g, b, a) \
( \
	((r & 0xff) << HERO_COLOR_CHAN_R) | \
	((g & 0xff) << HERO_COLOR_CHAN_G) | \
	((b & 0xff) << HERO_COLOR_CHAN_B) | \
	((a & 0xff) << HERO_COLOR_CHAN_A) \
)

#define hero_color_chan_get(color, chan) (((color) >> (chan)) & 0xff)
#define hero_color_chan_set(color, chan, value) (hero_color_chan_clear(color) | (v << (chan)))
#define hero_color_chan_clear(color, chan) ((color) & ~(0xff << chan))
#define hero_color_r(color) ((color >> HERO_COLOR_CHAN_R) & 0xff)
#define hero_color_g(color) ((color >> HERO_COLOR_CHAN_G) & 0xff)
#define hero_color_b(color) ((color >> HERO_COLOR_CHAN_B) & 0xff)
#define hero_color_a(color) ((color >> HERO_COLOR_CHAN_A) & 0xff)

Vec4 hero_color_to_glsl(HeroColor color) {
	Vec4 glsl;
	F32 ratio = 1.0 / 255.0;
	glsl.x = hero_cast(F32, hero_color_r(color)) * ratio;
	glsl.y = hero_cast(F32, hero_color_g(color)) * ratio;
	glsl.z = hero_cast(F32, hero_color_b(color)) * ratio;
	glsl.w = hero_cast(F32, hero_color_a(color)) * ratio;
	return glsl;
}

#endif // _HERO_CORE_GLSL_SHARED_H_

