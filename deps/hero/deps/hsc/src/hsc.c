#include "hsc.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

#define HSC_DEBUG_CODE_SPAN_PUSH_POP 0
#define HSC_DEBUG_CODE_PREPROCESSOR 0

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
// Platform Abstraction
//
//
// ===========================================

void hsc_get_last_system_error_string(char* buf_out, U32 buf_out_size) {
#if _WIN32
	DWORD error = GetLastError();
	DWORD res = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPTSTR)buf_out, buf_out_size, NULL);
	if (res == 0) {
		error = GetLastError();
		HSC_ABORT("TODO handle error code: %u", error);
	}
#elif _GNU_SOURCE
	int error = errno;
	// GNU version (screw these guys for changing the way this works)
	char* buf = strerror_r(error, buf_out, buf_out_size);
	if (strcmp(buf, "Unknown error") == 0) {
		goto ERROR_1;
	}

	// if its static string then copy it to the buffer
	if (buf != buf_out) {
		strncpy(buf_out, buf, buf_out_size);

		U32 size = strsize(buf);
		if (size < buf_out_size) {
			goto ERROR_2;
		}
	}
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int error = errno;
	// use the XSI standard behavior.
	int res = strerror_r(error, buf_out, buf_out_size);
	if (res != 0) {
		int errnum = res;
		if (res == -1)
			errnum = errno;

		if (errnum == EINVAL) {
			goto ERROR_1;
		} else if (errnum == ERANGE) {
			goto ERROR_2;
		}
		HSC_ABORT("unexpected errno: %u", errnum);
	}
#else
#error "unimplemented for this platform"
#endif
	return;
ERROR_1:
	HSC_ABORT("invalid error %u", error);
ERROR_2:
	printf("internal warning: error buffer of size '%u' is not big enough", buf_out_size);
	HSC_ABORT("invalid error %u", error);
}

bool hsc_file_exist(char* path) {
#ifdef __linux__
	return access(path, F_OK) == 0;
#else
#error "unimplemented for this platform"
#endif
}

U8* hsc_file_read_all_the_codes(char* path, U64* size_out) {
#ifdef __linux__
	int fd_flags = O_CLOEXEC | O_RDONLY;
	int mode = 0666;
	int fd = open(path, fd_flags, mode);
	if (fd == -1) {
		return NULL;
	}

	struct stat s = {0};
	if (fstat(fd, &s) != 0) return NULL;
	*size_out = s.st_size;

	U8* bytes = HSC_ALLOC(s.st_size + _HSC_TOKENIZER_LOOK_HEAD_SIZE, alignof(U8));

	U64 remaining_read_size = s.st_size;
	U64 offset = 0;
	while (remaining_read_size) {
		ssize_t bytes_read = read(fd, bytes + offset, remaining_read_size);
		if (bytes_read == (ssize_t)-1) {
			return NULL;
		}
		offset += bytes_read;
		remaining_read_size -= bytes_read;
	}

	close(fd);

	return bytes;
#else
#error "unimplemented for this platform"
#endif
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
			if (value_out) *value_out = hash_table->values[idx];
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

bool hsc_hash_table_remove(HscHashTable* hash_table, U32 key, U32* value_out) {
	for (uint32_t idx = 0; idx < hash_table->count; idx += 1) {
		U32 found_key = hash_table->keys[idx];
		if (found_key == key) {
			if (value_out) {
				*value_out = hash_table->values[idx];
			}
			if (idx + 1 < hash_table->count) {
				memmove(&hash_table->keys[idx], &hash_table->keys[idx + 1], (hash_table->cap - idx - 1) * sizeof(U32));
				memmove(&hash_table->values[idx], &hash_table->values[idx + 1], (hash_table->cap - idx - 1) * sizeof(U32));
			}
			hash_table->count -= 1;
			return true;
		}

	}
	return false;
}

void hsc_hash_table_clear(HscHashTable* hash_table) {
	hash_table->count = 0;
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
	[HSC_DATA_TYPE_BOOL] = "_Bool",
	[HSC_DATA_TYPE_U8] = "uint8_t",
	[HSC_DATA_TYPE_U16] = "uint16_t",
	[HSC_DATA_TYPE_U32] = "uint32_t",
	[HSC_DATA_TYPE_U64] = "uint64_t",
	[HSC_DATA_TYPE_S8] = "int8_t",
	[HSC_DATA_TYPE_S16] = "int16_t",
	[HSC_DATA_TYPE_S32] = "int32_t",
	[HSC_DATA_TYPE_S64] = "int64_t",
	[HSC_DATA_TYPE_F16] = "half",
	[HSC_DATA_TYPE_F32] = "float",
	[HSC_DATA_TYPE_F64] = "double",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC2] = "vec2_t",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC3] = "vec3_t",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC4] = "vec4_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X2] = "mat2x2_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X3] = "mat2x3_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X4] = "mat2x4_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X2] = "mat3x2_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X3] = "mat3x3_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X4] = "mat3x4_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X2] = "mat4x2_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X3] = "mat4x3_t",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X4] = "mat4x4_t",
	[HSC_TOKEN_EOF] = "end of file",
	[HSC_TOKEN_IDENT] = "identifier",
	[HSC_TOKEN_CURLY_OPEN] = "{",
	[HSC_TOKEN_CURLY_CLOSE] = "}",
	[HSC_TOKEN_PARENTHESIS_OPEN] = "(",
	[HSC_TOKEN_PARENTHESIS_CLOSE] = ")",
	[HSC_TOKEN_SQUARE_OPEN] = "[",
	[HSC_TOKEN_SQUARE_CLOSE] = "]",
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
	[HSC_TOKEN_QUESTION_MARK] = "?",
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
	[HSC_TOKEN_INCREMENT] = "++",
	[HSC_TOKEN_DECREMENT] = "--",
	[HSC_TOKEN_LIT_U32] = "U32",
	[HSC_TOKEN_LIT_U64] = "U64",
	[HSC_TOKEN_LIT_S32] = "S32",
	[HSC_TOKEN_LIT_S64] = "S64",
	[HSC_TOKEN_LIT_F32] = "F32",
	[HSC_TOKEN_LIT_F64] = "F64",
	[HSC_TOKEN_KEYWORD_RETURN] = "return",
	[HSC_TOKEN_KEYWORD_IF] = "if",
	[HSC_TOKEN_KEYWORD_ELSE] = "else",
	[HSC_TOKEN_KEYWORD_DO] = "do",
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
	[HSC_TOKEN_KEYWORD_ENUM] = "enum",
	[HSC_TOKEN_KEYWORD_STRUCT] = "struct",
	[HSC_TOKEN_KEYWORD_UNION] = "union",
	[HSC_TOKEN_KEYWORD_TYPEDEF] = "typedef",
	[HSC_TOKEN_KEYWORD_STATIC] = "static",
	[HSC_TOKEN_KEYWORD_CONST] = "const",
	[HSC_TOKEN_KEYWORD_AUTO] = "auto",
	[HSC_TOKEN_KEYWORD_REGISTER] = "register",
	[HSC_TOKEN_KEYWORD_VOLATILE] = "volatile",
	[HSC_TOKEN_KEYWORD_EXTERN] = "extern",
	[HSC_TOKEN_KEYWORD_INLINE] = "inline",
	[HSC_TOKEN_KEYWORD_NO_RETURN] = "_Noreturn",
	[HSC_TOKEN_KEYWORD_SIZEOF] = "sizeof",
	[HSC_TOKEN_KEYWORD_ALIGNOF] = "_Alignof",
	[HSC_TOKEN_KEYWORD_ALIGNAS] = "_Alignas",
	[HSC_TOKEN_KEYWORD_STATIC_ASSERT] = "_Static_assert",
	[HSC_TOKEN_KEYWORD_RESTRICT] = "restrict",
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

HscIntrinsicFunction hsc_intrinsic_functions[HSC_FUNCTION_IDX_INTRINSIC_END] = {
	[HSC_FUNCTION_IDX_VEC2] = {
		.name = "vec2",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC2,
		.params_count = 2,
		.params = {
			{ .identifier_string_id = { HSC_STRING_ID_X }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = { HSC_STRING_ID_Y }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_IDX_VEC3] = {
		.name = "vec3",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC3,
		.params_count = 3,
		.params = {
			{ .identifier_string_id = { HSC_STRING_ID_X }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = { HSC_STRING_ID_Y }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = { HSC_STRING_ID_Z }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
		},
	},
	[HSC_FUNCTION_IDX_VEC4] = {
		.name = "vec4",
		.return_data_type = HSC_DATA_TYPE_GENERIC_VEC4,
		.params_count = 4,
		.params = {
			{ .identifier_string_id = { HSC_STRING_ID_X }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = { HSC_STRING_ID_Y }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = { HSC_STRING_ID_Z }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
			{ .identifier_string_id = { HSC_STRING_ID_W }, .data_type = HSC_DATA_TYPE_GENERIC_SCALAR, },
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

HscArrayDataType* hsc_array_data_type_get(HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_ARRAY(data_type), "internal error: expected array data type");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DATA_TYPE_IDX(data_type), astgen->array_data_types_count);
	return &astgen->array_data_types[HSC_DATA_TYPE_IDX(data_type)];
}

HscEnumDataType* hsc_enum_data_type_get(HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_ENUM_TYPE(data_type), "internal error: expected enum data type");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DATA_TYPE_IDX(data_type), astgen->enum_data_types_count);
	return &astgen->enum_data_types[HSC_DATA_TYPE_IDX(data_type)];
}

HscCompoundDataType* hsc_compound_data_type_get(HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_COMPOUND_TYPE(data_type), "internal error: expected compound data type");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DATA_TYPE_IDX(data_type), astgen->compound_data_types_count);
	return &astgen->compound_data_types[HSC_DATA_TYPE_IDX(data_type)];
}

HscCompoundField* _hsc_compound_data_type_find_field_by_name(HscAstGen* astgen, HscCompoundDataType* compound_data_type, HscStringId identifier_string_id, U32 nested_count) {
	HSC_ASSERT_ARRAY_BOUNDS(nested_count - 1, HSC_COMPOUND_TYPE_NESTED_FIELD_CAP);
	for (U32 field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
		HscCompoundField* field = &astgen->compound_fields[compound_data_type->fields_start_idx + field_idx];
		astgen->compound_type_find_fields[nested_count - 1].data_type = field->data_type;
		astgen->compound_type_find_fields[nested_count - 1].idx = field_idx;
		if (field->identifier_string_id.idx_plus_one == 0) {
			HscCompoundDataType* field_compound_data_type = hsc_compound_data_type_get(astgen, field->data_type);
			HscCompoundField* nested_field = _hsc_compound_data_type_find_field_by_name(astgen, field_compound_data_type, identifier_string_id, nested_count + 1);
			if (nested_field) {
				return nested_field;
			}
		}
		if (field->identifier_string_id.idx_plus_one == identifier_string_id.idx_plus_one) {
			astgen->compound_type_find_fields_count = nested_count;
			return field;
		}
	}

	return NULL;
}

HscCompoundField* hsc_compound_data_type_find_field_by_name(HscAstGen* astgen, HscCompoundDataType* compound_data_type, HscStringId identifier_string_id) {
	return _hsc_compound_data_type_find_field_by_name(astgen, compound_data_type, identifier_string_id, 1);
}

HscCompoundField* hsc_compound_data_type_find_field_by_name_checked(HscAstGen* astgen, HscDataType data_type, HscCompoundDataType* compound_data_type, HscStringId identifier_string_id) {
	HscCompoundField* field = hsc_compound_data_type_find_field_by_name(astgen, compound_data_type, identifier_string_id);
	if (field == NULL) {
		HscString data_type_name = hsc_data_type_string(astgen, data_type);
		HscString identifier_string = hsc_string_table_get(&astgen->string_table, identifier_string_id);
		hsc_astgen_error_2(astgen, compound_data_type->identifier_token_idx, "cannot find a '%.*s' field in the '%.*s' type", (int)identifier_string.size, identifier_string.data, (int)data_type_name.size, data_type_name.data);
	}
	return field;
}

void _hsc_compound_data_type_validate_field_names(HscAstGen* astgen, HscDataType outer_data_type, HscCompoundDataType* compound_data_type) {
	for (U32 field_idx = 0; field_idx < compound_data_type->fields_count; field_idx += 1) {
		HscCompoundField* field = &astgen->compound_fields[compound_data_type->fields_start_idx + field_idx];
		if (field->identifier_string_id.idx_plus_one == 0) {
			HscCompoundDataType* field_compound_data_type = hsc_compound_data_type_get(astgen, field->data_type);
			_hsc_compound_data_type_validate_field_names(astgen, outer_data_type, field_compound_data_type);
		} else {
			U32* dst_token_idx;
			bool result = hsc_hash_table_find_or_insert(&astgen->field_name_to_token_idx, field->identifier_string_id.idx_plus_one, &dst_token_idx);
			if (result) {
				astgen->token_read_idx = field->identifier_token_idx;
				HscString field_identifier_string = hsc_string_table_get(&astgen->string_table, field->identifier_string_id);
				HscString data_type_name = hsc_data_type_string(astgen, outer_data_type);
				hsc_astgen_error_2(astgen, *dst_token_idx, "duplicate field identifier '%.*s' in '%.*s'", (int)field_identifier_string.size, field_identifier_string.data, (int)data_type_name.size, data_type_name.data);
			}
			*dst_token_idx = field->identifier_token_idx;
		}
	}
}

HscTypedef* hsc_typedef_get(HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_TYPEDEF(data_type), "internal error: expected typedef");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DATA_TYPE_IDX(data_type), astgen->typedefs_count);
	return &astgen->typedefs[HSC_DATA_TYPE_IDX(data_type)];
}

HscDataType hsc_typedef_resolve(HscAstGen* astgen, HscDataType data_type) {
	while (1) {
		data_type = HSC_DATA_TYPE_STRIP_CONST(data_type);
		switch (data_type & 0xff) {
			case HSC_DATA_TYPE_ENUM:
				return HSC_DATA_TYPE_S32;
			case HSC_DATA_TYPE_TYPEDEF: {
				HscTypedef* typedef_ = hsc_typedef_get(astgen, data_type);
				data_type = typedef_->aliased_data_type;
				break;
			};
			default:
				return data_type;
		}
	}
}

HscMacro* hsc_macro_get(HscAstGen* astgen, U32 macro_idx) {
	HSC_ASSERT_ARRAY_BOUNDS(macro_idx, astgen->macros_cap);
	return &astgen->macros[macro_idx];
}

HscFunction* hsc_function_get(HscAstGen* astgen, HscDecl decl) {
	HSC_DEBUG_ASSERT(HSC_DECL_IS_FUNCTION(decl), "internal error: expected a function declaration");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DECL_IDX(decl), astgen->functions_count);
	return &astgen->functions[HSC_DECL_IDX(decl)];
}

HscEnumValue* hsc_enum_value_get(HscAstGen* astgen, HscDecl decl) {
	HSC_DEBUG_ASSERT(HSC_DECL_IS_ENUM_VALUE(decl), "internal error: expected a enum value");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DATA_TYPE_IDX(decl), astgen->enum_values_count);
	return &astgen->enum_values[HSC_DECL_IDX(decl)];
}

HscVariable* hsc_global_variable_get(HscAstGen* astgen, HscDecl decl) {
	HSC_DEBUG_ASSERT(HSC_DECL_IS_GLOBAL_VARIABLE(decl), "internal error: expected a global variable");
	HSC_ASSERT_ARRAY_BOUNDS(HSC_DATA_TYPE_IDX(decl), astgen->global_variables_count);
	return &astgen->global_variables[HSC_DECL_IDX(decl)];
}

U32 hsc_data_type_token_idx(HscAstGen* astgen, HscDataType data_type) {
	switch (data_type & 0xff) {
		case HSC_DATA_TYPE_TYPEDEF:
			return hsc_typedef_get(astgen, data_type)->identifier_token_idx;
		default:
			return -1;
	}
}

U32 hsc_decl_token_idx(HscAstGen* astgen, HscDecl decl) {
	switch (decl & 0xff) {
		case HSC_DECL_FUNCTION:
			return hsc_function_get(astgen, decl)->identifier_token_idx;
		case HSC_DECL_ENUM_VALUE:
			return hsc_enum_value_get(astgen, decl)->identifier_token_idx;
		default:
			if (HSC_DECL_IS_DATA_TYPE(decl)) {
				return hsc_data_type_token_idx(astgen, (HscDataType)decl);
			}
			return -1;
	}
}

void hsc_found_data_type(HscAstGen* astgen, HscDataType data_type) {
	HSC_ASSERT_ARRAY_BOUNDS(astgen->ordered_data_types_count, astgen->ordered_data_types_cap);
	astgen->ordered_data_types[astgen->ordered_data_types_count] = data_type;
	astgen->ordered_data_types_count += 1;
}

HscString hsc_data_type_string(HscAstGen* astgen, HscDataType data_type) {
	HscStringId string_id;
	bool is_const = HSC_DATA_TYPE_IS_CONST(data_type);
	data_type = HSC_DATA_TYPE_STRIP_CONST(data_type);
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
		char buf[1024];
		switch (data_type & 0xff) {
			case HSC_DATA_TYPE_TYPEDEF: {
				HscTypedef* typedef_ = hsc_typedef_get(astgen, data_type);
				return hsc_string_table_get(&astgen->string_table, typedef_->identifier_string_id);
			};
			case HSC_DATA_TYPE_ARRAY: {
				HscArrayDataType* d = hsc_array_data_type_get(astgen, data_type);
				HscString element_string = hsc_data_type_string(astgen, d->element_data_type);
				HscConstant constant = hsc_constant_table_get(&astgen->constant_table, d->size_constant_id);
				U64 size;
				HSC_DEBUG_ASSERT(hsc_constant_as_uint(constant, &size), "internal error: array size is not an unsigned integer");
				U32 string_size = snprintf(buf, sizeof(buf), "%.*s[%zu]", (int)element_string.size, element_string.data, size);
				string_id = hsc_string_table_deduplicate(&astgen->string_table, buf, string_size);
				break;
			};
			case HSC_DATA_TYPE_STRUCT:
			case HSC_DATA_TYPE_UNION:
			{
				char* compound_name = HSC_DATA_TYPE_IS_STRUCT(data_type) ? "struct" : "union";
				HscCompoundDataType* d = hsc_compound_data_type_get(astgen, data_type);
				HscString identifier = hsc_string_lit("<anonymous>");
				if (d->identifier_string_id.idx_plus_one) {
					identifier = hsc_string_table_get(&astgen->string_table, d->identifier_string_id);
				}
				U32 string_size = snprintf(buf, sizeof(buf), "%s(#%u) %.*s", compound_name, HSC_DATA_TYPE_IDX(data_type), (int)identifier.size, identifier.data);
				string_id = hsc_string_table_deduplicate(&astgen->string_table, buf, string_size);
				break;
			};
			case HSC_DATA_TYPE_ENUM:
			{
				HscEnumDataType* d = hsc_enum_data_type_get(astgen, data_type);
				HscString identifier = hsc_string_lit("<anonymous>");
				if (d->identifier_string_id.idx_plus_one) {
					identifier = hsc_string_table_get(&astgen->string_table, d->identifier_string_id);
				}
				U32 string_size = snprintf(buf, sizeof(buf), "enum(#%u) %.*s", HSC_DATA_TYPE_IDX(data_type), (int)identifier.size, identifier.data);
				string_id = hsc_string_table_deduplicate(&astgen->string_table, buf, string_size);
				break;
			};
			default:
				HSC_ABORT("unhandled data type '%u'", data_type);
		}
	}

	if (is_const) {
		char buf[1024];
		HscString data_type_string = hsc_string_table_get(&astgen->string_table, string_id);
		U32 string_size = snprintf(buf, sizeof(buf), "const %.*s", (int)data_type_string.size, data_type_string.data);
		string_id = hsc_string_table_deduplicate(&astgen->string_table, buf, string_size);
	}

	return hsc_string_table_get(&astgen->string_table, string_id);
}

void hsc_data_type_size_align(HscAstGen* astgen, HscDataType data_type, Uptr* size_out, Uptr* align_out) {
	data_type = hsc_typedef_resolve(astgen, data_type);

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
		switch (data_type & 0xff) {
			case HSC_DATA_TYPE_STRUCT:
			case HSC_DATA_TYPE_UNION: {
				HscCompoundDataType* d = hsc_compound_data_type_get(astgen, data_type);
				*size_out = d->size;
				*align_out = d->align;
				break;
			};
			case HSC_DATA_TYPE_ARRAY: {
				HscArrayDataType* d = hsc_array_data_type_get(astgen, data_type);
				HscConstant constant = hsc_constant_table_get(&astgen->constant_table, d->size_constant_id);
				U64 count;
				hsc_constant_as_uint(constant, &count);

				Uptr size;
				Uptr align;
				hsc_data_type_size_align(astgen, d->element_data_type, &size, &align);

				*size_out = size * count;
				*align_out = align;
				break;
			};
			default:
				HSC_ABORT("unhandled data type '%u'", data_type);
		}
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
		case HSC_DATA_TYPE_F16:
			fprintf(f, "F16 TODO");
			break;
		case HSC_DATA_TYPE_F32: float_ = *(F32*)data; goto FLOAT;
		case HSC_DATA_TYPE_F64: float_ = *(F64*)data; goto FLOAT;
FLOAT:
			fprintf(f, "%f", float_);
			break;
	}
}

HscDataType hsc_data_type_unsigned_to_signed(HscDataType data_type) {
	HSC_DEBUG_ASSERT(data_type < HSC_DATA_TYPE_VECTOR_END, "data_type must be a basic or vector type");
	HscDataType scalar_data_type = HSC_DATA_TYPE_SCALAR(data_type);
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_U8 <= scalar_data_type && scalar_data_type <= HSC_DATA_TYPE_U64, "scalar_data_type must be a unsigned integer");
	return data_type + 4;
}

HscDataType hsc_data_type_signed_to_unsigned(HscDataType data_type) {
	HSC_DEBUG_ASSERT(data_type < HSC_DATA_TYPE_VECTOR_END, "data_type must be a basic or vector type");
	HscDataType scalar_data_type = HSC_DATA_TYPE_SCALAR(data_type);
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_S8 <= scalar_data_type && scalar_data_type <= HSC_DATA_TYPE_S64, "scalar_data_type must be a signed integer");
	return data_type - 4;
}

