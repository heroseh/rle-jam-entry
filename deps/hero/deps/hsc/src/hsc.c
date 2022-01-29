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
// Syntax Generator
//
//
// ===========================================

char* hsc_token_strings[HSC_TOKEN_COUNT] = {
	[HSC_TOKEN_INVALID] = "invalid",
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
	[HSC_TOKEN_INTRINSIC_TYPE_U8] = "U8",
	[HSC_TOKEN_INTRINSIC_TYPE_U16] = "U16",
	[HSC_TOKEN_INTRINSIC_TYPE_U32] = "U32",
	[HSC_TOKEN_INTRINSIC_TYPE_U64] = "U64",
	[HSC_TOKEN_INTRINSIC_TYPE_F8] = "F8",
	[HSC_TOKEN_INTRINSIC_TYPE_F16] = "F16",
	[HSC_TOKEN_INTRINSIC_TYPE_F32] = "F32",
	[HSC_TOKEN_INTRINSIC_TYPE_F64] = "F64",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC2] = "Vec2",
	[HSC_TOKEN_INTRINSIC_TYPE_UVEC2] = "UVec2",
	[HSC_TOKEN_INTRINSIC_TYPE_SVEC2] = "SVec2",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC3] = "Vec3",
	[HSC_TOKEN_INTRINSIC_TYPE_UVEC3] = "UVec3",
	[HSC_TOKEN_INTRINSIC_TYPE_SVEC3] = "SVec3",
	[HSC_TOKEN_INTRINSIC_TYPE_VEC4] = "Vec4",
	[HSC_TOKEN_INTRINSIC_TYPE_UVEC4] = "UVec4",
	[HSC_TOKEN_INTRINSIC_TYPE_SVEC4] = "SVec4",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X2] = "Mat2x2",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X3] = "Mat2x3",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT2X4] = "Mat2x4",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X2] = "Mat3x2",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X3] = "Mat3x3",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT3X4] = "Mat3x4",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X2] = "Mat4x2",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X3] = "Mat4x3",
	[HSC_TOKEN_INTRINSIC_TYPE_MAT4X4] = "Mat4x4",
};

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

		if (string_size == entry->size && strncmp(string_table->data + entry->start_idx, string, string_size) == 0) {
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

void hsc_syngen_init(HscSynGen* syngen, uint32_t tokens_cap, U32 lines_cap) {
	syngen->tokens = HSC_ALLOC_ARRAY(HscToken, tokens_cap);
	HSC_ASSERT(syngen->tokens, "out of memory");
	syngen->token_locations = HSC_ALLOC_ARRAY(HscLocation, tokens_cap);
	HSC_ASSERT(syngen->token_locations, "out of memory");
	syngen->token_values = HSC_ALLOC_ARRAY(HscLocation, tokens_cap);
	HSC_ASSERT(syngen->token_values, "out of memory");
	syngen->tokens_cap = tokens_cap;
	syngen->line_code_start_indices = HSC_ALLOC_ARRAY(U32, lines_cap);
	HSC_ASSERT(syngen->line_code_start_indices, "out of memory");
	syngen->lines_cap = lines_cap;
	syngen->print_color = true;
}

void hsc_syngen_print_code_line(HscSynGen* syngen, U32 display_line_num_size, U32 line) {
	U32 code_start_idx = syngen->line_code_start_indices[line];
	U32 code_end_idx;
	if (line >= syngen->lines_count) {
		code_end_idx = syngen->bytes_count;
	} else {
		code_end_idx = syngen->line_code_start_indices[line + 1];
	}
	while (1) {
		code_end_idx -= 1;
		if (code_end_idx == 0) {
			break;
		}
		U8 byte = syngen->bytes[code_end_idx];
		if (byte != '\r' && byte != '\n') {
			code_end_idx += 1;
			break;
		}
	}

	if (code_end_idx <= code_start_idx) {
		U32 code_size = code_end_idx - code_start_idx;
		char* code = (char*)&syngen->bytes[code_start_idx];
		const char* fmt = syngen->print_color
			? "\x1b[1;94m%*u|\x1b[0m\n"
			: "%*u|\n";
		printf(fmt, display_line_num_size, line);
	} else {
		U32 code_size = code_end_idx - code_start_idx;
		char* code = (char*)&syngen->bytes[code_start_idx];

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

		const char* fmt = syngen->print_color
			? "\x1b[1;94m%*u|\x1b[0m %.*s\n"
			: "%*u| %.*s\n";
		printf(fmt, display_line_num_size, line, code_size, code_without_tabs);
	}
}

void hsc_syngen_print_code(HscSynGen* syngen, HscLocation* location) {
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
		hsc_syngen_print_code_line(syngen, display_line_num_size, line - 2);
	}
	if (line > 1) {
		hsc_syngen_print_code_line(syngen, display_line_num_size, line - 1);
	}

	hsc_syngen_print_code_line(syngen, display_line_num_size, line);

	for (U32 i = 0; i < display_line_num_size + 1; i += 1) {
		putchar(' ');
	}

	for (U32 i = 0; i < location->column_start; i += 1) {
		if (location->code_start_idx > location->column_start && syngen->bytes[location->code_start_idx - location->column_start + i] == '\t') {
			printf("    ");
		} else {
			putchar(' ');
		}
	}

	if (syngen->print_color) {
		printf("\x1b[1;93m");
	}
	for (U32 i = 0; i < location->column_end - location->column_start; i += 1) {
		putchar('^');
	}
	if (syngen->print_color) {
		printf("\x1b[0m");
	}
	printf("\n");

	if (line + 1 <= syngen->lines_count) {
		hsc_syngen_print_code_line(syngen, display_line_num_size, line + 1);
	}
	if (line + 2 <= syngen->lines_count) {
		hsc_syngen_print_code_line(syngen, display_line_num_size, line + 2);
	}
}

