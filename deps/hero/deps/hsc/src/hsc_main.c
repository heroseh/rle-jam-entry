#include "hsc.c"

int main(int argc, char** argv) {
	HscCompilerSetup compiler_setup = {
		.tokens_cap = 64 * 1024,
		.lines_cap = 64 * 1024,
		.functions_cap = 64 * 1024,
		.function_params_cap = 64 * 1024,
		.exprs_cap = 64 * 1024,
		.variable_stack_cap = 64 * 1024,
		.string_table_data_cap = 64 * 1024 * 1024,
		.string_table_entries_cap = 64 * 1024,
	};

	HscCompiler compiler = {0};
	hsc_compiler_init(&compiler, &compiler_setup);

	hsc_compiler_compile(&compiler, "test.hsc");

	printf("found '%u' tokens\n", compiler.astgen.tokens_count);

	return 0;
}


