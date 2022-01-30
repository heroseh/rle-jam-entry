
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

// align must be a power of 2
#define HSC_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))
// align must be a power of 2
#define HSC_INT_ROUND_DOWN_ALIGN(i, align) ((i) & ~((align) - 1))

#define HSC_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define HSC_IS_POWER_OF_TWO(v) (((v) != 0) && (((v) & ((v) - 1)) == 0))
#define HSC_PTR_ADD(ptr, by) (void*)((Uptr)(ptr) + (Uptr)(by))
#define HSC_PTR_SUB(ptr, by) (void*)((Uptr)(ptr) - (Uptr)(by))
#define HSC_PTR_DIFF(to, from) ((char*)(to) - (char*)(from))
// align must be a power of 2
#define HSC_PTR_ROUND_UP_ALIGN(ptr, align) ((void*)HSC_INT_ROUND_UP_ALIGN((Uptr)ptr, align))
// align must be a power of 2
#define HSC_PTR_ROUND_DOWN_ALIGN(ptr, align) ((void*)HSC_INT_ROUND_DOWN_ALIGN((Uptr)ptr, align))
#define HSC_ZERO_ELMT(ptr) memset(ptr, 0, sizeof(*(ptr)))
#define HSC_ONE_ELMT(ptr) memset(ptr, 0xff, sizeof(*(ptr)))
#define HSC_ZERO_ELMT_MANY(ptr, elmts_count) memset(ptr, 0, sizeof(*(ptr)) * (elmts_count))
#define HSC_ONE_ELMT_MANY(ptr, elmts_count) memset(ptr, 0xff, sizeof(*(ptr)) * (elmts_count))
#define HSC_ZERO_ARRAY(array) memset(array, 0, sizeof(array))
#define HSC_ONE_ARRAY(array) memset(array, 0xff, sizeof(array))
#define HSC_COPY_ARRAY(dst, src) memcpy(dst, src, sizeof(dst))
#define HSC_COPY_ELMT_MANY(dst, src, elmts_count) memcpy(dst, src, elmts_count * sizeof(*(dst)))
#define HSC_COPY_OVERLAP_ELMT_MANY(dst, src, elmts_count) memmove(dst, src, elmts_count * sizeof(*(dst)))
#define HSC_CMP_ARRAY(a, b) (memcmp(a, b, sizeof(a)) == 0)
#define HSC_CMP_ELMT(a, b) (memcmp(a, b, sizeof(*(a))) == 0)
#define HSC_CMP_ELMT_MANY(a, b, elmts_count) (memcmp(a, b, elmts_count * sizeof(*(a))) == 0)

#define HSC_DEFINE_ID(Name) typedef struct Name { uint32_t idx_plus_one; } Name;
HSC_DEFINE_ID(HscFileId);
HSC_DEFINE_ID(HscStringId);
HSC_DEFINE_ID(HscCompoundTypeId);
HSC_DEFINE_ID(HscArrayTypeId);
HSC_DEFINE_ID(HscFnTypeId);

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
// Hash Table
//
//
// ===========================================

typedef struct HscHashTable HscHashTable;
struct HscHashTable {
	U32* keys;
	U32* values;
	U32 count;
	U32 cap;
};

void hsc_hash_table_init(HscHashTable* hash_table);
bool hsc_hash_table_find(HscHashTable* hash_table, U32 key, U32** value_ptr_out);
bool hsc_hash_table_find_or_insert(HscHashTable* hash_table, U32 key, U32** value_ptr_out);

// ===========================================
//
//
// Syntax Generator
//
//
// ===========================================