void hsc_syngen_found_newline(HscSynGen* syngen) {
	syngen->location.line_start += 1;
	syngen->location.line_end += 1;
	syngen->location.column_start = 1;
	syngen->location.column_end = 1;

	if (syngen->lines_count >= syngen->lines_cap) {
		hsc_syngen_token_error_1(syngen, "internal error: the lines capacity of '%u' has been exceeded", syngen->lines_cap);
	}

	syngen->line_code_start_indices[syngen->lines_count] = syngen->location.code_end_idx;
	syngen->lines_count += 1;
}

void hsc_syngen_token_count_extra_newlines(HscSynGen* syngen) {
	U32 lines_count = 3;
	HscLocation location = syngen->location;
	while (syngen->location.code_end_idx < syngen->bytes_count) {
		U8 byte = syngen->bytes[syngen->location.code_end_idx];
		syngen->location.code_end_idx += 1;
		if (byte == '\n') {
			hsc_syngen_found_newline(syngen);
			lines_count -= 1;
			if (lines_count == 0) {
				break;
			}
		}
	}
	syngen->location = location;
}

void hsc_syngen_token_error_start(HscSynGen* syngen, const char* fmt, va_list va_args) {
	hsc_syngen_token_count_extra_newlines(syngen);

	const char* error_fmt = syngen->print_color
		? "\x1b[1;91merror\x1b[0m: "
		: "error: ";
	printf("%s", error_fmt);

	if (syngen->print_color) {
		printf("\x1b[1;97m");
	}

	vprintf(fmt, va_args);

	const char* file_path = "test.hsc";
	error_fmt = syngen->print_color
		? "\x1b[1;95m\nfile\x1b[97m: %s:%u:%u\n\x1b[0m"
		: "\nfile: %s:%u:%u\n";
	printf(error_fmt, file_path, syngen->location.line_start, syngen->location.column_start);

}

