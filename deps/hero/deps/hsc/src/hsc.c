#include "hsc.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// ===========================================
//
//
// General
//
//
// ===========================================

//
// the padded zero bytes at the end of the file we add so we can look ahead in the tokenizer comparisions
#define _HSC_TOKENIZER_LOOK_HEAD_SIZE 4

#define _HSC_TOKENIZER_NESTED_BRACKETS_CAP 32

void _hsc_assert_failed(const char* cond, const char* file, int line, const char* message, ...) {
	fprintf(stderr, "assertion failed: %s\nmessage: ", cond);

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

HSC_NORETURN void _hsc_abort(const char* file, int line, const char* message, ...) {
	fprintf(stderr, "abort: ");

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

void hsc_hash_table_init(HscHashTable* hash_table) {
	hash_table->keys = HSC_ALLOC_ARRAY(U32, 1024);
	HSC_ASSERT(hash_table->keys, "out of memory");
	HSC_ZERO_ELMT_MANY(hash_table->keys, 1024);
	hash_table->values = HSC_ALLOC_ARRAY(U32, 1024);
	HSC_ASSERT(hash_table->values, "out of memory");
	hash_table->count = 0;
	hash_table->cap = 1024;
}

bool hsc_hash_table_find(HscHashTable* hash_table, U32 key, U32* value_out) {
	for (uint32_t idx = 0; idx < hash_table->count; idx += 1) {
		U32 found_key = hash_table->keys[idx];
		if (found_key == key) {
			*value_out = hash_table->values[idx];
			return true;
		}
	}
	return false;
}

bool hsc_hash_table_find_or_insert(HscHashTable* hash_table, U32 key, U32** value_ptr_out) {
	for (uint32_t idx = 0; idx < hash_table->count; idx += 1) {
		U32 found_key = hash_table->keys[idx];
		if (found_key == key) {
			*value_ptr_out = &hash_table->values[idx];
			return true;
		}
	}

	hash_table->keys[hash_table->count] = key;
	*value_ptr_out = &hash_table->values[hash_table->count];
	HSC_ASSERT(hash_table->count < hash_table->cap, "hash table full");
	hash_table->count += 1;
	return false;
}

// ===========================================
//
//
// Syntax Generator
//
//
// ===========================================

char* hsc_token_strings[HSC_TOKEN_COUNT] = {
	[HSC_DATA_TYPE_VOID] = "void",
	[HSC_DATA_TYPE_BOOL] = "Bool",
	[HSC_DATA_TYPE_U8] = "U8",
	[HSC_DATA_TYPE_U16] = "U16",
	[HSC_DATA_TYPE_U32] = "U32",
	[HSC_DATA_TYPE_U64] = "U64",
	[HSC_DATA_TYPE_S8] = "S8",
	[HSC_DATA_TYPE_S16] = "S16",
	[HSC_DATA_TYPE_S32] = "S32",
	[HSC_DATA_TYPE_S64] = "S64",
	[HSC_DATA_TYPE_F16] = "F16",
	[HSC_DATA_TYPE_F32] = "F32",
	[HSC_DATA_TYPE_F64] = "F64",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC2] = "Vec2",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC3] = "Vec3",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC4] = "Vec4",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X2] = "Mat2x2",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X3] = "Mat2x3",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X4] = "Mat2x4",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X2] = "Mat3x2",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X3] = "Mat3x3",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X4] = "Mat3x4",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X2] = "Mat4x2",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X3] = "Mat4x3",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X4] = "Mat4x4",
	[HSC_TOKEN_EOF] = "end of file",
	[HSC_TOKEN_IDENT] = "identifier",
	[HSC_TOKEN_CURLY_OPEN] = "{",
	[HSC_TOKEN_CURLY_CLOSE] = "}",
	[HSC_TOKEN_PARENTHESIS_OPEN] = "(",
	[HSC_TOKEN_PARENTHESIS_CLOSE] = ")",
	[HSC_TOKEN_FULL_STOP] = ".",
	[HSC_TOKEN_COMMA] = ",",
	[HSC_TOKEN_SEMICOLON] = ";",
	[HSC_TOKEN_COLON] = ":",
	[HSC_TOKEN_PLUS] = "+",
	[HSC_TOKEN_MINUS] = "-",
	[HSC_TOKEN_FORWARD_SLASH] = "/",
	[HSC_TOKEN_ASTERISK] = "*",
	[HSC_TOKEN_PERCENT] = "%",
	[HSC_TOKEN_AMPERSAND] = "&",
	[HSC_TOKEN_PIPE] = "|",
	[HSC_TOKEN_CARET] = "^",
	[HSC_TOKEN_EXCLAMATION_MARK] = "!",
	[HSC_TOKEN_TILDE] = "~",
	[HSC_TOKEN_EQUAL] = "=",
	[HSC_TOKEN_LESS_THAN] = "<",
	[HSC_TOKEN_GREATER_THAN] = ">",
	[HSC_TOKEN_LOGICAL_AND] = "&&",
	[HSC_TOKEN_LOGICAL_OR] = "||",
	[HSC_TOKEN_LOGICAL_EQUAL] = "==",
	[HSC_TOKEN_LOGICAL_NOT_EQUAL] = "!=",
	[HSC_TOKEN_LESS_THAN_OR_EQUAL] = "<=",
	[HSC_TOKEN_GREATER_THAN_OR_EQUAL] = ">=",
	[HSC_TOKEN_BIT_SHIFT_LEFT] = "<<",
	[HSC_TOKEN_BIT_SHIFT_RIGHT] = ">>",
	[HSC_TOKEN_ADD_ASSIGN] = "+=",
	[HSC_TOKEN_SUBTRACT_ASSIGN] = "-=",
	[HSC_TOKEN_MULTIPLY_ASSIGN] = "*=",
	[HSC_TOKEN_DIVIDE_ASSIGN] = "/=",
	[HSC_TOKEN_MODULO_ASSIGN] = "%=",
	[HSC_TOKEN_BIT_SHIFT_LEFT_ASSIGN] = "<<=",
	[HSC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN] = ">>=",
	[HSC_TOKEN_BIT_AND_ASSIGN] = "&=",
	[HSC_TOKEN_BIT_XOR_ASSIGN] = "^=",
	[HSC_TOKEN_BIT_OR_ASSIGN] = "|=",
	[HSC_TOKEN_LIT_U32] = "U32",
	[HSC_TOKEN_LIT_U64] = "U64",
	[HSC_TOKEN_LIT_S32] = "S32",
	[HSC_TOKEN_LIT_S64] = "S64",
	[HSC_TOKEN_LIT_F32] = "F32",
	[HSC_TOKEN_LIT_F64] = "F64",
	[HSC_TOKEN_KEYWORD_RETURN] = "return",
	[HSC_TOKEN_KEYWORD_IF] = "if",
	[HSC_TOKEN_KEYWORD_ELSE] = "else",
	[HSC_TOKEN_KEYWORD_WHILE] = "while",
	[HSC_TOKEN_KEYWORD_FOR] = "for",
	[HSC_TOKEN_KEYWORD_SWITCH] = "switch",
	[HSC_TOKEN_KEYWORD_CASE] = "case",
	[HSC_TOKEN_KEYWORD_DEFAULT] = "default",
	[HSC_TOKEN_KEYWORD_BREAK] = "break",
	[HSC_TOKEN_KEYWORD_CONTINUE] = "continue",
	[HSC_TOKEN_KEYWORD_TRUE] = "true",
	[HSC_TOKEN_KEYWORD_FALSE] = "false",
	[HSC_TOKEN_KEYWORD_VERTEX] = "vertex",
	[HSC_TOKEN_KEYWORD_FRAGMENT] = "fragment",
	[HSC_TOKEN_KEYWORD_GEOMETRY] = "geometry",
	[HSC_TOKEN_KEYWORD_TESSELLATION] = "tessellation",
	[HSC_TOKEN_KEYWORD_COMPUTE] = "compute",
	[HSC_TOKEN_KEYWORD_MESHTASK] = "meshtask",
	[HSC_TOKEN_KEYWORD_STRUCT] = "struct",
	[HSC_TOKEN_KEYWORD_RO_BUFFER] = "ro_buffer",
	[HSC_TOKEN_KEYWORD_RW_BUFFER] = "rw_buffer",
	[HSC_TOKEN_KEYWORD_RO_IMAGE1D] = "ro_image1d",
	[HSC_TOKEN_KEYWORD_RW_IMAGE1D] = "rw_image1d",
	[HSC_TOKEN_KEYWORD_RO_IMAGE2D] = "ro_image2d",
	[HSC_TOKEN_KEYWORD_RW_IMAGE2D] = "rw_image2d",
	[HSC_TOKEN_KEYWORD_RO_IMAGE3D] = "ro_image3d",
	[HSC_TOKEN_KEYWORD_RW_IMAGE3D] = "rw_image3d",
};

char* hsc_string_intrinsic_param_names[HSC_STRING_ID_INTRINSIC_PARAM_NAMES_END] = {
	[HSC_STRING_ID_GENERIC_SCALAR] = "GScalar",
	[HSC_STRING_ID_GENERIC_VEC2] = "GVec2",
	[HSC_STRING_ID_GENERIC_VEC3] = "GVec3",
	[HSC_STRING_ID_GENERIC_VEC4] = "GVec4",
	[HSC_STRING_ID_SCALAR] = "scalar",
	[HSC_STRING_ID_X] = "x",
	[HSC_STRING_ID_Y] = "y",
	[HSC_STRING_ID_Z] = "z",
	[HSC_STRING_ID_W] = "w",
};

char* hsc_function_shader_stage_strings[HSC_FUNCTION_SHADER_STAGE_COUNT] = {
	[HSC_FUNCTION_SHADER_STAGE_NONE] = "none",
	[HSC_FUNCTION_SHADER_STAGE_VERTEX] = "vertex",
	[HSC_FUNCTION_SHADER_STAGE_FRAGMENT] = "fragment",
	[HSC_FUNCTION_SHADER_STAGE_GEOMETRY] = "geometry",
	[HSC_FUNCTION_SHADER_STAGE_TESSELLATION] = "tessellation",
	[HSC_FUNCTION_SHADER_STAGE_COMPUTE] = "compute",
	[HSC_FUNCTION_SHADER_STAGE_MESHTASK] = "meshtask",
};

U32 hsc_intrinsic_function_overloads_count[HSC_FUNCTION_ID_INTRINSIC_END] = {
	[HSC_FUNCTION_ID_VEC2_SINGLE] = 2,
	[HSC_FUNCTION_ID_VEC2_MULTI] = 0,

	[HSC_FUNCTION_ID_VEC3_SINGLE] = 2,
	[HSC_FUNCTION_ID_VEC3_MULTI] = 0,

	[HSC_FUNCTION_ID_VEC4_SINGLE] = 2,
	[HSC_FUNCTION_ID_VEC4_MULTI] = 0,

	[HSC_FUNCTION_ID_MAT2x2] = 1,
	[HSC_FUNCTION_ID_MAT2x3] = 1,
	[HSC_FUNCTION_ID_MAT2x4] = 1,
	[HSC_FUNCTION_ID_MAT3x2] = 1,
	[HSC_FUNCTION_ID_MAT3x3] = 1,
	[HSC_FUNCTION_ID_MAT3x4] = 1,
	[HSC_FUNCTION_ID_MAT4x2] = 1,
	[HSC_FUNCTION_ID_MAT4x3] = 1,
	[HSC_FUNCTION_ID_MAT4x4] = 1,
};

