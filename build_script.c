#include <stdlib.h>
#include "deps/cmd_arger.h"
#include "deps/cmd_arger.c"
#include "deps/hero/core.h"
#include "deps/hero/core.c"

/*
#define game_core_src_file "src/game.c"
#define game_core_out_file "build/libgame_core.so"
*/

#define game_src_file "src/game.c"
#define game_out_file "build/game"

#define BUF_SIZE 1024

typedef U8 HscToken;
enum {
	HSC_TOKEN_INVALID,
	HSC_TOKEN_EOF,
	HSC_TOKEN_IDENT,

	//
	// keywords
	//
	HSC_TOKEN_KEYWORD_STRUCT,
	HSC_TOKEN_KEYWORD_SHADER,
	HSC_TOKEN_KEYWORD_RO_BUFFER,
	HSC_TOKEN_KEYWORD_RW_BUFFER,
	HSC_TOKEN_KEYWORD_RO_IMAGE1D,
	HSC_TOKEN_KEYWORD_RW_IMAGE1D,
	HSC_TOKEN_KEYWORD_RO_IMAGE2D,
	HSC_TOKEN_KEYWORD_RW_IMAGE2D,
	HSC_TOKEN_KEYWORD_RO_IMAGE3D,
	HSC_TOKEN_KEYWORD_RW_IMAGE3D,

	//
	// intrinsic types
	//
	HSC_TOKEN_INTRINSIC_TYPE_U32,
	HSC_TOKEN_INTRINSIC_TYPE_F32,
	HSC_TOKEN_INTRINSIC_TYPE_VEC2,
	HSC_TOKEN_INTRINSIC_TYPE_UVEC2,
	HSC_TOKEN_INTRINSIC_TYPE_SVEC2,
	HSC_TOKEN_INTRINSIC_TYPE_VEC3,
	HSC_TOKEN_INTRINSIC_TYPE_UVEC3,
	HSC_TOKEN_INTRINSIC_TYPE_SVEC3,
	HSC_TOKEN_INTRINSIC_TYPE_VEC4,
	HSC_TOKEN_INTRINSIC_TYPE_UVEC4,
	HSC_TOKEN_INTRINSIC_TYPE_SVEC4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X4,

	//
	// symbols
	//
	HSC_TOKEN_CURLY_OPEN,
	HSC_TOKEN_CURLY_CLOSE,
	HSC_TOKEN_PARENTHESIS_OPEN,
	HSC_TOKEN_PARENTHESIS_CLOSE,

	HSC_TOKEN_COUNT,
};

const char* hsc_token_strings[HSC_TOKEN_COUNT] = {
	[HSC_TOKEN_INVALID] = "invalid",
	[HSC_TOKEN_EOF] = "end of file",
	[HSC_TOKEN_IDENT] = "identifier",
	[HSC_TOKEN_KEYWORD_STRUCT] = "struct",
	[HSC_TOKEN_KEYWORD_SHADER] = "shader",
	[HSC_TOKEN_KEYWORD_RO_BUFFER] = "ro_buffer",
	[HSC_TOKEN_KEYWORD_RW_BUFFER] = "rw_buffer",
	[HSC_TOKEN_KEYWORD_RO_IMAGE1D] = "ro_image1d",
	[HSC_TOKEN_KEYWORD_RW_IMAGE1D] = "rw_image1d",
	[HSC_TOKEN_KEYWORD_RO_IMAGE2D] = "ro_image2d",
	[HSC_TOKEN_KEYWORD_RW_IMAGE2D] = "rw_image2d",
	[HSC_TOKEN_KEYWORD_RO_IMAGE3D] = "ro_image3d",
	[HSC_TOKEN_KEYWORD_RW_IMAGE3D] = "rw_image3d",
	[HSC_TOKEN_INTRINSIC_TYPE_U32] = "U32",
	[HSC_TOKEN_INTRINSIC_TYPE_F32] = "F32",
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
	[HSC_TOKEN_CURLY_OPEN] = "{",
	[HSC_TOKEN_CURLY_CLOSE] = "}",
	[HSC_TOKEN_PARENTHESIS_OPEN] = "(",
	[HSC_TOKEN_PARENTHESIS_CLOSE] = ")",
};

