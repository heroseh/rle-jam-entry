#ifndef _HERO_CORE_H_
#define _HERO_CORE_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <byteswap.h>
#include <execinfo.h>
#elif _WIN32
#include <Dbghelp.h>
#endif // __linux__

#define HERO_STACKTRACE_LEVELS_MAX 128

#ifdef __x86_64__
#define HERO_ARCH_X86_64 1
#endif // __x86_64__

// ===========================================
//
//
// General
//
//
// ===========================================

#ifndef HERO_STATIC_ASSERT
#define HERO_STATIC_ASSERT(x, msg) int hero_sa(int hero_sa[(x)?1:-1])
#endif

#ifndef HERO_ABORT
#define HERO_ABORT(...) _hero_abort(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HERO_ASSERT
#define HERO_ASSERT(cond, ...) if (HERO_UNLIKELY(!(cond))) _hero_assert_failed(#cond, __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef HERO_DEBUG_ASSERTIONS
#define HERO_DEBUG_ASSERTIONS 0
#endif

#define HERO_ASSERT_ARRAY_BOUNDS(idx, count) HERO_ASSERT((idx) < (count), "idx '%zu' is out of bounds for an array of count '%zu'", (idx), (count));

#define HERO_STRINGIFY(v) #v
#define HERO_CONCAT_0(a, b) a##b
#define HERO_CONCAT(a, b) HERO_CONCAT_0(a, b)

#if HERO_DEBUG_ASSERTIONS
#define HERO_DEBUG_ASSERT HERO_ASSERT
#else
#define HERO_DEBUG_ASSERT(cond, ...) (void)(cond)
#endif

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif

typedef unsigned char U8;
typedef char S8;

typedef unsigned short U16;
typedef short S16;

typedef unsigned int U32;
typedef int S32;
#define HERO_S32_MIN -2147483648
#define HERO_S32_MAX 2147483647

typedef S32 Codept;

typedef unsigned long U64;
typedef long S64;

typedef U64 Uptr;
typedef S64 Sptr;

typedef float F32;
typedef double F64;

typedef U8 B8;
typedef U32 B32;

#ifndef offsetof
#define offsetof(Struct, member) ((size_t)&(((Struct*)0)->member))
#endif

#ifndef alignof
#define alignof(Type) offsetof(struct { char c; Type d; }, d)
#endif

HERO_STATIC_ASSERT(sizeof(U32) == 4, "U32 must be 4 bytes");
HERO_STATIC_ASSERT(sizeof(S32) == 4, "S32 must be 4 bytes");

HERO_STATIC_ASSERT(sizeof(U64) == 8, "U64 must be 8 bytes");
HERO_STATIC_ASSERT(sizeof(S64) == 8, "S64 must be 8 bytes");

HERO_STATIC_ASSERT(sizeof(Uptr) == sizeof(void*), "Uptr must be the same size as a pointer");
HERO_STATIC_ASSERT(sizeof(Sptr) == sizeof(void*), "Sptr must be the same size as a pointer");

#ifdef __GNUC__
#define HERO_LIKELY(expr) __builtin_expect((expr), 1)
#define HERO_UNLIKELY(expr) __builtin_expect((expr), 0)
#else
#define HERO_LIKELY(expr) expr
#define HERO_UNLIKELY(expr) expr
#endif


#ifdef __GNUC__
#define HERO_NORETURN __attribute__((noreturn))
#else
#define HERO_NORETURN
#endif

#ifdef __linux__
#define hero_bswap_16(x) bswap_16(x)
#define hero_bswap_32(x) bswap_32(x)
#define hero_bswap_64(x) bswap_64(x)
#else
#error "unimplemented bswap for this platform"
#endif

// ===========================================
//
//
// General
//
//
// ===========================================

#define HERO_ENGINE_NAME "hero engine"

#define HERO_VERSION_MAJOR 0
#define HERO_VERSION_MINOR 1
#define HERO_VERSION_PATCH 0

#define HERO_VERSION_MAJOR_GET(version) ((version >> HERO_VERSION_MAJOR_SHIFT) & HERO_VERSION_MAJOR_MASK)
#define HERO_VERSION_MINOR_GET(version) ((version >> HERO_VERSION_MINOR_SHIFT) & HERO_VERSION_MINOR_MASK)
#define HERO_VERSION_PATCH_GET(version) ((version >> HERO_VERSION_PATCH_SHIFT) & HERO_VERSION_PATCH_MASK)

#define HERO_VERSION_MAJOR_MASK  0x7ff
#define HERO_VERSION_MAJOR_SHIFT 0
#define HERO_VERSION_MINOR_MASK  0x7ff
#define HERO_VERSION_MINOR_SHIFT 11
#define HERO_VERSION_PATCH_MASK  0x3ff
#define HERO_VERSION_PATCH_SHIFT 22

#define HERO_VERSION ( \
	((HERO_VERSION_MAJOR & HERO_VERSION_MAJOR_MASK) << HERO_VERSION_MAJOR_SHIFT) | \
	((HERO_VERSION_MINOR & HERO_VERSION_MINOR_MASK) << HERO_VERSION_MINOR_SHIFT) | \
	((HERO_VERSION_PATCH & HERO_VERSION_PATCH_MASK) << HERO_VERSION_PATCH_SHIFT) \
)

typedef S32 HeroResult;
enum {
	HERO_SUCCESS,
	HERO_SUCCESS_IS_NEW,
	HERO_SUCCESS_FINISHED,


	HERO_SUCCESS_WINDOW_START = 1000,
	HERO_SUCCESS_GFX_START = 2000,

	HERO_SUCCESS_COUNT,
};

extern char* hero_stacktrace_debug;
#define HERO_ERROR(NAME) ((hero_stacktrace_debug = hero_stacktrace(1, hero_system_alctor, NULL)), (-HERO_ERROR_##NAME))
#define HERO_ERROR_DEF(NAME) HERO_ERROR_##NAME

enum {
	HERO_ERROR_GENERAL = 1,
	HERO_ERROR_ALLOCATION_FAILURE,
	HERO_ERROR_OBJECT_ID_IS_NULL,
	HERO_ERROR_USE_AFTER_FREE,
	HERO_ERROR_DOES_NOT_EXIST,
	HERO_ERROR_ALREADY_EXISTS,
	HERO_ERROR_ENDED_BY_USER,
	HERO_ERROR_OPENING_DLL,
	HERO_ERROR_UNHANDLED_ENUM,
	HERO_ERROR_NOT_STARTED,
	HERO_ERROR_ALREADY_STARTED,
	HERO_ERROR_FULL,

	HERO_ERROR_WINDOW_START = 1000,
	HERO_ERROR_GFX_START = 2000,
	HERO_ERROR_UI_START = 3000,

	HERO_ERROR_COUNT,
};

const char* hero_result_string(HeroResult result);
HeroResult hero_result_from_errno(int errnum);

#define HERO_RESULT_ASSERT(result) HERO_ASSERT((result) >= 0, "unexpected error: %s\nstacktrace:\n%s", hero_result_string(result), hero_stacktrace_debug);

void _hero_assert_failed(const char* cond, const char* file, int line, const char* message, ...);
HERO_NORETURN void _hero_abort(const char* file, int line, const char* message, ...);

#define for_range(i, start, end) for (Uptr i = start; i < end; i += 1)
#define for_range_s(i, start, end) for (Sptr i = start; i < end; i += 1)

// ===========================================
//
//
// Number Utilities
//
//
// ===========================================

// align must be a power of 2
#define HERO_INT_ROUND_UP_ALIGN(i, align) (((i) + ((align) - 1)) & ~((align) - 1))
// align must be a power of 2
#define HERO_INT_ROUND_DOWN_ALIGN(i, align) ((i) & ~((align) - 1))