HscIntrinsicFunction hsc_intrinsic_functions[HSC_FUNCTION_ID_INTRINSIC_END] = {
	[HSC_FUNCTION_ID_VEC2_SINGLE] = {
		.name = "vec2",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC2,
		.params_count = 1,
		.params = {
			{ .identifier_string_id = HSC_STRING_ID_SCALAR, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_ID_VEC2_MULTI] = {
		.name = "vec2",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC2,
		.params_count = 2,
		.params = {
			{ .identifier_string_id = HSC_STRING_ID_X, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = HSC_STRING_ID_Y, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_ID_VEC3_SINGLE] = {
		.name = "vec3",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC3,
		.params_count = 1,
		.params = {
			{ .identifier_string_id = HSC_STRING_ID_SCALAR, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_ID_VEC3_MULTI] = {
		.name = "vec3",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC3,
		.params_count = 3,
		.params = {
			{ .identifier_string_id = HSC_STRING_ID_X, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = HSC_STRING_ID_Y, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = HSC_STRING_ID_Z, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_ID_VEC4_SINGLE] = {
		.name = "vec4",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC4,
		.params_count = 1,
		.params = {
			{ .identifier_string_id = HSC_STRING_ID_SCALAR, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_ID_VEC4_MULTI] = {
		.name = "vec4",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC4,
		.params_count = 4,
		.params = {
			{ .identifier_string_id = HSC_STRING_ID_X, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = HSC_STRING_ID_Y, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = HSC_STRING_ID_Z, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = HSC_STRING_ID_W, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
};

bool hsc_opt_is_enabled(HscOpts* opts, HscOpt opt) {
	U64 bit = ((U64)1 << (opt % 64));
	return (opts->bitset[opt / 64] & bit) == bit;
}

void hsc_opt_set_enabled(HscOpts* opts, HscOpt opt) {
	U64 bit = (U64)1 << (opt % 64);
	opts->bitset[opt / 64] |= bit;
}

HscString hsc_data_type_string(HscAstGen* astgen, HscDataType data_type) {
	HscStringId string_id;
	if (data_type < HSC_DATA_TYPE_BASIC_END) {
		string_id.idx_plus_one = HSC_STRING_ID_INTRINSIC_TYPES_START + data_type;
	} else if (HSC_DATA_TYPE_VEC2_START <= data_type && data_type < HSC_DATA_TYPE_VEC2_END) {
		string_id.idx_plus_one = HSC_STRING_ID_INTRINSIC_TYPES_START + (HSC_TOKEN_INTRINSIC_TYPE_VEC2 - HSC_TOKEN_INTRINSIC_TYPES_START);
	} else if (HSC_DATA_TYPE_VEC3_START <= data_type && data_type < HSC_DATA_TYPE_VEC3_END) {
		string_id.idx_plus_one = HSC_STRING_ID_INTRINSIC_TYPES_START + (HSC_TOKEN_INTRINSIC_TYPE_VEC3 - HSC_TOKEN_INTRINSIC_TYPES_START);
	} else if (HSC_DATA_TYPE_VEC4_START <= data_type && data_type < HSC_DATA_TYPE_VEC4_END) {
		string_id.idx_plus_one = HSC_STRING_ID_INTRINSIC_TYPES_START + (HSC_TOKEN_INTRINSIC_TYPE_VEC4 - HSC_TOKEN_INTRINSIC_TYPES_START);
	} else if (HSC_DATA_TYPE_GENERIC_SCALAR <= data_type && data_type <= HSC_DATA_TYPE_GENERIC_VEC4) {
		string_id.idx_plus_one = HSC_STRING_ID_GENERIC_SCALAR + (data_type - HSC_DATA_TYPE_GENERIC_SCALAR);
	} else {
		HSC_ABORT("unhandle type '%u'", data_type);
	}

	return hsc_string_table_get(&astgen->string_table, string_id);
}

void hsc_data_type_size_align(HscAstGen* astgen, HscDataType data_type, Uptr* size_out, Uptr* align_out) {
	if (data_type < HSC_DATA_TYPE_MATRIX_END) {
		switch (HSC_DATA_TYPE_SCALAR(data_type)) {
			case HSC_DATA_TYPE_VOID:
				*size_out = 0;
				*align_out = 0;
				break;
			case HSC_DATA_TYPE_BOOL:
			case HSC_DATA_TYPE_U8:
			case HSC_DATA_TYPE_S8:
				*size_out = 1;
				*align_out = 1;
				break;
			case HSC_DATA_TYPE_U16:
			case HSC_DATA_TYPE_S16:
			case HSC_DATA_TYPE_F16:
				*size_out = 2;
				*align_out = 2;
				break;
			case HSC_DATA_TYPE_U32:
			case HSC_DATA_TYPE_S32:
			case HSC_DATA_TYPE_F32:
				*size_out = 4;
				*align_out = 4;
				break;
			case HSC_DATA_TYPE_U64:
			case HSC_DATA_TYPE_S64:
			case HSC_DATA_TYPE_F64:
				*size_out = 8;
				*align_out = 8;
				break;
		}

		if (data_type >= HSC_DATA_TYPE_VECTOR_START) {
			if (data_type < HSC_DATA_TYPE_VECTOR_END) {
				U32 componments_count = HSC_DATA_TYPE_VECTOR_COMPONENTS(data_type);
				*size_out *= componments_count;
				*align_out *= componments_count;
			} else {
				U32 rows_count = HSC_DATA_TYPE_MATRX_ROWS(data_type);
				U32 columns_count = HSC_DATA_TYPE_MATRX_COLUMNS(data_type);
				*size_out *= rows_count * columns_count;
				*align_out *= rows_count * columns_count;
			}
		}
	} else {
		HSC_ABORT("unhandle type '%u'", data_type);
	}
}

HscDataType hsc_data_type_resolve_generic(HscAstGen* astgen, HscDataType data_type) {
	switch (data_type) {
		case HSC_DATA_TYPE_GENERIC_SCALAR:
			HSC_DEBUG_ASSERT(astgen->generic_data_type_state.scalar, "internal error: cannot resolve scalar generic when a scalar type has not been found");
			return astgen->generic_data_type_state.scalar;
		case HSC_DATA_TYPE_GENERIC_VEC2:
			HSC_DEBUG_ASSERT(astgen->generic_data_type_state.vec2 || astgen->generic_data_type_state.scalar, "internal error: cannot resolve vec2 generic when a vec2 or scalar type has not been found");
			if (astgen->generic_data_type_state.vec2) {
				return astgen->generic_data_type_state.vec2;
			}
			return HSC_DATA_TYPE_VEC2(astgen->generic_data_type_state.scalar);
		case HSC_DATA_TYPE_GENERIC_VEC3:
			HSC_DEBUG_ASSERT(astgen->generic_data_type_state.vec3 || astgen->generic_data_type_state.scalar, "internal error: cannot resolve vec3 generic when a vec3 or scalar type has not been found");
			if (astgen->generic_data_type_state.vec3) {
				return astgen->generic_data_type_state.vec3;
			}
			return HSC_DATA_TYPE_VEC3(astgen->generic_data_type_state.scalar);
		case HSC_DATA_TYPE_GENERIC_VEC4:
			HSC_DEBUG_ASSERT(astgen->generic_data_type_state.vec4 || astgen->generic_data_type_state.scalar, "internal error: cannot resolve vec4 generic when a vec4 or scalar type has not been found");
			if (astgen->generic_data_type_state.vec4) {
				return astgen->generic_data_type_state.vec4;
			}
			return HSC_DATA_TYPE_VEC4(astgen->generic_data_type_state.scalar);
	}

	return data_type;
}

void hsc_data_type_print_basic(HscAstGen* astgen, HscDataType data_type, void* data, FILE* f) {
	HSC_DEBUG_ASSERT(data_type < HSC_DATA_TYPE_BASIC_END, "internal error: expected a basic data type but got '%s'", hsc_data_type_string(astgen, data_type));

	U64 uint;
	S64 sint;
	F64 float_;
	switch (data_type) {
		case HSC_DATA_TYPE_VOID:
			fprintf(f, "void");
			break;
		case HSC_DATA_TYPE_BOOL:
			fprintf(f, *(U8*)data ? "true" : "false");
			break;
		case HSC_DATA_TYPE_U8: uint = *(U8*)data; goto UINT;
		case HSC_DATA_TYPE_U16: uint = *(U16*)data; goto UINT;
		case HSC_DATA_TYPE_U32: uint = *(U32*)data; goto UINT;
		case HSC_DATA_TYPE_U64: uint = *(U64*)data; goto UINT;
UINT:
			fprintf(f, "%zu", uint);
			break;
		case HSC_DATA_TYPE_S8: sint = *(S8*)data; goto SINT;
		case HSC_DATA_TYPE_S16: sint = *(S16*)data; goto SINT;
		case HSC_DATA_TYPE_S32: sint = *(S32*)data; goto SINT;
		case HSC_DATA_TYPE_S64: sint = *(S64*)data; goto SINT;
SINT:
			fprintf(f, "%zd", sint);
			break;
		case HSC_DATA_TYPE_F16: HSC_ABORT("TODO");
		case HSC_DATA_TYPE_F32: float_ = *(F32*)data; goto FLOAT;
		case HSC_DATA_TYPE_F64: float_ = *(F64*)data; goto FLOAT;
FLOAT:
			fprintf(f, "%f", float_);
			break;
	}
}

void hsc_constant_print(HscAstGen* astgen, HscConstantId constant_id, FILE* f) {
	HscConstant constant = hsc_constant_table_get(&astgen->constant_table, constant_id);
	if (constant.data_type < HSC_DATA_TYPE_BASIC_END) {
		hsc_data_type_print_basic(astgen, constant.data_type, constant.data, f);
	} else if (HSC_DATA_TYPE_VECTOR_START <= constant.data_type && constant.data_type < HSC_DATA_TYPE_MATRIX_END) {
		U32 componment_size;
		switch (HSC_DATA_TYPE_SCALAR(constant.data_type)) {
			case HSC_DATA_TYPE_VOID:
				componment_size = 0;
				break;
			case HSC_DATA_TYPE_BOOL:
			case HSC_DATA_TYPE_U8:
			case HSC_DATA_TYPE_S8:
				componment_size = 1;
				break;
			case HSC_DATA_TYPE_U16:
			case HSC_DATA_TYPE_S16:
			case HSC_DATA_TYPE_F16:
				componment_size = 2;
				break;
			case HSC_DATA_TYPE_U32:
			case HSC_DATA_TYPE_S32:
			case HSC_DATA_TYPE_F32:
				componment_size = 4;
				break;
			case HSC_DATA_TYPE_U64:
			case HSC_DATA_TYPE_S64:
			case HSC_DATA_TYPE_F64:
				componment_size = 8;
				break;
		}

		U32 componments_count;
		if (constant.data_type < HSC_DATA_TYPE_VECTOR_END) {
			componments_count = HSC_DATA_TYPE_VECTOR_COMPONENTS(constant.data_type);
			fprintf(f, "Vec%u(", componments_count);
		} else {
			U32 rows_count = HSC_DATA_TYPE_MATRX_ROWS(constant.data_type);
			U32 columns_count = HSC_DATA_TYPE_MATRX_COLUMNS(constant.data_type);
			componments_count = rows_count * columns_count;
			fprintf(f, "Mat%u%u(", rows_count, columns_count);
		}

		HscConstantId* constants = constant.data;
		for (U32 i = 0; i < componments_count; i += 1) {
			hsc_constant_print(astgen, constants[i], f);
			fprintf(f, i + 1 < componments_count ? ", " : ")");
		}
	} else {
		HSC_ABORT("unhandle type '%u'", constant.data_type);
	}
}

bool hsc_data_type_is_condition(HscAstGen* astgen, HscDataType data_type) {
	return HSC_DATA_TYPE_BOOL <= data_type && data_type < HSC_DATA_TYPE_BASIC_END;
}

U32 hsc_data_type_composite_fields_count(HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(!HSC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a composite type but got '%s'", hsc_data_type_string(astgen, data_type));

	if (HSC_DATA_TYPE_VECTOR_START <= data_type && data_type < HSC_DATA_TYPE_MATRIX_END) {
		U32 componments_count;
		if (data_type < HSC_DATA_TYPE_VECTOR_END) {
			componments_count = HSC_DATA_TYPE_VECTOR_COMPONENTS(data_type);
		} else {
			U32 rows_count = HSC_DATA_TYPE_MATRX_ROWS(data_type);
			U32 columns_count = HSC_DATA_TYPE_MATRX_COLUMNS(data_type);
			componments_count = rows_count * columns_count;
		}
		return  componments_count;
	} else {
		HSC_ABORT("unhandled data type '%u'", data_type);
	}
}

void hsc_string_table_init(HscStringTable* string_table, uint32_t data_cap, uint32_t entries_cap) {
	string_table->data = HSC_ALLOC_ARRAY(char, data_cap);
	HSC_ASSERT(string_table->data, "out of memory");
	string_table->entries = HSC_ALLOC_ARRAY(HscStringEntry, entries_cap);
	HSC_ASSERT(string_table->entries, "out of memory");
	string_table->data_cap = data_cap;
	string_table->entries_cap = entries_cap;
}

HscStringId hsc_string_table_deduplicate(HscStringTable* string_table, char* string, uint32_t string_size) {
	//
	// TODO: make this a hash table look up
	for (uint32_t entry_idx = 0; entry_idx < string_table->entries_count; entry_idx += 1) {
		HscStringEntry* entry = &string_table->entries[entry_idx];

		if (string_size == entry->size && memcmp(string_table->data + entry->start_idx, string, string_size) == 0) {
			return (HscStringId) { .idx_plus_one = entry_idx + 1 };
		}
	}

	if (string_table->entries_count >= string_table->entries_cap) {
		HSC_ABORT("string tables entries capacity exceeded TODO make this error message proper");
	}

	if (string_table->data_used_size + string_size >= string_table->data_cap) {
		HSC_ABORT("string tables entries capacity exceeded TODO make this error message proper");
	}


	uint32_t new_entry_idx = string_table->entries_count;
	string_table->entries_count += 1;
	HscStringEntry* entry = &string_table->entries[new_entry_idx];
	entry->start_idx = string_table->data_used_size;
	entry->size = string_size;

	memcpy(string_table->data + string_table->data_used_size, string, string_size);
	string_table->data_used_size += string_size;

	return (HscStringId) { .idx_plus_one = new_entry_idx + 1 };
}

HscString hsc_string_table_get(HscStringTable* string_table, HscStringId id) {
	HSC_DEBUG_ASSERT(id.idx_plus_one, "string id is null");

	HscStringEntry* entry = &string_table->entries[id.idx_plus_one - 1];

	HscString string;
	string.data = string_table->data + entry->start_idx;
	string.size = entry->size;
	return string;
}

void hsc_constant_table_init(HscConstantTable* constant_table, uint32_t data_cap, uint32_t entries_cap) {
	constant_table->data = HSC_ALLOC_ARRAY(char, data_cap);
	HSC_ASSERT(constant_table->data, "out of memory");
	constant_table->entries = HSC_ALLOC_ARRAY(HscConstantEntry, entries_cap);
	HSC_ASSERT(constant_table->entries, "out of memory");
	constant_table->data_cap = data_cap;
	constant_table->entries_cap = entries_cap;
}

HscConstantId _hsc_constant_table_deduplicate_end(HscConstantTable* constant_table, HscDataType data_type, void* data, U32 data_size, U32 data_align);

HscConstantId hsc_constant_table_deduplicate_basic(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type, void* data) {
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a basic type but got '%s'", hsc_data_type_string(astgen, data_type));
	HSC_DEBUG_ASSERT(constant_table->fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");

	Uptr size;
	Uptr align;
	hsc_data_type_size_align(astgen, data_type, &size, &align);

	return _hsc_constant_table_deduplicate_end(constant_table, data_type, data, size, align);
}

void hsc_constant_table_deduplicate_composite_start(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(!HSC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a non basic type but got '%s'", hsc_data_type_string(astgen, data_type));
	HSC_DEBUG_ASSERT(constant_table->fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");

	U32 fields_count;
	fields_count = hsc_data_type_composite_fields_count(astgen, data_type);

	constant_table->data_type = data_type;
	constant_table->fields_count = 0;
	constant_table->fields_cap = fields_count;
	constant_table->data_write_ptr = HSC_PTR_ROUND_UP_ALIGN(constant_table->data + constant_table->data_used_size, alignof(HscConstantId));
}

void hsc_constant_table_deduplicate_composite_add(HscConstantTable* constant_table, HscConstantId constant_id) {
	HSC_DEBUG_ASSERT(constant_table->fields_cap, "internal error: cannot add data when deduplication of constant has not started");
	HSC_DEBUG_ASSERT(constant_table->fields_count < constant_table->fields_cap, "internal error: the expected constant with '%u' fields has been exceeded", constant_table->fields_cap);

	constant_table->data_write_ptr[constant_table->fields_count] = constant_id;
	constant_table->fields_count += 1;
}

HscConstantId hsc_constant_table_deduplicate_composite_end(HscConstantTable* constant_table) {
	HSC_DEBUG_ASSERT(constant_table->fields_count == constant_table->fields_cap, "internal error: the composite constant for deduplication is incomplete, expected to be '%u' fields but got '%u'", constant_table->fields_count, constant_table->fields_cap);
	constant_table->fields_cap = 0;

	return _hsc_constant_table_deduplicate_end(constant_table, constant_table->data_type, constant_table->data_write_ptr, constant_table->fields_count * sizeof(HscConstantId), alignof(HscConstantId));
}

HscConstantId hsc_constant_table_deduplicate_zero(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type) {
	if (HSC_DATA_TYPE_IS_BASIC(data_type)) {
		U64 data = 0; // largest basic data type set to zero
		return hsc_constant_table_deduplicate_basic(constant_table, astgen, data_type, &data);
	} else {
		HSC_ABORT("TODO: make a new fuction we recursively call inside this deduplicate_end function to make a zeroed composite type and then deduplicate it");
		return _hsc_constant_table_deduplicate_end(constant_table, data_type, NULL, 0, alignof(HscConstantId));
	}
}

HscConstantId _hsc_constant_table_deduplicate_end(HscConstantTable* constant_table, HscDataType data_type, void* data, U32 data_size, U32 data_align) {
	//
	// TODO: make this a hash table look up
	for (uint32_t entry_idx = 0; entry_idx < constant_table->entries_count; entry_idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[entry_idx];
		if (entry->data_type == data_type && data_size == entry->size && memcmp(constant_table->data + entry->start_idx, data, data_size) == 0) {
			return (HscConstantId) { .idx_plus_one = entry_idx + 1 };
		}
	}

	if (constant_table->entries_count >= constant_table->entries_cap) {
		HSC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	if (constant_table->data_used_size + data_size >= constant_table->data_cap) {
		HSC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	constant_table->data_used_size = HSC_INT_ROUND_UP_ALIGN(constant_table->data_used_size, data_align);

	uint32_t new_entry_idx = constant_table->entries_count;
	constant_table->entries_count += 1;
	HscConstantEntry* entry = &constant_table->entries[new_entry_idx];
	entry->start_idx = constant_table->data_used_size;
	entry->size = data_size;
	entry->data_type = data_type;

	constant_table->data_used_size += data_size;

	if (constant_table->data_write_ptr != data) {
		memcpy(HSC_PTR_ADD(constant_table->data, entry->start_idx), data, data_size);
		F32 v = *(F32*)HSC_PTR_ADD(constant_table->data, entry->start_idx);
	}

	return (HscConstantId) { .idx_plus_one = new_entry_idx + 1 };
}

HscConstant hsc_constant_table_get(HscConstantTable* constant_table, HscConstantId id) {
	HSC_DEBUG_ASSERT(id.idx_plus_one, "constant id is null");

	HscConstantEntry* entry = &constant_table->entries[id.idx_plus_one - 1];

	HscConstant constant;
	constant.data_type = entry->data_type;
	constant.data = constant_table->data + entry->start_idx;
	constant.size = entry->size;
	return constant;
}

void hsc_add_intrinsic_function(HscAstGen* astgen, U32 function_id) {
	HscIntrinsicFunction* intrinsic_function = &hsc_intrinsic_functions[function_id];

	U32 name_size = strlen(intrinsic_function->name);
	HscStringId identifier_string_id = hsc_string_table_deduplicate(&astgen->string_table, intrinsic_function->name, name_size);
	if (hsc_intrinsic_function_overloads_count[function_id]) {
		//
		// this is the first overloaded function so add it to the global declarations
		HscDecl* decl_ptr;
		bool result = hsc_hash_table_find_or_insert(&astgen->global_declarations, identifier_string_id.idx_plus_one, &decl_ptr);
		HSC_ASSERT(!result, "internal error: intrinsic function '%.*s' already declared", name_size, intrinsic_function->name);
		*decl_ptr = HSC_DECL_FUNCTION_RAW(function_id);
	}

	HscFunction* function = &astgen->functions[function_id - 1];
	HSC_ZERO_ELMT(function);
	function->identifier_string_id = identifier_string_id;
	function->params_count = intrinsic_function->params_count;
	function->params_start_idx = astgen->function_params_and_local_variables_count;
	function->return_data_type = intrinsic_function->return_data_type;

	HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_and_local_variables_count + intrinsic_function->params_count - 1, astgen->function_params_and_local_variables_cap);
	HSC_COPY_ELMT_MANY(&astgen->function_params_and_local_variables[astgen->function_params_and_local_variables_count], intrinsic_function->params, intrinsic_function->params_count);
	astgen->function_params_and_local_variables_count += intrinsic_function->params_count;
}

void hsc_astgen_init(HscAstGen* astgen, HscCompilerSetup* setup) {
	astgen->function_params_and_local_variables = HSC_ALLOC_ARRAY(HscLocalVariable, setup->function_params_and_local_variables_cap);
	HSC_ASSERT(astgen->function_params_and_local_variables, "out of memory");
	astgen->functions = HSC_ALLOC_ARRAY(HscFunction, setup->functions_cap);
	HSC_ASSERT(astgen->functions, "out of memory");
	astgen->exprs = HSC_ALLOC_ARRAY(HscExpr, setup->exprs_cap);
	HSC_ASSERT(astgen->exprs, "out of memory");
	astgen->expr_locations = HSC_ALLOC_ARRAY(HscLocation, setup->exprs_cap);
	HSC_ASSERT(astgen->expr_locations, "out of memory");
	astgen->function_params_and_local_variables_cap = setup->function_params_and_local_variables_cap;
	astgen->functions_cap = setup->functions_cap;
	astgen->exprs_cap = setup->exprs_cap;

	astgen->variable_stack_strings = HSC_ALLOC_ARRAY(HscStringId, setup->variable_stack_cap);
	HSC_ASSERT(astgen->variable_stack_strings, "out of memory");
	astgen->variable_stack_var_indices = HSC_ALLOC_ARRAY(HscStringId, setup->variable_stack_cap);
	HSC_ASSERT(astgen->variable_stack_var_indices, "out of memory");
	astgen->variable_stack_cap = setup->variable_stack_cap;

	astgen->tokens = HSC_ALLOC_ARRAY(HscToken, setup->tokens_cap);
	HSC_ASSERT(astgen->tokens, "out of memory");
	astgen->token_locations = HSC_ALLOC_ARRAY(HscLocation, setup->tokens_cap);
	HSC_ASSERT(astgen->token_locations, "out of memory");
	astgen->token_values = HSC_ALLOC_ARRAY(HscLocation, setup->tokens_cap);
	HSC_ASSERT(astgen->token_values, "out of memory");
	astgen->tokens_cap = setup->tokens_cap;
	astgen->line_code_start_indices = HSC_ALLOC_ARRAY(U32, setup->lines_cap);
	HSC_ASSERT(astgen->line_code_start_indices, "out of memory");
	astgen->lines_cap = setup->lines_cap;
	astgen->print_color = true;

	hsc_hash_table_init(&astgen->global_declarations);
	{
		for (U32 function_id = HSC_FUNCTION_ID_NULL + 1; function_id <= HSC_FUNCTION_ID_VEC4_MULTI; function_id += 1) {
			hsc_add_intrinsic_function(astgen, function_id);
		}
		astgen->functions_count = HSC_FUNCTION_ID_USER_START;
	}
}

void hsc_astgen_print_code_line(HscAstGen* astgen, U32 display_line_num_size, U32 line) {
	U32 code_start_idx = astgen->line_code_start_indices[line];
	U32 code_end_idx;
	if (line >= astgen->lines_count) {
		code_end_idx = astgen->bytes_count;
	} else {
		code_end_idx = astgen->line_code_start_indices[line + 1];
	}
	while (1) {
		code_end_idx -= 1;
		if (code_end_idx == 0) {
			break;
		}
		U8 byte = astgen->bytes[code_end_idx];
		if (byte != '\r' && byte != '\n') {
			code_end_idx += 1;
			break;
		}
	}

	if (code_end_idx <= code_start_idx) {
		U32 code_size = code_end_idx - code_start_idx;
		char* code = (char*)&astgen->bytes[code_start_idx];
		const char* fmt = astgen->print_color
			? "\x1b[1;94m%*u|\x1b[0m\n"
			: "%*u|\n";
		printf(fmt, display_line_num_size, line);
	} else {
		U32 code_size = code_end_idx - code_start_idx;
		char* code = (char*)&astgen->bytes[code_start_idx];

		char code_without_tabs[1024];
		U32 dst_idx = 0;
		U32 src_idx = 0;
		for (; dst_idx < HSC_MIN(sizeof(code_without_tabs), code_size); dst_idx += 1, src_idx += 1) {
			char byte = code[src_idx];
			if (byte == '\t') {
				code_without_tabs[dst_idx + 0] = ' ';
				code_without_tabs[dst_idx + 1] = ' ';
				code_without_tabs[dst_idx + 2] = ' ';
				code_without_tabs[dst_idx + 3] = ' ';
				dst_idx += 3;
				code_size += 3;
			} else {
				code_without_tabs[dst_idx] = byte;
			}
		}

		const char* fmt = astgen->print_color
			? "\x1b[1;94m%*u|\x1b[0m %.*s\n"
			: "%*u| %.*s\n";
		printf(fmt, display_line_num_size, line, code_size, code_without_tabs);
	}
}

void hsc_astgen_print_code(HscAstGen* astgen, HscLocation* location) {
	U32 display_line_num_size = 0;
	U32 line = location->line_start + 2;
	while (line) {
		if (line < 10) {
			line = 0;
		} else {
			line /= 10;
		}
		display_line_num_size += 1;
	}

	display_line_num_size = HSC_MAX(display_line_num_size, 5);
	U32 tab_size = 4;
	display_line_num_size = HSC_INT_ROUND_UP_ALIGN(display_line_num_size, tab_size) - 2;

	line = location->line_start;
	if (line > 2) {
		hsc_astgen_print_code_line(astgen, display_line_num_size, line - 2);
	}
	if (line > 1) {
		hsc_astgen_print_code_line(astgen, display_line_num_size, line - 1);
	}

	hsc_astgen_print_code_line(astgen, display_line_num_size, line);

	for (U32 i = 0; i < display_line_num_size + 1; i += 1) {
		putchar(' ');
	}

	for (U32 i = 0; i < location->column_start; i += 1) {
		if (location->code_start_idx > location->column_start && astgen->bytes[location->code_start_idx - location->column_start + i] == '\t') {
			printf("    ");
		} else {
			putchar(' ');
		}
	}

	if (astgen->print_color) {
		printf("\x1b[1;93m");
	}
	for (U32 i = 0; i < location->column_end - location->column_start; i += 1) {
		putchar('^');
	}
	if (astgen->print_color) {
		printf("\x1b[0m");
	}
	printf("\n");

	if (line + 1 <= astgen->lines_count) {
		hsc_astgen_print_code_line(astgen, display_line_num_size, line + 1);
	}
	if (line + 2 <= astgen->lines_count) {
		hsc_astgen_print_code_line(astgen, display_line_num_size, line + 2);
	}
}

void hsc_astgen_found_newline(HscAstGen* astgen) {
	astgen->location.line_start += 1;
	astgen->location.line_end += 1;
	astgen->location.column_start = 1;
	astgen->location.column_end = 1;

	if (astgen->lines_count >= astgen->lines_cap) {
		hsc_astgen_error_1(astgen, "internal error: the lines capacity of '%u' has been exceeded", astgen->lines_cap);
	}

	astgen->line_code_start_indices[astgen->lines_count] = astgen->location.code_end_idx;
	astgen->lines_count += 1;
}

void hsc_astgen_token_count_extra_newlines(HscAstGen* astgen) {
	U32 lines_count = 3;
	HscLocation location = astgen->location;
	while (astgen->location.code_end_idx < astgen->bytes_count) {
		U8 byte = astgen->bytes[astgen->location.code_end_idx];
		astgen->location.code_end_idx += 1;
		if (byte == '\n') {
			hsc_astgen_found_newline(astgen);
			lines_count -= 1;
			if (lines_count == 0) {
				break;
			}
		}
	}
	astgen->location = location;
}

HSC_NORETURN void hsc_astgen_error(HscAstGen* astgen, HscLocation* location, HscLocation* other_location, const char* fmt, va_list va_args) {
	hsc_astgen_token_count_extra_newlines(astgen);

	const char* error_fmt = astgen->print_color
		? "\x1b[1;91merror\x1b[0m: "
		: "error: ";
	printf("%s", error_fmt);

	if (astgen->print_color) {
		printf("\x1b[1;97m");
	}

	vprintf(fmt, va_args);

	const char* file_path = "test.hsc";
	error_fmt = astgen->print_color
		? "\x1b[1;95m\nfile\x1b[97m: %s:%u:%u\n\x1b[0m"
		: "\nfile: %s:%u:%u\n";
	printf(error_fmt, file_path, location->line_start, location->column_start);

	hsc_astgen_print_code(astgen, location);

	if (other_location) {
		const char* error_fmt = astgen->print_color
			? "\x1b[1;97\nmoriginally defined here\x1b[0m: \n"
			: "\noriginally defined here: ";
		printf("%s", error_fmt);

		const char* file_path = "test.hsc";
		error_fmt = astgen->print_color
			? "\x1b[1;95mfile\x1b[97m: %s:%u:%u\n\x1b[0m"
			: "file: %s:%u:%u\n";
		printf(error_fmt, file_path, other_location->line_start, other_location->column_start);

		hsc_astgen_print_code(astgen, other_location);
	}

	if (astgen->error_info) {
		printf("\n%s\n", astgen->error_info);
	}

	exit(1);
}

void hsc_astgen_token_error_1(HscAstGen* astgen, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	hsc_astgen_error(astgen, &astgen->location, NULL, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_token_error_2(HscAstGen* astgen, HscLocation* other_location, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	hsc_astgen_error(astgen, &astgen->location, other_location, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_error_1(HscAstGen* astgen, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	HscLocation* location = &astgen->token_locations[HSC_MIN(astgen->token_read_idx, astgen->tokens_count - 1)];
	hsc_astgen_error(astgen, location, NULL, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_error_2(HscAstGen* astgen, HscLocation* other_location, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	HscLocation* location = &astgen->token_locations[HSC_MIN(astgen->token_read_idx, astgen->tokens_count - 1)];
	hsc_astgen_error(astgen, location, other_location, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_add_token(HscAstGen* astgen, HscToken token) {
	if (astgen->tokens_count >= astgen->tokens_cap) {
		hsc_astgen_token_error_1(astgen, "internal error: the tokens capacity of '%u' has been exceeded", astgen->tokens_cap);
	}

	astgen->tokens[astgen->tokens_count] = token;
	astgen->token_locations[astgen->tokens_count] = astgen->location;
	astgen->tokens_count += 1;
}

void hsc_astgen_add_token_value(HscAstGen* astgen, HscTokenValue value) {
	if (astgen->token_values_count >= astgen->tokens_cap) {
		hsc_astgen_token_error_1(astgen, "internal error: the token values capacity of '%u' has been exceeded", astgen->tokens_cap);
	}

	astgen->token_values[astgen->token_values_count] = value;
	astgen->token_values_count += 1;
}

bool hsc_u64_checked_add(uint64_t a, uint64_t b, uint64_t* out) {
	if (b > ((uint64_t)-1 - a)) { return false; }
	*out = a + b;
	return true;
}

bool hsc_s64_checked_add(int64_t a, int64_t b, int64_t* out) {
	if (a >= 0) {
		if (b > ((uint64_t)-1 - a)) { return false; }
	} else {
		if (b < ((uint64_t)-1 - a)) { return false; }
	}

	*out = a + b;
	return true;
}

bool hsc_i64_checked_mul(uint64_t a, uint64_t b, uint64_t* out) {
	uint64_t r = a * b;
	if (a != 0 && b != 0 && a != r / b) {
		return false;
	}
	*out = r;
	return true;
}

uint32_t hsc_parse_num(HscAstGen* astgen, HscToken* token_out) {
	char* num_string = (char*)&astgen->bytes[astgen->location.code_end_idx];
	uint32_t remaining_size = astgen->bytes_count - astgen->location.code_end_idx;
	uint32_t token_size = 0;

	bool is_negative = num_string[0] == '-';
	if (is_negative) {
		token_size += 1;
	}

	HscToken token = HSC_TOKEN_LIT_S32;
	uint8_t radix = 10;
	if (num_string[0] == '0') {
		switch (num_string[1]) {
			case 'x':
			case 'X':
				radix = 16;
				token_size += 2;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				radix = 8;
				token_size += 2;
				break;
		}
	}

	U64 u64 = 0;
	S64 s64 = 0;
	F64 f64 = 0.0;
	F64 pow_10 = 10.0;
	while (token_size < remaining_size) {
		uint8_t digit = num_string[token_size];
		token_size += 1;

		switch (digit) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			{
				if (radix == 8 && digit > 7) {
					hsc_astgen_token_error_1(astgen, "octal digits must be from 0 to 7 inclusively");
				}
				uint32_t int_digit = digit - '0';
				switch (token) {
					case HSC_TOKEN_LIT_U32:
					case HSC_TOKEN_LIT_U64:
					case HSC_TOKEN_LIT_S32:
					case HSC_TOKEN_LIT_S64:
						if (
							!hsc_i64_checked_mul(u64, radix, &u64)        ||
							!hsc_u64_checked_add(u64, int_digit, &u64)
						) {
							hsc_astgen_token_error_1(astgen, "integer literal is too large and will overflow a U64 integer");
						}
						break;
					case HSC_TOKEN_LIT_F32:
					case HSC_TOKEN_LIT_F64:
						f64 += (F64)(int_digit) / pow_10;
						if (isinf(f64)) {
							hsc_astgen_token_error_1(astgen, "float literal is too large and will overflow a f64");
						}
						pow_10 *= 10.0;
						break;
				}
				break;
			};
			case 'u':
			case 'U':
			{
				if (token == HSC_TOKEN_LIT_F64 || is_negative) {
					hsc_astgen_token_error_1(astgen, "the 'u' suffix can only be applied to positive integer numbers. e.g. 3369");
				}
				token = HSC_TOKEN_LIT_U32;
				digit = num_string[token_size];
				if (digit != 'l' && digit != 'L') {
					goto NUM_END;
				}
				// fallthrough
			};
			case 'l':
			case 'L':
				switch (token) {
					case HSC_TOKEN_LIT_F32:
					case HSC_TOKEN_LIT_F64:
						hsc_astgen_token_error_1(astgen, "the 'l' suffix for a long double is unsupported");
					case HSC_TOKEN_LIT_S32:
						token = HSC_TOKEN_LIT_S64;
						break;
					case HSC_TOKEN_LIT_U32:
						token = HSC_TOKEN_LIT_U64;
						break;
				}

				digit = num_string[token_size];
				if (digit == 'l' || digit == 'L') {
					token_size += 1; // ignore LL suffixes
				}
				goto NUM_END;
			case '.':
				if (token == HSC_TOKEN_LIT_F64) {
					hsc_astgen_token_error_1(astgen, "float literals can only have a single '.'");
				}
				if (radix != 10) {
					hsc_astgen_token_error_1(astgen, "octal and hexidecimal digits are not supported for float literals");
				}

				token = HSC_TOKEN_LIT_F64;
				f64 = (F64)u64;
				if ((U64)f64 != u64) {
					hsc_astgen_token_error_1(astgen, "float literal is too large and will overflow a f64");
				}
				break;
			default: {
				if (radix == 16 && ((digit >= 'a' && digit <= 'f') || (digit >= 'A' && digit <= 'F'))) {
					uint32_t int_digit = 10 + (digit >= 'A' ? (digit - 'A') : (digit - 'a'));
					switch (token) {
						case HSC_TOKEN_LIT_U32:
						case HSC_TOKEN_LIT_U64:
						case HSC_TOKEN_LIT_S32:
						case HSC_TOKEN_LIT_S64:
							if (
								!hsc_i64_checked_mul(u64, radix, &u64)        ||
								!hsc_u64_checked_add(u64, int_digit, &u64)
							) {
								hsc_astgen_token_error_1(astgen, "integer literal is too large and will overflow a U64 integer");
							}
							break;
					}
				} else if (digit == 'f' || digit == 'F') {
					if (token != HSC_TOKEN_LIT_F64) {
						hsc_astgen_token_error_1(astgen, "only float literals can be made into a float literal with a 'f' suffix. e.g. 0.f or 1.0f");
					}
					token = HSC_TOKEN_LIT_F32;
					goto NUM_END;
				} else if ((digit >= 'a' && digit <= 'z') || (digit >= 'A' && digit <= 'Z')) {
					switch (token) {
						case HSC_TOKEN_LIT_U32:
						case HSC_TOKEN_LIT_U64:
						case HSC_TOKEN_LIT_S32:
						case HSC_TOKEN_LIT_S64:
							hsc_astgen_token_error_1(astgen, "invalid suffix for integer literals");
						case HSC_TOKEN_LIT_F32:
						case HSC_TOKEN_LIT_F64:
							hsc_astgen_token_error_1(astgen, "invalid suffix for float literals");
					}
				} else {
					token_size -= 1;
					goto NUM_END;
				}
			};
		}
	}
NUM_END:

	if (is_negative) {
		switch (token) {
			case HSC_TOKEN_LIT_S32:
			case HSC_TOKEN_LIT_S64:
			{
				if (u64 > (U64)S64_MAX + 1) {
					hsc_astgen_token_error_1(astgen, "integer literal is too large and will overflow a S64 integer");
				}
				s64 = -u64;
				break;
			};
			case HSC_TOKEN_LIT_F32:
			case HSC_TOKEN_LIT_F64:
				f64 = -f64;
				break;
		}
	}

	switch (token) {
		case HSC_TOKEN_LIT_U32:
			if (u64 > U32_MAX) {
				token = HSC_TOKEN_LIT_U64;
			}
			break;
		case HSC_TOKEN_LIT_S32:
			if (is_negative) {
				if (s64 > S32_MAX || s64 < S32_MIN) {
					token = HSC_TOKEN_LIT_S64;
				}
			} else {
				if (u64 > S32_MAX) {
					if (radix != 10 && u64 <= U32_MAX) {
						token = HSC_TOKEN_LIT_U32;
					} else if (u64 <= S64_MAX) {
						token = HSC_TOKEN_LIT_S64;
					} else if (radix != 10) {
						token = HSC_TOKEN_LIT_U64;
					} else {
						hsc_astgen_token_error_1(astgen, "integer literal is too large and will overflow a S64 integer, consider using 'u' suffix to promote to an unsigned type. e.g. 1000u");
					}
				}
				s64 = u64;
			}
			break;
	}

	U32 u32;
	S32 s32;
	F32 f32;
	HscDataType data_type;
	void* data;
	switch (token) {
		case HSC_TOKEN_LIT_U32: data = &u32; u32 = u64; data_type = HSC_DATA_TYPE_U32; break;
		case HSC_TOKEN_LIT_U64: data = &u64; data_type = HSC_DATA_TYPE_U64; break;
		case HSC_TOKEN_LIT_S32: data = &s32; s32 = s64; data_type = HSC_DATA_TYPE_S32; break;
		case HSC_TOKEN_LIT_S64: data = &s64; data_type = HSC_DATA_TYPE_S64; break;
		case HSC_TOKEN_LIT_F32: data = &f32; f32 = f64; data_type = HSC_DATA_TYPE_F32; break;
		case HSC_TOKEN_LIT_F64: data = &f64; data_type = HSC_DATA_TYPE_F64; break;
	}

	HscTokenValue token_value;
	token_value.constant_id = hsc_constant_table_deduplicate_basic(&astgen->constant_table, astgen, data_type, data);
	hsc_astgen_add_token_value(astgen, token_value);

	*token_out = token;
	return token_size;
}

void hsc_astgen_tokenize(HscAstGen* astgen) {
	HscToken brackets_to_close[_HSC_TOKENIZER_NESTED_BRACKETS_CAP];
	HscLocation brackets_to_close_locations[_HSC_TOKENIZER_NESTED_BRACKETS_CAP];
	uint32_t brackets_to_close_count = 0;

	bool is_negative;
	char num_buf[20];

	astgen->line_code_start_indices[0] = 0;
	astgen->line_code_start_indices[1] = 0;
	astgen->lines_count += 2;

	astgen->location.code_start_idx = 0;
	astgen->location.code_end_idx = 0;
	astgen->location.line_start = 1;
	astgen->location.line_end = 1;
	astgen->location.column_start = 1;
	astgen->location.column_end = 1;
	while (astgen->location.code_end_idx < astgen->bytes_count) {
		uint8_t byte = astgen->bytes[astgen->location.code_end_idx];

		astgen->location.code_start_idx = astgen->location.code_end_idx;
		astgen->location.line_start = astgen->location.line_end;
		astgen->location.column_start = astgen->location.column_end;

		HscToken token = HSC_TOKEN_COUNT;
		HscToken close_token;
		uint32_t token_size = 1;
		switch (byte) {
			case ' ':
			case '\t':
				astgen->location.code_start_idx += 1;
				astgen->location.code_end_idx += 1;
				astgen->location.column_start += 1;
				astgen->location.column_end += 1;
				continue;
			case '\r':
			case '\n':
				astgen->location.code_start_idx += 1;
				astgen->location.code_end_idx += 1;
				if (byte == '\n') {
					hsc_astgen_found_newline(astgen);
				}
				continue;
			case '.': token = HSC_TOKEN_FULL_STOP; break;
			case ',': token = HSC_TOKEN_COMMA; break;
			case ';': token = HSC_TOKEN_SEMICOLON; break;
			case ':': token = HSC_TOKEN_COLON; break;
			case '~': token = HSC_TOKEN_TILDE; break;
			case '+':
				if (astgen->bytes[astgen->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_ADD_ASSIGN;
				} else {
					token = HSC_TOKEN_PLUS;
				}
				break;
			case '-': {
				U8 next_byte = astgen->bytes[astgen->location.code_end_idx + 1];
				if (isdigit(next_byte)) {
					token_size = hsc_parse_num(astgen, &token);
				} else if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_SUBTRACT_ASSIGN;
				} else {
					token = HSC_TOKEN_MINUS;
				}
				break;
			};
			case '/': {
				uint8_t next_byte = astgen->bytes[astgen->location.code_end_idx + 1];
				if (next_byte == '/') {
					astgen->location.code_end_idx += 2;
					while (astgen->location.code_end_idx < astgen->bytes_count) {
						uint8_t b = astgen->bytes[astgen->location.code_end_idx];
						astgen->location.code_end_idx += 1;
						if (b == '\n') {
							break;
						}
					}

					token_size = astgen->location.code_end_idx - astgen->location.code_start_idx;
					astgen->location.column_start += token_size;
					astgen->location.column_end += token_size;
					continue;
				} else if (next_byte == '*') {
					astgen->location.code_end_idx += 2;
					astgen->location.column_end += 2;
					while (astgen->location.code_end_idx < astgen->bytes_count) {
						uint8_t b = astgen->bytes[astgen->location.code_end_idx];
						astgen->location.code_end_idx += 1;
						astgen->location.column_end += 1;
						if (b == '*') {
							b = astgen->bytes[astgen->location.code_end_idx];
							if (b == '/') { // no need to check in bounds see _HSC_TOKENIZER_LOOK_HEAD_SIZE
								astgen->location.code_end_idx += 1;
								astgen->location.column_end += 1;
								break;
							}
						} else if (byte == '\n') {
							hsc_astgen_found_newline(astgen);
						}
					}

					astgen->location.column_start = astgen->location.column_end;
					continue;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_DIVIDE_ASSIGN;
				} else {
					token = HSC_TOKEN_FORWARD_SLASH;
				}
				break;
			};
			case '*':
				if (astgen->bytes[astgen->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_MULTIPLY_ASSIGN;
				} else {
					token = HSC_TOKEN_ASTERISK;
				}
				break;
			case '%':
				if (astgen->bytes[astgen->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_MODULO_ASSIGN;
				} else {
					token = HSC_TOKEN_PERCENT;
				}
				break;
			case '&': {
				U8 next_byte = astgen->bytes[astgen->location.code_end_idx + 1];
				if (next_byte == '&') {
					token_size = 2;
					token = HSC_TOKEN_LOGICAL_AND;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_BIT_AND_ASSIGN;
				} else {
					token = HSC_TOKEN_AMPERSAND;
				}
				break;
			};
			case '|': {
				U8 next_byte = astgen->bytes[astgen->location.code_end_idx + 1];
				if (next_byte == '|') {
					token_size = 2;
					token = HSC_TOKEN_LOGICAL_OR;
				} else if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_BIT_OR_ASSIGN;
				} else {
					token = HSC_TOKEN_PIPE;
				}
				break;
			};
			case '^':
				if (astgen->bytes[astgen->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_BIT_XOR_ASSIGN;
				} else {
					token = HSC_TOKEN_CARET;
				}
				break;
			case '!':
				if (astgen->bytes[astgen->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_LOGICAL_NOT_EQUAL;
				} else {
					token = HSC_TOKEN_EXCLAMATION_MARK;
				}
				break;
			case '=':
				if (astgen->bytes[astgen->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_LOGICAL_EQUAL;
				} else {
					token = HSC_TOKEN_EQUAL;
				}
				break;
			case '<': {
				U8 next_byte = astgen->bytes[astgen->location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_LESS_THAN_OR_EQUAL;
				} else if (next_byte == '<') {
					if (astgen->bytes[astgen->location.code_end_idx + 2] == '=') {
						token_size = 3;
						token = HSC_TOKEN_BIT_SHIFT_LEFT_ASSIGN;
					} else {
						token_size = 2;
						token = HSC_TOKEN_BIT_SHIFT_LEFT;
					}
				} else {
					token = HSC_TOKEN_LESS_THAN;
				}
				break;
			};
			case '>': {
				U8 next_byte = astgen->bytes[astgen->location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_GREATER_THAN_OR_EQUAL;
				} else if (next_byte == '>') {
					if (astgen->bytes[astgen->location.code_end_idx + 2] == '=') {
						token_size = 3;
						token = HSC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN;
					} else {
						token_size = 2;
						token = HSC_TOKEN_BIT_SHIFT_RIGHT;
					}
				} else {
					token = HSC_TOKEN_GREATER_THAN;
				}
				break;
			};
			case '{':
				token = HSC_TOKEN_CURLY_OPEN;
				close_token = HSC_TOKEN_CURLY_CLOSE;
				goto OPEN_BRACKETS;
			case '(':
				token = HSC_TOKEN_PARENTHESIS_OPEN;
				close_token = HSC_TOKEN_PARENTHESIS_CLOSE;
OPEN_BRACKETS:
			{
				if (brackets_to_close_count >= _HSC_TOKENIZER_NESTED_BRACKETS_CAP) {
					hsc_astgen_token_error_1(astgen, "nested brackets capacity of '%u' has been exceeded", _HSC_TOKENIZER_NESTED_BRACKETS_CAP);
				}
				brackets_to_close[brackets_to_close_count] = close_token;
				brackets_to_close_locations[brackets_to_close_count] = astgen->location;
				brackets_to_close_locations[brackets_to_close_count].code_end_idx += 1;
				brackets_to_close_locations[brackets_to_close_count].column_end += 1;
				brackets_to_close_count += 1;
				break;
			};
			case '}':
				token = HSC_TOKEN_CURLY_CLOSE;
				goto CLOSE_BRACKETS;
			case ')':
				token = HSC_TOKEN_PARENTHESIS_CLOSE;
CLOSE_BRACKETS:
			{
				if (brackets_to_close_count == 0) {
					hsc_astgen_token_error_1(astgen, "no brackets are open to close '%c'", byte);
				}

				brackets_to_close_count -= 1;
				if (brackets_to_close[brackets_to_close_count] != token) {
					astgen->location.code_end_idx += 1;
					astgen->location.column_end += 1;
					hsc_astgen_token_error_2(astgen, &brackets_to_close_locations[brackets_to_close_count], "expected to close bracket pair with '%s' but got '%c'", hsc_token_strings[brackets_to_close[brackets_to_close_count]], byte);
				}
				break;
			};

			default: {
				if ('0' <= byte && byte <= '9') {
					token_size = hsc_parse_num(astgen, &token);
					break;
				}

				if (
					(byte < 'a' || 'z' < byte) &&
					(byte < 'A' || 'Z' < byte)
				) {
					hsc_astgen_token_error_1(astgen, "invalid token '%c'", byte);
				}

				uint8_t* ident_string = &astgen->bytes[astgen->location.code_end_idx];
				while (astgen->location.code_end_idx < astgen->bytes_count) {
					uint8_t ident_byte = ident_string[token_size];

					switch (ident_byte) {
						case ' ':
						case '.':
						case ',':
						case '+':
						case '-':
						case '*':
						case '/':
						case '%':
						case '\t':
						case '(':
						case ')':
						case '{':
						case '}':
						case ':':
						case ';':
						case '\n':
						case '\r':
							goto IDENT_END;
					}

					if (
						(ident_byte < 'a' || 'z' < ident_byte) &&
						(ident_byte < 'A' || 'Z' < ident_byte) &&
						(ident_byte < '0' || '9' < ident_byte) &&
						(ident_byte != '_')
					) {
						hsc_astgen_token_error_1(astgen, "identifier character must be alphanumeric but got '%c'", ident_byte);
					}
					token_size += 1;
				}
IDENT_END: {}

				HscStringId string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)ident_string, token_size);
				if (string_id.idx_plus_one < HSC_STRING_ID_INTRINSIC_TYPES_END) {
					if (string_id.idx_plus_one < HSC_STRING_ID_KEYWORDS_END) {
						token = HSC_TOKEN_KEYWORDS_START + (string_id.idx_plus_one - HSC_STRING_ID_KEYWORDS_START);
					} else {
						token = HSC_TOKEN_INTRINSIC_TYPES_START + (string_id.idx_plus_one - HSC_STRING_ID_INTRINSIC_TYPES_START);
					}
				} else {
					token = HSC_TOKEN_IDENT;
					HscTokenValue token_value;
					token_value.string_id = string_id;
					hsc_astgen_add_token_value(astgen, token_value);
				}

				break;
			};
		}

		astgen->location.code_end_idx += token_size;
		astgen->location.column_end += token_size;

		static int i = 0;
		i += 1;
		if (i == 12) {
			//hsc_astgen_token_error_1(astgen, "test error");
		}
		hsc_astgen_add_token(astgen, token);
	}

	hsc_astgen_add_token(astgen, HSC_TOKEN_EOF);
}

HscToken hsc_token_peek(HscAstGen* astgen) {
	return astgen->tokens[HSC_MIN(astgen->token_read_idx, astgen->tokens_count - 1)];
}

HscToken hsc_token_peek_ahead(HscAstGen* astgen, U32 by) {
	return astgen->tokens[HSC_MIN(astgen->token_read_idx + by, astgen->tokens_count - 1)];
}

HscToken hsc_token_next(HscAstGen* astgen) {
	astgen->token_read_idx += 1;
	return astgen->tokens[HSC_MIN(astgen->token_read_idx, astgen->tokens_count - 1)];
}

void hsc_token_consume(HscAstGen* astgen, U32 amount) {
	astgen->token_read_idx += amount;
}

void hsc_token_value_consume(HscAstGen* astgen, U32 amount) {
	astgen->token_value_read_idx += amount;
}

HscTokenValue hsc_token_value_next(HscAstGen* astgen) {
	HscTokenValue value = astgen->token_values[HSC_MIN(astgen->token_value_read_idx, astgen->token_values_count - 1)];
	astgen->token_value_read_idx += 1;
	return value;
}

bool hsc_astgen_generate_data_type(HscAstGen* astgen, HscDataType* type_out) {
	HscToken token = hsc_token_peek(astgen);
	if (HSC_TOKEN_IS_BASIC_TYPE(token)) {
		*type_out = (HscDataType)token;
		hsc_token_consume(astgen, 1);
		return true;
	}
	switch (token) {
		case HSC_TOKEN_INTRINSIC_TYPE_VEC2:
		case HSC_TOKEN_INTRINSIC_TYPE_VEC3:
		case HSC_TOKEN_INTRINSIC_TYPE_VEC4:
			// TODO peek ahead and check for a vector using a different basic type
			switch (token) {
				case HSC_TOKEN_INTRINSIC_TYPE_VEC2:
					*type_out = HSC_DATA_TYPE_VEC2(HSC_DATA_TYPE_F32);
					break;
				case HSC_TOKEN_INTRINSIC_TYPE_VEC3:
					*type_out = HSC_DATA_TYPE_VEC3(HSC_DATA_TYPE_F32);
					break;
				case HSC_TOKEN_INTRINSIC_TYPE_VEC4:
					*type_out = HSC_DATA_TYPE_VEC4(HSC_DATA_TYPE_F32);
					break;
			}
			hsc_token_consume(astgen, 1);
			return true;
	}

	//
	// TODO add more parsing of more types and maybe allow the parser to continue if this is just an identifier that is undeclared.
	hsc_astgen_error_1(astgen, "expected type here");
}

HscLocation* hsc_decl_location(HscAstGen* astgen, HscDecl decl) {
	if (HSC_DECL_IS_FUNCTION(decl)) {
		HscFunctionId function_id = HSC_DECL_FUNCTION_ID(decl);
		return &astgen->functions[function_id.idx_plus_one - 1].location;
	}

	return NULL;
}

HscExpr* hsc_astgen_alloc_expr(HscAstGen* astgen, HscExprType type) {
	HSC_ASSERT(astgen->exprs_count < astgen->exprs_cap, "expression are full");
	HscExpr* expr = &astgen->exprs[astgen->exprs_count];
	expr->type = type;
	expr->is_stmt_block_entry = false;
	astgen->exprs_count += 1;
	return expr;
}

HscExpr* hsc_astgen_alloc_expr_many(HscAstGen* astgen, U32 amount) {
	HSC_ASSERT(astgen->exprs_count + amount <= astgen->exprs_cap, "expression are full");
	HscExpr* exprs = &astgen->exprs[astgen->exprs_count];
	astgen->exprs_count += amount;
	return exprs;
}

HscExpr* hsc_astgen_generate_unary_expr(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	HscUnaryOp unary_op;
	switch (token) {
		case HSC_TOKEN_KEYWORD_TRUE:
		case HSC_TOKEN_KEYWORD_FALSE:
		case HSC_TOKEN_LIT_U32:
		case HSC_TOKEN_LIT_U64:
		case HSC_TOKEN_LIT_S32:
		case HSC_TOKEN_LIT_S64:
		case HSC_TOKEN_LIT_F32:
		case HSC_TOKEN_LIT_F64: {
			HscDataType data_type;
			HscConstantId constant_id;
			HscTokenValue value;
			switch (token) {
				case HSC_TOKEN_KEYWORD_TRUE:
					data_type = HSC_DATA_TYPE_BOOL;
					constant_id = astgen->true_constant_id;
					break;
				case HSC_TOKEN_KEYWORD_FALSE:
					data_type = HSC_DATA_TYPE_BOOL;
					constant_id = astgen->false_constant_id;
					break;
				case HSC_TOKEN_LIT_U32: data_type = HSC_DATA_TYPE_U32; break;
				case HSC_TOKEN_LIT_U64: data_type = HSC_DATA_TYPE_U64; break;
				case HSC_TOKEN_LIT_S32: data_type = HSC_DATA_TYPE_S32; break;
				case HSC_TOKEN_LIT_S64: data_type = HSC_DATA_TYPE_S64; break;
				case HSC_TOKEN_LIT_F32: data_type = HSC_DATA_TYPE_F32; break;
				case HSC_TOKEN_LIT_F64: data_type = HSC_DATA_TYPE_F64; break;
			}
			if (data_type != HSC_DATA_TYPE_BOOL) {
				constant_id = hsc_token_value_next(astgen).constant_id;
			}

			HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CONSTANT);
			expr->constant.id = constant_id.idx_plus_one;
			expr->data_type = data_type;
			hsc_token_consume(astgen, 1);
			return expr;
		};
		case HSC_TOKEN_IDENT: {
			HscTokenValue identifier_value = hsc_token_value_next(astgen);
			hsc_token_consume(astgen, 1);

			U32 existing_variable_id = hsc_astgen_variable_stack_find(astgen, identifier_value.string_id);
			if (existing_variable_id) {
				HscFunction* function = &astgen->functions[astgen->functions_count - 1];
				HscLocalVariable* local_variable = &astgen->function_params_and_local_variables[function->params_start_idx + existing_variable_id - 1];

				HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_LOCAL_VARIABLE);
				expr->variable.idx = existing_variable_id - 1;
				expr->data_type = local_variable->data_type;
				return expr;
			}

			HscDecl decl;
			if (hsc_hash_table_find(&astgen->global_declarations, identifier_value.string_id.idx_plus_one, &decl)) {
				if (!HSC_DECL_IS_FUNCTION(decl)) {
					HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
					HscLocation* other_location = hsc_decl_location(astgen, decl);
					hsc_astgen_error_2(astgen, other_location, "type '%.*s' cannot be used here", (int)string.size, string.data);
				}

				HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_FUNCTION);
				HscFunctionId function_id = HSC_DECL_FUNCTION_ID(decl);
				expr->function.id = function_id.idx_plus_one;
				return expr;
			}

			HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
			hsc_astgen_error_1(astgen, "undeclared identifier '%.*s'", (int)string.size, string.data);
		};
		case HSC_TOKEN_TILDE: unary_op = HSC_UNARY_OP_BIT_NOT; goto UNARY;
		case HSC_TOKEN_EXCLAMATION_MARK: unary_op = HSC_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HSC_TOKEN_PLUS: unary_op = HSC_UNARY_OP_PLUS; goto UNARY;
		case HSC_TOKEN_MINUS: unary_op = HSC_UNARY_OP_NEGATE; goto UNARY;
UNARY:
		{
			HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_UNARY_OP_START + unary_op);
			hsc_token_consume(astgen, 1);

			HscExpr* inner_expr = hsc_astgen_generate_unary_expr(astgen);
			expr->unary.expr_rel_idx = inner_expr - expr;
			expr->data_type = token == HSC_TOKEN_EXCLAMATION_MARK ? HSC_DATA_TYPE_BOOL : inner_expr->data_type;
			return expr;
		};
		case HSC_TOKEN_PARENTHESIS_OPEN: {
			hsc_token_consume(astgen, 1);
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			HscToken token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
				hsc_astgen_error_1(astgen, "expected a ')' here to finish the expression");
			}
			hsc_token_consume(astgen, 1);
			return expr;
		};
		case HSC_DATA_TYPE_VOID:
		case HSC_DATA_TYPE_BOOL:
		case HSC_DATA_TYPE_U8:
		case HSC_DATA_TYPE_U16:
		case HSC_DATA_TYPE_U32:
		case HSC_DATA_TYPE_U64:
		case HSC_DATA_TYPE_S8:
		case HSC_DATA_TYPE_S16:
		case HSC_DATA_TYPE_S32:
		case HSC_DATA_TYPE_S64:
		case HSC_DATA_TYPE_F16:
		case HSC_DATA_TYPE_F32:
		case HSC_DATA_TYPE_F64:
		case HSC_TOKEN_INTRINSIC_TYPE_VEC2:
		case HSC_TOKEN_INTRINSIC_TYPE_VEC3:
		case HSC_TOKEN_INTRINSIC_TYPE_VEC4:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT2X2:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT2X3:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT2X4:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT3X2:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT3X3:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT3X4:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT4X2:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT4X3:
		case HSC_TOKEN_INTRINSIC_TYPE_MAT4X4:
		{
			HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_DATA_TYPE);
			hsc_astgen_generate_data_type(astgen, &expr->data_type);
			return expr;
		};
		default:
			hsc_astgen_error_1(astgen, "expected an expression here but got '%s'", hsc_token_strings[token]);
	}
}

void hsc_astgen_generate_binary_op(HscAstGen* astgen, HscExprType* binary_op_type_out, U32* precedence_out, bool* is_assignment_out) {
	HscToken token = hsc_token_peek(astgen);
	*is_assignment_out = false;
	switch (token) {
		case HSC_TOKEN_PARENTHESIS_OPEN:
			*binary_op_type_out = HSC_EXPR_TYPE_CALL;
			*precedence_out = 1;
			break;
		case HSC_TOKEN_ASTERISK:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(MULTIPLY);
			*precedence_out = 3;
			break;
		case HSC_TOKEN_FORWARD_SLASH:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(DIVIDE);
			*precedence_out = 3;
			break;
		case HSC_TOKEN_PERCENT:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(MODULO);
			*precedence_out = 3;
			break;
		case HSC_TOKEN_PLUS:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(ADD);
			*precedence_out = 4;
			break;
		case HSC_TOKEN_MINUS:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(SUBTRACT);
			*precedence_out = 4;
			break;
		case HSC_TOKEN_BIT_SHIFT_LEFT:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT);
			*precedence_out = 5;
			break;
		case HSC_TOKEN_BIT_SHIFT_RIGHT:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT);
			*precedence_out = 5;
			break;
		case HSC_TOKEN_LESS_THAN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(LESS_THAN);
			*precedence_out = 6;
			break;
		case HSC_TOKEN_LESS_THAN_OR_EQUAL:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(LESS_THAN_OR_EQUAL);
			*precedence_out = 6;
			break;
		case HSC_TOKEN_GREATER_THAN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(GREATER_THAN);
			*precedence_out = 6;
			break;
		case HSC_TOKEN_GREATER_THAN_OR_EQUAL:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(GREATER_THAN_OR_EQUAL);
			*precedence_out = 6;
			break;
		case HSC_TOKEN_LOGICAL_EQUAL:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(EQUAL);
			*precedence_out = 7;
			break;
		case HSC_TOKEN_LOGICAL_NOT_EQUAL:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(NOT_EQUAL);
			*precedence_out = 7;
			break;
		case HSC_TOKEN_AMPERSAND:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_AND);
			*precedence_out = 8;
			break;
		case HSC_TOKEN_CARET:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_XOR);
			*precedence_out = 9;
			break;
		case HSC_TOKEN_PIPE:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_OR);
			*precedence_out = 10;
			break;
		case HSC_TOKEN_LOGICAL_AND:
			*binary_op_type_out = HSC_EXPR_TYPE_LOGICAL_AND;
			*precedence_out = 11;
			break;
		case HSC_TOKEN_LOGICAL_OR:
			*binary_op_type_out = HSC_EXPR_TYPE_LOGICAL_OR;
			*precedence_out = 12;
			break;
		case HSC_TOKEN_EQUAL:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(ASSIGN);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_ADD_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(ADD);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_SUBTRACT_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(SUBTRACT);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_MULTIPLY_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(MULTIPLY);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_DIVIDE_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(DIVIDE);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_MODULO_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(MODULO);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_BIT_SHIFT_LEFT_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_BIT_AND_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_AND);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_BIT_XOR_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_XOR);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		case HSC_TOKEN_BIT_OR_ASSIGN:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(BIT_OR);
			*precedence_out = 14;
			*is_assignment_out = true;
			break;
		default:
			*binary_op_type_out = HSC_EXPR_TYPE_NONE;
			*precedence_out = 0;
			break;
	}
}

bool hsc_data_type_check_compatible(HscAstGen* astgen, HscDataType target_data_type, HscDataType source_data_type) {
	if (target_data_type == source_data_type) {
		return true;
	}

	switch (target_data_type) {
		case HSC_DATA_TYPE_GENERIC_SCALAR:
			if (astgen->generic_data_type_state.scalar != HSC_DATA_TYPE_VOID && astgen->generic_data_type_state.scalar != source_data_type) {
				return false;
			}
			if (
				source_data_type != HSC_DATA_TYPE_VOID &&
				HSC_DATA_TYPE_BASIC_START <= source_data_type &&
				source_data_type < HSC_DATA_TYPE_BASIC_END
			) {
				astgen->generic_data_type_state.scalar = source_data_type;
				return true;
			}
			break;
		case HSC_DATA_TYPE_GENERIC_VEC2:
			if (astgen->generic_data_type_state.vec2 != HSC_DATA_TYPE_VOID && astgen->generic_data_type_state.vec2 != source_data_type) {
				return false;
			}
			if (HSC_DATA_TYPE_VEC2_START <= source_data_type && source_data_type < HSC_DATA_TYPE_VEC2_END) {
				astgen->generic_data_type_state.vec2 = source_data_type;
				return true;
			}
			break;
		case HSC_DATA_TYPE_GENERIC_VEC3:
			if (astgen->generic_data_type_state.vec3 != HSC_DATA_TYPE_VOID && astgen->generic_data_type_state.vec3 != source_data_type) {
				return false;
			}
			if (HSC_DATA_TYPE_VEC3_START <= source_data_type && source_data_type < HSC_DATA_TYPE_VEC3_END) {
				astgen->generic_data_type_state.vec3 = source_data_type;
				return true;
			}
			break;
		case HSC_DATA_TYPE_GENERIC_VEC4:
			if (astgen->generic_data_type_state.vec4 != HSC_DATA_TYPE_VOID && astgen->generic_data_type_state.vec4 != source_data_type) {
				return false;
			}
			if (HSC_DATA_TYPE_VEC4_START <= source_data_type && source_data_type < HSC_DATA_TYPE_VEC4_END) {
				astgen->generic_data_type_state.vec4 = source_data_type;
				return true;
			}
			break;
	}

	return false;
}

void hsc_astgen_error_data_type_mismatch(HscAstGen* astgen, HscLocation* other_location, HscDataType target_data_type, HscDataType source_data_type) {
	HscString target_data_type_name = hsc_data_type_string(astgen, target_data_type);
	HscString source_data_type_name = hsc_data_type_string(astgen, source_data_type);
	hsc_astgen_error_2(astgen, other_location, "type mismatch '%.*s' is does not implicitly cast to '%.*s'", (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
}

void hsc_data_type_ensure_compatible(HscAstGen* astgen, HscLocation* other_location, HscDataType target_data_type, HscDataType source_data_type) {
	if (!hsc_data_type_check_compatible(astgen, target_data_type, source_data_type)) {
		hsc_astgen_error_data_type_mismatch(astgen, other_location, target_data_type, source_data_type);
	}
}

bool hsc_astgen_check_function_args(HscAstGen* astgen, HscFunction* function, HscExpr* call_args_expr, U32 args_count, bool report_errors) {
	if (args_count < function->params_count) {
		if (report_errors) {
			HscString string = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
			hsc_astgen_error_2(astgen, &function->location, "not enough arguments, expected '%u' but got '%u' for '%.*s'", function->params_count, args_count, (int)string.size, string.data);
		}
		return false;
	} else if (args_count > function->params_count) {
		if (report_errors) {
			HscString string = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
			hsc_astgen_error_2(astgen, &function->location, "too many arguments, expected '%u' but got '%u' for '%.*s'", function->params_count, args_count, (int)string.size, string.data);
		}
		return false;
	} else {
		HscLocalVariable* params = &astgen->function_params_and_local_variables[function->params_start_idx];
		U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];
		HscExpr* arg_expr = call_args_expr;
		bool result = true;
		astgen->generic_data_type_state = (HscGenericDataTypeState){0};
		for (U32 i = 0; i < args_count; i += 1) {
			arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
			HscLocalVariable* param = &params[i];

			if (!hsc_data_type_check_compatible(astgen, param->data_type, arg_expr->data_type)) {
				if (report_errors) {
					hsc_astgen_error_data_type_mismatch(astgen, &function->location, arg_expr->data_type, param->data_type);
					result = false;
				} else {
					return false;
				}
			}
		}
		return result;
	}
}

U32 hsc_local_variable_to_string(HscAstGen* astgen, HscLocalVariable* local_variable, char* buf, U32 buf_size, bool color) {
	char* fmt;
	if (color) {
		fmt = "\x1b[1;94m%.*s \x1b[97m%.*s\x1b[0m";
	} else {
		fmt = "%.*s %.*s";
	}

	HscString type_name = hsc_data_type_string(astgen, local_variable->data_type);
	HscString variable_name = hsc_string_table_get(&astgen->string_table, local_variable->identifier_string_id);
	return snprintf(buf, buf_size, fmt, (int)type_name.size, type_name.data, (int)variable_name.size, variable_name.data);
}

U32 hsc_function_to_string(HscAstGen* astgen, HscFunction* function, char* buf, U32 buf_size, bool color) {
	char* function_fmt;
	if (color) {
		function_fmt = "\x1b[1;94m%.*s \x1b[97m%.*s\x1b[0m";
	} else {
		function_fmt = "%.*s %.*s";
	}

	HscString return_type_name = hsc_data_type_string(astgen, function->return_data_type);
	HscString name = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
	U32 cursor = 0;
	cursor += snprintf(buf + cursor, buf_size - cursor, function_fmt, (int)return_type_name.size, return_type_name.data, (int)name.size, name.data);
	cursor += snprintf(buf + cursor, buf_size - cursor, "(");
	for (U32 param_idx = 0; param_idx < function->params_count; param_idx += 1) {
		HscLocalVariable* param = &astgen->function_params_and_local_variables[function->params_start_idx + param_idx];
		cursor += hsc_local_variable_to_string(astgen, param, buf + cursor, buf_size - cursor, color);
		if (param_idx + 1 < function->params_count) {
			cursor += snprintf(buf + cursor, buf_size - cursor, ", ");
		}
	}
	cursor += snprintf(buf + cursor, buf_size - cursor, ")");
	return cursor;
}

bool hsc_astgen_check_function_args_and_resolve_intrinsic_overload(HscAstGen* astgen, HscFunctionId* function_id_mut, HscExpr* call_args_expr, U32 args_count, U32 intrinsic_function_token_idx) {
	if (function_id_mut->idx_plus_one < HSC_FUNCTION_ID_INTRINSIC_END) {
		U32 overloads_count = hsc_intrinsic_function_overloads_count[function_id_mut->idx_plus_one];
		HSC_DEBUG_ASSERT(overloads_count, "internal error: intrinsic function should not have 0 overloads");

		U32 found_overload_id = 0;
		for (U32 i = 0; i < overloads_count; i += 1) {
			HscFunction* function = &astgen->functions[function_id_mut->idx_plus_one + i - 1];
			if (hsc_astgen_check_function_args(astgen, function, call_args_expr, args_count, false)) {
				found_overload_id = i + 1;
			}
		}

		if (found_overload_id) {
			function_id_mut->idx_plus_one += found_overload_id - 1;
			return true;
		}

		char* info;
		if (astgen->print_color) {
			info = "\x1b[1;97mhere are the available candidates:\n";
		} else {
			info = "here are the available candidates:\n";
		}

		char buf[2048];
		U32 cursor = 0;
		cursor += snprintf(buf + cursor, sizeof(buf) - cursor, "%s", info);
		for (U32 i = 0; i < overloads_count; i += 1) {
			HscFunction* function = &astgen->functions[function_id_mut->idx_plus_one + i - 1];
			cursor += snprintf(buf + cursor, sizeof(buf) - cursor, "\t");
			cursor += hsc_function_to_string(astgen, function, buf + cursor, sizeof(buf) - cursor, astgen->print_color);
			cursor += snprintf(buf + cursor, sizeof(buf) - cursor, "\n");
		}

		astgen->error_info = buf;
		astgen->token_read_idx = intrinsic_function_token_idx;
		HscFunction* function = &astgen->functions[function_id_mut->idx_plus_one - 1];
		HscString name = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
		hsc_astgen_error_1(astgen, "could not find the overload for the '%.*s' intrinsic function", (int)name.size, name.data);
		return false;
	} else {
		HscFunction* function = &astgen->functions[function_id_mut->idx_plus_one - 1];
		return hsc_astgen_check_function_args(astgen, function, call_args_expr, args_count, true);
	}
}

HscExpr* hsc_astgen_generate_call_expr(HscAstGen* astgen, HscExpr* function_expr) {
	U32 args_count = 0;
	HscExpr* call_args_expr = NULL;

	HscFunctionId function_id = { .idx_plus_one = function_expr->function.id };

	HscToken token = hsc_token_peek(astgen);
	U32 intrinsic_function_token_idx = astgen->token_read_idx - 2;
	if (token == HSC_TOKEN_PARENTHESIS_CLOSE) {
		hsc_astgen_check_function_args_and_resolve_intrinsic_overload(astgen, &function_id, call_args_expr, args_count, intrinsic_function_token_idx);
		return NULL;
	}

	//
	// scan ahead an count how many arguments we are going to have.
	args_count = 1;
	U32 ahead_by = 1;
	U32 parenthesis_open = 0; // to avoid counting the comma operator
	while (1) {
		HscToken token = hsc_token_peek_ahead(astgen, ahead_by);
		switch (token) {
			case HSC_TOKEN_EOF:
				goto END_ARG_COUNT;
			case HSC_TOKEN_COMMA:
				args_count += 1;
				break;
			case HSC_TOKEN_PARENTHESIS_OPEN:
				parenthesis_open += 1;
				break;
			case HSC_TOKEN_PARENTHESIS_CLOSE:
				if (parenthesis_open == 0) {
					goto END_ARG_COUNT;
				}
				parenthesis_open -= 1;
				break;
		}
		ahead_by += 1;
	}
END_ARG_COUNT: {}

	//
	// preallocating enough room in and after the call_args_expr to store relative indices to the next args for each argument expression
	U32 required_header_expressions = ((args_count + 2) / 8) + 1;
	call_args_expr = hsc_astgen_alloc_expr_many(astgen, required_header_expressions);
	call_args_expr->type = HSC_EXPR_TYPE_CALL_ARG_LIST;
	call_args_expr->is_stmt_block_entry = true;
	((U8*)call_args_expr)[1] = args_count;
	U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];

	HscExpr* prev_arg_expr = call_args_expr;

	U32 arg_idx = 0;
	token = hsc_token_peek(astgen);
	bool is_constant = true;
	while (1) {
		HscExpr* arg_expr = hsc_astgen_generate_expr(astgen, 0);
		next_arg_expr_rel_indices[arg_idx] = arg_expr - prev_arg_expr;
		arg_idx += 1;
		is_constant &= arg_expr->type == HSC_EXPR_TYPE_CONSTANT;

		token = hsc_token_peek(astgen);
		if (token != HSC_TOKEN_COMMA) {
			if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
				hsc_astgen_error_1(astgen, "expected a ',' to declaring more function arguments or a ')' to finish declaring function arguments");
			}
			token = hsc_token_next(astgen);
			break;
		}
		token = hsc_token_next(astgen);
		prev_arg_expr = arg_expr;
	}

	hsc_astgen_check_function_args_and_resolve_intrinsic_overload(astgen, &function_id, call_args_expr, args_count, intrinsic_function_token_idx);

	HscDataType return_data_type = astgen->functions[function_id.idx_plus_one - 1].return_data_type;
	return_data_type = hsc_data_type_resolve_generic(astgen, return_data_type);

	if (hsc_opt_is_enabled(&astgen->opts, HSC_OPT_CONSTANT_FOLDING) && is_constant) {
		bool is_single = false;
		U32 componments_count = 0;
		switch (function_id.idx_plus_one) {
			case HSC_FUNCTION_ID_VEC2_SINGLE: is_single = true; componments_count = 2; break;
			case HSC_FUNCTION_ID_VEC2_MULTI: is_single = false; componments_count = 2; break;
			case HSC_FUNCTION_ID_VEC3_SINGLE: is_single = true; componments_count = 3; break;
			case HSC_FUNCTION_ID_VEC3_MULTI: is_single = false; componments_count = 3; break;
			case HSC_FUNCTION_ID_VEC4_SINGLE: is_single = true; componments_count = 4; break;
			case HSC_FUNCTION_ID_VEC4_MULTI: is_single = false; componments_count = 4; break;
				break;
		}
		if (componments_count) {
			hsc_constant_table_deduplicate_composite_start(&astgen->constant_table, astgen, return_data_type);
			HscExpr* arg_expr = call_args_expr;
			U32 args_count = ((U8*)call_args_expr)[1];
			U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];
			for (U32 i = 0; i < args_count; i += 1) {
				arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
				HscConstantId constant_id = { .idx_plus_one = arg_expr->constant.id };
				U32 repeat_count = is_single ? componments_count : 1;
				for (U32 j = 0; j < repeat_count; j += 1) {
					hsc_constant_table_deduplicate_composite_add(&astgen->constant_table, constant_id);
				}
			}
			HscConstantId constant_id = hsc_constant_table_deduplicate_composite_end(&astgen->constant_table);

			//
			// recycle the function expression and change it into a constant
			function_expr->type = HSC_EXPR_TYPE_CONSTANT;
			function_expr->constant.id = constant_id.idx_plus_one;
			function_expr->data_type = return_data_type;
			return function_expr;
		}
	}

	function_expr->function.id = function_id.idx_plus_one;

	HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CALL);
	expr->binary.left_expr_rel_idx = expr - function_expr;
	expr->binary.right_expr_rel_idx = expr - call_args_expr;
	expr->data_type = return_data_type;
	return expr;
}

HscExpr* hsc_astgen_generate_expr(HscAstGen* astgen, U32 min_precedence) {
	U32 expr_idx = astgen->exprs_count;
	U32 callee_token_idx = astgen->token_read_idx;
	HscExpr* left_expr = hsc_astgen_generate_unary_expr(astgen);

	while (1) {
		HscExprType binary_op_type;
		U32 precedence;
		bool is_assignment;
		hsc_astgen_generate_binary_op(astgen, &binary_op_type, &precedence, &is_assignment);
		if (binary_op_type == HSC_EXPR_TYPE_NONE || (min_precedence && min_precedence < precedence)) {
			return left_expr;
		}
		hsc_token_next(astgen);

		if (binary_op_type == HSC_EXPR_TYPE_CALL) {
			if (left_expr->type != HSC_EXPR_TYPE_FUNCTION) {
				HscLocation* other_location = &astgen->token_locations[callee_token_idx];
				hsc_astgen_error_2(astgen, other_location, "unexpected '(', this can only be used when the callee is a function");
			}

			left_expr = hsc_astgen_generate_call_expr(astgen, left_expr);
		} else {
			HscExpr* right_expr = hsc_astgen_generate_expr(astgen, precedence);

			HscDataType data_type;
			if (HSC_EXPR_TYPE_BINARY_OP(EQUAL) <= binary_op_type && binary_op_type <= HSC_EXPR_TYPE_LOGICAL_OR) {
				data_type = HSC_DATA_TYPE_BOOL;
			} else {
				if (!hsc_data_type_check_compatible(astgen, left_expr->data_type, right_expr->data_type)) {
					HscString left_data_type_name = hsc_data_type_string(astgen, left_expr->data_type);
					HscString right_data_type_name = hsc_data_type_string(astgen, right_expr->data_type);
					hsc_astgen_error_1(astgen, "type mismatch '%.*s' is does not implicitly cast to '%.*s'", (int)left_data_type_name.size, left_data_type_name.data, (int)right_data_type_name.size, right_data_type_name.data);
				}
				data_type = left_expr->data_type; // TODO make implicit conversions explicit in the AST and make the error above work correctly
			}

			if (
				hsc_opt_is_enabled(&astgen->opts, HSC_OPT_CONSTANT_FOLDING) &&
				left_expr->type == HSC_EXPR_TYPE_CONSTANT &&
				right_expr->type == HSC_EXPR_TYPE_CONSTANT
			) {
				HscConstantId left_constant_id = { .idx_plus_one = left_expr->constant.id };
				HscConstantId right_constant_id = { .idx_plus_one = right_expr->constant.id };
				HSC_ABORT("TODO CONSTANT FOLDING");
				//
				// combine left_expr and right_expr and store them in the left_expr
				//
			} else {
				HscExpr* expr = hsc_astgen_alloc_expr(astgen, binary_op_type);
				expr->binary.left_expr_rel_idx = expr - left_expr;
				expr->binary.right_expr_rel_idx = right_expr ? expr - right_expr : 0;
				expr->binary.is_assignment = is_assignment;
				expr->data_type = data_type;
				left_expr = expr;
			}
		}
	}
}

void hsc_astgen_ensure_semicolon(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_SEMICOLON) {
		hsc_astgen_error_1(astgen, "missing ';' to end the statement");
	}
	hsc_token_next(astgen);
}

bool hsc_stmt_has_return(HscExpr* stmt) {
	if (stmt->type == HSC_EXPR_TYPE_STMT_RETURN) {
		return true;
	} else if (stmt->type == HSC_EXPR_TYPE_STMT_BLOCK && stmt->stmt_block.has_return_stmt) {
		return true;
	} else if (stmt->type == HSC_EXPR_TYPE_STMT_IF && stmt[stmt->if_.true_stmt_rel_idx].if_aux.true_and_false_stmts_have_return_stmt) {
		return true;
	}
	return false;
}

HscExpr* hsc_astgen_generate_cond_expr(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_OPEN) {
		hsc_astgen_error_1(astgen, "expected a '(' for the if statement condition");
	}
	token = hsc_token_next(astgen);

	HscExpr* cond_expr = hsc_astgen_generate_expr(astgen, 0);
	if (!hsc_data_type_is_condition(astgen, cond_expr->data_type)) {
		HscString data_type_name = hsc_data_type_string(astgen, cond_expr->data_type);
		hsc_astgen_error_1(astgen, "the condition expression must be convertable to a 'bool' but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
		hsc_astgen_error_1(astgen, "expected a ')' to finish the if statement condition");
	}
	token = hsc_token_next(astgen);
	return cond_expr;
}

HscExpr* hsc_astgen_generate_variable_decl(HscAstGen* astgen, HscExpr* type_expr) {
	HSC_DEBUG_ASSERT(type_expr->type == HSC_EXPR_TYPE_DATA_TYPE, "expected a data type expression here");

	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_IDENT) {
		hsc_astgen_error_1(astgen, "expected an identifier for a variable declaration");
	}
	HscTokenValue identifier_value = hsc_token_value_next(astgen);

	U32 existing_variable_id = hsc_astgen_variable_stack_find(astgen, identifier_value.string_id);
	if (existing_variable_id) {
		HscLocation* other_location = NULL; // TODO: location of existing variable
		HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
		hsc_astgen_error_2(astgen, other_location, "redefinition of '%.*s' local variable identifier", (int)string.size, string.data);
	}
	U32 variable_idx = hsc_astgen_variable_stack_add(astgen, identifier_value.string_id);

	HscLocalVariable* local_variable = &astgen->function_params_and_local_variables[astgen->function_params_and_local_variables_count];
	astgen->function_params_and_local_variables_count += 1;
	local_variable->identifier_string_id = identifier_value.string_id;
	local_variable->identifier_token_idx = astgen->token_read_idx;
	local_variable->data_type = type_expr->data_type;
	astgen->stmt_block->stmt_block.local_variables_count += 1;

	token = hsc_token_next(astgen);
	switch (token) {
		case HSC_TOKEN_SEMICOLON:
			hsc_token_next(astgen);
			break;
		case HSC_TOKEN_EQUAL: {
			HscExpr* left_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_LOCAL_VARIABLE);
			left_expr->variable.idx = variable_idx;
			left_expr->data_type = type_expr->data_type;
			hsc_token_next(astgen);

			HscExpr* right_expr = hsc_astgen_generate_expr(astgen, 0);
			HscLocation* other_location = NULL; // TODO
			hsc_data_type_ensure_compatible(astgen, other_location, left_expr->data_type, right_expr->data_type);

			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_BINARY_OP(ASSIGN));
			stmt->binary.is_assignment = true;
			stmt->binary.left_expr_rel_idx = stmt - left_expr;
			stmt->binary.right_expr_rel_idx = stmt - right_expr;

			return stmt;
		};
		default:
			hsc_astgen_error_1(astgen, "'expected a ';' to end the declaration or a '=' to assign to the new variable");
	}

	return NULL;
}

HscExpr* hsc_astgen_generate_stmt(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	switch (token) {
		case HSC_TOKEN_CURLY_OPEN: {
			hsc_astgen_variable_stack_open(astgen);

			HscExpr* prev_stmt = NULL;

			HscExpr* stmt_block = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_BLOCK);
			U32 stmts_count = 0;

			stmt_block->is_stmt_block_entry = true;
			stmt_block->stmt_block.local_variables_count = 0;
			HscExpr* prev_stmt_block = stmt_block;
			astgen->stmt_block = stmt_block;

			token = hsc_token_next(astgen);
			while (token != HSC_TOKEN_CURLY_CLOSE) {
				HscExpr* stmt = hsc_astgen_generate_stmt(astgen);
				if (stmt == NULL) {
					continue;
				}
				stmt->is_stmt_block_entry = true;
				stmt_block->stmt_block.has_return_stmt |= stmt->type == HSC_EXPR_TYPE_STMT_RETURN;

				if (prev_stmt) {
					prev_stmt->next_expr_rel_idx = stmt - prev_stmt;
				} else {
					stmt_block->stmt_block.first_expr_rel_idx = stmt - stmt_block;
				}

				stmts_count += 1;
				token = hsc_token_peek(astgen);
				prev_stmt = stmt;
			}

			stmt_block->stmt_block.stmts_count = stmts_count;
			hsc_astgen_variable_stack_close(astgen);
			token = hsc_token_next(astgen);
			astgen->stmt_block = prev_stmt_block;
			return stmt_block;
		};
		case HSC_TOKEN_KEYWORD_RETURN: {
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_RETURN);
			hsc_token_next(astgen);
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			stmt->unary.expr_rel_idx = expr - stmt;
			hsc_astgen_ensure_semicolon(astgen);
			return stmt;
		};
		case HSC_TOKEN_KEYWORD_IF: {
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_IF);
			hsc_token_next(astgen);
			HscExpr* cond_expr = hsc_astgen_generate_cond_expr(astgen);

			HscExpr* true_stmt = hsc_astgen_generate_stmt(astgen);
			true_stmt->is_stmt_block_entry = true;

			token = hsc_token_peek(astgen);
			HscExpr* false_stmt = NULL;
			if (token == HSC_TOKEN_KEYWORD_ELSE) {
				token = hsc_token_next(astgen);
				if (token != HSC_TOKEN_KEYWORD_IF && token != HSC_TOKEN_CURLY_OPEN) {
					hsc_astgen_error_1(astgen, "expected either 'if' or '{' to follow the 'else' keyword");
				}
				false_stmt = hsc_astgen_generate_stmt(astgen);
				false_stmt->is_stmt_block_entry = true;
			}

			stmt->type = HSC_EXPR_TYPE_STMT_IF;
			stmt->if_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->if_.true_stmt_rel_idx = true_stmt - stmt;

			true_stmt->if_aux.false_stmt_rel_idx = false_stmt ? false_stmt - true_stmt : 0;
			true_stmt->if_aux.true_and_false_stmts_have_return_stmt = false;
			if (false_stmt) {
				true_stmt->if_aux.true_and_false_stmts_have_return_stmt = hsc_stmt_has_return(true_stmt) && hsc_stmt_has_return(false_stmt);
			}
			return stmt;
		};
		case HSC_TOKEN_KEYWORD_SWITCH: {
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_SWITCH);
			token = hsc_token_next(astgen);

			HscExpr* cond_expr;
			{
				if (token != HSC_TOKEN_PARENTHESIS_OPEN) {
					hsc_astgen_error_1(astgen, "expected a '(' for the if statement condition");
				}
				token = hsc_token_next(astgen);

				cond_expr = hsc_astgen_generate_expr(astgen, 0);
				if (
					cond_expr->data_type < HSC_DATA_TYPE_U8 ||
					cond_expr->data_type > HSC_DATA_TYPE_S64
				) {
					HscString data_type_name = hsc_data_type_string(astgen, cond_expr->data_type);
					hsc_astgen_error_1(astgen, "switch condition expression must be convertable to a integer type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
				}

				token = hsc_token_peek(astgen);
				if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
					hsc_astgen_error_1(astgen, "expected a ')' to finish the if statement condition");
				}
				token = hsc_token_next(astgen);
			}
			stmt->switch_.cond_expr_rel_idx = cond_expr - stmt;

			if (token != HSC_TOKEN_CURLY_OPEN) {
				hsc_astgen_error_1(astgen, "expected a '{' to begin the switch statement");
			}

			HscSwitchState prev_switch_state = astgen->switch_state;

			astgen->switch_state.switch_stmt = stmt;
			astgen->switch_state.first_switch_case = NULL;
			astgen->switch_state.prev_switch_case = NULL;
			astgen->switch_state.default_switch_case = NULL;
			astgen->switch_state.switch_condition_type = cond_expr->data_type;
			astgen->switch_state.case_stmts_count = 0;

			HscExpr* block_stmt = hsc_astgen_generate_stmt(astgen);
			block_stmt->is_stmt_block_entry = true;
			block_stmt->switch_aux.case_stmts_count = astgen->switch_state.case_stmts_count;
			block_stmt->switch_aux.first_case_expr_rel_idx = astgen->switch_state.first_switch_case ? astgen->switch_state.first_switch_case - block_stmt : 0;

			stmt->switch_.block_expr_rel_idx = block_stmt - stmt;
			stmt->alt_next_expr_rel_idx = astgen->switch_state.default_switch_case ? astgen->switch_state.default_switch_case - stmt : 0;

			astgen->switch_state = prev_switch_state;
			return stmt;
		};
		case HSC_TOKEN_KEYWORD_WHILE: {
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_WHILE);
			hsc_token_next(astgen);

			HscExpr* cond_expr = hsc_astgen_generate_cond_expr(astgen);

			bool prev_is_in_loop = astgen->is_in_loop;
			astgen->is_in_loop = true;
			HscExpr* loop_stmt = hsc_astgen_generate_stmt(astgen);
			loop_stmt->is_stmt_block_entry = true;
			astgen->is_in_loop = prev_is_in_loop;

			stmt->while_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->while_.loop_stmt_rel_idx = loop_stmt - stmt;

			return stmt;
		};
		case HSC_TOKEN_KEYWORD_FOR: {
			hsc_astgen_variable_stack_open(astgen);

			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_FOR);
			token = hsc_token_next(astgen);

			if (token != HSC_TOKEN_PARENTHESIS_OPEN) {
				hsc_astgen_error_1(astgen, "expected a '(' for the if statement condition");
			}
			token = hsc_token_next(astgen);

			HscExpr* init_expr = hsc_astgen_generate_expr(astgen, 0);
			if (init_expr->type == HSC_EXPR_TYPE_DATA_TYPE) {
				init_expr = hsc_astgen_generate_variable_decl(astgen, init_expr);
			}
			hsc_astgen_ensure_semicolon(astgen);

			HscExpr* cond_expr = hsc_astgen_generate_expr(astgen, 0);
			if (!hsc_data_type_is_condition(astgen, cond_expr->data_type)) {
				HscString data_type_name = hsc_data_type_string(astgen, cond_expr->data_type);
				hsc_astgen_error_1(astgen, "the condition expression must be convertable to a 'bool' but got '%.*s'", (int)data_type_name.size, data_type_name.data);
			}
			hsc_astgen_ensure_semicolon(astgen);

			HscExpr* inc_expr = hsc_astgen_generate_expr(astgen, 0);

			token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
				hsc_astgen_error_1(astgen, "expected a ')' to finish the if statement condition");
			}
			token = hsc_token_next(astgen);

			bool prev_is_in_loop = astgen->is_in_loop;
			astgen->is_in_loop = true;
			HscExpr* loop_stmt = hsc_astgen_generate_stmt(astgen);
			loop_stmt->is_stmt_block_entry = true;
			astgen->is_in_loop = prev_is_in_loop;

			stmt->for_.init_expr_rel_idx = init_expr - stmt;
			stmt->for_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->for_.inc_expr_rel_idx = inc_expr - stmt;
			stmt->for_.loop_stmt_rel_idx = loop_stmt - stmt;

			hsc_astgen_variable_stack_close(astgen);
			return stmt;
		};
		case HSC_TOKEN_KEYWORD_CASE: {
			if (astgen->switch_state.switch_condition_type == HSC_DATA_TYPE_VOID) {
				hsc_astgen_error_1(astgen, "case statement must be inside a switch statement");
			}

			token = hsc_token_next(astgen);

			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			if (expr->type != HSC_EXPR_TYPE_CONSTANT) {
				hsc_astgen_error_1(astgen, "the value of a switch case statement must be a constant");
			}
			HscLocation* other_location = NULL; // TODO: the switch condition expr
			hsc_data_type_ensure_compatible(astgen, NULL, astgen->switch_state.switch_condition_type, expr->data_type);

			expr->type = HSC_EXPR_TYPE_STMT_CASE;
			expr->is_stmt_block_entry = true;
			expr->next_expr_rel_idx = 0;
			expr->alt_next_expr_rel_idx = 0;

			token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_COLON) {
				hsc_astgen_error_1(astgen, "':' must follow the constant of the case statement");
			}
			hsc_token_next(astgen);

			//
			// TODO: add this constant to a linear array with a location in a parallel array and check to see
			// if this constant has already been used in the switch case

			if (astgen->switch_state.prev_switch_case) {
				astgen->switch_state.prev_switch_case->alt_next_expr_rel_idx = expr - astgen->switch_state.prev_switch_case;
			} else {
				astgen->switch_state.first_switch_case = expr;
			}

			astgen->switch_state.case_stmts_count += 1;
			astgen->switch_state.prev_switch_case = expr;
			return expr;
		};
		case HSC_TOKEN_KEYWORD_DEFAULT: {
			if (astgen->switch_state.switch_condition_type == HSC_DATA_TYPE_VOID) {
				hsc_astgen_error_1(astgen, "default case statement must be inside a switch statement");
			}
			if (astgen->switch_state.default_switch_case) {
				hsc_astgen_error_1(astgen, "default case statement has already been declared");
			}

			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_DEFAULT);
			stmt->is_stmt_block_entry = true;

			token = hsc_token_next(astgen);
			if (token != HSC_TOKEN_COLON) {
				hsc_astgen_error_1(astgen, "':' must follow the default keyword");
			}
			hsc_token_next(astgen);

			astgen->switch_state.default_switch_case = stmt;
			return stmt;
		};
		case HSC_TOKEN_KEYWORD_BREAK: {
			if (astgen->switch_state.switch_condition_type == HSC_DATA_TYPE_VOID && !astgen->is_in_loop) {
				hsc_astgen_error_1(astgen, "'break' can only be used within a switch statement, a for loop or a while loop");
			}
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_BREAK);
			stmt->is_stmt_block_entry = true;
			hsc_token_next(astgen);
			hsc_astgen_ensure_semicolon(astgen);
			return stmt;
		};
		case HSC_TOKEN_KEYWORD_CONTINUE: {
			if (astgen->switch_state.switch_condition_type == HSC_DATA_TYPE_VOID && !astgen->is_in_loop) {
				hsc_astgen_error_1(astgen, "'continue' can only be used within a switch statement, a for loop or a while loop");
			}
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_CONTINUE);
			stmt->is_stmt_block_entry = true;
			hsc_token_next(astgen);
			hsc_astgen_ensure_semicolon(astgen);
			return stmt;
		};
		default: {
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			if (expr->type == HSC_EXPR_TYPE_DATA_TYPE) {
				expr = hsc_astgen_generate_variable_decl(astgen, expr);
			}
			hsc_astgen_ensure_semicolon(astgen);
			return expr;
		};
	}
}