typedef struct HscLocation HscLocation;
struct HscLocation {
	U32 code_idx;
	U32 row;
	U32 column;
};

typedef struct HscParser HscParser;
struct HscParser {
	HscToken* tokens;
	HscLocation* token_locations;
	U32 token_read_idx;
	U32 tokens_count;
	U32 tokens_cap;
	HscLocation location;
	const char* file_path;
	U8* bytes;
	U32 bytes_count;
};

void hsc_parser_init(HscParser* parser, U32 tokens_cap) {
	parser->tokens = hero_alloc_array(HscToken, hero_system_alctor, 0, tokens_cap);
	HERO_ASSERT(parser->tokens, "out of memory");
	parser->token_locations = hero_alloc_array(HscLocation, hero_system_alctor, 0, tokens_cap);
	HERO_ASSERT(parser->token_locations, "out of memory");
}

void hsc_parser_error_1(HscParser* parser, const char* fmt, ...) {

	printf("error:");

	va_list va_args;
	va_start(va_args, fmt);
	vprintf(fmt, va_args);
	va_end(va_args);

	printf("\nfile: %s:%u:%u\n");

	exit(1);
}

void hsc_parser_error_2(HscParser* parser, HscLocation* other_location, const char* fmt, ...) {

	printf("error:");

	va_list va_args;
	va_start(va_args, fmt);
	vprintf(fmt, va_args);
	va_end(va_args);

	printf("\nfile: %s:%u:%u\n");

	exit(1);
}

void hsc_parser_add_token(HscParser* parser, HscToken token) {
	if (parser->tokens_count >= parser->tokens_cap) {
		hsc_parser_error_1(parser, "internal error: the tokens capacity of '%u' has been exceeded", parser->tokens_cap);
	}

	parser->tokens[parser->tokens_count] = token;
	parser->token_locations[parser->tokens_count] = parser->location;
	parser->tokens_count += 1;
}

void hsc_parser_tokenize(HscParser* parser) {
	HscToken last_open_bracket = HSC_TOKEN_INVALID;
	HscLocation last_open_bracket_location;

	parser->location.code_idx = 0;
	parser->location.row = 1;
	parser->location.column = 1;
	while (parser->location.code_idx < parser->bytes_count) {
		U8 byte = parser->bytes[parser->location.code_idx];

		HscToken token = HSC_TOKEN_INVALID;
		U32 token_size = 1;
		switch (byte) {
			case ' ':
			case '\t':
				parser->location.code_idx += 1;
				continue;
			case '\r':
			case '\n':
				parser->location.code_idx += 1;
				if (byte == '\n') {
					parser->location.row += 1;
					parser->location.column = 1;
				}
				continue;
			case '{': {
				if (last_open_bracket != HSC_TOKEN_INVALID) {
					hsc_parser_error_2(parser, last_open_bracket_location, "cannot open another bracket inside another bracket pair");
				}
				last_open_bracket = HSC_TOKEN_CURLY_OPEN;
				last_open_bracket_location = paser->location;
				token = HSC_TOKEN_CURLY_OPEN;
				break;
			};
			case '}': {
				if (last_open_bracket == HSC_TOKEN_CURLY_OPEN) {
					hsc_parser_error_2(parser, last_open_bracket_location, "expected '}' to close the open bracket '{'");
				}
				last_open_bracket = HSC_TOKEN_INVALID;
				last_open_bracket_location = paser->location;
				token = HSC_TOKEN_CURLY_CLOSE;
				break;
			};
			case '(': {
				if (last_open_bracket != HSC_TOKEN_INVALID) {
					hsc_parser_error_2(parser, last_open_bracket_location, "cannot open another bracket inside another bracket pair");
				}
				last_open_bracket = HSC_TOKEN_PARENTHESIS_OPEN;
				last_open_bracket_location = paser->location;
				token = HSC_TOKEN_PARENTHESIS_OPEN;
				break;
			};
			case ')': {
				if (last_open_bracket == HSC_TOKEN_PARENTHESIS_OPEN) {
					hsc_parser_error_2(parser, last_open_bracket_location, "expected ')' to close the open bracket '('");
				}
				last_open_bracket = HSC_TOKEN_INVALID;
				last_open_bracket_location = paser->location;
				token = HSC_TOKEN_PARENTHESIS_CLOSE;
				break;
			};

			default: {
				if (
					(byte < 'a' || 'z' < byte) &&
					(byte < 'A' || 'Z' < byte)
				) {
					hsc_parser_error_1(parser, "invalid token '%c'", byte);
				}

				U8* ident_string = &parser->bytes[parser->location.code_idx];
				U32 code_idx = parser->location.code_idx + 1;
				while (code_idx < parser->bytes_count) {
					U8 ident_byte = parser->bytes[code_idx];

					switch (ident_byte) {
						case '(':
						case ')':
						case '{':
						case '}':
						case '\n':
							break;
					}

					if (
						(ident_byte < 'a' || 'z' < ident_byte) &&
						(ident_byte < 'A' || 'Z' < ident_byte) &&
						(ident_byte < '0' || '9' < ident_byte)
					) {
						hsc_parser_error_1(parser, "identifier character must be alphanumeric but got '%c'", ident_byte);
					}
				}

				U32 ident_size = code_idx - parser->location.code_idx;

				token = HSC_TOKEN_IDENT;
				token_size = ident_size;
				for_range(t, 0, HSC_TOKEN_COUNT) {
					const char* t_string = hsc_token_string[t];
					U32 t_size = strlen(t_string);
					if (t_size == ident_size && strncmp(ident_string, t_string, t_size) == 0) {
						token = t;
						token_size = t_size;
						break;
					}
				}

				break;
			};
		}

		parser->location.code_idx += token_size;
		parser->location.column += token_size;
		hsc_parser_add_token(parser, token);
	}

	hsc_parser_add_token(parser, HSC_TOKEN_EOF);
}