typedef U32 HscType;
enum {
#define HSC_TYPE_BASIC_START HSC_TYPE_VOID
	HSC_TYPE_VOID,
	HSC_TYPE_BOOL,
	HSC_TYPE_U8,
	HSC_TYPE_U16,
	HSC_TYPE_U32,
	HSC_TYPE_U64,
	HSC_TYPE_S8,
	HSC_TYPE_S16,
	HSC_TYPE_S32,
	HSC_TYPE_S64,
	HSC_TYPE_F8,
	HSC_TYPE_F16,
	HSC_TYPE_F32,
	HSC_TYPE_F64,
#define HSC_TYPE_BASIC_END (HSC_TYPE_F64 + 1)
#define HSC_TYPE_BASIC_COUNT (HSC_TYPE_BASIC_END - HSC_TYPE_BASIC_START)

#define HSC_TYPE_VECTOR_START HSC_TYPE_VEC2_START
	HSC_TYPE_VEC2_START = 16,
	HSC_TYPE_VEC3_START = HSC_TYPE_VEC2_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_VEC4_START = HSC_TYPE_VEC3_START + HSC_TYPE_VEC2_START,
#define HSC_TYPE_VECTOR_END HSC_TYPE_MAT2x2_START
#define HSC_TYPE_MATRIX_START HSC_TYPE_MAT2x2_START
	HSC_TYPE_MAT2x2_START = HSC_TYPE_VEC4_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT2x3_START = HSC_TYPE_MAT2x2_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT2x4_START = HSC_TYPE_MAT2x3_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT3x2_START = HSC_TYPE_MAT2x4_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT3x3_START = HSC_TYPE_MAT3x2_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT3x4_START = HSC_TYPE_MAT3x3_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT4x2_START = HSC_TYPE_MAT3x4_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT4x3_START = HSC_TYPE_MAT4x2_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_MAT4x4_START = HSC_TYPE_MAT4x3_START + HSC_TYPE_VEC2_START,
#define HSC_TYPE_MATRIX_END HSC_TYPE_STRUCT_FLAG
	HSC_TYPE_STRUCT_FLAG = HSC_TYPE_MAT4x4_START + HSC_TYPE_VEC2_START,
	HSC_TYPE_UNION_FLAG,
	HSC_TYPE_ARRAY_FLAG,
	HSC_TYPE_COUNT,
};

#define HSC_TYPE_SCALAR(type)            ((type) & (HSC_TYPE_VEC2_START - 1))
#define HSC_TYPE_IS_BASIC(type)          ((type) >= HSC_TYPE_VEC2_START && (type) <= HSC_TYPE_MAT2x2_START)
#define HSC_TYPE_IS_VECTOR(type)         ((type) >= HSC_TYPE_VECTOR_START && (type) < HSC_TYPE_VECTOR_END)
#define HSC_TYPE_IS_MATRIX(type)         ((type) >= HSC_TYPE_MATRIX_START && (type) < HSC_TYPE_MATRIX_END)
#define HSC_TYPE_IS_STRUCT(type)         ((type) & HSC_TYPE_STRUCT_FLAG)
#define HSC_TYPE_IS_UNION(type)          ((type) & HSC_TYPE_UNION_FLAG)
#define HSC_TYPE_IS_COMPOUND_TYPE(type)  ((type) & (HSC_TYPE_STRUCT_FLAG | HSC_TYPE_UNION_FLAG))
#define HSC_TYPE_IS_ARRAY(type)          ((type) & HSC_TYPE_ARRAY_FLAG)
#define HSC_TYPE_VECTOR_COMPONENTS(type) ((type) / HSC_TYPE_VEC2_START)
#define HSC_TYPE_MATRX_COLUMNS(type)     (((type) + 32) / 48)
#define HSC_TYPE_MATRX_ROWS(type)        (((((type) - 64) / 16) + 1) & 3) + 2)
#define HSC_TYPE_COMPOUND_TYPE_ID(type)  ((HscCompoundTypeId) { .idx_plus_one = (type) >> 8 })
#define HSC_TYPE_ARRAY_TYPE_ID(type)     ((HscArrayId) { .idx_plus_one = (type) >> 8 })

//
// 'basic_type' must be HSC_TYPE_IS_BASIC(basic_type) == true
#define HSC_TYPE_VEC2(basic_type)   (HSC_TYPE_VEC2_START + (basic_type))
#define HSC_TYPE_VEC3(basic_type)   (HSC_TYPE_VEC3_START + (basic_type))
#define HSC_TYPE_VEC4(basic_type)   (HSC_TYPE_VEC4_START + (basic_type))
#define HSC_TYPE_MAT2x2(basic_type) (HSC_TYPE_MAT2x2_START + (basic_type))
#define HSC_TYPE_MAT2x3(basic_type) (HSC_TYPE_MAT2x3_START + (basic_type))
#define HSC_TYPE_MAT2x4(basic_type) (HSC_TYPE_MAT2x4_START + (basic_type))
#define HSC_TYPE_MAT3x2(basic_type) (HSC_TYPE_MAT3x2_START + (basic_type))
#define HSC_TYPE_MAT3x3(basic_type) (HSC_TYPE_MAT3x3_START + (basic_type))
#define HSC_TYPE_MAT3x4(basic_type) (HSC_TYPE_MAT3x4_START + (basic_type))
#define HSC_TYPE_MAT4x2(basic_type) (HSC_TYPE_MAT4x2_START + (basic_type))
#define HSC_TYPE_MAT4x3(basic_type) (HSC_TYPE_MAT4x3_START + (basic_type))
#define HSC_TYPE_MAT4x4(basic_type) (HSC_TYPE_MAT4x4_START + (basic_type))