#define HERO_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HERO_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define HERO_CLAMP(v, min, max) (((v) > (max)) ? (max) : ((v) < (min)) ? (min) : (v))
#define HERO_EPSILON 0.0001

static inline F32 hero_lerp(F32 from, F32 to, F32 t) { return (to - from) * t + from; }
static inline F32 hero_lerp_inv(F32 from, F32 to, F32 value) { return (value - from) / (to - from); }
static inline F32 hero_bilerp(F32 tl, F32 tr, F32 bl, F32 br, F32 tx, F32 ty){
	return hero_lerp(hero_lerp(tl, tr, tx), hero_lerp(bl, br, tx), ty);
}
static inline bool hero_approx_eq(F32 a, F32 b) { return fabs(a - b) <= HERO_EPSILON; }
static inline F32 hero_sign(F32 v) { return copysignf(1.f, v); }

#define ENUM_FLAG(value) (1 << (value))

#define HERO_BITSET_CONTAINS_ALL(bitset, flags) (((bitset) & (flags)) == (bitset))
#define HERO_BITSET_CONTAINS_ANY(bitset, flags) (((bitset) & (flags)) != 0)

#if HERO_ARCH_X86_64 // TODO deal with different sizes on other arch / platform in an elegant way
#define HERO_BITSET_COUNT_U32(bitset) __builtin_popcount(bitset)
#define HERO_BITSET_COUNT_U64(bitset) __builtin_popcountl(bitset)
#define HERO_LEAST_SET_BIT_IDX_U32(bitset) __builtin_clz(bitset)
#define HERO_LEAST_SET_BIT_IDX_U64(bitset) __builtin_clzl(bitset)
#define HERO_LEAST_SET_BIT_REMOVE(bitset) ((bitset) & ((bitset) - 1))
#endif // HERO_ARCH_X86_64

// ===========================================
//
//
// Memory Utilities
//
//
// ===========================================

#define HERO_ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))
#define HERO_IS_POWER_OF_TWO(v) (((v) != 0) && (((v) & ((v) - 1)) == 0))
#define HERO_PTR_ADD(ptr, by) (void*)((Uptr)(ptr) + (Uptr)(by))
#define HERO_PTR_SUB(ptr, by) (void*)((Uptr)(ptr) - (Uptr)(by))
#define HERO_PTR_DIFF(to, from) ((char*)(to) - (char*)(from))
// align must be a power of 2
#define HERO_PTR_ROUND_UP_ALIGN(ptr, align) ((void*)HERO_INT_ROUND_UP_ALIGN((Uptr)ptr, align))
// align must be a power of 2
#define HERO_PTR_ROUND_DOWN_ALIGN(ptr, align) ((void*)HERO_INT_ROUND_DOWN_ALIGN((Uptr)ptr, align))
#define HERO_ZERO_ELMT(ptr) memset(ptr, 0, sizeof(*(ptr)))
#define HERO_ZERO_ELMT_MANY(ptr, elmts_count) memset(ptr, 0, sizeof(*(ptr)) * (elmts_count))
#define HERO_ZERO_ARRAY(array) memset(array, 0, sizeof(array))
#define HERO_COPY_ARRAY(dst, src) memcpy(dst, src, sizeof(dst))
#define HERO_COPY_ELMT_MANY(dst, src, elmts_count) memcpy(dst, src, elmts_count * sizeof(*(dst)))
#define HERO_COPY_OVERLAP_ELMT_MANY(dst, src, elmts_count) memmove(dst, src, elmts_count * sizeof(*(dst)))
#define HERO_CMP_ARRAY(a, b) (memcmp(a, b, sizeof(a)) == 0)
#define HERO_CMP_ELMT(a, b) (memcmp(a, b, sizeof(*(a))) == 0)
#define HERO_CMP_ELMT_MANY(a, b, elmts_count) (memcmp(a, b, elmts_count * sizeof(*(a))) == 0)

// TODO: on other architectures this could be different, like the Apple M1
#define HERO_CACHE_LINE_SIZE 64
#define HERO_CACHE_LINE_ALIGN 64

typedef U64 HeroAllocTag;
enum {
	HERO_ALLOC_TAG_TODO,
	HERO_ALLOC_TAG_FILE_SAFE_FULL_WRITE_PATH,
	HERO_ALLOC_TAG_STACKTRACE,

	HERO_ALLOC_TAG_WINDOW_START = 1000,
	HERO_ALLOC_TAG_GFX_START = 2000,

};

typedef void* (*HeroIAlctorAllocFn)(void* alctor_data, HeroAllocTag tag, Uptr size, Uptr align);
typedef void* (*HeroIAlctorRellocFn)(void* alctor_data, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr new_size, Uptr align);
typedef void (*HeroIAlctorDellocFn)(void* alctor_data, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr align);

typedef struct HeroIAlctorVTable HeroIAlctorVTable;
struct HeroIAlctorVTable {
	HeroIAlctorAllocFn alloc_fn;
	HeroIAlctorRellocFn realloc_fn;
	HeroIAlctorDellocFn dealloc_fn;
};

typedef struct HeroIAlctor HeroIAlctor;
struct HeroIAlctor {
	void* alctor_data;
	HeroIAlctorVTable* vtable;
};

#define HERO_ALLOC_TAG(major, minor) (((HeroAllocTag)(minor) << 32) | (HeroAllocTag)major)
#define HERO_ALLOC_TAG_MAJOR(tag) ((U32)((tag) & 0xffffffff))
#define HERO_ALLOC_TAG_MINOR(tag) ((U32)((tag) >> 32))

static inline void* hero_alloc(HeroIAlctor alctor, HeroAllocTag tag, Uptr size, Uptr align) {
	return alctor.vtable->alloc_fn(alctor.alctor_data, tag, size, align);
}
static inline void* hero_realloc(HeroIAlctor alctor, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr new_size, Uptr align) {
	return alctor.vtable->realloc_fn(alctor.alctor_data, tag, allocated_ptr, size, new_size, align);
}
static inline void hero_dealloc(HeroIAlctor alctor, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr align) {
	alctor.vtable->dealloc_fn(alctor.alctor_data, tag, allocated_ptr, size, align);
}

#define hero_alloc_array(T, alctor, tag, count) \
	(T*)hero_alloc(alctor, tag, sizeof(T) * (count), alignof(T))

#define hero_realloc_array(T, alctor, tag, allocated_ptr, count, new_count) \
	(T*)hero_realloc(alctor, tag, allocated_ptr, sizeof(T) * (count), sizeof(T) * (new_count), alignof(T))

#define hero_dealloc_array(T, alctor, tag, allocated_ptr, count) \
	hero_dealloc(alctor, tag, allocated_ptr, sizeof(T) * (count), alignof(T))

#define hero_alloc_elmt(T, alctor, tag) \
	(T*)hero_alloc(alctor, tag, sizeof(T), alignof(T))

#define hero_dealloc_elmt(T, alctor, tag, allocated_ptr) \
	hero_dealloc(alctor, tag, allocated_ptr, sizeof(T), alignof(T))

void* hero_system_alloc(void* alctor_data, HeroAllocTag tag, Uptr size, Uptr align);
void* hero_system_realloc(void* alctor_data, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr new_size, Uptr align);
void hero_system_dealloc(void* alctor_data, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr align);

static HeroIAlctorVTable hero_system_alctor_vtable = {
	.alloc_fn = hero_system_alloc,
	.realloc_fn = hero_system_realloc,
	.dealloc_fn = hero_system_dealloc,
};

static HeroIAlctor hero_system_alctor = { .alctor_data = NULL, .vtable = &hero_system_alctor_vtable };