void hsc_syngen_token_error_1(HscSynGen* syngen, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	hsc_syngen_token_error_start(syngen, fmt, va_args);
	va_end(va_args);

	hsc_syngen_print_code(syngen, &syngen->location);

	exit(1);
}

void hsc_syngen_token_error_2(HscSynGen* syngen, HscLocation* other_location, const char* fmt, ...) {
	va_list va_args;
	va_start(va_args, fmt);
	hsc_syngen_token_error_start(syngen, fmt, va_args);
	va_end(va_args);

	hsc_syngen_print_code(syngen, &syngen->location);

	const char* error_fmt = syngen->print_color
		? "\x1b[1;97\nmoriginally defined here\x1b[0m: \n"
		: "\noriginally defined here: ";
	printf("%s", error_fmt);

	const char* file_path = "test.hsc";
	error_fmt = syngen->print_color
		? "\x1b[1;95mfile\x1b[97m: %s:%u:%u\n\x1b[0m"
		: "file: %s:%u:%u\n";
	printf(error_fmt, file_path, other_location->line_start, other_location->column_start);

	hsc_syngen_print_code(syngen, other_location);

	exit(1);
}

void hsc_syngen_add_token(HscSynGen* syngen, HscToken token) {
	if (syngen->tokens_count >= syngen->tokens_cap) {
		hsc_syngen_token_error_1(syngen, "internal error: the tokens capacity of '%u' has been exceeded", syngen->tokens_cap);
	}

	syngen->tokens[syngen->tokens_count] = token;
	syngen->token_locations[syngen->tokens_count] = syngen->location;
	syngen->tokens_count += 1;
}