//
// inherits HscType
typedef U32 HscDecl;
enum {
	HSC_DECL_FN_FLAG = HSC_TYPE_COUNT,
};
#define HSC_DECL_IS_FN(type) ((type) & HSC_DECL_FN_FLAG)
#define HSC_DECL_FN_ID(type) ((HscFnId) { .idx_plus_one = (type) >> 8 })

typedef U8 HscToken;
enum {
#define HSC_TOKEN_INTRINSIC_TYPES_START HSC_TYPE_BASIC_START
	//
	// INFO:
	// HSC_TYPE_BASIC_START - HSC_TYPE_BASIC_END are used as HscToken too!
	//
#define HSC_TOKEN_INTRINSIC_TYPE_VECTORS_START HSC_TOKEN_INTRINSIC_TYPE_VEC2
	HSC_TOKEN_INTRINSIC_TYPE_VEC2 = HSC_TYPE_BASIC_END,
	HSC_TOKEN_INTRINSIC_TYPE_VEC3,
	HSC_TOKEN_INTRINSIC_TYPE_VEC4,
#define HSC_TOKEN_INTRINSIC_TYPE_VECTORS_END HSC_TOKEN_INTRINSIC_TYPE_MAT2X2
#define HSC_TOKEN_INTRINSIC_TYPE_MATRICES_START HSC_TOKEN_INTRINSIC_TYPE_MAT2X2
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT2X4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT3X4,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X2,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X3,
	HSC_TOKEN_INTRINSIC_TYPE_MAT4X4,
#define HSC_TOKEN_INTRINSIC_TYPE_MATRICES_END HSC_TOKEN_EOF
#define HSC_TOKEN_INTRINSIC_TYPES_END HSC_TOKEN_EOF
#define HSC_TOKEN_INTRINSIC_TYPES_COUNT (HSC_TOKEN_INTRINSIC_TYPES_END - HSC_TOKEN_INTRINSIC_TYPES_START)

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
#define HSC_TOKEN_KEYWORDS_START HSC_TOKEN_KEYWORD_RETURN
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
#define HSC_TOKEN_KEYWORDS_END HSC_TOKEN_COUNT
#define HSC_TOKEN_KEYWORDS_COUNT (HSC_TOKEN_KEYWORDS_END - HSC_TOKEN_KEYWORDS_START)

	HSC_TOKEN_COUNT,
};

enum {
	HSC_STRING_ID_NULL = 0,

	HSC_STRING_ID_KEYWORDS_START,
#define HSC_STRING_ID_KEYWORDS_END HSC_STRING_ID_INTRINSIC_TYPES_START

	HSC_STRING_ID_INTRINSIC_TYPES_START = HSC_STRING_ID_KEYWORDS_START + HSC_TOKEN_KEYWORDS_COUNT,
	HSC_STRING_ID_INTRINSIC_TYPES_END = HSC_STRING_ID_INTRINSIC_TYPES_START + HSC_TOKEN_INTRINSIC_TYPES_COUNT,
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

typedef struct HscAstGen HscAstGen;
struct HscAstGen {
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

void hsc_astgen_init(HscAstGen* astgen, uint32_t tokens_cap, U32 lines_cap);
void hsc_astgen_token_error_1(HscAstGen* astgen, const char* fmt, ...);
void hsc_astgen_token_error_2(HscAstGen* astgen, HscLocation* other_location, const char* fmt, ...);
void hsc_astgen_add_token(HscAstGen* astgen, HscToken token);
void hsc_astgen_add_token_value(HscAstGen* astgen, HscTokenValue value);
void hsc_astgen_tokenize(HscAstGen* astgen);
HscToken hsc_token_peek(HscAstGen* astgen);
HscToken hsc_token_next(HscAstGen* astgen);

// ===========================================
//
//
// Compiler
//
//
// ===========================================

typedef struct HscCompiler HscCompiler;
struct HscCompiler {
	HscAstGen astgen;

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

