
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
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
#define HSC_DEBUG_ASSERTIONS 1
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
#define HSC_UNREACHABLE(...) HSC_ABORT("unreachable code: " __VA_ARGS__);
#else
#define HSC_UNREACHABLE(...) __builtin_unreachable()
#endif

#ifdef __GNUC__
#define HSC_NORETURN __attribute__((noreturn))
#else
#define HSC_NORETURN
#endif

#define HSC_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HSC_MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifndef alignof
#define alignof _Alignof
#endif

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
#define HSC_PTR_ROUND_UP_ALIGN(ptr, align) ((void*)HSC_INT_ROUND_UP_ALIGN((Uptr)(ptr), align))
// align must be a power of 2
#define HSC_PTR_ROUND_DOWN_ALIGN(ptr, align) ((void*)HSC_INT_ROUND_DOWN_ALIGN((Uptr)(ptr), align))
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

#define HSC_DIV_ROUND_UP(a, b) (((a) / (b)) + ((a) % (b) != 0))

#define HSC_DEFINE_ID(Name) typedef struct Name { uint32_t idx_plus_one; } Name;
HSC_DEFINE_ID(HscFileId);
HSC_DEFINE_ID(HscStringId);
HSC_DEFINE_ID(HscConstantId);
HSC_DEFINE_ID(HscFunctionTypeId);
HSC_DEFINE_ID(HscExprId);

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

typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;
typedef uintptr_t Uptr;
typedef int8_t    S8;
typedef int16_t   S16;
typedef int32_t   S32;
typedef int64_t   S64;
typedef intptr_t  Sptr;
typedef float     F32;
typedef double    F64;

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

typedef struct HscCompiler HscCompiler;

#define HSC_COMPOUND_TYPE_NESTED_FIELD_CAP 16

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

#define HscHashTable(Key, Value) HscHashTable

typedef struct HscHashTable HscHashTable;
struct HscHashTable {
	U32* keys;
	U32* values;
	U32 count;
	U32 cap;
};

void hsc_hash_table_init(HscHashTable* hash_table);
bool hsc_hash_table_find(HscHashTable* hash_table, U32 key, U32* value_out);
bool hsc_hash_table_find_or_insert(HscHashTable* hash_table, U32 key, U32** value_ptr_out);
bool hsc_hash_table_remove(HscHashTable* hash_table, U32 key, U32* value_out);
void hsc_hash_table_clear(HscHashTable* hash_table);

// ===========================================
//
//
// Syntax Generator
//
//
// ===========================================

typedef U32 HscDataType;
enum {
#define HSC_DATA_TYPE_BASIC_START HSC_DATA_TYPE_VOID
	HSC_DATA_TYPE_VOID,
	HSC_DATA_TYPE_BOOL,
	HSC_DATA_TYPE_U8,
	HSC_DATA_TYPE_U16,
	HSC_DATA_TYPE_U32,
	HSC_DATA_TYPE_U64,
	HSC_DATA_TYPE_S8,
	HSC_DATA_TYPE_S16,
	HSC_DATA_TYPE_S32,
	HSC_DATA_TYPE_S64,
	HSC_DATA_TYPE_F16,
	HSC_DATA_TYPE_F32,
	HSC_DATA_TYPE_F64,
#define HSC_DATA_TYPE_BASIC_END (HSC_DATA_TYPE_F64 + 1)
#define HSC_DATA_TYPE_BASIC_COUNT (HSC_DATA_TYPE_BASIC_END - HSC_DATA_TYPE_BASIC_START)

#define HSC_DATA_TYPE_VECTOR_START HSC_DATA_TYPE_VEC2_START
	HSC_DATA_TYPE_VEC2_START = 16,
#define HSC_DATA_TYPE_VEC2_END HSC_DATA_TYPE_VEC3_START
	HSC_DATA_TYPE_VEC3_START = HSC_DATA_TYPE_VEC2_START + HSC_DATA_TYPE_VEC2_START,
#define HSC_DATA_TYPE_VEC3_END HSC_DATA_TYPE_VEC4_START
	HSC_DATA_TYPE_VEC4_START = HSC_DATA_TYPE_VEC3_START + HSC_DATA_TYPE_VEC2_START,
#define HSC_DATA_TYPE_VEC4_END HSC_DATA_TYPE_MAT2x2_START
#define HSC_DATA_TYPE_VECTOR_END HSC_DATA_TYPE_MAT2x2_START
#define HSC_DATA_TYPE_MATRIX_START HSC_DATA_TYPE_MAT2x2_START
	HSC_DATA_TYPE_MAT2x2_START = HSC_DATA_TYPE_VEC4_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT2x3_START = HSC_DATA_TYPE_MAT2x2_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT2x4_START = HSC_DATA_TYPE_MAT2x3_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT3x2_START = HSC_DATA_TYPE_MAT2x4_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT3x3_START = HSC_DATA_TYPE_MAT3x2_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT3x4_START = HSC_DATA_TYPE_MAT3x3_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT4x2_START = HSC_DATA_TYPE_MAT3x4_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT4x3_START = HSC_DATA_TYPE_MAT4x2_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_MAT4x4_START = HSC_DATA_TYPE_MAT4x3_START + HSC_DATA_TYPE_VEC2_START,
#define HSC_DATA_TYPE_MATRIX_END HSC_DATA_TYPE_STRUCT
	HSC_DATA_TYPE_ENUM = HSC_DATA_TYPE_MAT4x4_START + HSC_DATA_TYPE_VEC2_START,
	HSC_DATA_TYPE_STRUCT,
	HSC_DATA_TYPE_UNION,
	HSC_DATA_TYPE_ARRAY,
	HSC_DATA_TYPE_TYPEDEF,

#define HSC_DATA_TYPE_GENERIC_START HSC_DATA_TYPE_GENERIC_SCALAR
	HSC_DATA_TYPE_GENERIC_SCALAR,
	HSC_DATA_TYPE_GENERIC_VEC2,
	HSC_DATA_TYPE_GENERIC_VEC3,
	HSC_DATA_TYPE_GENERIC_VEC4,
#define HSC_DATA_TYPE_GENERIC_END (HSC_DATA_TYPE_GENERIC_VEC4 + 1)

	HSC_DATA_TYPE_COUNT,
};