void hsc_constant_print(HscAstGen* astgen, HscConstantId constant_id, FILE* f) {
	HscConstant constant = hsc_constant_table_get(&astgen->constant_table, constant_id);
	if (constant.size == 0) {
		HscString data_type_name = hsc_data_type_string(astgen, constant.data_type);
		fprintf(f, "%.*s: <ZERO>", (int)data_type_name.size, data_type_name.data);
		return;
	}

	if (constant.data_type < HSC_DATA_TYPE_BASIC_END) {
		hsc_data_type_print_basic(astgen, constant.data_type, constant.data, f);
	} else if (HSC_DATA_TYPE_VECTOR_START <= constant.data_type && constant.data_type < HSC_DATA_TYPE_MATRIX_END) {
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

void hsc_data_type_ensure_is_condition(HscAstGen* astgen, HscDataType data_type) {
	data_type = hsc_typedef_resolve(astgen, data_type);
	if (!hsc_data_type_is_condition(astgen, data_type)) {
		HscString data_type_name = hsc_data_type_string(astgen, data_type);
		hsc_astgen_error_1(astgen, "the condition expression must be convertable to a boolean but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}
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
		switch (data_type & 0xff) {
			case HSC_DATA_TYPE_ARRAY: {
				HscArrayDataType* d = hsc_array_data_type_get(astgen, data_type);
				HscConstant constant = hsc_constant_table_get(&astgen->constant_table, d->size_constant_id);
				U64 count;
				hsc_constant_as_uint(constant, &count);
				return count;
			};
			default:
				HSC_ABORT("unhandled data type '%u'", data_type);
		}
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

HscConstantId _hsc_constant_table_deduplicate_end(HscConstantTable* constant_table, HscDataType data_type, void* data, U32 data_size, U32 data_align, HscStringId debug_string_id);

HscConstantId hsc_constant_table_deduplicate_basic(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type, void* data) {
	HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a basic type but got '%s'", hsc_data_type_string(astgen, data_type));
	HSC_DEBUG_ASSERT(constant_table->fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");

	Uptr size;
	Uptr align;
	hsc_data_type_size_align(astgen, data_type, &size, &align);

	constant_table->data_write_ptr = NULL;
	HscStringId debug_string_id = {0};
	return _hsc_constant_table_deduplicate_end(constant_table, data_type, data, size, align, debug_string_id);
}

void hsc_constant_table_deduplicate_composite_start(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type) {
	HSC_DEBUG_ASSERT(!HSC_DATA_TYPE_IS_BASIC(data_type), "internal error: expected a non basic type but got '%s'", hsc_data_type_string(astgen, data_type));
	HSC_DEBUG_ASSERT(constant_table->fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");

	U32 fields_count;
	fields_count = hsc_data_type_composite_fields_count(astgen, data_type);

	constant_table->data_type = data_type;
	constant_table->fields_count = 0;
	constant_table->fields_cap = fields_count;
	constant_table->data_write_ptr = HSC_PTR_ROUND_UP_ALIGN(HSC_PTR_ADD(constant_table->data, constant_table->data_used_size), alignof(HscConstantId));
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

	HscStringId debug_string_id = {0};
	return _hsc_constant_table_deduplicate_end(constant_table, constant_table->data_type, constant_table->data_write_ptr, constant_table->fields_count * sizeof(HscConstantId), alignof(HscConstantId), debug_string_id);
}

HscConstantId hsc_constant_table_deduplicate_zero(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type) {
	if (HSC_DATA_TYPE_IS_BASIC(data_type)) {
		//
		// basic type's need to store their zero data into the consant table. this is so that
		// when the spirv code is generated it will generate OpConstant instructions for the consants instead of OpConstantNull.
		// this will allow them to be used as indices in OpAccessChain.
		U64 zero = 0;
		return hsc_constant_table_deduplicate_basic(constant_table, astgen, data_type, &zero);
	} else {
		HSC_DEBUG_ASSERT(constant_table->fields_cap == 0, "internal error: starting to deduplicate a constant before ending another");
		HscStringId debug_string_id = {0};
		return _hsc_constant_table_deduplicate_end(constant_table, data_type, NULL, 0, 0, debug_string_id);
	}
}

HscConstantId _hsc_constant_table_deduplicate_end(HscConstantTable* constant_table, HscDataType data_type, void* data, U32 data_size, U32 data_align, HscStringId debug_string_id) {
	//
	// TODO: make this a hash table look up
	for (uint32_t entry_idx = 0; entry_idx < constant_table->entries_count; entry_idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[entry_idx];
		if (entry->data_type == data_type && data_size == entry->size && memcmp(HSC_PTR_ADD(constant_table->data, entry->start_idx), data, data_size) == 0) {
			return (HscConstantId) { .idx_plus_one = entry_idx + 1 };
		}
	}

	if (constant_table->entries_count >= constant_table->entries_cap) {
		HSC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	if (constant_table->data_used_size + data_size > constant_table->data_cap) {
		HSC_ABORT("constant tables entries capacity exceeded TODO make this error message proper");
	}

	uint32_t new_entry_idx = constant_table->entries_count;
	constant_table->entries_count += 1;
	HscConstantEntry* entry = &constant_table->entries[new_entry_idx];
	entry->size = data_size;
	entry->data_type = data_type;
	entry->debug_string_id = debug_string_id;

	if (data_align) {
		constant_table->data_used_size = HSC_INT_ROUND_UP_ALIGN(constant_table->data_used_size, data_align);
		entry->start_idx = constant_table->data_used_size;
		constant_table->data_used_size += data_size;
	}

	if (constant_table->data_write_ptr != data && data_size) {
		memcpy(HSC_PTR_ADD(constant_table->data, entry->start_idx), data, data_size);
	}

	return (HscConstantId) { .idx_plus_one = new_entry_idx + 1 };
}

HscConstant hsc_constant_table_get(HscConstantTable* constant_table, HscConstantId id) {
	HSC_DEBUG_ASSERT(id.idx_plus_one, "constant id is null");

	HscConstantEntry* entry = &constant_table->entries[id.idx_plus_one - 1];

	HscConstant constant;
	constant.data_type = entry->data_type;
	constant.data = HSC_PTR_ADD(constant_table->data, entry->start_idx);
	constant.size = entry->size;
	return constant;
}

bool hsc_constant_as_uint(HscConstant constant, U64* out) {
	S64 signed_value = 0;
	switch (constant.data_type) {
		case HSC_DATA_TYPE_U8: *out = *(U8*)constant.data; break;
		case HSC_DATA_TYPE_U16: *out = *(U16*)constant.data; break;
		case HSC_DATA_TYPE_U32: *out = *(U32*)constant.data; break;
		case HSC_DATA_TYPE_U64: *out = *(U64*)constant.data; break;
		case HSC_DATA_TYPE_S8:
			signed_value = *(S8*)constant.data;
			goto SIGNED_VALUE;
		case HSC_DATA_TYPE_S16:
			signed_value = *(S16*)constant.data;
			goto SIGNED_VALUE;
		case HSC_DATA_TYPE_S32:
			signed_value = *(S32*)constant.data;
			goto SIGNED_VALUE;
		case HSC_DATA_TYPE_S64:
			signed_value = *(S64*)constant.data;
SIGNED_VALUE:
			if (signed_value < 0) {
				return false;
			}
			*out = signed_value;
			break;
		default:
			return false;
	}

	return true;
}

bool hsc_constant_as_sint(HscConstant constant, S64* out) {
	switch (constant.data_type) {
		case HSC_DATA_TYPE_U8: *out = *(U8*)constant.data; break;
		case HSC_DATA_TYPE_U16: *out = *(U16*)constant.data; break;
		case HSC_DATA_TYPE_U32: *out = *(U32*)constant.data; break;
		case HSC_DATA_TYPE_S8: *out = *(S8*)constant.data; break;
		case HSC_DATA_TYPE_S16: *out = *(S16*)constant.data; break;
		case HSC_DATA_TYPE_S32: *out = *(S32*)constant.data; break;
		case HSC_DATA_TYPE_S64: *out = *(S64*)constant.data; break;
		default:
			return false;
	}

	return true;
}

bool hsc_constant_as_float(HscConstant constant, F64* out) {
	switch (constant.data_type) {
		case HSC_DATA_TYPE_U8: *out = *(U8*)constant.data; break;
		case HSC_DATA_TYPE_U16: *out = *(U16*)constant.data; break;
		case HSC_DATA_TYPE_U32: *out = *(U32*)constant.data; break;
		case HSC_DATA_TYPE_S8: *out = *(S8*)constant.data; break;
		case HSC_DATA_TYPE_S16: *out = *(S16*)constant.data; break;
		case HSC_DATA_TYPE_S32: *out = *(S32*)constant.data; break;
		case HSC_DATA_TYPE_S64: *out = *(S64*)constant.data; break;
		case HSC_DATA_TYPE_F32: *out = *(F32*)constant.data; break;
		case HSC_DATA_TYPE_F64: *out = *(F64*)constant.data; break;
		default:
			return false;
	}

	return true;
}

void hsc_add_intrinsic_function(HscAstGen* astgen, U32 function_idx) {
	HscIntrinsicFunction* intrinsic_function = &hsc_intrinsic_functions[function_idx];

	U32 name_size = strlen(intrinsic_function->name);
	HscStringId identifier_string_id = hsc_string_table_deduplicate(&astgen->string_table, intrinsic_function->name, name_size);

	HscDecl* decl_ptr;
	bool result = hsc_hash_table_find_or_insert(&astgen->global_declarations, identifier_string_id.idx_plus_one, &decl_ptr);
	HSC_ASSERT(!result, "internal error: intrinsic function '%.*s' already declared", name_size, intrinsic_function->name);
	*decl_ptr = HSC_DECL_INIT(HSC_DECL_FUNCTION, function_idx);

	HscFunction* function = &astgen->functions[function_idx];
	HSC_ZERO_ELMT(function);
	function->identifier_string_id = identifier_string_id;
	function->params_count = intrinsic_function->params_count;
	function->params_start_idx = astgen->function_params_and_variables_count;
	function->return_data_type = intrinsic_function->return_data_type;

	HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_and_variables_count + intrinsic_function->params_count - 1, astgen->function_params_and_variables_cap);
	HSC_COPY_ELMT_MANY(&astgen->function_params_and_variables[astgen->function_params_and_variables_count], intrinsic_function->params, intrinsic_function->params_count);
	astgen->function_params_and_variables_count += intrinsic_function->params_count;
}

HscCodeSpan* hsc_code_span_get(HscAstGen* astgen, U32 span_idx) {
	HSC_ASSERT_ARRAY_BOUNDS(span_idx, astgen->code_spans_count);
	return &astgen->code_spans[span_idx];
}

char* hsc_predefined_macro_identifier_strings[HSC_PREDEFINED_MACRO_COUNT] = {
	[HSC_PREDEFINED_MACRO___FILE__] = "__FILE__",
	[HSC_PREDEFINED_MACRO___LINE__] = "__LINE__",
	[HSC_PREDEFINED_MACRO___COUNTER__] = "__COUNTER__",
	[HSC_PREDEFINED_MACRO___HSC__] = "__HSC__",
};

void hsc_astgen_init(HscAstGen* astgen, HscCompilerSetup* setup) {
	astgen->code_span_stack = HSC_ALLOC_ARRAY(U32, setup->exprs_cap);
	HSC_ASSERT(astgen->code_span_stack, "out of memory");
	astgen->code_span_stack_cap = setup->exprs_cap;

	astgen->code_files = HSC_ALLOC_ARRAY(HscCodeFile, setup->exprs_cap);
	HSC_ASSERT(astgen->code_files, "out of memory");
	astgen->code_files_cap = setup->exprs_cap;

	astgen->pp_if_spans_stack = HSC_ALLOC_ARRAY(HscPPIfSpan, setup->exprs_cap);
	HSC_ASSERT(astgen->pp_if_spans_stack, "out of memory");
	astgen->pp_if_spans_stack_cap = setup->exprs_cap;

	astgen->code_spans = HSC_ALLOC_ARRAY(HscCodeSpan, setup->exprs_cap);
	HSC_ASSERT(astgen->code_spans, "out of memory");
	astgen->code_spans_cap = setup->exprs_cap;

	astgen->function_params_and_variables = HSC_ALLOC_ARRAY(HscVariable, setup->function_params_and_variables_cap);
	HSC_ASSERT(astgen->function_params_and_variables, "out of memory");
	astgen->functions = HSC_ALLOC_ARRAY(HscFunction, setup->functions_cap);
	HSC_ASSERT(astgen->functions, "out of memory");
	astgen->exprs = HSC_ALLOC_ARRAY(HscExpr, setup->exprs_cap);
	HSC_ASSERT(astgen->exprs, "out of memory");
	astgen->expr_locations = HSC_ALLOC_ARRAY(HscLocation, setup->exprs_cap);
	HSC_ASSERT(astgen->expr_locations, "out of memory");
	astgen->function_params_and_variables_cap = setup->function_params_and_variables_cap;
	astgen->functions_cap = setup->functions_cap;
	astgen->exprs_cap = setup->exprs_cap;

	astgen->compound_data_types = HSC_ALLOC_ARRAY(HscCompoundDataType, setup->exprs_cap);
	HSC_ASSERT(astgen->compound_data_types, "out of memory");
	astgen->compound_data_types_cap = setup->exprs_cap;

	astgen->compound_fields = HSC_ALLOC_ARRAY(HscCompoundField, setup->exprs_cap);
	HSC_ASSERT(astgen->compound_fields, "out of memory");
	astgen->compound_fields_cap = setup->exprs_cap;

	astgen->typedefs = HSC_ALLOC_ARRAY(HscTypedef, setup->exprs_cap);
	HSC_ASSERT(astgen->typedefs, "out of memory");
	astgen->typedefs_cap = setup->exprs_cap;

	astgen->macros = HSC_ALLOC_ARRAY(HscMacro, setup->exprs_cap);
	HSC_ASSERT(astgen->macros, "out of memory");
	astgen->macros_cap = setup->exprs_cap;

	astgen->macro_params = HSC_ALLOC_ARRAY(HscStringId, setup->exprs_cap);
	HSC_ASSERT(astgen->macro_params, "out of memory");
	astgen->macro_params_cap = setup->exprs_cap;

	astgen->macro_args = HSC_ALLOC_ARRAY(HscMacroArg, setup->exprs_cap);
	HSC_ASSERT(astgen->macro_args, "out of memory");
	astgen->macro_args_cap = setup->exprs_cap;

	astgen->enum_data_types = HSC_ALLOC_ARRAY(HscEnumDataType, setup->exprs_cap);
	HSC_ASSERT(astgen->enum_data_types, "out of memory");
	astgen->enum_data_types_cap = setup->exprs_cap;

	astgen->enum_values = HSC_ALLOC_ARRAY(HscEnumValue, setup->exprs_cap);
	HSC_ASSERT(astgen->enum_values, "out of memory");
	astgen->enum_values_cap = setup->exprs_cap;

	astgen->ordered_data_types = HSC_ALLOC_ARRAY(HscDataType, setup->exprs_cap);
	HSC_ASSERT(astgen->ordered_data_types, "out of memory");
	astgen->ordered_data_types_cap = setup->exprs_cap;

	astgen->lines_cap = setup->lines_cap;

	astgen->curly_initializer_gen.entry_indices = HSC_ALLOC_ARRAY(U64, setup->exprs_cap);
	astgen->curly_initializer_gen.data_types = HSC_ALLOC_ARRAY(HscDataType, setup->exprs_cap);
	astgen->curly_initializer_gen.found_designators = HSC_ALLOC_ARRAY(bool, setup->exprs_cap);
	HSC_ASSERT(astgen->curly_initializer_gen.entry_indices, "out of memory");
	astgen->curly_initializer_gen.entry_indices_cap = setup->exprs_cap;

	astgen->curly_initializer_gen.nested_designators_start_entry_indices = HSC_ALLOC_ARRAY(U32, setup->exprs_cap);
	HSC_ASSERT(astgen->curly_initializer_gen.nested_designators_start_entry_indices, "out of memory");
	astgen->curly_initializer_gen.nested_designators_cap = setup->exprs_cap;

	astgen->field_indices = HSC_ALLOC_ARRAY(U32, setup->exprs_cap);
	HSC_ASSERT(astgen->field_indices, "out of memory");
	astgen->field_indices_cap = setup->exprs_cap;

	astgen->entry_indices = HSC_ALLOC_ARRAY(U64, setup->exprs_cap);
	HSC_ASSERT(astgen->entry_indices, "out of memory");
	astgen->entry_indices_cap = setup->exprs_cap;

	astgen->macro_paste_buffer = HSC_ALLOC_ARRAY(char, setup->exprs_cap);
	HSC_ASSERT(astgen->macro_paste_buffer, "out of memory");
	astgen->macro_paste_buffer_cap = setup->exprs_cap;

	astgen->string_buffer = HSC_ALLOC_ARRAY(char, setup->exprs_cap);
	HSC_ASSERT(astgen->string_buffer, "out of memory");
	astgen->string_buffer_cap = setup->exprs_cap;

	astgen->global_variables = HSC_ALLOC_ARRAY(HscVariable, setup->exprs_cap);
	HSC_ASSERT(astgen->global_variables, "out of memory");
	astgen->global_variables_cap = setup->exprs_cap;

	astgen->used_static_variables = HSC_ALLOC_ARRAY(HscDecl, setup->exprs_cap);
	HSC_ASSERT(astgen->used_static_variables, "out of memory");
	astgen->used_static_variables_cap = setup->exprs_cap;

	astgen->array_data_types = HSC_ALLOC_ARRAY(HscArrayDataType, setup->exprs_cap);
	HSC_ASSERT(astgen->array_data_types, "out of memory");
	astgen->array_data_types_cap = setup->exprs_cap;

	astgen->variable_stack_strings = HSC_ALLOC_ARRAY(HscStringId, setup->variable_stack_cap);
	HSC_ASSERT(astgen->variable_stack_strings, "out of memory");
	astgen->variable_stack_var_indices = HSC_ALLOC_ARRAY(HscStringId, setup->variable_stack_cap);
	HSC_ASSERT(astgen->variable_stack_var_indices, "out of memory");
	astgen->variable_stack_cap = setup->variable_stack_cap;

	astgen->tokens = HSC_ALLOC_ARRAY(HscToken, setup->tokens_cap);
	HSC_ASSERT(astgen->tokens, "out of memory");
	astgen->token_location_indices = HSC_ALLOC_ARRAY(U32, setup->tokens_cap);
	HSC_ASSERT(astgen->token_location_indices, "out of memory");
	astgen->token_locations = HSC_ALLOC_ARRAY(HscLocation, setup->tokens_cap);
	HSC_ASSERT(astgen->token_locations, "out of memory");
	astgen->token_locations_cap = setup->tokens_cap;
	astgen->token_values = HSC_ALLOC_ARRAY(HscLocation, setup->tokens_cap);
	HSC_ASSERT(astgen->token_values, "out of memory");
	astgen->tokens_cap = setup->tokens_cap;
	astgen->print_color = true;

	astgen->va_args_string_id = hsc_string_table_deduplicate_lit(&astgen->string_table, "__VA_ARGS__");
	astgen->defined_string_id = hsc_string_table_deduplicate_lit(&astgen->string_table, "defined");

	hsc_hash_table_init(&astgen->global_declarations);
	{
		for (U32 function_idx = 0; function_idx <= HSC_FUNCTION_IDX_VEC4; function_idx += 1) {
			hsc_add_intrinsic_function(astgen, function_idx);
		}
		astgen->functions_count = HSC_FUNCTION_IDX_USER_START;
	}
	hsc_hash_table_init(&astgen->path_to_code_file_id_map);
	hsc_hash_table_init(&astgen->struct_declarations);
	hsc_hash_table_init(&astgen->union_declarations);
	hsc_hash_table_init(&astgen->enum_declarations);
	hsc_hash_table_init(&astgen->field_name_to_token_idx);

	astgen->predefined_macro_identifier_string_start_id.idx_plus_one = astgen->string_table.entries_count + 1;
	for (U32 predefined_macro = 0; predefined_macro < HSC_PREDEFINED_MACRO_COUNT; predefined_macro += 1) {
		char* identifier_string = hsc_predefined_macro_identifier_strings[predefined_macro];
		HscStringId string_id = hsc_string_table_deduplicate_c_string(&astgen->string_table, identifier_string);
		HSC_DEBUG_ASSERT(
			astgen->predefined_macro_identifier_string_start_id.idx_plus_one + predefined_macro == string_id.idx_plus_one,
			"internal error: predefined macro string ids to not map to the enum"
		);
	}

	hsc_hash_table_init(&astgen->macro_declarations);
	for (U32 predefined_macro = 0; predefined_macro < HSC_PREDEFINED_MACRO_COUNT; predefined_macro += 1) {
		U32* macro_idx_ptr;
		HscStringId identifier_string_id = { astgen->predefined_macro_identifier_string_start_id.idx_plus_one + predefined_macro };
		hsc_hash_table_find_or_insert(&astgen->macro_declarations, identifier_string_id.idx_plus_one, &macro_idx_ptr);
		*macro_idx_ptr = U32_MAX;
	}
}

void hsc_astgen_error_file_line(HscAstGen* astgen, char* file_path, U32 line, U32 column) {
	const char* error_fmt = astgen->print_color
		? "\x1b[1;95mfile\x1b[97m: %s:%u:%u\n\x1b[0m"
		: "file: %s:%u:%u\n";
	printf(error_fmt, file_path, line, column);
}

void hsc_astgen_print_code_line(HscAstGen* astgen, HscCodeSpan* span, U32 display_line_num_size, U32 line) {
	U32 code_start_idx = span->line_code_start_indices[line];
	U32 code_end_idx;
	if (line >= span->lines_count) {
		code_end_idx = span->code_size;
	} else {
		code_end_idx = span->line_code_start_indices[line + 1];
	}

	while (code_end_idx) {
		code_end_idx -= 1;
		U8 byte = span->code[code_end_idx];
		if (byte != '\r' && byte != '\n') {
			code_end_idx += 1;
			break;
		}
	}

	if (code_end_idx <= code_start_idx) {
		const char* fmt = astgen->print_color
			? "\x1b[1;94m%*u|\x1b[0m\n"
			: "%*u|\n";
		printf(fmt, display_line_num_size, line);
	} else {
		U32 code_size = code_end_idx - code_start_idx;
		char* code = (char*)&span->code[code_start_idx];

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
	HscCodeSpan* span = hsc_code_span_get(astgen, location->span_idx);
	if (location->parent_location_idx != -1) {
		if (span->macro || span->macro_arg_id) {
			HscLocation* parent_location = &astgen->token_locations[location->parent_location_idx];
			hsc_astgen_print_code(astgen, parent_location);

			if (span->macro) {
				const char* error_fmt = astgen->print_color
					? "\x1b[1;97\nmexpanded from macro\x1b[0m: \n"
					: "\nexpanded from macro: ";
				printf("%s", error_fmt);

				HscLocation* macro_location = &span->macro->location;
				char* file_path = "test.hsc";
				hsc_astgen_error_file_line(astgen, file_path, macro_location->line_start, macro_location->column_start);
			} else {
				return;
			}
		}
	} else {
		char* file_path = "test.hsc";
		hsc_astgen_error_file_line(astgen, file_path, location->line_start, location->column_start);
	}

	HscCodeSpan* file_span = span;
	while (file_span->macro || file_span->macro_arg_id || file_span->is_preprocessor_expression) {
		HscLocation* parent_location = &astgen->token_locations[file_span->location.parent_location_idx];
		HscCodeSpan* parent_span = hsc_code_span_get(astgen, parent_location->span_idx);
		if (parent_span->code_file) {
			HscLocation* applied_location = &span->macro->location;
			if (file_span->is_preprocessor_expression) {
				applied_location = &parent_span->location;
				location->column_start += applied_location->column_start;
				location->column_end += applied_location->column_start;
			} else {
				applied_location = &span->macro->location;
				location->column_start += span->macro->value_string_column_start;
				location->column_end += span->macro->value_string_column_start;
			}
			location->code_start_idx += applied_location->code_start_idx;
			location->code_end_idx += applied_location->code_end_idx;
			location->line_start += applied_location->line_start;
			location->line_end += applied_location->line_end;
		}

		file_span = parent_span;
	}

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
		hsc_astgen_print_code_line(astgen, file_span, display_line_num_size, line - 2);
	}
	if (line > 1) {
		hsc_astgen_print_code_line(astgen, file_span, display_line_num_size, line - 1);
	}

	hsc_astgen_print_code_line(astgen, file_span, display_line_num_size, line);

	for (U32 i = 0; i < display_line_num_size + 1; i += 1) {
		putchar(' ');
	}

	for (U32 i = 0; i < location->column_start; i += 1) {
		if (location->code_start_idx > location->column_start && file_span->code[location->code_start_idx - location->column_start + i] == '\t') {
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

	if (line + 1 <= file_span->lines_count) {
		hsc_astgen_print_code_line(astgen, file_span, display_line_num_size, line + 1);
	}
	if (line + 2 <= file_span->lines_count) {
		hsc_astgen_print_code_line(astgen, file_span, display_line_num_size, line + 2);
	}
}

void hsc_astgen_add_line_start_idx(HscAstGen* astgen, HscCodeSpan* span) {
	if (span->lines_count >= span->lines_cap) {
		hsc_astgen_error_1(astgen, "internal error: the lines capacity of '%u' has been exceeded", astgen->lines_cap);
	}

	span->line_code_start_indices[span->lines_count] = span->location.code_end_idx;
	span->lines_count += 1;
}

void hsc_astgen_found_newline(HscAstGen* astgen, HscCodeSpan* span) {
	span->location.line_start += 1;
	span->location.line_end += 1;
	span->location.column_start = 1;
	span->location.column_end = 1;

	hsc_astgen_add_line_start_idx(astgen, span);
}

void hsc_astgen_token_count_extra_newlines(HscAstGen* astgen) {
	U32 lines_count = 3;
	for (U32 span_stack_idx = astgen->code_span_stack_count; span_stack_idx-- > 0; span_stack_idx += 1) {
		HscCodeSpan* span = &astgen->code_spans[astgen->code_span_stack[span_stack_idx]];
		while (span->location.code_end_idx < span->code_size) {
			U8 byte = span->code[span->location.code_end_idx];
			span->location.code_end_idx += 1;
			if (byte == '\n') {
				hsc_astgen_add_line_start_idx(astgen, span);
				lines_count -= 1;
				if (lines_count == 0) {
					break;
				}
			}
		}
		span_stack_idx -= 1;
	}
}

HSC_NORETURN void hsc_astgen_error(HscAstGen* astgen, U32 token_idx, U32 other_token_idx, const char* fmt, va_list va_args) {
	const char* error_fmt = astgen->print_color
		? "\x1b[1;91merror\x1b[0m: "
		: "error: ";
	printf("%s", error_fmt);

	if (astgen->print_color) {
		printf("\x1b[1;97m");
	}

	vprintf(fmt, va_args);

	printf("\n");

	U32 location_idx = astgen->token_location_indices[token_idx];
	HscLocation* location = &astgen->token_locations[location_idx];

	hsc_astgen_print_code(astgen, location);

	if (other_token_idx != -1) {
		U32 other_location_idx = astgen->token_location_indices[other_token_idx];
		HscLocation* other_location = &astgen->token_locations[other_location_idx];

		const char* error_fmt = astgen->print_color
			? "\x1b[1;97\nmoriginally defined here\x1b[0m: \n"
			: "\noriginally defined here: ";
		printf("%s", error_fmt);

		char* file_path = "test.hsc";
		hsc_astgen_error_file_line(astgen, file_path, other_location->line_start, other_location->column_start);

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
	hsc_astgen_add_token(astgen, 0);
	hsc_astgen_token_count_extra_newlines(astgen);
	hsc_astgen_error(astgen, astgen->tokens_count - 1, -1, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_token_error_2(HscAstGen* astgen, U32 other_token_idx, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	hsc_astgen_add_token(astgen, 0);
	hsc_astgen_token_count_extra_newlines(astgen);
	hsc_astgen_error(astgen, astgen->tokens_count - 1, other_token_idx, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_error_1(HscAstGen* astgen, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	U32 token_idx = HSC_MIN(astgen->token_read_idx, astgen->tokens_count - 1);
	hsc_astgen_error(astgen, token_idx, -1, fmt, va_args);
	va_end(va_args);
}

void hsc_astgen_error_2(HscAstGen* astgen, U32 other_token_idx, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	U32 token_idx = HSC_MIN(astgen->token_read_idx, astgen->tokens_count - 1);
	hsc_astgen_error(astgen, token_idx, other_token_idx, fmt, va_args);
	va_end(va_args);
}

void _hsc_token_location_add(HscAstGen* astgen, HscLocation* location) {
	HSC_ASSERT_ARRAY_BOUNDS(astgen->token_locations_count, astgen->token_locations_cap);
	astgen->token_locations[astgen->token_locations_count] = *location;
	astgen->token_locations_count += 1;
}

HscCodeSpan* _hsc_code_span_push(HscAstGen* astgen) {
	U32 span_stack_idx = astgen->code_span_stack_count;
	HSC_ASSERT_ARRAY_BOUNDS(span_stack_idx, astgen->code_span_stack_cap);
	astgen->code_span_stack[span_stack_idx] = astgen->code_spans_count;
	astgen->code_span_stack_count += 1;

	U32 span_idx = astgen->code_spans_count;
	HSC_ASSERT_ARRAY_BOUNDS(span_idx, astgen->code_spans_cap);
	HscCodeSpan* span = &astgen->code_spans[span_idx];
	HSC_ZERO_ELMT(span);

	astgen->code_spans_count += 1;
	astgen->span = span;

	span->location.span_idx = span_idx;
	if (span_stack_idx) {
		HscCodeSpan* prev_span = &astgen->code_spans[astgen->code_span_stack[span_stack_idx - 1]];
		span->location.parent_location_idx = astgen->token_locations_count;
		_hsc_token_location_add(astgen, &prev_span->location);
	} else {
		span->location.parent_location_idx = -1;
	}

	return span;
}

void hsc_code_span_push_file(HscAstGen* astgen, HscCodeFileId code_file_id) {
	HscCodeSpan* span = _hsc_code_span_push(astgen);
	HscCodeFile* code_file = hsc_code_file_get(astgen, code_file_id);

	span->code_file = code_file;
	span->code = code_file->code;
	span->code_size = code_file->code_size;

	//
	// TODO make this a linear allocator that we can just claim back the unused lines
	span->line_code_start_indices = HSC_ALLOC_ARRAY(U32, astgen->lines_cap);
	HSC_ASSERT(span->line_code_start_indices, "out of memory");
	span->lines_cap = astgen->lines_cap;

	span->location.line_start = 1;
	span->location.line_end = 1;
	span->location.column_start = 1;
	span->location.column_end = 1;

	span->line_code_start_indices[0] = 0;
	span->line_code_start_indices[1] = 0;
	span->lines_count += 2;
}

void hsc_code_span_push_macro_arg(HscAstGen* astgen, HscCodeSpan* macro_span, U32 macro_arg_idx, U8* code, U32 code_size) {
#if HSC_DEBUG_CODE_SPAN_PUSH_POP
	U32 param_idx = macro_arg_idx - macro_span->macro_args_start_idx;
	HscStringId param_string_id = astgen->macro_params[macro_span->macro->params_start_idx + param_idx];
	if (param_string_id.idx_plus_one == 0) {
		param_string_id = astgen->va_args_string_id;
	}
	HscString param_name = hsc_string_table_get(&astgen->string_table, param_string_id);
	printf("PUSH_ARG(%.*s = %.*s)\n", (int)param_name.size, param_name.data, code_size, code);
#endif // HSC_DEBUG_CODE_SPAN_PUSH_POP

	HscCodeSpan* span = _hsc_code_span_push(astgen);
	span->code = code;
	span->code_size = code_size;
	span->macro_arg_id = macro_arg_idx + 1;
}

HscCodeSpan* hsc_code_span_find_evaluating_macro(HscAstGen* astgen, HscCodeSpan** parent_span_out) {
	U32 found_args = 0;
	U32 found_macros = 0;
	HscCodeSpan* span = NULL;
	for (U32 idx = astgen->code_span_stack_count; idx-- > 0;) {
		U32 span_idx = astgen->code_span_stack[idx];
		HscCodeSpan* parent_span = &astgen->code_spans[span_idx];
		if (parent_span->macro) {
			if (found_macros == found_args && idx + 1 < astgen->code_span_stack_count) {
				*parent_span_out = parent_span;
				return span;
			}
			found_macros = HSC_MIN(found_macros + 1, found_args);
		} else if (parent_span->macro_arg_id) {
			found_args += 1;
		}
		span = parent_span;
	}
	return NULL;
}

bool hsc_code_span_is_macro_on_stack(HscAstGen* astgen, HscMacro* macro) {
	U32 found_args = 0;
	U32 found_macros = 0;
	for (U32 idx = astgen->code_span_stack_count; idx-- > 0;) {
		U32 span_idx = astgen->code_span_stack[idx];
		HscCodeSpan* span = &astgen->code_spans[span_idx];
		if (span->macro) {
			if (found_macros == found_args && idx + 1 < astgen->code_span_stack_count) {
				if (span->macro->identifier_string_id.idx_plus_one == macro->identifier_string_id.idx_plus_one) {
					return true;
				}
				found_macros = 0;
				found_args = 0;
			} else {
				found_macros = HSC_MIN(found_macros + 1, found_args);
			}
		} else if (span->macro_arg_id) {
			found_args += 1;
		}
	}

	return false;
}

void hsc_code_span_push_predefined_macro(HscAstGen* astgen, HscPredefinedMacro predefined_macro) {
	HscString code;
	switch (predefined_macro) {
		case HSC_PREDEFINED_MACRO___FILE__: {
			HscString path_string = astgen->span->code_file->path_string;
			code = hsc_string(&astgen->string_buffer[astgen->string_buffer_size], astgen->string_buffer_size);
			hsc_string_buffer_append_fmt(astgen, "\"%.*s\"", (int)path_string.size, path_string.data);
			code.size = astgen->string_buffer_size - code.size;
			break;
		};
		case HSC_PREDEFINED_MACRO___LINE__:
			code = hsc_string(&astgen->string_buffer[astgen->string_buffer_size], astgen->string_buffer_size);
			hsc_string_buffer_append_fmt(astgen, "%u", astgen->span->location.line_end);
			code.size = astgen->string_buffer_size - code.size;
			break;
		case HSC_PREDEFINED_MACRO___COUNTER__:
			code = hsc_string(&astgen->string_buffer[astgen->string_buffer_size], astgen->string_buffer_size);
			hsc_string_buffer_append_fmt(astgen, "%u", astgen->__counter__);
			code.size = astgen->string_buffer_size - code.size;
			astgen->__counter__ += 1;
			break;
		case HSC_PREDEFINED_MACRO___HSC__:
			return;
	}

	HscCodeSpan* span = _hsc_code_span_push(astgen);
#if HSC_DEBUG_CODE_SPAN_PUSH_POP
	HscString name = hsc_string_table_get(&astgen->string_table, macro->identifier_string_id);
	printf("PUSH_PREDEFINED_MACRO(%s)\n", hsc_predefined_macro_identifier_strings[predefined_macro]);
#endif // HSC_DEBUG_CODE_SPAN_PUSH_POP

	span->code = (U8*)code.data;
	span->code_size = code.size;
}

void hsc_code_span_push_macro(HscAstGen* astgen, HscMacro* macro) {
	HscCodeSpan* span = _hsc_code_span_push(astgen);
#if HSC_DEBUG_CODE_SPAN_PUSH_POP
	HscString name = hsc_string_table_get(&astgen->string_table, macro->identifier_string_id);
	printf("PUSH_MACRO(%.*s)\n", (int)name.size, name.data);
#endif // HSC_DEBUG_CODE_SPAN_PUSH_POP

	span->code = (U8*)macro->value_string.data;
	span->code_size = macro->value_string.size;
	span->macro = macro;
	span->macro_args_start_idx = astgen->macro_args_count - macro->params_count;
}

void hsc_code_span_push_preprocessor_expression(HscAstGen* astgen, HscString expression) {
	HscCodeSpan* span = _hsc_code_span_push(astgen);
#if HSC_DEBUG_CODE_SPAN_PUSH_POP
	HscString name = hsc_string_table_get(&astgen->string_table, macro->identifier_string_id);
	printf("PUSH_PREPROCESSOR_EXPR(%.*s)\n", (int)expression.size, expression.data);
#endif // HSC_DEBUG_CODE_SPAN_PUSH_POP

	span->code = (U8*)expression.data;
	span->code_size = expression.size;
	span->is_preprocessor_expression = true;
}

bool hsc_code_span_pop(HscAstGen* astgen) {
	HSC_DEBUG_ASSERT(astgen->code_span_stack_count, "internal error: we have popped all of the code spans!");
	astgen->code_span_stack_count -= 1;
	if (astgen->code_span_stack_count) {
		HscCodeSpan* popped_span = &astgen->code_spans[astgen->code_span_stack[astgen->code_span_stack_count]];
		if (popped_span->macro) {
			astgen->macro_args_count -= popped_span->macro->params_count;
		}
		astgen->span = &astgen->code_spans[astgen->code_span_stack[astgen->code_span_stack_count - 1]];
#if HSC_DEBUG_CODE_SPAN_PUSH_POP
		if (popped_span->macro_arg_id) {
			printf("POP_ARG(%.*s)\n", popped_span->code_size, popped_span->code);
		} else if (popped_span->macro) {
			HscString name = hsc_string_table_get(&astgen->string_table, popped_span->macro->identifier_string_id);
			printf("POP_MACRO(%.*s)\n", (int)name.size, name.data);
		} else if (popped_span->is_preprocessor_expression) {
			HscString name = hsc_string_table_get(&astgen->string_table, popped_span->macro->identifier_string_id);
			printf("POP_PREPROCESSOR_EXPR(%.*s)\n", (int)popped_span->code_size, popped_span->code);
		}
#endif // HSC_DEBUG_CODE_SPAN_PUSH_POP
		return popped_span->is_preprocessor_expression;
	} else {
		hsc_astgen_add_token(astgen, HSC_TOKEN_EOF);
		astgen->span = NULL;
		return true;
	}
}

void hsc_astgen_add_token(HscAstGen* astgen, HscToken token) {
	HscCodeSpan* span = astgen->span;

	HSC_ASSERT_ARRAY_BOUNDS(astgen->tokens_count, astgen->tokens_cap);
	astgen->tokens[astgen->tokens_count] = token;
	astgen->token_location_indices[astgen->tokens_count] = astgen->token_locations_count;
	astgen->tokens_count += 1;

	_hsc_token_location_add(astgen, &span->location);
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

static inline bool hsc_ascii_is_alpha(U32 byte) {
	return ((byte | 32u) - 97u) < 26u;
}

static inline bool hsc_ascii_is_digit(U32 byte) {
	return (byte - 16) <= 10;
}

static inline bool hsc_ascii_is_a_to_f(U32 byte) {
	return ((byte | 32u) - 97u) < 6u;
}

void hsc_string_buffer_clear(HscAstGen* astgen) {
	astgen->string_buffer_size = 0;
}

void hsc_string_buffer_append_byte(HscAstGen* astgen, char byte) {
	HSC_ASSERT_ARRAY_BOUNDS(astgen->string_buffer_size, astgen->string_buffer_cap);
	astgen->string_buffer[astgen->string_buffer_size] = byte;
	astgen->string_buffer_size += 1;
}

void hsc_string_buffer_append_string(HscAstGen* astgen, char* string, U32 string_size) {
	char* dst = &astgen->string_buffer[astgen->string_buffer_size];
	astgen->string_buffer_size += string_size;
	HSC_ASSERT_ARRAY_BOUNDS(astgen->string_buffer_size - 1, astgen->string_buffer_cap);
	memcpy(dst, string, string_size);
}

void hsc_string_buffer_append_fmt(HscAstGen* astgen, char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	va_list args_copy;
	va_copy(args_copy, args);

	//
	// add 1 so we have enough room for the null terminator that vsnprintf always outputs
	// vsnprintf will return -1 on an encoding error.
	Uptr count = vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);
	HSC_DEBUG_ASSERT(count >= 1, "a vsnprintf encoding error has occurred");

	//
	// resize the stack to have enough room to store the pushed formatted string with the null terminator
	Uptr insert_idx = astgen->string_buffer_size;
	Uptr new_count = astgen->string_buffer_size + count;
	HSC_ASSERT_ARRAY_BOUNDS(new_count - 1, astgen->string_buffer_cap);

	//
	// now call vsnprintf for real this time, with a buffer
	// to actually copy the formatted string.
	char* ptr = &astgen->string_buffer[astgen->string_buffer_size];
	count = vsnprintf(ptr, count, fmt, args);
	astgen->string_buffer_size = new_count - 1; // now set the new count minus the null terminator

	va_end(args);
}

uint32_t hsc_parse_num(HscAstGen* astgen, HscToken* token_out) {
	char* num_string = (char*)&astgen->span->code[astgen->span->location.code_end_idx];
	uint32_t remaining_size = astgen->span->code_size - astgen->span->location.code_end_idx;
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

void hsc_parse_string(HscAstGen* astgen, char terminator_byte) {
	HscCodeSpan* span = astgen->span;
	span->location.code_end_idx += 1;
	span->location.column_end += 1;

	hsc_string_buffer_clear(astgen);
	if (astgen->is_preprocessor_include) {
		bool ended_with_terminator = false;
		while (span->location.code_end_idx < span->code_size) {
			char byte = span->code[span->location.code_end_idx];
			span->location.column_end += 1;
			span->location.code_end_idx += 1;

			if (byte == '\\') {
				if (span->code[span->location.code_end_idx] == '\r') {
					span->location.column_end += 2;
					span->location.code_end_idx += 2;
				} else if (span->code[span->location.code_end_idx] == '\n') {
					span->location.column_end += 1;
					span->location.code_end_idx += 1;
				} else {
					hsc_string_buffer_append_byte(astgen, '/');
				}
			} else if (byte == terminator_byte) {
				ended_with_terminator = true;
				break;
			} else {
				hsc_string_buffer_append_byte(astgen, byte);
			}
		}

		if (!ended_with_terminator) {
			span->location.column_start = span->location.column_end;
			span->location.column_end += 1;
			span->location.code_end_idx -= 1;
			hsc_astgen_token_error_1(astgen, "unclosed string literal. close with '%c' or strings spanning to the next line must end the line with '\\'", terminator_byte);
		}

		hsc_string_buffer_append_byte(astgen, '\0');
	} else {
		HSC_ABORT("TODO add string support");
	}

	HscTokenValue token_value;
	token_value.string_id = hsc_string_table_deduplicate(&astgen->string_table, astgen->string_buffer, astgen->string_buffer_size);

	hsc_astgen_add_token(astgen, terminator_byte == '>' ? HSC_TOKEN_INCLUDE_PATH_SYSTEM : HSC_TOKEN_STRING);
	hsc_astgen_add_token_value(astgen, token_value);
}

HscString hsc_astgen_parse_ident_from_code(HscAstGen* astgen, HscString code, char* error_fmt) {
	U8 byte = code.data[0];
	if (
		(byte < 'a' || 'z' < byte) &&
		(byte < 'A' || 'Z' < byte) &&
		byte != '_'
	) {
		hsc_astgen_token_error_1(astgen, error_fmt, byte);
	}

	HscString ident_string = hsc_string(code.data, 0);
	while (ident_string.size < code.size) {
		U8 ident_byte = ident_string.data[ident_string.size];
		if (
			(ident_byte < 'a' || 'z' < ident_byte) &&
			(ident_byte < 'A' || 'Z' < ident_byte) &&
			(ident_byte < '0' || '9' < ident_byte) &&
			(ident_byte != '_')
		) {
			break;
		}
		ident_string.size += 1;
	}

	return ident_string;
}

HscString hsc_astgen_parse_ident(HscAstGen* astgen, char* error_fmt) {
	HscString code = hsc_string((char*)&astgen->span->code[astgen->span->location.code_end_idx], astgen->span->code_size - astgen->span->location.code_end_idx);
	return hsc_astgen_parse_ident_from_code(astgen, code, error_fmt);
}

void hsc_astgen_token_consume_whitespace(HscAstGen* astgen) {
	while (astgen->span->location.code_end_idx < astgen->span->code_size) {
		U8 byte = astgen->span->code[astgen->span->location.code_end_idx];
		if (byte != ' ' && byte != '\t') {
			break;
		}

		astgen->span->location.code_end_idx += 1;
		astgen->span->location.column_end += 1;
	}
}

void hsc_astgen_token_consume_backslash(HscAstGen* astgen) {
	astgen->span->location.code_end_idx += 1;
	astgen->span->location.column_end += 1;
	hsc_astgen_token_consume_whitespace(astgen);
	U8 byte = astgen->span->code[astgen->span->location.code_end_idx];
	if (byte == '\r') {
		astgen->span->location.code_end_idx += 1;
		astgen->span->location.column_end += 1;
	}
	if (byte == '\n'){
		astgen->span->location.code_end_idx += 1;
		astgen->span->location.column_end += 1;
	}
	astgen->span->location.line_end += 1;
	astgen->span->location.code_end_idx += 1;
	astgen->span->location.column_end += 1;
	hsc_astgen_add_line_start_idx(astgen, astgen->span);
}

void hsc_astgen_token_consume_until_any_byte(HscAstGen* astgen, char* terminator_bytes) {
	while (astgen->span->location.code_end_idx < astgen->span->code_size) {
		U8 byte = astgen->span->code[astgen->span->location.code_end_idx];
		if (byte == '\\') {
			hsc_astgen_token_consume_backslash(astgen);
			byte = astgen->span->code[astgen->span->location.code_end_idx];
		}

		if (strchr(terminator_bytes, byte)) {
			break;
		}

		astgen->span->location.code_end_idx += 1;
		astgen->span->location.column_end += 1;
	}
}

void hsc_astgen_parse_preprocessor_define(HscAstGen* astgen) {
	hsc_astgen_token_consume_whitespace(astgen);
	astgen->span->location.code_start_idx = astgen->span->location.code_end_idx;
	astgen->span->location.column_start = astgen->span->location.column_end;

	HscString ident_string = hsc_astgen_parse_ident(astgen, "invalid token '%c' for macro identifier");
	astgen->span->location.code_end_idx += ident_string.size;
	astgen->span->location.column_end += ident_string.size;
	U32 column_end = astgen->span->location.column_end;
	U32 code_end_idx = astgen->span->location.code_end_idx;
	HscStringId identifier_string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)ident_string.data, ident_string.size);

	U32* macro_idx_ptr;
	if (hsc_hash_table_find_or_insert(&astgen->macro_declarations, identifier_string_id.idx_plus_one, &macro_idx_ptr)) {
		HscMacro* macro = hsc_macro_get(astgen, *macro_idx_ptr);
		U32 other_macro_token_idx = astgen->tokens_count;
		hsc_astgen_add_token(astgen, 0);
		astgen->token_locations[astgen->token_locations_count - 1] = macro->location;
		hsc_astgen_token_error_2(astgen, other_macro_token_idx, "'%.*s' has already been defined", (int)ident_string.size, ident_string.data);
	}
	*macro_idx_ptr = astgen->macros_count;

	U32 params_start_idx = astgen->macro_params_count;
	U32 params_count = 0;
	bool is_function = false;
	if (astgen->span->code[astgen->span->location.code_end_idx] == '(') {
		bool was_last_param_vararg = false;
		is_function = true;

		astgen->span->location.column_end += 1;
		astgen->span->location.code_end_idx += 1;
		hsc_astgen_token_consume_whitespace(astgen);

		if (astgen->span->code[astgen->span->location.code_end_idx] == ')') {
			astgen->span->location.column_end += 1;
			astgen->span->location.code_end_idx += 1;
		} else {
			while (1) {
				if (was_last_param_vararg) {
					hsc_astgen_token_error_1(astgen, "cannot declare another parameter, the vararg '...' parameter must come last");
				}

				HscStringId param_string_id;
				if (
					astgen->span->code[astgen->span->location.code_end_idx] == '.' &&
					astgen->span->code[astgen->span->location.code_end_idx + 1] == '.' &&
					astgen->span->code[astgen->span->location.code_end_idx + 2] == '.'
				) {
					astgen->span->location.column_end += 3;
					astgen->span->location.code_end_idx += 3;

					was_last_param_vararg = true;
					param_string_id.idx_plus_one = 0;
				} else {
					HscString ident_string = hsc_astgen_parse_ident(astgen, "invalid token '%c' for macro argument identifier");
					param_string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)ident_string.data, ident_string.size);

					astgen->span->location.column_end += ident_string.size;
					astgen->span->location.code_end_idx += ident_string.size;

					for (U32 idx = params_start_idx; idx < astgen->macro_params_count; idx += 1) {
						if (astgen->macro_params[astgen->macro_params_count].idx_plus_one == param_string_id.idx_plus_one) {
							hsc_astgen_token_error_1(astgen, "duplicate macro argument identifier '%.*s'", (int)ident_string.size, ident_string.data);
						}
					}
				}

				HSC_ASSERT_ARRAY_BOUNDS(astgen->macro_params_count, astgen->macro_params_cap);
				astgen->macro_params[astgen->macro_params_count] = param_string_id;
				astgen->macro_params_count += 1;
				params_count += 1;

				hsc_astgen_token_consume_whitespace(astgen);

				U8 byte = astgen->span->code[astgen->span->location.code_end_idx];
				if (byte == ')') {
					astgen->span->location.column_end += 1;
					astgen->span->location.code_end_idx += 1;
					break;
				}

				if (byte != ',') {
					hsc_astgen_token_error_1(astgen, "expected a ',' to declaring more macro arguments or a ')' to finish declaring macro arguments");
				}

				astgen->span->location.column_end += 1;
				astgen->span->location.code_end_idx += 1;

				hsc_astgen_token_consume_whitespace(astgen);
			}
		}
	}

	hsc_astgen_token_consume_whitespace(astgen);

	U32 value_string_column_start = astgen->span->location.column_end;
	HscString value_string = hsc_string((char*)&astgen->span->code[astgen->span->location.code_end_idx], 0);
	hsc_astgen_token_consume_until_any_byte(astgen, "\n");
	value_string.size = (char*)&astgen->span->code[astgen->span->location.code_end_idx] - value_string.data;

	HSC_ASSERT_ARRAY_BOUNDS(astgen->macros_count, astgen->macros_cap);
	HscMacro* macro = &astgen->macros[astgen->macros_count];
	macro->identifier_string_id = identifier_string_id;
	macro->value_string = value_string;
	macro->location = astgen->span->location;
	macro->location.column_end = column_end;
	macro->location.code_end_idx = code_end_idx;
	macro->value_string_column_start = value_string_column_start;
	macro->params_start_idx = params_start_idx;
	macro->params_count = params_count;
	macro->is_function = is_function;
	astgen->macros_count += 1;
}

void hsc_astgen_parse_preprocessor_undef(HscAstGen* astgen) {
	hsc_astgen_token_consume_whitespace(astgen);
	astgen->span->location.code_start_idx = astgen->span->location.code_end_idx;
	astgen->span->location.column_start = astgen->span->location.column_end;

	HscString ident_string = hsc_astgen_parse_ident(astgen, "invalid token '%c' for macro identifier");
	astgen->span->location.code_end_idx += ident_string.size;
	astgen->span->location.column_end += ident_string.size;
	hsc_astgen_token_consume_whitespace(astgen);
	HscStringId identifier_string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)ident_string.data, ident_string.size);

	hsc_hash_table_remove(&astgen->macro_declarations, identifier_string_id.idx_plus_one, NULL);
}

void hsc_astgen_parse_preprocessor_include(HscAstGen* astgen) {
	hsc_astgen_token_consume_whitespace(astgen);
	HscCodeSpan* span = astgen->span;

	span->location.column_start = span->location.column_end;

	HscString path = hsc_string((char*)&astgen->span->code[astgen->span->location.code_end_idx], 0);
	hsc_astgen_token_consume_until_any_byte(astgen, "\n");
	path.size = (char*)&astgen->span->code[astgen->span->location.code_end_idx] - path.data;

	U32 token_start_idx = astgen->tokens_count;
	U32 token_value_start_idx = astgen->token_values_count;
	U32 token_location_start_idx = astgen->token_locations_count;
	hsc_code_span_push_preprocessor_expression(astgen, path);

	astgen->is_preprocessor_include = true;
	void hsc_astgen_tokenize_run(HscAstGen* astgen);
	hsc_astgen_tokenize_run(astgen);
	astgen->is_preprocessor_include = false;

	if (token_start_idx == astgen->tokens_count) {
ERROR:
		hsc_astgen_token_error_1(astgen, "expected a '<' or '\"' to define the path to the file you wish to include");
	}

	if (token_start_idx + 1 != astgen->tokens_count) {
		hsc_astgen_token_error_1(astgen, "too many arguments for the '#include' directive");
	}

	HscToken token = astgen->tokens[token_start_idx];
	HscStringId path_string_id = astgen->token_values[token_value_start_idx].string_id;
	HscString path_string = hsc_string_table_get(&astgen->string_table, path_string_id);
	if (path_string.size <= 1) { // <= as it has a null terminator
		hsc_astgen_token_error_1(astgen, "no file path was provided for this include directive");
	}

	bool search_the_include_paths = false;
	switch (token) {
		case HSC_TOKEN_STRING:
			if (hsc_file_exist(path_string.data)) {
				break;
			}

			// fallthrough
		case HSC_TOKEN_INCLUDE_PATH_SYSTEM:
			search_the_include_paths = path_string.data[0] != '/';
			break;
		default: goto ERROR;
	}

	if (search_the_include_paths) {
		for (U32 idx = 0; idx < astgen->include_paths_count; idx += 1) {
			hsc_string_buffer_clear(astgen);

			char* include_dir_path = astgen->include_paths[idx];
			U32 include_dir_path_size = strlen(include_dir_path);
			HSC_DEBUG_ASSERT(include_dir_path_size > 0, "internal error: include directory path cannot be zero sized");

			hsc_string_buffer_append_string(astgen, include_dir_path, include_dir_path_size);
			if (include_dir_path[include_dir_path_size - 1] != '/') {
				hsc_string_buffer_append_byte(astgen, '/');
			}
			hsc_string_buffer_append_string(astgen, path_string.data, path_string.size);

			if (hsc_file_exist(astgen->string_buffer)) {
				path_string_id = hsc_string_table_deduplicate(&astgen->string_table, astgen->string_buffer, astgen->string_buffer_size);
				path_string = hsc_string_table_get(&astgen->string_table, path_string_id);
				break;
			}
		}
	}
	HscCodeFileId code_file_id;
	HscCodeFile* code_file;
	bool found_file = hsc_code_file_find_or_insert(astgen, path_string_id, &code_file_id, &code_file);
	if (!found_file) {
		U64 code_size;
		U8* code = hsc_file_read_all_the_codes(path_string.data, &code_size);
		if (code == NULL) {
			char buf[512];
			hsc_get_last_system_error_string(buf, sizeof(buf));
			hsc_astgen_token_error_1(astgen, "failed to read file '%s': %s", path_string.data, buf);
		}

		code_file->code = code;
		code_file->code_size = code_size;
	}

	if (!(code_file->flags & HSC_CODE_FILE_FLAGS_INCLUDE_GUARD)) {
		astgen->include_code_file_id = code_file_id;
	}

	astgen->tokens_count = token_start_idx;
	astgen->token_values_count = token_value_start_idx;
	astgen->token_locations_count = token_location_start_idx;
}

void hsc_astgen_parse_preprocessor_error(HscAstGen* astgen) {
	hsc_astgen_token_consume_whitespace(astgen);
	HscCodeSpan* span = astgen->span;

	span->location.column_start = span->location.column_end;

	HscString message = hsc_string((char*)&astgen->span->code[astgen->span->location.code_end_idx], 0);
	hsc_astgen_token_consume_until_any_byte(astgen, "\n");
	message.size = (char*)&astgen->span->code[astgen->span->location.code_end_idx] - message.data;

	hsc_astgen_token_error_1(astgen, "%.*s", (int)message.size, message.data);
}

bool hsc_astgen_parse_preprocessor_directive(HscAstGen* astgen, bool is_skipping_code, bool is_skipping_until_endif, U32 nested_level);

void hsc_astgen_parse_preprocessor_skip_false_conditional(HscAstGen* astgen, bool is_skipping_until_endif) {
	HSC_DEBUG_ASSERT(astgen->code_span_stack_count == 1, "internal error: condition preprocessor code should not be inside of macro expansions");
	HscCodeSpan* span = astgen->span;
	bool first_non_white_space_char = false;
	U32 nested_level = astgen->pp_if_spans_stack_count;
	while (span->location.code_end_idx < span->code_size) {
		U8 byte = span->code[astgen->span->location.code_end_idx];
		switch (byte) {
			case '\n':
				astgen->span->location.code_end_idx += 1;
				hsc_astgen_found_newline(astgen, span);
				first_non_white_space_char = true;
				continue;
			case '#':
				if (!first_non_white_space_char) {
					astgen->span->location.column_end += 1;
					hsc_astgen_token_error_1(astgen, "invalid token '#', preprocessor directives must be the first non-whitespace on the line");
				}

				if (hsc_astgen_parse_preprocessor_directive(astgen, true, is_skipping_until_endif, nested_level)) {
					return;
				}

				first_non_white_space_char = false;
				continue;
			case ' ':
				break;
			case '\0':
				return;
			default:
				first_non_white_space_char = false;
				break;
		}
		astgen->span->location.code_end_idx += 1;
		astgen->span->location.column_end += 1;
	}
}

bool hsc_astgen_parse_preprocessor_ifdef(HscAstGen* astgen, bool is_true) {
	hsc_astgen_token_consume_whitespace(astgen);
	astgen->span->location.code_start_idx = astgen->span->location.code_end_idx;
	astgen->span->location.column_start = astgen->span->location.column_end;

	HscString ident_string = hsc_astgen_parse_ident(astgen, "invalid token '%c' for macro identifier");
	astgen->span->location.code_end_idx += ident_string.size;
	astgen->span->location.column_end += ident_string.size;
	hsc_astgen_token_consume_whitespace(astgen);
	HscStringId identifier_string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)ident_string.data, ident_string.size);

	return hsc_hash_table_find(&astgen->macro_declarations, identifier_string_id.idx_plus_one, NULL) == is_true;
}

typedef struct HscPPEval HscPPEval;
struct HscPPEval {
	union {
		U64 u64;
		S64 s64;
	};
	bool is_signed;
};

HscPPEval hsc_astgen_preprocessor_eval_expr(HscAstGen* astgen, U32 min_precedence, U32* token_idx_mut, U32* token_value_idx_mut);

HscPPEval hsc_astgen_preprocessor_eval_unary_expr(HscAstGen* astgen, U32* token_idx_mut, U32* token_value_idx_mut) {
	HSC_ASSERT_ARRAY_BOUNDS(*token_idx_mut, astgen->tokens_count);
	HscToken token = astgen->tokens[*token_idx_mut];
	*token_idx_mut += 1;

	HscPPEval eval;
	HscUnaryOp unary_op;
	switch (token) {
		case HSC_TOKEN_LIT_U32:
		case HSC_TOKEN_LIT_U64: {
			HscConstantId constant_id = astgen->token_values[*token_value_idx_mut].constant_id;
			*token_value_idx_mut += 1;
			HscConstant constant = hsc_constant_table_get(&astgen->constant_table, constant_id);
			U64 u64;
			HSC_DEBUG_ASSERT(hsc_constant_as_uint(constant, &u64), "internal error: expected to be a unsigned int");

			eval.is_signed = false;
			eval.u64 = u64;
			break;
		};

		case HSC_TOKEN_LIT_S32:
		case HSC_TOKEN_LIT_S64: {
			HscConstantId constant_id = astgen->token_values[*token_value_idx_mut].constant_id;
			*token_value_idx_mut += 1;
			HscConstant constant = hsc_constant_table_get(&astgen->constant_table, constant_id);
			S64 s64;
			HSC_DEBUG_ASSERT(hsc_constant_as_sint(constant, &s64), "internal error: expected to be a signed int");

			eval.is_signed = true;
			eval.s64 = s64;
			break;
		};

		case HSC_TOKEN_PARENTHESIS_OPEN:
			eval = hsc_astgen_preprocessor_eval_expr(astgen, 0, token_idx_mut, token_value_idx_mut);
			HSC_ASSERT_ARRAY_BOUNDS(*token_idx_mut, astgen->tokens_count);
			token = astgen->tokens[*token_idx_mut];
			if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
				hsc_astgen_token_error_1(astgen, "expected a ')' here to finish the expression");
			}
			*token_idx_mut += 1;
			break;

		case HSC_TOKEN_TILDE: unary_op = HSC_UNARY_OP_BIT_NOT; goto UNARY;
		case HSC_TOKEN_EXCLAMATION_MARK: unary_op = HSC_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HSC_TOKEN_PLUS: unary_op = HSC_UNARY_OP_PLUS; goto UNARY;
		case HSC_TOKEN_MINUS: unary_op = HSC_UNARY_OP_NEGATE; goto UNARY;
UNARY:
		{
			eval = hsc_astgen_preprocessor_eval_expr(astgen, 0, token_idx_mut, token_value_idx_mut);
			switch (unary_op) {
				case HSC_TOKEN_TILDE:            eval.u64 = ~eval.u64; break;
				case HSC_TOKEN_EXCLAMATION_MARK: eval.u64 = !eval.u64; break;
				case HSC_TOKEN_PLUS:                                   break;
				case HSC_TOKEN_MINUS:            eval.u64 = -eval.u64; break;
				default: HSC_UNREACHABLE();
			}
			break;
		};

		default:
			if (HSC_TOKEN_IS_KEYWORD(token) || token == HSC_TOKEN_IDENT) {
				eval.is_signed = true;
				eval.s64 = 0;

				if (hsc_opt_is_enabled(&astgen->opts, HSC_OPT_PP_UNDEF_EVAL)) {
					hsc_astgen_token_error_1(astgen, "undefined identifier in preprocessor expression");
				}
			} else {
				*token_idx_mut -= 1;
				hsc_astgen_token_error_1(astgen, "'%s' is not a valid preprocessor unary expression token", hsc_token_strings[token]);
			}
			break;
	}

	return eval;
}

void hsc_astgen_preprocessor_eval_binary_op(HscAstGen* astgen, U32* token_idx_mut, HscBinaryOp* binary_op_type_out, U32* precedence_out) {
	HSC_ASSERT_ARRAY_BOUNDS(*token_idx_mut, astgen->tokens_count);
	HscToken token = astgen->tokens[*token_idx_mut];
	switch (token) {
		case HSC_TOKEN_ASTERISK:              *binary_op_type_out = HSC_BINARY_OP_MULTIPLY;              *precedence_out = 3;  break;
		case HSC_TOKEN_FORWARD_SLASH:         *binary_op_type_out = HSC_BINARY_OP_DIVIDE;                *precedence_out = 3;  break;
		case HSC_TOKEN_PERCENT:               *binary_op_type_out = HSC_BINARY_OP_MODULO;                *precedence_out = 3;  break;
		case HSC_TOKEN_PLUS:                  *binary_op_type_out = HSC_BINARY_OP_ADD;                   *precedence_out = 4;  break;
		case HSC_TOKEN_MINUS:                 *binary_op_type_out = HSC_BINARY_OP_SUBTRACT;              *precedence_out = 4;  break;
		case HSC_TOKEN_BIT_SHIFT_LEFT:        *binary_op_type_out = HSC_BINARY_OP_BIT_SHIFT_LEFT;        *precedence_out = 5;  break;
		case HSC_TOKEN_BIT_SHIFT_RIGHT:       *binary_op_type_out = HSC_BINARY_OP_BIT_SHIFT_RIGHT;       *precedence_out = 5;  break;
		case HSC_TOKEN_LESS_THAN:             *binary_op_type_out = HSC_BINARY_OP_LESS_THAN;             *precedence_out = 6;  break;
		case HSC_TOKEN_LESS_THAN_OR_EQUAL:    *binary_op_type_out = HSC_BINARY_OP_LESS_THAN_OR_EQUAL;    *precedence_out = 6;  break;
		case HSC_TOKEN_GREATER_THAN:          *binary_op_type_out = HSC_BINARY_OP_GREATER_THAN;          *precedence_out = 6;  break;
		case HSC_TOKEN_GREATER_THAN_OR_EQUAL: *binary_op_type_out = HSC_BINARY_OP_GREATER_THAN_OR_EQUAL; *precedence_out = 6;  break;
		case HSC_TOKEN_LOGICAL_EQUAL:         *binary_op_type_out = HSC_BINARY_OP_EQUAL;                 *precedence_out = 7;  break;
		case HSC_TOKEN_LOGICAL_NOT_EQUAL:     *binary_op_type_out = HSC_BINARY_OP_NOT_EQUAL;             *precedence_out = 7;  break;
		case HSC_TOKEN_AMPERSAND:             *binary_op_type_out = HSC_BINARY_OP_BIT_AND;               *precedence_out = 8;  break;
		case HSC_TOKEN_CARET:                 *binary_op_type_out = HSC_BINARY_OP_BIT_XOR;               *precedence_out = 9;  break;
		case HSC_TOKEN_PIPE:                  *binary_op_type_out = HSC_BINARY_OP_BIT_OR;                *precedence_out = 10; break;
		case HSC_TOKEN_LOGICAL_AND:           *binary_op_type_out = HSC_BINARY_OP_LOGICAL_AND;           *precedence_out = 11; break;
		case HSC_TOKEN_LOGICAL_OR:            *binary_op_type_out = HSC_BINARY_OP_LOGICAL_OR;            *precedence_out = 12; break;
		case HSC_TOKEN_QUESTION_MARK:         *binary_op_type_out = HSC_BINARY_OP_TERNARY;               *precedence_out = 13; break;

		case HSC_TOKEN_PARENTHESIS_CLOSE:
		case HSC_TOKEN_COLON:
			*binary_op_type_out = HSC_BINARY_OP_ASSIGN;
			*precedence_out = 0;
			break;

		default:
			hsc_astgen_token_error_1(astgen, "'%s' is not a valid preprocessor binary operator", hsc_token_strings[token]);
			break;
	}
}

HscPPEval hsc_astgen_preprocessor_eval_expr(HscAstGen* astgen, U32 min_precedence, U32* token_idx_mut, U32* token_value_idx_mut) {
	U32 callee_token_idx = astgen->token_read_idx;
	HscPPEval left_eval = hsc_astgen_preprocessor_eval_unary_expr(astgen, token_idx_mut, token_value_idx_mut);

	while (*token_idx_mut < astgen->tokens_count) {
		HscBinaryOp binary_op_type;
		U32 precedence;
		hsc_astgen_preprocessor_eval_binary_op(astgen, token_idx_mut, &binary_op_type, &precedence);
		if (binary_op_type == HSC_BINARY_OP_ASSIGN || (min_precedence && min_precedence <= precedence)) {
			return left_eval;
		}
		*token_idx_mut += 1;

		HscPPEval eval;
		if (binary_op_type == HSC_BINARY_OP_TERNARY) {
			HscPPEval true_eval = hsc_astgen_preprocessor_eval_expr(astgen, 0, token_idx_mut, token_value_idx_mut);
			HSC_ASSERT_ARRAY_BOUNDS(*token_idx_mut, astgen->tokens_count);
			HscToken token = astgen->tokens[*token_idx_mut];
			if (token != HSC_TOKEN_COLON) {
				hsc_astgen_token_error_1(astgen, "expected a ':' for the false side of the ternary expression");
			}
			*token_idx_mut += 1;
			HscPPEval false_eval = hsc_astgen_preprocessor_eval_expr(astgen, 0, token_idx_mut, token_value_idx_mut);
			eval = left_eval.u64 ? true_eval : false_eval;
		} else {
			HscPPEval right_eval = hsc_astgen_preprocessor_eval_expr(astgen, precedence, token_idx_mut, token_value_idx_mut);
			eval.is_signed = left_eval.is_signed == right_eval.is_signed;
			switch (binary_op_type) {
				case HSC_BINARY_OP_ADD: eval.u64 = left_eval.u64 + right_eval.u64; break;
				case HSC_BINARY_OP_SUBTRACT: eval.u64 = left_eval.u64 - right_eval.u64; break;
				case HSC_BINARY_OP_MULTIPLY: eval.u64 = left_eval.u64 * right_eval.u64; break;
				case HSC_BINARY_OP_DIVIDE:
					if (eval.is_signed) { eval.s64 = left_eval.s64 / right_eval.s64; }
					else {                eval.u64 = left_eval.u64 / right_eval.u64; }
					break;
				case HSC_BINARY_OP_MODULO:
					if (eval.is_signed) { eval.s64 = left_eval.s64 % right_eval.s64; }
					else {                eval.u64 = left_eval.u64 % right_eval.u64; }
					break;
				case HSC_BINARY_OP_BIT_AND:
					if (eval.is_signed) { eval.s64 = left_eval.s64 & right_eval.s64; }
					else {                eval.u64 = left_eval.u64 & right_eval.u64; }
					break;
				case HSC_BINARY_OP_BIT_OR:
					if (eval.is_signed) { eval.s64 = left_eval.s64 | right_eval.s64; }
					else {                eval.u64 = left_eval.u64 | right_eval.u64; }
					break;
				case HSC_BINARY_OP_BIT_XOR:
					if (eval.is_signed) { eval.s64 = left_eval.s64 ^ right_eval.s64; }
					else {                eval.u64 = left_eval.u64 ^ right_eval.u64; }
					break;
				case HSC_BINARY_OP_BIT_SHIFT_LEFT:
					if (eval.is_signed) { eval.s64 = left_eval.s64 << right_eval.s64; }
					else {                eval.u64 = left_eval.u64 << right_eval.u64; }
					break;
				case HSC_BINARY_OP_BIT_SHIFT_RIGHT:
					if (eval.is_signed) { eval.s64 = left_eval.s64 >> right_eval.s64; }
					else {                eval.u64 = left_eval.u64 >> right_eval.u64; }
					break;
				case HSC_BINARY_OP_EQUAL:     eval.u64 = left_eval.u64 == right_eval.u64; break;
				case HSC_BINARY_OP_NOT_EQUAL: eval.u64 = left_eval.u64 != right_eval.u64; break;
				case HSC_BINARY_OP_LESS_THAN:
					if (eval.is_signed) { eval.s64 = left_eval.s64 < right_eval.s64; }
					else {                eval.u64 = left_eval.u64 < right_eval.u64; }
					break;
				case HSC_BINARY_OP_LESS_THAN_OR_EQUAL:
					if (eval.is_signed) { eval.s64 = left_eval.s64 <= right_eval.s64; }
					else {                eval.u64 = left_eval.u64 <= right_eval.u64; }
					break;
				case HSC_BINARY_OP_GREATER_THAN:
					if (eval.is_signed) { eval.s64 = left_eval.s64 > right_eval.s64; }
					else {                eval.u64 = left_eval.u64 > right_eval.u64; }
					break;
				case HSC_BINARY_OP_GREATER_THAN_OR_EQUAL:
					if (eval.is_signed) { eval.s64 = left_eval.s64 >= right_eval.s64; }
					else {                eval.u64 = left_eval.u64 >= right_eval.u64; }
					break;
				case HSC_BINARY_OP_LOGICAL_AND:
					eval.u64 = left_eval.u64 && right_eval.u64;
					break;
				case HSC_BINARY_OP_LOGICAL_OR:
					eval.u64 = left_eval.u64 || right_eval.u64;
					break;
				default:
					HSC_UNREACHABLE();
			}
		}

		left_eval = eval;
	}

	return left_eval;
}

bool hsc_astgen_eval_preprocessor_if(HscAstGen* astgen, HscString condition) {
	U32 token_start_idx = astgen->tokens_count;
	U32 token_value_start_idx = astgen->token_values_count;
	U32 token_location_start_idx = astgen->token_locations_count;
	hsc_code_span_push_preprocessor_expression(astgen, condition);

	astgen->is_preprocessor_if_expression = true;
	void hsc_astgen_tokenize_run(HscAstGen* astgen);
	hsc_astgen_tokenize_run(astgen);
	astgen->is_preprocessor_if_expression = false;

	if (token_start_idx == astgen->tokens_count) {
		hsc_astgen_token_error_1(astgen, "condition expands to no preprocessor expression tokens");
	}

	U32 token_idx = token_start_idx;
	U32 token_value_idx = token_value_start_idx;
	bool is_true = !!hsc_astgen_preprocessor_eval_expr(astgen, 0, &token_idx, &token_value_idx).u64;

	HSC_DEBUG_ASSERT(token_idx == astgen->tokens_count, "internal error: preprocessor expression has not been fully evaluated");

	astgen->tokens_count = token_start_idx;
	astgen->token_values_count = token_value_start_idx;
	astgen->token_locations_count = token_location_start_idx;

	return is_true;
}

bool hsc_astgen_parse_preprocessor_if(HscAstGen* astgen) {
	hsc_astgen_token_consume_whitespace(astgen);
	astgen->span->location.code_start_idx = astgen->span->location.code_end_idx;
	astgen->span->location.column_start = astgen->span->location.column_end;

	HscString condition = hsc_string((char*)&astgen->span->code[astgen->span->location.code_end_idx], 0);
	hsc_astgen_token_consume_until_any_byte(astgen, "\n");
	condition.size = (char*)&astgen->span->code[astgen->span->location.code_end_idx] - condition.data;

	bool is_true = hsc_astgen_eval_preprocessor_if(astgen, condition);

	astgen->location.column_end += condition.size;
	astgen->location.code_end_idx += condition.size;

	return is_true;
}

void hsc_astgen_parse_preprocessor_defined(HscAstGen* astgen) {
	HscCodeSpan* span = astgen->span;

	span->location.column_end += sizeof("defined") - 1;
	span->location.code_end_idx += sizeof("defined") - 1;
	hsc_astgen_token_consume_whitespace(astgen);

	bool has_parenthesis = span->code[span->location.code_end_idx] == '(';
	if (has_parenthesis) {
		span->location.column_end += 1;
		span->location.code_end_idx += 1;
		hsc_astgen_token_consume_whitespace(astgen);
	}

	HscString macro_ident_string = hsc_astgen_parse_ident(astgen, "expected an 'identifier' of a macro to follow the 'defined' preprocessor unary operator");
	HscStringId macro_string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)macro_ident_string.data, macro_ident_string.size);

	bool does_macro_exist = hsc_hash_table_find(&astgen->macro_declarations, macro_string_id.idx_plus_one, NULL);
	span->location.column_end += macro_ident_string.size;
	span->location.code_end_idx += macro_ident_string.size;

	if (has_parenthesis) {
		hsc_astgen_token_consume_whitespace(astgen);
		if (span->code[span->location.code_end_idx] != ')') {
			hsc_astgen_token_error_1(astgen, "expected an ')' to finish the 'defined' preprocessor unary operator");
		}
		span->location.code_end_idx += 1;
		span->location.column_end += 1;
	}

	HscConstantId* basic_type_constant_ids = does_macro_exist ? astgen->basic_type_one_constant_ids : astgen->basic_type_zero_constant_ids;
	HscTokenValue token_value;
	token_value.constant_id = basic_type_constant_ids[HSC_DATA_TYPE_U32];

	hsc_astgen_add_token(astgen, HSC_TOKEN_LIT_U32);
	hsc_astgen_add_token_value(astgen, token_value);
}

char* hsc_pp_directive_enum_strings[HSC_PP_DIRECTIVE_COUNT] = {
	[HSC_PP_DIRECTIVE_DEFINE] = "HSC_PP_DIRECTIVE_DEFINE",
	[HSC_PP_DIRECTIVE_UNDEF] = "HSC_PP_DIRECTIVE_UNDEF",
	[HSC_PP_DIRECTIVE_INCLUDE] = "HSC_PP_DIRECTIVE_INCLUDE",
	[HSC_PP_DIRECTIVE_IF] = "HSC_PP_DIRECTIVE_IF",
	[HSC_PP_DIRECTIVE_IFDEF] = "HSC_PP_DIRECTIVE_IFDEF",
	[HSC_PP_DIRECTIVE_IFNDEF] = "HSC_PP_DIRECTIVE_IFNDEF",
	[HSC_PP_DIRECTIVE_ELSE] = "HSC_PP_DIRECTIVE_ELSE",
	[HSC_PP_DIRECTIVE_ELIF] = "HSC_PP_DIRECTIVE_ELIF",
	[HSC_PP_DIRECTIVE_ELIFDEF] = "HSC_PP_DIRECTIVE_ELIFDEF",
	[HSC_PP_DIRECTIVE_ELIFNDEF] = "HSC_PP_DIRECTIVE_ELIFNDEF",
	[HSC_PP_DIRECTIVE_ENDIF] = "HSC_PP_DIRECTIVE_ENDIF",
	[HSC_PP_DIRECTIVE_LINE] = "HSC_PP_DIRECTIVE_LINE",
	[HSC_PP_DIRECTIVE_ERROR] = "HSC_PP_DIRECTIVE_ERROR",
	[HSC_PP_DIRECTIVE_PRAGMA] = "HSC_PP_DIRECTIVE_PRAGMA",
};

char* hsc_pp_directive_strings[HSC_PP_DIRECTIVE_COUNT] = {
	[HSC_PP_DIRECTIVE_DEFINE] = "define",
	[HSC_PP_DIRECTIVE_UNDEF] = "undef",
	[HSC_PP_DIRECTIVE_INCLUDE] = "include",
	[HSC_PP_DIRECTIVE_IF] = "if",
	[HSC_PP_DIRECTIVE_IFDEF] = "ifdef",
	[HSC_PP_DIRECTIVE_IFNDEF] = "ifndef",
	[HSC_PP_DIRECTIVE_ELSE] = "else",
	[HSC_PP_DIRECTIVE_ELIF] = "elif",
	[HSC_PP_DIRECTIVE_ELIFDEF] = "elifdef",
	[HSC_PP_DIRECTIVE_ELIFNDEF] = "elifndef",
	[HSC_PP_DIRECTIVE_ENDIF] = "endif",
	[HSC_PP_DIRECTIVE_LINE] = "line",
	[HSC_PP_DIRECTIVE_ERROR] = "error",
	[HSC_PP_DIRECTIVE_PRAGMA] = "pragma",
};

U32 hsc_pp_directive_hashes[] = {
	[HSC_PP_DIRECTIVE_DEFINE] = 0x6a9f8552,
	[HSC_PP_DIRECTIVE_UNDEF] = 0x75191b51,
	[HSC_PP_DIRECTIVE_INCLUDE] = 0x75597a67,
	[HSC_PP_DIRECTIVE_IF] = 0x39386e06,
	[HSC_PP_DIRECTIVE_IFDEF] = 0x546cc0ed,
	[HSC_PP_DIRECTIVE_IFNDEF] = 0xf1e59c9f,
	[HSC_PP_DIRECTIVE_ELSE] = 0xbdbf5bf0,
	[HSC_PP_DIRECTIVE_ELIF] = 0xc0a5c8c3,
	[HSC_PP_DIRECTIVE_ELIFDEF] = 0x7aeee312,
	[HSC_PP_DIRECTIVE_ELIFNDEF] = 0xdc679c86,
	[HSC_PP_DIRECTIVE_ENDIF] = 0x1c12ad4d,
	[HSC_PP_DIRECTIVE_LINE] = 0x17db1627,
	[HSC_PP_DIRECTIVE_ERROR] = 0x21918751,
	[HSC_PP_DIRECTIVE_PRAGMA] = 0x19fa4625,
};

#define HSC_FNV_HASH_32_INITIAL 0x811c9dc5
U32 hsc_fnv_hash_32(char* bytes, U32 byte_count, U32 hash) {
	char* bytes_end = bytes + byte_count;
	while (bytes < bytes_end) {
		hash = hash ^ *bytes;
		hash = hash * 0x01000193;
		bytes += 1;
	}
	return hash;
}

void hsc_generate_enum_hashes(char* array_name, char** strings, char** enum_strings, U32 enums_count) {
	U32 used_hashes[128];
	HSC_ASSERT(enums_count <= HSC_ARRAY_COUNT(used_hashes), "internal error: used_hashes needs to be atleast %u", enums_count);

	for (U32 idx = 0; idx < enums_count; idx += 1) {
		char* string = strings[idx];
		U32 string_size = strlen(string);
		U32 hash = hsc_fnv_hash_32(string, string_size, HSC_FNV_HASH_32_INITIAL);
		for (U32 used_idx = 0; used_idx < idx; used_idx += 1) {
			HSC_ASSERT(used_hashes[used_idx] != hash, "internal error: '%s' and '%s' have the same hash of '0x%x'", strings[idx], strings[used_idx]);
		}
		used_hashes[idx] = hash;
	}

	printf("U32 %s[] = {\n", array_name);
	for (U32 idx = 0; idx < enums_count; idx += 1) {
		char* enum_string = enum_strings[idx];
		U32 hash = used_hashes[idx];
		printf("\t[%s] = 0x%x,\n", enum_string, hash);
	}
	printf("};\n");
};

void hsc_generate_hashes() {
	hsc_generate_enum_hashes("hsc_pp_directive_hashes", hsc_pp_directive_strings, hsc_pp_directive_enum_strings, HSC_PP_DIRECTIVE_COUNT);
	exit(0);
}

U32 hsc_string_to_enum_hashed_find(HscString string, U32* enum_hashes, U32 enums_count) {
	U32 hash = hsc_fnv_hash_32(string.data, string.size, HSC_FNV_HASH_32_INITIAL);
	for (U32 enum_ = 0; enum_ < enums_count; enum_ += 1) {
		if (enum_hashes[enum_] == hash) {
			return enum_;
		}
	}
	return enums_count;
}

void hsc_parse_preprocessor_found_else(HscAstGen* astgen) {
	HscPPIfSpan* pp_if_span = hsc_pp_if_span_peek_top(astgen);
	pp_if_span->has_else = true;
}

void hsc_parse_preprocessor_ensure_first_else(HscAstGen* astgen, HscString ident_string) {
	HscPPIfSpan* pp_if_span = hsc_pp_if_span_peek_top(astgen);
	if (pp_if_span->has_else) {
		U32 pp_if_token_idx = astgen->tokens_count;
		hsc_astgen_add_token(astgen, 0);
		astgen->token_locations[astgen->token_locations_count - 1] = pp_if_span->location;
		hsc_astgen_token_error_2(astgen, pp_if_token_idx, "'#%.*s' cannot follow an '#else' in the same preprocessor if chain", (int)ident_string.size, ident_string.data);
	}
}

//
// return true if code should be processed and false if code should be skipped
bool hsc_astgen_parse_preprocessor_directive(HscAstGen* astgen, bool is_skipping_code, bool is_skipping_until_endif, U32 nested_level) {
	HscCodeSpan* span = astgen->span;
	span->location.code_end_idx += 1; // skip '#'
	span->location.column_end += 1;
	hsc_astgen_token_consume_whitespace(astgen);

	U8 byte = span->code[span->location.code_end_idx];
	switch (byte) {
		case '\r':
		case '\n':
		case '\0':
			return false;
	}

	HscString ident_string = hsc_astgen_parse_ident(astgen, "invalid token '%c' for preprocessor directive");
	span->location.code_end_idx += ident_string.size;
	span->location.column_end += ident_string.size;
	if (ident_string.size == 0) {
		goto END;
	}

	HscPPDirective directive = hsc_string_to_enum_hashed_find(ident_string, hsc_pp_directive_hashes, HSC_PP_DIRECTIVE_COUNT);
	if (directive == HSC_PP_DIRECTIVE_COUNT) {
		hsc_astgen_token_error_1(astgen, "invalid preprocessor directive '#%.*s'", (int)ident_string.size, ident_string.data);
	}

#if HSC_DEBUG_CODE_PREPROCESSOR
	U32 debug_indent_level = astgen->pp_if_spans_stack_count;
	U32 debug_line = span->location.line_end;
	if (directive == HSC_PP_DIRECTIVE_ENDIF) {
		debug_indent_level -= 1;
	}
#endif

	if (is_skipping_code) {
		switch (directive) {
			case HSC_PP_DIRECTIVE_DEFINE:
			case HSC_PP_DIRECTIVE_UNDEF:
			case HSC_PP_DIRECTIVE_INCLUDE:
			case HSC_PP_DIRECTIVE_LINE:
			case HSC_PP_DIRECTIVE_ERROR:
			case HSC_PP_DIRECTIVE_PRAGMA:
				break;
			case HSC_PP_DIRECTIVE_IF:
			case HSC_PP_DIRECTIVE_IFDEF:
			case HSC_PP_DIRECTIVE_IFNDEF: {
				hsc_pp_if_span_push(astgen, directive);
				break;
			};
			case HSC_PP_DIRECTIVE_ENDIF:
				if (astgen->pp_if_spans_stack_count == nested_level) {
					is_skipping_code = false;
				}
				hsc_pp_if_span_pop(astgen);
				break;
			case HSC_PP_DIRECTIVE_ELSE:
			case HSC_PP_DIRECTIVE_ELIF:
			case HSC_PP_DIRECTIVE_ELIFDEF:
			case HSC_PP_DIRECTIVE_ELIFNDEF:
				hsc_parse_preprocessor_ensure_first_else(astgen, ident_string);
				if (directive == HSC_PP_DIRECTIVE_ELSE) {
					hsc_parse_preprocessor_found_else(astgen);
				}
				if (!is_skipping_until_endif && astgen->pp_if_spans_stack_count == nested_level) {
					switch (directive) {
						case HSC_PP_DIRECTIVE_ELSE:
							is_skipping_code = false;
							break;
						case HSC_PP_DIRECTIVE_ELIF:
							is_skipping_code = !hsc_astgen_parse_preprocessor_if(astgen);
							break;
						case HSC_PP_DIRECTIVE_ELIFDEF:
						case HSC_PP_DIRECTIVE_ELIFNDEF:
							is_skipping_code = !hsc_astgen_parse_preprocessor_ifdef(astgen, directive == HSC_PP_DIRECTIVE_ELIFDEF);
							break;
					}
				}
				break;
		}

#if HSC_DEBUG_CODE_PREPROCESSOR
		char* plus_or_minus = is_skipping_code ? "-" : "+";
		printf("%s(%u)#%-7s at line %u\n", plus_or_minus, debug_indent_level, hsc_pp_directive_strings[directive], debug_line);
#endif
	} else {
		switch (directive) {
			case HSC_PP_DIRECTIVE_DEFINE:
				hsc_astgen_parse_preprocessor_define(astgen);
				break;
			case HSC_PP_DIRECTIVE_UNDEF:
				hsc_astgen_parse_preprocessor_undef(astgen);
				break;
			case HSC_PP_DIRECTIVE_INCLUDE:
				hsc_astgen_parse_preprocessor_include(astgen);
				break;
			case HSC_PP_DIRECTIVE_LINE:
				HSC_ABORT("TODO");
				// hsc_astgen_parse_preprocessor_line(astgen);
				break;
			case HSC_PP_DIRECTIVE_ERROR:
				hsc_astgen_parse_preprocessor_error(astgen);
				break;
			case HSC_PP_DIRECTIVE_PRAGMA:
				HSC_ABORT("TODO");
				// hsc_astgen_parse_preprocessor_pragma(astgen);
				break;
			case HSC_PP_DIRECTIVE_IF: {
				hsc_pp_if_span_push(astgen, directive);
				is_skipping_code = !hsc_astgen_parse_preprocessor_if(astgen);
				break;
			};
			case HSC_PP_DIRECTIVE_IFDEF:
			case HSC_PP_DIRECTIVE_IFNDEF: {
				hsc_pp_if_span_push(astgen, directive);
				is_skipping_code = !hsc_astgen_parse_preprocessor_ifdef(astgen, directive == HSC_PP_DIRECTIVE_IFDEF);
				break;
			};
			case HSC_PP_DIRECTIVE_ENDIF:
				if (astgen->pp_if_spans_stack_count == 0) {
					hsc_astgen_token_error_1(astgen, "'#%.*s' must follow an open #if/#ifdef/#ifndef", (int)ident_string.size, ident_string.data);
				}
				hsc_pp_if_span_pop(astgen);
				break;
			case HSC_PP_DIRECTIVE_ELSE:
			case HSC_PP_DIRECTIVE_ELIF:
			case HSC_PP_DIRECTIVE_ELIFDEF:
			case HSC_PP_DIRECTIVE_ELIFNDEF:
				hsc_parse_preprocessor_ensure_first_else(astgen, ident_string);
				if (directive == HSC_PP_DIRECTIVE_ELSE) {
					hsc_parse_preprocessor_found_else(astgen);
				}

				if (astgen->pp_if_spans_stack_count == 0) {
					hsc_astgen_token_error_1(astgen, "'#%.*s' must follow an open #if/#ifdef/#ifndef", (int)ident_string.size, ident_string.data);
				}
				is_skipping_code = true;
				is_skipping_until_endif = true;
				break;
		}

#if HSC_DEBUG_CODE_PREPROCESSOR
		printf("+(%u)#%-7s at line %u\n", debug_indent_level, hsc_pp_directive_strings[directive], debug_line);
#endif

		if (is_skipping_code) {
			hsc_astgen_parse_preprocessor_skip_false_conditional(astgen, is_skipping_until_endif);
		}
		is_skipping_code = false;
	}

	hsc_astgen_token_consume_whitespace(astgen);
END: {}
	if (!is_skipping_code) {
		byte = span->code[span->location.code_end_idx];
		char next_byte = span->code[span->location.code_end_idx + 1];
		if (
			!(
				byte == '\r' ||
				byte == '\n' ||
				(byte == '/' && next_byte == '/') ||
				(byte == '/' && next_byte == '*')
			 )
		) {
			if (span->location.code_end_idx < span->code_size) {
				hsc_astgen_token_error_1(astgen, "too many arguments for the '#%.*s' directive", (int)ident_string.size, ident_string.data);
			}
		}
	}

	return !is_skipping_code;
}

U32 hsc_astgen_macro_find_param(HscAstGen* astgen, HscMacro* macro, HscStringId param_string_id) {
	U32 params_start_idx = macro->params_start_idx;
	for (U32 idx = 0; idx < macro->params_count; idx += 1) {
		if (param_string_id.idx_plus_one == astgen->macro_params[params_start_idx + idx].idx_plus_one) {
			return idx + 1;
		}
	}

	return 0;
}

bool hsc_astgen_macro_has_va_args(HscAstGen* astgen, HscMacro* macro) {
	return macro->params_count && astgen->macro_params[macro->params_start_idx + macro->params_count - 1].idx_plus_one == 0;
}

void hsc_macro_paste_buffer_append(HscAstGen* astgen, HscString string) {
	U32 insert_idx = astgen->macro_paste_buffer_size;
	HSC_ASSERT_ARRAY_BOUNDS(insert_idx + string.size - 1, astgen->macro_paste_buffer_cap);
	memcpy(astgen->macro_paste_buffer + insert_idx, string.data, string.size);
	astgen->macro_paste_buffer_size += string.size;
}

void hsc_macro_args_add(HscAstGen* astgen, HscMacroArg arg) {
	HSC_ASSERT_ARRAY_BOUNDS(astgen->macro_args_count, astgen->macro_args_cap);
	astgen->macro_args[astgen->macro_args_count] = arg;
	astgen->macro_args_count += 1;
}

bool hsc_code_span_push_macro_arg_if_it_is_one(HscAstGen* astgen, HscString ident_string, HscStringId identifier_string_id) {
	HscCodeSpan* span = astgen->span;
	HscMacro* macro = span->macro;
	HscCodeSpan* macro_span = span;
	HscCodeSpan* evaluating_macro_span = NULL;
	if (!macro) {
		evaluating_macro_span = hsc_code_span_find_evaluating_macro(astgen, &macro_span);
		macro = macro_span->macro;
		if (evaluating_macro_span == NULL) {
			return false;
		}
	}

	U32 param_id;
	if (identifier_string_id.idx_plus_one == astgen->va_args_string_id.idx_plus_one) {
		if (hsc_astgen_macro_has_va_args(astgen, macro)) {
			param_id = macro->params_count;
		} else {
			hsc_astgen_token_error_1(astgen, "'__VA_ARGS__' can only be used in a function-like macro that has '...' as it's last parameter");
		}
	} else {
		param_id = hsc_astgen_macro_find_param(astgen, macro, identifier_string_id);
	}

	if (param_id) {
		span->location.column_end += ident_string.size;
		span->location.code_end_idx += ident_string.size;

		U32 macro_arg_idx = macro_span->macro_args_start_idx + param_id - 1;
		HscMacroArg arg = astgen->macro_args[macro_arg_idx];

		hsc_code_span_push_macro_arg(astgen, macro_span, macro_arg_idx, (U8*)arg.string.data, arg.string.size);
		if (!evaluating_macro_span) {
			evaluating_macro_span = &astgen->code_spans[astgen->code_span_stack[astgen->code_span_stack_count - 1]];
		}

		HscLocation* callsite_location = &astgen->token_locations[evaluating_macro_span->location.parent_location_idx];
		callsite_location->parent_location_idx = arg.callsite_location_idx;

		return true;
	}
	return false;
}

bool hsc_code_span_push_macro_if_it_is_one(HscAstGen* astgen, HscString ident_string, HscStringId string_id) {
	U32 macro_idx;
	if (!hsc_hash_table_find(&astgen->macro_declarations, string_id.idx_plus_one, &macro_idx)) {
		return false;
	}

	HscCodeSpan* span = astgen->span;
	if (macro_idx == U32_MAX) {
		span->location.column_start = span->location.column_end;
		span->location.code_start_idx = span->location.code_end_idx;
		span->location.column_end += ident_string.size;
		span->location.code_end_idx += ident_string.size;

		HscPredefinedMacro predefined_macro = string_id.idx_plus_one - astgen->predefined_macro_identifier_string_start_id.idx_plus_one;
		HSC_DEBUG_ASSERT(predefined_macro < HSC_PREDEFINED_MACRO_COUNT, "internal error: expected this to be a predefined macro");
		hsc_code_span_push_predefined_macro(astgen, predefined_macro);
		return true;
	}

	HscMacro* macro = hsc_macro_get(astgen, macro_idx);

	if (hsc_code_span_is_macro_on_stack(astgen, macro)) {
		return false;
	}

	span->location.column_start = span->location.column_end;
	span->location.code_start_idx = span->location.code_end_idx;
	span->location.column_end += ident_string.size;
	span->location.code_end_idx += ident_string.size;

	if (!macro->is_function) {
		if (macro->value_string.size) {
			hsc_code_span_push_macro(astgen, macro);
		}
		return true;
	}

	if (span->code[span->location.code_end_idx] != '(') {
		hsc_astgen_token_error_1(astgen, "'%.*s' is defined as a macro function, please use a '(' to begin specifying arguments", (int)ident_string.size, ident_string.data);
	}
	span->location.column_end += 1;
	span->location.code_end_idx += 1;
	hsc_astgen_token_consume_whitespace(astgen);

	HscLocation arg_location = span->location;
	U32 args_count = 0;
	if (span->code[span->location.code_end_idx] == ')') {
		span->location.column_end += 1;
		span->location.code_end_idx += 1;
	} else {
		HscStringId* macro_params = &astgen->macro_params[macro->params_start_idx];
		bool span_has_args = span->macro && span->macro->params_count;

		HscString va_args = {0};
		while (1) {
			bool is_vararg = args_count ? macro_params[HSC_MIN(args_count, macro->params_count)].idx_plus_one == 0 : false;

			HscLocation arg_location = span->location;
			arg_location.column_start = span->location.column_end;
			arg_location.code_start_idx = span->location.code_end_idx;

			HscMacroArg arg;
			arg.callsite_location_idx = astgen->token_locations_count;
			arg.string = hsc_string((char*)&span->code[span->location.code_end_idx], 0);

			U32 nested_parenthesis = 0;
			bool arg_used_va_args = false;
			while (1) {
				hsc_astgen_token_consume_until_any_byte(astgen, is_vararg || nested_parenthesis ? "()_" : ",()_");
				U8 byte = span->code[span->location.code_end_idx];
				if (byte == '(') {
					nested_parenthesis += 1;
				} else if (byte == '_') {
					if (nested_parenthesis == 0) {
						HscString code = hsc_string((char*)&span->code[span->location.code_end_idx], span->code_size - span->location.code_end_idx);
						HscString ident = hsc_astgen_parse_ident_from_code(astgen, code, "");
						if (hsc_string_eq_lit(ident, "__VA_ARGS__")) {
							span->location.column_end += ident.size - 1;
							span->location.code_end_idx += ident.size - 1;
							arg_used_va_args = true;
							if (va_args.data == NULL) {
								HscCodeSpan* macro_span;
								HscCodeSpan* evaluating_macro_span = hsc_code_span_find_evaluating_macro(astgen, &macro_span);

								if (!hsc_astgen_macro_has_va_args(astgen, macro_span->macro)) {
									hsc_astgen_token_error_1(astgen, "'__VA_ARGS__' can only be used in a function-like macro that has '...' as it's last parameter");
								}

								va_args = astgen->macro_args[span->macro_args_start_idx + macro_span->macro->params_count - 1].string;
							}

						}
					}
				} else {
					if (nested_parenthesis) {
						nested_parenthesis -= 1;
					} else {
						break;
					}
				}
				span->location.column_end += 1;
				span->location.code_end_idx += 1;
			}
			arg.string.size = &span->code[span->location.code_end_idx] - (U8*)arg.string.data;

			if (arg_used_va_args) {
				HscString pasted_arg = hsc_string(&astgen->macro_paste_buffer[astgen->macro_paste_buffer_size], 0);
				for (U32 idx = 0; idx < arg.string.size;) {
					HscString cursor = hsc_string_slice_start(arg.string, idx);
					if (
						arg.string.data[idx] == '_' &&
						hsc_string_eq_lit(hsc_string_slice_end(cursor, HSC_MIN(cursor.size, sizeof("__VA_ARGS__") - 1)), "__VA_ARGS__")
					) {
						hsc_macro_paste_buffer_append(astgen, va_args);
						idx += sizeof("__VA_ARGS__") - 1;
					} else {
						cursor.size = 1;
						hsc_macro_paste_buffer_append(astgen, cursor);
						idx += 1;
					}
				}

				pasted_arg.size = &astgen->macro_paste_buffer[astgen->macro_paste_buffer_size] - pasted_arg.data;
				arg.string = pasted_arg;
			}

			arg_location.column_end = span->location.column_end;
			arg_location.code_end_idx = span->location.code_end_idx;
			_hsc_token_location_add(astgen, &arg_location);

			if (arg_used_va_args) {
				U32 arg_start_idx = 0;
				HscString pasted_arg = arg.string;
				for (U32 arg_end_idx = 0; arg_end_idx < pasted_arg.size; arg_end_idx += 1) {
					if (pasted_arg.data[arg_end_idx] == ',' || arg_end_idx + 1 == pasted_arg.size) {
						arg.string = hsc_string_slice(pasted_arg, arg_start_idx, arg_end_idx);
						if (arg_end_idx + 1 == pasted_arg.size) {
							arg.string.size += 1;
						}

						hsc_macro_args_add(astgen, arg);
						args_count += 1;
						arg_start_idx = arg_end_idx + 1;
					}
				}
			} else {
				hsc_macro_args_add(astgen, arg);
				args_count += 1;
			}

			U8 byte = span->code[span->location.code_end_idx];
			span->location.column_end += 1;
			span->location.code_end_idx += 1;
			if (byte == ')') {
				break;
			}

			hsc_astgen_token_consume_whitespace(astgen);
		}
	}

	if (hsc_astgen_macro_has_va_args(astgen, macro) && args_count + 1 == macro->params_count) {
		HscMacroArg arg;
		arg.callsite_location_idx = astgen->token_locations_count;
		arg.string = (HscString){0};

		HscLocation arg_location = span->location;
		arg_location.code_start_idx -= 1;
		arg_location.column_start -= 1;
		_hsc_token_location_add(astgen, &arg_location);

		hsc_macro_args_add(astgen, arg);
		args_count += 1;
	}

	hsc_astgen_ensure_macro_args_count(astgen, macro, args_count);

	if (macro->value_string.size) {
		hsc_code_span_push_macro(astgen, macro);
	}
	return true;
}

void hsc_astgen_tokenize_run(HscAstGen* astgen) {
	while (astgen->span) {
		if (astgen->span->location.code_end_idx >= astgen->span->code_size) {
			if (hsc_code_span_pop(astgen)) {
				return;
			}
			continue;
		}

		uint8_t byte = astgen->span->code[astgen->span->location.code_end_idx];

		astgen->span->location.code_start_idx = astgen->span->location.code_end_idx;
		astgen->span->location.line_start = astgen->span->location.line_end;
		astgen->span->location.column_start = astgen->span->location.column_end;

		HscToken token = HSC_TOKEN_COUNT;
		HscToken close_token;
		uint32_t token_size = 1;
		switch (byte) {
			case ' ':
			case '\t':
				hsc_astgen_token_consume_whitespace(astgen);
				continue;
			case '\r':
			case '\n':
				astgen->span->location.code_start_idx += 1;
				astgen->span->location.code_end_idx += 1;
				if (byte == '\n') {
					hsc_astgen_found_newline(astgen, astgen->span);
				}
				continue;
			case '.': token = HSC_TOKEN_FULL_STOP; break;
			case ',': token = HSC_TOKEN_COMMA; break;
			case ';': token = HSC_TOKEN_SEMICOLON; break;
			case ':': token = HSC_TOKEN_COLON; break;
			case '~': token = HSC_TOKEN_TILDE; break;
			case '?': token = HSC_TOKEN_QUESTION_MARK; break;
			case '+': {
				U8 next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_ADD_ASSIGN;
				} else if (next_byte == '+') {
					token_size = 2;
					token = HSC_TOKEN_INCREMENT;
				} else {
					token = HSC_TOKEN_PLUS;
				}
				break;
			};
			case '-': {
				U8 next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
				if (isdigit(next_byte)) {
					token_size = hsc_parse_num(astgen, &token);
				} else if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_SUBTRACT_ASSIGN;
				} else if (next_byte == '-') {
					token_size = 2;
					token = HSC_TOKEN_DECREMENT;
				} else {
					token = HSC_TOKEN_MINUS;
				}
				break;
			};
			case '/': {
				uint8_t next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
				if (next_byte == '/') {
					astgen->span->location.code_end_idx += 2;
					while (astgen->span->location.code_end_idx < astgen->span->code_size) {
						uint8_t b = astgen->span->code[astgen->span->location.code_end_idx];
						if (b == '\n') {
							break;
						}
						astgen->span->location.code_end_idx += 1;
					}

					token_size = astgen->span->location.code_end_idx - astgen->span->location.code_start_idx;
					astgen->span->location.column_start += token_size;
					astgen->span->location.column_end += token_size;
					continue;
				} else if (next_byte == '*') {
					astgen->span->location.code_end_idx += 2;
					astgen->span->location.column_end += 2;
					while (astgen->span->location.code_end_idx < astgen->span->code_size) {
						uint8_t b = astgen->span->code[astgen->span->location.code_end_idx];
						astgen->span->location.code_end_idx += 1;
						astgen->span->location.column_end += 1;
						if (b == '*') {
							b = astgen->span->code[astgen->span->location.code_end_idx];
							if (b == '/') { // no need to check in bounds see _HSC_TOKENIZER_LOOK_HEAD_SIZE
								astgen->span->location.code_end_idx += 1;
								astgen->span->location.column_end += 1;
								break;
							}
						} else if (byte == '\n') {
							hsc_astgen_found_newline(astgen, astgen->span);
						}
					}

					astgen->span->location.column_start = astgen->span->location.column_end;
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
				if (astgen->span->code[astgen->span->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_MULTIPLY_ASSIGN;
				} else {
					token = HSC_TOKEN_ASTERISK;
				}
				break;
			case '%':
				if (astgen->span->code[astgen->span->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_MODULO_ASSIGN;
				} else {
					token = HSC_TOKEN_PERCENT;
				}
				break;
			case '&': {
				U8 next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
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
				U8 next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
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
				if (astgen->span->code[astgen->span->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_BIT_XOR_ASSIGN;
				} else {
					token = HSC_TOKEN_CARET;
				}
				break;
			case '!':
				if (astgen->span->code[astgen->span->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_LOGICAL_NOT_EQUAL;
				} else {
					token = HSC_TOKEN_EXCLAMATION_MARK;
				}
				break;
			case '=':
				if (astgen->span->code[astgen->span->location.code_end_idx + 1] == '=') {
					token_size = 2;
					token = HSC_TOKEN_LOGICAL_EQUAL;
				} else {
					token = HSC_TOKEN_EQUAL;
				}
				break;
			case '<': {
				if (astgen->is_preprocessor_include) {
					hsc_parse_string(astgen, '>');
					continue;
				}
				U8 next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_LESS_THAN_OR_EQUAL;
				} else if (next_byte == '<') {
					if (astgen->span->code[astgen->span->location.code_end_idx + 2] == '=') {
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
				U8 next_byte = astgen->span->code[astgen->span->location.code_end_idx + 1];
				if (next_byte == '=') {
					token_size = 2;
					token = HSC_TOKEN_GREATER_THAN_OR_EQUAL;
				} else if (next_byte == '>') {
					if (astgen->span->code[astgen->span->location.code_end_idx + 2] == '=') {
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
				goto OPEN_BRACKETS;
			case '[':
				token = HSC_TOKEN_SQUARE_OPEN;
				close_token = HSC_TOKEN_SQUARE_CLOSE;
				goto OPEN_BRACKETS;
OPEN_BRACKETS:
			{
				if (astgen->brackets_to_close_count >= _HSC_TOKENIZER_NESTED_BRACKETS_CAP) {
					hsc_astgen_token_error_1(astgen, "nested brackets capacity of '%u' has been exceeded", _HSC_TOKENIZER_NESTED_BRACKETS_CAP);
				}
				astgen->brackets_to_close[astgen->brackets_to_close_count] = close_token;
				astgen->brackets_to_close_token_indices[astgen->brackets_to_close_count] = astgen->tokens_count;
				astgen->brackets_to_close_count += 1;
				break;
			};
			case '}':
				token = HSC_TOKEN_CURLY_CLOSE;
				goto CLOSE_BRACKETS;
			case ')':
				token = HSC_TOKEN_PARENTHESIS_CLOSE;
				goto CLOSE_BRACKETS;
			case ']':
				token = HSC_TOKEN_SQUARE_CLOSE;
				goto CLOSE_BRACKETS;
CLOSE_BRACKETS:
			{
				if (astgen->brackets_to_close_count == 0) {
					hsc_astgen_token_error_1(astgen, "no brackets are open to close '%c'", byte);
				}

				astgen->brackets_to_close_count -= 1;
				if (astgen->brackets_to_close[astgen->brackets_to_close_count] != token) {
					astgen->span->location.code_end_idx += 1;
					astgen->span->location.column_end += 1;
					hsc_astgen_token_error_2(astgen, astgen->brackets_to_close_token_indices[astgen->brackets_to_close_count], "expected to close bracket pair with '%s' but got '%c'", hsc_token_strings[astgen->brackets_to_close[astgen->brackets_to_close_count]], byte);
				}
				break;
			};

			case '"':
				hsc_parse_string(astgen, '"');
				continue;

			case '\\':
				hsc_astgen_token_consume_backslash(astgen);
				continue;

			default: {
				if ('0' <= byte && byte <= '9') {
					token_size = hsc_parse_num(astgen, &token);
					break;
				}

				if (byte == '#') {
					if (astgen->span->macro || astgen->span->macro_arg_id) {
						astgen->span->location.column_end += 1;
						hsc_astgen_token_error_1(astgen, "invalid token '#', preprocessor directives cannot be in macros");
					}

					if (astgen->tokens_count == 0 || astgen->token_locations[astgen->tokens_count - 1].line_end != astgen->span->location.line_start) {
						hsc_astgen_parse_preprocessor_directive(astgen, false, false, 0);
						if (astgen->include_code_file_id.idx_plus_one) {
							hsc_code_span_push_file(astgen, astgen->include_code_file_id);
							astgen->include_code_file_id.idx_plus_one = 0;
						}
						continue;
					} else {
						hsc_astgen_token_error_1(astgen, "invalid token '#', preprocessor directives must be the first non-whitespace on the line");
					}
				}

				HscString ident_string = hsc_astgen_parse_ident(astgen, "invalid token '%c'");
				token_size = ident_string.size;

				HscStringId string_id = hsc_string_table_deduplicate(&astgen->string_table, (char*)ident_string.data, ident_string.size);
				if (astgen->is_preprocessor_if_expression && string_id.idx_plus_one == astgen->defined_string_id.idx_plus_one) {
					hsc_astgen_parse_preprocessor_defined(astgen);
					continue;
				}

				if (astgen->span->macro || astgen->span->macro_arg_id) {
					if (hsc_code_span_push_macro_arg_if_it_is_one(astgen, ident_string, string_id)) {
						continue;
					}
				} else {
					if (string_id.idx_plus_one == astgen->va_args_string_id.idx_plus_one) {
						astgen->span->location.column_end += token_size;
						hsc_astgen_token_error_1(astgen, "'__VA_ARGS__' can only be used in a function-like macro that has '...' as it's last parameter");
					}
				}

				if (hsc_code_span_push_macro_if_it_is_one(astgen, ident_string, string_id)) {
					continue;
				}

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

		astgen->span->location.code_end_idx += token_size;
		astgen->span->location.column_end += token_size;
		hsc_astgen_add_token(astgen, token);
	}
}

void hsc_astgen_tokenize(HscAstGen* astgen) {
	astgen->brackets_to_close_count = 0;

	hsc_astgen_tokenize_run(astgen);
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

HscTokenValue hsc_token_value_peek(HscAstGen* astgen) {
	HscTokenValue value = astgen->token_values[HSC_MIN(astgen->token_value_read_idx, astgen->token_values_count - 1)];
	return value;
}

HscTokenValue hsc_token_value_next(HscAstGen* astgen) {
	HscTokenValue value = astgen->token_values[HSC_MIN(astgen->token_value_read_idx, astgen->token_values_count - 1)];
	astgen->token_value_read_idx += 1;
	return value;
}

void hsc_astgen_ensure_semicolon(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_SEMICOLON) {
		hsc_astgen_error_1(astgen, "missing ';' to end the statement");
	}
	hsc_token_next(astgen);
}

bool hsc_astgen_generate_data_type(HscAstGen* astgen, HscDataType* type_out);
HscDataType hsc_astgen_generate_variable_decl_array(HscAstGen* astgen, HscDataType element_data_type);

void hsc_astgen_insert_global_declaration(HscAstGen* astgen, HscStringId identifier_string_id, HscDecl decl) {
	HscDecl* decl_ptr;
	if (hsc_hash_table_find_or_insert(&astgen->global_declarations, identifier_string_id.idx_plus_one, &decl_ptr)) {
		U32 other_token_idx = hsc_decl_token_idx(astgen, *decl_ptr);
		HscString string = hsc_string_table_get(&astgen->string_table, identifier_string_id);
		hsc_astgen_error_2(astgen, other_token_idx, "redefinition of the '%.*s' identifier", (int)string.size, string.data);
	}
	*decl_ptr = decl;
}

HscDataType hsc_astgen_generate_enum_data_type(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	HSC_DEBUG_ASSERT(token == HSC_TOKEN_KEYWORD_ENUM, "internal error: expected 'enum' but got '%s'", hsc_token_strings[token]);
	token = hsc_token_next(astgen);

	HscDataType data_type;
	HscStringId identifier_string_id = {0};
	HscEnumDataType* enum_data_type = NULL;
	if (token == HSC_TOKEN_IDENT) {
		token = hsc_token_next(astgen);
		identifier_string_id = hsc_token_value_next(astgen).string_id;

		HscDataType* insert_value_ptr;
		if (hsc_hash_table_find_or_insert(&astgen->enum_declarations, identifier_string_id.idx_plus_one, &insert_value_ptr)) {
			data_type = *insert_value_ptr;
			enum_data_type = hsc_enum_data_type_get(astgen, data_type);
		} else {
			*insert_value_ptr = HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_ENUM, astgen->enum_data_types_count);
			goto MAKE_NEW;
		}
	} else {
MAKE_NEW: {}
		U32 enum_data_type_idx = astgen->enum_data_types_count;
		HSC_ASSERT_ARRAY_BOUNDS(astgen->enum_data_types_count, astgen->enum_data_types_cap);
		astgen->enum_data_types_count += 1;

		enum_data_type = &astgen->enum_data_types[enum_data_type_idx];
		memset(enum_data_type, 0x0, sizeof(*enum_data_type));
		enum_data_type->identifier_token_idx = astgen->token_read_idx;
		enum_data_type->identifier_string_id = identifier_string_id;

		data_type = HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_ENUM, enum_data_type_idx);
	}

	if (token != HSC_TOKEN_CURLY_OPEN) {
		if (identifier_string_id.idx_plus_one) {
			return data_type;
		}
		hsc_astgen_error_1(astgen, "expected '{' to declare enum type values");
	}

	if (enum_data_type->values_count) {
		HscString data_type_name = hsc_data_type_string(astgen, data_type);
		astgen->token_read_idx -= 1;
		hsc_astgen_error_2(astgen, enum_data_type->identifier_token_idx, "redefinition of '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	token = hsc_token_next(astgen);
	enum_data_type->identifier_token_idx = astgen->token_read_idx - 2;
	enum_data_type->values_start_idx = astgen->enum_values_count;

	HscEnumValue* values = &astgen->enum_values[enum_data_type->values_start_idx];

	if (token == HSC_TOKEN_CURLY_CLOSE) {
		hsc_astgen_error_1(astgen, "cannot have an empty enum, please declare some identifiers inside the {}");
	}

	U32 value_idx = 0;
	S64 next_value = 0;
	while (token != HSC_TOKEN_CURLY_CLOSE) {
		HscEnumValue* enum_value = &values[value_idx];

		if (token != HSC_TOKEN_IDENT) {
			hsc_astgen_error_1(astgen, "expected an identifier for the enum value name");
		}

		if (next_value > S32_MAX) {
			hsc_astgen_error_1(astgen, "enum value overflows a 32 bit signed integer");
		}

		HscStringId value_identifier_string_id = hsc_token_value_next(astgen).string_id;
		enum_value->identifier_token_idx = astgen->token_read_idx;
		enum_value->identifier_string_id = value_identifier_string_id;

		HscDecl decl = HSC_DECL_INIT(HSC_DECL_ENUM_VALUE, enum_data_type->values_start_idx + value_idx);
		hsc_astgen_insert_global_declaration(astgen, value_identifier_string_id, decl);

		token = hsc_token_next(astgen);
		bool has_explicit_value = token == HSC_TOKEN_EQUAL;
		if (has_explicit_value) {
			token = hsc_token_next(astgen);

			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			if (expr->type != HSC_EXPR_TYPE_CONSTANT || expr->data_type == HSC_DATA_TYPE_U64 || !HSC_DATA_TYPE_IS_INT(expr->data_type)) {
				hsc_astgen_error_1(astgen, "expected a constant integer value");
			}

			HscConstantId value_constant_id = { .idx_plus_one = expr->constant.id };
			HscConstant constant = hsc_constant_table_get(&astgen->constant_table, value_constant_id);

			S64 value;
			HSC_DEBUG_ASSERT(hsc_constant_as_sint(constant, &value), "internal error: expected to be a signed int");

			if (value < S32_MIN || value > S32_MAX) {
				hsc_astgen_error_1(astgen, "expected a constant integer value that fits into signed 32 bits");
			}

			next_value = value;
			token = hsc_token_peek(astgen);
		}

		//
		// do not deduplicate when adding this constant to the constant table so we can pass in a debug name for the code generation to use for the enum value debug info
		S32 v = (S32)next_value;
		HscConstantId value_constant_id = _hsc_constant_table_deduplicate_end(&astgen->constant_table, HSC_DATA_TYPE_S32, &v, sizeof(S32), sizeof(S32), enum_value->identifier_string_id);

		enum_value->value_constant_id = value_constant_id;
		next_value += 1;

		if (token == HSC_TOKEN_COMMA) {
			token = hsc_token_next(astgen);
		} else if (token != HSC_TOKEN_CURLY_CLOSE) {
			char* message = has_explicit_value
				? "expected an '=' to assign a value explicitly, ',' to declare another value or a '}' to finish the enum values"
				: "expected a ',' to declare another value or a '}' to finish the enum values";
			hsc_astgen_error_1(astgen, message);
		}

		value_idx += 1;
		HSC_ASSERT_ARRAY_BOUNDS(astgen->enum_values_count, astgen->enum_values_cap);
		astgen->enum_values_count += 1;
	}

	enum_data_type->values_count = value_idx;

	token = hsc_token_next(astgen);
	hsc_found_data_type(astgen, data_type);
	return data_type;
}

HscDataType hsc_astgen_generate_compound_data_type(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	bool is_union = false;
	switch (token) {
		case HSC_TOKEN_KEYWORD_STRUCT: break;
		case HSC_TOKEN_KEYWORD_UNION:
			is_union = true;
			break;
		default:
			HSC_UNREACHABLE("internal error: expected 'struct' or 'union' but got '%s'", hsc_token_strings[token]);
	}
	token = hsc_token_next(astgen);

	HscDataType data_type;
	HscStringId identifier_string_id = {0};
	HscCompoundDataType* compound_data_type = NULL;
	if (token == HSC_TOKEN_IDENT) {
		token = hsc_token_next(astgen);
		identifier_string_id = hsc_token_value_next(astgen).string_id;
		HscDataType* insert_value_ptr;
		HscHashTable(HscStringId, HscDataType)* declarations;
		if (is_union) {
			declarations = &astgen->union_declarations;
		} else {
			declarations = &astgen->struct_declarations;
		}

		if (hsc_hash_table_find_or_insert(declarations, identifier_string_id.idx_plus_one, &insert_value_ptr)) {
			data_type = *insert_value_ptr;
			compound_data_type = hsc_compound_data_type_get(astgen, data_type);
		} else {
			*insert_value_ptr = HSC_DATA_TYPE_INIT(is_union ? HSC_DATA_TYPE_UNION : HSC_DATA_TYPE_STRUCT, astgen->compound_data_types_count);
			goto MAKE_NEW;
		}
	} else {
MAKE_NEW: {}
		U32 compound_data_type_idx = astgen->compound_data_types_count;
		HSC_ASSERT_ARRAY_BOUNDS(astgen->compound_data_types_count, astgen->compound_data_types_cap);
		astgen->compound_data_types_count += 1;

		compound_data_type = &astgen->compound_data_types[compound_data_type_idx];
		memset(compound_data_type, 0x0, sizeof(*compound_data_type));
		compound_data_type->identifier_token_idx = astgen->token_read_idx;
		compound_data_type->identifier_string_id = identifier_string_id;
		if (is_union) {
			compound_data_type->flags |= HSC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION;
			data_type = HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_UNION, compound_data_type_idx);
		} else {
			data_type = HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_STRUCT, compound_data_type_idx);
		}
	}

	if (token != HSC_TOKEN_CURLY_OPEN) {
		if (identifier_string_id.idx_plus_one) {
			return data_type;
		}
		hsc_astgen_error_1(astgen, "expected '{' to declare compound type fields");
	}

	if (compound_data_type->fields_count) {
		HscString data_type_name = hsc_data_type_string(astgen, data_type);
		astgen->token_read_idx -= 1;
		hsc_astgen_error_2(astgen, compound_data_type->identifier_token_idx, "redefinition of '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	token = hsc_token_next(astgen);
	compound_data_type->identifier_token_idx = astgen->token_read_idx - 2;
	compound_data_type->fields_start_idx = astgen->compound_fields_count;

	//
	// scan ahead an count how many fields we are going to have.
	U32 ahead_by = 0;
	U32 curly_open = 0; // to avoid counting the comma operator
	while (1) {
		HscToken token = hsc_token_peek_ahead(astgen, ahead_by);
		switch (token) {
			case HSC_TOKEN_EOF:
				goto END_FIELDS_COUNT;
			case HSC_TOKEN_SEMICOLON:
				if (curly_open == 0) {
					compound_data_type->fields_count += 1;
				}
				break;
			case HSC_TOKEN_CURLY_OPEN:
				curly_open += 1;
				break;
			case HSC_TOKEN_CURLY_CLOSE:
				if (curly_open == 0) {
					goto END_FIELDS_COUNT;
				}
				curly_open -= 1;
				break;
		}
		ahead_by += 1;
	}
END_FIELDS_COUNT: {}

	HSC_ASSERT_ARRAY_BOUNDS(compound_data_type->fields_count && astgen->compound_fields_count + compound_data_type->fields_count - 1, astgen->compound_fields_cap);
	HscCompoundField* fields = &astgen->compound_fields[compound_data_type->fields_start_idx];
	astgen->compound_fields_count += compound_data_type->fields_count;

	U32 field_idx = 0;
	while (1) {
		HscCompoundField* compound_field = &fields[field_idx];
		bool requires_name;
		switch (token) {
			case HSC_TOKEN_CURLY_CLOSE:
				goto END;
			case HSC_TOKEN_KEYWORD_STRUCT:
			case HSC_TOKEN_KEYWORD_UNION: {
				compound_field->data_type = hsc_astgen_generate_compound_data_type(astgen);
				requires_name = false;
				break;
			};
			case HSC_TOKEN_KEYWORD_ENUM: {
				compound_field->data_type = hsc_astgen_generate_enum_data_type(astgen);
				requires_name = true;
				break;
			};
			default: {
				if (!hsc_astgen_generate_data_type(astgen, &compound_field->data_type)) {
					hsc_astgen_error_1(astgen, "expected 'type name', 'struct' or 'union' to declare another field or '}' to finish declaring the compound type fields");
				}
				requires_name = true;
				break;
			};
		}

		token = hsc_token_peek(astgen);
		if (token != HSC_TOKEN_IDENT) {
			if (requires_name) {
				hsc_astgen_error_1(astgen, "expected an identifier for the field name");
			}

			compound_field->identifier_token_idx = 0;
			compound_field->identifier_string_id.idx_plus_one = 0;
		} else {
			HscStringId field_identifier_string_id = hsc_token_value_next(astgen).string_id;
			compound_field->identifier_token_idx = astgen->token_read_idx;
			compound_field->identifier_string_id = field_identifier_string_id;

			token = hsc_token_next(astgen);
			if (token == HSC_TOKEN_SQUARE_OPEN) {
				compound_field->data_type = hsc_astgen_generate_variable_decl_array(astgen, compound_field->data_type);
			}
		}
		hsc_astgen_ensure_semicolon(astgen);
		token = hsc_token_peek(astgen);

		Uptr size;
		Uptr align;
		hsc_data_type_size_align(astgen, compound_field->data_type, &size, &align);
		if (is_union) {
			if (compound_data_type->size < size) {
				compound_data_type->largest_sized_field_idx = field_idx;
				compound_data_type->size = size;
			}
		} else {
			compound_data_type->size = HSC_INT_ROUND_UP_ALIGN(compound_data_type->size, align) + size;
		}
		compound_data_type->align = HSC_MAX(compound_data_type->align, align);
		field_idx += 1;
	}

END:{}
	hsc_hash_table_clear(&astgen->field_name_to_token_idx);
	_hsc_compound_data_type_validate_field_names(astgen, data_type, compound_data_type);

	token = hsc_token_next(astgen);
	if (!is_union) {
		compound_data_type->size = HSC_INT_ROUND_UP_ALIGN(compound_data_type->size, compound_data_type->align);
	}
	hsc_found_data_type(astgen, data_type);
	return data_type;
}

bool hsc_astgen_generate_data_type(HscAstGen* astgen, HscDataType* data_type_out) {
	HscToken token = hsc_token_peek(astgen);
	if (HSC_TOKEN_IS_BASIC_TYPE(token)) {
		*data_type_out = (HscDataType)token;
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
					*data_type_out = HSC_DATA_TYPE_VEC2(HSC_DATA_TYPE_F32);
					break;
				case HSC_TOKEN_INTRINSIC_TYPE_VEC3:
					*data_type_out = HSC_DATA_TYPE_VEC3(HSC_DATA_TYPE_F32);
					break;
				case HSC_TOKEN_INTRINSIC_TYPE_VEC4:
					*data_type_out = HSC_DATA_TYPE_VEC4(HSC_DATA_TYPE_F32);
					break;
			}
			hsc_token_consume(astgen, 1);
			return true;
		case HSC_TOKEN_KEYWORD_STRUCT:
		case HSC_TOKEN_KEYWORD_UNION:
			*data_type_out = hsc_astgen_generate_compound_data_type(astgen);
			return true;
		case HSC_TOKEN_KEYWORD_ENUM:
			*data_type_out = hsc_astgen_generate_enum_data_type(astgen);
			return true;
		case HSC_TOKEN_IDENT: {
			HscDecl decl;
			HscStringId identifier_string_id = hsc_token_value_next(astgen).string_id;
			if (hsc_hash_table_find(&astgen->global_declarations, identifier_string_id.idx_plus_one, &decl)) {
				if (HSC_DECL_IS_DATA_TYPE(decl)) {
					*data_type_out = decl;
					hsc_token_consume(astgen, 1);
					return true;
				}
			}
			break;
		};
	}

	return false;
}

HscDataType hsc_astgen_generate_typedef(HscAstGen* astgen) {
	HSC_DEBUG_ASSERT(hsc_token_peek(astgen) == HSC_TOKEN_KEYWORD_TYPEDEF, "internal error: expected a typedef token");
	hsc_token_consume(astgen, 1);
	HscDataType aliased_data_type;
	if (!hsc_astgen_generate_data_type(astgen, &aliased_data_type)) {
		HscToken token = hsc_token_peek(astgen);
		hsc_astgen_error_1(astgen, "expected a 'type name' here but got '%s'", hsc_token_strings[token]);
	}

	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_IDENT) {
		hsc_astgen_error_1(astgen, "expected an 'identifier' for the typedef here but got '%s'", hsc_token_strings[token]);
	}
	HscStringId identifier_string_id = hsc_token_value_next(astgen).string_id;

	HscDataType* insert_value_ptr;
	HscTypedef* typedef_ = NULL;
	HscDataType data_type;
	if (hsc_hash_table_find_or_insert(&astgen->global_declarations, identifier_string_id.idx_plus_one, &insert_value_ptr)) {
		data_type = *insert_value_ptr;
		typedef_ = hsc_typedef_get(astgen, data_type);
		if (typedef_->aliased_data_type != aliased_data_type) {
			HscString data_type_name = hsc_string_table_get(&astgen->string_table, identifier_string_id);
			hsc_astgen_error_2(astgen, typedef_->identifier_token_idx, "redefinition of typename '%.*s'", (int)data_type_name.size, data_type_name.data);
		}
	} else {
		data_type = HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_TYPEDEF, astgen->typedefs_count);
		HSC_ASSERT_ARRAY_BOUNDS(astgen->typedefs_count, astgen->typedefs_cap);
		typedef_ = &astgen->typedefs[astgen->typedefs_count];
		typedef_->identifier_token_idx = astgen->token_read_idx;
		typedef_->identifier_string_id = identifier_string_id;
		typedef_->aliased_data_type = aliased_data_type;
		astgen->typedefs_count += 1;

		hsc_found_data_type(astgen, data_type);
		*insert_value_ptr = data_type;
	}

	hsc_token_consume(astgen, 1);
	hsc_astgen_ensure_semicolon(astgen);
	return data_type;
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

static U8 hsc_data_type_basic_type_ranks[HSC_DATA_TYPE_BASIC_COUNT] = {
	[HSC_DATA_TYPE_BOOL] = 1,
	[HSC_DATA_TYPE_U8] = 2,
	[HSC_DATA_TYPE_S8] = 2,
	[HSC_DATA_TYPE_U16] = 3,
	[HSC_DATA_TYPE_S16] = 3,
	[HSC_DATA_TYPE_S32] = 4,
	[HSC_DATA_TYPE_U32] = 4,
	[HSC_DATA_TYPE_U64] = 5,
	[HSC_DATA_TYPE_S64] = 5,
	[HSC_DATA_TYPE_F16] = 6,
	[HSC_DATA_TYPE_F32] = 7,
	[HSC_DATA_TYPE_F64] = 8,
};

#define HSC_DEBUG_ASSERT_EVAL(expr) HSC_DEBUG_ASSERT(expr->type == HSC_EXPR_TYPE_CONSTANT, "internal error: expected to be evaluating a constant")

void hsc_astgen_eval_cast(HscAstGen* astgen, HscExpr* expr, HscDataType dst_data_type) {
	HSC_DEBUG_ASSERT_EVAL(expr);
	HscConstantId constant_id = { .idx_plus_one = expr->constant.id };
	HscConstant constant = hsc_constant_table_get(&astgen->constant_table, constant_id);

	union {
		_Bool bool_;
		U64 uint;
		S8 s8;
		S16 s16;
		S32 s32;
		S64 s64;
		F32 f32;
		F64 f64;
	} dst;

	if (HSC_DATA_TYPE_IS_UINT(expr->data_type)) {
		U64 uint;
		hsc_constant_as_uint(constant, &uint);

		switch (dst_data_type) {
			case HSC_DATA_TYPE_BOOL: dst.bool_ = uint; break;
			case HSC_DATA_TYPE_U8:
			case HSC_DATA_TYPE_U16:
			case HSC_DATA_TYPE_U32:
			case HSC_DATA_TYPE_U64:
				dst.uint = uint;
				break;
			case HSC_DATA_TYPE_S8: dst.s8 = uint; break;
			case HSC_DATA_TYPE_S16: dst.s16 = uint; break;
			case HSC_DATA_TYPE_S32: dst.s32 = uint; break;
			case HSC_DATA_TYPE_S64: dst.s64 = uint; break;
			case HSC_DATA_TYPE_F32: dst.f32 = uint; break;
			case HSC_DATA_TYPE_F64: dst.f64 = uint; break;
			default:
				HSC_ABORT("unhandled data type '%u'", dst_data_type);
		}
	} else if (HSC_DATA_TYPE_IS_SINT(expr->data_type)) {
		S64 sint;
		hsc_constant_as_sint(constant, &sint);

		switch (dst_data_type) {
			case HSC_DATA_TYPE_BOOL: dst.bool_ = sint; break;
			case HSC_DATA_TYPE_U8:
			case HSC_DATA_TYPE_U16:
			case HSC_DATA_TYPE_U32:
			case HSC_DATA_TYPE_U64:
				dst.uint = sint;
				break;
			case HSC_DATA_TYPE_S8: dst.s8 = sint; break;
			case HSC_DATA_TYPE_S16: dst.s16 = sint; break;
			case HSC_DATA_TYPE_S32: dst.s32 = sint; break;
			case HSC_DATA_TYPE_S64: dst.s64 = sint; break;
			case HSC_DATA_TYPE_F32: dst.f32 = sint; break;
			case HSC_DATA_TYPE_F64: dst.f64 = sint; break;
			default:
				HSC_ABORT("unhandled data type '%u'", dst_data_type);
		}
	} else if (HSC_DATA_TYPE_IS_FLOAT(expr->data_type)) {
		F64 float_;
		hsc_constant_as_float(constant, &float_);

		switch (dst_data_type) {
			case HSC_DATA_TYPE_BOOL: dst.bool_ = (_Bool)float_; break;
			case HSC_DATA_TYPE_U8:
			case HSC_DATA_TYPE_U16:
			case HSC_DATA_TYPE_U32:
			case HSC_DATA_TYPE_U64:
				dst.uint = (U64)float_;
				break;
			case HSC_DATA_TYPE_S8: dst.s8 = (S8)float_; break;
			case HSC_DATA_TYPE_S16: dst.s16 = (S16)float_; break;
			case HSC_DATA_TYPE_S32: dst.s32 = (S32)float_; break;
			case HSC_DATA_TYPE_S64: dst.s64 = (S64)float_; break;
			case HSC_DATA_TYPE_F32: dst.f32 = float_; break;
			case HSC_DATA_TYPE_F64: dst.f64 = float_; break;
			default:
				HSC_ABORT("unhandled data type '%u'", dst_data_type);
		}
	} else {
		HSC_ABORT("unhandled data type '%u'", expr->data_type);
	}

	constant_id = hsc_constant_table_deduplicate_basic(&astgen->constant_table, astgen, dst_data_type, &dst.uint);
	expr->constant.id = constant_id.idx_plus_one;
	expr->data_type = dst_data_type;
}

void hsc_astgen_implicit_cast(HscAstGen* astgen, HscDataType dst_data_type, HscExpr** expr_mut) {
	HscExpr* expr = *expr_mut;
	if (expr->type == HSC_EXPR_TYPE_CONSTANT) {
		hsc_astgen_eval_cast(astgen, expr, dst_data_type);
		return;
	}

	HscExpr* cast_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CAST);
	cast_expr->unary.expr_rel_idx = cast_expr - expr;
	cast_expr->data_type = dst_data_type;
	*expr_mut = cast_expr;
}

bool hsc_data_type_check_compatible_assignment(HscAstGen* astgen, HscDataType target_data_type, HscExpr** source_expr_mut) {
	HscExpr* source_expr = *source_expr_mut;
	HscDataType source_data_type = source_expr->data_type;
	if (HSC_DATA_TYPE_IS_CONST(target_data_type) && !HSC_DATA_TYPE_IS_CONST(source_data_type)) {
		return false;
	}

	target_data_type = hsc_typedef_resolve(astgen, target_data_type);
	source_data_type = hsc_typedef_resolve(astgen, source_data_type);
	target_data_type = HSC_DATA_TYPE_STRIP_CONST(target_data_type);
	source_data_type = HSC_DATA_TYPE_STRIP_CONST(source_data_type);
	if (target_data_type == source_data_type) {
		return true;
	}

	if (HSC_DATA_TYPE_IS_BASIC(target_data_type) && HSC_DATA_TYPE_IS_BASIC(source_data_type)) {
		hsc_astgen_implicit_cast(astgen, target_data_type, source_expr_mut);
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

void hsc_data_type_ensure_compatible_assignment(HscAstGen* astgen, U32 other_token_idx, HscDataType target_data_type, HscExpr** source_expr_mut) {
	if (!hsc_data_type_check_compatible_assignment(astgen, target_data_type, source_expr_mut)) {
		HscString target_data_type_name = hsc_data_type_string(astgen, target_data_type);
		HscString source_data_type_name = hsc_data_type_string(astgen, (*source_expr_mut)->data_type);
		hsc_astgen_error_2(astgen, other_token_idx, "type mismatch '%.*s' is does not implicitly cast to '%.*s'", (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
	}
}

bool hsc_data_type_check_compatible_arithmetic(HscAstGen* astgen, HscExpr** left_expr_mut, HscExpr** right_expr_mut) {
	HscExpr* left_expr = *left_expr_mut;
	HscExpr* right_expr = *right_expr_mut;

	HscDataType left_data_type = hsc_typedef_resolve(astgen, left_expr->data_type);
	HscDataType right_data_type = hsc_typedef_resolve(astgen, right_expr->data_type);
	left_data_type = HSC_DATA_TYPE_STRIP_CONST(left_data_type);
	right_data_type = HSC_DATA_TYPE_STRIP_CONST(right_data_type);
	if (left_data_type == right_data_type) {
		return true;
	}

	if (HSC_DATA_TYPE_IS_BASIC(left_data_type) && HSC_DATA_TYPE_IS_BASIC(right_data_type)) {
		bool left_is_float = HSC_DATA_TYPE_IS_FLOAT(left_data_type);
		bool right_is_float = HSC_DATA_TYPE_IS_FLOAT(right_data_type);
		U8 left_rank = hsc_data_type_basic_type_ranks[left_data_type];
		U8 right_rank = hsc_data_type_basic_type_ranks[right_data_type];
		if (left_is_float || right_is_float) {
			//
			// if one of operands is a float, then cast lower ranked operand
			// into the type of the higher ranked operand
			if (left_rank < right_rank) {
				hsc_astgen_implicit_cast(astgen, right_data_type, left_expr_mut);
			} else if (left_rank > right_rank) {
				hsc_astgen_implicit_cast(astgen, left_data_type, right_expr_mut);
			}
		} else {
			//
			// both operands are integers
			//

			{
				//
				// promote each operand to an int if it has a lower rank
				//

				U8 int_rank = hsc_data_type_basic_type_ranks[HSC_DATA_TYPE_S32];
				if (left_rank < int_rank) {
					hsc_astgen_implicit_cast(astgen, HSC_DATA_TYPE_S32, left_expr_mut);
					left_data_type = HSC_DATA_TYPE_S32;
					left_rank = int_rank;
				}

				if (right_rank < int_rank) {
					hsc_astgen_implicit_cast(astgen, HSC_DATA_TYPE_S32, right_expr_mut);
					right_data_type = HSC_DATA_TYPE_S32;
					right_rank = int_rank;
				}
			}

			if (left_data_type != right_data_type) {
				bool left_is_unsigned = HSC_DATA_TYPE_IS_UINT(left_data_type);
				bool right_is_unsigned = HSC_DATA_TYPE_IS_UINT(right_data_type);
				if (left_is_unsigned || right_is_unsigned) {
					//
					// one of the operands is unsigned, convert the other operand
					// into the unsigned data type if it's rank if less than or
					// equal to the unsigned's data type rank.
					//

					if (!left_is_unsigned && left_rank <= right_rank) {
						hsc_astgen_implicit_cast(astgen, right_data_type, left_expr_mut);
						return true;
					} else if (!right_is_unsigned && left_rank >= right_rank) {
						hsc_astgen_implicit_cast(astgen, left_data_type, right_expr_mut);
						return true;
					}
				}

				bool left_is_signed = !left_is_unsigned;
				bool right_is_signed = !right_is_unsigned;
				if ((left_is_signed || right_is_signed)) {
					//
					// one of the operands is signed, convert the other operand
					// into the signed data type if it's rank if less than or
					// equal to the signed's data type rank.
					//

					if (!left_is_signed && left_rank <= right_rank) {
						hsc_astgen_implicit_cast(astgen, right_data_type, left_expr_mut);
					} else if (!right_is_signed && left_rank >= right_rank) {
						hsc_astgen_implicit_cast(astgen, left_data_type, right_expr_mut);
					}
				}
			}
		}

		return true;
	}

	return false;
}

void hsc_data_type_ensure_compatible_arithmetic(HscAstGen* astgen, U32 other_token_idx, HscExpr** left_expr_mut, HscExpr** right_expr_mut, HscToken operator_token) {
	if (!hsc_data_type_check_compatible_arithmetic(astgen, left_expr_mut, right_expr_mut)) {
		HscString left_data_type_name = hsc_data_type_string(astgen, (*left_expr_mut)->data_type);
		HscString right_data_type_name = hsc_data_type_string(astgen, (*right_expr_mut)->data_type);
		hsc_astgen_error_2(astgen, other_token_idx, "operator '%s' is not supported for data type '%.*s' and '%.*s'", hsc_token_strings[operator_token], (int)right_data_type_name.size, right_data_type_name.data, (int)left_data_type_name.size, left_data_type_name.data);
	}
}

void hsc_curly_initializer_gen_init_composite(HscAstGen* astgen, HscDataType data_type, bool add_null_entry) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;

	if (add_null_entry) {
		HSC_ASSERT_ARRAY_BOUNDS(gen->entry_indices_count, gen->entry_indices_cap);
		gen->entry_indices[gen->entry_indices_count] = -1;
		gen->data_types[gen->entry_indices_count] = data_type;
		gen->found_designators[gen->entry_indices_count] = false;
		gen->entry_indices_count += 1;
	}

	gen->composite_data_type = data_type;
	gen->resolved_composite_data_type = hsc_typedef_resolve(astgen, gen->composite_data_type);
	if (HSC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
		gen->array_data_type = hsc_array_data_type_get(astgen, gen->resolved_composite_data_type);

		HscConstant constant = hsc_constant_table_get(&astgen->constant_table, gen->array_data_type->size_constant_id);
		U64 cap;
		hsc_constant_as_uint(constant, &cap);

		gen->entry_data_type = gen->array_data_type->element_data_type;
		gen->resolved_entry_data_type = hsc_typedef_resolve(astgen, gen->entry_data_type);
		gen->entries_cap = cap;
	} else {
		gen->compound_data_type = hsc_compound_data_type_get(astgen, gen->resolved_composite_data_type);
		gen->compound_fields = &astgen->compound_fields[gen->compound_data_type->fields_start_idx];

		gen->entries_cap = HSC_DATA_TYPE_IS_UNION(gen->resolved_composite_data_type) ? 1 : gen->compound_data_type->fields_count;
	}
}

void hsc_curly_initializer_gen_init(HscAstGen* astgen, HscDataType data_type, HscExpr* first_expr) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	gen->entry_indices_count = 0;
	gen->entry_data_type = HSC_DATA_TYPE_VOID;
	hsc_curly_initializer_gen_init_composite(astgen, data_type, true);
	gen->prev_initializer_expr = first_expr;
	gen->first_initializer_expr = first_expr;
}

void hsc_curly_initializer_gen_entry_next(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	U64* entry_idx = &gen->entry_indices[gen->entry_indices_count - 1];
	*entry_idx += 1;

	HscToken token = hsc_token_peek(astgen);
	if (*entry_idx >= gen->entries_cap && token != HSC_TOKEN_FULL_STOP && token != HSC_TOKEN_SQUARE_OPEN) {
		HscString data_type_name = hsc_data_type_string(astgen, gen->composite_data_type);
		hsc_astgen_error_1(astgen, "we have reached the end of members for the '%.*s' type", (int)data_type_name.size, data_type_name.data);
	}

	if (!HSC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
		gen->entry_data_type = gen->compound_fields[*entry_idx].data_type;
		gen->resolved_entry_data_type = hsc_typedef_resolve(astgen, gen->entry_data_type);
	}
}

void hsc_curly_initializer_gen_initializer_open(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	if (!HSC_DATA_TYPE_IS_COMPOSITE_TYPE(gen->resolved_entry_data_type)) {
		HscString data_type_name = hsc_data_type_string(astgen, gen->entry_data_type);
		hsc_astgen_error_1(astgen, "'{' can only be used for structure or array types but got '%.*s'", (int)data_type_name.size, data_type_name.data);
	}

	hsc_curly_initializer_gen_init_composite(astgen, gen->entry_data_type, true);
}

HscToken hsc_curly_initializer_gen_designator_entry_indices(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	HscToken token = hsc_token_peek(astgen);

	HSC_ASSERT_ARRAY_BOUNDS(gen->nested_designators_count, gen->nested_designators_cap);
	gen->nested_designators_start_entry_indices[gen->nested_designators_count] = gen->entry_indices_count;
	gen->nested_designators_count += 1;

	U32 entry_start_idx = gen->entry_indices_count - 1;
	gen->entry_indices_count -= 1;

	HSC_DEBUG_ASSERT(token == HSC_TOKEN_FULL_STOP || token == HSC_TOKEN_SQUARE_OPEN, "internal error: expected '.' or '['");

	while (1) {
		switch (token) {
			case HSC_TOKEN_FULL_STOP:
				if (HSC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					HscString data_type_name = hsc_data_type_string(astgen, gen->composite_data_type);
					hsc_astgen_error_1(astgen, "field designator cannot be used for an the '%.*s' array type, please use '[' instead", (int)data_type_name.size, data_type_name.data);
				}

				token = hsc_token_next(astgen);
				if (token != HSC_TOKEN_IDENT) {
					HscString data_type_name = hsc_data_type_string(astgen, gen->composite_data_type);
					hsc_astgen_error_1(astgen, "expected an the field identifier that you wish to initialize from '%.*s'", (int)data_type_name.size, data_type_name.data);
				}


				HscStringId identifier_string_id = hsc_token_value_next(astgen).string_id;
				hsc_compound_data_type_find_field_by_name_checked(astgen, gen->composite_data_type, gen->compound_data_type, identifier_string_id);
				for (U32 i = 0; i < astgen->compound_type_find_fields_count; i += 1) {
					U32 entry_idx = gen->entry_indices_count;
					HSC_ASSERT_ARRAY_BOUNDS(entry_idx + 1, astgen->entry_indices_cap);
					gen->entry_indices[entry_idx] = astgen->compound_type_find_fields[i].idx;
					gen->data_types[entry_idx + 1] = astgen->compound_type_find_fields[i].data_type;
					gen->entry_indices_count += 1;
				}

				gen->entry_data_type = astgen->compound_type_find_fields[astgen->compound_type_find_fields_count - 1].data_type;
				gen->resolved_entry_data_type = hsc_typedef_resolve(astgen, gen->entry_data_type);
				token = hsc_token_next(astgen);
				break;
			case HSC_TOKEN_SQUARE_OPEN:
				if (!HSC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					HscString data_type_name = hsc_data_type_string(astgen, gen->composite_data_type);
					hsc_astgen_error_1(astgen, "array designator cannot be used for an the '%.*s' compound type, please use '.' instead", (int)data_type_name.size, data_type_name.data);
				}

				token = hsc_token_next(astgen);
				HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
				if (expr->type != HSC_EXPR_TYPE_CONSTANT || !HSC_DATA_TYPE_IS_INT(expr->data_type)) {
					hsc_astgen_error_1(astgen, "expected a constant integer value");
				}

				HscConstantId value_constant_id = { .idx_plus_one = expr->constant.id };
				HscConstant constant = hsc_constant_table_get(&astgen->constant_table, value_constant_id);

				U64 elmt_idx;
				if (!hsc_constant_as_uint(constant, &elmt_idx)) {
					hsc_astgen_error_1(astgen, "expected a constant unsigned integer value");
				}

				token = hsc_token_peek(astgen);
				if (token != HSC_TOKEN_SQUARE_CLOSE) {
					hsc_astgen_error_1(astgen, "expected ']' to finish the array designator");
				}
				token = hsc_token_next(astgen);

				U32 entry_idx = gen->entry_indices_count;
				HSC_ASSERT_ARRAY_BOUNDS(entry_idx + 1, astgen->entry_indices_cap);
				gen->entry_indices[entry_idx] = elmt_idx;
				gen->data_types[entry_idx + 1] = gen->array_data_type->element_data_type;
				gen->entry_indices_count += 1;
				break;
			case HSC_TOKEN_EQUAL:
				goto END;
			default: {
				const char* message;
				if (HSC_DATA_TYPE_IS_ARRAY(gen->resolved_composite_data_type)) {
					message = "expected an '=' to assign a value or a '[' for an array designator";
				} else {
					message = "expected an '=' to assign a value or a '.' for an field designator";
				}
				hsc_astgen_error_1(astgen, message);
			};
		}

		if (token == HSC_TOKEN_EQUAL) {
			goto END;
		} else if (!HSC_DATA_TYPE_IS_COMPOSITE_TYPE(gen->resolved_entry_data_type)) {
			hsc_astgen_error_1(astgen, "expected an '=' to assign a value");
		}

		hsc_curly_initializer_gen_init_composite(astgen, gen->entry_data_type, false);
	}
END: {}
	token = hsc_token_next(astgen);

	for (U32 i = entry_start_idx; i < gen->entry_indices_count; i += 1) {
		gen->found_designators[i] = true;
	}

	return token;
}

void hsc_curly_initializer_gen_designator_undo_entry_indices(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	if (!gen->found_designators[gen->entry_indices_count - 1]) {
		return;
	}

	HSC_DEBUG_ASSERT(gen->nested_designators_count, "internal error: there are no more nested designators");

	gen->nested_designators_count -= 1;
	gen->entry_indices_count = gen->nested_designators_start_entry_indices[gen->nested_designators_count];
	HscDataType composite_data_type = gen->data_types[gen->entry_indices_count - 1];
	hsc_curly_initializer_gen_init_composite(astgen, composite_data_type, false);
}

HscToken hsc_curly_initializer_gen_initializer_close(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	gen->entry_indices_count -= 1;
	HscDataType composite_data_type = gen->data_types[gen->entry_indices_count - 1];
	hsc_curly_initializer_gen_init_composite(astgen, composite_data_type, false);
	hsc_curly_initializer_gen_designator_undo_entry_indices(astgen);
	return hsc_token_next(astgen);
}

HscExpr* hsc_curly_initiaizer_generate_designated_initializer(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;

	HscExpr* initializer_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_DESIGNATED_INITIALIZER);
	U64* entry_indices = &astgen->entry_indices[astgen->entry_indices_count];
	U32 entry_indices_count = astgen->curly_initializer_gen.entry_indices_count;
	initializer_expr->designated_initializer.entry_indices_count = entry_indices_count;
	initializer_expr->alt_next_expr_rel_idx = astgen->entry_indices_count;
	HSC_ASSERT_ARRAY_BOUNDS(astgen->entry_indices_count + entry_indices_count - 1, astgen->entry_indices_cap);
	HSC_COPY_ELMT_MANY(entry_indices, astgen->curly_initializer_gen.entry_indices, entry_indices_count);
	astgen->entry_indices_count += entry_indices_count;

	initializer_expr->is_stmt_block_entry = true;
	initializer_expr->next_expr_rel_idx = 0;

	if (gen->prev_initializer_expr) {
		gen->prev_initializer_expr->next_expr_rel_idx = initializer_expr - gen->prev_initializer_expr;
	} else {
		gen->first_initializer_expr = initializer_expr;
	}
	gen->prev_initializer_expr = initializer_expr;

	return initializer_expr;
}

bool hsc_curly_initializer_gen_consume_if_zero(HscAstGen* astgen) {
	HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;
	bool consume = gen->entries_cap > 1
		&& hsc_token_peek(astgen) == HSC_TOKEN_LIT_S32
		&& hsc_token_peek_ahead(astgen, 1) == HSC_TOKEN_CURLY_CLOSE
		&& hsc_token_value_peek(astgen).constant_id.idx_plus_one == astgen->basic_type_zero_constant_ids[HSC_DATA_TYPE_S32].idx_plus_one
		;

	if (consume) {
		hsc_token_consume(astgen, 2);
		hsc_token_value_consume(astgen, 1);
		return true;
	}
	return false;
}

void hsc_used_static_variable(HscAstGen* astgen, HscDecl decl) {
	bool found = false;
	HscFunction* function = &astgen->functions[astgen->functions_count - 1];
	for (U32 idx = function->used_static_variables_start_idx; idx < astgen->used_static_variables_count; idx += 1) {
		if (astgen->used_static_variables[idx] == decl) {
			found = true;
			break;
		}
	}
	if (!found) {
		HSC_ASSERT_ARRAY_BOUNDS(astgen->used_static_variables_count, astgen->used_static_variables_cap);
		astgen->used_static_variables[astgen->used_static_variables_count] = decl;
		astgen->used_static_variables_count += 1;
	}
}

HscExpr* hsc_astgen_generate_unary_op(HscAstGen* astgen, HscExpr* inner_expr, HscUnaryOp unary_op, HscToken operator_token) {
	HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_UNARY_OP_START + unary_op);
	HscDataType resolved_data_type = hsc_typedef_resolve(astgen, inner_expr->data_type);

	if (!HSC_DATA_TYPE_IS_NON_VOID_BASIC(resolved_data_type)) {
		HscString data_type_name = hsc_data_type_string(astgen, inner_expr->data_type);
		hsc_astgen_error_1(astgen, "operator '%s' is not supported for the '%.*s' data type", hsc_token_strings[operator_token], (int)data_type_name.size, data_type_name.data);
	}

	if (unary_op != HSC_UNARY_OP_LOGICAL_NOT) {
		if (HSC_DATA_TYPE_IS_INT(resolved_data_type)) {
			U8 rank = hsc_data_type_basic_type_ranks[resolved_data_type];
			U8 int_rank = hsc_data_type_basic_type_ranks[HSC_DATA_TYPE_S32];
			if (rank < int_rank) {
				hsc_astgen_implicit_cast(astgen, HSC_DATA_TYPE_S32, &inner_expr);
			}
		}
	}

	expr->unary.expr_rel_idx = expr - inner_expr;
	expr->data_type = unary_op == HSC_UNARY_OP_LOGICAL_NOT ? HSC_DATA_TYPE_BOOL : inner_expr->data_type;

	return expr;
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
			switch (token) {
				case HSC_TOKEN_KEYWORD_TRUE:
					data_type = HSC_DATA_TYPE_BOOL;
					constant_id = astgen->basic_type_one_constant_ids[HSC_DATA_TYPE_BOOL];
					break;
				case HSC_TOKEN_KEYWORD_FALSE:
					data_type = HSC_DATA_TYPE_BOOL;
					constant_id = astgen->basic_type_zero_constant_ids[HSC_DATA_TYPE_BOOL];
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
				HscVariable* variable = &astgen->function_params_and_variables[function->params_start_idx + existing_variable_id - 1];

				HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_LOCAL_VARIABLE);
				expr->variable.idx = existing_variable_id - 1;
				expr->data_type = variable->data_type;

				if (variable->is_static) {
					hsc_used_static_variable(astgen, HSC_DECL_INIT(HSC_DECL_LOCAL_VARIABLE, existing_variable_id - 1));
				}
				return expr;
			}

			HscDecl decl;
			if (hsc_hash_table_find(&astgen->global_declarations, identifier_value.string_id.idx_plus_one, &decl)) {
				if (HSC_DECL_IS_DATA_TYPE(decl)) {
					HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_DATA_TYPE);
					expr->data_type = decl;
					return expr;
				} else if (HSC_DECL_IS_FUNCTION(decl)) {
					HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_FUNCTION);
					expr->function.idx = HSC_DECL_IDX(decl);
					return expr;
				} else if (HSC_DECL_IS_ENUM_VALUE(decl)) {
					HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CONSTANT);
					HscEnumValue* enum_value = hsc_enum_value_get(astgen, decl);
					expr->constant.id = enum_value->value_constant_id.idx_plus_one;
					expr->data_type = HSC_DATA_TYPE_S32;
					return expr;
				} else if (HSC_DECL_IS_GLOBAL_VARIABLE(decl)) {
					HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_GLOBAL_VARIABLE);
					HscVariable* variable = hsc_global_variable_get(astgen, decl);
					expr->variable.idx = HSC_DECL_IDX(decl);
					expr->data_type = variable->data_type;

					hsc_used_static_variable(astgen, decl);
					return expr;
				}

				HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
				U32 other_token_idx = hsc_decl_token_idx(astgen, decl);
				hsc_astgen_error_2(astgen, other_token_idx, "type '%.*s' cannot be used here", (int)string.size, string.data);
			}

			HscString string = hsc_string_table_get(&astgen->string_table, identifier_value.string_id);
			hsc_astgen_error_1(astgen, "undeclared identifier '%.*s'", (int)string.size, string.data);
		};
		case HSC_TOKEN_TILDE: unary_op = HSC_UNARY_OP_BIT_NOT; goto UNARY;
		case HSC_TOKEN_EXCLAMATION_MARK: unary_op = HSC_UNARY_OP_LOGICAL_NOT; goto UNARY;
		case HSC_TOKEN_PLUS: unary_op = HSC_UNARY_OP_PLUS; goto UNARY;
		case HSC_TOKEN_MINUS: unary_op = HSC_UNARY_OP_NEGATE; goto UNARY;
		case HSC_TOKEN_INCREMENT: unary_op = HSC_UNARY_OP_PRE_INCREMENT; goto UNARY;
		case HSC_TOKEN_DECREMENT: unary_op = HSC_UNARY_OP_PRE_DECREMENT; goto UNARY;
UNARY:
		{
			HscToken operator_token = token;
			hsc_token_consume(astgen, 1);

			HscExpr* inner_expr = hsc_astgen_generate_unary_expr(astgen);
			return hsc_astgen_generate_unary_op(astgen, inner_expr, unary_op, operator_token);
		};
		case HSC_TOKEN_PARENTHESIS_OPEN: {
			hsc_token_consume(astgen, 1);
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			HscToken token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
				hsc_astgen_error_1(astgen, "expected a ')' here to finish the expression");
			}
			token = hsc_token_next(astgen);

			if (expr->type == HSC_EXPR_TYPE_DATA_TYPE) {
				if (token == HSC_TOKEN_CURLY_OPEN) {
					//
					// found compound literal
					astgen->assign_data_type = expr->data_type;
					return hsc_astgen_generate_unary_expr(astgen);
				} else {
					HscExpr* right_expr = hsc_astgen_generate_expr(astgen, 2);
					if (expr->data_type != right_expr->data_type) {
						HscDataType resolved_cast_data_type = hsc_typedef_resolve(astgen, expr->data_type);
						HscDataType resolved_castee_data_type = hsc_typedef_resolve(astgen, right_expr->data_type);

						if (resolved_cast_data_type >= HSC_DATA_TYPE_VECTOR_END || resolved_castee_data_type >= HSC_DATA_TYPE_VECTOR_END) {
							HscString target_data_type_name = hsc_data_type_string(astgen, expr->data_type);
							HscString source_data_type_name = hsc_data_type_string(astgen, right_expr->data_type);
							hsc_astgen_error_1(astgen, "cannot cast '%.*s' to '%.*s'", (int)source_data_type_name.size, source_data_type_name.data, (int)target_data_type_name.size, target_data_type_name.data);
						}

						HscExpr* cast_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CAST);
						cast_expr->unary.expr_rel_idx = cast_expr - right_expr;
						cast_expr->data_type = expr->data_type;
						return cast_expr;
					}
					return right_expr;
				}
			}

			return expr;
		};
		case HSC_TOKEN_CURLY_OPEN: {
			HscDataType assign_data_type = astgen->assign_data_type;
			HscDataType resolved_assign_data_type = hsc_typedef_resolve(astgen, assign_data_type);
			astgen->assign_data_type = HSC_DATA_TYPE_VOID;
			HscCurlyInitializerGen* gen = &astgen->curly_initializer_gen;

			if (HSC_DATA_TYPE_IS_COMPOSITE_TYPE(resolved_assign_data_type)) {
				HscExpr* curly_initializer_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CURLY_INITIALIZER);
				curly_initializer_expr->data_type = assign_data_type;
				token = hsc_token_next(astgen);

				HscExpr* variable_expr;
				{
					HscVariable* variable = &astgen->function_params_and_variables[astgen->function_params_and_variables_count];
					astgen->function_params_and_variables_count += 1;
					variable->identifier_string_id.idx_plus_one = 0;
					variable->identifier_token_idx = 0;
					variable->data_type = assign_data_type;
					astgen->stmt_block->stmt_block.variables_count += 1;

					U32 variable_idx = astgen->next_var_idx;
					astgen->next_var_idx += 1;

					variable_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_LOCAL_VARIABLE);
					variable_expr->variable.idx = variable_idx;
					variable_expr->next_expr_rel_idx = 0;
				}

				hsc_curly_initializer_gen_init(astgen, resolved_assign_data_type, variable_expr);

				if (hsc_curly_initializer_gen_consume_if_zero(astgen)) {
					goto CURLY_INITIALIZER_END;
				}

				astgen->compound_type_find_fields_count = 0;
				U32 nested_count = 0;
				while (1) {
					if (!gen->found_designators[gen->entry_indices_count - 1]) {
						hsc_curly_initializer_gen_entry_next(astgen);
					}

					if (token == HSC_TOKEN_FULL_STOP || token == HSC_TOKEN_SQUARE_OPEN) {
						token = hsc_curly_initializer_gen_designator_entry_indices(astgen);
					} else if (gen->found_designators[gen->entry_indices_count - 1]) {
						hsc_astgen_error_1(astgen, "you must continue using field/array designators after they have been used");
					}

					if (token == HSC_TOKEN_CURLY_OPEN) {
						HscExpr* initializer_expr = hsc_curly_initiaizer_generate_designated_initializer(astgen);
						initializer_expr->designated_initializer.value_expr_rel_idx = 0;

						token = hsc_token_next(astgen);
						if (hsc_curly_initializer_gen_consume_if_zero(astgen)) {
							goto CURLY_INITIALIZER_AFTER_VALUE;
						} else {
							hsc_curly_initializer_gen_initializer_open(astgen);
							nested_count += 1;
							continue;
						}
					}

					HscExpr* initializer_expr = hsc_curly_initiaizer_generate_designated_initializer(astgen);

					HscExpr* value_expr = hsc_astgen_generate_expr(astgen, 0);
					U32 other_token_idx = -1;
					hsc_data_type_ensure_compatible_assignment(astgen, other_token_idx, astgen->curly_initializer_gen.entry_data_type, &value_expr);

					initializer_expr->designated_initializer.value_expr_rel_idx = value_expr - initializer_expr;

					hsc_curly_initializer_gen_designator_undo_entry_indices(astgen);
					token = hsc_token_peek(astgen);

CURLY_INITIALIZER_AFTER_VALUE: {}
					while (1) {
						bool found_one = false;
						if (token == HSC_TOKEN_CURLY_CLOSE) {
							if (nested_count) {
								token = hsc_curly_initializer_gen_initializer_close(astgen);
								nested_count -= 1;
							} else {
								goto CURLY_INITIALIZER_FINISH;
							}
							found_one = true;
						}

						if (token == HSC_TOKEN_COMMA) {
							token = hsc_token_next(astgen);
							if (token != HSC_TOKEN_CURLY_CLOSE) {
								break;
							}
							found_one = true;
						}

						if (!found_one) {
							hsc_astgen_error_1(astgen, "expected a '}' to finish the initializer list or a ',' to declare another initializer");
						}
					}
				}
CURLY_INITIALIZER_FINISH: {}
				token = hsc_token_next(astgen);

CURLY_INITIALIZER_END:
				curly_initializer_expr->curly_initializer.first_expr_rel_idx = gen->first_initializer_expr - curly_initializer_expr;
				return curly_initializer_expr;
			} else if (assign_data_type == HSC_DATA_TYPE_VOID) {
				hsc_astgen_error_1(astgen, "'{' can only be used as the assignment of variable declarations or compound literals");
			} else {
				HscString data_type_name = hsc_data_type_string(astgen, assign_data_type);
				hsc_astgen_error_1(astgen, "'{' can only be used for structure or array types but got '%.*s'", (int)data_type_name.size, data_type_name.data);
			}

			HSC_UNREACHABLE();
		};
		case HSC_TOKEN_KEYWORD_SIZEOF:
		case HSC_TOKEN_KEYWORD_ALIGNOF:
		{
			bool is_sizeof = token == HSC_TOKEN_KEYWORD_SIZEOF;
			token = hsc_token_next(astgen);
			bool has_curly = token == HSC_TOKEN_PARENTHESIS_OPEN;
			if (has_curly) {
				token = hsc_token_next(astgen);
			}
			HscExpr* expr = hsc_astgen_generate_unary_expr(astgen);
			if (has_curly) {
				token = hsc_token_peek(astgen);
				if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
					hsc_astgen_error_1(astgen, "expected a ')' here to finish the expression");
				}
				token = hsc_token_next(astgen);
			} else if (expr->type == HSC_EXPR_TYPE_DATA_TYPE) {
				hsc_astgen_error_1(astgen, "the type after 'sizeof' be wrapped in parenthesis. eg. sizeof(uint32_t)");
			}

			Uptr size;
			Uptr align;
			hsc_data_type_size_align(astgen, expr->data_type, &size, &align);

			U32 TODO_int_64_support_plz = is_sizeof ? size : align;

			expr->type = HSC_EXPR_TYPE_CONSTANT;
			expr->constant.id = hsc_constant_table_deduplicate_basic(&astgen->constant_table, astgen, HSC_DATA_TYPE_U32, &TODO_int_64_support_plz).idx_plus_one;
			expr->data_type = HSC_DATA_TYPE_U32;
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
		case HSC_TOKEN_KEYWORD_STRUCT:
		case HSC_TOKEN_KEYWORD_UNION:
		default: {
			HscDataType data_type;
			if (hsc_astgen_generate_data_type(astgen, &data_type)) {
				HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_DATA_TYPE);
				expr->data_type = data_type;
				return expr;
			}
			hsc_astgen_error_1(astgen, "expected an expression here but got '%s'", hsc_token_strings[token]);
		};
	}
}