// ==========================================
//
//
// Stacktrace
//
//
// ==========================================

char* hero_stacktrace(U32 ignore_levels_count, HeroIAlctor alctor, U32* size_out);

// ===========================================
//
//
// Stack
//
//
// ===========================================

typedef struct HeroStack HeroStack;
struct HeroStack {
	void* data;
	Uptr count;
	Uptr cap;
};

#define HeroStack(ElmtName) HeroStack_##ElmtName
#define hero_stack(ElmtName, NAME) hero_stack_##NAME##_##ElmtName

static inline void _hero_stack_deinit(HeroStack* stack, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align) {
	hero_dealloc(alctor, tag, stack->data, stack->cap * elmt_size, elmt_align);
}

static inline void* _hero_stack_get(HeroStack* stack, Uptr idx, Uptr elmt_size) {
	if (idx >= stack->count) return NULL;
	return HERO_PTR_ADD(stack->data, idx * elmt_size);
}

static inline HeroResult _hero_stack_resize_cap(HeroStack* stack, Uptr new_cap, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align) {
	void* data = hero_realloc(alctor, tag, stack->data, stack->cap * elmt_size, new_cap * elmt_size, elmt_align);
	if (data == NULL) return HERO_ERROR(ALLOCATION_FAILURE);
	stack->data = data;
	stack->cap = new_cap;
	return HERO_SUCCESS;
}

HeroResult _hero_stack_push_many(HeroStack* stack, Uptr amount, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align, void** elmts_ptr_out);
static inline HeroResult _hero_stack_push(HeroStack* stack, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align, void** elmt_ptr_out) {
	return _hero_stack_push_many(stack, 1, alctor, tag, elmt_size, elmt_align, elmt_ptr_out);
}

HeroResult _hero_stack_insert_many(HeroStack* stack, Uptr idx, Uptr amount, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align, void** elmts_ptr_out);
static inline HeroResult _hero_stack_insert(HeroStack* stack, Uptr idx, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align, void** elmt_ptr_out) {
	return _hero_stack_insert_many(stack, idx, 1, alctor, tag, elmt_size, elmt_align, elmt_ptr_out);
}

HeroResult _hero_stack_remove_shift_range(HeroStack* stack, Uptr start_idx, Uptr end_idx, Uptr elmt_size);
static inline HeroResult _hero_stack_remove_shift(HeroStack* stack, Uptr idx, Uptr elmt_size) {
	return _hero_stack_remove_shift_range(stack, idx, idx + 1, elmt_size);
}

static inline HeroResult _hero_stack_pop(HeroStack* stack) {
	if (stack->count == 0) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	stack->count -= 1;
	return HERO_SUCCESS;
}

#define HERO_STACK_ELMT_TYPE U8
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE U16
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE U32
#include "stack_gen.inl"

#define HERO_STACK_ELMT_TYPE Uptr
#include "stack_gen.inl"

// ===========================================
//
//
// String
//
//
// ===========================================

typedef struct HeroString HeroString;
struct HeroString {
	U8* data;
	Uptr size;
};

#define hero_string(data, size) ((HeroString) { (data), (size) })
#define hero_string_lit(string) ((HeroString) { ((U8*)string), sizeof(string) - 1 })
#define hero_string_c(string) ((HeroString) { ((U8*)string), strlen(string) })

static inline HeroString hero_string_from_c(char* string) {
	return ((HeroString) { (U8*)string, strlen(string) });
}

HeroResult hero_string_fmt(HeroString* string, HeroIAlctor alctor, HeroAllocTag tag, char* fmt, ...);
HeroResult hero_string_fmtv(HeroString* string, HeroIAlctor alctor, HeroAllocTag tag, char* fmt, va_list args);

static inline HeroString hero_string_slice(HeroString string, Uptr start, Uptr end) {
	start = HERO_MIN(start, string.size);
	end = HERO_MIN(end, string.size);
	string.data = (U8*)HERO_PTR_ADD(string.data, start);
	string.size = end - start;
	return string;
}

static inline HeroString hero_string_slice_start(HeroString string, Uptr start) {
	start = HERO_MIN(start, string.size);
	string.data = (U8*)HERO_PTR_ADD(string.data, start);
	string.size -= start;
	return string;
}

static inline HeroString hero_string_slice_end(HeroString string, Uptr end) {
	end = HERO_MIN(end, string.size);
	string.size = end;
	return string;
}

// ===========================================
//
//
// Hashing
//
//
// ===========================================

#define HERO_HASH_SIZE 32

#if HERO_HASH_SIZE == 64
typedef U64 HeroHash;
#define HERO_HASH_TABLE_BUCKET_ENTRIES_COUNT 8
#define HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT 3
#define HERO_HASH_INIT 0xcbf29ce484222325
#elif HERO_HASH_SIZE == 32
typedef U32 HeroHash;
#define HERO_HASH_TABLE_BUCKET_ENTRIES_COUNT 16
#define HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT 4
#define HERO_HASH_INIT 0x811c9dc5
#else
#error "unimplemented for hash size"
#endif

typedef HeroHash (*HeroHashFn)(void* data, HeroHash hash);
typedef bool (*HeroKeyCmpFn)(void* a, void* b);

HeroHash hero_hash(const void* data, Uptr size, HeroHash hash);

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

typedef struct HeroHashTable HeroHashTable;
struct HeroHashTable {
	HeroHash* hashes;
	void* entries;
	Uptr count;
	Uptr cap;
};
#define HERO_HASH_TABLE_INIT_CAP 64

#define HeroHashTable(Key, Value) HeroHashTable_##Key##_##Value
#define HeroHashTableEntry(Key, Value) HeroHashTableEntry_##Key##_##Value
#define hero_hash_table(Key, Value, NAME) hero_hash_table_##NAME##_##Key##_##Value

enum {
	HERO_HASH_TABLE_HASH_EMPTY = 0,
	HERO_HASH_TABLE_HASH_TOMBSTONE = 1,
	HERO_HASH_TABLE_HASH_START = 2
};

HeroResult _hero_hash_table_init(HeroHashTable* hash_table, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align);
void _hero_hash_table_deinit(HeroHashTable* hash_table, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align);
HeroResult _hero_hash_table_find(HeroHashTable* hash_table, const void* key, Uptr key_size, Uptr entry_size, void** entry_ptr_out);
HeroResult _hero_hash_table_find_or_insert(HeroHashTable* hash_table, const void* key, Uptr key_size, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align, void** entry_ptr_out);
HeroResult _hero_hash_table_remove(HeroHashTable* hash_table, const void* key, Uptr key_size, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align);
HeroResult _hero_hash_table_iter_next(HeroHashTable* hash_table, Uptr entry_size, Uptr* next_entry_idx_mut, void** entry_ptr_out);
HeroResult _hero_hash_table_clear(HeroHashTable* hash_table, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align);
bool hero_cmp(const void* a, const void* b, Uptr size);

// ===========================================
//
//
// Object Pool
//
//
// ===========================================

typedef struct HeroObjectPool HeroObjectPool;
struct HeroObjectPool {
	void* data;
	U32 count;
	U32 cap;
	U32 free_list_head_idx;
	U32 allocated_list_head_idx;
	U32 allocated_list_tail_idx;
};

#define HeroObjectPool(Type) HeroObjectPool_##Type
#define hero_object_pool(Type, NAME) hero_object_pool_##NAME##_##Type

#define HERO_TYPEDEF_OBJECT_ID(Name) \
	typedef struct Name Name; \
	struct Name { HeroObjectIdRaw raw; }

typedef U32 HeroObjectIdRaw;

typedef struct HeroObjectHeader HeroObjectHeader;
struct HeroObjectHeader {
	HeroObjectIdRaw next_id;
	U32 prev_idx;
};

