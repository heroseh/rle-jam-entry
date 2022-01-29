
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

// ===========================================
//
//
// General
//
//
// ===========================================

#ifndef HSC_ALLOC
#define HSC_ALLOC(size, align) malloc(size)
#endif

#ifndef HSC_DEALLOC
#define HSC_DEALLOC(ptr, size, align) free(ptr)
#endif

#define HSC_ALLOC_ELMT(T) HSC_ALLOC(sizeof(T), alignof(T))
#define HSC_DEALLOC_ELMT(T, ptr) HSC_DEALLOC(ptr, sizeof(T), alignof(T))
#define HSC_ALLOC_ARRAY(T, count) HSC_ALLOC((count) * sizeof(T), alignof(T))
#define HSC_DEALLOC_ARRAY(T, ptr, count) HSC_DEALLOC(ptr, (count) * sizeof(T), alignof(T))

#ifndef HSC_STATIC_ASSERT
#define HSC_STATIC_ASSERT(x, msg) int hsc_sa(int hsc_sa[(x)?1:-1])
#endif

#ifndef HSC_ABORT
#define HSC_ABORT(...) _hsc_abort(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HSC_ASSERT
#define HSC_ASSERT(cond, ...) if (HSC_UNLIKELY(!(cond))) _hsc_assert_failed(#cond, __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HSC_DEBUG_ASSERTIONS
#define HSC_DEBUG_ASSERTIONS 0
#endif

#define HSC_ASSERT_ARRAY_BOUNDS(idx, count) HSC_ASSERT((idx) < (count), "idx '%zu' is out of bounds for an array of count '%zu'", (idx), (count));

#define HSC_STRINGIFY(v) #v
#define HSC_CONCAT_0(a, b) a##b
#define HSC_CONCAT(a, b) HSC_CONCAT_0(a, b)

#if HSC_DEBUG_ASSERTIONS
#define HSC_DEBUG_ASSERT HSC_ASSERT
#else
#define HSC_DEBUG_ASSERT(cond, ...) (void)(cond)
#endif

#ifdef __GNUC__
#define HSC_LIKELY(expr) __builtin_expect((expr), 1)
#define HSC_UNLIKELY(expr) __builtin_expect((expr), 0)
#else
#define HSC_LIKELY(expr) expr
#define HSC_UNLIKELY(expr) expr
#endif

#if HSC_DEBUG_ASSERTIONS
#define HSC_UNREACHABLE() HSC_ABORT("unreachable code");
#else
#define HSC_UNREACHABLE() __builtin_unreachable()
#endif

#ifdef __GNUC__
#define HSC_NORETURN __attribute__((noreturn))
#else
#define HSC_NORETURN
#endif

#define HSC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HSC_MAX(a, b) (((a) > (b)) ? (a) : (b))

void _hsc_assert_failed(const char* cond, const char* file, int line, const char* message, ...);
HSC_NORETURN void _hsc_abort(const char* file, int line, const char* message, ...);

#define HSC_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))

#define HSC_DEFINE_ID(Name) typedef struct Name { uint32_t idx_plus_one; } Name;
HSC_DEFINE_ID(HscFileId);
HSC_DEFINE_ID(HscStringId);

typedef struct HscLocation HscLocation;
struct HscLocation {
	HscFileId file_id;
	uint32_t code_start_idx;
	uint32_t code_end_idx;
	uint32_t line_start;
	uint32_t line_end;
	uint32_t column_start;
	uint32_t column_end;
};

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef uint8_t  S8;
typedef uint16_t S16;
typedef uint32_t S32;
typedef uint64_t S64;
typedef float    F32;
typedef double   F64;

#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX
#define S8_MIN  INT8_MIN
#define S8_MAX  INT8_MAX
#define S16_MIN INT16_MIN
#define S16_MAX INT16_MAX
#define S32_MIN INT32_MIN
#define S32_MAX INT32_MAX
#define S64_MIN INT64_MIN
#define S64_MAX INT64_MAX

// ===========================================
//
//
// Syntax Generator
//
//
// ===========================================

typedef uint8_t HscToken;
enum {
	HSC_TOKEN_INVALID,
	HSC_TOKEN_EOF,
	HSC_TOKEN_IDENT,

