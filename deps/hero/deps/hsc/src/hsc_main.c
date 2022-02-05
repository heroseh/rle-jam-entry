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

	uint32_t token_value_idx = 0;
	for (uint32_t i = 0; i < compiler.astgen.tokens_count; i += 1) {
		HscToken token = compiler.astgen.tokens[i];
		HscTokenValue value;
		HscString string;
		HscDataType data_type;
		switch (token) {
			case HSC_TOKEN_IDENT:
				value = compiler.astgen.token_values[token_value_idx];
				token_value_idx += 1;
				string = hsc_string_table_get(&compiler.astgen.string_table, value.string_id);
				printf("%s -> %.*s\n", hsc_token_strings[token], (int)string.size, string.data);
				break;
			case HSC_TOKEN_LIT_U32: data_type = HSC_DATA_TYPE_U32; goto PRINT_LIT;
			case HSC_TOKEN_LIT_U64: data_type = HSC_DATA_TYPE_U64; goto PRINT_LIT;
			case HSC_TOKEN_LIT_S32: data_type = HSC_DATA_TYPE_S32; goto PRINT_LIT;
			case HSC_TOKEN_LIT_S64: data_type = HSC_DATA_TYPE_S64; goto PRINT_LIT;
			case HSC_TOKEN_LIT_F32: data_type = HSC_DATA_TYPE_F32; goto PRINT_LIT;
			case HSC_TOKEN_LIT_F64: data_type = HSC_DATA_TYPE_F64; goto PRINT_LIT;
PRINT_LIT:
				value = compiler.astgen.token_values[token_value_idx];
				HscConstant constant = hsc_constant_table_get(&compiler.astgen.constant_table, value.constant_id);
				hsc_data_type_print(&compiler.astgen, data_type, constant.data, stdout);
				printf("\n");
				token_value_idx += 1;
				break;
			default:
				printf("%s\n", hsc_token_strings[token]);
				break;
		}
	}

	return 0;
}