void hsc_astgen_generate_binary_op(HscAstGen* astgen, HscExprType* binary_op_type_out, U32* precedence_out, bool* is_assignment_out) {
	HscToken token = hsc_token_peek(astgen);
	*is_assignment_out = false;
	switch (token) {
		case HSC_TOKEN_INCREMENT:
			*binary_op_type_out = HSC_EXPR_TYPE_UNARY_OP(POST_INCREMENT);
			*precedence_out = 1;
			break;
		case HSC_TOKEN_DECREMENT:
			*binary_op_type_out = HSC_EXPR_TYPE_UNARY_OP(POST_DECREMENT);
			*precedence_out = 1;
			break;
		case HSC_TOKEN_FULL_STOP:
			*binary_op_type_out = HSC_EXPR_TYPE_FIELD_ACCESS;
			*precedence_out = 1;
			break;
		case HSC_TOKEN_PARENTHESIS_OPEN:
			*binary_op_type_out = HSC_EXPR_TYPE_CALL;
			*precedence_out = 1;
			break;
		case HSC_TOKEN_SQUARE_OPEN:
			*binary_op_type_out = HSC_EXPR_TYPE_ARRAY_SUBSCRIPT;
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
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(LOGICAL_AND);
			*precedence_out = 11;
			break;
		case HSC_TOKEN_LOGICAL_OR:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(LOGICAL_OR);
			*precedence_out = 12;
			break;
		case HSC_TOKEN_QUESTION_MARK:
			*binary_op_type_out = HSC_EXPR_TYPE_BINARY_OP(TERNARY);
			*precedence_out = 13;
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

void hsc_astgen_ensure_function_args_count(HscAstGen* astgen, HscFunction* function, U32 args_count) {
	if (args_count < function->params_count) {
		HscString string = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
		hsc_astgen_error_2(astgen, function->identifier_token_idx, "not enough arguments, expected '%u' but got '%u' for '%.*s'", function->params_count, args_count, (int)string.size, string.data);
	} else if (args_count > function->params_count) {
		HscString string = hsc_string_table_get(&astgen->string_table, function->identifier_string_id);
		hsc_astgen_error_2(astgen, function->identifier_token_idx, "too many arguments, expected '%u' but got '%u' for '%.*s'", function->params_count, args_count, (int)string.size, string.data);
	}
}

void hsc_astgen_ensure_macro_args_count(HscAstGen* astgen, HscMacro* macro, U32 args_count) {
	if (args_count < macro->params_count) {
		HscString string = hsc_string_table_get(&astgen->string_table, macro->identifier_string_id);
		U32 macro_token_idx = astgen->tokens_count;
		hsc_astgen_add_token(astgen, 0);
		astgen->token_locations[astgen->token_locations_count - 1] = macro->location;
		hsc_astgen_token_error_2(astgen, macro_token_idx, "not enough arguments, expected '%u' but got '%u' for '%.*s'", macro->params_count, args_count, (int)string.size, string.data);
	} else if (args_count > macro->params_count) {
		HscString string = hsc_string_table_get(&astgen->string_table, macro->identifier_string_id);
		U32 macro_token_idx = astgen->tokens_count;
		hsc_astgen_add_token(astgen, 0);
		astgen->token_locations[astgen->token_locations_count - 1] = macro->location;
		hsc_astgen_token_error_2(astgen, macro_token_idx, "too many arguments, expected '%u' but got '%u' for '%.*s'", macro->params_count, args_count, (int)string.size, string.data);
	}
}

U32 hsc_variable_to_string(HscAstGen* astgen, HscVariable* variable, char* buf, U32 buf_size, bool color) {
	char* fmt;
	if (color) {
		fmt = "\x1b[1;95m%s\x1b[1;94m%.*s \x1b[97m%.*s\x1b[0m";
	} else {
		fmt = "%s%.*s %.*s";
	}

	char* specifiers;
	if (variable->is_static) {
		specifiers = "static ";
	} else {
		// no need to handle const since hsc_data_type_string does this at a type level
		specifiers = "";
	}
	HscString type_name = hsc_data_type_string(astgen, variable->data_type);
	HscString variable_name = hsc_string_table_get(&astgen->string_table, variable->identifier_string_id);
	return snprintf(buf, buf_size, fmt, specifiers, (int)type_name.size, type_name.data, (int)variable_name.size, variable_name.data);
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
		HscVariable* param = &astgen->function_params_and_variables[function->params_start_idx + param_idx];
		cursor += hsc_variable_to_string(astgen, param, buf + cursor, buf_size - cursor, color);
		if (param_idx + 1 < function->params_count) {
			cursor += snprintf(buf + cursor, buf_size - cursor, ", ");
		}
	}
	cursor += snprintf(buf + cursor, buf_size - cursor, ")");
	return cursor;
}

HscExpr* hsc_astgen_generate_call_expr(HscAstGen* astgen, HscExpr* function_expr) {
	U32 args_count = 0;
	HscExpr* call_args_expr = NULL;

	U32 function_idx = function_expr->function.idx;
	HscFunction* function = hsc_function_get(astgen, HSC_DECL_INIT(HSC_DECL_FUNCTION, function_idx));

	HscToken token = hsc_token_peek(astgen);
	if (token == HSC_TOKEN_PARENTHESIS_CLOSE) {
		hsc_astgen_ensure_function_args_count(astgen, function, 0);
		return NULL;
	}

	//
	// scan ahead an count how many arguments we are going to have.
	args_count = 1;
	U32 ahead_by = 0;
	U32 parenthesis_open = 0; // to avoid counting the comma operator
	while (1) {
		HscToken token = hsc_token_peek_ahead(astgen, ahead_by);
		switch (token) {
			case HSC_TOKEN_EOF:
				goto END_ARG_COUNT;
			case HSC_TOKEN_COMMA:
				if (parenthesis_open == 0) {
					args_count += 1;
				}
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
	HscVariable* params_array = &astgen->function_params_and_variables[function->params_start_idx];
	astgen->generic_data_type_state = (HscGenericDataTypeState){0};
	while (1) {
		HscExpr* arg_expr = hsc_astgen_generate_expr(astgen, 0);
		HscVariable* param = &params_array[arg_idx];
		HscDataType param_data_type = HSC_DATA_TYPE_STRIP_CONST(param->data_type);
		hsc_data_type_ensure_compatible_assignment(astgen, param->identifier_token_idx, param_data_type, &arg_expr);

		next_arg_expr_rel_indices[arg_idx] = arg_expr - prev_arg_expr;
		arg_idx += 1;

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

	HSC_DEBUG_ASSERT(arg_idx == args_count, "internal error: the scan ahead arguments count code is out of sync with the parser");

	hsc_astgen_ensure_function_args_count(astgen, function, arg_idx);

	HscDataType return_data_type = astgen->functions[function_idx].return_data_type;
	return_data_type = hsc_data_type_resolve_generic(astgen, return_data_type);

	function_expr->function.idx = function_idx;

	HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_CALL);
	expr->binary.left_expr_rel_idx = expr - function_expr;
	expr->binary.right_expr_rel_idx = expr - call_args_expr;
	expr->data_type = return_data_type;
	return expr;
}

HscExpr* hsc_astgen_generate_array_subscript_expr(HscAstGen* astgen, HscExpr* array_expr) {
	HscExpr* index_expr = hsc_astgen_generate_expr(astgen, 0);
	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_SQUARE_CLOSE) {
		hsc_astgen_error_1(astgen, "expected ']' to finish the array subscript");
	}
	hsc_token_next(astgen);

	HscArrayDataType* d = hsc_array_data_type_get(astgen, array_expr->data_type);

	HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_ARRAY_SUBSCRIPT);
	expr->binary.left_expr_rel_idx = expr - array_expr;
	expr->binary.right_expr_rel_idx = expr - index_expr;
	expr->data_type = d->element_data_type;
	if (HSC_DATA_TYPE_IS_CONST(array_expr->data_type)) {
		expr->data_type = HSC_DATA_TYPE_CONST(expr->data_type);
	}
	return expr;
}

HscExpr* hsc_astgen_generate_field_access_expr(HscAstGen* astgen, HscExpr* left_expr) {
	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_IDENT) {
		HscString left_data_type_name = hsc_data_type_string(astgen, left_expr->data_type);
		hsc_astgen_error_1(astgen, "expected an the field identifier that you wish to access from '%.*s'", (int)left_data_type_name.size, left_data_type_name.data);
	}

	HscCompoundDataType* compound_data_type = hsc_compound_data_type_get(astgen, left_expr->data_type);

	HscStringId identifier_string_id = hsc_token_value_next(astgen).string_id;
	hsc_compound_data_type_find_field_by_name_checked(astgen, left_expr->data_type, compound_data_type, identifier_string_id);

	hsc_token_next(astgen);

	HscDataType const_mask = 0;
	if (HSC_DATA_TYPE_IS_CONST(left_expr->data_type)) {
		const_mask = HSC_DATA_TYPE_CONST_MASK;
	}

	HscExpr* deepest_expr = &astgen->exprs[astgen->exprs_count];
	for (U32 i = 0; i < astgen->compound_type_find_fields_count; i += 1) {
		HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_FIELD_ACCESS);
		expr->binary.left_expr_rel_idx = 1; // link to the previous expression
		expr->binary.right_expr_rel_idx = astgen->compound_type_find_fields[i].idx;
		expr->data_type = astgen->compound_type_find_fields[i].data_type | const_mask;
	}
	deepest_expr->binary.left_expr_rel_idx = deepest_expr - left_expr;

	HscExpr* field_access_expr = &astgen->exprs[astgen->exprs_count - 1];
	return field_access_expr;
}