#define HSC_DATA_TYPE_SCALAR(type)                ((type) & (HSC_DATA_TYPE_VEC2_START - 1))
#define HSC_DATA_TYPE_IS_BASIC(type)              ((type) < HSC_DATA_TYPE_BASIC_END)
#define HSC_DATA_TYPE_IS_INT(type)                ((type) >= HSC_DATA_TYPE_U8 && (type) <= HSC_DATA_TYPE_S64)
#define HSC_DATA_TYPE_IS_VECTOR(type)             ((type) >= HSC_DATA_TYPE_VECTOR_START && (type) < HSC_DATA_TYPE_VECTOR_END)
#define HSC_DATA_TYPE_IS_MATRIX(type)             ((type) >= HSC_DATA_TYPE_MATRIX_START && (type) < HSC_DATA_TYPE_MATRIX_END)
#define HSC_DATA_TYPE_IS_STRUCT(type)             (((type) & 0xff) == HSC_DATA_TYPE_STRUCT)
#define HSC_DATA_TYPE_IS_UNION(type)              (((type) & 0xff) == HSC_DATA_TYPE_UNION)
#define HSC_DATA_TYPE_IS_COMPOUND_TYPE(type)      (HSC_DATA_TYPE_IS_STRUCT(type) || HSC_DATA_TYPE_IS_UNION(type))
#define HSC_DATA_TYPE_IS_ARRAY(type)              (((type) & 0xff) == HSC_DATA_TYPE_ARRAY)
#define HSC_DATA_TYPE_IS_TYPEDEF(type)            (((type) & 0xff) == HSC_DATA_TYPE_TYPEDEF)
#define HSC_DATA_TYPE_IS_ENUM_TYPE(type)          (((type) & 0xff) == HSC_DATA_TYPE_ENUM)
#define HSC_DATA_TYPE_IS_GENERIC(type)            ((type) >= HSC_DATA_TYPE_GENERIC_START && (type) < HSC_DATA_TYPE_GENERIC_END)
#define HSC_DATA_TYPE_VECTOR_COMPONENTS(type)     (((type) / HSC_DATA_TYPE_VEC2_START) + 1)
#define HSC_DATA_TYPE_MATRX_COLUMNS(type)         (((type) + 32) / 48)
#define HSC_DATA_TYPE_MATRX_ROWS(type)            ((((((type) - 64) / 16) + 1) & 3) + 2)
#define HSC_DATA_TYPE_INIT(type, idx)             (((idx) << 8) | (type))
#define HSC_DATA_TYPE_IDX(type)                   ((type) >> 8)

//
// 'basic_type' must be HSC_DATA_TYPE_IS_BASIC(basic_type) == true
#define HSC_DATA_TYPE_VEC2(basic_type)   (HSC_DATA_TYPE_VEC2_START + (basic_type))
#define HSC_DATA_TYPE_VEC3(basic_type)   (HSC_DATA_TYPE_VEC3_START + (basic_type))
#define HSC_DATA_TYPE_VEC4(basic_type)   (HSC_DATA_TYPE_VEC4_START + (basic_type))
#define HSC_DATA_TYPE_MAT2x2(basic_type) (HSC_DATA_TYPE_MAT2x2_START + (basic_type))
#define HSC_DATA_TYPE_MAT2x3(basic_type) (HSC_DATA_TYPE_MAT2x3_START + (basic_type))
#define HSC_DATA_TYPE_MAT2x4(basic_type) (HSC_DATA_TYPE_MAT2x4_START + (basic_type))
#define HSC_DATA_TYPE_MAT3x2(basic_type) (HSC_DATA_TYPE_MAT3x2_START + (basic_type))
#define HSC_DATA_TYPE_MAT3x3(basic_type) (HSC_DATA_TYPE_MAT3x3_START + (basic_type))
#define HSC_DATA_TYPE_MAT3x4(basic_type) (HSC_DATA_TYPE_MAT3x4_START + (basic_type))
#define HSC_DATA_TYPE_MAT4x2(basic_type) (HSC_DATA_TYPE_MAT4x2_START + (basic_type))
#define HSC_DATA_TYPE_MAT4x3(basic_type) (HSC_DATA_TYPE_MAT4x3_START + (basic_type))
#define HSC_DATA_TYPE_MAT4x4(basic_type) (HSC_DATA_TYPE_MAT4x4_START + (basic_type))

typedef struct HscArrayDataType HscArrayDataType;
struct HscArrayDataType {
	HscDataType element_data_type;
	HscConstantId size_constant_id;
};

typedef U16 HscCompoundDataTypeFlags;
enum {
	HSC_COMPOUND_DATA_TYPE_FLAGS_IS_UNION = 0x1,
};

typedef struct HscCompoundDataType HscCompoundDataType;
struct HscCompoundDataType {
	Uptr                     size;
	Uptr                     align;
	U32                      identifier_token_idx;
	HscStringId              identifier_string_id;
	U32                      fields_start_idx;
	U16                      fields_count;
	U16                      largest_sized_field_idx;
	HscCompoundDataTypeFlags flags;
};

typedef struct HscCompoundField HscCompoundField;
struct HscCompoundField {
	HscStringId identifier_string_id;
	HscDataType data_type;
	U32         identifier_token_idx;
};

typedef struct HscEnumDataType HscEnumDataType;
struct HscEnumDataType {
	U32         identifier_token_idx;
	HscStringId identifier_string_id;
	U32         values_start_idx;
	U32         values_count;
};

typedef struct HscEnumValue HscEnumValue;
struct HscEnumValue {
	U32           identifier_token_idx;
	HscStringId   identifier_string_id;
	HscConstantId value_constant_id;
};

typedef struct HscTypedef HscTypedef;
struct HscTypedef {
	U32 identifier_token_idx;
	HscStringId identifier_string_id;
	HscDataType aliased_data_type;
};

//
// inherits HscDataType
typedef U32 HscDecl;
enum {
	HSC_DECL_FUNCTION = HSC_DATA_TYPE_COUNT,
	HSC_DECL_ENUM_VALUE,
};
#define HSC_DECL_IS_DATA_TYPE(type) (((type) & 0xff) < HSC_DATA_TYPE_COUNT)
#define HSC_DECL_IS_FUNCTION(type) (((type) & 0xff) == HSC_DECL_FUNCTION)
#define HSC_DECL_IS_ENUM_VALUE(type) (((type) & 0xff) == HSC_DECL_ENUM_VALUE)
#define HSC_DECL_INIT(type, idx) HSC_DATA_TYPE_INIT(type, idx)
#define HSC_DECL_IDX(type) HSC_DATA_TYPE_IDX(type)