void hsc_syngen_add_token_value(HscSynGen* syngen, HscTokenValue value) {
	if (syngen->token_values_count >= syngen->tokens_cap) {
		hsc_syngen_token_error_1(syngen, "internal error: the token values capacity of '%u' has been exceeded", syngen->tokens_cap);
	}

	syngen->token_values[syngen->token_values_count] = value;
	syngen->token_values_count += 1;
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

uint32_t hsc_parse_num(HscSynGen* syngen, HscToken* token_out, HscTokenValue* token_value_out) {
	char* num_string = (char*)&syngen->bytes[syngen->location.code_end_idx];
	uint32_t remaining_size = syngen->bytes_count - syngen->location.code_end_idx;
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

	HscTokenValue token_value = {0};
	F64 pow_10 = 10.0;
	while (token_size < remaining_size) {
		uint8_t digit = num_string[token_size];
		token_size += 1;

		switch (digit) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
			{
				if (radix == 8 && digit > 7) {
					hsc_syngen_token_error_1(syngen, "octal digits must be from 0 to 7 inclusively");
				}
				uint32_t int_digit = digit - '0';
				switch (token) {
					case HSC_TOKEN_LIT_U32:
					case HSC_TOKEN_LIT_U64:
					case HSC_TOKEN_LIT_S32:
					case HSC_TOKEN_LIT_S64:
						if (
							!hsc_i64_checked_mul(token_value.u64, radix, &token_value.u64)        ||
							!hsc_u64_checked_add(token_value.u64, int_digit, &token_value.u64)
						) {
							hsc_syngen_token_error_1(syngen, "integer literal is too large and will overflow a U64 integer");
						}
						break;
					case HSC_TOKEN_LIT_F32:
					case HSC_TOKEN_LIT_F64:
						token_value.f64 += (F64)(int_digit) / pow_10;
						if (isinf(token_value.f64)) {
							hsc_syngen_token_error_1(syngen, "float literal is too large and will overflow a f64");
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
					hsc_syngen_token_error_1(syngen, "the 'u' suffix can only be applied to positive integer numbers. e.g. 3369");
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
						hsc_syngen_token_error_1(syngen, "the 'l' suffix for a long double is unsupported");
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
					hsc_syngen_token_error_1(syngen, "float literals can only have a single '.'");
				}
				if (radix != 10) {
					hsc_syngen_token_error_1(syngen, "octal and hexidecimal digits are not supported for float literals");
				}

				token = HSC_TOKEN_LIT_F64;
				F64 f64 = (F64)token_value.u64;
				if ((U64)f64 != token_value.u64) {
					hsc_syngen_token_error_1(syngen, "float literal is too large and will overflow a f64");
				}
				token_value.f64 = f64;
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
								!hsc_i64_checked_mul(token_value.u64, radix, &token_value.u64)        ||
								!hsc_u64_checked_add(token_value.u64, int_digit, &token_value.u64)
							) {
								hsc_syngen_token_error_1(syngen, "integer literal is too large and will overflow a U64 integer");
							}
							break;
					}
				} else if (digit == 'f' || digit == 'F') {
					if (token != HSC_TOKEN_LIT_F64) {
						hsc_syngen_token_error_1(syngen, "only float literals can be made into a float literal with a 'f' suffix. e.g. 0.f or 1.0f");
					}
					token = HSC_TOKEN_LIT_F32;
					goto NUM_END;
				} else if ((digit >= 'a' && digit <= 'z') || (digit >= 'A' && digit <= 'Z')) {
					switch (token) {
						case HSC_TOKEN_LIT_U32:
						case HSC_TOKEN_LIT_U64:
						case HSC_TOKEN_LIT_S32:
						case HSC_TOKEN_LIT_S64:
							hsc_syngen_token_error_1(syngen, "invalid suffix for integer literals");
						case HSC_TOKEN_LIT_F32:
						case HSC_TOKEN_LIT_F64:
							hsc_syngen_token_error_1(syngen, "invalid suffix for float literals");
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
				if (token_value.u64 > (U64)S64_MAX + 1) {
					hsc_syngen_token_error_1(syngen, "integer literal is too large and will overflow a S64 integer");
				}
				token_value.s64 = -token_value.u64;
				break;
			};
			case HSC_TOKEN_LIT_F32:
			case HSC_TOKEN_LIT_F64:
				token_value.f64 = -token_value.f64;
				break;
		}
	}

	switch (token) {
		case HSC_TOKEN_LIT_U32:
			if (token_value.u64 > U32_MAX) {
				token = HSC_TOKEN_LIT_U64;
			}
			break;
		case HSC_TOKEN_LIT_S32:
			if (is_negative) {
				if (token_value.s64 > S32_MAX || token_value.s64 < S32_MIN) {
					token = HSC_TOKEN_LIT_S64;
				}
			} else if (token_value.u64 > S32_MAX) {
				if (radix != 10 && token_value.u64 <= U32_MAX) {
					token = HSC_TOKEN_LIT_U32;
				} else if (token_value.u64 <= S64_MAX) {
					token = HSC_TOKEN_LIT_S64;
				} else if (radix != 10) {
					token = HSC_TOKEN_LIT_U64;
				} else {
					hsc_syngen_token_error_1(syngen, "integer literal is too large and will overflow a S64 integer, consider using 'u' suffix to promote to an unsigned type. e.g. 1000u");
				}
			}
			break;
	}

	*token_out = token;
	*token_value_out = token_value;
	return token_size;
}