HscExpr* hsc_astgen_generate_ternary_expr(HscAstGen* astgen, HscExpr* cond_expr) {
	hsc_data_type_ensure_is_condition(astgen, cond_expr->data_type);

	HscExpr* true_expr = hsc_astgen_generate_expr(astgen, 0);

	HscToken token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_COLON) {
		hsc_astgen_error_1(astgen, "expected a ':' for the false side of the ternary expression");
	}
	token = hsc_token_next(astgen);

	HscExpr* false_expr = hsc_astgen_generate_expr(astgen, 0);

	U32 other_token_idx = -1;
	if (!hsc_data_type_check_compatible_arithmetic(astgen, &true_expr, &false_expr)) {
		HscString true_data_type_name = hsc_data_type_string(astgen, true_expr->data_type);
		HscString false_data_type_name = hsc_data_type_string(astgen, false_expr->data_type);
		hsc_astgen_error_2(astgen, other_token_idx, "type mismatch '%.*s' and '%.*s'", (int)false_data_type_name.size, false_data_type_name.data, (int)true_data_type_name.size, true_data_type_name.data);
	}

	HscExpr* expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_BINARY_OP(TERNARY));
	expr->ternary.cond_expr_rel_idx = expr - cond_expr;
	expr->ternary.true_expr_rel_idx = expr - true_expr;
	expr->ternary.false_expr_rel_idx = expr - false_expr;
	expr->data_type = true_expr->data_type;
	return expr;
}