typedef U8 HscToken;
enum {
#define HSC_TOKEN_INTRINSIC_TYPES_START HSC_DATA_TYPE_BASIC_START
	//
	// INFO:
	// HSC_DATA_TYPE_BASIC_START - HSC_DATA_TYPE_BASIC_END are used as HscToken too!
	//
#define HSC_TOKEN_INTRINSIC_TYPE_VECTORS_START HSC_TOKEN_INTRINSIC_TYPE_VEC2
	HSC_TOKEN_INTRINSIC_TYPE_VEC2 = HSC_DATA_TYPE_BASIC_END,
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
	HSC_TOKEN_SQUARE_OPEN,
	HSC_TOKEN_SQUARE_CLOSE,
	HSC_TOKEN_FULL_STOP,
	HSC_TOKEN_COMMA,
	HSC_TOKEN_SEMICOLON,
	HSC_TOKEN_COLON,
	HSC_TOKEN_PLUS,
	HSC_TOKEN_MINUS,
	HSC_TOKEN_FORWARD_SLASH,
	HSC_TOKEN_ASTERISK,
	HSC_TOKEN_PERCENT,
	HSC_TOKEN_AMPERSAND,
	HSC_TOKEN_PIPE,
	HSC_TOKEN_CARET,
	HSC_TOKEN_EXCLAMATION_MARK,
	HSC_TOKEN_TILDE,
	HSC_TOKEN_EQUAL,
	HSC_TOKEN_LESS_THAN,
	HSC_TOKEN_GREATER_THAN,

	//
	// grouped symbols
	//
	HSC_TOKEN_LOGICAL_AND,
	HSC_TOKEN_LOGICAL_OR,
	HSC_TOKEN_LOGICAL_EQUAL,
	HSC_TOKEN_LOGICAL_NOT_EQUAL,
	HSC_TOKEN_LESS_THAN_OR_EQUAL,
	HSC_TOKEN_GREATER_THAN_OR_EQUAL,
	HSC_TOKEN_BIT_SHIFT_LEFT,
	HSC_TOKEN_BIT_SHIFT_RIGHT,
	HSC_TOKEN_ADD_ASSIGN,
	HSC_TOKEN_SUBTRACT_ASSIGN,
	HSC_TOKEN_MULTIPLY_ASSIGN,
	HSC_TOKEN_DIVIDE_ASSIGN,
	HSC_TOKEN_MODULO_ASSIGN,
	HSC_TOKEN_BIT_SHIFT_LEFT_ASSIGN,
	HSC_TOKEN_BIT_SHIFT_RIGHT_ASSIGN,
	HSC_TOKEN_BIT_AND_ASSIGN,
	HSC_TOKEN_BIT_XOR_ASSIGN,
	HSC_TOKEN_BIT_OR_ASSIGN,

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
	HSC_TOKEN_KEYWORD_IF,
	HSC_TOKEN_KEYWORD_ELSE,
	HSC_TOKEN_KEYWORD_WHILE,
	HSC_TOKEN_KEYWORD_FOR,
	HSC_TOKEN_KEYWORD_SWITCH,
	HSC_TOKEN_KEYWORD_CASE,
	HSC_TOKEN_KEYWORD_DEFAULT,
	HSC_TOKEN_KEYWORD_BREAK,
	HSC_TOKEN_KEYWORD_CONTINUE,
	HSC_TOKEN_KEYWORD_TRUE,
	HSC_TOKEN_KEYWORD_FALSE,
	HSC_TOKEN_KEYWORD_VERTEX,
	HSC_TOKEN_KEYWORD_FRAGMENT,
	HSC_TOKEN_KEYWORD_GEOMETRY,
	HSC_TOKEN_KEYWORD_TESSELLATION,
	HSC_TOKEN_KEYWORD_COMPUTE,
	HSC_TOKEN_KEYWORD_MESHTASK,
	HSC_TOKEN_KEYWORD_ENUM,
	HSC_TOKEN_KEYWORD_STRUCT,
	HSC_TOKEN_KEYWORD_UNION,
	HSC_TOKEN_KEYWORD_TYPEDEF,
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

#define HSC_TOKEN_IS_BASIC_TYPE(token) ((token) < HSC_DATA_TYPE_BASIC_END)

enum {
	HSC_STRING_ID_NULL = 0,

#define HSC_STRING_ID_INTRINSIC_PARAM_NAMES_START HSC_STRING_ID_GENERIC_SCALAR
	HSC_STRING_ID_GENERIC_SCALAR,
	HSC_STRING_ID_GENERIC_VEC2,
	HSC_STRING_ID_GENERIC_VEC3,
	HSC_STRING_ID_GENERIC_VEC4,
	HSC_STRING_ID_SCALAR,
	HSC_STRING_ID_X,
	HSC_STRING_ID_Y,
	HSC_STRING_ID_Z,
	HSC_STRING_ID_W,
#define HSC_STRING_ID_INTRINSIC_PARAM_NAMES_END HSC_STRING_ID_KEYWORDS_START

	HSC_STRING_ID_KEYWORDS_START,
#define HSC_STRING_ID_KEYWORDS_END HSC_STRING_ID_INTRINSIC_TYPES_START

	HSC_STRING_ID_INTRINSIC_TYPES_START = HSC_STRING_ID_KEYWORDS_START + HSC_TOKEN_KEYWORDS_COUNT,
	HSC_STRING_ID_INTRINSIC_TYPES_END = HSC_STRING_ID_INTRINSIC_TYPES_START + HSC_TOKEN_INTRINSIC_TYPES_COUNT,
};

extern char* hsc_string_intrinsic_param_names[HSC_STRING_ID_INTRINSIC_PARAM_NAMES_END];

typedef union HscTokenValue HscTokenValue;
union HscTokenValue {
	HscConstantId constant_id;
	HscStringId string_id;
};

extern char* hsc_token_strings[HSC_TOKEN_COUNT];

typedef struct HscString HscString;
struct HscString {
	char* data;
	uintptr_t size;
};
#define hsc_string(data, size) ((HscString) { data, size });
#define hsc_string_lit(lit) ((HscString) { lit, sizeof(lit) - 1 });
#define hsc_string_c(string) ((HscString) { string, strlen(string) });

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

typedef struct HscConstant HscConstant;
struct HscConstant {
	void* data;
	uint32_t size;
	HscDataType data_type;
};

typedef struct HscConstantEntry HscConstantEntry;
struct HscConstantEntry {
	uint32_t start_idx;
	uint32_t size;
	HscDataType data_type;
	HscStringId debug_string_id;
};

typedef struct HscConstantTable HscConstantTable;
struct HscConstantTable {
	void*             data;
	HscConstantEntry* entries;
	uint32_t          data_used_size;
	uint32_t          data_cap;
	uint32_t          entries_count;
	uint32_t          entries_cap;