#define HERO_OBJECT_ID_IS_NULL(id) (id.raw == 0)

#define HERO_OBJECT_ID_IDX_MASK(IDX_BITS_COUNT) ((1 << IDX_BITS_COUNT) - 1)
#define HERO_OBJECT_ID_IDX_SHIFT 0
#define HERO_OBJECT_ID_COUNTER_MASK(IDX_BITS_COUNT) ((1 << ((32 - IDX_BITS_COUNT) - 1)) - 1)
#define HERO_OBJECT_ID_COUNTER_SHIFT(IDX_BITS_COUNT) IDX_BITS_COUNT

static inline bool _hero_object_id_raw_init(U32 idx, U32 counter, U32 idx_bits_count) {
	return
		((idx & HERO_OBJECT_ID_IDX_MASK(idx_bits_count)) << HERO_OBJECT_ID_IDX_SHIFT) |
		((counter & HERO_OBJECT_ID_COUNTER_MASK(idx_bits_count)) << HERO_OBJECT_ID_COUNTER_SHIFT(idx_bits_count))
		;
}

static inline U32 _hero_object_id_raw_idx(HeroObjectIdRaw raw_id, U32 idx_bits_count) {
	return (raw_id >> HERO_OBJECT_ID_IDX_SHIFT) & HERO_OBJECT_ID_IDX_MASK(idx_bits_count);
}

static inline void _hero_object_id_raw_set_idx(HeroObjectIdRaw* raw_id, U32 idx, U32 idx_bits_count) {
	*raw_id &= ~(HERO_OBJECT_ID_IDX_MASK(idx_bits_count) << HERO_OBJECT_ID_IDX_SHIFT); // remove the old index
	*raw_id |= (idx & HERO_OBJECT_ID_IDX_MASK(idx_bits_count)) << HERO_OBJECT_ID_IDX_SHIFT; // set the new index
}

static inline U32 _hero_object_id_raw_counter(HeroObjectIdRaw raw_id, U32 idx_bits_count) {
	return (raw_id >> HERO_OBJECT_ID_COUNTER_SHIFT(idx_bits_count)) & HERO_OBJECT_ID_COUNTER_MASK(idx_bits_count);
}

HeroResult _hero_object_pool_init(HeroObjectPool* object_pool, U32 cap, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align);
void _hero_object_pool_deinit(HeroObjectPool* object_pool, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align);
HeroResult _hero_object_pool_alloc(HeroObjectPool* object_pool, Uptr elmt_size, U32 idx_bits_count, void** ptr_out, HeroObjectIdRaw* raw_id_out);
HeroResult _hero_object_pool_dealloc(HeroObjectPool* object_pool, HeroObjectIdRaw raw_id, Uptr elmt_size, U32 idx_bits_count);
HeroResult _hero_object_pool_get(HeroObjectPool* object_pool, HeroObjectIdRaw raw_id, Uptr elmt_size, U32 idx_bits_count, void** ptr_out);
HeroResult _hero_object_pool_get_id(HeroObjectPool* object_pool, void* object, Uptr elmt_size, U32 idx_bits_count, HeroObjectIdRaw* raw_id_out);
void _hero_object_pool_clear(HeroObjectPool* object_pool);
HeroResult _hero_object_pool_iter_next(HeroObjectPool* object_pool, HeroObjectIdRaw* raw_id_mut, Uptr elmt_size, U32 idx_bits_count, void** ptr_out);
HeroResult _hero_object_pool_iter_prev(HeroObjectPool* object_pool, HeroObjectIdRaw* raw_id_mut, Uptr elmt_size, U32 idx_bits_count, void** ptr_out);

// ===========================================
//
//
// Virtual Memory Abstraction
//
//
// ===========================================

typedef U8 HeroVirtMemProtection;
enum {
	HERO_VIRT_MEM_PROTECTION_NO_ACCESS,
	HERO_VIRT_MEM_PROTECTION_READ,
	HERO_VIRT_MEM_PROTECTION_READ_WRITE,
	HERO_VIRT_MEM_PROTECTION_EXEC_READ,
	HERO_VIRT_MEM_PROTECTION_EXEC_READ_WRITE,
};

static Uptr hero_virt_mem_page_size;
static Uptr hero_virt_mem_reserve_align;

//
// @param(page_size_out):
//     the page size of the OS.
//     used to align the parameters of the virtual memory functions to a page.
//
// @param(reserve_align_out):
//     a pointer to recieve the alignment of the virtual memory reserve address.
//     this is used as the alignment the requested_addr parameter of hero_virt_mem_reserve.
//     this is guaranteed to the be the same as page size or a multiple of it.
//     On Unix: this is just the page_size
//     On Windows: this is what known as the page granularity.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_virt_mem_page_size_get(Uptr* page_size_out, Uptr* reserve_align_out);

//
// reserve a range of the virtual address space but does not commit any physical pages of memory.
// none of this memory cannot be used until hero_virt_mem_commit is called.
//
// WARNING: on Windows, you cannot release sub sections of the address space.
//          you can only release the full reserved address space that is issued by this function call.
//          there are also restriction on protection, see hero_virt_mem_protection_set.
//
// @param(requested_addr): the requested start address you wish to reserve.
//     be a aligned to the reserve_align that is retrieved from hero_virt_mem_page_size function.
//     this is not guaranteed and is only used as hint.
//     NULL will not be used as a hint, instead the OS will choose an address for you.
//
// @param(size): the size in bytes you wish to reserve from the @param(requested_addr)
//     must be a multiple of the reserve_align that is retrieved from hero_virt_mem_page_size function.
//
// @param(addr_out) a pointer to a value that is set to the start of the reserved block of memory
//     when this function returns successfully.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_virt_mem_reserve(void* requested_addr, Uptr size, void** addr_out);

//
// requests the OS to commit physical pages of memory to the the address space.
// this address space must be a full or subsection of the reserved address space with hero_virt_mem_reserve.
// the memory in the commited address space will be zeroed after calling this function.
//
// @param(addr): the start address of the memory you wish to commit.
//     must be a aligned to the page size hero_virt_mem_page_size returns.
//     this is not guaranteed and is only used as hint.
//     NULL will not be used as a hint.
//
// @param(size): the size in bytes you wish to reserve from the @param(addr)
//     must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @param(protection): what the memory is allowed to be used for
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_virt_mem_commit(void* addr, Uptr size, HeroVirtMemProtection protection);

//
// change the protection of a range of memory.
// this memory must have been reserved with hero_virt_mem_reserve.
//
// WARNING: on Windows, you can change protection of any number pages
//          but that they all must come from the same call they where reserved with.
//
// @param(addr): the start of the pages you wish to change the protection for.
//             must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @param(size): the size in bytes of the memory you wish to change protection for.
//             must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @param(protection): what the memory is allowed to be used for
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_virt_mem_protection_set(void* addr, Uptr size, HeroVirtMemProtection protection);

//
// gives the memory back to the OS but will keep the address space reserved
//
// @param(addr): the start of the pages you wish to decommit.
//             must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @param(size): the size in bytes of the memory you wish to release.
//             must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_virt_mem_decommit(void* addr, Uptr size);

//
// gives the reserved pages back to the OS. the address range must have be reserved with hero_virt_mem_reserve.
// all commit pages in the released address space are automatically decommit when you release.
//
// on non Windows systems only:
//     you can target sub pages of the original allocation but just make sure the parameters are aligned.
//
// WARNING: on Windows, you cannot release sub sections of the address space.
//          you can only release the full reserved address space that is issued by hero_virt_mem_reserve.
//          so @param(size) is ignored on Windows.
//
// @param(addr): the start of the pages you wish to release.
//             must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @param(size): the size in bytes of the memory you wish to release.
//             must be a aligned to the page size hero_virt_mem_page_size returns.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_virt_mem_release(void* addr, Uptr size);