void hsc_syngen_tokenize(HscSynGen* syngen) {
	HscToken brackets_to_close[_HSC_TOKENIZER_NESTED_BRACKETS_CAP];
	HscLocation brackets_to_close_locations[_HSC_TOKENIZER_NESTED_BRACKETS_CAP];
	uint32_t brackets_to_close_count = 0;

	bool is_negative;
	char num_buf[20];

	syngen->line_code_start_indices[0] = 0;
	syngen->line_code_start_indices[1] = 0;
	syngen->lines_count += 2;

	syngen->location.code_start_idx = 0;
	syngen->location.code_end_idx = 0;
	syngen->location.line_start = 1;
	syngen->location.line_end = 1;
	syngen->location.column_start = 1;
	syngen->location.column_end = 1;
	while (syngen->location.code_end_idx < syngen->bytes_count) {
		uint8_t byte = syngen->bytes[syngen->location.code_end_idx];

		syngen->location.code_start_idx = syngen->location.code_end_idx;
		syngen->location.line_start = syngen->location.line_end;
		syngen->location.column_start = syngen->location.column_end;

		HscToken token = HSC_TOKEN_INVALID;
		HscToken close_token;
		uint32_t token_size = 1;
		switch (byte) {
			case ' ':
			case '\t':
				syngen->location.code_start_idx += 1;
				syngen->location.code_end_idx += 1;
				syngen->location.column_start += 1;
				syngen->location.column_end += 1;
				continue;
			case '\r':
			case '\n':
				syngen->location.code_start_idx += 1;
				syngen->location.code_end_idx += 1;
				if (byte == '\n') {
					hsc_syngen_found_newline(syngen);
				}
				continue;
			case '.': token = HSC_TOKEN_FULL_STOP; break;
			case ',': token = HSC_TOKEN_COMMA; break;
			case ';': token = HSC_TOKEN_SEMICOLON; break;
			case '+': token = HSC_TOKEN_PLUS; break;
			case '-':
				if (isdigit(syngen->bytes[syngen->location.code_end_idx + 1])) {
					HscTokenValue token_value;
					token_size = hsc_parse_num(syngen, &token, &token_value);
					hsc_syngen_add_token_value(syngen, token_value);
				} else {
					token = HSC_TOKEN_MINUS;
				}
				break;
			case '/': {
				uint8_t next_byte = syngen->bytes[syngen->location.code_end_idx + 1];
				if (next_byte == '/') {
					syngen->location.code_end_idx += 2;
					while (syngen->location.code_end_idx < syngen->bytes_count) {
						uint8_t b = syngen->bytes[syngen->location.code_end_idx];
						syngen->location.code_end_idx += 1;
						if (b == '\n') {
							break;
						}
					}

					token_size = syngen->location.code_end_idx - syngen->location.code_start_idx;
					syngen->location.column_start += token_size;
					syngen->location.column_end += token_size;
					continue;
				} else if (next_byte == '*') {
					syngen->location.code_end_idx += 2;
					syngen->location.column_end += 2;
					while (syngen->location.code_end_idx < syngen->bytes_count) {
						uint8_t b = syngen->bytes[syngen->location.code_end_idx];
						syngen->location.code_end_idx += 1;
						syngen->location.column_end += 1;
						if (b == '*') {
							b = syngen->bytes[syngen->location.code_end_idx];
							if (b == '/') { // no need to check in bounds see _HSC_TOKENIZER_LOOK_HEAD_SIZE
								syngen->location.code_end_idx += 1;
								syngen->location.column_end += 1;
								break;
							}
						} else if (byte == '\n') {
							hsc_syngen_found_newline(syngen);
						}
					}

					syngen->location.column_start = syngen->location.column_end;
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
					hsc_syngen_token_error_1(syngen, "nested brackets capacity of '%u' has been exceeded", _HSC_TOKENIZER_NESTED_BRACKETS_CAP);
				}
				brackets_to_close[brackets_to_close_count] = close_token;
				brackets_to_close_locations[brackets_to_close_count] = syngen->location;
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
					hsc_syngen_token_error_1(syngen, "no brackets are open to close '%c'", byte);
				}

				brackets_to_close_count -= 1;
				if (brackets_to_close[brackets_to_close_count] != token) {
					syngen->location.code_end_idx += 1;
					syngen->location.column_end += 1;
					hsc_syngen_token_error_2(syngen, &brackets_to_close_locations[brackets_to_close_count], "expected to close bracket pair with '%s' but got '%c'", hsc_token_strings[brackets_to_close[brackets_to_close_count]], byte);
				}
				break;
			};

			default: {
				if ('0' <= byte && byte <= '9') {
					HscTokenValue token_value;
					token_size = hsc_parse_num(syngen, &token, &token_value);
					hsc_syngen_add_token_value(syngen, token_value);
					break;
				}

				if (
					(byte < 'a' || 'z' < byte) &&
					(byte < 'A' || 'Z' < byte)
				) {
					hsc_syngen_token_error_1(syngen, "invalid token '%c'", byte);
				}

				uint8_t* ident_string = &syngen->bytes[syngen->location.code_end_idx];
				while (syngen->location.code_end_idx < syngen->bytes_count) {
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
						hsc_syngen_token_error_1(syngen, "identifier character must be alphanumeric but got '%c'", ident_byte);
					}
					token_size += 1;
				}
IDENT_END: {}

				HscStringId string_id = hsc_string_table_deduplicate(&syngen->string_table, (char*)ident_string, token_size);
				if (string_id.idx_plus_one <= HSC_STRING_ID_INTRINSIC_TYPES_LAST) {
					if (string_id.idx_plus_one <= HSC_STRING_ID_KEYWORDS_LAST) {
						token = HSC_TOKEN_KEYWORDS_FIRST + (string_id.idx_plus_one - HSC_STRING_ID_KEYWORDS_FIRST);
					} else {
						token = HSC_TOKEN_INTRINSIC_TYPES_FIRST + (string_id.idx_plus_one - HSC_STRING_ID_INTRINSIC_TYPES_FIRST);
					}
				} else {
					token = HSC_TOKEN_IDENT;
					HscTokenValue token_value;
					token_value.string_id = string_id;
					hsc_syngen_add_token_value(syngen, token_value);
				}

				break;
			};
		}

		syngen->location.code_end_idx += token_size;
		syngen->location.column_end += token_size;

		static int i = 0;
		i += 1;
		if (i == 12) {
			//hsc_syngen_token_error_1(syngen, "test error");
		}
		hsc_syngen_add_token(syngen, token);
	}

	hsc_syngen_add_token(syngen, HSC_TOKEN_EOF);
}

