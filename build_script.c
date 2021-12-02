#include <stdlib.h>
#include "deps/cmd_arger.h"
#include "deps/cmd_arger.c"

/*
#define game_core_src_file "src/game.c"
#define game_core_out_file "build/libgame_core.so"
*/

#define game_src_file "src/game.c"
#define game_out_file "build/game"

#define BUF_SIZE 1024

int compile_shader(const char* name) {
	char buf[BUF_SIZE];

	snprintf(buf, BUF_SIZE, "glslangValidator -V -DVERTEX -DHERO_GLSL -S vert --source-entrypoint main -e vertex -o build/%s.vertex.spv src/shaders/%s.glsl", name, name);
	int exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	snprintf(buf, BUF_SIZE, "glslangValidator -V -DFRAGMENT -DHERO_GLSL -S frag --source-entrypoint main -e fragment -o build/%s.fragment.spv src/shaders/%s.glsl", name, name);
	exe_res = system(buf);
	if (exe_res != 0) { return exe_res; }

	snprintf(buf, BUF_SIZE, "spirv-link -o build/%s.spv build/%s.vertex.spv build/%s.fragment.spv", name, name, name);
	exe_res = system(buf);
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
	cflags_idx += snprintf(cflags + cflags_idx, sizeof(cflags) - cflags_idx, " -Werror -Wfloat-conversion -Wpedantic -std=c99 -DHERO_X11_ENABLE -DHERO_VULKAN_ENABLE -D_XOPEN_SOURCE=700 -D_GNU_SOURCE");
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
		exe_res = compile_shader("basic");
		if (exe_res != 0) { return exe_res; }
	}

	return exe_res;
}