	HscDataType       data_type;
	U32               fields_count;
	U32               fields_cap;
	HscConstantId*    data_write_ptr;
};

typedef struct HscLocalVariable HscLocalVariable;
struct HscLocalVariable {
	HscStringId identifier_string_id;
	HscDataType data_type;
	U32         identifier_token_idx;
};

typedef U8 HscFunctionShaderStage;
enum {
	HSC_FUNCTION_SHADER_STAGE_NONE,
	HSC_FUNCTION_SHADER_STAGE_VERTEX,
	HSC_FUNCTION_SHADER_STAGE_FRAGMENT,
	HSC_FUNCTION_SHADER_STAGE_GEOMETRY,
	HSC_FUNCTION_SHADER_STAGE_TESSELLATION,
	HSC_FUNCTION_SHADER_STAGE_COMPUTE,
	HSC_FUNCTION_SHADER_STAGE_MESHTASK,

	HSC_FUNCTION_SHADER_STAGE_COUNT,
};
extern char* hsc_function_shader_stage_strings[HSC_FUNCTION_SHADER_STAGE_COUNT];

typedef struct HscFunction HscFunction;
struct HscFunction {
	HscLocation            location;
	HscStringId            identifier_string_id;
	HscDataType            return_data_type;
	U32                    params_start_idx;
	U16                    local_variables_count;
	U8                     params_count;
	HscFunctionShaderStage shader_stage;
	HscExprId              block_expr_id;
};

enum {
	HSC_FUNCTION_IDX_VEC2_SINGLE,
	HSC_FUNCTION_IDX_VEC2_MULTI,
	HSC_FUNCTION_IDX_VEC3_SINGLE,
	HSC_FUNCTION_IDX_VEC3_MULTI,
	HSC_FUNCTION_IDX_VEC4_SINGLE,
	HSC_FUNCTION_IDX_VEC4_MULTI,
	HSC_FUNCTION_IDX_MAT2x2,
	HSC_FUNCTION_IDX_MAT2x3,
	HSC_FUNCTION_IDX_MAT2x4,
	HSC_FUNCTION_IDX_MAT3x2,
	HSC_FUNCTION_IDX_MAT3x3,
	HSC_FUNCTION_IDX_MAT3x4,
	HSC_FUNCTION_IDX_MAT4x2,
	HSC_FUNCTION_IDX_MAT4x3,
	HSC_FUNCTION_IDX_MAT4x4,

#define HSC_FUNCTION_IDX_INTRINSIC_END HSC_FUNCTION_IDX_USER_START
	HSC_FUNCTION_IDX_USER_START,
};

typedef struct HscIntrinsicFunction HscIntrinsicFunction;
struct HscIntrinsicFunction {
	char* name;
	HscDataType return_data_type;
	U32 params_count;
	HscLocalVariable params[16];
};

extern U32 hsc_intrinsic_function_overloads_count[HSC_FUNCTION_IDX_INTRINSIC_END];
extern HscIntrinsicFunction hsc_intrinsic_functions[HSC_FUNCTION_IDX_INTRINSIC_END];

enum {
	HSC_INTRINSIC_FUNCTION_IDX_USER_START,
};

typedef U8 HscBinaryOp;
enum {
	HSC_BINARY_OP_ASSIGN,
	HSC_BINARY_OP_ADD,
	HSC_BINARY_OP_SUBTRACT,
	HSC_BINARY_OP_MULTIPLY,
	HSC_BINARY_OP_DIVIDE,
	HSC_BINARY_OP_MODULO,
	HSC_BINARY_OP_BIT_AND,
	HSC_BINARY_OP_BIT_OR,
	HSC_BINARY_OP_BIT_XOR,
	HSC_BINARY_OP_BIT_SHIFT_LEFT,
	HSC_BINARY_OP_BIT_SHIFT_RIGHT,

	//
	// logical operators (return a bool)
	HSC_BINARY_OP_EQUAL,
	HSC_BINARY_OP_NOT_EQUAL,
	HSC_BINARY_OP_LESS_THAN,
	HSC_BINARY_OP_LESS_THAN_OR_EQUAL,
	HSC_BINARY_OP_GREATER_THAN,
	HSC_BINARY_OP_GREATER_THAN_OR_EQUAL,

	HSC_BINARY_OP_COUNT,
};

typedef U8 HscUnaryOp;
enum {
	HSC_UNARY_OP_LOGICAL_NOT,
	HSC_UNARY_OP_BIT_NOT,
	HSC_UNARY_OP_PLUS,
	HSC_UNARY_OP_NEGATE,

	HSC_UNARY_OP_COUNT,
};

typedef U8 HscExprType;
enum {
	HSC_EXPR_TYPE_NONE,

	//
	// binary ops
	HSC_EXPR_TYPE_BINARY_OP_START,
#define HSC_EXPR_TYPE_BINARY_OP(OP) (HSC_EXPR_TYPE_BINARY_OP_START + HSC_BINARY_OP_##OP)
	HSC_EXPR_TYPE_BINARY_OP_END = HSC_EXPR_TYPE_BINARY_OP_START + HSC_BINARY_OP_COUNT,

	HSC_EXPR_TYPE_LOGICAL_AND,
	HSC_EXPR_TYPE_LOGICAL_OR,
	HSC_EXPR_TYPE_CALL,
	HSC_EXPR_TYPE_ARRAY_SUBSCRIPT,
	HSC_EXPR_TYPE_ARRAY_LIT,
	HSC_EXPR_TYPE_ARRAY_DESIGNATOR,
	HSC_EXPR_TYPE_FIELD_ACCESS,
	HSC_EXPR_TYPE_CAST,

	HSC_EXPR_TYPE_LOCAL_VARIABLE,

	HSC_EXPR_TYPE_CONSTANT,
	HSC_EXPR_TYPE_DATA_TYPE,
	HSC_EXPR_TYPE_FUNCTION,
	HSC_EXPR_TYPE_CALL_ARG_LIST,
	HSC_EXPR_TYPE_STMT_IF,
	HSC_EXPR_TYPE_STMT_SWITCH,
	HSC_EXPR_TYPE_STMT_WHILE,
	HSC_EXPR_TYPE_STMT_FOR,
	HSC_EXPR_TYPE_STMT_CASE,
	HSC_EXPR_TYPE_STMT_DEFAULT,
	HSC_EXPR_TYPE_STMT_BREAK,
	HSC_EXPR_TYPE_STMT_CONTINUE,

	//
	// unary ops
	HSC_EXPR_TYPE_UNARY_OP_START,
#define HSC_EXPR_TYPE_UNARY_OP(OP) (HSC_EXPR_TYPE_UNARY_OP_START + HSC_UNARY_OP_##OP)
	HSC_EXPR_TYPE_UNARY_OP_END = HSC_EXPR_TYPE_UNARY_OP_START + HSC_UNARY_OP_COUNT,

