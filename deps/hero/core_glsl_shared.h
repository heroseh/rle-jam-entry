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
#define HERO_UNIFORM_BUFFER_END(name) } name
#define hero_cast(T, expr) (T(expr))
#define INFINITY (1.0 / 0.0)
#define HERO_INLINE
#else
#define hero_mod fmodf
#define HERO_UNIFORM_BUFFER(NAME, set_, binding_) \
	typedef struct NAME NAME; \
	struct NAME \
	/* end */
#define HERO_UNIFORM_BUFFER_END(name) }

#define hero_cast(T, expr) ((T)(expr))
#define HERO_INLINE static inline
#endif // HERO_GLSL

#define HERO_GFX_DESCRIPTOR_SET_GLOBAL   0
#define HERO_GFX_DESCRIPTOR_SET_MATERIAL 1
#define HERO_GFX_DESCRIPTOR_SET_DRAW_CMD 2
#define HERO_GFX_DESCRIPTOR_SET_COUNT    3

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
// Bitfields
//
//
// ===========================================

#ifndef HERO_GLSL
HERO_INLINE U8 hero_bitfield_extract_u8(U8 bits, U32 bit_shift, U32 bits_size) {
	U8 mask = (hero_cast(U8, 1) << bits_size) - 1;
	return (bits >> bit_shift) & mask;
}

HERO_INLINE U16 hero_bitfield_extract_u16(U16 bits, U32 bit_shift, U32 bits_size) {
	U16 mask = (hero_cast(U16, 1) << bits_size) - 1;
	return (bits >> bit_shift) & mask;
}

HERO_INLINE U64 hero_bitfield_extract_u64(U64 bits, U32 bit_shift, U32 bits_size) {
	U64 mask = (hero_cast(U64, 1) << bits_size) - 1;
	return (bits >> bit_shift) & mask;
}
#endif

HERO_INLINE U32 hero_bitfield_extract_u32(U32 bits, U32 bit_shift, U32 bits_size) {
	U32 mask = (hero_cast(U32, 1) << bits_size) - 1;
	return (bits >> bit_shift) & mask;
}

#ifndef HERO_GLSL
HERO_INLINE S8 hero_bitfield_extract_s8(U8 bits, U32 bit_shift, U32 bits_size) {
	U8 mask = (hero_cast(U8, 1) << bits_size) - 1;
	U8 sign_mask = hero_cast(U8, 1) << (bits_size - 1);
	U8 value = (bits >> bit_shift) & mask;
	return (value ^ sign_mask) - sign_mask;
}

HERO_INLINE S16 hero_bitfield_extract_s16(U16 bits, U32 bit_shift, U32 bits_size) {
	U16 mask = (hero_cast(U16, 1) << bits_size) - 1;
	U16 sign_mask = hero_cast(U16, 1) << (bits_size - 1);
	U16 value = (bits >> bit_shift) & mask;
	return (value ^ sign_mask) - sign_mask;
}
#endif

HERO_INLINE S32 hero_bitfield_extract_s32(U32 bits, U32 bit_shift, U32 bits_size) {
	U32 mask = (hero_cast(U32, 1) << bits_size) - 1;
	U32 sign_mask = hero_cast(U32, 1) << (bits_size - 1);
	U32 value = (bits >> bit_shift) & mask;
	return hero_cast(int, (value ^ sign_mask) - sign_mask);
}

#ifndef HERO_GLSL
HERO_INLINE S64 hero_bitfield_extract_s64(U64 bits, U32 bit_shift, U32 bits_size) {
	U64 mask = (hero_cast(U64, 1) << bits_size) - 1;
	U64 sign_mask = hero_cast(U64, 1) << (bits_size - 1);
	U64 value = (bits >> bit_shift) & mask;
	return (value ^ sign_mask) - sign_mask;
}

HERO_INLINE void hero_bitfield_insert_u8(U8* bits, U8 value, U32 bit_shift, U32 bits_size) {
	U8 mask = (hero_cast(U8, 1) << bits_size) - 1;
	*bits &= ~(mask << bit_shift); // clear the value
	*bits |= ((value & mask) << bit_shift); // set the value
}

HERO_INLINE void hero_bitfield_insert_u16(U16* bits, U16 value, U32 bit_shift, U32 bits_size) {
	U16 mask = (hero_cast(U16, 1) << bits_size) - 1;
	*bits &= ~(mask << bit_shift); // clear the value
	*bits |= ((value & mask) << bit_shift); // set the value
}

HERO_INLINE void hero_bitfield_insert_u32(U32* bits, U32 value, U32 bit_shift, U32 bits_size) {
	U32 mask = (hero_cast(U32, 1) << bits_size) - 1;
	*bits &= ~(mask << bit_shift); // clear the value
	*bits |= ((value & mask) << bit_shift); // set the value
}

HERO_INLINE void hero_bitfield_insert_u64(U64* bits, U64 value, U32 bit_shift, U32 bits_size) {
	U64 mask = (hero_cast(U64, 1) << bits_size) - 1;
	*bits &= ~(mask << bit_shift); // clear the value
	*bits |= ((value & mask) << bit_shift); // set the value
}

HERO_INLINE void hero_bitfield_insert_s8(U8* bits, S8 value, U32 bit_shift, U32 bits_size) {
	U8 sign_mask = hero_cast(U8, 1) << (sizeof(U8) - 1);
	U8 shifted_sign = (value & sign_mask) >> (sizeof(U8) - (bit_shift + bits_size) - 1);
	U8 value_ = value | shifted_sign;
	hero_bitfield_insert_u8(bits, value_, bit_shift, bits_size);
}

HERO_INLINE void hero_bitfield_insert_s16(U16* bits, S16 value, U32 bit_shift, U32 bits_size) {
	U16 sign_mask = hero_cast(U16, 1) << (sizeof(U16) - 1);
	U16 shifted_sign = (value & sign_mask) >> (sizeof(U16) - (bit_shift + bits_size) - 1);
	U16 value_ = value | shifted_sign;
	hero_bitfield_insert_u16(bits, value_, bit_shift, bits_size);
}

HERO_INLINE void hero_bitfield_insert_s32(U32* bits, S32 value, U32 bit_shift, U32 bits_size) {
	U32 sign_mask = hero_cast(U32, 1) << (sizeof(U32) - 1);
	U32 shifted_sign = (value & sign_mask) >> (sizeof(U32) - (bit_shift + bits_size) - 1);
	U32 value_ = value | shifted_sign;
	hero_bitfield_insert_u32(bits, value_, bit_shift, bits_size);
}

HERO_INLINE void hero_bitfield_insert_s64(U64* bits, S64 value, U32 bit_shift, U32 bits_size) {
	U64 sign_mask = hero_cast(U64, 1) << (sizeof(U64) - 1);
	U64 shifted_sign = (value & sign_mask) >> (sizeof(U64) - (bit_shift + bits_size) - 1);
	U64 value_ = value | shifted_sign;
	hero_bitfield_insert_u64(bits, value_, bit_shift, bits_size);
}
#endif

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
#define hero_color_r(color) (((color) >> HERO_COLOR_CHAN_R) & 0xff)
#define hero_color_g(color) (((color) >> HERO_COLOR_CHAN_G) & 0xff)
#define hero_color_b(color) (((color) >> HERO_COLOR_CHAN_B) & 0xff)
#define hero_color_a(color) (((color) >> HERO_COLOR_CHAN_A) & 0xff)

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