	//
	// symbols
	//
	HSC_TOKEN_CURLY_OPEN,
	HSC_TOKEN_CURLY_CLOSE,
	HSC_TOKEN_PARENTHESIS_OPEN,
	HSC_TOKEN_PARENTHESIS_CLOSE,
	HSC_TOKEN_FULL_STOP,
	HSC_TOKEN_COMMA,
	HSC_TOKEN_SEMICOLON,
	HSC_TOKEN_PLUS,
	HSC_TOKEN_MINUS,
	HSC_TOKEN_FORWARD_SLASH,
	HSC_TOKEN_ASTERISK,
	HSC_TOKEN_PERCENT,

	HSC_TOKEN_LIT_TRUE,
	HSC_TOKEN_LIT_FALSE,
	HSC_TOKEN_LIT_U32,
	HSC_TOKEN_LIT_U64,
	HSC_TOKEN_LIT_S32,
	HSC_TOKEN_LIT_S64,
	HSC_TOKEN_LIT_F32,
	HSC_TOKEN_LIT_F64,

	//
	// keywords
	//
#define HSC_TOKEN_KEYWORDS_FIRST HSC_TOKEN_KEYWORD_RETURN
	HSC_TOKEN_KEYWORD_RETURN,
	HSC_TOKEN_KEYWORD_VERTEX,
	HSC_TOKEN_KEYWORD_FRAGMENT,
	HSC_TOKEN_KEYWORD_GEOMETRY,
	HSC_TOKEN_KEYWORD_TESSELLATION,
	HSC_TOKEN_KEYWORD_COMPUTE,
	HSC_TOKEN_KEYWORD_MESHTASK,
	HSC_TOKEN_KEYWORD_STRUCT,
	HSC_TOKEN_KEYWORD_RO_BUFFER,
	HSC_TOKEN_KEYWORD_RW_BUFFER,
	HSC_TOKEN_KEYWORD_RO_IMAGE1D,
	HSC_TOKEN_KEYWORD_RW_IMAGE1D,
	HSC_TOKEN_KEYWORD_RO_IMAGE2D,
	HSC_TOKEN_KEYWORD_RW_IMAGE2D,
	HSC_TOKEN_KEYWORD_RO_IMAGE3D,
	HSC_TOKEN_KEYWORD_RW_IMAGE3D,
#define HSC_TOKEN_KEYWORDS_LAST HSC_TOKEN_KEYWORD_RW_IMAGE3D
#define HSC_TOKEN_KEYWORDS_COUNT HSC_TOKEN_KEYWORDS_LAST - HSC_TOKEN_KEYWORDS_FIRST

	//
	// intrinsic types
	//
#define HSC_TOKEN_INTRINSIC_TYPES_FIRST HSC_TOKEN_INTRINSIC_TYPE_U8
	HSC_TOKEN_INTRINSIC_TYPE_U8,
	HSC_TOKEN_INTRINSIC_TYPE_U16,
	HSC_TOKEN_INTRINSIC_TYPE_U32,
	HSC_TOKEN_INTRINSIC_TYPE_U64,
	HSC_TOKEN_INTRINSIC_TYPE_F8,
	HSC_TOKEN_INTRINSIC_TYPE_F16,
	HSC_TOKEN_INTRINSIC_TYPE_F32,
	HSC_TOKEN_INTRINSIC_TYPE_F64,
#define HSC_TOKEN_INTRINSIC_TYPE_VECTORS_FIRST HSC_TOKEN_INTRINSIC_TYPE_VEC2
	HSC_TOKEN_INTRINSIC_TYPE_VEC2,
	HSC_TOKEN_INTRINSIC_TYPE_UVEC2,
	HSC_TOKEN_INTRINSIC_TYPE_SVEC2,
	HSC_TOKEN_INTRINSIC_TYPE_VEC3,
	HSC_TOKEN_INTRINSIC_TYPE_UVEC3,
	HSC_TOKEN_INTRINSIC_TYPE_SVEC3,
	HSC_TOKEN_INTRINSIC_TYPE_VEC4,
	HSC_TOKEN_INTRINSIC_TYPE_UVEC4,
	HSC_TOKEN_INTRINSIC_TYPE_SVEC4,
#define HSC_TOKEN_INTRINSIC_TYPE_VECTORS_LAST HSC_TOKEN_INTRINSIC_TYPE_SVEC4
#define HSC_TOKEN_INTRINSIC_TYPE_MATRICES_FIRST HSC_TOKEN_INTRINSIC_TYPE_MAT2X2
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X4,
#define HSC_TOKEN_INTRINSIC_TYPE_MATRICES_LAST HSC_TOKEN_INTRINSIC_TYPE_MAT4X4
#define HSC_TOKEN_INTRINSIC_TYPES_LAST HSC_TOKEN_INTRINSIC_TYPE_MAT4X4
#define HSC_TOKEN_INTRINSIC_TYPES_COUNT HSC_TOKEN_INTRINSIC_TYPES_LAST - HSC_TOKEN_INTRINSIC_TYPES_FIRST