void hsc_astgen_generate_function(HscAstGen* astgen) {
	HSC_ASSERT(astgen->functions_count < astgen->functions_cap, "functions are full");
	HscFunction* function = &astgen->functions[astgen->functions_count];
	HscFunctionId function_id = { .idx_plus_one = astgen->functions_count };
	astgen->functions_count += 1;

	HscToken token = hsc_token_peek(astgen);
	U32 shader_stage_read_idx = astgen->token_read_idx;
	switch (token) {
		case HSC_TOKEN_KEYWORD_VERTEX:   function->shader_stage = HSC_FUNCTION_SHADER_STAGE_VERTEX; break;
		case HSC_TOKEN_KEYWORD_FRAGMENT: function->shader_stage = HSC_FUNCTION_SHADER_STAGE_FRAGMENT; break;
		default:                         function->shader_stage = HSC_FUNCTION_SHADER_STAGE_NONE; break;
	}

	if (function->shader_stage != HSC_FUNCTION_SHADER_STAGE_NONE) {
		token = hsc_token_next(astgen);
	}

	hsc_astgen_generate_data_type(astgen, &function->return_data_type);
	token = hsc_token_peek(astgen);

	if (token != HSC_TOKEN_IDENT) {
		HscLocation* other_location = &astgen->token_locations[shader_stage_read_idx];
		hsc_astgen_error_2(astgen, other_location, "expected an identifier for a function since a shader stage was used");
	}
	HscTokenValue identifier_value = hsc_token_value_next(astgen);
	function->identifier_string_id = identifier_value.string_id;

	HscDecl* decl_ptr;
	if (hsc_hash_table_find_or_insert(&astgen->global_declarations, identifier_value.string_id.idx_plus_one, &decl_ptr)) {
		HscLocation* other_location = hsc_decl_location(astgen, *decl_ptr);
		HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
		hsc_astgen_error_2(astgen, other_location, "redefinition of the '%.*s' identifier", (int)string.size, string.data);
	}
	*decl_ptr = HSC_DECL_FUNCTION(function_id);

	token = hsc_token_next(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_OPEN) {
		HscLocation* other_location = &astgen->token_locations[shader_stage_read_idx];
		hsc_astgen_error_2(astgen, other_location, "expected an '(' to start defining function parameters since a shader stage was used");
	}

	hsc_astgen_variable_stack_open(astgen);

	function->params_start_idx = astgen->function_params_and_local_variables_count;
	function->params_count = 0;
	token = hsc_token_next(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
		while (1) {
			HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_and_local_variables_count, astgen->function_params_and_local_variables_cap);
			HscLocalVariable* param = &astgen->function_params_and_local_variables[astgen->function_params_and_local_variables_count];
			function->params_count += 1;
			astgen->function_params_and_local_variables_count += 1;

			hsc_astgen_generate_data_type(astgen, &param->data_type);
			token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_IDENT) {
				// TODO replace error message U32 type with the actual type name of param->type
				hsc_astgen_error_1(astgen, "expected an identifier for a function parameter e.g. U32 param_identifier");
			}
			identifier_value = hsc_token_value_next(astgen);
			param->identifier_string_id = identifier_value.string_id;

			U32 existing_variable_id = hsc_astgen_variable_stack_find(astgen, identifier_value.string_id);
			if (existing_variable_id) {
				HscLocation* other_location = NULL; // TODO: location of existing variable
				HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
				hsc_astgen_error_2(astgen, other_location, "redefinition of '%.*s' local variable identifier", (int)string.size, string.data);
			}
			hsc_astgen_variable_stack_add(astgen, identifier_value.string_id);
			token = hsc_token_next(astgen);

			if (token != HSC_TOKEN_COMMA) {
				if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
					hsc_astgen_error_1(astgen, "expected a ',' to declaring more function parameters or a ')' to finish declaring function parameters");
				}
				token = hsc_token_next(astgen);
				break;
			}
			token = hsc_token_next(astgen);
		}
	}

	function->block_expr_id.idx_plus_one = 0;
	if (token == HSC_TOKEN_CURLY_OPEN) {
		HscExpr* expr = hsc_astgen_generate_stmt(astgen);
		function->block_expr_id.idx_plus_one = (expr - astgen->exprs) + 1;
	}

	hsc_astgen_variable_stack_close(astgen);
	function->local_variables_count = astgen->next_var_idx;
}