HscToken hsc_token_peek(HscParser* parser) {
	return parser->tokens[HERO_MIN(parser->token_read_idx, parser->tokens_count)];
}

HscToken hsc_token_next(HscParser* parser) {
	parser->token_read_idx += 1;
	return parser->tokens[HERO_MIN(parser->token_read_idx, parser->tokens_count)];
}

void hsc_parse_memory(U8* string, U32 size) {
	HeroTextReader reader;
	hero_text_reader_init(&reader, string, size);

	while (hero_text_reader_has_content(&reader)) {
		hero_text_reader_consume_whitespace_and_newlines(&reader);

		U8* keyword = hero_text_reader_cursor(&reader);
		U32 keyword_size = hero_text_reader_consume_until_byte(&reader, ' ');

		hero_text_reader_consume_whitespace(&reader);

		U8* identifier = hero_text_reader_cursor(&reader);
		U32 identifier_size = hero_text_reader_consume_until_byte(&reader, ' ');


		hero_text_reader_consume_whitespace_and_newlines(&reader);

		U8* identifier = hero_text_reader_cursor(&reader);
		if (!hero_text_reader_consume_byte(&reader, '{')) {
			hsc_error("expected '{'");
		}

		if (strncmp(keyword, "struct", keyword_size) == 0) {

		} else if (strncmp(keyword, "shader", keyword_size) == 0) {

		} else {
			hsc_error("invalid keyword '%.*s', expected either 'struct' or 'shader'", keyword_size, keyword);
		}
	}
}

void hsc_parse_file(const char* file_path) {
	U8* bytes;
	Uptr file_size;
	HeroResult result = hero_file_read_all(file_path, hero_system_alctor, 0, &bytes, &file_size);
	HERO_RESULT_ASSERT(result);

	hsc_parse_memory(bytes, size);
}

int compile_shader(const char* src_dir, const char* name) {
	char buf[BUF_SIZE];

	snprintf(buf, BUF_SIZE, "glslangValidator -V -DVERTEX -DHERO_GLSL -S vert --source-entrypoint main -e vertex -o build/%s.vertex.spv %s/%s.glsl", name, src_dir, name);
	int exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	snprintf(buf, BUF_SIZE, "glslangValidator -V -DFRAGMENT -DHERO_GLSL -S frag --source-entrypoint main -e fragment -o build/%s.fragment.spv %s/%s.glsl", name, src_dir, name);
	exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	snprintf(buf, BUF_SIZE, "spirv-link -o build/%s.spv build/%s.vertex.spv build/%s.fragment.spv", name, name, name);
	exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	return 0;
}