HscExpr* hsc_astgen_generate_expr(HscAstGen* astgen, U32 min_precedence) {
	U32 callee_token_idx = astgen->token_read_idx;
	HscExpr* left_expr = hsc_astgen_generate_unary_expr(astgen);
	if (left_expr->type == HSC_EXPR_TYPE_DATA_TYPE) {
		return left_expr;
	}

	while (1) {
		HscExprType binary_op_type;
		U32 precedence;
		bool is_assignment;
		HscToken operator_token = hsc_token_peek(astgen);
		hsc_astgen_generate_binary_op(astgen, &binary_op_type, &precedence, &is_assignment);
		if (binary_op_type == HSC_EXPR_TYPE_NONE || (min_precedence && min_precedence <= precedence)) {
			return left_expr;
		}
		hsc_token_next(astgen);

		if (binary_op_type == HSC_EXPR_TYPE_CALL) {
			if (left_expr->type != HSC_EXPR_TYPE_FUNCTION) {
				hsc_astgen_error_2(astgen, callee_token_idx, "unexpected '(', this can only be used when the left expression is a function");
			}

			left_expr = hsc_astgen_generate_call_expr(astgen, left_expr);
		} else if (binary_op_type == HSC_EXPR_TYPE_ARRAY_SUBSCRIPT) {
			if (!HSC_DATA_TYPE_IS_ARRAY(left_expr->data_type)) {
				HscString left_data_type_name = hsc_data_type_string(astgen, left_expr->data_type);
				hsc_astgen_error_2(astgen, callee_token_idx, "unexpected '[', this can only be used when the left expression is an array but got '%.*s'", (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hsc_astgen_generate_array_subscript_expr(astgen, left_expr);
		} else if (binary_op_type == HSC_EXPR_TYPE_FIELD_ACCESS) {
			if (!HSC_DATA_TYPE_IS_COMPOUND_TYPE(left_expr->data_type)) {
				HscString left_data_type_name = hsc_data_type_string(astgen, left_expr->data_type);
				hsc_astgen_error_2(astgen, callee_token_idx, "unexpected '.', this can only be used when the left expression is a struct or union type but got '%.*s'", (int)left_data_type_name.size, left_data_type_name.data);
			}

			left_expr = hsc_astgen_generate_field_access_expr(astgen, left_expr);
		} else if (binary_op_type == HSC_EXPR_TYPE_BINARY_OP(TERNARY)) {
			left_expr = hsc_astgen_generate_ternary_expr(astgen, left_expr);
		} else if (binary_op_type == HSC_EXPR_TYPE_UNARY_OP(POST_INCREMENT) || binary_op_type == HSC_EXPR_TYPE_UNARY_OP(POST_DECREMENT)) {
			HscUnaryOp unary_op = binary_op_type - HSC_EXPR_TYPE_UNARY_OP_START;
			left_expr = hsc_astgen_generate_unary_op(astgen, left_expr, unary_op, operator_token);
		} else {
			HscExpr* right_expr = hsc_astgen_generate_expr(astgen, precedence);

			U32 other_token_idx = -1;
			if (is_assignment) {
				hsc_data_type_ensure_compatible_assignment(astgen, other_token_idx, left_expr->data_type, &right_expr);
			} else {
				hsc_data_type_ensure_compatible_arithmetic(astgen, other_token_idx, &left_expr, &right_expr, operator_token);
			}

			HscDataType data_type;
			if (HSC_EXPR_TYPE_BINARY_OP(EQUAL) <= binary_op_type && binary_op_type <= HSC_EXPR_TYPE_BINARY_OP(LOGICAL_OR)) {
				data_type = HSC_DATA_TYPE_BOOL;
			} else {
				data_type = left_expr->data_type; // TODO make implicit conversions explicit in the AST and make the error above work correctly
			}
			if (HSC_DATA_TYPE_IS_CONST(left_expr->data_type)) {
				data_type = HSC_DATA_TYPE_CONST(data_type);
			}

			if (
				hsc_opt_is_enabled(&astgen->opts, HSC_OPT_CONSTANT_FOLDING) &&
				left_expr->type == HSC_EXPR_TYPE_CONSTANT &&
				right_expr->type == HSC_EXPR_TYPE_CONSTANT
			) {
				//HscConstantId left_constant_id = { .idx_plus_one = left_expr->constant.id };
				//HscConstantId right_constant_id = { .idx_plus_one = right_expr->constant.id };
				HSC_ABORT("TODO CONSTANT FOLDING");
				//
				// combine left_expr and right_expr and store them in the left_expr
				//
			} else {
				if (is_assignment && HSC_DATA_TYPE_IS_CONST(data_type)) {
					HscString left_data_type_name = hsc_data_type_string(astgen, data_type);
					hsc_astgen_error_1(astgen, "cannot assign to a target that has a constant data type of '%.*s'", (int)left_data_type_name.size, left_data_type_name.data);
				}

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
	hsc_data_type_ensure_is_condition(astgen, cond_expr->data_type);

	token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
		hsc_astgen_error_1(astgen, "expected a ')' to finish the if statement condition");
	}
	token = hsc_token_next(astgen);
	return cond_expr;
}

HscDataType hsc_astgen_deduplicate_array_data_type(HscAstGen* astgen, HscDataType element_data_type, HscConstantId size_constant_id) {
	element_data_type = hsc_typedef_resolve(astgen, element_data_type);
	for (U32 i = 0; i < astgen->array_data_types_count; i += 1) {
		HscArrayDataType* d = &astgen->array_data_types[i];
		if (d->element_data_type == element_data_type && d->size_constant_id.idx_plus_one == size_constant_id.idx_plus_one) {
			return HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_ARRAY, i);
		}
	}

	HSC_ASSERT_ARRAY_BOUNDS(astgen->array_data_types_count, astgen->array_data_types_cap);
	U32 array_data_types_idx = astgen->array_data_types_count;
	HscArrayDataType* d = &astgen->array_data_types[array_data_types_idx];
	d->element_data_type = element_data_type;
	d->size_constant_id = size_constant_id;
	astgen->array_data_types_count += 1;

	HscDataType data_type = HSC_DATA_TYPE_INIT(HSC_DATA_TYPE_ARRAY, array_data_types_idx);
	hsc_found_data_type(astgen, data_type);
	return data_type;
}

HscDataType hsc_astgen_generate_variable_decl_array(HscAstGen* astgen, HscDataType element_data_type) {
	HscToken token = hsc_token_next(astgen);
	if (token == HSC_TOKEN_SQUARE_CLOSE) {
		hsc_astgen_error_1(astgen, "expected the array size here");
	}
	HscExpr* size_expr = hsc_astgen_generate_expr(astgen, 0);
	if (size_expr->type != HSC_EXPR_TYPE_CONSTANT) {
		hsc_astgen_error_1(astgen, "expected an expression to resolve to an integer for the array size here");
	}

	if (size_expr->data_type < HSC_DATA_TYPE_U8 || size_expr->data_type > HSC_DATA_TYPE_S64) {
		hsc_astgen_error_1(astgen, "expected an integer type for the array size here");
	}

	HscConstantId size_constant_id = { .idx_plus_one = size_expr->constant.id };
	HscConstant constant = hsc_constant_table_get(&astgen->constant_table, size_constant_id);
	U64 size;
	if (!hsc_constant_as_uint(constant, &size)) {
		hsc_astgen_error_1(astgen, "the array size cannot be negative");
	}
	if (size == 0) {
		hsc_astgen_error_1(astgen, "the array size cannot be zero");
	}

	token = hsc_token_peek(astgen);
	if (token != HSC_TOKEN_SQUARE_CLOSE) {
		hsc_astgen_error_1(astgen, "expected a ']' after the array size expression");
	}
	token = hsc_token_next(astgen);

	HscDataType data_type = hsc_astgen_deduplicate_array_data_type(astgen, element_data_type, size_constant_id);
	if (token == HSC_TOKEN_SQUARE_OPEN) {
		data_type = hsc_astgen_generate_variable_decl_array(astgen, data_type);
	}
	return data_type;
}

HscToken hsc_astgen_consume_specifiers(HscAstGen* astgen) {
	HscToken token = hsc_token_peek(astgen);
	while (1) {
		HscAstGenFlags flag = 0;
		switch (token) {
			case HSC_TOKEN_KEYWORD_STATIC: flag = HSC_ASTGEN_FLAGS_FOUND_STATIC; break;
			case HSC_TOKEN_KEYWORD_CONST: flag = HSC_ASTGEN_FLAGS_FOUND_CONST; break;
			case HSC_TOKEN_KEYWORD_INLINE: flag = HSC_ASTGEN_FLAGS_FOUND_INLINE; break;
			case HSC_TOKEN_KEYWORD_NO_RETURN: flag = HSC_ASTGEN_FLAGS_FOUND_NO_RETURN; break;
			case HSC_TOKEN_KEYWORD_AUTO: break;
			case HSC_TOKEN_KEYWORD_VOLATILE:
			case HSC_TOKEN_KEYWORD_EXTERN:
				hsc_astgen_error_1(astgen, "'%s' is currently unsupported", hsc_token_strings[token]);
			default: return token;
		}

		if (astgen->flags & flag) {
			hsc_astgen_error_1(astgen, "'%s' has already been used for this declaration", hsc_token_strings[token]);
		}
		astgen->flags |= flag;
		token = hsc_token_next(astgen);
	}
}

void _hsc_astgen_ensure_no_unused_specifiers(HscAstGen* astgen, char* what) {
	if (astgen->flags & (HSC_ASTGEN_FLAGS_FOUND_STATIC | HSC_ASTGEN_FLAGS_FOUND_CONST | HSC_ASTGEN_FLAGS_FOUND_INLINE)) {
		const char* message = NULL;
		if (astgen->flags & HSC_ASTGEN_FLAGS_FOUND_STATIC) {
			message = "the 'static' keyword was used, so we are expecting %s for a declaration but got '%s'";
		} else if (astgen->flags & HSC_ASTGEN_FLAGS_FOUND_CONST) {
			message = "the 'const' keyword was used, so we are expecting %s to declare a variable but got '%s'";
		} else if (astgen->flags & HSC_ASTGEN_FLAGS_FOUND_INLINE) {
			message = "the 'inline' keyword was used, so we are expecting %s to declare a function but got '%s'";
		} else if (astgen->flags & HSC_ASTGEN_FLAGS_FOUND_NO_RETURN) {
			message = "the '_Noreturn' keyword was used, so we are expecting %s to declare a function but got '%s'";
		}
		hsc_astgen_error_1(astgen, message, what, hsc_token_strings[hsc_token_peek(astgen)]);
	}
}

void hsc_astgen_ensure_no_unused_specifiers_data_type(HscAstGen* astgen) {
	_hsc_astgen_ensure_no_unused_specifiers(astgen, "a data type");
}

void hsc_astgen_ensure_no_unused_specifiers_identifier(HscAstGen* astgen) {
	_hsc_astgen_ensure_no_unused_specifiers(astgen, "an identifier");
}

U32 hsc_astgen_generate_variable_decl(HscAstGen* astgen, bool is_global, HscDataType* data_type_mut, HscExpr** init_expr_out) {
	HscToken token = hsc_token_peek(astgen);
	HSC_DEBUG_ASSERT(token == HSC_TOKEN_IDENT, "internal error: expected an identifier for a variable declaration");
	HscStringId identifier_string_id = hsc_token_value_next(astgen).string_id;

	if (astgen->flags & HSC_ASTGEN_FLAGS_FOUND_INLINE) {
		hsc_astgen_error_1(astgen, "the 'inline' keyword cannot be used on this variable declaration as it is a function specifier");
	}

	if (astgen->flags & HSC_ASTGEN_FLAGS_FOUND_NO_RETURN) {
		hsc_astgen_error_1(astgen, "the '_Noreturn' keyword cannot be used on this variable declaration as it is a function specifier");
	}

	U32 existing_variable_id = hsc_astgen_variable_stack_find(astgen, identifier_string_id);
	if (existing_variable_id) {
		U32 other_token_idx = -1;// TODO: location of existing variable
		HscString string = hsc_string_table_get(&astgen->string_table, identifier_string_id);
		hsc_astgen_error_2(astgen, other_token_idx, "redefinition of '%.*s' local variable identifier", (int)string.size, string.data);
	}

	U32 variable_idx;
	HscVariable* variable;
	if (is_global) {
		variable_idx = astgen->global_variables_count;
		HSC_ASSERT_ARRAY_BOUNDS(astgen->global_variables_count, astgen->global_variables_cap);
		variable = &astgen->global_variables[astgen->global_variables_count];
		astgen->global_variables_count += 1;

		HscDecl decl = HSC_DECL_INIT(HSC_DECL_GLOBAL_VARIABLE, variable_idx);
		hsc_astgen_insert_global_declaration(astgen, identifier_string_id, decl);
	} else {
		variable_idx = hsc_astgen_variable_stack_add(astgen, identifier_string_id);
		HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_and_variables_count, astgen->function_params_and_variables_cap);
		variable = &astgen->function_params_and_variables[astgen->function_params_and_variables_count];
		astgen->function_params_and_variables_count += 1;
		astgen->stmt_block->stmt_block.variables_count += 1;
	}
	variable->identifier_string_id = identifier_string_id;
	variable->identifier_token_idx = astgen->token_read_idx;
	variable->data_type = *data_type_mut;
	variable->is_static = !!(astgen->flags & HSC_ASTGEN_FLAGS_FOUND_STATIC) || is_global;
	variable->is_const = !!(astgen->flags & HSC_ASTGEN_FLAGS_FOUND_CONST);
	variable->initializer_constant_id.idx_plus_one = 0;

	token = hsc_token_next(astgen);
	if (token == HSC_TOKEN_SQUARE_OPEN) {
		variable->data_type = hsc_astgen_generate_variable_decl_array(astgen, variable->data_type);
		*data_type_mut = variable->data_type;
		token = hsc_token_peek(astgen);
	}

	if (variable->is_const) {
		variable->data_type = HSC_DATA_TYPE_CONST(variable->data_type);
		*data_type_mut = variable->data_type;
	}

	switch (token) {
		case HSC_TOKEN_SEMICOLON:
			if (init_expr_out) *init_expr_out = NULL;
			if (variable->is_static) {
				variable->initializer_constant_id = hsc_constant_table_deduplicate_zero(&astgen->constant_table, astgen, variable->data_type);
			}
			break;
		case HSC_TOKEN_EQUAL: {
			hsc_token_next(astgen);

			astgen->assign_data_type = variable->data_type;
			HscExpr* init_expr = hsc_astgen_generate_expr(astgen, 0);
			U32 other_token_idx = -1;
			HscDataType variable_data_type = HSC_DATA_TYPE_STRIP_CONST(variable->data_type);
			hsc_data_type_ensure_compatible_assignment(astgen, other_token_idx, variable_data_type, &init_expr);
			astgen->assign_data_type = HSC_DATA_TYPE_VOID;

			if (variable->is_static) {
				if (init_expr->type != HSC_EXPR_TYPE_CONSTANT) {
					hsc_astgen_error_1(astgen, "variable declaration is static, so this initializer expression must be a constant");
				}
				variable->initializer_constant_id.idx_plus_one = init_expr->constant.id;
				if (init_expr_out) *init_expr_out = NULL;
			} else {
				if (init_expr_out) *init_expr_out = init_expr;
			}
			break;
		};
		default:
			hsc_astgen_error_1(astgen, "expected a ';' to end the declaration or a '=' to assign to the new variable");
	}

	astgen->flags &= ~(HSC_ASTGEN_FLAGS_FOUND_STATIC | HSC_ASTGEN_FLAGS_FOUND_CONST);
	return variable_idx;
}

HscExpr* hsc_astgen_generate_variable_decl_expr(HscAstGen* astgen, HscDataType data_type) {
	HscExpr* init_expr = NULL;
	U32 variable_idx = hsc_astgen_generate_variable_decl(astgen, false, &data_type, &init_expr);
	if (init_expr) {
		HscExpr* left_expr = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_LOCAL_VARIABLE);
		left_expr->variable.idx = variable_idx;
		left_expr->data_type = data_type;

		HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_BINARY_OP(ASSIGN));
		stmt->binary.is_assignment = true;
		stmt->binary.left_expr_rel_idx = stmt - left_expr;
		stmt->binary.right_expr_rel_idx = stmt - init_expr;
		return stmt;
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
			stmt_block->stmt_block.variables_count = 0;
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
			hsc_token_next(astgen);
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);

			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_RETURN);
			stmt->unary.expr_rel_idx = stmt - expr;
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
		case HSC_TOKEN_KEYWORD_DO: {
			HscExpr* stmt = hsc_astgen_alloc_expr(astgen, HSC_EXPR_TYPE_STMT_WHILE);
			hsc_token_next(astgen);

			bool prev_is_in_loop = astgen->is_in_loop;
			astgen->is_in_loop = true;
			HscExpr* loop_stmt = hsc_astgen_generate_stmt(astgen);
			loop_stmt->is_stmt_block_entry = true;
			astgen->is_in_loop = prev_is_in_loop;

			token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_KEYWORD_WHILE) {
				hsc_astgen_error_1(astgen, "expected 'while' to define the condition of the do while loop");
			}
			token = hsc_token_next(astgen);

			HscExpr* cond_expr = hsc_astgen_generate_cond_expr(astgen);

			stmt->while_.cond_expr_rel_idx = cond_expr - stmt;
			stmt->while_.loop_stmt_rel_idx = loop_stmt - stmt;

			hsc_astgen_ensure_semicolon(astgen);
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
				token = hsc_token_peek(astgen);
				if (token != HSC_TOKEN_IDENT) {
					hsc_astgen_error_1(astgen, "expected an identifier for a variable declaration");
				}
				init_expr = hsc_astgen_generate_variable_decl_expr(astgen, init_expr->data_type);
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
			U32 other_token_idx = -1;// TODO: the switch condition expr
			hsc_data_type_ensure_compatible_assignment(astgen, other_token_idx, astgen->switch_state.switch_condition_type, &expr);

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
		case HSC_TOKEN_KEYWORD_TYPEDEF:
			hsc_astgen_generate_typedef(astgen);
			return NULL;
		case HSC_TOKEN_KEYWORD_STATIC:
		case HSC_TOKEN_KEYWORD_CONST:
			hsc_astgen_consume_specifiers(astgen);
			goto EXPR;
		case HSC_TOKEN_SEMICOLON:
			hsc_token_next(astgen);
			return NULL;
		default: {
EXPR: {}
			HscExpr* expr = hsc_astgen_generate_expr(astgen, 0);
			if (expr->type == HSC_EXPR_TYPE_DATA_TYPE) {
				token = hsc_token_peek(astgen);
				switch (token) {
					case HSC_TOKEN_KEYWORD_STATIC:
					case HSC_TOKEN_KEYWORD_CONST:
						token = hsc_astgen_consume_specifiers(astgen);
						break;
				}

				if (token == HSC_TOKEN_IDENT) {
					expr = hsc_astgen_generate_variable_decl_expr(astgen, expr->data_type);
				} else {
					hsc_astgen_ensure_no_unused_specifiers_identifier(astgen);
					expr = NULL;
				}
			} else {
				hsc_astgen_ensure_no_unused_specifiers_data_type(astgen);
			}

			hsc_astgen_ensure_semicolon(astgen);
			return expr;
		};
	}
}