void hsc_astgen_generate(HscAstGen* astgen) {
	while (1) {
		HscToken token = hsc_token_peek(astgen);

		switch (token) {
			case HSC_TOKEN_KEYWORD_VERTEX:
			case HSC_TOKEN_KEYWORD_FRAGMENT:
				hsc_astgen_generate_function(astgen);
				break;
			case HSC_TOKEN_EOF:
				return;
			default:
				HSC_ABORT("TODO at scope see if this token is a type and varify it is a function, and support enums found token '%s'", hsc_token_strings[token]);
		}
	}
}

void hsc_astgen_variable_stack_open(HscAstGen* astgen) {
	HSC_ASSERT(astgen->variable_stack_count < astgen->variable_stack_cap, "variable stack is full");
	if (astgen->variable_stack_count == 0) {
		astgen->next_var_idx = 0;
	}
	astgen->variable_stack_strings[astgen->variable_stack_count].idx_plus_one = 0;
	astgen->variable_stack_var_indices[astgen->variable_stack_count] = U32_MAX;
	astgen->variable_stack_count += 1;
}

void hsc_astgen_variable_stack_close(HscAstGen* astgen) {
	while (astgen->variable_stack_count) {
		astgen->variable_stack_count -= 1;
		if (astgen->variable_stack_strings[astgen->variable_stack_count].idx_plus_one == 0) {
			break;
		}
	}
}