	HSC_EXPR_TYPE_STMT_RETURN,
	HSC_EXPR_TYPE_STMT_BLOCK,
};

typedef struct HscExpr HscExpr;
struct HscExpr {
	union {
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
		};
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 idx: 25;
		} function;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 expr_rel_idx: 25;
		} unary;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 is_assignment: 1;
			U32 left_expr_rel_idx: 11;
			U32 right_expr_rel_idx: 13;
		} binary;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 id: 25;
		} constant;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 has_return_stmt: 1;
			U32 stmts_count: 11;
			U32 first_expr_rel_idx: 6;
			U32 local_variables_count: 6;
		} stmt_block;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 first_expr_rel_idx: 12;
			U32 values_count: 13;
		} array_lit;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 12;
			U32 true_stmt_rel_idx: 13;
		} if_;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 12;
			U32 block_expr_rel_idx: 13;
			// alt_next_expr_rel_idx is the default_case_expr_rel_idx
		} switch_;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 cond_expr_rel_idx: 12;
			U32 loop_stmt_rel_idx: 13;
		} while_;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 init_expr_rel_idx: 5;
			U32 cond_expr_rel_idx: 6;
			U32 inc_expr_rel_idx: 7;
			U32 loop_stmt_rel_idx: 7;
		} for_;
		struct {
			U32 type: 6; // HscExprType
			U32 is_stmt_block_entry: 1;
			U32 idx: 25;
		} variable;
	};

	union {
		struct { // if is_stmt_block_entry
			U16 alt_next_expr_rel_idx;
			U16 next_expr_rel_idx;
		};
		struct { // this is stored in the true statement of the if statement
			U32 false_stmt_rel_idx: 16;
			U32 true_and_false_stmts_have_return_stmt: 1;
			U32 __unused: 15;
		} if_aux;
		struct {
			U32 case_stmts_count: 17;
			U32 first_case_expr_rel_idx: 15;
		} switch_aux;
		HscDataType data_type;
	};
};

HSC_STATIC_ASSERT(sizeof(HscExpr) == sizeof(U64), "HscExpr must be 8 bytes");

typedef U16 HscOpt;
enum {
	HSC_OPT_CONSTANT_FOLDING,

	HSC_OPT_COUNT,
};

typedef struct HscOpts HscOpts;
struct HscOpts {
	U64 bitset[HSC_DIV_ROUND_UP(HSC_OPT_COUNT, 8)];
};

typedef struct HscGenericDataTypeState HscGenericDataTypeState;
struct HscGenericDataTypeState {
	HscDataType scalar;
	HscDataType vec2;
	HscDataType vec3;
	HscDataType vec4;
};

typedef struct HscSwitchState HscSwitchState;
struct HscSwitchState {
	HscExpr* switch_stmt;
	HscExpr* first_switch_case;
	HscExpr* prev_switch_case;
	HscExpr* default_switch_case;
	HscDataType switch_condition_type;
	U32 case_stmts_count;
};

typedef struct HscFieldAccess HscFieldAccess;
struct HscFieldAccess {
	HscDataType data_type;
	U32 idx;
};

typedef struct HscAstGen HscAstGen;
struct HscAstGen {
	HscLocalVariable* function_params_and_local_variables;
	HscFunction*      functions;
	HscExpr*          exprs;
	HscLocation*      expr_locations;
	U32 function_params_and_local_variables_count;
	U32 function_params_and_local_variables_cap;
	U32 functions_count;
	U32 functions_cap;
	U32 exprs_count;
	U32 exprs_cap;

	HscArrayDataType* array_data_types;
	U32 array_data_types_count;
	U32 array_data_types_cap;

	HscCompoundDataType* compound_data_types;
	U32 compound_data_types_count;
	U32 compound_data_types_cap;
	HscCompoundField* compound_fields;
	U32 compound_fields_count;
	U32 compound_fields_cap;

	HscTypedef* typedefs;
	U32 typedefs_count;
	U32 typedefs_cap;

	HscEnumDataType* enum_data_types;
	U32 enum_data_types_count;
	U32 enum_data_types_cap;
	HscEnumValue* enum_values;
	U32 enum_values_count;
	U32 enum_values_cap;

	HscDataType* ordered_data_types;
	U32 ordered_data_types_count;
	U32 ordered_data_types_cap;

	HscDataType assign_data_type;

	HscFieldAccess compound_type_find_fields[HSC_COMPOUND_TYPE_NESTED_FIELD_CAP];
	U16 compound_type_find_fields_count;

	HscExpr* stmt_block;
	HscFunction* print_function;
	U32 print_variable_base_idx;

	HscSwitchState switch_state;
	bool is_in_loop;

	HscGenericDataTypeState generic_data_type_state;

	HscOpts opts;

	HscStringId* variable_stack_strings;
	U32*         variable_stack_var_indices;
	U32          variable_stack_count;
	U32          variable_stack_cap;
	U32          next_var_idx;

	HscHashTable(HscStringId, HscDecl) global_declarations;
	HscHashTable(HscStringId, HscDataType) struct_declarations;
	HscHashTable(HscStringId, HscDataType) union_declarations;
	HscHashTable(HscStringId, HscDataType) enum_declarations;

	HscHashTable(HscStringId, U32) field_name_to_token_idx;

	char* error_info;