void hsc_astgen_generate_function(HscAstGen* astgen) {
	HSC_ASSERT(astgen->functions_count < astgen->functions_cap, "functions are full");
	HscFunction* function = &astgen->functions[astgen->functions_count];
	U32 function_idx = astgen->functions_count;
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

	if (!hsc_astgen_generate_data_type(astgen, &function->return_data_type)) {
		hsc_astgen_error_1(astgen, "expected type here");
	}
	token = hsc_token_peek(astgen);

	if (token != HSC_TOKEN_IDENT) {
		hsc_astgen_error_2(astgen, shader_stage_read_idx, "expected an identifier for a function since a shader stage was used");
	}
	HscStringId identifier_string_id = hsc_token_value_next(astgen).string_id;
	function->identifier_string_id = identifier_string_id;

	HscDecl decl = HSC_DECL_INIT(HSC_DECL_FUNCTION, function_idx);
	hsc_astgen_insert_global_declaration(astgen, identifier_string_id, decl);

	token = hsc_token_next(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_OPEN) {
		hsc_astgen_error_2(astgen, shader_stage_read_idx, "expected an '(' to start defining function parameters since a shader stage was used");
	}

	hsc_astgen_variable_stack_open(astgen);

	function->params_start_idx = astgen->function_params_and_variables_count;
	function->params_count = 0;
	token = hsc_token_next(astgen);
	if (token != HSC_TOKEN_PARENTHESIS_CLOSE) {
		while (1) {
			HSC_ASSERT_ARRAY_BOUNDS(astgen->function_params_and_variables_count, astgen->function_params_and_variables_cap);
			HscVariable* param = &astgen->function_params_and_variables[astgen->function_params_and_variables_count];
			function->params_count += 1;
			astgen->function_params_and_variables_count += 1;

			if (!hsc_astgen_generate_data_type(astgen, &param->data_type)) {
				hsc_astgen_error_1(astgen, "expected type here");
			}
			token = hsc_token_peek(astgen);
			if (token != HSC_TOKEN_IDENT) {
				// TODO replace error message U32 type with the actual type name of param->type
				hsc_astgen_error_1(astgen, "expected an identifier for a function parameter e.g. U32 param_identifier");
			}
			identifier_string_id = hsc_token_value_next(astgen).string_id;
			param->identifier_string_id = identifier_string_id;

			U32 existing_variable_id = hsc_astgen_variable_stack_find(astgen, identifier_string_id);
			if (existing_variable_id) {
					U32 other_token_idx = -1;// TODO: location of existing variable
				HscString string = hsc_string_table_get(&astgen->string_table, identifier_string_id);
				hsc_astgen_error_2(astgen, other_token_idx, "redefinition of '%.*s' local variable identifier", (int)string.size, string.data);
			}
			hsc_astgen_variable_stack_add(astgen, identifier_string_id);
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

	U32 ordered_data_types_start_idx = astgen->ordered_data_types_count;
	function->used_static_variables_start_idx = astgen->used_static_variables_count;

	function->block_expr_id.idx_plus_one = 0;
	if (token == HSC_TOKEN_CURLY_OPEN) {
		HscExpr* expr = hsc_astgen_generate_stmt(astgen);
		function->block_expr_id.idx_plus_one = (expr - astgen->exprs) + 1;
	}

	hsc_astgen_variable_stack_close(astgen);
	function->variables_count = astgen->next_var_idx;

	function->used_static_variables_count = astgen->used_static_variables_count - function->used_static_variables_start_idx;

	for (U32 i = ordered_data_types_start_idx; i < astgen->ordered_data_types_count; i += 1) {
		HscDataType data_type = astgen->ordered_data_types[i];
		switch (data_type & 0xff) {
		case HSC_DATA_TYPE_STRUCT:
		case HSC_DATA_TYPE_UNION: {
			HscCompoundDataType* d = hsc_compound_data_type_get(astgen, data_type);
			if (d->identifier_string_id.idx_plus_one) {
				HscHashTable(HscStringId, HscDataType)* declarations;
				if (HSC_DATA_TYPE_IS_UNION(data_type)) {
					declarations = &astgen->union_declarations;
				} else {
					declarations = &astgen->struct_declarations;
				}

				HSC_DEBUG_ASSERT(hsc_hash_table_remove(declarations, d->identifier_string_id.idx_plus_one, NULL), "internal error: compound type should have existed");
			}
			break;
		};
		case HSC_DATA_TYPE_ENUM: {
			HscEnumDataType* enum_data_type = hsc_enum_data_type_get(astgen, data_type);
			HSC_DEBUG_ASSERT(hsc_hash_table_remove(&astgen->enum_declarations, enum_data_type->identifier_string_id.idx_plus_one, NULL), "internal error: enum type should have existed");
			break;
		};
		case HSC_DATA_TYPE_TYPEDEF: {
			HscTypedef* typedef_ = hsc_typedef_get(astgen, data_type);
			HSC_DEBUG_ASSERT(hsc_hash_table_remove(&astgen->global_declarations, typedef_->identifier_string_id.idx_plus_one, NULL), "internal error: typedef should have existed");
			break;
		};
		}
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
			case HSC_TOKEN_KEYWORD_TYPEDEF:
				hsc_astgen_generate_typedef(astgen);
				break;
			case HSC_TOKEN_KEYWORD_STATIC:
			case HSC_TOKEN_KEYWORD_CONST:
				hsc_astgen_consume_specifiers(astgen);
				// fallthrough
			default: {
				HscDataType data_type;
				if (hsc_astgen_generate_data_type(astgen, &data_type)) {
					token = hsc_token_peek(astgen);
					switch (token) {
						case HSC_TOKEN_KEYWORD_STATIC:
						case HSC_TOKEN_KEYWORD_CONST:
							token = hsc_astgen_consume_specifiers(astgen);
							break;
					}
					if (token == HSC_TOKEN_IDENT) {
						hsc_astgen_generate_variable_decl(astgen, true, &data_type, NULL);
					} else {
						hsc_astgen_ensure_no_unused_specifiers_identifier(astgen);
					}
					hsc_astgen_ensure_semicolon(astgen);
					break;
				} else {
					hsc_astgen_ensure_no_unused_specifiers_data_type(astgen);
				}
				HSC_ABORT("TODO at scope see if this token is a type and varify it is a function, and support enums found token '%s'", hsc_token_strings[token]);
			};
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
		switch (token) {
			case HSC_TOKEN_IDENT:
				value = astgen->token_values[token_value_idx];
				token_value_idx += 1;
				string = hsc_string_table_get(&astgen->string_table, value.string_id);
				fprintf(f, "%s -> %.*s\n", hsc_token_strings[token], (int)string.size, string.data);
				break;
			case HSC_TOKEN_LIT_U32:
			case HSC_TOKEN_LIT_U64:
			case HSC_TOKEN_LIT_S32:
			case HSC_TOKEN_LIT_S64:
			case HSC_TOKEN_LIT_F32:
			case HSC_TOKEN_LIT_F64:
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
			U32 variables_count = expr->stmt_block.variables_count;
			for (U32 i = 0; i < variables_count; i += 1) {
				char buf[1024] = "<CURLY_INITIALIZER_RESULT>";
				U32 variable_idx = astgen->print_variable_base_idx + i;
				HscVariable* variable = &astgen->function_params_and_variables[astgen->print_function->params_start_idx + variable_idx];
				if (variable->identifier_string_id.idx_plus_one) {
					hsc_variable_to_string(astgen, variable, buf, sizeof(buf), false);
				}
				fprintf(f, "%.*sLOCAL_VARIABLE(#%u): %s", indent + 1, indent_chars, variable_idx, buf);
				if (variable->initializer_constant_id.idx_plus_one) {
					fprintf(f, " = ");
					hsc_constant_print(astgen, variable->initializer_constant_id, f);
				}
				fprintf(f, "\n");
			}
			astgen->print_variable_base_idx += variables_count;

			for (U32 i = 0; i < stmts_count; i += 1) {
				hsc_astgen_print_expr(astgen, stmt, indent + 1, true, f);
				stmt = &stmt[stmt->next_expr_rel_idx];
			}
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_FUNCTION: {
			HscFunction* function = &astgen->functions[expr->function.idx];
			char buf[1024];
			hsc_function_to_string(astgen, function, buf, sizeof(buf), false);
			fprintf(f, "EXPR_FUNCTION Function(#%u): %s", expr->function.idx, buf);
			break;
		};
		case HSC_EXPR_TYPE_STMT_RETURN: expr_name = "STMT_RETURN"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(LOGICAL_NOT): expr_name = "EXPR_LOGICAL_NOT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(BIT_NOT): expr_name = "EXPR_BIT_NOT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(PLUS): expr_name = "EXPR_PLUS"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(NEGATE): expr_name = "EXPR_NEGATE"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT): expr_name = "EXPR_PRE_INCREMENT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(PRE_DECREMENT): expr_name = "EXPR_PRE_DECREMENT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(POST_INCREMENT): expr_name = "EXPR_POST_INCREMENT"; goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(POST_DECREMENT): expr_name = "EXPR_POST_DECREMENT"; goto UNARY;
UNARY:
		{
			fprintf(f, "%s: {\n", expr_name);
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			hsc_astgen_print_expr(astgen, unary_expr, indent + 1, false, f);
			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_CAST: {
			fprintf(f, "EXPR_CAST: {\n");
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;
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
			fprintf(f, "%s: {\n", expr->while_.cond_expr_rel_idx > expr->while_.loop_stmt_rel_idx ? "STMT_DO_WHILE" : "STMT_WHILE");

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
		case HSC_EXPR_TYPE_BINARY_OP(LOGICAL_AND): expr_name = "LOGICAL_AND"; goto BINARY;
		case HSC_EXPR_TYPE_BINARY_OP(LOGICAL_OR): expr_name = "LOGICAL_OR"; goto BINARY;
		case HSC_EXPR_TYPE_CALL: expr_name = "CALL"; goto BINARY;
		case HSC_EXPR_TYPE_ARRAY_SUBSCRIPT: expr_name = "ARRAY_SUBSCRIPT"; goto BINARY;
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
		case HSC_EXPR_TYPE_CURLY_INITIALIZER: {
			fprintf(f, "%s: {\n", "EXPR_CURLY_INITIALIZER");

			////////////////////////////////////////////////////////////////////////////
			// skip the internal variable expression that sits t the start of the initializer_expr list
			HscExpr* initializer_expr = &expr[expr->curly_initializer.first_expr_rel_idx];
			U32 expr_rel_idx;
			////////////////////////////////////////////////////////////////////////////

			while (1) {
				expr_rel_idx = initializer_expr->next_expr_rel_idx;
				if (expr_rel_idx == 0) {
					break;
				}
				initializer_expr = &initializer_expr[expr_rel_idx];

				U64* entry_indices = &astgen->entry_indices[initializer_expr->alt_next_expr_rel_idx];
				U32 entry_indices_count = initializer_expr->designated_initializer.entry_indices_count;
				fprintf(f, "%.*s", indent + 1, indent_chars);
				HscDataType data_type = expr->data_type;
				for (U32 idx = 0; idx < entry_indices_count; idx += 1) {
					data_type = hsc_typedef_resolve(astgen, data_type);
					U64 entry_idx = entry_indices[idx];
					if (HSC_DATA_TYPE_IS_ARRAY(data_type)) {
						HscArrayDataType* array_data_type = hsc_array_data_type_get(astgen, data_type);
						fprintf(f, "[%zu]", entry_idx);
						data_type = array_data_type->element_data_type;
					} else if (HSC_DATA_TYPE_IS_COMPOUND_TYPE(data_type)) {
						HscCompoundDataType* compound_data_type = hsc_compound_data_type_get(astgen, data_type);
						HscCompoundField* field = &astgen->compound_fields[compound_data_type->fields_start_idx + entry_idx];
						if (field->identifier_string_id.idx_plus_one) {
							HscString identifier_string = hsc_string_table_get(&astgen->string_table, field->identifier_string_id);
							fprintf(f, ".%.*s", (int)identifier_string.size, identifier_string.data);
						}
						data_type = field->data_type;
					}
				}
				fprintf(f, " = ");

				if (initializer_expr->designated_initializer.value_expr_rel_idx) {
					HscExpr* value_expr = &initializer_expr[initializer_expr->designated_initializer.value_expr_rel_idx];
					hsc_astgen_print_expr(astgen, value_expr, 0, false, f);
				} else {
					fprintf(f, "<ZERO>\n");
				}
			}

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		case HSC_EXPR_TYPE_FIELD_ACCESS: {
			fprintf(f, "%s: {\n", "EXPR_FIELD_ACCESS");

			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			U32 field_idx = expr->binary.right_expr_rel_idx;
			hsc_astgen_print_expr(astgen, left_expr, indent + 1, false, f);

			HscCompoundDataType* compound_data_type = hsc_compound_data_type_get(astgen, left_expr->data_type);
			HscCompoundField* field = &astgen->compound_fields[compound_data_type->fields_start_idx + field_idx];

			HscString field_data_type_name = hsc_data_type_string(astgen, field->data_type);
			if (field->identifier_string_id.idx_plus_one) {
				HscString identifier_string = hsc_string_table_get(&astgen->string_table, field->identifier_string_id);
				fprintf(f, "%.*sfield_idx(%u): %.*s %.*s\n", indent + 1, indent_chars, field_idx, (int)field_data_type_name.size, field_data_type_name.data, (int)identifier_string.size, identifier_string.data);
			} else {
				fprintf(f, "%.*sfield_idx(%u): %.*s\n", indent + 1, indent_chars, field_idx, (int)field_data_type_name.size, field_data_type_name.data);
			}

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
			HscVariable* variable = &astgen->function_params_and_variables[astgen->print_function->params_start_idx + expr->variable.idx];
			hsc_variable_to_string(astgen, variable, buf, sizeof(buf), false);
			fprintf(f, "LOCAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		case HSC_EXPR_TYPE_GLOBAL_VARIABLE: {
			char buf[1024];
			HscVariable* variable = &astgen->global_variables[expr->variable.idx];
			hsc_variable_to_string(astgen, variable, buf, sizeof(buf), false);
			fprintf(f, "GLOBAL_VARIABLE(#%u): %s", expr->variable.idx, buf);
			break;
		};
		case HSC_EXPR_TYPE_BINARY_OP(TERNARY): {
			fprintf(f, "%s: {\n", "STMT_TERNARY");

			HscExpr* cond_expr = expr - expr->ternary.cond_expr_rel_idx;
			fprintf(f, "%.*sCONDITION_EXPR:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, cond_expr, indent + 2, false, f);

			HscExpr* true_stmt = expr - expr->ternary.true_expr_rel_idx;
			fprintf(f, "%.*sTRUE_STMT:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, true_stmt, indent + 2, false, f);

			HscExpr* false_stmt = expr - expr->ternary.false_expr_rel_idx;
			fprintf(f, "%.*sFALSE_STMT:\n", indent + 1, indent_chars);
			hsc_astgen_print_expr(astgen, false_stmt, indent + 2, false, f);

			fprintf(f, "%.*s}", indent, indent_chars);
			break;
		};
		default:
			HSC_ABORT("unhandle expr type %u\n", expr->type);
	}
	fprintf(f, "\n");
}

void hsc_astgen_print_ast(HscAstGen* astgen, FILE* f) {
	for (U32 macros_idx = 0; macros_idx < astgen->macros_count; macros_idx += 1) {
		HscMacro* d = &astgen->macros[macros_idx];
		HscString name = hsc_string_table_get(&astgen->string_table, d->identifier_string_id);
		fprintf(f, "#define %.*s %.*s\n", (int)name.size, name.data, (int)d->value_string.size, d->value_string.data);
	}

	for (U32 enum_type_idx = 0; enum_type_idx < astgen->enum_data_types_count; enum_type_idx += 1) {
		HscEnumDataType* d = &astgen->enum_data_types[enum_type_idx];
		HscString name = hsc_string_lit("<anonymous>");
		if (d->identifier_string_id.idx_plus_one) {
			name = hsc_string_table_get(&astgen->string_table, d->identifier_string_id);
		}
		fprintf(f, "ENUM(#%u): %.*s {\n", enum_type_idx, (int)name.size, name.data);
		for (U32 value_idx = 0; value_idx < d->values_count; value_idx += 1) {
			HscEnumValue* value = &astgen->enum_values[d->values_start_idx + value_idx];
			HscString identifier = hsc_string_table_get(&astgen->string_table, value->identifier_string_id);

			HscConstant constant = hsc_constant_table_get(&astgen->constant_table, value->value_constant_id);

			S64 v;
			HSC_DEBUG_ASSERT(hsc_constant_as_sint(constant, &v), "internal error: expected to be a signed int");
			fprintf(f, "\t%.*s = %ld\n", (int)identifier.size, identifier.data, v);
		}
		fprintf(f, "}\n");
	}

	for (U32 compound_type_idx = 0; compound_type_idx < astgen->compound_data_types_count; compound_type_idx += 1) {
		HscCompoundDataType* d = &astgen->compound_data_types[compound_type_idx];
		HscString name = hsc_string_lit("<anonymous>");
		if (d->identifier_string_id.idx_plus_one) {
			name = hsc_string_table_get(&astgen->string_table, d->identifier_string_id);
		}
		char* compound_name = d->flags & HSC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION ? "UNION" : "STRUCT";
		fprintf(f, "%s(#%u): %.*s {\n", compound_name, compound_type_idx, (int)name.size, name.data);
		fprintf(f, "\tsize: %zu\n", d->size);
		fprintf(f, "\talign: %zu\n", d->align);
		fprintf(f, "\tfields: {\n");
		for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
			HscCompoundField* field = &astgen->compound_fields[d->fields_start_idx + field_idx];
			HscString data_type_name = hsc_data_type_string(astgen, field->data_type);
			fprintf(f, "\t\t%.*s ", (int)data_type_name.size, data_type_name.data);
			if (field->identifier_string_id.idx_plus_one) {
				HscString identifier = hsc_string_table_get(&astgen->string_table, field->identifier_string_id);
				fprintf(f, "%.*s\n", (int)identifier.size, identifier.data);
			} else {
				fprintf(f, "\n");
			}
		}
		fprintf(f, "\t}\n");
		fprintf(f, "}\n");
	}

	for (U32 array_type_idx = 0; array_type_idx < astgen->array_data_types_count; array_type_idx += 1) {
		HscArrayDataType* d = &astgen->array_data_types[array_type_idx];
		HscString data_type_name = hsc_data_type_string(astgen, d->element_data_type);

		HscConstant constant = hsc_constant_table_get(&astgen->constant_table, d->size_constant_id);

		U64 count;
		HSC_DEBUG_ASSERT(hsc_constant_as_uint(constant, &count), "internal error: expected to be a unsigned int");

		fprintf(f, "ARRAY(#%u): %.*s[%zu]\n", array_type_idx, (int)data_type_name.size, data_type_name.data, count);
	}

	for (U32 typedefs_idx = 0; typedefs_idx < astgen->typedefs_count; typedefs_idx += 1) {
		HscTypedef* d = &astgen->typedefs[typedefs_idx];
		HscString name = hsc_string_table_get(&astgen->string_table, d->identifier_string_id);
		HscString aliased_data_type_name = hsc_data_type_string(astgen, d->aliased_data_type);
		fprintf(f, "typedef(#%u) %.*s %.*s\n", typedefs_idx, (int)aliased_data_type_name.size, aliased_data_type_name.data, (int)name.size, name.data);
	}

	for (U32 variable_idx = 0; variable_idx < astgen->global_variables_count; variable_idx += 1) {
		HscVariable* variable = &astgen->global_variables[variable_idx];

		char buf[1024];
		hsc_variable_to_string(astgen, variable, buf, sizeof(buf), false);
		fprintf(f, "GLOBAL_VARIABLE(#%u): %s", variable_idx, buf);
		fprintf(f, " = ");
		hsc_constant_print(astgen, variable->initializer_constant_id, f);
		fprintf(f, "\n");
	}

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
				HscVariable* param = &astgen->function_params_and_variables[function->params_start_idx + param_idx];
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
	HSC_ASSERT_ARRAY_BOUNDS(ir->basic_blocks_count, ir->basic_blocks_cap);
	HscIRBasicBlock* basic_block = &ir->basic_blocks[ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count];
	ir->basic_blocks_count += 1;
	ir_function->basic_blocks_count += 1;
	basic_block->instructions_start_idx = ir_function->instructions_count;
	return basic_block;
}

U16 hsc_ir_add_value(HscIR* ir, HscIRFunction* ir_function, HscDataType data_type) {
	HSC_ASSERT_ARRAY_BOUNDS(ir->values_count, ir->values_cap);
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
	HSC_ASSERT_ARRAY_BOUNDS(ir->instructions_count, ir->instructions_cap);
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

void hsc_ir_remove_last_instruction(HscIR* ir) {
	HscIRFunction* ir_function = &ir->functions[ir->functions_count];
	HscIRBasicBlock* basic_block = &ir->basic_blocks[ir->basic_blocks_count - 1];

	U16 operands_count = ir->instructions[ir->instructions_count - 1].operands_count;

	ir->instructions_count -= 1;
	ir->operands_count -= operands_count;
	ir_function->instructions_count -= 1;
	ir_function->operands_count -= operands_count;
	basic_block->instructions_count -= 1;
}

HscIROperand* hsc_ir_add_operands_many(HscIR* ir, HscIRFunction* ir_function, U32 amount) {
	HSC_ASSERT_ARRAY_BOUNDS(ir_function->operands_start_idx + (U32)ir_function->operands_count + amount - 1, ir->operands_cap);
	HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)ir_function->operands_count];
	ir->operands_count += amount;
	ir_function->operands_count += amount;
	return operands;
}

void hsc_ir_shrink_last_operands_count(HscIR* ir, HscIRFunction* ir_function, HscIROperand* operands, U32 new_amount) {
	HscIRInstr* instruction = &ir->instructions[ir->instructions_count - 1];
	U32 amount = instruction->operands_count;
	HSC_DEBUG_ASSERT(amount >= new_amount, "internal error: new amount is larger than the original");
	U32 shrink_by = amount - new_amount;
	ir->operands_count -= shrink_by;
	ir_function->operands_count -= shrink_by;
	instruction->operands_count -= shrink_by;
}

HscIRBasicBlock* hsc_ir_generate_instructions(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr);

HscIRBasicBlock* hsc_ir_generate_instructions_from_intrinsic_function(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr, HscFunction* function, U32 function_idx, HscExpr* call_args_expr) {
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
			HscVariable* variable = &astgen->function_params_and_variables[function->params_start_idx + HSC_IR_OPERAND_VARIABLE_IDX(ir_operand)];
			return variable->data_type;
		};
		case HSC_IR_OPERAND_GLOBAL_VARIABLE: {
			HscVariable* variable = &astgen->global_variables[HSC_IR_OPERAND_VARIABLE_IDX(ir_operand)];
			return variable->data_type;
		};
		default:
			return (HscDataType)ir_operand;
	}
}

void hsc_ir_generate_convert_to_bool(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIROperand cond_operand) {
	HscDataType cond_data_type = hsc_typedef_resolve(astgen, hsc_ir_operand_data_type(ir, astgen, ir_function, cond_operand));
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

HscIRBasicBlock* hsc_ir_generate_condition_expr(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* cond_expr) {
	basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, cond_expr);
	HscIROperand cond_operand = ir->last_operand;
	HscDataType cond_data_type = hsc_ir_operand_data_type(ir, astgen, ir_function, cond_operand);
	if (cond_data_type != HSC_DATA_TYPE_BOOL) {
		hsc_ir_generate_convert_to_bool(ir, astgen, ir_function, cond_operand);
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

void hsc_ir_generate_load(HscIR* ir, HscIRFunction* ir_function, HscDataType data_type, HscIROperand src_operand) {
	HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 2);
	U16 return_value_idx = hsc_ir_add_value(ir, ir_function, data_type);
	operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = src_operand;
	hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_LOAD, operands, 2);

	ir->last_operand = operands[0];
}

void hsc_ir_generate_store(HscIR* ir, HscIRFunction* ir_function, HscIROperand dst_operand, HscIROperand src_operand) {
	HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 2);
	operands[0] = dst_operand;
	operands[1] = src_operand;
	hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_STORE, operands, 2);
}

void hsc_ir_finalize_access_chain_instruction(HscIR* ir, HscIRFunction* ir_function, HscDataType data_type) {
	U16 operands_count = ir->instructions[ir->instructions_count - 1].operands_count;
	HscIROperand* operands = &ir->operands[ir->operands_count - operands_count];
	if (operands_count == 3) {
		//
		// no accesses where generated so remove the access chain instruction and
		// return the original source of the access chain.
		hsc_ir_remove_last_instruction(ir);
		ir->last_operand = operands[1];
		return;
	}

	HscIRValue* value = &ir->values[ir_function->values_start_idx + HSC_IR_OPERAND_VALUE_IDX(operands[0])];
	value->data_type = data_type;
	operands[2] = data_type;
}

HscIROperand* hsc_ir_start_access_chain_instruction(HscIR* ir, HscIRFunction* ir_function, U32 count) {
	HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, count + 3);
	hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_ACCESS_CHAIN, operands, count + 3);

	U16 return_value_idx = hsc_ir_add_value(ir, ir_function, HSC_DATA_TYPE_VOID);
	operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = ir->last_operand;

	ir->last_operand = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
	return operands;
}