U32 hsc_astgen_variable_stack_add(HscAstGen* astgen, HscStringId string_id) {
	HSC_ASSERT(astgen->variable_stack_count < astgen->variable_stack_cap, "variable stack is full");
	U32 var_idx = astgen->next_var_idx;
	astgen->variable_stack_strings[astgen->variable_stack_count] = string_id;
	astgen->variable_stack_var_indices[astgen->variable_stack_count] = var_idx;
	astgen->variable_stack_count += 1;
	astgen->next_var_idx += 1;
	return var_idx;
}

U32 hsc_astgen_variable_stack_find(HscAstGen* astgen, HscStringId string_id) {
	HSC_DEBUG_ASSERT(string_id.idx_plus_one, "string id is null");
	for (U32 idx = astgen->variable_stack_count; idx-- > 0;) {
		if (astgen->variable_stack_strings[idx].idx_plus_one == string_id.idx_plus_one) {
			return astgen->variable_stack_var_indices[idx] + 1;
		}
	}
	return 0;
}

void hsc_tokens_print(HscAstGen* astgen, FILE* f) {
	uint32_t token_value_idx = 0;
	for (uint32_t i = 0; i < astgen->tokens_count; i += 1) {
		HscToken token = astgen->tokens[i];
		HscTokenValue value;
		HscString string;
		HscDataType data_type;
		switch (token) {
			case HSC_TOKEN_IDENT:
				value = astgen->token_values[token_value_idx];
				token_value_idx += 1;
				string = hsc_string_table_get(&astgen->string_table, value.string_id);
				fprintf(f, "%s -> %.*s\n", hsc_token_strings[token], (int)string.size, string.data);
				break;
			case HSC_TOKEN_LIT_U32: data_type = HSC_DATA_TYPE_U32; goto PRINT_LIT;
			case HSC_TOKEN_LIT_U64: data_type = HSC_DATA_TYPE_U64; goto PRINT_LIT;
			case HSC_TOKEN_LIT_S32: data_type = HSC_DATA_TYPE_S32; goto PRINT_LIT;
			case HSC_TOKEN_LIT_S64: data_type = HSC_DATA_TYPE_S64; goto PRINT_LIT;
			case HSC_TOKEN_LIT_F32: data_type = HSC_DATA_TYPE_F32; goto PRINT_LIT;
			case HSC_TOKEN_LIT_F64: data_type = HSC_DATA_TYPE_F64; goto PRINT_LIT;
PRINT_LIT:
				value = astgen->token_values[token_value_idx];
				hsc_constant_print(astgen, value.constant_id, stdout);
				fprintf(f, "\n");
				token_value_idx += 1;
				break;
			default:
				fprintf(f, "%s\n", hsc_token_strings[token]);
				break;
		}
	}
}