	HscToken* tokens;
	HscLocation* token_locations;
	HscTokenValue* token_values;
	U32* line_code_start_indices;
	HscStringTable string_table;
	HscConstantTable constant_table;
	HscConstantId false_constant_id;
	HscConstantId true_constant_id;
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

bool hsc_opt_is_enabled(HscOpts* opts, HscOpt opt);
void hsc_opt_set_enabled(HscOpts* opts, HscOpt opt);

HscString hsc_data_type_string(HscAstGen* astgen, HscDataType data_type);
void hsc_data_type_size_align(HscAstGen* astgen, HscDataType data_type, Uptr* size_out, Uptr* align_out);
HscDataType hsc_data_type_resolve_generic(HscAstGen* astgen, HscDataType data_type);
void hsc_data_type_print_basic(HscAstGen* astgen, HscDataType data_type, void* data, FILE* f);
HscDataType hsc_data_type_unsigned_to_signed(HscDataType data_type);
HscDataType hsc_data_type_signed_to_unsigned(HscDataType data_type);

void hsc_string_table_init(HscStringTable* string_table, uint32_t data_cap, uint32_t entries_cap);
#define hsc_string_table_deduplicate_lit(string_table, string_lit) hsc_string_table_deduplicate(string_table, string_lit, sizeof(string_lit) - 1)
HscStringId hsc_string_table_deduplicate(HscStringTable* string_table, char* string, uint32_t string_size);
HscString hsc_string_table_get(HscStringTable* string_table, HscStringId id);

void hsc_constant_table_init(HscConstantTable* constant_table, uint32_t data_cap, uint32_t entries_cap);
HscConstantId hsc_constant_table_deduplicate_basic(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type, void* data);
void hsc_constant_table_deduplicate_composite_start(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type);
void hsc_constant_table_deduplicate_composite_add(HscConstantTable* constant_table, HscConstantId constant_id);
HscConstantId hsc_constant_table_deduplicate_composite_end(HscConstantTable* constant_table);
HscConstantId hsc_constant_table_deduplicate_zero(HscConstantTable* constant_table, HscAstGen* astgen, HscDataType data_type);
HscConstant hsc_constant_table_get(HscConstantTable* constant_table, HscConstantId id);
bool hsc_constant_as_uint(HscConstant constant, U64* out);
bool hsc_constant_as_sint(HscConstant constant, S64* out);

typedef struct HscCompilerSetup HscCompilerSetup;
void hsc_astgen_init(HscAstGen* astgen, HscCompilerSetup* setup);
HSC_NORETURN void hsc_astgen_error_1(HscAstGen* astgen, const char* fmt, ...);
HSC_NORETURN void hsc_astgen_error_2(HscAstGen* astgen, HscLocation* other_location, const char* fmt, ...);
HSC_NORETURN void hsc_astgen_token_error_1(HscAstGen* astgen, const char* fmt, ...);
HSC_NORETURN void hsc_astgen_token_error_2(HscAstGen* astgen, HscLocation* other_location, const char* fmt, ...);
void hsc_astgen_add_token(HscAstGen* astgen, HscToken token);
void hsc_astgen_add_token_value(HscAstGen* astgen, HscTokenValue value);
void hsc_astgen_tokenize(HscAstGen* astgen);
HscToken hsc_token_peek(HscAstGen* astgen);
HscToken hsc_token_peek_ahead(HscAstGen* astgen, U32 by);
void hsc_token_consume(HscAstGen* astgen, U32 amount);
HscToken hsc_token_next(HscAstGen* astgen);
void hsc_token_value_consume(HscAstGen* astgen, U32 amount);
HscTokenValue hsc_token_value_next(HscAstGen* astgen);

HscExpr* hsc_astgen_generate_expr(HscAstGen* astgen, U32 min_precedence);
HscExpr* hsc_astgen_generate_stmt(HscAstGen* astgen);
void hsc_astgen_generate(HscAstGen* astgen);

void hsc_astgen_variable_stack_open(HscAstGen* astgen);
void hsc_astgen_variable_stack_close(HscAstGen* astgen);
U32 hsc_astgen_variable_stack_add(HscAstGen* astgen, HscStringId string_id);
U32 hsc_astgen_variable_stack_find(HscAstGen* astgen, HscStringId string_id);

// ===========================================
//
//
// IR
//
//
// ===========================================

typedef U8 HscIROpCode;
enum {
	HSC_IR_OP_CODE_NO_OP,
	HSC_IR_OP_CODE_LOAD,
	HSC_IR_OP_CODE_STORE,

	HSC_IR_OP_CODE_BINARY_OP_START,
#define HSC_IR_OP_CODE_BINARY_OP(OP) (HSC_IR_OP_CODE_BINARY_OP_START + HSC_BINARY_OP_##OP)
	HSC_IR_OP_CODE_BINARY_OP_END = HSC_IR_OP_CODE_BINARY_OP_START + HSC_BINARY_OP_COUNT,

	HSC_IR_OP_CODE_UNARY_OP_START,
#define HSC_IR_OP_CODE_UNARY_OP(OP) (HSC_IR_OP_CODE_UNARY_OP_START + HSC_UNARY_OP_##OP)
	HSC_IR_OP_CODE_UNARY_OP_END = HSC_IR_OP_CODE_UNARY_OP_START + HSC_UNARY_OP_COUNT,

	HSC_IR_OP_CODE_PHI,

	HSC_IR_OP_CODE_SWITCH,

	HSC_IR_OP_CODE_CONVERT,
	HSC_IR_OP_CODE_BITCAST,

	HSC_IR_OP_CODE_COMPOSITE_INIT,
	HSC_IR_OP_CODE_ACCESS_CHAIN,
	HSC_IR_OP_CODE_FUNCTION_CALL,
	HSC_IR_OP_CODE_FUNCTION_RETURN,
	HSC_IR_OP_CODE_SELECTION_MERGE,
	HSC_IR_OP_CODE_LOOP_MERGE,
	HSC_IR_OP_CODE_BRANCH,
	HSC_IR_OP_CODE_BRANCH_CONDITIONAL,
	HSC_IR_OP_CODE_UNREACHABLE,
};

typedef struct HscIRConst HscIRConst;
struct HscIRConst {
	HscDataType data_type;
	U32 values_offset;
};

typedef struct HscIRValue HscIRValue;
struct HscIRValue {
	HscDataType data_type;
	U16 defined_instruction_idx;
	U16 last_used_instruction_idx;
};

typedef struct HscIRInstr HscIRInstr;
struct HscIRInstr {
	U16 operands_start_idx;
	U8 operands_count;
	U8 op_code;
};

typedef struct HscIRBasicBlock HscIRBasicBlock;
struct HscIRBasicBlock {
	U16 instructions_start_idx;
	U16 instructions_count;
};

//
// inherits HscDataType
typedef U32 HscIROperand;
enum {
	HSC_IR_OPERAND_VALUE = HSC_DATA_TYPE_COUNT,
	HSC_IR_OPERAND_CONSTANT,
	HSC_IR_OPERAND_BASIC_BLOCK,
	HSC_IR_OPERAND_LOCAL_VARIABLE,
};
#define HSC_IR_OPERAND_VALUE_INIT(value_idx) (((value_idx) << 8) | HSC_IR_OPERAND_VALUE)
#define HSC_IR_OPERAND_IS_VALUE(operand) (((operand) & 0xff) == HSC_IR_OPERAND_VALUE)
#define HSC_IR_OPERAND_VALUE_IDX(operand) ((operand) >> 8)

#define HSC_IR_OPERAND_CONSTANT_INIT(constant_id) (((constant_id) << 8) | HSC_IR_OPERAND_CONSTANT)
#define HSC_IR_OPERAND_IS_CONSTANT(operand) (((operand) & 0xff) == HSC_IR_OPERAND_CONSTANT)
#define HSC_IR_OPERAND_CONSTANT_ID(operand) ((HscConstantId) { .idx_plus_one = ((operand) >> 8) })

#define HSC_IR_OPERAND_BASIC_BLOCK_INIT(basic_block_idx) (((basic_block_idx) << 8) | HSC_IR_OPERAND_BASIC_BLOCK)
#define HSC_IR_OPERAND_IS_BASIC_BLOCK(operand) (((operand) & 0xff) == HSC_IR_OPERAND_BASIC_BLOCK)
#define HSC_IR_OPERAND_BASIC_BLOCK_IDX(operand) ((operand) >> 8)

#define HSC_IR_OPERAND_LOCAL_VARIABLE_INIT(local_variable_idx) (((local_variable_idx) << 8) | HSC_IR_OPERAND_LOCAL_VARIABLE)
#define HSC_IR_OPERAND_IS_VARIABLE(operand) (((operand) & 0xff) == HSC_IR_OPERAND_LOCAL_VARIABLE)
#define HSC_IR_OPERAND_LOCAL_VARIABLE_IDX(operand) ((operand) >> 8)

typedef struct HscIRFunction HscIRFunction;
struct HscIRFunction {
	U32 basic_blocks_start_idx;
	U32 instructions_start_idx;
	U32 values_start_idx;
	U32 operands_start_idx;
	U16 basic_blocks_count;
	U16 instructions_count;
	U16 values_count;
	U16 operands_count;
};

typedef struct HscIRBranchState HscIRBranchState;
struct HscIRBranchState {
	bool all_cases_return;
	U32 break_branch_linked_list_head;
	U32 break_branch_linked_list_tail;
	U32 continue_branch_linked_list_head;
	U32 continue_branch_linked_list_tail;
};

typedef struct HscIR HscIR;
struct HscIR {
	HscIRFunction* functions;
	U32 functions_count;
	U32 functions_cap;