void hsc_ir_generate_bitcast(HscIR* ir, HscIRFunction* ir_function, HscDataType dst_data_type, HscIROperand src_operand) {
	HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 3);
	hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BITCAST, operands, 3);

	U16 return_value_idx = hsc_ir_add_value(ir, ir_function, dst_data_type);
	operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
	operands[1] = dst_data_type;
	operands[2] = src_operand;

	ir->last_operand = operands[0];
}

void hsc_ir_bitcast_union_field(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscDataType union_data_type, U32 field_idx, HscIROperand src_operand) {
	HscCompoundDataType* compound_data_type = hsc_compound_data_type_get(astgen, union_data_type);
	HscCompoundField* field = &astgen->compound_fields[compound_data_type->fields_start_idx + field_idx];

	hsc_ir_generate_bitcast(ir, ir_function, field->data_type, src_operand);
}

HscIRBasicBlock* hsc_ir_generate_access_chain_instruction(HscIR* ir, HscAstGen* astgen, HscIRFunction* ir_function, HscIRBasicBlock* basic_block, HscExpr* expr, U32 count) {
	switch (expr->type) {
		case HSC_EXPR_TYPE_FIELD_ACCESS: {
			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			U32 child_count = count + 1;
			if (HSC_DATA_TYPE_IS_UNION(left_expr->data_type)) {
				child_count = 0;
			}
			basic_block = hsc_ir_generate_access_chain_instruction(ir, astgen, ir_function, basic_block, left_expr, child_count);
			S32 field_idx = expr->binary.right_expr_rel_idx;

			if (HSC_DATA_TYPE_IS_UNION(left_expr->data_type)) {
				hsc_ir_finalize_access_chain_instruction(ir, ir_function, left_expr->data_type);
				hsc_ir_bitcast_union_field(ir, astgen, ir_function, left_expr->data_type, field_idx, ir->last_operand);
				if (count != 0) {
					hsc_ir_start_access_chain_instruction(ir, ir_function, count);
				}
			} else {
				HscConstantId constant_id = hsc_constant_table_deduplicate_basic(&astgen->constant_table, astgen, HSC_DATA_TYPE_S32, &field_idx);
				ir->operands[ir->operands_count - count - 1] = HSC_IR_OPERAND_CONSTANT_INIT(constant_id.idx_plus_one);
			}
			break;
		};
		case HSC_EXPR_TYPE_ARRAY_SUBSCRIPT: {
			HscExpr* right_expr = expr - expr->binary.right_expr_rel_idx;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, right_expr);
			HscIROperand right_operand = ir->last_operand;

			HscExpr* left_expr = expr - expr->binary.left_expr_rel_idx;
			basic_block = hsc_ir_generate_access_chain_instruction(ir, astgen, ir_function, basic_block, left_expr, count + 1);

			ir->operands[ir->operands_count - count - 1] = right_operand;
			break;
		};
		default: {
			ir->do_not_load_variable = true;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, expr);

			hsc_ir_start_access_chain_instruction(ir, ir_function, count);
			break;
		};
	}
	return basic_block;
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
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 1);
			operands[0] = ir->last_operand;
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_FUNCTION_RETURN, operands, 1);
			break;
		};
		case HSC_EXPR_TYPE_UNARY_OP(PLUS): {
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);
			break;
		};
		case HSC_EXPR_TYPE_UNARY_OP(LOGICAL_NOT): op_code = HSC_IR_OP_CODE_UNARY_OP(LOGICAL_NOT); goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(BIT_NOT): op_code = HSC_IR_OP_CODE_UNARY_OP(BIT_NOT); goto UNARY;
		case HSC_EXPR_TYPE_UNARY_OP(NEGATE): op_code = HSC_IR_OP_CODE_UNARY_OP(NEGATE); goto UNARY;