void hsc_astgen_print_expr(HscAstGen* astgen, HscExpr* expr, U32 indent, bool is_stmt, FILE* f) {
	static char* indent_chars = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	fprintf(f, "%.*s", indent, indent_chars);
	if (!expr->is_stmt_block_entry) {
		HscString data_type_name = hsc_data_type_string(astgen, expr->data_type);
		fprintf(f, "(%.*s)", (int)data_type_name.size, data_type_name.data);
	}

	const char* expr_name;
	switch (expr->type) {
		case HSC_EXPR_TYPE_CONSTANT: expr_name = "EXPR_CONSTANT"; goto CONSTANT;
		case HSC_EXPR_TYPE_STMT_CASE: expr_name = "STMT_CASE"; goto CONSTANT;
CONSTANT: {
			fprintf(f, "%s ", expr_name);
			HscConstantId constant_id = { .idx_plus_one = expr->constant.id };
			hsc_constant_print(astgen, constant_id, stdout);
			break;
		};
		case HSC_EXPR_TYPE_STMT_BLOCK: {
			U32 stmts_count = expr->stmt_block.stmts_count;
			fprintf(f, "STMT_BLOCK[%u] {\n", stmts_count);
			HscExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
			U32 local_variables_count = expr->stmt_block.local_variables_count;
			for (U32 i = 0; i < local_variables_count; i += 1) {
				char buf[1024];
				U32 local_variable_idx = astgen->print_variable_base_idx + i;
				HscLocalVariable* local_variable = &astgen->function_params_and_local_variables[astgen->print_function->params_start_idx + local_variable_idx];
				hsc_local_variable_to_string(astgen, local_variable, buf, sizeof(buf), false);
				fprintf(f, "%.*sLOCAL_VARIABLE(#%u): %s\n", indent + 1, indent_chars, local_variable_idx, buf);
			}
			astgen->print_variable_base_idx += local_variables_count;

			for (U32 i = 0; i < stmts_count; i += 1) {
				hsc_astgen_print_expr(astgen, stmt, indent + 1, true, f);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_FUNCTION: {
			HscFunction* function = &astgen->functions[expr->function.id - 1];
			char buf[1024];
			hsc_function_to_string(astgen, function, buf, sizeof(buf), false);
			fprintf(f, "EXPR_FUNCTION Function(#%u): %s", expr->function.id - 1, buf);
			break;
		};
		case HSC_EXPR_TYPE_STMT_RETURN: expr_name = "STMT_RETURN"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(LOGICAL_NOT): expr_name = "EXPR_LOGICAL_NOT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(BIT_NOT): expr_name = "EXPR_BIT_NOT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(PLUS): expr_name = "EXPR_PLUS"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(NEGATE): expr_name = "EXPR_NEGATE"; goto UNARY;
UNARY:
		{
			fprintf(f, "%s: {\n", expr_name);
			HscExpr* unary_expr = &expr[expr->unary.expr_rel_idx];
			hsc_astgen_print_expr(astgen, unary_expr, indent + 1, false, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_STMT_IF: {
			fprintf(f, "%s: {\n", "STMT_IF");

			HscExpr* cond_expr = &expr[expr->if_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, cond_expr, indent + 2, false, f);

			HscExpr* true_stmt = &expr[expr->if_.true_stmt_rel_idx];
			fprintf(f, "%.*sTRUE_STMT:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, true_stmt, indent + 2, false, f);

			if (true_stmt->if_aux.false_stmt_rel_idx) {
				HscExpr* false_stmt = &true_stmt[true_stmt->if_aux.false_stmt_rel_idx];
				fprintf(f, "%.*sFALSE_STMT:\n", indent + 1, indent_chars);
				hsc_astgen_print_expr(astgen, false_stmt, indent + 2, false, f);
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_STMT_SWITCH: {
			fprintf(f, "%s: {\n", "STMT_SWITCH");

			HscExpr* block_expr = &expr[expr->switch_.block_expr_rel_idx];
			hsc_astgen_print_expr(astgen, block_expr, indent + 1, false, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_STMT_WHILE: {
			fprintf(f, "%s: {\n", "STMT_WHILE");

			HscExpr* cond_expr = &expr[expr->while_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, cond_expr, indent + 2, false, f);

			HscExpr* loop_stmt = &expr[expr->while_.loop_stmt_rel_idx];
			fprintf(f, "%.*sLOOP_STMT:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, loop_stmt, indent + 2, false, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_STMT_FOR: {
			fprintf(f, "%s: {\n", "STMT_FOR");

			HscExpr* init_expr = &expr[expr->for_.init_expr_rel_idx];
			fprintf(f, "%.*sINIT_EXPR:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, init_expr, indent + 2, false, f);

			HscExpr* cond_expr = &expr[expr->for_.cond_expr_rel_idx];
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, cond_expr, indent + 2, false, f);

			HscExpr* inc_expr = &expr[expr->for_.inc_expr_rel_idx];
			fprintf(f, "%.*sINCREMENT_EXPR:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, inc_expr, indent + 2, false, f);

			HscExpr* loop_stmt = &expr[expr->for_.loop_stmt_rel_idx];
			fprintf(f, "%.*sLOOP_STMT:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, loop_stmt, indent + 2, false, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_STMT_DEFAULT: {
			fprintf(f, "%s:\n", "STMT_DEFAULT");
			break;
		};
		case HSC_EXPR_TYPE_STMT_BREAK: {
			fprintf(f, "%s:\n", "STMT_BREAK");
			break;
		};
		case HSC_EXPR_TYPE_STMT_CONTINUE: {
			fprintf(f, "%s:\n", "STMT_CONTINUE");
			break;
		};
		case HSC_EXPR_TYPE_BINARY_OP(ASSIGN): expr_name = "ASSIGN"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(ADD): expr_name = "ADD"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(SUBTRACT): expr_name = "SUBTRACT"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(MULTIPLY): expr_name = "MULTIPLY"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(DIVIDE): expr_name = "DIVIDE"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(MODULO): expr_name = "MODULO"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(BIT_AND): expr_name = "BIT_AND"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(BIT_OR): expr_name = "BIT_OR"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(BIT_XOR): expr_name = "BIT_XOR"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT): expr_name = "BIT_SHIFT_LEFT"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT): expr_name = "BIT_SHIFT_RIGHT"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(EQUAL): expr_name = "EQUAL"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(NOT_EQUAL): expr_name = "NOT_EQUAL"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(LESS_THAN): expr_name = "LESS_THAN"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(LESS_THAN_OR_EQUAL): expr_name = "LESS_THAN_OR_EQUAL"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(GREATER_THAN): expr_name = "GREATER_THAN"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(GREATER_THAN_OR_EQUAL): expr_name = "GREATER_THAN_OR_EQUAL"; goto BINARY;
		case HSC_EXPR_TYPE_LOGICAL_AND: expr_name = "LOGICAL_AND"; goto BINARY;
		case HSC_EXPR_TYPE_LOGICAL_OR: expr_name = "LOGICAL_OR"; goto BINARY;
		case HSC_EXPR_TYPE_CALL: expr_name = "CALL"; goto BINARY;
BINARY:
		{
			char* prefix = expr->binary.is_assignment ? "STMT_ASSIGN_" : "EXPR_";
			fprintf(f, "%s%s: {\n", prefix, expr_name);
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			hsc_astgen_print_expr(astgen, left_expr, indent + 1, false, f);
			hsc_astgen_print_expr(astgen, right_expr, indent + 1, false, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_CALL_ARG_LIST: {
			fprintf(f, "EXPR_CALL_ARG_LIST: {\n");
			HscExpr* arg_expr = expr;
			U32 args_count = ((U8*)expr)[1];
			U8* next_arg_expr_rel_indices = &((U8*)expr)[2];
			for (U32 i = 0; i < args_count; i += 1) {
				arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
				hsc_astgen_print_expr(astgen, arg_expr, indent + 1, false, f);
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_LOCAL_VARIABLE: {
			char buf[1024];
			HscLocalVariable* local_variable = &astgen->function_params_and_local_variables[astgen->print_function->params_start_idx + expr->variable.idx];
			hsc_local_variable_to_string(astgen, local_variable, buf, sizeof(buf), false);
			fprintf(f, "LOCAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		default:
			HSC_ABORT("unhandle expr type %u\n", expr->type);
	}
	fprintf(f, "\n");
}

void hsc_astgen_print_ast(HscAstGen* astgen, FILE* f) {
	for (U32 function_idx = 0; function_idx < astgen->functions_count; function_idx += 1) {
		HscFunction* function = &astgen->functions[function_idx];
		if (function->identifier_string_id.idx_plus_one == 0) {
			continue;
		}
		HscString name = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
		HscString return_data_type_name = hsc_data_type_string(astgen, function->return_data_type);
		fprintf(f, "Function(#%u): %.*s {\n", function_idx, (int)name.size, name.data);
		fprintf(f, "\treturn_type: %.*s\n", (int)return_data_type_name.size, return_data_type_name.data);
		fprintf(f, "\tshader_stage: %s\n", hsc_function_shader_stage_strings[function->shader_stage]);
		if (function->params_count) {
			fprintf(f, "\tparams[%u]: {\n", function->params_count);
			for (U32 param_idx = 0; param_idx < function->params_count; param_idx += 1) {
				HscLocalVariable* param = &astgen->function_params_and_local_variables[function->params_start_idx + param_idx];
				HscString type_name = hsc_data_type_string(astgen, param->data_type);
				HscString param_name = hsc_string_table_get(&astgen->string_table, param->identifier_string_id);
				fprintf(f, "\t\t%.*s %.*s\n", (int)type_name.size, type_name.data, (int)param_name.size, param_name.data);
			}
			fprintf(f, "\t}\n");
		}
		if (function->block_expr_id.idx_plus_one) {
			astgen->print_function = function;
			astgen->print_variable_base_idx = function->params_count;
			HscExpr* expr = &astgen->exprs[function->block_expr_id.idx_plus_one - 1];
			hsc_astgen_print_expr(astgen, expr, 1, true, f);
		}
		fprintf(f, "}\n");
	}
}

// ===========================================
//
//
// IR
//
//
// ===========================================

void hsc_ir_init(HscIR* ir) {
	ir->functions = HSC_ALLOC_ARRAY(HscIRFunction, 8192);
	HSC_ASSERT(ir->functions, "out of memory");
	ir->basic_blocks = HSC_ALLOC_ARRAY(HscIRBasicBlock, 8192);
	HSC_ASSERT(ir->basic_blocks, "out of memory");
	ir->values = HSC_ALLOC_ARRAY(HscIRValue, 8192);
	HSC_ASSERT(ir->values, "out of memory");
	ir->instructions = HSC_ALLOC_ARRAY(HscIRInstr, 8192);
	HSC_ASSERT(ir->instructions, "out of memory");
	ir->operands = HSC_ALLOC_ARRAY(HscIROperand, 8192);
	HSC_ASSERT(ir->operands, "out of memory");
	ir->functions_cap = 8192;
	ir->basic_blocks_cap = 8192;
	ir->values_cap = 8192;
	ir->instructions_cap = 8192;
	ir->operands_cap = 8192;
}

HscIRBasicBlock* hsc_ir_add_basic_block(HscIR* ir, HscIRFunction* ir_function) {
	HscIRBasicBlock* basic_block = &ir->basic_blocks[ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count];
	ir->basic_blocks_count += 1;
	ir_function->basic_blocks_count += 1;
	basic_block->instructions_start_idx = ir_function->instructions_count;
	return basic_block;
}

U16 hsc_ir_add_value(HscIR* ir, HscIRFunction* ir_function, HscDataType data_type) {
	HscIRValue* value = &ir->values[ir_function->values_start_idx + (U32)ir_function->values_count];
	value->data_type = data_type;
	value->defined_instruction_idx = ir_function->instructions_count - 1;
	value->last_used_instruction_idx = ir_function->instructions_count - 1;
	ir->values_count += 1;
	U16 value_idx = ir_function->values_count;
	ir_function->values_count += 1;
	return value_idx;
}

void hsc_ir_add_instruction(HscIR* ir, HscIRFunction* ir_function, HscIROpCode op_code, HscIROperand* operands, U32 operands_count) {
	HscIRInstr* instruction = &ir->instructions[ir_function->instructions_start_idx + (U32)ir_function->instructions_count];
	instruction->op_code = op_code;
	instruction->operands_start_idx = (operands - ir->operands) - ir_function->operands_start_idx;
	instruction->operands_count = operands_count;
	HscIRBasicBlock* basic_block = &ir->basic_blocks[ir->basic_blocks_count - 1];
	basic_block->instructions_count += 1;
	ir->instructions_count += 1;
	ir_function->instructions_count += 1;
#if HSC_DEBUG_ASSERTIONS
	//
	// TODO validation
	switch (instruction->op_code) {
		default:break;
	}
#endif // HSC_DEBUG_ASSERTIONS
}

HscIROperand* hsc_ir_add_operands_many(HscIR* ir, HscIRFunction* ir_function, U32 amount) {
	HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)ir_function->operands_count];
	ir->operands_count += amount;
	ir_function->operands_count += amount;
	return operands;
}

HscIRBasicBlock* hsc_ir_generate_instructions(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr);

HscIRBasicBlock* hsc_ir_generate_instructions_from_intrinsic_function(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr, HscFunction* function, U32 function_id, HscExpr* call_args_expr) {
	U32 args_count = ((U8*)call_args_expr)[1];
	U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];
	HscExpr* arg_expr = call_args_expr;

	HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, args_count + 1);
	U16 return_value_idx = hsc_ir_add_value(ir, ir_function, expr->data_type);
	operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);

	for (U32 idx = 0; idx < args_count; idx += 1) {
		arg_expr = &arg_expr[next_arg_expr_rel_indices[idx]];
		basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, arg_expr);
		operands[idx + 1] = ir->last_operand;
	}

	hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_COMPOSITE_INIT, operands, args_count + 1);

	ir->last_operand = operands[0];
	return basic_block;
}

U16 hsc_ir_basic_block_idx(HscIR* ir, HscIRFunction* ir_function, HscIRBasicBlock* basic_block) {
	return (basic_block - ir->basic_blocks) - ir_function->basic_blocks_start_idx;
}

HscDataType hsc_ir_operand_data_type(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIROperand ir_operand) {
	U32 word;
	switch (ir_operand & 0xff) {
		case HSC_IR_OPERAND_VALUE: {
			HscIRValue* value = &ir->values[ir_function->values_start_idx + HSC_IR_OPERAND_VALUE_IDX(ir_operand)];
			return value->data_type;
		};
		case HSC_IR_OPERAND_CONSTANT: {
			HscConstant constant = hsc_constant_table_get(&astgen->constant_table, HSC_IR_OPERAND_CONSTANT_ID(ir_operand));
			return constant.data_type;
		};
		case HSC_IR_OPERAND_BASIC_BLOCK:
			HSC_UNREACHABLE("cannot get the type of a basic block");
		case HSC_IR_OPERAND_LOCAL_VARIABLE: {
			U32 function_idx = ir_function - ir->functions;
			HscFunction* function = &astgen->functions[function_idx];
			HscLocalVariable* local_variable = &astgen->function_params_and_local_variables[function->params_start_idx + HSC_IR_OPERAND_LOCAL_VARIABLE_IDX(ir_operand)];
			return local_variable->data_type;
		};
		default:
			return (HscDataType)ir_operand;
	}
}

HscIRBasicBlock* hsc_ir_generate_condition_expr(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* cond_expr) {
	basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, cond_expr);
	HscIROperand cond_operand = ir->last_operand;
	HscDataType cond_data_type = hsc_ir_operand_data_type(ir, astgen, ir_function, cond_operand);
	if (cond_data_type != HSC_DATA_TYPE_BOOL) {
		if (HSC_DATA_TYPE_IS_STRUCT(cond_data_type) || HSC_DATA_TYPE_IS_MATRIX(cond_data_type)) {
			HscString data_type_name = hsc_data_type_string(astgen, cond_operand);
			// TODO emitt the error in the AST generation instead
			hsc_astgen_error_1(astgen, "a condition expression must be a non-structure & non-matrix type but got '%.*s'", (int)data_type_name.size, data_type_name.data);
		}

		HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 3);
		hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BINARY_OP(NOT_EQUAL), operands, 3);

		HscDataType new_cond_data_type;
		if (cond_data_type >= HSC_DATA_TYPE_VEC4_START) {
			new_cond_data_type = HSC_DATA_TYPE_VEC4(HSC_DATA_TYPE_BOOL);
		} else if (cond_data_type >= HSC_DATA_TYPE_VEC3_START) {
			new_cond_data_type = HSC_DATA_TYPE_VEC3(HSC_DATA_TYPE_BOOL);
		} else if (cond_data_type >= HSC_DATA_TYPE_VEC2_START) {
			new_cond_data_type = HSC_DATA_TYPE_VEC2(HSC_DATA_TYPE_BOOL);
		} else {
			new_cond_data_type = HSC_DATA_TYPE_BOOL;
		}

		U16 return_value_idx = hsc_ir_add_value(ir, ir_function, new_cond_data_type);
		operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
		operands[1] = cond_operand;
		operands[2] = HSC_IR_OPERAND_CONSTANT_INIT(hsc_constant_table_deduplicate_zero(&astgen->constant_table, astgen, cond_data_type).idx_plus_one);
		ir->last_operand = operands[0];
	}
	return basic_block;
}

typedef U8 HscBasicTypeClass;
enum {
	HSC_BASIC_TYPE_CLASS_VOID,
	HSC_BASIC_TYPE_CLASS_BOOL,
	HSC_BASIC_TYPE_CLASS_UINT,
	HSC_BASIC_TYPE_CLASS_SINT,
	HSC_BASIC_TYPE_CLASS_FLOAT,

	HSC_BASIC_TYPE_CLASS_COUNT,
};

HscBasicTypeClass hsc_basic_type_class(HscDataType data_type) {
	switch (data_type) {
		case HSC_DATA_TYPE_BOOL: return HSC_BASIC_TYPE_CLASS_BOOL;
		case HSC_DATA_TYPE_U8:
		case HSC_DATA_TYPE_U16:
		case HSC_DATA_TYPE_U32:
		case HSC_DATA_TYPE_U64: return HSC_BASIC_TYPE_CLASS_UINT;
		case HSC_DATA_TYPE_S8:
		case HSC_DATA_TYPE_S16:
		case HSC_DATA_TYPE_S32:
		case HSC_DATA_TYPE_S64: return HSC_BASIC_TYPE_CLASS_SINT;
		case HSC_DATA_TYPE_F16:
		case HSC_DATA_TYPE_F32:
		case HSC_DATA_TYPE_F64: return HSC_BASIC_TYPE_CLASS_FLOAT;
		default: HSC_UNREACHABLE("internal error: expected a basic type");
	}
}

HscIRBasicBlock* hsc_ir_generate_case_instructions(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* first_stmt) {
	HscExpr* stmt = first_stmt;
	while (1) {
		basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, stmt);
		if (
			stmt->type == HSC_EXPR_TYPE_STMT_CASE ||
			stmt->type == HSC_EXPR_TYPE_STMT_DEFAULT
		) {
			break;
		}
		stmt = &stmt[stmt->next_expr_rel_idx];
	}
	ir->branch_state.all_cases_return &= hsc_stmt_has_return(stmt);
	return basic_block;
}

void hsc_ir_generate_store(HscIR* ir, HscIRFunction* ir_function, HscIROperand left_operand, HscIROperand right_operand) {
	HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 2);
	operands[0] = left_operand;
	operands[1] = right_operand;
	hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_STORE, operands, 2);
}

HscIRBasicBlock* hsc_ir_generate_instructions(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr) {
	HscIROpCode op_code;
	switch (expr->type) {
		case HSC_EXPR_TYPE_STMT_BLOCK: {
			if (!basic_block) {
				basic_block = hsc_ir_add_basic_block(ir, ir_function);
			}

			U32 stmts_count = expr->stmt_block.stmts_count;
			HscExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
			for (U32 i = 0; i < stmts_count; i += 1) {
				basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, stmt);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}

			break;
		};
		case HSC_EXPR_TYPE_STMT_RETURN: {
			HscExpr* unary_expr = &expr[expr->unary.expr_rel_idx];
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			operands[0] = ir->last_operand;
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_FUNCTION_RETURN, operands, 1);
			break;
		};
		case HSC_EXPR_TYPE_UNARY_OP(PLUS): {
			HscExpr* unary_expr = &expr[expr->unary.expr_rel_idx];
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);
			break;
		};
		case HSC_EXPR_TYPE_UNARY_OP(LOGICAL_NOT): op_code = HSC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT); goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(BIT_NOT): op_code = HSC_IR_OP_CODE_UNARY_OP(BIT_NOT); goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(NEGATE): op_code = HSC_IR_OP_CODE_UNARY_OP(NEGATE); goto UNARY;
UNARY:
		{
			HscExpr* unary_expr = &expr[expr->unary.expr_rel_idx];
			if (op_code == HSC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT) && HSC_DATA_TYPE_SCALAR(unary_expr->data_type) != HSC_DATA_TYPE_BOOL) {
				basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, unary_expr);
			} else {
				basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);
			}

			U16 return_value_idx = hsc_ir_add_value(ir, ir_function, expr->data_type);
			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 2);
			operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
			operands[1] = ir->last_operand;
			hsc_ir_add_instruction(ir, ir_function, op_code, operands, 2);

			ir->last_operand = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
			break;
		};
		case HSC_EXPR_TYPE_STMT_IF: {
			HscExpr* cond_expr = &expr[expr->if_.cond_expr_rel_idx];
			basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, cond_expr);
			HscIROperand cond_operand = ir->last_operand;

			HscIROperand* selection_merge_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_SELECTION_MERGE, selection_merge_operands, 1);

			HscIROperand* cond_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 3);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
			cond_branch_operands[0] = cond_operand;

			HscExpr* true_stmt = &expr[expr->if_.true_stmt_rel_idx];
			bool true_needs_branch = !hsc_stmt_has_return(true_stmt);
			HscIRBasicBlock* true_basic_block = hsc_ir_add_basic_block(ir, ir_function);
			hsc_ir_generate_instructions(ir, astgen, ir_function, true_basic_block, true_stmt);
			cond_branch_operands[1] = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, true_basic_block));

			HscIROperand* true_branch_operands;
			if (true_needs_branch) {
				true_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, true_branch_operands, 1);
			}

			HscIROperand* false_branch_operands;
			bool false_needs_branch;
			if (true_stmt->if_aux.false_stmt_rel_idx) {
				HscExpr* false_stmt = &true_stmt[true_stmt->if_aux.false_stmt_rel_idx];
				false_needs_branch = !hsc_stmt_has_return(false_stmt);
				HscIRBasicBlock* false_basic_block = hsc_ir_add_basic_block(ir, ir_function);
				hsc_ir_generate_instructions(ir, astgen, ir_function, false_basic_block, false_stmt);
				cond_branch_operands[2] = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, false_basic_block));

				if (false_needs_branch) {
					false_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
					hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, false_branch_operands, 1);
				}
			}

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand converging_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			selection_merge_operands[0] = converging_basic_block_operand;
			if (true_needs_branch) {
				true_branch_operands[0] = converging_basic_block_operand;
			}

			if (true_stmt->if_aux.false_stmt_rel_idx) {
				if (false_needs_branch) {
					false_branch_operands[0] = converging_basic_block_operand;
				}
			} else {
				cond_branch_operands[2] = converging_basic_block_operand;
			}

			if (!true_needs_branch && !false_needs_branch) {
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_UNREACHABLE, NULL, 0);
			}
			break;
		};
		case HSC_EXPR_TYPE_STMT_SWITCH: {
			HscExpr* block_stmt = &expr[expr->switch_.block_expr_rel_idx];
			if (block_stmt->switch_aux.first_case_expr_rel_idx == 0) {
				break;
			}

			HscIROperand* selection_merge_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_SELECTION_MERGE, selection_merge_operands, 1);

			U32 operands_count = 2 + block_stmt->switch_aux.case_stmts_count * 2;
			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, operands_count);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_SWITCH, operands, operands_count);

			HscExpr* cond_expr = &expr[expr->switch_.cond_expr_rel_idx];
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, cond_expr);
			operands[0] = ir->last_operand;

			HscExpr* case_expr = &block_stmt[block_stmt->switch_aux.first_case_expr_rel_idx];
			U32 case_idx = 0;

			HscIRBranchState prev_branch_state = ir->branch_state;
			ir->branch_state.all_cases_return = true;
			ir->branch_state.break_branch_linked_list_head = -1;
			ir->branch_state.break_branch_linked_list_tail = -1;
			ir->branch_state.continue_branch_linked_list_head = -1;
			ir->branch_state.continue_branch_linked_list_tail = -1;
			while (1) {
				basic_block = hsc_ir_add_basic_block(ir, ir_function);
				operands[2 + (case_idx * 2) + 0] = HSC_IR_OPERAND_CONSTANT_INIT(case_expr->constant.id);
				operands[2 + (case_idx * 2) + 1] = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
				if (case_expr->next_expr_rel_idx == 0) {
					break;
				}

				HscExpr* first_stmt = &case_expr[case_expr->next_expr_rel_idx];
				basic_block = hsc_ir_generate_case_instructions(ir, astgen, ir_function, basic_block, first_stmt);

				if (case_expr->alt_next_expr_rel_idx == 0) {
					break;
				}
				case_expr = &case_expr[case_expr->alt_next_expr_rel_idx];
				case_idx += 1;
			}

			HscIROperand* default_branch_operands;
			if (expr->alt_next_expr_rel_idx) {
				HscExpr* default_case_expr = &expr[expr->alt_next_expr_rel_idx];

				HscIRBasicBlock* default_basic_block = hsc_ir_add_basic_block(ir, ir_function);
				basic_block = default_basic_block;

				HscExpr* first_stmt = &default_case_expr[1];
				basic_block = hsc_ir_generate_case_instructions(ir, astgen, ir_function, basic_block, first_stmt);

				/*
				default_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, default_branch_operands, 1);
				*/

				operands[1] = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, default_basic_block));
			}

			HscIRBasicBlock* converging_basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand converging_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, converging_basic_block));

			selection_merge_operands[0] = converging_basic_block_operand;
			basic_block = converging_basic_block;
			if (expr->alt_next_expr_rel_idx) {
				//default_branch_operands[1] = converging_basic_block_operand;
			} else {
				operands[1] = converging_basic_block_operand;
			}

			if (ir->branch_state.all_cases_return) {
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_UNREACHABLE, NULL, 0);
			}

			while (ir->branch_state.break_branch_linked_list_head != -1) {
				U32 next = ir->operands[ir_function->operands_start_idx + ir->branch_state.break_branch_linked_list_head];
				ir->operands[ir_function->operands_start_idx + ir->branch_state.break_branch_linked_list_head] = converging_basic_block_operand;
				ir->branch_state.break_branch_linked_list_head = next;
			}

			ir->branch_state = prev_branch_state;

			break;
		};
		case HSC_EXPR_TYPE_STMT_WHILE:
		case HSC_EXPR_TYPE_STMT_FOR: {
			HscExpr* init_expr;
			HscExpr* cond_expr;
			HscExpr* inc_expr;
			HscExpr* loop_stmt;
			if (expr->type == HSC_EXPR_TYPE_STMT_FOR) {
				init_expr = &expr[expr->for_.init_expr_rel_idx];
				cond_expr = &expr[expr->for_.cond_expr_rel_idx];
				inc_expr = &expr[expr->for_.inc_expr_rel_idx];
				loop_stmt = &expr[expr->for_.loop_stmt_rel_idx];
			} else {
				init_expr = NULL;
				cond_expr = &expr[expr->while_.cond_expr_rel_idx];
				inc_expr = NULL;
				loop_stmt = &expr[expr->while_.loop_stmt_rel_idx];
			}

			if (init_expr) {
				basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, init_expr);
			}

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, operands, 1);

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand starting_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			operands[0] = starting_basic_block_operand;

			basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, cond_expr);
			HscIROperand cond_operand = ir->last_operand;

			HscIROperand* loop_merge_operands = hsc_ir_add_operands_many(ir, ir_function, 2);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_LOOP_MERGE, loop_merge_operands, 2);

			HscIROperand* cond_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 3);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
			cond_branch_operands[0] = cond_operand;

			HscIRBranchState prev_branch_state = ir->branch_state;
			ir->branch_state.break_branch_linked_list_head = -1;
			ir->branch_state.break_branch_linked_list_tail = -1;
			ir->branch_state.continue_branch_linked_list_head = -1;
			ir->branch_state.continue_branch_linked_list_tail = -1;

			HscIRBasicBlock* loop_basic_block = hsc_ir_add_basic_block(ir, ir_function);
			hsc_ir_generate_instructions(ir, astgen, ir_function, loop_basic_block, loop_stmt);
			cond_branch_operands[1] = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, loop_basic_block));

			HscIROpCode last_op_code = ir->instructions[ir->instructions_count - 1].op_code;
			HscIROperand* loop_branch_operands;
			if (last_op_code != HSC_IR_OP_CODE_BRANCH || last_op_code != HSC_IR_OP_CODE_FUNCTION_RETURN) {
				loop_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, loop_branch_operands, 1);
			}

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand continue_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			loop_branch_operands[0] = continue_basic_block_operand;
			loop_merge_operands[1] = continue_basic_block_operand;
			if (inc_expr) {
				basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, inc_expr);
			}
			operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, operands, 1);
			operands[0] = starting_basic_block_operand;

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand converging_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			cond_branch_operands[2] = converging_basic_block_operand;
			loop_merge_operands[0] = converging_basic_block_operand;

			while (ir->branch_state.break_branch_linked_list_head != -1) {
				U32 next = ir->operands[ir_function->operands_start_idx + ir->branch_state.break_branch_linked_list_head];
				ir->operands[ir_function->operands_start_idx + ir->branch_state.break_branch_linked_list_head] = converging_basic_block_operand;
				ir->branch_state.break_branch_linked_list_head = next;
			}

			while (ir->branch_state.continue_branch_linked_list_head != -1) {
				U32 next = ir->operands[ir_function->operands_start_idx + ir->branch_state.continue_branch_linked_list_head];
				ir->operands[ir_function->operands_start_idx + ir->branch_state.continue_branch_linked_list_head] = continue_basic_block_operand;
				ir->branch_state.continue_branch_linked_list_head = next;
			}

			ir->branch_state = prev_branch_state;
			break;
		};
		case HSC_EXPR_TYPE_BINARY_OP(ASSIGN):
		{
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;

			ir->do_not_load_variable = left_expr->type == HSC_EXPR_TYPE_LOCAL_VARIABLE;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, left_expr);
			HscIROperand left_operand = ir->last_operand;

			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, right_expr);
			HscIROperand right_operand = ir->last_operand;

			hsc_ir_generate_store(ir, ir_function, left_operand, right_operand);
			ir->last_operand = left_operand;
			break;
		};
		case HSC_EXPR_TYPE_BINARY_OP(ADD):
		case HSC_EXPR_TYPE_BINARY_OP(SUBTRACT):
		case HSC_EXPR_TYPE_BINARY_OP(MULTIPLY):
		case HSC_EXPR_TYPE_BINARY_OP(DIVIDE):
		case HSC_EXPR_TYPE_BINARY_OP(MODULO):
		case HSC_EXPR_TYPE_BINARY_OP(BIT_AND):
		case HSC_EXPR_TYPE_BINARY_OP(BIT_OR):
		case HSC_EXPR_TYPE_BINARY_OP(BIT_XOR):
		case HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_LEFT):
		case HSC_EXPR_TYPE_BINARY_OP(BIT_SHIFT_RIGHT):
		case HSC_EXPR_TYPE_BINARY_OP(EQUAL):
		case HSC_EXPR_TYPE_BINARY_OP(NOT_EQUAL):
		case HSC_EXPR_TYPE_BINARY_OP(LESS_THAN):
		case HSC_EXPR_TYPE_BINARY_OP(LESS_THAN_OR_EQUAL):
		case HSC_EXPR_TYPE_BINARY_OP(GREATER_THAN):
		case HSC_EXPR_TYPE_BINARY_OP(GREATER_THAN_OR_EQUAL):
		{
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			HscIROpCode op_code = HSC_IR_OP_CODE_BINARY_OP_START + (expr->type - HSC_EXPR_TYPE_BINARY_OP_START);

			HscDataType data_type;
			if (expr->binary.is_assignment) {
				data_type = left_expr->data_type;
			} else {
				data_type = expr->data_type;
			}

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 3);

			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, left_expr);
			operands[1] = ir->last_operand;

			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, right_expr);
			operands[2] = ir->last_operand;

			U16 return_value_idx = hsc_ir_add_value(ir, ir_function, data_type);
			operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
			hsc_ir_add_instruction(ir, ir_function, op_code, operands, 3);

			if (expr->binary.is_assignment) {
				HscIROperand dst_operand;
				if (left_expr->type == HSC_EXPR_TYPE_LOCAL_VARIABLE) {
					dst_operand = HSC_IR_OPERAND_LOCAL_VARIABLE_INIT(left_expr->variable.idx);
				} else {
					dst_operand = operands[1];
				}

				hsc_ir_generate_store(ir, ir_function, dst_operand, operands[0]);
			}

			ir->last_operand = operands[0];
			break;
		};
		case HSC_EXPR_TYPE_LOGICAL_AND:
		case HSC_EXPR_TYPE_LOGICAL_OR:
		{
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;

			HscIROperand starting_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));

			basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, left_expr);
			HscIROperand cond_operand = ir->last_operand;

			HscIROperand* selection_merge_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_SELECTION_MERGE, selection_merge_operands, 1);

			HscIROperand* cond_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 3);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
			cond_branch_operands[0] = cond_operand;
			U32 success_idx = expr->type != HSC_EXPR_TYPE_LOGICAL_AND;
			U32 converging_idx = expr->type == HSC_EXPR_TYPE_LOGICAL_AND;

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand success_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			cond_branch_operands[1 + success_idx] = success_basic_block_operand;

			basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, right_expr);
			HscIROperand success_cond_operand = ir->last_operand;

			HscIROperand* success_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, success_branch_operands, 1);

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand converging_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			selection_merge_operands[0] = converging_basic_block_operand;
			success_branch_operands[0] = converging_basic_block_operand;
			cond_branch_operands[1 + converging_idx] = converging_basic_block_operand;

			HscIROperand* phi_operands = hsc_ir_add_operands_many(ir, ir_function, 5);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_PHI, phi_operands, 5);
			U16 return_value_idx = hsc_ir_add_value(ir, ir_function, hsc_ir_operand_data_type(ir, astgen, ir_function, cond_operand));
			phi_operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
			phi_operands[1] = cond_operand;
			phi_operands[2] = starting_basic_block_operand;
			phi_operands[3] = success_cond_operand;
			phi_operands[4] = success_basic_block_operand;

			ir->last_operand = phi_operands[0];
			break;
		};
		case HSC_EXPR_TYPE_CALL: {
			HscExpr* function_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* call_args_expr = expr - expr->binary.right_expr_rel_idx;
			HSC_DEBUG_ASSERT(function_expr->type == HSC_EXPR_TYPE_FUNCTION, "expected an function expression");
			HSC_DEBUG_ASSERT(call_args_expr->type == HSC_EXPR_TYPE_CALL_ARG_LIST, "expected call argument list expression");
			HscFunction* function = &astgen->functions[function_expr->function.id - 1];
			if (function_expr->function.id < HSC_FUNCTION_ID_USER_START) {
				basic_block = hsc_ir_generate_instructions_from_intrinsic_function(ir, astgen, ir_function, basic_block, expr, function, function_expr->function.id, call_args_expr);
			} else {
				HscExpr* arg_expr = call_args_expr;
				U32 args_count = ((U8*)call_args_expr)[1];
				U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];

				HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, args_count + 1);
				U16 return_value_idx = hsc_ir_add_value(ir, ir_function, function->return_data_type);
				operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);

				for (U32 i = 0; i < args_count; i += 1) {
					arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
					basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, arg_expr);
					operands[i + 1] = ir->last_operand;
				}

				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_FUNCTION_CALL, operands, args_count);
				ir->last_operand = operands[0];
			}

			break;
		};
		case HSC_EXPR_TYPE_CONSTANT: {
			ir->last_operand = HSC_IR_OPERAND_CONSTANT_INIT(expr->constant.id);
			break;
		};
		case HSC_EXPR_TYPE_STMT_BREAK: {
			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			operands[0] = -1; // the operand is initialized later by the callee
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, operands, 1);

			if (ir->branch_state.break_branch_linked_list_tail == -1) {
				ir->branch_state.break_branch_linked_list_head = ir_function->operands_count - 1;
			} else {
				ir->operands[ir_function->operands_start_idx + ir->branch_state.break_branch_linked_list_tail] = ir_function->operands_count - 1;
			}
			ir->branch_state.break_branch_linked_list_tail = ir_function->operands_count - 1;

			if (expr->next_expr_rel_idx) {
				basic_block = hsc_ir_add_basic_block(ir, ir_function);
			}

			break;
		};
		case HSC_EXPR_TYPE_STMT_CONTINUE: {
			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			operands[0] = -1; // the operand is initialized later by the callee
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, operands, 1);

			if (ir->branch_state.continue_branch_linked_list_tail == -1) {
				ir->branch_state.continue_branch_linked_list_head = ir_function->operands_count - 1;
			} else {
				ir->operands[ir_function->operands_start_idx + ir->branch_state.continue_branch_linked_list_tail] = ir_function->operands_count - 1;
			}
			ir->branch_state.continue_branch_linked_list_tail = ir_function->operands_count - 1;

			if (expr->next_expr_rel_idx) {
				basic_block = hsc_ir_add_basic_block(ir, ir_function);
			}

			break;
		};
		case HSC_EXPR_TYPE_STMT_CASE:
		case HSC_EXPR_TYPE_STMT_DEFAULT: {
			//
			// 'case' and 'default' will get found when hsc_ir_generate_case_instructions
			// is processing the statements of a 'case' and 'default' block.
			// they will implicitly fallthrough to this next 'case' and 'default' block
			// so make the next operand reference the next basic block index that will get made.
			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			operands[0] = HSC_IR_OPERAND_BASIC_BLOCK_INIT(ir_function->basic_blocks_count);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, operands, 1);
			break;
		};
		case HSC_EXPR_TYPE_LOCAL_VARIABLE: {
			if (ir->do_not_load_variable) {
				ir->last_operand = HSC_IR_OPERAND_LOCAL_VARIABLE_INIT(expr->variable.idx);
				ir->do_not_load_variable = false;
			} else {
				U32 function_idx = ir_function - ir->functions;
				HscFunction* function = &astgen->functions[function_idx];
				HscLocalVariable* local_variable = &astgen->function_params_and_local_variables[function->params_start_idx + expr->variable.idx];

				HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 2);
				U16 return_value_idx = hsc_ir_add_value(ir, ir_function, local_variable->data_type);
				operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
				operands[1] = HSC_IR_OPERAND_LOCAL_VARIABLE_INIT(expr->variable.idx);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_LOAD, operands, 2);

				ir->last_operand = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
			}
			break;
		};
		default:
			HSC_ABORT("unhandle expr type %u\n", expr->type);
	}

	return basic_block;
}

void hsc_ir_generate_function(HscIR* ir, HscAstGen* astgen, U32 function_idx) {
	HscFunction* function = &astgen->functions[function_idx];
	HscIRFunction* ir_function = &ir->functions[function_idx];
	ir_function->basic_blocks_start_idx = ir->basic_blocks_count;
	ir_function->basic_blocks_count = 0;
	ir_function->instructions_start_idx = ir->instructions_count;
	ir_function->instructions_count = 0;
	ir_function->values_start_idx = ir->values_count;
	ir_function->values_count = 0;
	ir_function->operands_start_idx = ir->operands_count;
	ir_function->operands_count = 0;

	HSC_DEBUG_ASSERT(function->block_expr_id.idx_plus_one, "expected to have a function body");

	HscExpr* expr = &astgen->exprs[function->block_expr_id.idx_plus_one - 1];
	hsc_ir_generate_instructions(ir, astgen, ir_function, NULL, expr);
}

void hsc_ir_generate(HscIR* ir, HscAstGen* astgen) {
	for (U32 function_idx = HSC_FUNCTION_ID_USER_START; function_idx < astgen->functions_count; function_idx += 1) {
		hsc_ir_generate_function(ir, astgen, function_idx);
	}
}

void hsc_ir_print_operand(HscIR* ir, HscAstGen* astgen, HscIROperand operand, FILE* f) {
	switch (operand & 0xff) {
		case HSC_IR_OPERAND_VALUE:
			fprintf(f, "v%u", HSC_IR_OPERAND_VALUE_IDX(operand));
			break;
		case HSC_IR_OPERAND_CONSTANT:
			fprintf(f, "c%u", HSC_IR_OPERAND_CONSTANT_ID(operand).idx_plus_one - 1);
			break;
		case HSC_IR_OPERAND_BASIC_BLOCK:
			fprintf(f, "b%u", HSC_IR_OPERAND_BASIC_BLOCK_IDX(operand));
			break;
		case HSC_IR_OPERAND_LOCAL_VARIABLE:
			fprintf(f, "var%u", HSC_IR_OPERAND_LOCAL_VARIABLE_IDX(operand));
			break;
		default: {
			HscString data_type_name = hsc_data_type_string(astgen, operand);
			fprintf(f, "%.*s", (int)data_type_name.size, data_type_name.data);
			break;
		};
	}
}