	HscIRBasicBlock* basic_blocks;
	U32 basic_blocks_count;
	U32 basic_blocks_cap;

	HscIRValue* values;
	U32 values_count;
	U32 values_cap;

	HscIRInstr* instructions;
	U32 instructions_count;
	U32 instructions_cap;

	HscIROperand* operands;
	U32 operands_count;
	U32 operands_cap;

	HscIROperand last_operand;
	bool do_not_load_variable;
	HscDataType assign_data_type;

	HscIRBranchState branch_state;
};

void hsc_ir_init(HscIR* ir);
void hsc_ir_generate(HscIR* ir, HscAstGen* astgen);

// ===========================================
//
//
// SPIR-V
//
//
// ===========================================

typedef U16 HscSpirvOp;
enum {
	HSC_SPIRV_OP_NO_OP = 0,
	HSC_SPIRV_OP_MEMORY_MODEL = 14,
	HSC_SPIRV_OP_ENTRY_POINT = 15,
	HSC_SPIRV_OP_EXECUTION_MODE = 16,
	HSC_SPIRV_OP_CAPABILITY = 17,
	HSC_SPIRV_OP_TYPE_VOID = 19,
	HSC_SPIRV_OP_TYPE_BOOL = 20,
	HSC_SPIRV_OP_TYPE_INT = 21,
	HSC_SPIRV_OP_TYPE_FLOAT = 22,
	HSC_SPIRV_OP_TYPE_VECTOR = 23,
	HSC_SPIRV_OP_TYPE_ARRAY = 28,
	HSC_SPIRV_OP_TYPE_STRUCT = 30,
	HSC_SPIRV_OP_TYPE_POINTER = 32,
	HSC_SPIRV_OP_TYPE_FUNCTION = 33,

	HSC_SPIRV_OP_CONSTANT_TRUE = 41,
	HSC_SPIRV_OP_CONSTANT_FALSE = 42,
	HSC_SPIRV_OP_CONSTANT = 43,
	HSC_SPIRV_OP_CONSTANT_COMPOSITE = 44,