// ===========================================
//
//
// File Utilities
//
//
// ===========================================

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define HERO_PATH_SEPARATOR '/'
#elif _WIN32
#define HERO_PATH_SEPARATOR '\\'
#else
#error "unimplemented file API for this platform"
#endif

typedef U8 HeroFileFlags;
enum {
	// specifies that the open file can be read from.
	HERO_FILE_FLAGS_READ = 0x1,
	// specifies that the open file can be written to.
	// starts the cursor at the start of the file if append is not set.
	HERO_FILE_FLAGS_WRITE = 0x2,
	// specifies that the open file can be written to.
	// the cursor at the end of the file.
	HERO_FILE_FLAGS_APPEND = 0x4,
	// truncate an existing file by removing it's contents are starting from 0 bytes.
	// must be opened with write or append.
	HERO_FILE_FLAGS_TRUNCATE = 0x8,
	// creates a new file if it does not exist.
	// must be opened with write or append.
	HERO_FILE_FLAGS_CREATE_IF_NOT_EXIST = 0x10,
	// only creates a new file and errors if the file exists.
	// enabling this will ignore create_if_not_exist and truncate.
	HERO_FILE_FLAGS_CREATE_NEW = 0x20,
};

typedef struct HeroFileHandle HeroFileHandle;
struct HeroFileHandle {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int raw; // native Unix File Descriptor
#elif _WIN32
	HANDLE raw; // native Windows File Handle
#else
#error "unimplemented file API for this platform"
#endif
};

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#define HERO_FILE_HANDLE_NULL (HeroFileHandle) { .raw = -1 }
#elif _WIN32
#define HERO_FILE_HANDLE_NULL (HeroFileHandle) { .raw = INVALID_HANDLE }
#else
#error "unimplemented file API for this platform"
#endif

//
// opens a file to be use by the other file functions.
//
// @param(path): a path to the file you wish to open
//
// @param(flags): flags to choose how you want the file to be opened.
//     see HeroFileFlags for more information.
//
// @param(file_handle_out): a pointer to the file handle that is set
//     when this function returns successfully.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_open(const char* path, HeroFileFlags flags, HeroFileHandle* file_handle_out);

//
// closes a file that was opened with hero_file_open.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_close(HeroFileHandle handle);

//
// gets the size of the file in bytes
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @param(size_out): a pointer to a value that is set to size of the file when this function returns successfully.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_size(HeroFileHandle handle, U64* size_out);

//
// attempts to read bytes from a file at it's current cursor in one go.
// the cursor is incremented by the number of bytes read.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @param(data_out): a pointer to where the data is read to.
//
// @param(size): the number of bytes you wish to try and read into @param(data_out) in one go.
//
// @param(bytes_read_out): a pointer to a value that is set to the number of bytes read when this function returns successfully.
//     on success: if *bytes_read_out == 0 then the end of the file has been reached and no bytes where read
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_read(HeroFileHandle handle, void* data_out, Uptr size, Uptr* bytes_read_out);

//
// attempts to read bytes from a file at it's current cursor until the supplied output buffer is filled up.
// the cursor is incremented by the number of bytes read.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @param(data_out): a pointer to where the data is read to.
//
// @param(size): the number of bytes you wish to try and read into @param(data_out)
//
// @param(bytes_read_out): a pointer to a value that is set to the number of bytes read when this function returns successfully.
//     on success: if *bytes_read_out == 0 then the end of the file has been reached and no bytes where read
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_read_exact(HeroFileHandle handle, void* data_out, Uptr size, Uptr* bytes_read_out);

//
// attempts to write bytes to a file at it's current cursor in one go.
// the cursor is incremented by the number of bytes written.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @param(data): a pointer to where the data is written to.
//
// @param(size): the number of bytes you wish to try and write into @param(data) in one go.
//
// @param(bytes_written_out): a pointer to a value that is set to the number of bytes written when this function returns successfully.
//     on success: if *bytes_written_out == 0 then the end of the file has been reached and no bytes where written
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_write(HeroFileHandle handle, void* data, Uptr size, Uptr* bytes_written_out);

//
// attempts to write bytes from a file at it's current cursor until the supplied output buffer is filled up.
// the cursor is incremented by the number of bytes written.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @param(data): a pointer to where the data is written to.
//
// @param(size): the number of bytes you wish to try and write into @param(data)
//
// @param(bytes_written_out): a pointer to a value that is set to the number of bytes written when this function returns successfully.
//     on success: if *bytes_written_out == 0 then the end of the file has been reached and no bytes where written
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_write_exact(HeroFileHandle handle, void* data, Uptr size, Uptr* bytes_written_out);

typedef U8 HeroFileSeekFrom;
enum {
	// the file cursor offset will be set to @param(offset) in bytes
	HERO_FILE_SEEK_FROM_START,
	// the file cursor offset will be set to the file's current cursor offset + @param(offset) in bytes
	HERO_FILE_SEEK_FROM_CURRENT,
	// the file cursor offset will be set to the file's size + @param(offset) in bytes
	HERO_FILE_SEEK_FROM_END
};

//
// attempts to move the file's cursor to a different location in the file to read and write from.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @param(offset): offset from where the @param(from) is targeting. see HeroFileSeekFrom for more information
//
// @param(from): a target to seek from in the file. see HeroFileSeekFrom for more information
//
// @param(cursor_offset_out): a pointer to a value that is set to the file's cursor offset when this function returns successfully.
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_seek(HeroFileHandle handle, S64 offset, HeroFileSeekFrom from, U64* cursor_offset_out);

//
// flush any queued data in the OS for the file to the storage device.
//
// @param(handle): the file handle created with successful call to hero_file_open
//
// @return: 0 on success, otherwise a error code to indicate the error.
//
HeroResult hero_file_flush(HeroFileHandle handle);

HeroResult hero_file_rename(const char* dst_path, const char* src_path);

HeroResult hero_file_safe_full_write_start(HeroFileHandle* handle_mut, const char* path, char** tmp_path_out);
HeroResult hero_file_safe_full_write_end(HeroFileHandle* handle_mut, const char* path, char* tmp_path);

HeroResult hero_file_path_canonicalize(const char* path, HeroAllocTag tag, HeroString* path_out);

HeroResult hero_dir_make(const char* path);

HeroResult hero_file_read_all(const char* path, HeroIAlctor alctor, HeroAllocTag tag, U8** bytes_out, Uptr* file_size_out);

// ===========================================
//
//
// Free Range
//
//
// ===========================================

typedef struct HeroRangeU8 HeroRangeU8;
struct HeroRangeU8 {
	U8 start_idx;
	U8 end_idx;
};

typedef struct HeroRangeU16 HeroRangeU16;
struct HeroRangeU16 {
	U16 start_idx;
	U16 end_idx;
};

typedef struct HeroRangeU32 HeroRangeU32;
struct HeroRangeU32 {
	U32 start_idx;
	U32 end_idx;
};

typedef struct HeroRange HeroRange;
struct HeroRange {
	Uptr start_idx;
	Uptr end_idx;
};

#define HERO_STACK_ELMT_TYPE HeroRange
#include "stack_gen.inl"

typedef struct HeroFreeRanges HeroFreeRanges;
struct HeroFreeRanges {
	// ranges are ordered
	HeroStack(HeroRange) ranges;
};