void hsc_ir_print(HscIR* ir, HscAstGen* astgen, FILE* f) {
	HscConstantTable* constant_table = &astgen->constant_table;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[idx];
		fprintf(f, "Constant(c%u): ", idx);
		HscConstantId constant_id = { .idx_plus_one = idx + 1 };
		hsc_constant_print(astgen, constant_id, stdout);
		fprintf(f, "\n");
	}

	for (U32 function_idx = HSC_FUNCTION_ID_USER_START; function_idx < astgen->functions_count; function_idx += 1) {
		HscFunction* function = &astgen->functions[function_idx];
		HscIRFunction* ir_function = &ir->functions[function_idx];
		char buf[1024];
		hsc_function_to_string(astgen, function, buf, sizeof(buf), false);
		fprintf(f, "Function(#%u): %s\n", function_idx, buf);
		for (U32 basic_block_idx = ir_function->basic_blocks_start_idx; basic_block_idx < ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count; basic_block_idx += 1) {
			HscIRBasicBlock* basic_block = &ir->basic_blocks[basic_block_idx];
			fprintf(f, "\tBASIC_BLOCK(#%u):\n", basic_block_idx - ir_function->basic_blocks_start_idx);
			for (U32 instruction_idx = basic_block->instructions_start_idx; instruction_idx < basic_block->instructions_start_idx + (U32)basic_block->instructions_count; instruction_idx += 1) {
				HscIRInstr* instruction = &ir->instructions[ir_function->instructions_start_idx + instruction_idx];
				char* op_name;
				switch (instruction->op_code) {
#if 0
					case HSC_IR_OP_CODE_LOAD:
						break;
					case HSC_IR_OP_CODE_ACCESS_CHAIN:
						break;
					case HSC_IR_OP_CODE_FUNCTION_CALL:
						break;
#endif
					case HSC_IR_OP_CODE_LOAD:
					{
						fprintf(f, "\t\tOP_LOAD: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_STORE:
					{
						fprintf(f, "\t\tOP_STORE: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_COMPOSITE_INIT: {
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						fprintf(f, "\t\t");
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, " = OP_COMPOSITE_INIT: ");
						for (U32 idx = 1; idx < instruction->operands_count; idx += 1) {
							hsc_ir_print_operand(ir, astgen, operands[idx], f);
							if (idx + 1 < instruction->operands_count) {
								fprintf(f, ", ");
							}
						}
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_FUNCTION_RETURN:
						fprintf(f, "\t\tOP_FUNCTION_RETURN: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, "\n");
						break;
					case HSC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT): op_name = "LOGICAL_NOT"; goto UNARY;
					case HSC_IR_OP_CODE_UNARY_OP(BIT_NOT): op_name = "BIT_NOT"; goto UNARY;
					case HSC_IR_OP_CODE_UNARY_OP(NEGATE): op_name = "NEGATE"; goto UNARY;
UNARY:				{
						fprintf(f, "\t\tOP_%s: ", op_name);
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_SELECTION_MERGE: {
						fprintf(f, "\t\tOP_SELECTION_MERGE: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_LOOP_MERGE: {
						fprintf(f, "\t\tOP_LOOP_MERGE: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_BINARY_OP(ADD): op_name = "OP_ADD"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(SUBTRACT): op_name = "OP_SUBTRACT"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(MULTIPLY): op_name = "OP_MULTIPLY"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(DIVIDE): op_name = "OP_DIVIDE"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(MODULO): op_name = "OP_MODULO"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(BIT_AND): op_name = "OP_BIT_AND"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(BIT_OR): op_name = "OP_BIT_OR"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(BIT_XOR): op_name = "OP_BIT_XOR"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_LEFT): op_name = "OP_BIT_SHIFT_LEFT"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_RIGHT): op_name = "OP_BIT_SHIFT_RIGHT"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(EQUAL): op_name = "OP_EQUAL"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(NOT_EQUAL): op_name = "OP_NOT_EQUAL"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(LESS_THAN): op_name = "OP_LESS_THAN"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(LESS_THAN_OR_EQUAL): op_name = "OP_LESS_THAN_OR_EQUAL"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(GREATER_THAN): op_name = "OP_GREATER_THAN"; goto BINARY_OP;
					case HSC_IR_OP_CODE_BINARY_OP(GREATER_THAN_OR_EQUAL): op_name = "OP_GREATER_THAN_OR_EQUAL"; goto BINARY_OP;
BINARY_OP:
					{
						fprintf(f, "\t\t%s: ", op_name);
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_BRANCH: {
						fprintf(f, "\t\tOP_BRANCH: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_BRANCH_CONDITIONAL: {
						fprintf(f, "\t\tOP_BRANCH_CONDITIONAL: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_PHI: {
						fprintf(f, "\t\tOP_PHI: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						for (U32 idx = 1; idx < instruction->operands_count; idx += 2) {
							fprintf(f, "(");
							hsc_ir_print_operand(ir, astgen, operands[idx + 1], f);
							fprintf(f, ": ");
							hsc_ir_print_operand(ir, astgen, operands[idx + 0], f);
							fprintf(f, ")");
						}
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_SWITCH: {
						fprintf(f, "\t\tOP_SWITCH: ");
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, ", ");
						for (U32 idx = 2; idx < instruction->operands_count; idx += 2) {
							fprintf(f, "(");
							hsc_ir_print_operand(ir, astgen, operands[idx + 0], f);
							fprintf(f, ": ");
							hsc_ir_print_operand(ir, astgen, operands[idx + 1], f);
							fprintf(f, ")");
							if (idx + 1 < instruction->operands_count) {
								fprintf(f, ", ");
							}
						}
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_UNREACHABLE: {
						fprintf(f, "\t\tOP_UNREACHABLE:\n");
						break;
					};
					default:
						HSC_ABORT("unhandled instruction '%u'", instruction->op_code);
				}
			}
		}
	}
}

// ===========================================
//
//
// SPIR-V
//
//
// ===========================================

HscSpirvOp hsc_spriv_binary_ops[HSC_BINARY_OP_COUNT][HSC_BASIC_TYPE_CLASS_COUNT] = {
	[HSC_BINARY_OP_ADD] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_I_ADD,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_I_ADD,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_ADD,
	},
	[HSC_BINARY_OP_SUBTRACT] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_I_SUB,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_I_SUB,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_SUB,
	},
	[HSC_BINARY_OP_MULTIPLY] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_I_MUL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_I_MUL,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_MUL,
	},
	[HSC_BINARY_OP_DIVIDE] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_U_DIV,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_DIV,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_DIV,
	},
	[HSC_BINARY_OP_MODULO] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_U_MOD,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_MOD,
	},
	[HSC_BINARY_OP_BIT_AND] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_BITWISE_AND,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_BITWISE_AND,
	},
	[HSC_BINARY_OP_BIT_OR] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_BITWISE_OR,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_BITWISE_OR,
	},
	[HSC_BINARY_OP_BIT_XOR] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_BITWISE_XOR,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_BITWISE_XOR,
	},
	[HSC_BINARY_OP_BIT_SHIFT_LEFT] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL,
	},
	[HSC_BINARY_OP_BIT_SHIFT_RIGHT] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC,
	},
	[HSC_BINARY_OP_EQUAL] = {
		[HSC_BASIC_TYPE_CLASS_BOOL] = HSC_SPIRV_OP_LOGICAL_EQUAL,
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_I_EQUAL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_I_EQUAL,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_UNORD_EQUAL,
	},
	[HSC_BINARY_OP_NOT_EQUAL] = {
		[HSC_BASIC_TYPE_CLASS_BOOL] = HSC_SPIRV_OP_LOGICAL_NOT_EQUAL,
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_I_NOT_EQUAL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_I_NOT_EQUAL,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_UNORD_NOT_EQUAL,
	},
	[HSC_BINARY_OP_LESS_THAN] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_U_LESS_THAN,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_LESS_THAN,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_UNORD_LESS_THAN,
	},
	[HSC_BINARY_OP_LESS_THAN_OR_EQUAL] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_U_LESS_THAN_EQUAL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_LESS_THAN_EQUAL,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL,
	},
	[HSC_BINARY_OP_GREATER_THAN] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_U_GREATER_THAN,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_GREATER_THAN,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_UNORD_GREATER_THAN,
	},
	[HSC_BINARY_OP_GREATER_THAN_OR_EQUAL] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_U_GREATER_THAN_EQUAL,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_GREATER_THAN_EQUAL,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL,
	},
};

HscSpirvOp hsc_spriv_unary_ops[HSC_UNARY_OP_COUNT][HSC_BASIC_TYPE_CLASS_COUNT] = {
	[HSC_UNARY_OP_LOGICAL_NOT] = {
		[HSC_BASIC_TYPE_CLASS_BOOL] = HSC_SPIRV_OP_LOGICAL_NOT,
	},
	[HSC_UNARY_OP_BIT_NOT] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_BITWISE_NOT,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_BITWISE_NOT,
	},
	[HSC_UNARY_OP_NEGATE] = {
		[HSC_BASIC_TYPE_CLASS_UINT] = HSC_SPIRV_OP_S_NEGATE,
		[HSC_BASIC_TYPE_CLASS_SINT] = HSC_SPIRV_OP_S_NEGATE,
		[HSC_BASIC_TYPE_CLASS_FLOAT] = HSC_SPIRV_OP_F_NEGATE,
	},
};

void hsc_spirv_type_table_init(HscSpirvTypeTable* table) {
	U32 cap = 8192;
	table->data_types = HSC_ALLOC_ARRAY(HscDataType, cap);
	HSC_ASSERT(table->data_types, "out of memory");
	table->data_types_cap = cap;
	table->entries = HSC_ALLOC_ARRAY(HscSpirvTypeEntry, cap);
	HSC_ASSERT(table->entries, "out of memory");
	table->entries_cap = cap;
}

U32 hsc_spirv_type_table_deduplicate_function(HscCompiler* c, HscSpirvTypeTable* table, HscFunction* function) {
	HSC_DEBUG_ASSERT(function->shader_stage == HSC_FUNCTION_SHADER_STAGE_NONE, "internal error: shader stage functions do not belong in the function type table");
	//
	// TODO make this a hash table look for speeeds
	for (U32 i = 0; i < table->entries_count; i += 1) {
		HscSpirvTypeEntry* entry = &table->entries[i];
		U32 function_data_types_count = function->params_count + 1;
		if (entry->kind != HSC_SPIRV_TYPE_KIND_FUNCTION) {
			continue;
		}
		if (entry->data_types_count != function_data_types_count) {
			continue;
		}

		HscDataType* data_types = &table->data_types[entry->data_types_start_idx];
		if (data_types[0] != function->return_data_type) {
			continue;
		}

		bool is_match = true;
		HscLocalVariable* params = &c->astgen.function_params_and_local_variables[function->params_start_idx];
		for (U32 j = 0; j < entry->data_types_count; j += 1) {
			if (data_types[j + 1] != params[j].data_type) {
				is_match = false;
				break;
			}
		}

		if (is_match) {
			return entry->spirv_id;
		}
	}

	HSC_ASSERT_ARRAY_BOUNDS(table->entries_count, table->entries_cap);
	HscSpirvTypeEntry* entry = &table->entries[table->entries_count];
	table->entries_count += 1;

	entry->data_types_start_idx = table->data_types_count;
	entry->data_types_count = function->params_count + 1;
	entry->spirv_id = c->spirv.next_id;
	entry->kind = HSC_SPIRV_TYPE_KIND_FUNCTION;

	HSC_ASSERT_ARRAY_BOUNDS(table->data_types_count + function->params_count, table->data_types_cap);
	HscDataType* data_types = &table->data_types[table->data_types_count];
	table->data_types_count += entry->data_types_count;

	data_types[0] = function->return_data_type;
	HscLocalVariable* params = &c->astgen.function_params_and_local_variables[function->params_start_idx];
	for (U32 j = 0; j < entry->data_types_count; j += 1) {
		data_types[j + 1] = params[j].data_type;
	}

	return entry->spirv_id;
}

U32 hsc_spirv_type_table_deduplicate_function_variable(HscCompiler* c, HscSpirvTypeTable* table, HscDataType data_type) {
	//
	// TODO make this a hash table look for speeeds
	for (U32 i = 0; i < table->entries_count; i += 1) {
		HscSpirvTypeEntry* entry = &table->entries[i];
		if (entry->kind != HSC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE) {
			continue;
		}

		if (table->data_types[entry->data_types_start_idx] == data_type) {
			return entry->spirv_id;
		}
	}

	HSC_ASSERT_ARRAY_BOUNDS(table->entries_count, table->entries_cap);
	HscSpirvTypeEntry* entry = &table->entries[table->entries_count];
	table->entries_count += 1;

	entry->data_types_start_idx = table->data_types_count;
	entry->data_types_count = 1;
	entry->spirv_id = c->spirv.next_id;
	entry->kind = HSC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE;

	HSC_ASSERT_ARRAY_BOUNDS(table->data_types_count, table->data_types_cap);
	HscDataType* data_types = &table->data_types[table->data_types_count];
	table->data_types_count += entry->data_types_count;

	data_types[0] = data_type;

	return entry->spirv_id;
}

void hsc_spirv_init(HscCompiler* c) {
	U32 words_cap = 8192;

	hsc_spirv_type_table_init(&c->spirv.type_table);

	c->spirv.next_id += 1;

	c->spirv.out_capabilities = HSC_ALLOC_ARRAY(U32, words_cap);
	HSC_ASSERT(c->spirv.out_capabilities, "out of memory");
	c->spirv.out_capabilities_cap = words_cap;

	c->spirv.out_entry_points = HSC_ALLOC_ARRAY(U32, words_cap);
	HSC_ASSERT(c->spirv.out_entry_points, "out of memory");
	c->spirv.out_entry_points_cap = words_cap;

	c->spirv.out_debug_info = HSC_ALLOC_ARRAY(U32, words_cap);
	HSC_ASSERT(c->spirv.out_debug_info, "out of memory");
	c->spirv.out_debug_info_cap = words_cap;

	c->spirv.out_annotations = HSC_ALLOC_ARRAY(U32, words_cap);
	HSC_ASSERT(c->spirv.out_annotations, "out of memory");
	c->spirv.out_annotations_cap = words_cap;

	c->spirv.out_types_variables_constants = HSC_ALLOC_ARRAY(U32, words_cap);
	HSC_ASSERT(c->spirv.out_types_variables_constants, "out of memory");
	c->spirv.out_types_variables_constants_cap = words_cap;

	c->spirv.out_functions = HSC_ALLOC_ARRAY(U32, words_cap);
	HSC_ASSERT(c->spirv.out_functions, "out of memory");
	c->spirv.out_functions_cap = words_cap;
}

U32 hsc_spirv_resolve_type_id(HscCompiler* c, HscDataType data_type) {
	if (data_type < HSC_DATA_TYPE_MATRIX_END) {
		return data_type + 1;
	} else {
		HSC_ABORT("unhandled data type '%u'", data_type);
	}
}

void hsc_spirv_instr_start(HscCompiler* c, HscSpirvOp op) {
	HSC_DEBUG_ASSERT(c->spirv.instr_op == HSC_SPIRV_OP_NO_OP, "internal error: hsc_spirv_instr_end has not be called before a new instruction was started");
	c->spirv.instr_op = op;
	c->spirv.instr_operands_count = 0;
}

void hsc_spirv_instr_add_operand(HscCompiler* c, U32 word) {
	HSC_DEBUG_ASSERT(c->spirv.instr_op != HSC_SPIRV_OP_NO_OP, "internal error: hsc_spirv_instr_start has not been called when making an instruction");
	HSC_ASSERT_ARRAY_BOUNDS(c->spirv.instr_operands_count, HSC_SPIRV_INSTR_OPERANDS_CAP);
	c->spirv.instr_operands[c->spirv.instr_operands_count] = word;
	c->spirv.instr_operands_count += 1;
}

U32 hsc_spirv_convert_operand(HscCompiler* c, HscIROperand ir_operand) {
	switch (ir_operand & 0xff) {
		case HSC_IR_OPERAND_VALUE: return c->spirv.value_base_id + HSC_IR_OPERAND_VALUE_IDX(ir_operand);
		case HSC_IR_OPERAND_CONSTANT: return c->spirv.constant_base_id + HSC_IR_OPERAND_CONSTANT_ID(ir_operand).idx_plus_one - 1;
		case HSC_IR_OPERAND_BASIC_BLOCK: return c->spirv.basic_block_base_spirv_id + HSC_IR_OPERAND_BASIC_BLOCK_IDX(ir_operand);
		case HSC_IR_OPERAND_LOCAL_VARIABLE: return c->spirv.local_variable_base_spirv_id + HSC_IR_OPERAND_LOCAL_VARIABLE_IDX(ir_operand);
		default: return hsc_spirv_resolve_type_id(c, ir_operand);
	}
}

void hsc_spirv_instr_add_converted_operand(HscCompiler* c, HscIROperand ir_operand) {
	U32 word = hsc_spirv_convert_operand(c, ir_operand);
	hsc_spirv_instr_add_operand(c, word);
}

void hsc_spirv_instr_add_result_operand(HscCompiler* c) {
	hsc_spirv_instr_add_operand(c, c->spirv.next_id);
	c->spirv.next_id += 1;
}

void hsc_spirv_instr_end(HscCompiler* c) {
	HSC_DEBUG_ASSERT(c->spirv.instr_op != HSC_SPIRV_OP_NO_OP, "internal error: hsc_spirv_instr_start has not been called when making an instruction");

	U32* out;
	U32* count_ptr;
	switch (c->spirv.instr_op) {
		case HSC_SPIRV_OP_CAPABILITY:
			HSC_DEBUG_ASSERT(c->spirv.out_capabilities_count < c->spirv.out_capabilities_cap, "internal error: spirv types variables constants array has been filled up");
			out = &c->spirv.out_capabilities[c->spirv.out_capabilities_count];
			count_ptr = &c->spirv.out_capabilities_count;
			break;
		case HSC_SPIRV_OP_MEMORY_MODEL:
		case HSC_SPIRV_OP_ENTRY_POINT:
		case HSC_SPIRV_OP_EXECUTION_MODE:
			HSC_DEBUG_ASSERT(c->spirv.out_entry_points_count < c->spirv.out_entry_points_cap, "internal error: spirv types variables constants array has been filled up");
			out = &c->spirv.out_entry_points[c->spirv.out_entry_points_count];
			count_ptr = &c->spirv.out_entry_points_count;
			break;
		case HSC_SPIRV_OP_DECORATE:
			HSC_DEBUG_ASSERT(c->spirv.out_debug_info_count < c->spirv.out_debug_info_cap, "internal error: spirv types variables constants array has been filled up");
			out = &c->spirv.out_debug_info[c->spirv.out_debug_info_count];
			count_ptr = &c->spirv.out_debug_info_count;
			break;
		case HSC_SPIRV_OP_TYPE_VOID:
		case HSC_SPIRV_OP_TYPE_BOOL:
		case HSC_SPIRV_OP_TYPE_INT:
		case HSC_SPIRV_OP_TYPE_FLOAT:
		case HSC_SPIRV_OP_TYPE_VECTOR:
		case HSC_SPIRV_OP_TYPE_POINTER:
		case HSC_SPIRV_OP_TYPE_FUNCTION:
		case HSC_SPIRV_OP_CONSTANT_TRUE:
		case HSC_SPIRV_OP_CONSTANT_FALSE:
		case HSC_SPIRV_OP_CONSTANT:
		case HSC_SPIRV_OP_CONSTANT_COMPOSITE:
TYPES_VARIABLES_CONSTANTS:
			HSC_DEBUG_ASSERT(c->spirv.out_types_variables_constants_count < c->spirv.out_types_variables_constants_cap, "internal error: spirv types variables constants array has been filled up");
			out = &c->spirv.out_types_variables_constants[c->spirv.out_types_variables_constants_count];
			count_ptr = &c->spirv.out_types_variables_constants_count;
			break;
		case HSC_SPIRV_OP_FUNCTION:
		case HSC_SPIRV_OP_FUNCTION_PARAMETER:
		case HSC_SPIRV_OP_FUNCTION_END:
		case HSC_SPIRV_OP_COMPOSITE_CONSTRUCT:
		case HSC_SPIRV_OP_S_NEGATE:
		case HSC_SPIRV_OP_F_NEGATE:
		case HSC_SPIRV_OP_I_ADD:
		case HSC_SPIRV_OP_F_ADD:
		case HSC_SPIRV_OP_I_SUB:
		case HSC_SPIRV_OP_F_SUB:
		case HSC_SPIRV_OP_I_MUL:
		case HSC_SPIRV_OP_F_MUL:
		case HSC_SPIRV_OP_U_DIV:
		case HSC_SPIRV_OP_S_DIV:
		case HSC_SPIRV_OP_F_DIV:
		case HSC_SPIRV_OP_U_MOD:
		case HSC_SPIRV_OP_S_MOD:
		case HSC_SPIRV_OP_F_MOD:
		case HSC_SPIRV_OP_LOGICAL_EQUAL:
		case HSC_SPIRV_OP_LOGICAL_NOT_EQUAL:
		case HSC_SPIRV_OP_LOGICAL_OR:
		case HSC_SPIRV_OP_LOGICAL_AND:
		case HSC_SPIRV_OP_LOGICAL_NOT:
		case HSC_SPIRV_OP_SELECT:
		case HSC_SPIRV_OP_I_EQUAL:
		case HSC_SPIRV_OP_I_NOT_EQUAL:
		case HSC_SPIRV_OP_U_GREATER_THAN:
		case HSC_SPIRV_OP_S_GREATER_THAN:
		case HSC_SPIRV_OP_U_GREATER_THAN_EQUAL:
		case HSC_SPIRV_OP_S_GREATER_THAN_EQUAL:
		case HSC_SPIRV_OP_U_LESS_THAN:
		case HSC_SPIRV_OP_S_LESS_THAN:
		case HSC_SPIRV_OP_U_LESS_THAN_EQUAL:
		case HSC_SPIRV_OP_S_LESS_THAN_EQUAL:
		case HSC_SPIRV_OP_F_UNORD_EQUAL:
		case HSC_SPIRV_OP_F_UNORD_NOT_EQUAL:
		case HSC_SPIRV_OP_F_UNORD_LESS_THAN:
		case HSC_SPIRV_OP_F_UNORD_GREATER_THAN:
		case HSC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL:
		case HSC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL:
		case HSC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL:
		case HSC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC:
		case HSC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL:
		case HSC_SPIRV_OP_BITWISE_OR:
		case HSC_SPIRV_OP_BITWISE_XOR:
		case HSC_SPIRV_OP_BITWISE_AND:
		case HSC_SPIRV_OP_BITWISE_NOT:
		case HSC_SPIRV_OP_PHI:
		case HSC_SPIRV_OP_LOOP_MERGE:
		case HSC_SPIRV_OP_SELECTION_MERGE:
		case HSC_SPIRV_OP_LABEL:
		case HSC_SPIRV_OP_BRANCH:
		case HSC_SPIRV_OP_BRANCH_CONDITIONAL:
		case HSC_SPIRV_OP_SWITCH:
		case HSC_SPIRV_OP_RETURN:
		case HSC_SPIRV_OP_RETURN_VALUE:
		case HSC_SPIRV_OP_UNREACHABLE:
		case HSC_SPIRV_OP_LOAD:
		case HSC_SPIRV_OP_STORE:
FUNCTIONS:
			HSC_DEBUG_ASSERT(c->spirv.out_functions_count < c->spirv.out_functions_cap, "internal error: spirv types variables constants array has been filled up");
			out = &c->spirv.out_functions[c->spirv.out_functions_count];
			count_ptr = &c->spirv.out_functions_count;
			break;
		case HSC_SPIRV_OP_VARIABLE: {
			U32 storage_class = c->spirv.instr_operands[2];
			if (storage_class == HSC_SPIRV_STORAGE_CLASS_FUNCTION) {
				goto FUNCTIONS;
			} else {
				goto TYPES_VARIABLES_CONSTANTS;
			}
			break;
		};
		default:
			HSC_ABORT("unhandled spirv instruction op");
	}
	*count_ptr += c->spirv.instr_operands_count + 1;

	out[0] = (((c->spirv.instr_operands_count + 1) & 0xffff) << 16) | (c->spirv.instr_op & 0xffff);
	for (U32 i = 0; i < c->spirv.instr_operands_count; i += 1) {
		out[i + 1] = c->spirv.instr_operands[i];
	}

	printf("INSTRUCTION(%u): ", c->spirv.instr_op);
	for (U32 i = 1; i < c->spirv.instr_operands_count + 1; i += 1) {
		printf("%u, ", out[i]);
	}
	printf("\n");

	c->spirv.instr_op = HSC_SPIRV_OP_NO_OP;
}

void hsc_spirv_generate_pointer_type_input(HscCompiler* c, HscDataType data_type) {
	HSC_DEBUG_ASSERT(data_type < HSC_DATA_TYPE_MATRIX_END, "internal error: expected instrinic type but got '%u'", data_type);
	if (c->spirv.pointer_type_inputs_made_bitset[data_type / 64] & (1 << (data_type % 64))) {
		return;
	}

	c->spirv.pointer_type_inputs_made_bitset[data_type / 64] |= (1 << (data_type % 64));
	U32 id = c->spirv.pointer_type_inputs_base_id + data_type;

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_POINTER);
	hsc_spirv_instr_add_result_operand(c);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_INPUT);
	hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, data_type));
	hsc_spirv_instr_end(c);
}

void hsc_spirv_generate_pointer_type_output(HscCompiler* c, HscDataType data_type) {
	HSC_DEBUG_ASSERT(data_type < HSC_DATA_TYPE_MATRIX_END, "internal error: expected instrinic type but got '%u'", data_type);
	if (c->spirv.pointer_type_outputs_made_bitset[data_type / 64] & (1 << (data_type % 64))) {
		return;
	}

	c->spirv.pointer_type_outputs_made_bitset[data_type / 64] |= (1 << (data_type % 64));
	U32 id = c->spirv.pointer_type_outputs_base_id + data_type;

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_POINTER);
	hsc_spirv_instr_add_operand(c, id);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_OUTPUT);
	hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, data_type));
	hsc_spirv_instr_end(c);
}

enum {
	HSC_SPIRV_FUNCTION_CTRL_NONE         = 0x0,
	HSC_SPIRV_FUNCTION_CTRL_INLINE       = 0x1,
	HSC_SPIRV_FUNCTION_CTRL_DONT_INLINE  = 0x2,
	HSC_SPIRV_FUNCTION_CTRL_PURE         = 0x4,
	HSC_SPIRV_FUNCTION_CTRL_CONST        = 0x8,
};

U32 hsc_spirv_generate_function_type(HscCompiler* c, HscFunction* function) {
	if (function->shader_stage != HSC_FUNCTION_SHADER_STAGE_NONE) {
		return c->spirv.shader_stage_function_type_spirv_id;
	}

	U32 function_type_id = hsc_spirv_type_table_deduplicate_function(c, &c->spirv.type_table, function);

	if (function_type_id == c->spirv.next_id) {
		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_FUNCTION);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, function->return_data_type));
		HscLocalVariable* params = &c->astgen.function_params_and_local_variables[function->params_start_idx];
		for (U32 i = 0; i < function->params_count; i += 1) {
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, params[i].data_type));
		}
		hsc_spirv_instr_end(c);
		c->spirv.next_id += 1;
	}

	return function_type_id;
}

U32 hsc_spirv_generate_function_variable_type(HscCompiler* c, HscDataType data_type) {
	U32 type_id = hsc_spirv_type_table_deduplicate_function_variable(c, &c->spirv.type_table, data_type);
	if (type_id == c->spirv.next_id) {
		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_POINTER);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_FUNCTION);
		hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, data_type));
		hsc_spirv_instr_end(c);
		c->spirv.next_id += 1;
	}

	return type_id;
}