	HSC_SPIRV_OP_FUNCTION = 54,
	HSC_SPIRV_OP_FUNCTION_PARAMETER = 55,
	HSC_SPIRV_OP_FUNCTION_END = 56,
	HSC_SPIRV_OP_VARIABLE = 59,
	HSC_SPIRV_OP_LOAD = 61,
	HSC_SPIRV_OP_STORE = 62,
	HSC_SPIRV_OP_ACCESS_CHAIN = 65,
	HSC_SPIRV_OP_DECORATE = 71,
	HSC_SPIRV_OP_COMPOSITE_CONSTRUCT = 80,
	HSC_SPIRV_OP_CONVERT_F_TO_U = 109,
	HSC_SPIRV_OP_CONVERT_F_TO_S = 110,
	HSC_SPIRV_OP_CONVERT_S_TO_F = 111,
	HSC_SPIRV_OP_CONVERT_U_TO_F = 112,
	HSC_SPIRV_OP_U_CONVERT = 113,
	HSC_SPIRV_OP_S_CONVERT = 114,
	HSC_SPIRV_OP_F_CONVERT = 115,
	HSC_SPIRV_OP_BITCAST = 124,
	HSC_SPIRV_OP_S_NEGATE = 126,
	HSC_SPIRV_OP_F_NEGATE = 127,
	HSC_SPIRV_OP_I_ADD = 128,
	HSC_SPIRV_OP_F_ADD = 129,
	HSC_SPIRV_OP_I_SUB = 130,
	HSC_SPIRV_OP_F_SUB = 131,
	HSC_SPIRV_OP_I_MUL = 132,
	HSC_SPIRV_OP_F_MUL = 133,
	HSC_SPIRV_OP_U_DIV = 134,
	HSC_SPIRV_OP_S_DIV = 135,
	HSC_SPIRV_OP_F_DIV = 136,
	HSC_SPIRV_OP_U_MOD = 137,
	HSC_SPIRV_OP_S_MOD = 139,
	HSC_SPIRV_OP_F_MOD = 141,
	HSC_SPIRV_OP_LOGICAL_EQUAL = 164,
	HSC_SPIRV_OP_LOGICAL_NOT_EQUAL = 165,
	HSC_SPIRV_OP_LOGICAL_OR = 166,
	HSC_SPIRV_OP_LOGICAL_AND = 167,
	HSC_SPIRV_OP_LOGICAL_NOT = 168,
	HSC_SPIRV_OP_SELECT = 169,
	HSC_SPIRV_OP_I_EQUAL = 170,
	HSC_SPIRV_OP_I_NOT_EQUAL = 171,
	HSC_SPIRV_OP_U_GREATER_THAN = 172,
	HSC_SPIRV_OP_S_GREATER_THAN = 173,
	HSC_SPIRV_OP_U_GREATER_THAN_EQUAL = 174,
	HSC_SPIRV_OP_S_GREATER_THAN_EQUAL = 175,
	HSC_SPIRV_OP_U_LESS_THAN = 176,
	HSC_SPIRV_OP_S_LESS_THAN = 177,
	HSC_SPIRV_OP_U_LESS_THAN_EQUAL = 178,
	HSC_SPIRV_OP_S_LESS_THAN_EQUAL = 179,
	HSC_SPIRV_OP_F_UNORD_EQUAL = 181,
	HSC_SPIRV_OP_F_UNORD_NOT_EQUAL = 183,
	HSC_SPIRV_OP_F_UNORD_LESS_THAN = 185,
	HSC_SPIRV_OP_F_UNORD_GREATER_THAN = 187,
	HSC_SPIRV_OP_F_UNORD_LESS_THAN_EQUAL = 189,
	HSC_SPIRV_OP_F_UNORD_GREATER_THAN_EQUAL = 191,
	HSC_SPIRV_OP_BITWISE_SHIFT_RIGHT_LOGICAL = 194,
	HSC_SPIRV_OP_BITWISE_SHIFT_RIGHT_ARITHMETIC = 195,
	HSC_SPIRV_OP_BITWISE_SHIFT_LEFT_LOGICAL = 196,
	HSC_SPIRV_OP_BITWISE_OR = 197,
	HSC_SPIRV_OP_BITWISE_XOR = 198,
	HSC_SPIRV_OP_BITWISE_AND = 199,
	HSC_SPIRV_OP_BITWISE_NOT = 200,
	HSC_SPIRV_OP_PHI = 245,
	HSC_SPIRV_OP_LOOP_MERGE = 246,
	HSC_SPIRV_OP_SELECTION_MERGE = 247,
	HSC_SPIRV_OP_LABEL = 248,
	HSC_SPIRV_OP_BRANCH = 249,
	HSC_SPIRV_OP_BRANCH_CONDITIONAL = 250,
	HSC_SPIRV_OP_SWITCH = 251,
	HSC_SPIRV_OP_RETURN = 253,
	HSC_SPIRV_OP_RETURN_VALUE = 254,
	HSC_SPIRV_OP_UNREACHABLE = 255,
};

enum {
	HSC_SPIRV_ADDRESS_MODEL_LOGICAL = 0,
};

enum {
	HSC_SPIRV_MEMORY_MODEL_GLSL450 = 1,
	HSC_SPIRV_MEMORY_MODEL_VULKAN = 3,
};

enum {
	HSC_SPIRV_EXECUTION_MODE_ORIGIN_LOWER_LEFT = 8,
};

enum {
	HSC_SPIRV_CAPABILITY_SHADER = 1,
	HSC_SPIRV_CAPABILITY_VULKAN_MEMORY_MODEL = 5345,
};

enum {
	HSC_SPIRV_STORAGE_CLASS_INPUT = 1,
	HSC_SPIRV_STORAGE_CLASS_UNIFORM = 2,
	HSC_SPIRV_STORAGE_CLASS_OUTPUT = 3,
	HSC_SPIRV_STORAGE_CLASS_FUNCTION = 7,
};

enum {
	HSC_SPIRV_EXECUTION_MODEL_VERTEX                  = 0,
	HSC_SPIRV_EXECUTION_MODEL_TESSELLATION_CONTROL    = 1,
	HSC_SPIRV_EXECUTION_MODEL_TESSELLATION_EVALUATION = 2,
	HSC_SPIRV_EXECUTION_MODEL_GEOMETRY                = 3,
	HSC_SPIRV_EXECUTION_MODEL_FRAGMENT                = 4,
	HSC_SPIRV_EXECUTION_MODEL_GL_COMPUTE              = 5,
};

enum {
	HSC_SPIRV_SELECTION_CONTROL_NONE          = 0,
	HSC_SPIRV_SELECTION_CONTROL_FLATTERN      = 1,
	HSC_SPIRV_SELECTION_CONTROL_DONT_FLATTERN = 2,
};

enum {
	HSC_SPIRV_LOOP_CONTROL_NONE = 0,
};

enum {
	HSC_SPRIV_DECORATION_LOCATION = 30,
};

#define HSC_SPIRV_INSTR_OPERANDS_CAP 24

typedef U8 HscSpirvTypeKind;
enum {
	HSC_SPIRV_TYPE_KIND_FUNCTION,
	HSC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE,
	HSC_SPIRV_TYPE_KIND_FUNCTION_VARIABLE_POINTER,
};

typedef struct HscSpirvTypeEntry HscSpirvTypeEntry;
struct HscSpirvTypeEntry {
	U32 data_types_start_idx;
	U32 spirv_id: 22;
	U32 data_types_count: 8;
	U32 kind: 2; // HscSpirvTypeKind
};

typedef struct HscSpirvTypeTable HscSpirvTypeTable;
struct HscSpirvTypeTable {
	HscDataType* data_types;
	HscSpirvTypeEntry* entries;
	U32 data_types_count;
	U32 data_types_cap;
	U32 entries_count;
	U32 entries_cap;
};

typedef struct HscSpirv HscSpirv;
struct HscSpirv {
	HscSpirvTypeTable type_table;

	U32* out_capabilities;
	U32 out_capabilities_count;
	U32 out_capabilities_cap;

	U32* out_entry_points;
	U32 out_entry_points_count;
	U32 out_entry_points_cap;

	U32* out_debug_info;
	U32 out_debug_info_count;
	U32 out_debug_info_cap;

	U32* out_annotations;
	U32 out_annotations_count;
	U32 out_annotations_cap;

	U32* out_types_variables_constants;
	U32 out_types_variables_constants_count;
	U32 out_types_variables_constants_cap;

	U32* out_functions;
	U32 out_functions_count;
	U32 out_functions_cap;

	U32 shader_stage_function_type_spirv_id;

	U32 pointer_type_inputs_base_id;
	U32 pointer_type_outputs_base_id;
	U64 pointer_type_inputs_made_bitset[4];
	U64 pointer_type_outputs_made_bitset[4];
	U32 compound_type_base_id;
	U32 array_type_base_id;

	U32 value_base_id;
	U32 constant_base_id;
	U32 basic_block_base_spirv_id;
	U32 local_variable_base_spirv_id;
	U32 next_id;
	HscSpirvOp instr_op;
	U16 instr_operands_count;
	U32 instr_operands[HSC_SPIRV_INSTR_OPERANDS_CAP];
};

void hsc_spirv_init(HscCompiler* c);
void hsc_spirv_generate(HscCompiler* c);

// ===========================================
//
//
// Compiler
//
//
// ===========================================

struct HscCompiler {
	HscAstGen astgen;
	HscIR ir;
	HscSpirv spirv;
	U16 available_basic_types;
};

struct HscCompilerSetup {
	uint32_t tokens_cap;
	uint32_t lines_cap;
	uint32_t functions_cap;
	uint32_t function_params_and_local_variables_cap;
	uint32_t exprs_cap;
	uint32_t variable_stack_cap;
	uint32_t string_table_data_cap;
	uint32_t string_table_entries_cap;
};

void hsc_compiler_init(HscCompiler* compiler, HscCompilerSetup* setup);

void hsc_compiler_compile(HscCompiler* compiler, const char* file_path);