UNARY:
		{
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;
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
		case HSC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT):
		case HSC_EXPR_TYPE_UNARY_OP(PRE_DECREMENT):
		case HSC_EXPR_TYPE_UNARY_OP(POST_INCREMENT):
		case HSC_EXPR_TYPE_UNARY_OP(POST_DECREMENT):
		{
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;

			ir->do_not_load_variable = true;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);
			HscIROperand target_operand = ir->last_operand;

			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);
			HscIROperand value_operand = ir->last_operand;

			HscIROpCode op_code;
			if (expr->type == HSC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT) || expr->type == HSC_EXPR_TYPE_UNARY_OP(POST_INCREMENT)) {
				op_code = HSC_IR_OP_CODE_BINARY_OP(ADD);
			} else {
				op_code = HSC_IR_OP_CODE_BINARY_OP(SUBTRACT);
			}

			HscDataType resolved_data_type = hsc_typedef_resolve(astgen, expr->data_type);
			HSC_DEBUG_ASSERT(HSC_DATA_TYPE_IS_BASIC(resolved_data_type), "internal error: expected basic type");

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 3);
			U16 result_value_idx = hsc_ir_add_value(ir, ir_function, resolved_data_type);
			operands[0] = HSC_IR_OPERAND_VALUE_INIT(result_value_idx);
			operands[1] = value_operand;
			operands[2] = HSC_IR_OPERAND_CONSTANT_INIT(astgen->basic_type_one_constant_ids[resolved_data_type].idx_plus_one);
			hsc_ir_add_instruction(ir, ir_function, op_code, operands, 3);
			hsc_ir_generate_store(ir, ir_function, target_operand, operands[0]);

			if (expr->type == HSC_EXPR_TYPE_UNARY_OP(PRE_INCREMENT) || expr->type == HSC_EXPR_TYPE_UNARY_OP(PRE_DECREMENT)) {
				ir->last_operand = operands[0];
			} else {
				ir->last_operand = value_operand;
			}

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

			//HscIROperand* default_branch_operands;
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
			bool is_do_while;
			if (expr->type == HSC_EXPR_TYPE_STMT_FOR) {
				is_do_while = false;
				init_expr = &expr[expr->for_.init_expr_rel_idx];
				cond_expr = &expr[expr->for_.cond_expr_rel_idx];
				inc_expr = &expr[expr->for_.inc_expr_rel_idx];
				loop_stmt = &expr[expr->for_.loop_stmt_rel_idx];
			} else {
				is_do_while = expr->while_.cond_expr_rel_idx > expr->while_.loop_stmt_rel_idx;
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

			HscIROperand cond_operand;
			if (!is_do_while) {
				basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, cond_expr);
				cond_operand = ir->last_operand;
			}

			HscIROperand* loop_merge_operands = hsc_ir_add_operands_many(ir, ir_function, 2);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_LOOP_MERGE, loop_merge_operands, 2);

			HscIROperand* cond_branch_operands;
			if (is_do_while) {
				cond_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, cond_branch_operands, 1);
			} else {
				cond_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 3);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);
				cond_branch_operands[0] = cond_operand;
			}

			HscIRBranchState prev_branch_state = ir->branch_state;
			ir->branch_state.break_branch_linked_list_head = -1;
			ir->branch_state.break_branch_linked_list_tail = -1;
			ir->branch_state.continue_branch_linked_list_head = -1;
			ir->branch_state.continue_branch_linked_list_tail = -1;

			HscIRBasicBlock* loop_basic_block = hsc_ir_add_basic_block(ir, ir_function);
			hsc_ir_generate_instructions(ir, astgen, ir_function, loop_basic_block, loop_stmt);
			HscIROperand loop_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, loop_basic_block));
			cond_branch_operands[is_do_while ? 0 : 1] = loop_basic_block_operand;

			HscIROpCode last_op_code = ir->instructions[ir->instructions_count - 1].op_code;
			HscIROperand* loop_branch_operands = NULL;
			if (last_op_code != HSC_IR_OP_CODE_BRANCH && last_op_code != HSC_IR_OP_CODE_FUNCTION_RETURN) {
				loop_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 1);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, loop_branch_operands, 1);
			}

			basic_block = hsc_ir_add_basic_block(ir, ir_function);
			HscIROperand continue_basic_block_operand = HSC_IR_OPERAND_BASIC_BLOCK_INIT(hsc_ir_basic_block_idx(ir, ir_function, basic_block));
			if (loop_branch_operands) {
				loop_branch_operands[0] = continue_basic_block_operand;
			}
			loop_merge_operands[1] = continue_basic_block_operand;
			if (inc_expr) {
				basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, inc_expr);
			}

			if (is_do_while) {
				basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, cond_expr);
				cond_operand = ir->last_operand;

				cond_branch_operands = hsc_ir_add_operands_many(ir, ir_function, 3);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH_CONDITIONAL, cond_branch_operands, 3);

				cond_branch_operands[0] = cond_operand;
				cond_branch_operands[1] = starting_basic_block_operand;
			} else {
				operands = hsc_ir_add_operands_many(ir, ir_function, 1);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_BRANCH, operands, 1);
				operands[0] = starting_basic_block_operand;
			}

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

			ir->do_not_load_variable = true;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, left_expr);
			HscIROperand left_operand = ir->last_operand;

			ir->assign_data_type = hsc_ir_operand_data_type(ir, astgen, ir_function, left_operand);
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
				} else if (left_expr->type == HSC_EXPR_TYPE_GLOBAL_VARIABLE) {
					dst_operand = HSC_IR_OPERAND_GLOBAL_VARIABLE_INIT(left_expr->variable.idx);
				} else {
					dst_operand = operands[1];
				}

				hsc_ir_generate_store(ir, ir_function, dst_operand, operands[0]);
			}

			ir->last_operand = operands[0];
			break;
		};
		case HSC_EXPR_TYPE_BINARY_OP(LOGICAL_AND):
		case HSC_EXPR_TYPE_BINARY_OP(LOGICAL_OR):
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
			U32 success_idx = expr->type != HSC_EXPR_TYPE_BINARY_OP(LOGICAL_AND);
			U32 converging_idx = expr->type == HSC_EXPR_TYPE_BINARY_OP(LOGICAL_AND);

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
			HscFunction* function = &astgen->functions[function_expr->function.idx];
			if (function_expr->function.idx < HSC_FUNCTION_IDX_USER_START) {
				basic_block = hsc_ir_generate_instructions_from_intrinsic_function(ir, astgen, ir_function, basic_block, expr, function, function_expr->function.idx, call_args_expr);
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
		case HSC_EXPR_TYPE_FIELD_ACCESS:
		case HSC_EXPR_TYPE_ARRAY_SUBSCRIPT:
		{
			bool do_load = !ir->do_not_load_variable;
			ir->do_not_load_variable = false;

			basic_block = hsc_ir_generate_access_chain_instruction(ir, astgen, ir_function, basic_block, expr, 0);
			if (ir->instructions[ir->instructions_count - 1].op_code == HSC_IR_OP_CODE_ACCESS_CHAIN) {
				hsc_ir_finalize_access_chain_instruction(ir, ir_function, expr->data_type);
			}

			if (do_load) {
				hsc_ir_generate_load(ir, ir_function, expr->data_type, ir->last_operand);
			}
			break;
		};
		case HSC_EXPR_TYPE_CURLY_INITIALIZER: {
			HscExpr* variable_expr = &expr[expr->curly_initializer.first_expr_rel_idx];
			HSC_DEBUG_ASSERT(variable_expr->type == HSC_EXPR_TYPE_LOCAL_VARIABLE, "internal error: expected the first node of the compound literial to be the hidden variable expression that we can mutate");
			HscIROperand variable_operand = HSC_IR_OPERAND_LOCAL_VARIABLE_INIT(variable_expr->variable.idx);

			//
			// store a zeroed value in the hidden local variable where the compound data type gets constructed
			HscConstantId zeroed_constant_id = hsc_constant_table_deduplicate_zero(&astgen->constant_table, astgen, expr->data_type);
			hsc_ir_generate_store(ir, ir_function, variable_operand, HSC_IR_OPERAND_CONSTANT_INIT(zeroed_constant_id.idx_plus_one));

			HscExpr* initializer_expr = variable_expr;
			while (1) {
				U32 expr_rel_idx = initializer_expr->next_expr_rel_idx;
				if (expr_rel_idx == 0) {
					break;
				}
				initializer_expr = &initializer_expr[expr_rel_idx];

				HSC_DEBUG_ASSERT(initializer_expr->type == HSC_EXPR_TYPE_DESIGNATED_INITIALIZER, "internal error: expected a designated initializer");

				ir->last_operand = variable_operand;

				HscIROperand dst_operand;
				HscDataType data_type = expr->data_type;
				{
					U64* entry_indices = &astgen->entry_indices[initializer_expr->alt_next_expr_rel_idx];
					U32 entry_indices_count = initializer_expr->designated_initializer.entry_indices_count;

					HscIROperand* operands = hsc_ir_start_access_chain_instruction(ir, ir_function, entry_indices_count);
					U32 operand_idx = 3;
					for (U32 entry_indices_idx = 0; entry_indices_idx < entry_indices_count; entry_indices_idx += 1) {
						data_type = hsc_typedef_resolve(astgen, data_type);
						U64 entry_idx = entry_indices[entry_indices_idx];

						if (HSC_DATA_TYPE_IS_UNION(data_type)) {
							hsc_ir_shrink_last_operands_count(ir, ir_function, operands, operand_idx);
							hsc_ir_finalize_access_chain_instruction(ir, ir_function, data_type);
							hsc_ir_bitcast_union_field(ir, astgen, ir_function, data_type, entry_idx, ir->last_operand);
							if (entry_indices_idx + 1 < entry_indices_count) {
								operands = hsc_ir_start_access_chain_instruction(ir, ir_function, entry_indices_count - entry_indices_idx);
								operand_idx = 3;
							}
						} else {
							U32 TODO_int_64_support_plz = entry_idx;

							HscConstantId entry_constant_id = hsc_constant_table_deduplicate_basic(&astgen->constant_table, astgen, HSC_DATA_TYPE_U32, &TODO_int_64_support_plz);
							operands[operand_idx] = HSC_IR_OPERAND_CONSTANT_INIT(entry_constant_id.idx_plus_one);
							operand_idx += 1;
						}

						if (HSC_DATA_TYPE_IS_ARRAY(data_type)) {
							HscArrayDataType* array_data_type = hsc_array_data_type_get(astgen, data_type);
							data_type = array_data_type->element_data_type;
						} else if (HSC_DATA_TYPE_IS_COMPOUND_TYPE(data_type)) {
							HscCompoundDataType* compound_data_type = hsc_compound_data_type_get(astgen, data_type);
							HscCompoundField* field = &astgen->compound_fields[compound_data_type->fields_start_idx + entry_idx];
							data_type = field->data_type;
						}
					}

					if (ir->instructions[ir->instructions_count - 1].op_code == HSC_IR_OP_CODE_ACCESS_CHAIN) {
						hsc_ir_shrink_last_operands_count(ir, ir_function, operands, operand_idx);
						hsc_ir_finalize_access_chain_instruction(ir, ir_function, data_type);
					}

					dst_operand = ir->last_operand;
				}


				HscIROperand value_operand;
				if (initializer_expr->designated_initializer.value_expr_rel_idx) {
					HscExpr* value_expr = &initializer_expr[initializer_expr->designated_initializer.value_expr_rel_idx];
					basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, value_expr);
					value_operand = ir->last_operand;
				} else {
					HscConstantId zeroed_constant_id = hsc_constant_table_deduplicate_zero(&astgen->constant_table, astgen, data_type);
					value_operand = HSC_IR_OPERAND_CONSTANT_INIT(zeroed_constant_id.idx_plus_one);
				}

				hsc_ir_generate_store(ir, ir_function, dst_operand, value_operand);
			}

			hsc_ir_generate_load(ir, ir_function, expr->data_type, variable_operand);
			break;
		};
		case HSC_EXPR_TYPE_CAST: {
			HscExpr* unary_expr = expr - expr->unary.expr_rel_idx;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, unary_expr);

			HscBasicTypeClass dst_type_class = hsc_basic_type_class(HSC_DATA_TYPE_SCALAR(expr->data_type));
			if (dst_type_class == HSC_BASIC_TYPE_CLASS_BOOL) {
				hsc_ir_generate_convert_to_bool(ir, astgen, ir_function, ir->last_operand);
			} else {
				HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 3);
				hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_CONVERT, operands, 3);

				U16 return_value_idx = hsc_ir_add_value(ir, ir_function, expr->data_type);
				operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
				operands[1] = expr->data_type;
				operands[2] = ir->last_operand;

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
				HscVariable* variable = &astgen->function_params_and_variables[function->params_start_idx + expr->variable.idx];
				hsc_ir_generate_load(ir, ir_function, variable->data_type, HSC_IR_OPERAND_LOCAL_VARIABLE_INIT(expr->variable.idx));
			}
			break;
		};
		case HSC_EXPR_TYPE_GLOBAL_VARIABLE: {
			if (ir->do_not_load_variable) {
				ir->last_operand = HSC_IR_OPERAND_GLOBAL_VARIABLE_INIT(expr->variable.idx);
				ir->do_not_load_variable = false;
			} else {
				HscVariable* variable = &astgen->global_variables[expr->variable.idx];
				hsc_ir_generate_load(ir, ir_function, variable->data_type, HSC_IR_OPERAND_GLOBAL_VARIABLE_INIT(expr->variable.idx));
			}
			break;
		};
		case HSC_EXPR_TYPE_BINARY_OP(TERNARY): {
			HscExpr* cond_expr = expr - expr->ternary.cond_expr_rel_idx;
			basic_block = hsc_ir_generate_condition_expr(ir, astgen, ir_function, basic_block, cond_expr);
			HscIROperand cond_operand = ir->last_operand;

			HscExpr* true_expr = expr - expr->ternary.true_expr_rel_idx;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, true_expr);
			HscIROperand true_operand = ir->last_operand;

			HscExpr* false_expr = expr - expr->ternary.false_expr_rel_idx;
			basic_block = hsc_ir_generate_instructions(ir, astgen, ir_function, basic_block, false_expr);
			HscIROperand false_operand = ir->last_operand;

			HscIROperand* operands = hsc_ir_add_operands_many(ir, ir_function, 4);
			hsc_ir_add_instruction(ir, ir_function, HSC_IR_OP_CODE_SELECT, operands, 4);

			U16 return_value_idx = hsc_ir_add_value(ir, ir_function, expr->data_type);
			operands[0] = HSC_IR_OPERAND_VALUE_INIT(return_value_idx);
			operands[1] = cond_operand;
			operands[2] = true_operand;
			operands[3] = false_operand;

			ir->last_operand = operands[0];
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
	for (U32 function_idx = HSC_FUNCTION_IDX_USER_START; function_idx < astgen->functions_count; function_idx += 1) {
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
			fprintf(f, "var%u", HSC_IR_OPERAND_VARIABLE_IDX(operand));
			break;
		case HSC_IR_OPERAND_GLOBAL_VARIABLE:
			fprintf(f, "global_var%u", HSC_IR_OPERAND_VARIABLE_IDX(operand));
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
		fprintf(f, "Constant(c%u): ", idx);
		HscConstantId constant_id = { .idx_plus_one = idx + 1 };
		hsc_constant_print(astgen, constant_id, stdout);
		fprintf(f, "\n");
	}

	for (U32 function_idx = HSC_FUNCTION_IDX_USER_START; function_idx < astgen->functions_count; function_idx += 1) {
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
					case HSC_IR_OP_CODE_LOAD:
					{
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						fprintf(f, "\t\t");
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, " = OP_LOAD: ");
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
					case HSC_IR_OP_CODE_ACCESS_CHAIN: {
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						fprintf(f, "\t\t");
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, " = OP_ACCESS_CHAIN: ");
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
					case HSC_IR_OP_CODE_CONVERT: {
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						fprintf(f, "\t\t");
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, " = OP_CONVERT: ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_BITCAST: {
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						fprintf(f, "\t\t");
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, " = OP_BITCAST: ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[2], f);
						fprintf(f, "\n");
						break;
					};
					case HSC_IR_OP_CODE_UNREACHABLE: {
						fprintf(f, "\t\tOP_UNREACHABLE:\n");
						break;
					};
					case HSC_IR_OP_CODE_SELECT: {
						HscIROperand* operands = &ir->operands[ir_function->operands_start_idx + (U32)instruction->operands_start_idx];
						fprintf(f, "\t\t");
						hsc_ir_print_operand(ir, astgen, operands[0], f);
						fprintf(f, " = OP_SELECT: ");
						hsc_ir_print_operand(ir, astgen, operands[1], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[2], f);
						fprintf(f, ", ");
						hsc_ir_print_operand(ir, astgen, operands[3], f);
						fprintf(f, "\n");
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
		HscVariable* params = &c->astgen.function_params_and_variables[function->params_start_idx];
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
	HscVariable* params = &c->astgen.function_params_and_variables[function->params_start_idx];
	for (U32 j = 0; j < entry->data_types_count; j += 1) {
		data_types[j + 1] = params[j].data_type;
	}

	return entry->spirv_id;
}

U32 hsc_spirv_type_table_deduplicate_variable(HscCompiler* c, HscSpirvTypeTable* table, HscDataType data_type, HscSpirvTypeKind kind) {
	//
	// TODO make this a hash table look for speeeds
	for (U32 i = 0; i < table->entries_count; i += 1) {
		HscSpirvTypeEntry* entry = &table->entries[i];
		if (entry->kind != kind) {
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
	entry->kind = kind;

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
	data_type = hsc_typedef_resolve(&c->astgen, data_type);
	if (data_type < HSC_DATA_TYPE_MATRIX_END) {
		return data_type + 1;
	} else {
		switch (data_type & 0xff) {
			case HSC_DATA_TYPE_STRUCT:
			case HSC_DATA_TYPE_UNION:
				return c->spirv.compound_type_base_id + HSC_DATA_TYPE_IDX(data_type);
			case HSC_DATA_TYPE_ARRAY:
				return c->spirv.array_type_base_id + HSC_DATA_TYPE_IDX(data_type);
			default:
				HSC_ABORT("unhandled data type '%u'", data_type);
		}
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
		case HSC_IR_OPERAND_LOCAL_VARIABLE: return c->spirv.local_variable_base_spirv_id + HSC_IR_OPERAND_VARIABLE_IDX(ir_operand);
		case HSC_IR_OPERAND_GLOBAL_VARIABLE: return c->spirv.global_variable_base_spirv_id + HSC_IR_OPERAND_VARIABLE_IDX(ir_operand);
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

#define hsc_spirv_instr_add_operands_string_lit(c, string) hsc_spirv_instr_add_operands_string(c, string, sizeof(string) - 1)
#define hsc_spirv_instr_add_operands_string_c(c, string) hsc_spirv_instr_add_operands_string(c, string, strlen(string))
void hsc_spirv_instr_add_operands_string(HscCompiler* c, char* string, U32 string_size) {
	for (U32 i = 0; i < string_size; i += 4) {
		U32 word = 0;
		word |= string[i] << 0;
		if (i + 1 < string_size) word |= string[i + 1] << 8;
		if (i + 2 < string_size) word |= string[i + 2] << 16;
		if (i + 3 < string_size) word |= string[i + 3] << 24;
		hsc_spirv_instr_add_operand(c, word);
	}
	if (string_size % 4 == 0) {
		hsc_spirv_instr_add_operand(c, 0);
	}
}

void hsc_spirv_instr_end(HscCompiler* c) {
	HSC_DEBUG_ASSERT(c->spirv.instr_op != HSC_SPIRV_OP_NO_OP, "internal error: hsc_spirv_instr_start has not been called when making an instruction");

	U32* out;
	U32* count_ptr;
	switch (c->spirv.instr_op) {
		case HSC_SPIRV_OP_CAPABILITY:
		case HSC_SPIRV_OP_EXTENSION:
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
		case HSC_SPIRV_OP_TYPE_ARRAY:
		case HSC_SPIRV_OP_TYPE_STRUCT:
		case HSC_SPIRV_OP_TYPE_POINTER:
		case HSC_SPIRV_OP_TYPE_FUNCTION:
		case HSC_SPIRV_OP_CONSTANT_TRUE:
		case HSC_SPIRV_OP_CONSTANT_FALSE:
		case HSC_SPIRV_OP_CONSTANT:
		case HSC_SPIRV_OP_CONSTANT_COMPOSITE:
		case HSC_SPIRV_OP_CONSTANT_NULL:
TYPES_VARIABLES_CONSTANTS:
			HSC_DEBUG_ASSERT(c->spirv.out_types_variables_constants_count < c->spirv.out_types_variables_constants_cap, "internal error: spirv types variables constants array has been filled up");
			out = &c->spirv.out_types_variables_constants[c->spirv.out_types_variables_constants_count];
			count_ptr = &c->spirv.out_types_variables_constants_count;
			break;
		case HSC_SPIRV_OP_FUNCTION:
		case HSC_SPIRV_OP_FUNCTION_PARAMETER:
		case HSC_SPIRV_OP_FUNCTION_END:
		case HSC_SPIRV_OP_COMPOSITE_CONSTRUCT:
		case HSC_SPIRV_OP_ACCESS_CHAIN:
		case HSC_SPIRV_OP_CONVERT_F_TO_U:
		case HSC_SPIRV_OP_CONVERT_F_TO_S:
		case HSC_SPIRV_OP_CONVERT_S_TO_F:
		case HSC_SPIRV_OP_CONVERT_U_TO_F:
		case HSC_SPIRV_OP_U_CONVERT:
		case HSC_SPIRV_OP_S_CONVERT:
		case HSC_SPIRV_OP_F_CONVERT:
		case HSC_SPIRV_OP_BITCAST:
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
	hsc_spirv_instr_add_operand(c, id);
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
		HscVariable* params = &c->astgen.function_params_and_variables[function->params_start_idx];
		for (U32 i = 0; i < function->params_count; i += 1) {
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, params[i].data_type));
		}
		hsc_spirv_instr_end(c);
		c->spirv.next_id += 1;
	}

	return function_type_id;
}

U32 hsc_spirv_generate_variable_type(HscCompiler* c, HscDataType data_type, bool is_static) {
	HscSpirvTypeKind type_kind = is_static ? HSC_SPIRV_TYPE_KIND_STATIC_VARIABLE : HSC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE;
	U32 type_id = hsc_spirv_type_table_deduplicate_variable(c, &c->spirv.type_table, data_type, type_kind);
	if (type_id == c->spirv.next_id) {
		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_POINTER);
		hsc_spirv_instr_add_result_operand(c);
		U32 storage_class = is_static ? HSC_SPIRV_STORAGE_CLASS_PRIVATE : HSC_SPIRV_STORAGE_CLASS_FUNCTION;
		hsc_spirv_instr_add_operand(c, storage_class);
		hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, data_type));
		hsc_spirv_instr_end(c);
		c->spirv.next_id += 1;
	}

	return type_id;
}

void hsc_spirv_generate_select(HscCompiler* c, U32 result_spirv_operand, HscDataType dst_type, U32 cond_value_spirv_operand, U32 a_spirv_operand, U32 b_spirv_operand) {
	hsc_spirv_instr_start(c, HSC_SPIRV_OP_SELECT);
	hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, dst_type));
	hsc_spirv_instr_add_operand(c, result_spirv_operand);
	hsc_spirv_instr_add_operand(c, cond_value_spirv_operand);
	hsc_spirv_instr_add_operand(c, a_spirv_operand);
	hsc_spirv_instr_add_operand(c, b_spirv_operand);
	hsc_spirv_instr_end(c);
}

void hsc_spirv_generate_convert(HscCompiler* c, HscSpirvOp spirv_convert_op, U32 result_spirv_operand, HscDataType dst_type, U32 value_spirv_operand) {
	hsc_spirv_instr_start(c, spirv_convert_op);
	hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, dst_type));
	hsc_spirv_instr_add_operand(c, result_spirv_operand);
	hsc_spirv_instr_add_operand(c, value_spirv_operand);
	hsc_spirv_instr_end(c);
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

	c->spirv.local_variable_base_spirv_id = c->spirv.next_id;
	c->spirv.next_id += function->params_count + function->variables_count;

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
			hsc_spirv_instr_add_operands_string(c, (char*)name.data, name.size);
			hsc_spirv_instr_add_operand(c, frag_color_spirv_id);
			for (U32 idx = function->used_static_variables_start_idx; idx < function->used_static_variables_start_idx + function->used_static_variables_count; idx += 1) {
				HscDecl decl = c->astgen.used_static_variables[idx];
				U32 spirv_base_id = HSC_DECL_IS_LOCAL_VARIABLE(decl)
					? c->spirv.local_variable_base_spirv_id
					: c->spirv.global_variable_base_spirv_id;
				hsc_spirv_instr_add_operand(c, spirv_base_id + HSC_DECL_IDX(decl));
			}
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

	//
	// generate the local variable types before we make the local variables as the global variables have a linear spirv id range
	{
		if (function->shader_stage == HSC_FUNCTION_SHADER_STAGE_NONE) {
			//
			// function parameters
			for (U32 variable_idx = 0; variable_idx < function->params_count; variable_idx += 1) {
				HscVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
				hsc_spirv_generate_variable_type(c, variable->data_type, false);
			}
		}

		//
		// local variables
		for (U32 variable_idx = function->params_count; variable_idx < function->variables_count; variable_idx += 1) {
			HscVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
			hsc_spirv_generate_variable_type(c, variable->data_type, variable->is_static);
		}
	}

	for (U32 basic_block_idx = ir_function->basic_blocks_start_idx; basic_block_idx < ir_function->basic_blocks_start_idx + (U32)ir_function->basic_blocks_count; basic_block_idx += 1) {
		HscIRBasicBlock* basic_block = &c->ir.basic_blocks[basic_block_idx];

		hsc_spirv_instr_start(c, HSC_SPIRV_OP_LABEL);
		hsc_spirv_instr_add_operand(c, c->spirv.basic_block_base_spirv_id + (basic_block_idx - ir_function->basic_blocks_start_idx));
		hsc_spirv_instr_end(c);

		if (basic_block_idx == ir_function->basic_blocks_start_idx) {
			//
			// function params
			for (U32 variable_idx = 0; variable_idx < function->params_count; variable_idx += 1) {
				if (function->shader_stage == HSC_FUNCTION_SHADER_STAGE_NONE) {
					HscVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
					U32 type_spirv_id = hsc_spirv_generate_variable_type(c, variable->data_type, false);
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_FUNCTION_PARAMETER);
					hsc_spirv_instr_add_operand(c, type_spirv_id);
					hsc_spirv_instr_add_operand(c, c->spirv.local_variable_base_spirv_id + variable_idx);
					hsc_spirv_instr_end(c);
				}
			}

			//
			// local variables
			for (U32 variable_idx = function->params_count; variable_idx < function->variables_count; variable_idx += 1) {
				HscVariable* variable = &c->astgen.function_params_and_variables[function->params_start_idx + variable_idx];
				U32 type_spirv_id = hsc_spirv_generate_variable_type(c, variable->data_type, variable->is_static);
				hsc_spirv_instr_start(c, HSC_SPIRV_OP_VARIABLE);
				hsc_spirv_instr_add_operand(c, type_spirv_id);
				hsc_spirv_instr_add_operand(c, c->spirv.local_variable_base_spirv_id + variable_idx);
				if (variable->is_static) {
					hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_PRIVATE);
					hsc_spirv_instr_add_converted_operand(c, HSC_IR_OPERAND_CONSTANT_INIT(variable->initializer_constant_id.idx_plus_one));
				} else {
					hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_FUNCTION);
				}
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
				case HSC_IR_OP_CODE_ACCESS_CHAIN: {
					U32 data_type_spirv_id = hsc_spirv_generate_variable_type(c, operands[2], false);

					hsc_spirv_instr_start(c, HSC_SPIRV_OP_ACCESS_CHAIN);
					hsc_spirv_instr_add_operand(c, data_type_spirv_id);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					for (U32 i = 3; i < instruction->operands_count; i += 1) {
						hsc_spirv_instr_add_converted_operand(c, operands[i]);
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
					HscDataType resolved_data_type = hsc_ir_operand_data_type(&c->ir, &c->astgen, ir_function, operands[1]);
					resolved_data_type = hsc_typedef_resolve(&c->astgen, resolved_data_type);
					HscBasicTypeClass type_class = hsc_basic_type_class(HSC_DATA_TYPE_SCALAR(resolved_data_type));
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
				case HSC_IR_OP_CODE_CONVERT: {
					HscDataType dst_type = operands[1];
					HscDataType src_type = hsc_ir_operand_data_type(&c->ir, &c->astgen, ir_function, operands[2]);
					dst_type = hsc_typedef_resolve(&c->astgen, dst_type);
					src_type = hsc_typedef_resolve(&c->astgen, src_type);
					HscBasicTypeClass dst_type_class = hsc_basic_type_class(HSC_DATA_TYPE_SCALAR(dst_type));
					HscBasicTypeClass src_type_class = hsc_basic_type_class(HSC_DATA_TYPE_SCALAR(src_type));

					U32 result_spirv_operand = hsc_spirv_convert_operand(c, operands[0]);
					U32 src_spirv_operand = hsc_spirv_convert_operand(c, operands[2]);
					switch (dst_type_class) {
						///////////////////////////////////////
						// case HSC_BASIC_TYPE_CLASS_BOOL:
						// ^^ this is handled in the HscIR, see hsc_ir_generate_convert_to_bool
						///////////////////////////////////////

						case HSC_BASIC_TYPE_CLASS_UINT:
							switch (src_type_class) {
								case HSC_BASIC_TYPE_CLASS_BOOL: {
									U32 true_spirv_operand = c->spirv.constant_base_id + c->astgen.basic_type_one_constant_ids[dst_type].idx_plus_one - 1;
									U32 false_spirv_operand = c->spirv.constant_base_id + c->astgen.basic_type_zero_constant_ids[dst_type].idx_plus_one - 1;
									hsc_spirv_generate_select(c, result_spirv_operand, dst_type, src_spirv_operand, true_spirv_operand, false_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_UINT:
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_U_CONVERT, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								case HSC_BASIC_TYPE_CLASS_SINT: {
									HscDataType signed_dst_type = hsc_data_type_unsigned_to_signed(dst_type);
									if (signed_dst_type != src_type) {
										hsc_spirv_generate_convert(c, HSC_SPIRV_OP_S_CONVERT, c->spirv.next_id, signed_dst_type, src_spirv_operand);
										src_spirv_operand = c->spirv.next_id;
										c->spirv.next_id += 1;
									}
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_BITCAST, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_FLOAT:
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_CONVERT_F_TO_U, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								default:
									HSC_UNREACHABLE();
							}
							break;
						case HSC_BASIC_TYPE_CLASS_SINT:
							switch (src_type_class) {
								case HSC_BASIC_TYPE_CLASS_BOOL: {
									U32 true_spirv_operand = c->spirv.constant_base_id + c->astgen.basic_type_one_constant_ids[dst_type].idx_plus_one - 1;
									U32 false_spirv_operand = c->spirv.constant_base_id + c->astgen.basic_type_zero_constant_ids[dst_type].idx_plus_one - 1;
									hsc_spirv_generate_select(c, result_spirv_operand, dst_type, src_spirv_operand, true_spirv_operand, false_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_UINT: {
									HscDataType unsigned_dst_type = hsc_data_type_signed_to_unsigned(dst_type);
									if (unsigned_dst_type != src_type) {
										hsc_spirv_generate_convert(c, HSC_SPIRV_OP_U_CONVERT, c->spirv.next_id, unsigned_dst_type, src_spirv_operand);
										src_spirv_operand = c->spirv.next_id;
										c->spirv.next_id += 1;
									}
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_BITCAST, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_SINT: {
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_S_CONVERT, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_FLOAT:
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_CONVERT_F_TO_S, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								default:
									HSC_UNREACHABLE();
							}
							break;
						case HSC_BASIC_TYPE_CLASS_FLOAT:
							switch (src_type_class) {
								case HSC_BASIC_TYPE_CLASS_BOOL: {
									U32 true_spirv_operand = c->spirv.constant_base_id + c->astgen.basic_type_one_constant_ids[dst_type].idx_plus_one - 1;
									U32 false_spirv_operand = c->spirv.constant_base_id + c->astgen.basic_type_zero_constant_ids[dst_type].idx_plus_one - 1;
									hsc_spirv_generate_select(c, result_spirv_operand, dst_type, src_spirv_operand, true_spirv_operand, false_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_UINT:
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_CONVERT_U_TO_F, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								case HSC_BASIC_TYPE_CLASS_SINT: {
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_CONVERT_S_TO_F, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								};
								case HSC_BASIC_TYPE_CLASS_FLOAT:
									hsc_spirv_generate_convert(c, HSC_SPIRV_OP_F_CONVERT, result_spirv_operand, dst_type, src_spirv_operand);
									break;
								default:
									HSC_UNREACHABLE();
							}
							break;
						default:
							HSC_UNREACHABLE();
					}

					break;
				};
				case HSC_IR_OP_CODE_BITCAST: {
					U32 type_spirv_id = hsc_spirv_generate_variable_type(c, operands[1], false);
					hsc_spirv_instr_start(c, HSC_SPIRV_OP_BITCAST);
					hsc_spirv_instr_add_operand(c, type_spirv_id);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[2]);
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
				case HSC_IR_OP_CODE_SELECT: {
					U32 return_value_idx = HSC_IR_OPERAND_VALUE_IDX(operands[0]);
					HscIRValue* return_value = &c->ir.values[ir_function->values_start_idx + return_value_idx];

					hsc_spirv_instr_start(c, HSC_SPIRV_OP_SELECT);
					hsc_spirv_instr_add_converted_operand(c, return_value->data_type);
					hsc_spirv_instr_add_converted_operand(c, operands[0]);
					hsc_spirv_instr_add_converted_operand(c, operands[1]);
					hsc_spirv_instr_add_converted_operand(c, operands[2]);
					hsc_spirv_instr_add_converted_operand(c, operands[3]);
					hsc_spirv_instr_end(c);
					break;
				};
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

void hsc_spirv_generate_basic_type_constants(HscCompiler* c) {
	HscConstantTable* constant_table = &c->astgen.constant_table;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[idx];
		if (!HSC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			continue;
		}

		if (entry->size == 0) {
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_CONSTANT_NULL);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, entry->data_type));
			hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + idx);
			hsc_spirv_instr_end(c);
		} else if (entry->data_type == HSC_DATA_TYPE_BOOL) {
			bool is_true = c->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_BOOL].idx_plus_one == idx + 1;
			hsc_spirv_instr_start(c, is_true ? HSC_SPIRV_OP_CONSTANT_TRUE : HSC_SPIRV_OP_CONSTANT_FALSE);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, HSC_DATA_TYPE_BOOL));
			hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + idx);
			hsc_spirv_instr_end(c);
		} else if (HSC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_CONSTANT);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, entry->data_type));
			hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + idx);

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
		}
	}
}

void hsc_spirv_generate_non_basic_type_constants(HscCompiler* c) {
	HscConstantTable* constant_table = &c->astgen.constant_table;
	for (U32 idx = 0; idx < constant_table->entries_count; idx += 1) {
		HscConstantEntry* entry = &constant_table->entries[idx];
		if (HSC_DATA_TYPE_IS_BASIC(entry->data_type)) {
			continue;
		}

		if (entry->size == 0) {
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_CONSTANT_NULL);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, entry->data_type));
			hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + idx);
			hsc_spirv_instr_end(c);
		} else {
			hsc_spirv_instr_start(c, HSC_SPIRV_OP_CONSTANT_COMPOSITE);
			hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, entry->data_type));
			hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + idx);

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
	U32 minor_version = 5;
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

	// generates the basic type constant before we make the array types (that use the constants)
	c->spirv.constant_base_id = c->spirv.next_id;
	c->spirv.next_id += c->astgen.constant_table.entries_count;
	hsc_spirv_generate_basic_type_constants(c);

	{
		c->spirv.array_type_base_id = c->spirv.next_id;
		c->spirv.next_id += c->astgen.array_data_types_count;

		c->spirv.compound_type_base_id = c->spirv.next_id;
		c->spirv.next_id += c->astgen.compound_data_types_count;

		for (U32 i = 0; i < c->astgen.ordered_data_types_count; i += 1) {
			HscDataType data_type = c->astgen.ordered_data_types[i];
			switch (data_type & 0xff) {
				case HSC_DATA_TYPE_STRUCT:
				case HSC_DATA_TYPE_UNION: {
					HscCompoundDataType* d = hsc_compound_data_type_get(&c->astgen, data_type);

					hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_STRUCT);
					hsc_spirv_instr_add_operand(c, c->spirv.compound_type_base_id + HSC_DATA_TYPE_IDX(data_type));
					if (HSC_DATA_TYPE_IS_UNION(data_type)) {
						HscCompoundField* largest_sized_field = &c->astgen.compound_fields[d->fields_start_idx + d->largest_sized_field_idx];
						hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, largest_sized_field->data_type));
					} else {
						for (U32 field_idx = 0; field_idx < d->fields_count; field_idx += 1) {
							HscCompoundField* field = &c->astgen.compound_fields[d->fields_start_idx + field_idx];
							hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, field->data_type));
						}
					}
					hsc_spirv_instr_end(c);
					break;
				};
				case HSC_DATA_TYPE_ARRAY: {
					HscArrayDataType* d = hsc_array_data_type_get(&c->astgen, data_type);

					hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_ARRAY);
					hsc_spirv_instr_add_operand(c, c->spirv.array_type_base_id + HSC_DATA_TYPE_IDX(data_type));
					hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, d->element_data_type));
					hsc_spirv_instr_add_operand(c, c->spirv.constant_base_id + d->size_constant_id.idx_plus_one - 1);
					hsc_spirv_instr_end(c);
					break;
				};
			}
		}
	}

	hsc_spirv_generate_non_basic_type_constants(c);

	{
		c->spirv.shader_stage_function_type_spirv_id = c->spirv.next_id;
		hsc_spirv_instr_start(c, HSC_SPIRV_OP_TYPE_FUNCTION);
		hsc_spirv_instr_add_result_operand(c);
		hsc_spirv_instr_add_operand(c, hsc_spirv_resolve_type_id(c, HSC_DATA_TYPE_VOID));
		hsc_spirv_instr_end(c);
	}

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_MEMORY_MODEL);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_ADDRESS_MODEL_PHYSICAL_STORAGE_BUFFER_64);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_MEMORY_MODEL_VULKAN);
	hsc_spirv_instr_end(c);

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_CAPABILITY);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_CAPABILITY_SHADER);
	hsc_spirv_instr_end(c);

	hsc_spirv_instr_start(c, HSC_SPIRV_OP_CAPABILITY);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL);
	hsc_spirv_instr_end(c);

	// TODO: i don't know if we can support unions if we don't use this.
	// this allows us to bitcast a pointer, so we can bitcast a union to it's field type.
	// this is the SPIRV side to VK_KHR_buffer_device_address.
	hsc_spirv_instr_start(c, HSC_SPIRV_OP_CAPABILITY);
	hsc_spirv_instr_add_operand(c, HSC_SPIRV_CAPABILITY_PHYSICAL_STORAGE_BUFFER);
	hsc_spirv_instr_end(c);

	//
	// generate the global variable types before we make the global variables as the global variables have a linear spirv id range
	for (U32 global_variable_idx = 0; global_variable_idx < c->astgen.global_variables_count; global_variable_idx += 1) {
		HscVariable* variable = &c->astgen.global_variables[global_variable_idx];
		hsc_spirv_generate_variable_type(c, variable->data_type, true);
	}

	c->spirv.global_variable_base_spirv_id = c->spirv.next_id;
	c->spirv.next_id += c->astgen.global_variables_count;
	for (U32 global_variable_idx = 0; global_variable_idx < c->astgen.global_variables_count; global_variable_idx += 1) {
		HscVariable* variable = &c->astgen.global_variables[global_variable_idx];
		U32 type_spirv_id = hsc_spirv_generate_variable_type(c, variable->data_type, true);
		hsc_spirv_instr_start(c, HSC_SPIRV_OP_VARIABLE);
		hsc_spirv_instr_add_operand(c, type_spirv_id);
		hsc_spirv_instr_add_operand(c, c->spirv.global_variable_base_spirv_id + global_variable_idx);
		hsc_spirv_instr_add_operand(c, HSC_SPIRV_STORAGE_CLASS_PRIVATE);
		hsc_spirv_instr_add_converted_operand(c, HSC_IR_OPERAND_CONSTANT_INIT(variable->initializer_constant_id.idx_plus_one));
		hsc_spirv_instr_end(c);
	}

	for (U32 function_idx = HSC_FUNCTION_IDX_USER_START; function_idx < c->astgen.functions_count; function_idx += 1) {
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

bool hsc_code_file_find_or_insert(HscAstGen* astgen, HscStringId path_string_id, HscCodeFileId* code_file_id_out, HscCodeFile** code_file_out) {
	U32* code_file_idx;
	if (hsc_hash_table_find_or_insert(&astgen->path_to_code_file_id_map, path_string_id.idx_plus_one, &code_file_idx)) {
		*code_file_id_out = ((HscCodeFileId) { .idx_plus_one = *code_file_idx + 1 });
		*code_file_out = &astgen->code_files[*code_file_idx];
		return true;
	}
	*code_file_idx = astgen->code_files_count;
	HSC_ASSERT_ARRAY_BOUNDS(astgen->code_files_count, astgen->code_files_cap);
	astgen->code_files_count += 1;

	HscCodeFile* code_file = &astgen->code_files[*code_file_idx];
	code_file->path_string = hsc_string_table_get(&astgen->string_table, path_string_id);
	*code_file_id_out = ((HscCodeFileId) { .idx_plus_one = *code_file_idx + 1 });
	*code_file_out = code_file;
	return false;
}

HscCodeFile* hsc_code_file_get(HscAstGen* astgen, HscCodeFileId code_file_id) {
	HSC_ASSERT_ARRAY_BOUNDS(astgen->code_files_count, astgen->code_files_cap);
	HSC_DEBUG_ASSERT(code_file_id.idx_plus_one, "internal error: file id was null");
	return &astgen->code_files[code_file_id.idx_plus_one - 1];
}

void hsc_pp_if_span_push(HscAstGen* astgen, HscPPDirective directive) {
	HscCodeSpan* span = astgen->span;
	HscCodeFile* code_file = span->code_file;

	HSC_ASSERT_ARRAY_BOUNDS(astgen->pp_if_spans_stack_count, astgen->pp_if_spans_stack_cap);
	HscPPIfSpan* pp_if_span = &astgen->pp_if_spans_stack[astgen->pp_if_spans_stack_count];
	astgen->pp_if_spans_stack_count += 1;
	pp_if_span->directive = directive;
	pp_if_span->has_else = false;
	pp_if_span->location = span->location;
}

void hsc_pp_if_span_pop(HscAstGen* astgen) {
	HSC_DEBUG_ASSERT(astgen->pp_if_spans_stack_count, "internal error: astgen->pp_if_spans_stack_count has no more elements to pop");
	astgen->pp_if_spans_stack_count -= 1;
}

HscPPIfSpan* hsc_pp_if_span_peek_top(HscAstGen* astgen) {
	HSC_DEBUG_ASSERT(astgen->pp_if_spans_stack_count, "internal error: astgen->pp_if_spans_stack_count must have atleast 1 element to peek_top");
	return &astgen->pp_if_spans_stack[astgen->pp_if_spans_stack_count - 1];
}

void hsc_compiler_init(HscCompiler* compiler, HscCompilerSetup* setup) {
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

	hsc_constant_table_init(&compiler->astgen.constant_table, setup->string_table_data_cap, setup->string_table_entries_cap);
	{
		for (HscDataType data_type = HSC_DATA_TYPE_BOOL; data_type < HSC_DATA_TYPE_BASIC_COUNT; data_type += 1) {
			if (!(compiler->available_basic_types & (1 << data_type))) {
				continue;
			}
			compiler->astgen.basic_type_zero_constant_ids[data_type] = hsc_constant_table_deduplicate_zero(&compiler->astgen.constant_table, &compiler->astgen, data_type);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_BOOL)) {
			U8 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_BOOL] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_BOOL, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_U8)) {
			U8 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_U8] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_U8, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_U16)) {
			U16 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_U16] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_U16, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_U32)) {
			U32 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_U32] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_U32, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_U64)) {
			U64 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_U64] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_U64, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_S8)) {
			S8 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_S8] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_S8, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_S16)) {
			S16 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_S16] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_S16, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_S32)) {
			S32 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_S32] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_S32, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_S64)) {
			S64 one = 1;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_S64] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_S64, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_F32)) {
			F32 one = 1.f;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_F32] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_F32, &one);
		}

		if (compiler->available_basic_types & (1 << HSC_DATA_TYPE_F64)) {
			F64 one = 1.f;
			compiler->astgen.basic_type_one_constant_ids[HSC_DATA_TYPE_F64] = hsc_constant_table_deduplicate_basic(&compiler->astgen.constant_table, &compiler->astgen, HSC_DATA_TYPE_F64, &one);
		}
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
}

void hsc_compiler_compile(HscCompiler* compiler, char* file_path) {
	U32 file_path_size = strlen(file_path) + 1;
	HscStringId path_string_id = hsc_string_table_deduplicate(&compiler->astgen.string_table, file_path, file_path_size);

	HscCodeFileId code_file_id;
	HscCodeFile* code_file;
	bool found_file = hsc_code_file_find_or_insert(&compiler->astgen, path_string_id, &code_file_id, &code_file);
	HSC_DEBUG_ASSERT(!found_file, "internal error: root file should be able to be inserted into the hash table no problem");

	U64 code_size;
	U8* code = hsc_file_read_all_the_codes(file_path, &code_size);
	if (code == NULL) {
		char buf[512];
		hsc_get_last_system_error_string(buf, sizeof(buf));
		printf("failed to read file '%s': %s\n", file_path, buf);
	}

	code_file->flags |= HSC_CODE_FILE_FLAGS_COMPILATION_UNIT;
	code_file->code = code;
	code_file->code_size = code_size;

	hsc_code_span_push_file(&compiler->astgen, code_file_id);

	hsc_astgen_tokenize(&compiler->astgen);

	HSC_DEBUG_ASSERT(compiler->astgen.code_span_stack_count == 0, "internal error: we did not pop all of the code spans!");

	hsc_astgen_generate(&compiler->astgen);
	hsc_ir_generate(&compiler->ir, &compiler->astgen);
	hsc_spirv_generate(compiler);

	hsc_tokens_print(&compiler->astgen, stdout);
	hsc_astgen_print_ast(&compiler->astgen, stdout);
	hsc_ir_print(&compiler->ir, &compiler->astgen, stdout);
}