HeroResult hero_free_ranges_init(HeroFreeRanges* r, Uptr init_cap, HeroIAlctor alctor, HeroAllocTag tag);
void hero_free_ranges_deinit(HeroFreeRanges* r, HeroIAlctor alctor, HeroAllocTag tag);
HeroResult hero_free_ranges_give(HeroFreeRanges* r, Uptr idx, HeroIAlctor alctor, HeroAllocTag tag);
HeroResult hero_free_ranges_take(HeroFreeRanges* r, Uptr* idx_out);

// ===========================================
//
//
// Time Utilities
//
//
// ===========================================

typedef struct HeroDuration HeroDuration;
struct HeroDuration {
	U64 secs;
	U32 nanosecs;
};

typedef U8 HeroTimeMode;
enum {
	// will follow the realtime system time, is effected by system time changes
	// use this if you want the system time.
	HERO_TIME_MODE_REALTIME,
	// some arbitrary time that is used as a reference point and not effected by system time.
	// use this to measure a duration between two times.
	HERO_TIME_MODE_MONOTONIC,
};

typedef struct HeroTime HeroTime;
struct HeroTime {
	U64 secs;
	U32 nanosecs;
};

HeroTime hero_time_now(HeroTimeMode mode);
HeroDuration hero_time_elapsed(HeroTime time, HeroTimeMode mode);
HeroDuration hero_time_diff(HeroTime to, HeroTime from);

#define hero_duration_init(secs_, nanosecs_) (HeroDuration) { .secs = secs_, .nanosecs = nanosecs_ }
#define hero_duration_from_millisecs(millisecs) (HeroDuration) { .secs = (millisecs) / 1000, .nanosecs = ((millisecs) % 1000) * 1000000 }
#define hero_duration_from_microsecs(microsecs) (HeroDuration) { .secs = (microsecs) / 1000000, .nanosecs = ((microsecs) % 1000000) * 1000 }
#define hero_duration_from_nanosecs(nanosecs_) (HeroDuration) { .secs = (nanosecs_) / 1000000000, .nanosecs = (nanosecs_) % 1000000000 }

bool hero_duration_has_secs(HeroDuration duration);
bool hero_duration_has_millisecs(HeroDuration duration);
bool hero_duration_has_microsecs(HeroDuration duration);
bool hero_duration_has_nanosecs(HeroDuration duration);
F32 hero_duration_to_f32_secs(HeroDuration duration);
F64 hero_duration_to_f64_secs(HeroDuration duration);
F32 hero_duration_to_f32_millisecs(HeroDuration duration);
F64 hero_duration_to_f64_millisecs(HeroDuration duration);
F32 hero_duration_to_f32_microsecs(HeroDuration duration);
F64 hero_duration_to_f64_microsecs(HeroDuration duration);
U64 hero_duration_secs(HeroDuration duration);
U64 hero_duration_millisecs(HeroDuration duration);
U64 hero_duration_microsecs(HeroDuration duration);
U64 hero_duration_nanosecs(HeroDuration duration);
U64 hero_duration_frame_to_fps(HeroDuration duration);
HeroDuration hero_duration_add(HeroDuration a, HeroDuration b);
HeroDuration hero_duration_sub(HeroDuration a, HeroDuration b);
bool hero_duration_lt(HeroDuration a, HeroDuration b);
bool hero_duration_gt(HeroDuration a, HeroDuration b);

// ===========================================
//
//
// Maths
//
//
// ===========================================

#define HERO_PI 3.14159265358979323846264338327950288
#define HERO_DEGREES_TO_RADIANS(degrees) ((HERO_PI / 180.0) * degrees)

// ===========================================
//
//
// Text Reader
//
//
// ===========================================

typedef struct HeroTextReader HeroTextReader;
struct HeroTextReader {
	U8* string;
	U32 size;
	U32 position;
	U32 row;
	U32 column;
};

void hero_text_reader_init(HeroTextReader* reader, U8* string, U32 size);
U32 hero_text_reader_remaining_size(HeroTextReader* reader);
bool hero_text_reader_has_content(HeroTextReader* reader);
bool hero_text_reader_is_empty(HeroTextReader* reader);

// returns the number of whitespace bytes consumed
U32 hero_text_reader_consume_whitespace(HeroTextReader* reader);

// returns the number of whitespace and newline bytes consumed
U32 hero_text_reader_consume_whitespace_and_newlines(HeroTextReader* reader);

// returns the number of bytes consumed, 0 is returned if the any of the bytes cannot be found
U32 hero_text_reader_consume_until_any_byte(HeroTextReader* reader, HeroString bytes);

// returns true if the byte is has been consumed
bool hero_text_reader_consume_byte(HeroTextReader* reader, U8 byte);

// ===========================================
//
//
// Float16
//
//
// ===========================================

typedef struct F16 F16;
struct F16 {
	U16 bits;
};

F32 f16_to_f32(F16 v);
F16 f16_from_f32(F32 v);
bool f16_is_nan(F16 v);
bool f16_is_inf(F16 v);

// ===========================================
//
//
// Vectors
//
//
// ===========================================

typedef struct Vec2 Vec2;
struct Vec2 {
	F32 x;
	F32 y;
};

typedef struct UVec2 UVec2;
struct UVec2 {
	U32 x;
	U32 y;
};

typedef struct SVec2 SVec2;
struct SVec2 {
	S32 x;
	S32 y;
};

typedef struct Vec2F16 Vec2F16;
struct Vec2F16 {
	F16 x;
	F16 y;
};

typedef struct Vec3 Vec3;
struct Vec3 {
	F32 x;
	F32 y;
	F32 z;
};

typedef struct Vec3F16 Vec3F16;
struct Vec3F16 {
	F16 x;
	F16 y;
	F16 z;
};

typedef struct Vec4 Vec4;
struct Vec4 {
	F32 x;
	F32 y;
	F32 z;
	F32 w;
};

typedef struct UVec4 UVec4;
struct UVec4 {
	U32 x;
	U32 y;
	U32 z;
	U32 w;
};

typedef struct Vec4F16 Vec4F16;
struct Vec4F16 {
	F16 x;
	F16 y;
	F16 z;
	F16 w;
};

#define VEC2_INIT(x_, y_) ((Vec2){ .x = x_, .y = y_ })
#define VEC2_INIT_EVEN(s) ((Vec2){ .x = (s), .y = (s) })
#define VEC2_FROM_F16(v) (Vec2){ hero_f16_to_F32((v).x), hero_f16_to_F32((v).y) }
#define VEC2_TO_F16(v) (Vec2F16){ hero_f16_from_F32((v).x), hero_f16_from_F32((v).y) }
#define VEC2_F16_INIT(x, y) (Vec2F16){ hero_f16_from_F32(x), hero_f16_from_F32(y) }
#define VEC2_ZERO ((Vec2){0})
#define VEC2_INF ((Vec2){INFINITY, INFINITY})
#define VEC2_NEG_INF ((Vec2){-INFINITY, -INFINITY})
bool vec2_eq(Vec2 a, Vec2 b);
Vec2 vec2_copysign(Vec2 v, Vec2 copy);
Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_mul(Vec2 a, Vec2 b);
Vec2 vec2_div(Vec2 a, Vec2 b);
Vec2 vec2_add_scalar(Vec2 v, F32 by);
Vec2 vec2_sub_scalar(Vec2 v, F32 by);
Vec2 vec2_mul_scalar(Vec2 v, F32 by);
Vec2 vec2_div_scalar(Vec2 v, F32 by);

Vec2 vec2_neg(Vec2 v);
F32 vec2_len(Vec2 v);
Vec2 vec2_norm(Vec2 v);
F32 vec2_dot(Vec2 a, Vec2 b);
F32 vec2_angle(Vec2 v);

Vec2 vec2_cross_scalar(Vec2 v, F32 s);
F32 vec2_cross_vec(Vec2 a, Vec2 b);
Vec2 vec2_rotate(Vec2 v, F32 angle);