int compile_shader_compute(const char* src_dir, const char* name) {
	char buf[BUF_SIZE];

	snprintf(buf, BUF_SIZE, "glslangValidator -V -DCOMPUTE -DHERO_GLSL -S comp --source-entrypoint main -e compute -o build/%s.spv %s/%s.glsl", name, src_dir, name);
	int exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	return 0;
}

int main(int argc, char** argv) {
	bool debug = false;
	bool debug_address = false;
	bool debug_memory = false;
	bool clean = false;
	bool sse = false;
	bool avx = false;
	bool compile_shaders = false;

	char* compiler = "clang";
	int64_t opt = 0;
	CmdArgerDesc desc[] = {
		cmd_arger_desc_flag(&debug, "debug", "compile in debuggable executable"),
		cmd_arger_desc_flag(&clean, "clean", "remove any built binaries"),
		cmd_arger_desc_flag(&compile_shaders, "shaders", "compile the shaders"),
		cmd_arger_desc_flag(&debug_address, "debug_address", "turns on address sanitizer"),
		cmd_arger_desc_flag(&debug_memory, "debug_memory", "turns on address memory sanitizer"),
		cmd_arger_desc_flag(&sse, "sse", "turns on sse optimizations"),
		cmd_arger_desc_flag(&avx, "avx", "turns on avx optimizations"),
		cmd_arger_desc_string(&compiler, "compiler", "the compiler command"),
		cmd_arger_desc_integer(&opt, "opt", "compiler code optimization level, 0 none, 3 max"),
	};

	char* app_name_and_version = "game build script";
	cmd_arger_parse(desc, sizeof(desc) / sizeof(*desc), NULL, 0, argc, argv, app_name_and_version, true);

	int exe_res;
	if (clean) {
		exe_res = system("rm "game_out_file);
		if (exe_res != 0) { return exe_res; }
		return exe_res;
	}


	char buf[BUF_SIZE];
	char cflags[512];
	size_t cflags_idx = 0;
	cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -Werror -Wfloat-conversion -Wpedantic -std=c11 -DHERO_X11_ENABLE -DHERO_VULKAN_ENABLE -D_XOPEN_SOURCE=700 -D_GNU_SOURCE");
	cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -O%zd -mstackrealign", opt);
	if (debug) {
		cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -g3 -DHERO_DEBUG_ASSERTIONS=1");
	}
	if (debug_address) {
		cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -fsanitize=address");
	}
	if (debug_memory) {
		cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -fsanitize=memory");
	}
	if (sse) {
		cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -msse4.2");
	}
	if (avx) {
		cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -mavx");
	}
	cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -lm -ldl -rdynamic -pthread");
	char* env_cflags = getenv("CFLAGS");
	if (env_cflags == NULL) {
		env_cflags = "";
	}

	char* include_paths = "-I./";

	char* ld_flags = "-Wl,-rpath,\\$ORIGIN";

	// ensure the build directory exists
	exe_res = system("mkdir -p build");
	if (exe_res != 0) { return exe_res; }

	/*
	// compile game core shared library
	snprintf(buf, BUF_SIZE, "%s %s %s -shared -fPIC -o %s %s %s %s", compiler, env_cflags, cflags, game_core_out_file, game_core_src_file, include_paths, ld_flags);
	exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }
	*/

	cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -lX11 -lXrandr -L./build/ ");

	// compile game
	snprintf(buf, BUF_SIZE, "%s %s %s -o %s %s %s %s", compiler, env_cflags, cflags, game_out_file, game_src_file, include_paths, ld_flags);
	exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	if (compile_shaders) {
		exe_res = compile_shader("src/shaders", "island_gen_debug");
		if (exe_res != 0) { return exe_res; }

		exe_res = compile_shader("src/shaders", "play_terrain");
		if (exe_res != 0) { return exe_res; }

		exe_res = compile_shader_compute("src/shaders", "play_voxel_raytrace");
		if (exe_res != 0) { return exe_res; }

		exe_res = compile_shader("src/shaders", "play_model");
		if (exe_res != 0) { return exe_res; }

		exe_res = compile_shader("src/shaders", "play_billboard");
		if (exe_res != 0) { return exe_res; }

		exe_res = compile_shader("deps/hero", "ui");
		if (exe_res != 0) { return exe_res; }
	}

	return exe_res;
}