	HSC_TOKEN_COUNT,
};

enum {
	HSC_STRING_ID_NULL = 0,

	HSC_STRING_ID_KEYWORDS_FIRST,
	HSC_STRING_ID_KEYWORDS_LAST = HSC_STRING_ID_KEYWORDS_FIRST + HSC_TOKEN_KEYWORDS_COUNT,

	HSC_STRING_ID_INTRINSIC_TYPES_FIRST,
	HSC_STRING_ID_INTRINSIC_TYPES_LAST = HSC_STRING_ID_INTRINSIC_TYPES_FIRST + HSC_TOKEN_INTRINSIC_TYPES_COUNT,
};

typedef union HscTokenValue HscTokenValue;
union HscTokenValue {
	uint64_t    u64;
	int64_t     s64;
	double      f64;
	HscStringId string_id;
};

extern char* hsc_token_strings[HSC_TOKEN_COUNT];

typedef struct HscString HscString;
struct HscString {
	char* data;
	uintptr_t size;
};

typedef struct HscStringEntry HscStringEntry;
struct HscStringEntry {
	uint32_t start_idx;
	uint32_t size;
};

typedef struct HscStringTable HscStringTable;
struct HscStringTable {
	char*           data;
	HscStringEntry* entries;
	uint32_t        data_used_size;
	uint32_t        data_cap;
	uint32_t        entries_count;
	uint32_t        entries_cap;
};

typedef struct HscSynGen HscSynGen;
struct HscSynGen {
	HscToken* tokens;
	HscLocation* token_locations;
	HscTokenValue* token_values;
	U32* line_code_start_indices;
	HscStringTable string_table;
	uint32_t token_read_idx;
	uint32_t token_value_read_idx;
	uint32_t tokens_count;
	uint32_t tokens_cap;
	uint32_t token_values_count;
	uint32_t token_values_cap;
	U32 lines_count;
	U32 lines_cap;
	HscLocation location;
	const char* file_path;
	uint8_t* bytes;
	uint32_t bytes_count;
	bool print_color;
};

void hsc_string_table_init(HscStringTable* string_table, uint32_t data_cap, uint32_t entries_cap);
HscStringId hsc_string_table_deduplicate(HscStringTable* string_table, char* string, uint32_t string_size);
HscString hsc_string_table_get(HscStringTable* string_table, HscStringId id);

void hsc_syngen_init(HscSynGen* syngen, uint32_t tokens_cap, U32 lines_cap);
void hsc_syngen_token_error_1(HscSynGen* syngen, const char* fmt, ...);
void hsc_syngen_token_error_2(HscSynGen* syngen, HscLocation* other_location, const char* fmt, ...);
void hsc_syngen_add_token(HscSynGen* syngen, HscToken token);
void hsc_syngen_add_token_value(HscSynGen* syngen, HscTokenValue value);
void hsc_syngen_tokenize(HscSynGen* syngen);
HscToken hsc_token_peek(HscSynGen* syngen);
HscToken hsc_token_next(HscSynGen* syngen);

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

typedef struct HscCompiler HscCompiler;
struct HscCompiler {
	HscSynGen syngen;

};

typedef struct HscCompilerSetup HscCompilerSetup;
struct HscCompilerSetup {
	uint32_t tokens_cap;
	uint32_t lines_cap;
	uint32_t string_table_data_cap;
	uint32_t string_table_entries_cap;
};

void hsc_compiler_init(HscCompiler* compiler, HscCompilerSetup* setup);

void hsc_compiler_compile(HscCompiler* compiler, const char* file_path);