#define VEC2_UP (Vec2) { 0.0, 1.0 }
#define VEC2_DOWN (Vec2) { 0.0, -1.0 }
#define VEC2_LEFT (Vec2) { -1.0, 0.0 }
#define VEC2_RIGHT (Vec2) { 1.0, 0.0 }
Vec2 vec2_perp_left(Vec2 v);
Vec2 vec2_perp_right(Vec2 v);

Vec2 vec2_min(Vec2 a, Vec2 b);
Vec2 vec2_max(Vec2 a, Vec2 b);
Vec2 vec2_clamp(Vec2 v, Vec2 min, Vec2 max);
Vec2 vec2_lerp_scalar(Vec2 from, Vec2 to, F32 t);
Vec2 vec2_lerp_vec(Vec2 from, Vec2 to, Vec2 t);
Vec2 vec2_sign(Vec2 v);
Vec2 vec2_abs(Vec2 v);
Vec2 vec2_floor(Vec2 v);
Vec2 vec2_ceil(Vec2 v);
Vec2 vec2_round(Vec2 v);
bool vec2_approx_eq(Vec2 a, Vec2 b);

#define VEC3_INIT(x_, y_, z_) ((Vec3){ .x = x_, .y = y_, .z = z_ })
#define VEC3_INIT_EVEN(s) ((Vec3){ .x = (s), .y = (s), .z = (s) })
#define VEC3_FROM_F16(v) (Vec3){ hero_f16_to_F32((v).x), hero_f16_to_F32((v).y), hero_f16_to_F32((v).z) }
#define VEC3_TO_F16(v) (Vec3F16){ hero_f16_from_F32((v).x), hero_f16_from_F32((v).y), hero_f16_from_F32((v).z) }
#define VEC3_F16_INIT(x, y, z) (Vec3F16){ hero_f16_from_F32(x), hero_f16_from_F32(y), hero_f16_from_F32(z) }
#define VEC3_ZERO (Vec3){0}
#define VEC3_INF (Vec3){INFINITY, INFINITY}
#define VEC3_NEG_INF (Vec3){-INFINITY, -INFINITY}
bool vec3_eq(Vec3 a, Vec3 b);
Vec3 vec3_copysign(Vec3 v, Vec3 copy);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 a, Vec3 b);
Vec3 vec3_div(Vec3 a, Vec3 b);
Vec3 vec3_add_scalar(Vec3 v, F32 by);
Vec3 vec3_sub_scalar(Vec3 v, F32 by);
Vec3 vec3_mul_scalar(Vec3 v, F32 by);
Vec3 vec3_div_scalar(Vec3 v, F32 by);

Vec3 vec3_neg(Vec3 v);
F32 vec3_len(Vec3 v);
Vec3 vec3_norm(Vec3 v);
F32 vec3_dot(Vec3 a, Vec3 b);

#define VEC3_UP       (Vec3) {  0.0,  1.0,  0.0 }
#define VEC3_DOWN     (Vec3) {  0.0, -1.0,  0.0 }
#define VEC3_LEFT     (Vec3) { -1.0,  0.0,  0.0 }
#define VEC3_RIGHT    (Vec3) {  1.0,  0.0,  0.0 }
#define VEC3_FORWARD  (Vec3) {  0.0,  0.0,  1.0 }
#define VEC3_BACKWARD (Vec3) {  0.0,  0.0, -1.0 }

Vec3 vec3_cross(Vec3 a, Vec3 b);


Vec3 vec3_perp_x_forward(Vec3 v);
Vec3 vec3_perp_x_backward(Vec3 v);
Vec3 vec3_perp_y_forward(Vec3 v);
Vec3 vec3_perp_y_backward(Vec3 v);

Vec3 vec3_perp_y_left(Vec3 v);
Vec3 vec3_perp_y_right(Vec3 v);
Vec3 vec3_perp_z_left(Vec3 v);
Vec3 vec3_perp_z_right(Vec3 v);

Vec3 vec3_perp_left(Vec3 v);
Vec3 vec3_perp_right(Vec3 v);
Vec3 vec3_perp_backward(Vec3 v);
Vec3 vec3_perp_forward(Vec3 v);

Vec3 vec3_min(Vec3 a, Vec3 b);
Vec3 vec3_max(Vec3 a, Vec3 b);
Vec3 vec3_clamp(Vec3 v, Vec3 min, Vec3 max);
Vec3 vec3_lerp_scalar(Vec3 from, Vec3 to, F32 t);
Vec3 vec3_lerp_vec(Vec3 from, Vec3 to, Vec3 t);
Vec3 vec3_sign(Vec3 v);
Vec3 vec3_abs(Vec3 v);
Vec3 vec3_floor(Vec3 v);
Vec3 vec3_ceil(Vec3 v);
Vec3 vec3_round(Vec3 v);
bool vec3_approx_eq(Vec3 a, Vec3 b);

#define VEC4_INIT(x_, y_, z_, w_) ((Vec4){ .x = x_, .y = y_, .z = z_, .w = w_ })
#define VEC4_INIT_EVEN(s) ((Vec4){ .x = (s), .y = (s), .z = (s), .w = (s) })
#define VEC4_ZERO ((Vec4){0})

// ===========================================
//
//
// Quaternion
//
//
// ===========================================

typedef struct Quat Quat;
struct Quat {
	F32 x;
	F32 y;
	F32 z;
	F32 w;
};

#define QUAT_INIT(x, y, z, w) ((Quat) { x, y, z, w })
#define QUAT_IDENTITY ((Quat) { 0.f, 0.f, 0.f, 1.f })
Quat quat_norm(Quat q);
Quat quat_add(Quat a, Quat b);
Quat quat_sub(Quat a, Quat b);
Quat quat_add_scalar(Quat q, F32 by);
Quat quat_sub_scalar(Quat q, F32 by);
Quat quat_mul(Quat a, Quat b);
Vec3 quat_mul_vec3(Quat q, Vec3 vec);
Quat quat_conj(Quat q);
Quat quat_rotate(float angle, Vec3 axis); // axis must be normalized
F32 quat_euler_roll(Quat q);
F32 quat_euler_pitch(Quat q);
F32 quat_euler_yaw(Quat q);
Quat quat_from_euler(F32 roll, F32 pitch, F32 yaw);

// ===========================================
//
//
// Matrices - row major order
//
//
// ===========================================

typedef union Mat3x2 Mat3x2;
union Mat3x2 {
	Vec2 col[3];
	F32 a[6];
};

void mat3x2_identity(Mat3x2* out);
void mat3x2_identity_translate(Mat3x2* out, Vec2 v);
void mat3x2_identity_scale(Mat3x2* out, Vec2 v);
void mat3x2_identity_rotate(Mat3x2* out, F32 angle);
Vec2 mat3x2_row(Mat3x2* m, U32 row_idx);
Vec3 mat3x2_column(Mat3x2* m, U32 column_idx);

void mat3x2_mul(Mat3x2* out, Mat3x2* a, Mat3x2* b);
Vec2 mat3x2_mul_point(Mat3x2* m, Vec2 pt);
Vec2 mat3x2_mul_vector(Mat3x2* m, Vec2 vector);

typedef union Mat4x4 Mat4x4;
union Mat4x4 {
	Vec4 col[4];
	F32 a[16];
};

void mat4x4_identity(Mat4x4* out);
void mat4x4_identity_scale(Mat4x4* out, Vec3 v);
void mat4x4_identity_rotate(Mat4x4* out, Vec3 v, F32 angle);
void mat4x4_identity_rotate_x(Mat4x4* out, F32 angle);
void mat4x4_identity_rotate_y(Mat4x4* out, F32 angle);
void mat4x4_identity_rotate_z(Mat4x4* out, F32 angle);
void mat4x4_identity_translate(Mat4x4* out, Vec3 v);
void mat4x4_from_quat(Mat4x4* out, Quat quat);