void hsc_spirv_generate_function(HscCompiler* c, U32 function_idx) {
	HscFunction* function = &c->astgen.functions[function_idx];
	HscIRFunction* ir_function = &c->ir.functions[function_idx];

	HscDataType return_data_type = function->return_data_type;
	switch (function->shader_stage) {
		case HSC_FUNCTION_SHADER_STAGE_VERTEX:
			return_data_type = HSC_DATA_TYPE_VOID;
			break;
		case HSC_FUNCTION_SHADER_STAGE_FRAGMENT:
			return_data_type = HSC_DATA_TYPE_VOID;
			break;
		case HSC_FUNCTION_SHADER_STAGE_NONE:
			break;
		default: HSC_ABORT("unhandle shader stage");
	}

	U32 function_type_id = hsc_spirv_generate_function_type(c, function);

	U32 function_spirv_id = c->spirv.next_id;
	hsc_spirv_instr_start(c, HSC_SPIRV_OP_FUNCTION);
	hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, return_data_type));
	hsc_spirv_instr_add_result_operand(c);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_FUNCTION_CTRL_NONE);
	hsc_spirv_instr_add_operand(c, function_type_id);
	hsc_spirv_instr_end(c);

	U32 frag_color_spirv_id;
	switch (function->shader_stage) {
		case HSC_FUNCTION_SHADER_STAGE_VERTEX:
			break;
		case HSC_FUNCTION_SHADER_STAGE_FRAGMENT:
			hsc_spirv_generate_pointer_type_output(c, HSC_DATA_TYPE_VEC4(HSC_DATA_TYPE_F32));

			frag_color_spirv_id = c->spirv.next_id;
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_VARIABLE);
			hsc_spirv_instr_add_operand(c, c->spirv.pointer_type_outputs_base_id + HSC_DATA_TYPE_VEC4(HSC_DATA_TYPE_F32));
			hsc_spirv_instr_add_result_operand(c);
			hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_OUTPUT);
			hsc_spirv_instr_end(c);

			hsc_spirv_instr_start(c, HSC_SPIRV_OP_DECORATE);
			hsc_spirv_instr_add_operand(c, frag_color_spirv_id);
			hsc_spirv_instr_add_operand(c, HSC_SPRIV_DECORATION_LOCATION);
			hsc_spirv_instr_add_operand(c, 0);
			hsc_spirv_instr_end(c);

			hsc_spirv_instr_start(c, HSC_SPIRV_OP_ENTRY_POINT);
			hsc_spirv_instr_add_operand(c, HSC_SPIRV_EXECUTION_MODEL_FRAGMENT);
			hsc_spirv_instr_add_operand(c, function_spirv_id);
			HscString name = hsc_string_table_get(&c->astgen.string_table, function->identifier_string_id);
			for (U32 i = 0; i < name.size; i += 4) {
				U32 word = 0;
				word |= name.data[i] << 0;
				if (i + 1 < name.size) word |= name.data[i + 1] << 8;
				if (i + 2 < name.size) word |= name.data[i + 2] << 16;
				if (i + 3 < name.size) word |= name.data[i + 3] << 24;
				hsc_spirv_instr_add_operand(c, word);
			}
			if (name.size % 4 == 0) {
				hsc_spirv_instr_add_operand(c, 0);
			}
			hsc_spirv_instr_add_operand(c, frag_color_spirv_id);
			hsc_spirv_instr_end(c);

			hsc_spirv_instr_start(c, HSC_SPIRV_OP_EXECUTION_MODE);
			hsc_spirv_instr_add_operand(c, function_spirv_id);
			hsc_spirv_instr_add_operand(c, HSC_SPIRV_EXECUTION_MODE_ORIGIN_LOWER_LEFT);
			hsc_spirv_instr_end(c);

			break;
		case HSC_FUNCTION_SHADER_STAGE_NONE:
			break;
		default: HSC_ABORT("unhandle shader stage");
	}

	c->spirv.basic_block_base_spirv_id = c->spirv.next_id;
	c->spirv.next_id += ir_function->basic_blocks_count;

	c->spirv.value_base_id = c->spirv.next_id;
	c->spirv.next_id += ir_function->values_count;

	if (function->shader_stage == HSC_FUNCTION_SHADER_STAGE_NONE) {
		for (U32 variable_idx = 0; variable_idx < function->params_count; variable_idx += 1) {
			HscLocalVariable* local_variable = &c->astgen.function_params_and_local_variables[function->params_start_idx + variable_idx];
			U32 type_spirv_id = hsc_spirv_generate_function_variable_type(c, local_variable->data_type);
		}
	}

	for (U32 variable_idx = function->params_count; variable_idx < function->local_variables_count; variable_idx += 1) {
		HscLocalVariable* local_variable = &c->astgen.function_params_and_local_variables[function->params_start_idx + variable_idx];
		U32 type_spirv_id = hsc_spirv_generate_function_variable_type(c, local_variable->data_type);
	}

	for (U32 basic_block_idx = ir_function->basic_blocks_start_idx; basic_block_idx < ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count; basic_block_idx += 1) {
		HscIRBasicBlock* basic_block = &c->ir.basic_blocks[basic_block_idx];

		hsc_spirv_instr_start(c, HSC_SPIRV_OP_LABEL);
		hsc_spirv_instr_add_operand(c, c->spirv.basic_block_base_spirv_id + (basic_block_idx - ir_function->basic_blocks_start_idx));
		hsc_spirv_instr_end(c);

		if (basic_block_idx == ir_function->basic_blocks_start_idx) {
			c->spirv.local_variable_base_spirv_id = c->spirv.next_id;
			for (U32 variable_idx = 0; variable_idx < function->params_count; variable_idx += 1) {
				if (function->shader_stage == HSC_FUNCTION_SHADER_STAGE_NONE) {
					HscLocalVariable* local_variable = &c->astgen.function_params_and_local_variables[function->params_start_idx + variable_idx];
					U32 type_spirv_id = hsc_spirv_generate_function_variable_type(c, local_variable->data_type);
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_FUNCTION_PARAMETER);
					hsc_spirv_instr_add_operand(c, type_spirv_id);
					hsc_spirv_instr_add_result_operand(c);
					hsc_spirv_instr_end(c);
				} else {
					c->spirv.next_id += 1;
				}
			}

			for (U32 variable_idx = function->params_count; variable_idx < function->local_variables_count; variable_idx += 1) {
				HscLocalVariable* local_variable = &c->astgen.function_params_and_local_variables[function->params_start_idx + variable_idx];
				U32 type_spirv_id = hsc_spirv_generate_function_variable_type(c, local_variable->data_type);
				hsc_spirv_instr_start(c, HSC_SPIRV_OP_VARIABLE);
				hsc_spirv_instr_add_operand(c, type_spirv_id);
				hsc_spirv_instr_add_result_operand(c);
				hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_FUNCTION);
				hsc_spirv_instr_end(c);
			}
		}

		for (U32 instruction_idx = basic_block->instructions_start_idx; instruction_idx < basic_block->instructions_start_idx + (U32)basic_block->instructions_count; instruction_idx += 1) {
			HscIRInstr* instruction = &c->ir.instructions[ir_function->instructions_start_idx + instruction_idx];
			HscIROperand* operands = &c->ir.operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
			switch (instruction->op_code) {
				case HSC_IR_OP_CODE_LOAD: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_LOAD);
					hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, hsc_ir_operand_data_type(&c->ir, &c->astgen, ir_function, operands[0])));
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_STORE: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_STORE);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_COMPOSITE_INIT: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_COMPOSITE_CONSTRUCT);
					U32 return_value_idx = HSC_IR_OPERAND_VALUE_IDX(operands[0]);
					HscIRValue* return_value = &c->ir.values[ir_function->values_start_idx + return_value_idx];

					hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, return_value->data_type));

					for (U32 i = 0; i < instruction->operands_count; i += 1) {
						hsc_spirv_instr_add_converted_operand(c, operands[i]);
					}

					U32 fields_count = hsc_data_type_composite_fields_count(&c->astgen, return_value->data_type);
					for (U32 i = instruction->operands_count; i < fields_count + 1; i += 1) {
						hsc_spirv_instr_add_converted_operand(c, operands[instruction->operands_count - 1]);
					}

					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_FUNCTION_RETURN: {
					if (function->shader_stage == HSC_FUNCTION_SHADER_STAGE_FRAGMENT) {
						hsc_spirv_instr_start(c, HSC_SPIRV_OP_STORE);
						hsc_spirv_instr_add_operand(c, frag_color_spirv_id);
						hsc_spirv_instr_add_converted_operand(c, operands[0]);
						hsc_spirv_instr_end(c);
					}

					if (return_data_type == HSC_DATA_TYPE_VOID) {
						hsc_spirv_instr_start(c, HSC_SPIRV_OP_RETURN);
						hsc_spirv_instr_end(c);
					} else {
						hsc_spirv_instr_start(c, HSC_SPIRV_OP_RETURN_VALUE);
						hsc_spirv_instr_add_converted_operand(c, operands[0]);
						hsc_spirv_instr_end(c);
					}

					break;
				};
				case HSC_IR_OP_CODE_LOOP_MERGE: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_LOOP_MERGE);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_add_operand(c, HSC_SPIRV_LOOP_CONTROL_NONE);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_SELECTION_MERGE: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_SELECTION_MERGE);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_operand(c, HSC_SPIRV_SELECTION_CONTROL_NONE);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_BINARY_OP(ADD):
				case HSC_IR_OP_CODE_BINARY_OP(SUBTRACT):
				case HSC_IR_OP_CODE_BINARY_OP(MULTIPLY):
				case HSC_IR_OP_CODE_BINARY_OP(DIVIDE):
				case HSC_IR_OP_CODE_BINARY_OP(MODULO):
				case HSC_IR_OP_CODE_BINARY_OP(BIT_AND):
				case HSC_IR_OP_CODE_BINARY_OP(BIT_OR):
				case HSC_IR_OP_CODE_BINARY_OP(BIT_XOR):
				case HSC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_LEFT):
				case HSC_IR_OP_CODE_BINARY_OP(BIT_SHIFT_RIGHT):
				case HSC_IR_OP_CODE_BINARY_OP(EQUAL):
				case HSC_IR_OP_CODE_BINARY_OP(NOT_EQUAL):
				case HSC_IR_OP_CODE_BINARY_OP(LESS_THAN):
				case HSC_IR_OP_CODE_BINARY_OP(LESS_THAN_OR_EQUAL):
				case HSC_IR_OP_CODE_BINARY_OP(GREATER_THAN):
				case HSC_IR_OP_CODE_BINARY_OP(GREATER_THAN_OR_EQUAL):
				{
					U32 return_value_idx = HSC_IR_OPERAND_VALUE_IDX(operands[0]);
					HscIRValue* return_value = &c->ir.values[ir_function->values_start_idx + return_value_idx];

					HscBinaryOp binary_op = instruction->op_code - HSC_IR_OP_CODE_BINARY_OP_START;
					HscBasicTypeClass type_class = hsc_basic_type_class(HSC_DATA_TYPE_SCALAR(hsc_ir_operand_data_type(&c->ir, &c->astgen, ir_function, operands[1])));
					printf("binary_op = %u, type_class = %u\n", binary_op, type_class);
					HscSpirvOp spirv_op = hsc_spriv_binary_ops[binary_op][type_class];
					HSC_DEBUG_ASSERT(spirv_op != HSC_SPIRV_OP_NO_OP, "internal error: invalid configuration for a binary op");

					hsc_spirv_instr_start(c, spirv_op);
					hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, return_value->data_type));
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_add_converted_operand(c, operands[2]);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT):
				case HSC_IR_OP_CODE_UNARY_OP(BIT_NOT):
				case HSC_IR_OP_CODE_UNARY_OP(NEGATE):
				{
					U32 return_value_idx = HSC_IR_OPERAND_VALUE_IDX(operands[0]);
					HscIRValue* return_value = &c->ir.values[ir_function->values_start_idx + return_value_idx];

					HscDataType scalar_data_type = HSC_DATA_TYPE_SCALAR(hsc_ir_operand_data_type(&c->ir, &c->astgen, ir_function, operands[1]));
					HscBasicTypeClass type_class = hsc_basic_type_class(scalar_data_type);

					HscUnaryOp unary_op = instruction->op_code - HSC_IR_OP_CODE_UNARY_OP_START;
					HscSpirvOp spirv_op = hsc_spriv_unary_ops[unary_op][type_class];
					HSC_DEBUG_ASSERT(spirv_op != HSC_SPIRV_OP_NO_OP, "internal error: invalid configuration for a unary op");

					hsc_spirv_instr_start(c, spirv_op);
					hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, return_value->data_type));
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_BRANCH: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_BRANCH);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_BRANCH_CONDITIONAL: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_BRANCH_CONDITIONAL);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_add_converted_operand(c, operands[2]);
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_SWITCH: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_SWITCH);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					for (U32 idx = 2; idx < instruction->operands_count; idx += 2) {
						HscConstant constant = hsc_constant_table_get(&c->astgen.constant_table, HSC_IR_OPERAND_CONSTANT_ID(operands[idx + 0]));

						U32 word;
						switch (constant.data_type) {
							case HSC_DATA_TYPE_U8:
							case HSC_DATA_TYPE_S8: word = *(U8*)constant.data; goto SWITCH_SINGLE_WORD_LITERAL;
							case HSC_DATA_TYPE_U16:
							case HSC_DATA_TYPE_S16: word = *(U16*)constant.data; goto SWITCH_SINGLE_WORD_LITERAL;
							case HSC_DATA_TYPE_U32:
							case HSC_DATA_TYPE_S32: word = *(U32*)constant.data; goto SWITCH_SINGLE_WORD_LITERAL;
SWITCH_SINGLE_WORD_LITERAL:
								hsc_spirv_instr_add_operand(c, word);
								break;
							case HSC_DATA_TYPE_U64:
							case HSC_DATA_TYPE_S64:
								word = ((U32*)constant.data)[0];
								hsc_spirv_instr_add_operand(c, word);
								word = ((U32*)constant.data)[1];
								hsc_spirv_instr_add_operand(c, word);
								break;
							default:
								HSC_UNREACHABLE("internal error: unhandle data type %u", constant.data_type);
						}

						hsc_spirv_instr_add_converted_operand(c, operands[idx + 1]);
					}
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_PHI: {
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_PHI);
					HscDataType data_type = hsc_ir_operand_data_type(&c->ir, &c->astgen, ir_function, operands[1]);
					hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, data_type));
					for (U32 idx = 0; idx < instruction->operands_count; idx += 1) {
						hsc_spirv_instr_add_converted_operand(c, operands[idx]);
					}
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_IR_OP_CODE_UNREACHABLE:
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_UNREACHABLE);
					hsc_spirv_instr_end(c);
					break;
				default:
					HSC_ABORT("unhandled instruction '%u'", instruction->op_code);
			}
		}
	}

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_FUNCTION_END);
	hsc_spirv_instr_end(c);
}

void hsc_spirv_generate_basic_types(HscCompiler* c) {
	hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_VOID);
	hsc_spirv_instr_add_result_operand(c);
	hsc_spirv_instr_end(c);

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_BOOL);
	hsc_spirv_instr_add_result_operand(c);
	hsc_spirv_instr_end(c);

	for (U32 i = 3; i < 7; i += 1) {
		HscDataType data_type = c->spirv.next_id - 1;
		if (!(c->available_basic_types & (1 << data_type))) {
			c->spirv.next_id += 1;
			continue;
		}

		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_INT);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, 1 << i);
		hsc_spirv_instr_add_operand(c, 0);
		hsc_spirv_instr_end(c);
	}

	for (U32 i = 3; i < 7; i += 1) {
		HscDataType data_type = c->spirv.next_id - 1;
		if (!(c->available_basic_types & (1 << data_type))) {
			c->spirv.next_id += 1;
			continue;
		}

		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_INT);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, 1 << i);
		hsc_spirv_instr_add_operand(c, 1);
		hsc_spirv_instr_end(c);
	}

	for (U32 i = 4; i < 7; i += 1) {
		HscDataType data_type = c->spirv.next_id - 1;
		if (!(c->available_basic_types & (1 << data_type))) {
			c->spirv.next_id += 1;
			continue;
		}

		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_FLOAT);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, 1 << i);
		hsc_spirv_instr_end(c);
	}

	U32 basic_type_padding = HSC_DATA_TYPE_VEC2_START - HSC_DATA_TYPE_BASIC_END;
	for (U32 i = 0; i < basic_type_padding; i += 1) {
		c->spirv.next_id += 1;
	}

	for (U32 j = 2; j < 5; j += 1) {
		c->spirv.next_id += 1; // skip HSC_DATA_TYPE_VOID
		for (U32 i = HSC_DATA_TYPE_BOOL; i < HSC_DATA_TYPE_BASIC_END; i += 1) {
			HscDataType data_type = c->spirv.next_id - 1;
			HscDataType scalar_data_type = HSC_DATA_TYPE_SCALAR(data_type);
			if (!(c->available_basic_types & (1 << scalar_data_type))) {
				c->spirv.next_id += 1;
				continue;
			}

			hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_VECTOR);
			hsc_spirv_instr_add_result_operand(c);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, i));
			hsc_spirv_instr_add_operand(c, j);
			hsc_spirv_instr_end(c);
		}

		for (U32 i = 0; i < basic_type_padding; i += 1) {
			c->spirv.next_id += 1;
		}
	}

	c->spirv.pointer_type_inputs_base_id = c->spirv.next_id;
	c->spirv.next_id += HSC_DATA_TYPE_MATRIX_END;

	c->spirv.pointer_type_outputs_base_id = c->spirv.next_id;
	c->spirv.next_id += HSC_DATA_TYPE_MATRIX_END;
}

void hsc_spirv_generate_constants(HscCompiler* c) {
	HscConstantTable* constant_table = &c->astgen.constant_table;
	c->spirv.constant_base_id = c->spirv.next_id;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[idx];
		if (entry->data_type == HSC_DATA_TYPE_BOOL) {
			bool is_true = c->astgen.true_constant_id.idx_plus_one == idx + 1;
			hsc_spirv_instr_start(c, is_true ? HSC_SPIRV_OP_CONSTANT_TRUE : HSC_SPIRV_OP_CONSTANT_FALSE);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, HSC_DATA_TYPE_BOOL));
			hsc_spirv_instr_add_result_operand(c);
			hsc_spirv_instr_end(c);
		} else if (HSC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_CONSTANT);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, entry->data_type));
			hsc_spirv_instr_add_result_operand(c);

			U32* data = HSC_PTR_ADD(constant_table->data, entry->start_idx);
			switch (entry->data_type) {
				case HSC_DATA_TYPE_U64:
				case HSC_DATA_TYPE_S64:
				case HSC_DATA_TYPE_F64:
					hsc_spirv_instr_add_operand(c, data[0]);
					hsc_spirv_instr_add_operand(c, data[1]);
					break;
				default:
					hsc_spirv_instr_add_operand(c, data[0]);
					break;
			}

			hsc_spirv_instr_end(c);
		} else {
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_CONSTANT_COMPOSITE);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, entry->data_type));
			hsc_spirv_instr_add_result_operand(c);

			HscConstantId* constants = HSC_PTR_ADD(constant_table->data, entry->start_idx);
			for (U32 i = 0; i < entry->size / sizeof(HscConstantId); i += 1) {
				hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + (constants[i].idx_plus_one - 1));
			}

			hsc_spirv_instr_end(c);
		}
	}
}

void hsc_spirv_write_binary_many(FILE* f, U32* words, U32 words_count, char* path) {
	U32 size = words_count * sizeof(U32);
	U32 written_size = fwrite(words, 1, size, f);
	HSC_ASSERT(size == written_size, "error writing file '%s'", path);
}

void hsc_spirv_write_binary(FILE* f, U32 word, char* path) {
	U32 written_size = fwrite(&word, 1, sizeof(U32), f);
	HSC_ASSERT(sizeof(U32) == written_size, "error writing file '%s'", path);
}

void hsc_spirv_generate_binary(HscCompiler* c) {
	char* path = "test.spv";
	FILE* f = fopen(path, "wb");
	HSC_ASSERT(f, "error opening file for write '%s'");

	U32 magic_number = 0x07230203;
	hsc_spirv_write_binary(f, magic_number, path);

	U32 major_version = 1;
	U32 minor_version = 3;
	U32 version = (major_version << 16) | (minor_version << 8);
	hsc_spirv_write_binary(f, version, path);

	U32 generator_number = 0; // TODO: when we are feeling ballsy enough, register with the khronos folks and get a number for the lang.
	hsc_spirv_write_binary(f, generator_number, path);

	hsc_spirv_write_binary(f, c->spirv.next_id, path);

	U32 reserved_instruction_schema = 0;
	hsc_spirv_write_binary(f, reserved_instruction_schema, path);

	hsc_spirv_write_binary_many(f, c->spirv.out_capabilities, c->spirv.out_capabilities_count, path);
	hsc_spirv_write_binary_many(f, c->spirv.out_entry_points, c->spirv.out_entry_points_count, path);
	hsc_spirv_write_binary_many(f, c->spirv.out_debug_info, c->spirv.out_debug_info_count, path);
	hsc_spirv_write_binary_many(f, c->spirv.out_annotations, c->spirv.out_annotations_count, path);
	hsc_spirv_write_binary_many(f, c->spirv.out_types_variables_constants, c->spirv.out_types_variables_constants_count, path);
	hsc_spirv_write_binary_many(f, c->spirv.out_functions, c->spirv.out_functions_count, path);

	fclose(f);
}

void hsc_spirv_generate(HscCompiler* c) {
	hsc_spirv_generate_basic_types(c);
	hsc_spirv_generate_constants(c);

	{
		c->spirv.shader_stage_function_type_spirv_id = c->spirv.next_id;
		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_FUNCTION);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, HSC_DATA_TYPE_VOID));
		hsc_spirv_instr_end(c);
	}

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_MEMORY_MODEL);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_ADDRESS_MODEL_LOGICAL);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_MEMORY_MODEL_GLSL450);
	hsc_spirv_instr_end(c);

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_CAPABILITY);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_CAPABILITY_SHADER);
	hsc_spirv_instr_end(c);

	for (U32 function_idx = HSC_FUNCTION_ID_USER_START; function_idx < c->astgen.functions_count; function_idx += 1) {
		hsc_spirv_generate_function(c, function_idx);
	}

	hsc_spirv_generate_binary(c);
}

// ===========================================
//
//
// Compiler
//
//
// ===========================================

void hsc_compiler_init(HscCompiler* compiler, HscCompilerSetup* setup) {
	hsc_constant_table_init(&compiler->astgen.constant_table, setup->string_table_data_cap, setup->string_table_entries_cap);
	{
		U8 value = false;
		compiler->astgen.false_constant_id = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_BOOL, &value);
		value = true;
		compiler->astgen.true_constant_id = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_BOOL, &value);
	}

	hsc_string_table_init(&compiler->astgen.string_table, setup->string_table_data_cap, setup->string_table_entries_cap);
	{
		for (U32 expected_string_id = HSC_STRING_ID_INTRINSIC_PARAM_NAMES_START; expected_string_id < HSC_STRING_ID_INTRINSIC_PARAM_NAMES_END; expected_string_id += 1) {
			char* string = hsc_string_intrinsic_param_names[expected_string_id];
			HscStringId id = hsc_string_table_deduplicate(&compiler->astgen.string_table, string, strlen(string));
			HSC_DEBUG_ASSERT(id.idx_plus_one == expected_string_id, "intrinsic string id for '%s' does not match! expected '%u' but got '%u'", string, expected_string_id, id.idx_plus_one);
		}

		for (HscToken t = HSC_TOKEN_KEYWORDS_START; t < HSC_TOKEN_KEYWORDS_END; t += 1) {
			char* string = hsc_token_strings[t];
			HscStringId id = hsc_string_table_deduplicate(&compiler->astgen.string_table, string, strlen(string));
			U32 expected_string_id = HSC_STRING_ID_KEYWORDS_START + (t - HSC_TOKEN_KEYWORDS_START);
			HSC_DEBUG_ASSERT(id.idx_plus_one == expected_string_id, "intrinsic string id for '%s' does not match! expected '%u' but got '%u'", string, expected_string_id, id.idx_plus_one);
		}

		for (HscToken t = HSC_TOKEN_INTRINSIC_TYPES_START; t < HSC_TOKEN_INTRINSIC_TYPES_END; t += 1) {
			char* string = hsc_token_strings[t];
			HscStringId id = hsc_string_table_deduplicate(&compiler->astgen.string_table, string, strlen(string));
			U32 expected_string_id = HSC_STRING_ID_INTRINSIC_TYPES_START + (t - HSC_TOKEN_INTRINSIC_TYPES_START);
			HSC_DEBUG_ASSERT(id.idx_plus_one == expected_string_id, "intrinsic string id for '%s' does not match! expected '%u' but got '%u'", string, expected_string_id, id.idx_plus_one);
		}
	}

	//hsc_opt_set_enabled(&compiler->astgen.opts, HSC_OPT_CONSTANT_FOLDING);

	hsc_astgen_init(&compiler->astgen, setup);
	hsc_ir_init(&compiler->ir);
	hsc_spirv_init(compiler);

	compiler->available_basic_types = 0xffff;
	compiler->available_basic_types &= ~( // remove support for these types for now, this is because they require SPIR-V capaibilities/vulkan features
		(1 << HSC_DATA_TYPE_U8)  |
		(1 << HSC_DATA_TYPE_S8)  |
		(1 << HSC_DATA_TYPE_U16) |
		(1 << HSC_DATA_TYPE_S16) |
		(1 << HSC_DATA_TYPE_F16) |
		(1 << HSC_DATA_TYPE_U64) |
		(1 << HSC_DATA_TYPE_S64) |
		(1 << HSC_DATA_TYPE_F64)
	);
}

void hsc_compiler_compile(HscCompiler* compiler, const char* file_path) {
	FILE* f = fopen(file_path, "rb");

	fseek(f, 0, SEEK_END);
	uint64_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	uint8_t* bytes = HSC_ALLOC(size + _HSC_TOKENIZER_LOOK_HEAD_SIZE, alignof(char));

	fread(bytes, 1, size, f);

	//
	// zero the look ahead so any tokenizer comparisions for equality will fail
	memset(bytes + size, 0x00, _HSC_TOKENIZER_LOOK_HEAD_SIZE);

	fclose(f);

	compiler->astgen.bytes = bytes;
	compiler->astgen.bytes_count = size;
	hsc_astgen_tokenize(&compiler->astgen);

	hsc_astgen_generate(&compiler->astgen);
	hsc_ir_generate(&compiler->ir, &compiler->astgen);
	hsc_spirv_generate(compiler);

	hsc_tokens_print(&compiler->astgen, stdout);
	hsc_astgen_print_ast(&compiler->astgen, stdout);
	hsc_ir_print(&compiler->ir, &compiler->astgen, stdout);
}