HscToken hsc_token_peek(HscSynGen* syngen) {
	return syngen->tokens[HSC_MIN(syngen->token_read_idx, syngen->tokens_count)];
}

HscToken hsc_token_next(HscSynGen* syngen) {
	syngen->token_read_idx += 1;
	return syngen->tokens[HSC_MIN(syngen->token_read_idx, syngen->tokens_count)];
}

HscTokenValue hsc_token_value_next(HscSynGen* syngen) {
	syngen->token_value_read_idx += 1;
	return syngen->token_values[HSC_MIN(syngen->token_value_read_idx, syngen->token_values_count)];
}

// ===========================================
//
//
// Semantic Generator
//
//
// ===========================================

// ===========================================
//
//
// Compiler
//
//
// ===========================================

void hsc_compiler_init(HscCompiler* compiler, HscCompilerSetup* setup) {
	hsc_syngen_init(&compiler->syngen, setup->tokens_cap, setup->lines_cap);
	hsc_string_table_init(&compiler->syngen.string_table, setup->string_table_data_cap, setup->string_table_entries_cap);

	for (HscToken t = HSC_TOKEN_KEYWORDS_FIRST; t <= HSC_TOKEN_KEYWORDS_LAST; t += 1) {
		char* string = hsc_token_strings[t];
		hsc_string_table_deduplicate(&compiler->syngen.string_table, string, strlen(string));
	}
	for (HscToken t = HSC_TOKEN_INTRINSIC_TYPES_FIRST; t <= HSC_TOKEN_INTRINSIC_TYPES_LAST; t += 1) {
		char* string = hsc_token_strings[t];
		hsc_string_table_deduplicate(&compiler->syngen.string_table, string, strlen(string));
	}
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

	compiler->syngen.bytes = bytes;
	compiler->syngen.bytes_count = size;
	hsc_syngen_tokenize(&compiler->syngen);
}