void mat4x4_scale(Mat4x4* m, Vec3 v);
void mat4x4_rotate(Mat4x4* m, Vec3 v, F32 angle);
void mat4x4_rotate_x(Mat4x4* m, F32 angle);
void mat4x4_rotate_y(Mat4x4* m, F32 angle);
void mat4x4_rotate_z(Mat4x4* m, F32 angle);
void mat4x4_translate(Mat4x4* m, Vec3 v);

void mat4x4_from_3x2(Mat4x4* out, Mat3x2* m);
Vec4 mat4x4_row(Mat4x4* m, U32 row_idx);
Vec4 mat4x4_column(Mat4x4* m, U32 column_idx);
void mat4x4_ortho(Mat4x4* out, F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far);
void mat4x4_perspective(Mat4x4* out, F32 fovy, F32 aspect_ratio, F32 z_near, F32 z_far);
void mat4x4_mul(Mat4x4* out, Mat4x4* a, Mat4x4* b);
void mat4x4_mul_quat(Mat4x4* out, Mat4x4* m, Quat q); // must be an orthoganal matrix
Vec3 mat4x4_mul_point(Mat4x4* m, Vec3 pt);
Vec3 mat4x4_mul_vector(Mat4x4* m, Vec3 v);
Vec4 mat4x4_mul_vec4(Mat4x4* m, Vec4 v);

// ===========================================
//
//
// Aabb
//
//
// ===========================================

typedef struct HeroAabb HeroAabb;
struct HeroAabb {
	F32 x;
	F32 y;
	F32 ex;
	F32 ey;
};

typedef struct HeroUAabb HeroUAabb;
struct HeroUAabb {
	S32 x;
	S32 y;
	S32 ex;
	S32 ey;
};

#define HERO_AABB_INIT(x, y, ex, ey) ((HeroAabb){ (x), (y), (ex), (ey) })
#define HERO_AABB_INIT_WH(x, y, w, h) ((HeroAabb){ (x), (y), (x) + (w), (y) + (h) })

static inline F32 hero_aabb_width(HeroAabb* aabb) {
	return aabb->ex - aabb->x;
}

static inline F32 hero_aabb_height(HeroAabb* aabb) {
	return aabb->ey - aabb->y;
}

static inline Vec2 hero_aabb_size(HeroAabb* aabb) {
	return VEC2_INIT(aabb->ex - aabb->x, aabb->ey - aabb->y);
}

static inline Vec2 hero_aabb_half_size(HeroAabb* aabb) {
	return vec2_div_scalar(hero_aabb_size(aabb), 2.f);
}

static inline Vec2 hero_aabb_top_left(HeroAabb* aabb) {
	return VEC2_INIT(aabb->x, aabb->y);
}

static inline Vec2 hero_aabb_top_right(HeroAabb* aabb) {
	return VEC2_INIT(aabb->ex, aabb->y);
}

static inline Vec2 hero_aabb_bottom_right(HeroAabb* aabb) {
	return VEC2_INIT(aabb->ex, aabb->ey);
}

static inline Vec2 hero_aabb_bottom_left(HeroAabb* aabb) {
	return VEC2_INIT(aabb->x, aabb->ey);
}

HeroAabb hero_aabb_cut_left(HeroAabb* parent, float len);
HeroAabb hero_aabb_cut_right(HeroAabb* parent, float len);
HeroAabb hero_aabb_cut_top(HeroAabb* parent, float len);
HeroAabb hero_aabb_cut_bottom(HeroAabb* parent, float len);
HeroAabb hero_aabb_cut_center(HeroAabb* parent, float len);
HeroAabb hero_aabb_cut_center_horizontal(HeroAabb* parent, float len);
HeroAabb hero_aabb_cut_center_vertical(HeroAabb* parent, float len);

HeroAabb hero_aabb_keep_overlapping(HeroAabb* a, HeroAabb* b);
bool hero_aabb_intersects_pt(HeroAabb* a, Vec2 pt);

void hero_aabb_print(HeroAabb* aabb, const char* name);

#define HERO_STACK_ELMT_TYPE HeroAabb
#include "stack_gen.inl"

#define HERO_UAABB_INIT(x, y, ex, ey) ((HeroUAabb){ (x), (y), (ex), (ey) })
#define HERO_UAABB_INIT_WH(x, y, w, h) ((HeroUAabb){ (x), (y), (x) + (w), (y) + (h) })

static inline F32 hero_uaabb_width(HeroUAabb* aabb) {
	return aabb->ex - aabb->x;
}

static inline F32 hero_uaabb_height(HeroUAabb* aabb) {
	return aabb->ey - aabb->y;
}

static inline Vec2 hero_uaabb_size(HeroUAabb* aabb) {
	return VEC2_INIT(aabb->ex - aabb->x, aabb->ey - aabb->y);
}

static inline Vec2 hero_uaabb_half_size(HeroUAabb* aabb) {
	return vec2_div_scalar(hero_uaabb_size(aabb), 2.f);
}

void hero_uaabb_print(HeroUAabb* aabb, const char* name);

#define HERO_STACK_ELMT_TYPE HeroUAabb
#include "stack_gen.inl"

// ==========================================
//
//
// Dynamic Library Loading
//
//
// ==========================================

typedef struct HeroDLL HeroDLL;
struct HeroDLL { void* handle; };

HeroResult hero_dll_open(HeroDLL* dll, const char* path);
void hero_dll_close(HeroDLL* dll);
HeroResult hero_dll_resolve_symbol(HeroDLL* dll, const char* symbol_name, void** addr_out);
const char* hero_dll_error(void);

// ==========================================
//
//
// UTF8
//
//
// ==========================================

static inline bool hero_utf8_is_codepoint_boundary(char ch) {
	// this is bit magic equivalent to: b < 128 || b >= 192
	return ch >= -0x40;
}

static inline U32 hero_utf8_codept_size(char first_byte) {
	static const char lengths[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
	};
	return lengths[first_byte >> 3];
}

static inline U32 hero_utf8_codept_decode(const U8* str, Codept* codept_out) {
	U32 size = hero_utf8_codept_size(str[0]);
	switch (size) {
		case 4:
			*codept_out = ((str[0] & 0x7) << 18) | ((str[1] & 0x3f) << 12) | ((str[2] & 0x3f) << 6) | (str[3] & 0x3f);
			break;
		case 3:
			*codept_out = ((str[0] & 0xf) << 12) | ((str[1] & 0x3f) << 6) | (str[2] & 0x3f);
			break;
		case 2:
			*codept_out = ((str[0] & 0x1f) << 6) | (str[1] & 0x3f);
			break;
		case 1:
			*codept_out = str[0];
			break;
	}

	return size;
}

static inline U32 hero_utf8_codept_iter(HeroString string, Uptr* idx_mut, Codept* codept_out) {
	if (*idx_mut >= string.size) {
		return 0;
	}

	U32 codept_size = hero_utf8_codept_decode(&string.data[*idx_mut], codept_out);
	*idx_mut += codept_size;

	return codept_size;
}

// ==========================================
//
//
// Hero
//
//
// ==========================================

typedef struct Hero Hero;
struct Hero {
	char*** enum_group_strings;
};

typedef struct HeroSetup HeroSetup;
struct HeroSetup {
	char*** enum_group_strings;
};

extern Hero hero;

HeroResult hero_init(HeroSetup* setup);

#include "core_glsl_shared.h"

#endif // _HERO_CORE_H_

