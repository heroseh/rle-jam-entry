#include "hsc.c"

int main(int argc, char** argv) {
	HscCompilerSetup compiler_setup = {
		.tokens_cap = 64 * 1024,
		.lines_cap = 64 * 1024,
		.string_table_data_cap = 64 * 1024 * 1024,
		.string_table_entries_cap = 64 * 1024,
	};

	HscCompiler compiler = {0};
	hsc_compiler_init(&compiler, &compiler_setup);

	hsc_compiler_compile(&compiler, "test.hsc");

	printf("found '%u' tokens\n", compiler.syngen.tokens_count);

	uint32_t token_value_idx = 0;
	for (uint32_t i = 0; i < compiler.syngen.tokens_count; i += 1) {
		HscToken token = compiler.syngen.tokens[i];
		HscTokenValue value;
		HscString string;
		switch (token) {
			case HSC_TOKEN_IDENT:
				value = compiler.syngen.token_values[token_value_idx];
				token_value_idx += 1;
				string = hsc_string_table_get(&compiler.syngen.string_table, value.string_id);
				printf("%s -> %.*s\n", hsc_token_strings[token], (int)string.size, string.data);
				break;
			case HSC_TOKEN_LIT_U32:
			case HSC_TOKEN_LIT_U64:
				value = compiler.syngen.token_values[token_value_idx];
				token_value_idx += 1;
				printf("%s -> %zu\n", hsc_token_strings[token], value.u64);
				break;
			case HSC_TOKEN_LIT_S32:
			case HSC_TOKEN_LIT_S64:
				value = compiler.syngen.token_values[token_value_idx];
				token_value_idx += 1;
				printf("%s -> %zd\n", hsc_token_strings[token], value.s64);
				break;
			case HSC_TOKEN_LIT_F32:
			case HSC_TOKEN_LIT_F64:
				value = compiler.syngen.token_values[token_value_idx];
				token_value_idx += 1;
				printf("%s -> %f\n", hsc_token_strings[token], value.f64);
				break;
			default:
				printf("%s\n", hsc_token_strings[token]);
				break;
		}
	}

	return 0;
}


