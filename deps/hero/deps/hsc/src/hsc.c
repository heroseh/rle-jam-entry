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
	[HSC_DATA_TYPE_F8] = "F8",
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
	[HSC_TOKEN_PLUS] = "+",
	[HSC_TOKEN_MINUS] = "-",
	[HSC_TOKEN_FORWARD_SLASH] = "/",
	[HSC_TOKEN_ASTERISK] = "*",
	[HSC_TOKEN_PERCENT] = "%",
	[HSC_TOKEN_LIT_TRUE] = "true",
	[HSC_TOKEN_LIT_FALSE] = "false",
	[HSC_TOKEN_LIT_U32] = "U32",
	[HSC_TOKEN_LIT_U64] = "U64",
	[HSC_TOKEN_LIT_S32] = "S32",
	[HSC_TOKEN_LIT_S64] = "S64",
	[HSC_TOKEN_LIT_F32] = "F32",
	[HSC_TOKEN_LIT_F64] = "F64",
	[HSC_TOKEN_KEYWORD_RETURN] = "return",
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
			case HSC_DATA_TYPE_F8:
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

void hsc_data_type_print(HscAstGen* astgen, HscDataType data_type, void* data, FILE* f) {
	if (data_type < HSC_DATA_TYPE_BASIC_END) {
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
			case HSC_DATA_TYPE_S8: sint = *(S8*)data; goto SINT;
			case HSC_DATA_TYPE_S16: sint = *(S16*)data; goto SINT;
			case HSC_DATA_TYPE_S32: sint = *(S32*)data; goto SINT;
			case HSC_DATA_TYPE_S64: sint = *(S64*)data; goto SINT;
SINT:
				fprintf(f, "%zd", sint);
			case HSC_DATA_TYPE_F8:
			case HSC_DATA_TYPE_F16: HSC_ABORT("TODO");
			case HSC_DATA_TYPE_F32: float_ = *(F32*)data; goto FLOAT;
			case HSC_DATA_TYPE_F64: float_ = *(F64*)data; goto FLOAT;
FLOAT:
				fprintf(f, "%f", float_);
				break;
		}
	} else if (HSC_DATA_TYPE_VECTOR_START <= data_type && data_type < HSC_DATA_TYPE_MATRIX_END) {
		U32 componment_size;
		switch (HSC_DATA_TYPE_SCALAR(data_type)) {
			case HSC_DATA_TYPE_VOID:
				componment_size = 0;
				break;
			case HSC_DATA_TYPE_BOOL:
			case HSC_DATA_TYPE_U8:
			case HSC_DATA_TYPE_S8:
			case HSC_DATA_TYPE_F8:
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
		if (data_type < HSC_DATA_TYPE_VECTOR_END) {
			componments_count = HSC_DATA_TYPE_VECTOR_COMPONENTS(data_type);
			fprintf(f, "Vec%u(", componments_count);
		} else {
			U32 rows_count = HSC_DATA_TYPE_MATRX_ROWS(data_type);
			U32 columns_count = HSC_DATA_TYPE_MATRX_COLUMNS(data_type);
			componments_count = rows_count * columns_count;
			fprintf(f, "Mat%u%u(", rows_count, columns_count);
		}
		for (U32 i = 0; i < componments_count; i += 1) {
			hsc_data_type_print(astgen, HSC_DATA_TYPE_SCALAR(data_type), data, f);
			fprintf(f, i + 1 < componments_count ? ", " : ")");
			data = HSC_PTR_ADD(data, componment_size);
		}
	} else {
		HSC_ABORT("unhandle type '%u'", data_type);
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

HscConstantId hsc_constant_table_deduplicate(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type, void* data) {
	hsc_constant_table_deduplicate_start(constant_table, astgen, data_type);
	hsc_constant_table_deduplicate_add_data(constant_table, data, constant_table->data_size);
	return hsc_constant_table_deduplicate_end(constant_table);
}

void hsc_constant_table_deduplicate_start(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(constant_table->size == 0, "internal error: starting to deduplicate a constant before ending another");

	Uptr size;
	Uptr align;
	hsc_data_type_size_align(astgen, data_type, &size, &align);
	HSC_DEBUG_ASSERT(size, "internal error: cannot add a constant with zero size");
	constant_table->data_type = data_type;
	constant_table->size = 0;
	constant_table->data_size = size;
	constant_table->data_write_ptr = HSC_PTR_ROUND_UP_ALIGN(constant_table->data + constant_table->data_used_size, align);
}

void hsc_constant_table_deduplicate_add_data(HscConstantTable* constant_table, void* data, uint32_t size) {
	HSC_DEBUG_ASSERT(constant_table->data_size, "internal error: cannot add data when deduplication of constant has not started");
	HSC_DEBUG_ASSERT(constant_table->size + size <= constant_table->data_size, "internal error: the expected constant data size '%u' has been exceeded", constant_table->data_size);

	memcpy(HSC_PTR_ADD(constant_table->data_write_ptr, constant_table->size), data, size);
	constant_table->size += size;
}

void hsc_constant_table_deduplicate_add_constant(HscConstantTable* constant_table, HscConstantId constant_id) {
	HscConstant constant = hsc_constant_table_get(constant_table, constant_id);
	hsc_constant_table_deduplicate_add_data(constant_table, constant.data, constant.size);
}

HscConstantId hsc_constant_table_deduplicate_end(HscConstantTable* constant_table) {
	U32 data_size = constant_table->data_size;
	HSC_DEBUG_ASSERT(constant_table->size == constant_table->data_size, "internal error: the constant for deduplication is incomplete, expected to be a size of '%u' but got '%u'", constant_table->data_size, constant_table->size);
	constant_table->size = 0;
	constant_table->data_size = 0;

	//
	// TODO: make this a hash table look up
	for (uint32_t entry_idx = 0; entry_idx < constant_table->entries_count; entry_idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[entry_idx];

		if (entry->data_type == constant_table->data_type && data_size == entry->size && memcmp(constant_table->data + entry->start_idx, constant_table->data_write_ptr, data_size) == 0) {
			return (HscConstantId) { .idx_plus_one = entry_idx + 1 };
		}
	}

	if (constant_table->entries_count >= constant_table->entries_cap) {
		HSC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	if (constant_table->data_used_size + data_size >= constant_table->data_cap) {
		HSC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	uint32_t new_entry_idx = constant_table->entries_count;
	constant_table->entries_count += 1;
	HscConstantEntry* entry = &constant_table->entries[new_entry_idx];
	entry->start_idx = constant_table->data_write_ptr - constant_table->data;
	entry->size = data_size;
	entry->data_type = constant_table->data_type;

	constant_table->data_used_size += constant_table->data_write_ptr - constant_table->data + data_size;

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
	function->params_start_idx = astgen->function_params_count;
	function->return_data_type = intrinsic_function->return_data_type;

	HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_count + intrinsic_function->params_count - 1, astgen->function_params_cap);
	HSC_COPY_ELMT_MANY(&astgen->function_params[astgen->function_params_count], intrinsic_function->params, intrinsic_function->params_count);
	astgen->function_params_count += intrinsic_function->params_count;
}

void hsc_astgen_init(HscAstGen* astgen, HscCompilerSetup* setup) {
	astgen->function_params = HSC_ALLOC_ARRAY(HscFunctionParam, setup->function_params_cap);
	HSC_ASSERT(astgen->function_params, "out of memory");
	astgen->functions = HSC_ALLOC_ARRAY(HscFunction, setup->functions_cap);
	HSC_ASSERT(astgen->functions, "out of memory");
	astgen->exprs = HSC_ALLOC_ARRAY(HscExpr, setup->exprs_cap);
	HSC_ASSERT(astgen->exprs, "out of memory");
	astgen->expr_locations = HSC_ALLOC_ARRAY(HscLocation, setup->exprs_cap);
	HSC_ASSERT(astgen->expr_locations, "out of memory");
	astgen->function_params_cap = setup->function_params_cap;
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
			} else if (u64 > S32_MAX) {
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
	token_value.constant_id = hsc_constant_table_deduplicate(&astgen->constant_table, astgen, data_type, data);
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
			case '+': token = HSC_TOKEN_PLUS; break;
			case '-':
				if (isdigit(astgen->bytes[astgen->location.code_end_idx + 1])) {
					token_size = hsc_parse_num(astgen, &token);
				} else {
					token = HSC_TOKEN_MINUS;
				}
				break;
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
				}

				token = HSC_TOKEN_FORWARD_SLASH;
				break;
			};
			case '*': token = HSC_TOKEN_ASTERISK; break;
			case '%': token = HSC_TOKEN_PERCENT; break;
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
	switch (token) {
		case HSC_TOKEN_LIT_TRUE:
		case HSC_TOKEN_LIT_FALSE:
		case HSC_TOKEN_LIT_U32:
		case HSC_TOKEN_LIT_U64:
		case HSC_TOKEN_LIT_F32:
		case HSC_TOKEN_LIT_F64: {
			HscDataType data_type;
			HscConstantId constant_id;
			HscTokenValue value;
			switch (token) {
				case HSC_TOKEN_LIT_TRUE:
					data_type = HSC_DATA_TYPE_BOOL;
					constant_id = astgen->true_constant_id;
					break;
				case HSC_TOKEN_LIT_FALSE:
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
				HSC_ABORT("TODO:");
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
		default:
			hsc_astgen_error_1(astgen, "expected an expression here");
	}
}

void hsc_astgen_generate_binary_op(HscAstGen* astgen, HscExprType* binary_op_type_out, U32* precedence_out) {
	HscToken token = hsc_token_peek(astgen);
	switch (token) {
		case HSC_TOKEN_PARENTHESIS_OPEN:
			*binary_op_type_out = HSC_EXPR_TYPE_CALL;
			*precedence_out = 1;
			break;
		case HSC_TOKEN_ASTERISK:
			//*binary_op_type_out = HSC_EXPR_TYPE_MULTIPLY;
			*precedence_out = 3;
			break;
		case HSC_TOKEN_FORWARD_SLASH:
			//*binary_op_type_out = HSC_EXPR_TYPE_DIVIDE;
			*precedence_out = 3;
			break;
		case HSC_TOKEN_PERCENT:
			//*binary_op_type_out = HSC_EXPR_TYPE_MODULO;
			*precedence_out = 3;
			break;
		case HSC_TOKEN_PLUS:
			//*binary_op_type_out = HSC_EXPR_TYPE_ADD;
			*precedence_out = 4;
			break;
		case HSC_TOKEN_MINUS:
			//*binary_op_type_out = HSC_EXPR_TYPE_SUBTRACT;
			*precedence_out = 4;
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
	char* TODO_get_type_names = "TODO_GET_TYPE_NAME";
	hsc_astgen_error_2(astgen, other_location, "type mismatch '%s' is does not implicitly cast to '%s'", TODO_get_type_names, TODO_get_type_names);
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
		HscFunctionParam* params = &astgen->function_params[function->params_start_idx];
		U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];
		HscExpr* arg_expr = call_args_expr;
		bool result = true;
		astgen->generic_data_type_state = (HscGenericDataTypeState){0};
		for (U32 i = 0; i < args_count; i += 1) {
			arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
			HscFunctionParam* param = &params[i];

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
		HscFunctionParam* param = &astgen->function_params[function->params_start_idx + param_idx];
		HscString type_name = hsc_data_type_string(astgen, param->data_type);
		HscString param_name = hsc_string_table_get(&astgen->string_table, param->identifier_string_id);
		cursor += snprintf(buf + cursor, buf_size - cursor, function_fmt, (int)type_name.size, type_name.data, (int)param_name.size, param_name.data);
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
	call_args_expr->is_stmt_block_entry = false;
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
	hsc_token_next(astgen);

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
			hsc_constant_table_deduplicate_start(&astgen->constant_table, astgen, return_data_type);
			HscExpr* arg_expr = call_args_expr;
			U32 args_count = ((U8*)call_args_expr)[1];
			U8* next_arg_expr_rel_indices = &((U8*)call_args_expr)[2];
			for (U32 i = 0; i < args_count; i += 1) {
				arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
				HscConstantId constant_id = { .idx_plus_one = arg_expr->constant.id };
				U32 repeat_count = is_single ? componments_count : 1;
				for (U32 j = 0; j < repeat_count; j += 1) {
					hsc_constant_table_deduplicate_add_constant(&astgen->constant_table, constant_id);
				}
			}
			HscConstantId constant_id = hsc_constant_table_deduplicate_end(&astgen->constant_table);

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
		hsc_astgen_generate_binary_op(astgen, &binary_op_type, &precedence);
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
			HscDataType data_type = 0; // TODO implicit conversions

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
				expr->data_type = data_type;
				left_expr = expr;
			}
		}
	}
}

HscExpr* hsc_astgen_generate_stmt(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	switch (token) {
		case HSC_TOKEN_CURLY_OPEN: {
			hsc_astgen_variable_stack_open(astgen);

			U32 prev_stmt_expr_idx = U32_MAX;
			HscExpr* prev_stmt = NULL;

			HscExpr* stmt_block = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_BLOCK);
			U32 stmts_count = 0;

			token = hsc_token_next(astgen);
			while (token != HSC_TOKEN_CURLY_CLOSE) {
				HscExpr* stmt = hsc_astgen_generate_stmt(astgen);
				stmt->is_stmt_block_entry = true;
				U32 stmt_expr_idx = stmt - astgen->exprs;

				if (prev_stmt_expr_idx != U32_MAX) {
					stmt->prev_expr_rel_idx = stmt_expr_idx - prev_stmt_expr_idx;
				}
				if (prev_stmt) {
					prev_stmt->next_expr_rel_idx = stmt_expr_idx - prev_stmt_expr_idx;
				} else {
					U32 stmt_block_expr_idx = stmt_block - astgen->exprs;
					stmt_block->stmt_block.first_expr_rel_idx = stmt_expr_idx - stmt_block_expr_idx;
				}

				stmts_count += 1;
				token = hsc_token_peek(astgen);
				prev_stmt_expr_idx = stmt_expr_idx;
				prev_stmt = stmt;
			}

			stmt_block->stmt_block.stmts_count = stmts_count;
			hsc_astgen_variable_stack_close(astgen);
			token = hsc_token_next(astgen);
			return stmt_block;
		};
		case HSC_TOKEN_KEYWORD_RETURN: {
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_RETURN);
			hsc_token_next(astgen);
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			stmt->unary.expr_idx = expr - stmt;
			return stmt;
		};
		default:
			hsc_astgen_error_1(astgen, "expected an expression here");
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

	function->params_start_idx = astgen->function_params_count;
	function->params_count = 0;
	token = hsc_token_next(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
		while (1) {
			HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_count, astgen->function_params_cap);
			HscFunctionParam* param = &astgen->function_params[astgen->function_params_count];
			function->params_count += 1;
			astgen->function_params_count += 1;

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

void hsc_astgen_variable_stack_add(HscAstGen* astgen, HscStringId string_id) {
	HSC_ASSERT(astgen->variable_stack_count < astgen->variable_stack_cap, "variable stack is full");
	astgen->variable_stack_strings[astgen->variable_stack_count] = string_id;
	astgen->variable_stack_var_indices[astgen->variable_stack_count] = astgen->next_var_idx;
	astgen->variable_stack_count += 1;
	astgen->next_var_idx += 1;
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

void hsc_astgen_print_expr(HscAstGen* astgen, HscExpr* expr, U32 indent, bool is_stmt, FILE* f) {
	static char* indent_chars = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	fprintf(f, "%.*s", indent, indent_chars);
	switch (expr->type) {
		case HSC_EXPR_TYPE_FUNCTION:
		case HSC_EXPR_TYPE_CALL_ARG_LIST:
			break;
		default: {
			if (!is_stmt) {
				HscString data_type_name = hsc_data_type_string(astgen, expr->data_type);
				fprintf(f, "(%.*s)", (int)data_type_name.size, data_type_name.data);
			}
			break;
		};
	}
	const char* expr_name;
	switch (expr->type) {
		case HSC_EXPR_TYPE_CONSTANT: {
			fprintf(f, "EXPR_CONSTANT ");
			HscConstantId constant_id = { .idx_plus_one = expr->constant.id };
			HscConstant constant = hsc_constant_table_get(&astgen->constant_table, constant_id);
			hsc_data_type_print(astgen, expr->data_type, constant.data, f);
			break;
		};
		case HSC_EXPR_TYPE_STMT_BLOCK: {
			U32 stmts_count = expr->stmt_block.stmts_count;
			fprintf(f, "EXPR_STMT_BLOCK[%u] {\n", stmts_count);
			HscExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
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
		case HSC_EXPR_TYPE_STMT_RETURN:
			expr_name = "EXPR_RETURN";
			goto UNARY;
UNARY:
		{
			fprintf(f, "%s: {\n", expr_name);
			HscExpr* unary_expr = &expr[expr->unary.expr_idx];
			hsc_astgen_print_expr(astgen, unary_expr, indent + 1, false, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_CALL:
			expr_name = "EXPR_CALL";
			goto BINARY;
BINARY:
		{
			fprintf(f, "%s: {\n", expr_name);
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			hsc_astgen_print_expr(astgen, left_expr, indent + 1, false, f);
			hsc_astgen_print_expr(astgen, right_expr, indent + 1, false, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_CALL_ARG_LIST: {
			fprintf(f, "EXPR_CALL_ARG_LIST {\n");
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
				HscFunctionParam* param = &astgen->function_params[function->params_start_idx + param_idx];
				HscString type_name = hsc_data_type_string(astgen, param->data_type);
				HscString param_name = hsc_string_table_get(&astgen->string_table, param->identifier_string_id);
				fprintf(f, "\t\t%.*s %.*s\n", (int)type_name.size, type_name.data, (int)param_name.size, param_name.data);
			}
			fprintf(f, "\t}\n");
		}
		if (function->block_expr_id.idx_plus_one) {
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
	ir->constant_values = HSC_ALLOC_ARRAY(U8, 8192);
	HSC_ASSERT(ir->constant_values, "out of memory");
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
	ir->constant_values_size_cap = 8192;
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

void hsc_ir_add_instrunction(HscIR* ir, HscIRFunction* ir_function, HscIROpCode op_code, HscIROperand* operands, U32 operands_count) {
	HscIRInstr* instruction = &ir->instructions[ir_function->instructions_start_idx + (U32)ir_function->instructions_count];
	instruction->op_code = op_code;
	instruction->operands_start_idx = (operands - ir->operands) - ir_function->operands_start_idx;
	instruction->operands_count = operands_count;
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

HscIRBasicBlock* hsc_ir_generate_instructions_from_intrinsic_function(HscIR* ir, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr) {

	return basic_block;
}

HscIRBasicBlock* hsc_ir_generate_instructions(HscIR* ir, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr) {
	switch (expr->type) {
		case HSC_EXPR_TYPE_STMT_BLOCK: {
			if (!basic_block) {
				basic_block = hsc_ir_add_basic_block(ir, ir_function);
			}

			U32 stmts_count = expr->stmt_block.stmts_count;
			HscExpr* stmt = &expr[expr->stmt_block.first_expr_rel_idx];
			for (U32 i = 0; i < stmts_count; i += 1) {
				basic_block = hsc_ir_generate_instructions(ir, ir_function, basic_block, stmt);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}

			break;
		};
		case HSC_EXPR_TYPE_STMT_RETURN: {
			HscExpr* unary_expr = &expr[expr->unary.expr_idx];
			basic_block = hsc_ir_generate_instructions(ir, ir_function, basic_block, unary_expr);
			U32 value_idx = ir->last_value_idx;

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			operands[0].value_idx = value_idx;
			hsc_ir_add_instrunction(ir, ir_function, HSC_IR_OP_CODE_FUNCTION_RETURN, operands, 1);
			break;
		};
		case HSC_EXPR_TYPE_CALL: {
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			HSC_DEBUG_ASSERT(left_expr->type == HSC_EXPR_TYPE_FUNCTION, "expected an function expression");
			HSC_DEBUG_ASSERT(right_expr->type == HSC_EXPR_TYPE_CALL_ARG_LIST, "expected call argument list expression");
			if (left_expr->function.id < HSC_FUNCTION_ID_USER_START) {
				basic_block = hsc_ir_generate_instructions_from_intrinsic_function(ir, ir_function, basic_block, expr);
			} else {
				HscExpr* arg_expr = right_expr;
				U32 args_count = ((U8*)right_expr)[1];
				U8* next_arg_expr_rel_indices = &((U8*)right_expr)[2];
				HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, args_count);
				for (U32 i = 0; i < args_count; i += 1) {
					arg_expr = &arg_expr[next_arg_expr_rel_indices[i]];
					basic_block = hsc_ir_generate_instructions(ir, ir_function, basic_block, arg_expr);
					operands[i].value_idx = ir->last_value_idx;
				}

				hsc_ir_add_instrunction(ir, ir_function, HSC_IR_OP_CODE_FUNCTION_CALL, operands, args_count);
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
	hsc_ir_generate_instructions(ir, ir_function, NULL, expr);
}

void hsc_ir_generate(HscIR* ir, HscAstGen* astgen) {
	for (U32 function_idx = HSC_FUNCTION_ID_USER_START; function_idx < astgen->functions_count; function_idx += 1) {

	}
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
		compiler->astgen.false_constant_id = hsc_constant_table_deduplicate(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_BOOL, &value);
		value = true;
		compiler->astgen.true_constant_id = hsc_constant_table_deduplicate(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_BOOL, &value);
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

	hsc_opt_set_enabled(&compiler->astgen.opts, HSC_OPT_CONSTANT_FOLDING);

	hsc_astgen_init(&compiler->astgen, setup);
	hsc_ir_init(&compiler->ir);
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
	hsc_astgen_print_ast(&compiler->astgen, stdout);
}

