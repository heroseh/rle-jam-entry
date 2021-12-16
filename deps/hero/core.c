#ifndef _HERO_CORE_H_
#include "core.h"
#endif

// ===========================================
//
//
// General
//
//
// ===========================================

char* hero_stacktrace_debug;

void _hero_assert_failed(const char* cond, const char* file, int line, const char* message, ...) {
	fprintf(stderr, "assertion failed: %s\nmessage: ", cond);

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

HERO_NORETURN void _hero_abort(const char* file, int line, const char* message, ...) {
	fprintf(stderr, "abort: ");

	va_list va_args;
	va_start(va_args, message);
	vfprintf(stderr, message, va_args);
	va_end(va_args);

	fprintf(stderr, "\nfile: %s:%u\n", file, line);
	abort();
}

const char* _hero_result_strings[HERO_SUCCESS_COUNT] = {
	[HERO_SUCCESS]          = "success",
	[HERO_SUCCESS_IS_NEW]   = "success is new",
	[HERO_SUCCESS_FINISHED] = "success finished",
};

const char* _hero_error_strings[] = {
	[HERO_ERROR_GENERAL]                                       = "general error",
	[HERO_ERROR_ALLOCATION_FAILURE]                            = "error allocation failure",
	[HERO_ERROR_OBJECT_ID_IS_NULL]                             = "error object id is null",
	[HERO_ERROR_USE_AFTER_FREE]                                = "error use after free",
	[HERO_ERROR_DOES_NOT_EXIST]                                = "error does not exist",
	[HERO_ERROR_ALREADY_EXISTS]                                = "error already exist",
	[HERO_ERROR_ENDED_BY_USER]                                 = "error ended by user",
	[HERO_ERROR_OPENING_DLL]                                   = "error opening dll",
	[HERO_ERROR_UNHANDLED_ENUM]                                = "error unhandle enumeration",
	[HERO_ERROR_NOT_STARTED]                                   = "error not started",
	[HERO_ERROR_FULL]                                          = "error full",

	/*
	[HERO_ERROR_NO_CAPABLE_GPU_FOUND]                          = "error no capable gpu found",
	[HERO_ERROR_SWAPCHAIN_IMAGE_FORMAT_NOT_SUPPORTED]          = "error the VK_FORMAT_B8G8R8A8_UNORM image format is not supported for your devices swapchain",
	*/
};

const char* hero_result_string(HeroResult result) {
	if (result < 0) {
		// TODO: fetch the error string from core, gfx and window system
		return NULL;
	}
	return _hero_result_strings[result];
}

HeroResult hero_result_from_errno(int errnum) {
	switch (errnum) {
		default:
			HERO_ABORT("unhandled errno 0x%x", errnum);
	}
}

// ===========================================
//
//
// Memory Utilities
//
//
// ===========================================

void* hero_system_alloc(void* alctor_data, HeroAllocTag tag, Uptr size, Uptr align) {
	if (size == 0) return NULL;

	HERO_DEBUG_ASSERT(align <= sizeof(void*), "an align is greater than a pointer is not support for this allocator ATM but is possible");
	HERO_DEBUG_ASSERT(HERO_IS_POWER_OF_TWO(align), "align must be a power of two");

	void* ptr = malloc(size);
	if (!ptr) return NULL;

	return ptr;
}

void* hero_system_realloc(void* alctor_data, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr new_size, Uptr align) {
	HERO_DEBUG_ASSERT(align <= sizeof(void*), "an align is greater than a pointer is not support for this allocator ATM but is possible");
	HERO_DEBUG_ASSERT(HERO_IS_POWER_OF_TWO(align), "align must be a power of two");

	void* ptr = realloc(allocated_ptr, new_size);
	if (!ptr) return NULL;

	return ptr;
}

void hero_system_dealloc(void* alctor_data, HeroAllocTag tag, void* allocated_ptr, Uptr size, Uptr align) {
	HERO_DEBUG_ASSERT(align <= sizeof(void*), "an align is greater than a pointer is not support for this allocator ATM but is possible");
	HERO_DEBUG_ASSERT(HERO_IS_POWER_OF_TWO(align), "align must be a power of two");

	free(allocated_ptr);
}

// ==========================================
//
//
// Stacktrace
//
//
// ==========================================

char* hero_stacktrace(U32 ignore_levels_count, HeroIAlctor alctor, U32* size_out) {
#if defined(__GNUC__)
	void* stacktrace_levels[HERO_STACKTRACE_LEVELS_MAX];
	U32 stacktrace_levels_count = backtrace(stacktrace_levels, HERO_STACKTRACE_LEVELS_MAX);

	char** symbol_strings = backtrace_symbols(stacktrace_levels, stacktrace_levels_count);
	if (symbol_strings == NULL) {
		return false;
	}

	U32 string_size = 0;
	for (U32 i = ignore_levels_count; i < stacktrace_levels_count; i += 1) {
		int size = snprintf(NULL, 0, "%u: %s\n", stacktrace_levels_count - i - 1, symbol_strings[i]);
		string_size += size;
	}

	string_size += 1; // null terminator

	char* string = hero_alloc(alctor, HERO_ALLOC_TAG_STACKTRACE, string_size, alignof(char*));

	U32 string_idx = 0;
	for (U32 i = ignore_levels_count; i < stacktrace_levels_count; i += 1) {
		int size = snprintf(string + string_idx, string_size - string_idx, "%u: %s\n", stacktrace_levels_count - i - 1, symbol_strings[i]);
		string_idx += size;
	}

	free(symbol_strings);
	if (size_out) {
		*size_out = string_size;
	}
	return string;
#elif defined(_WIN32)
	void* stacktrace_levels[HERO_STACKTRACE_LEVELS_MAX];
	HANDLE process = GetCurrentProcess();

	SymInitialize(process, NULL, TRUE);

	uint32_t stacktrace_levels_count = CaptureStackBackTrace(0, HERO_STACKTRACE_LEVELS_MAX, stacktrace_levels, NULL);
	SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	for (uint32_t i = ignore_levels_count; i < stacktrace_levels_count; i += 1) {
		SymFromAddr(process, (DWORD64)stacktrace_levels[i], 0, symbol);
		if (!DasStk_push_str_fmt(string_out, "%u: %s - [0x%0X]\n", stacktrace_levels_count - i - 1, symbol->Name, symbol->Address)) {
			return das_false;
		}
	}

	free(symbol);
	return das_true;
#else
#error "hero_stacktrace has been unimplemented for this platform"
#endif
}

// ===========================================
//
//
// Stack
//
//
// ===========================================

HeroResult _hero_stack_push_many(HeroStack* stack, Uptr amount, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align, void** elmts_ptr_out) {
	Uptr new_count = stack->count + amount;
	if (new_count > stack->cap) {
		HeroResult result = _hero_stack_resize_cap(stack, HERO_MAX(stack->cap * 2, new_count), alctor, tag, elmt_size, elmt_align);
		if (result < 0) return result;
	}

	*elmts_ptr_out = HERO_PTR_ADD(stack->data, stack->count * elmt_size);
	stack->count = new_count;
	return HERO_SUCCESS;
}

HeroResult _hero_stack_insert_many(HeroStack* stack, Uptr idx, Uptr amount, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align, void** elmts_ptr_out) {
	if (idx > stack->count) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	//
	// increase the capacity if the number of elements
	// inserted makes the stack exceed it's capacity.
	Uptr new_count = stack->count + amount;
	if (stack->cap < new_count) {
		HeroResult result = _hero_stack_resize_cap(stack, HERO_MAX(stack->cap * 2, new_count), alctor, tag, elmt_size, elmt_align);
		if (result < 0) return result;
	}

	void* dst = HERO_PTR_ADD(stack->data, idx * elmt_size);

	// shift the elements from idx to (idx + amount), to the right
	// to make room for the elements
	memmove(HERO_PTR_ADD(dst, amount * elmt_size), dst, (stack->count - idx) * elmt_size);
	stack->count = new_count;
	*elmts_ptr_out = dst;

	return HERO_SUCCESS;
}


HeroResult _hero_stack_remove_shift_range(HeroStack* stack, Uptr start_idx, Uptr end_idx, Uptr elmt_size) {
	if (start_idx > stack->count || end_idx > stack->count) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	//
	// get the pointer where the elements are being removed
	Uptr remove_count = end_idx - start_idx;
	void* dst = HERO_PTR_ADD(stack->data, start_idx * elmt_size);

	//
	// now replace the elements by shifting the elements to the right over them.
	if (end_idx < stack->count) {
		void* src = HERO_PTR_ADD(dst, remove_count * elmt_size);
		memmove(dst, src, (stack->count - (start_idx + remove_count)) * elmt_size);
	}
	stack->count -= remove_count;

	return HERO_SUCCESS;
}

// ===========================================
//
//
// String
//
//
// ===========================================

HeroResult hero_string_fmt(HeroString* string, HeroIAlctor alctor, HeroAllocTag tag, char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	HeroResult result = hero_string_fmtv(string, alctor, tag, fmt, args);
	va_end(args);
	return HERO_SUCCESS;
}

HeroResult hero_string_fmtv(HeroString* string, HeroIAlctor alctor, HeroAllocTag tag, char* fmt, va_list args) {
	va_list args_copy;
	va_copy(args_copy, args);

	// add 1 so we have enough room for the null terminator that vsnprintf always outputs
	// vsnprintf will return -1 on an encoding error.
	Uptr size = vsnprintf(NULL, 0, fmt, args_copy) + 1;
	va_end(args_copy);
	HERO_ASSERT(size >= 1, "a vsnprintf encoding error has occurred");

	// allocate the buffer for the string
	char* data = hero_alloc(alctor, tag, size, 1);
	if (data == NULL) return HERO_ERROR(ALLOCATION_FAILURE);

	//
	// now call vsnprintf for real this time, with a buffer
	// to actually copy the formatted string.
	size = vsnprintf(data, size, fmt, args);

	*string = hero_string((U8*)data, size);
	return HERO_SUCCESS;
}

// ===========================================
//
//
// Hashing
//
//
// ===========================================

HeroHash hero_hash(const void* data, Uptr size, HeroHash hash) {
	const char* bytes = data;
	if (size == 0) {
		const HeroString* string = data;
		bytes = (const char*)string->data;
		size = string->size;
	}
	const char* bytes_end = bytes + size;

	while (bytes < bytes_end) {
		hash = hash ^ *bytes;
#if HERO_HASH_SIZE == 32
		hash = hash * 0x01000193;
#elif HERO_HASH_SIZE == 64
		hash = hash * 0x00000100000001B3;
#else
#error "unimplemented for hash size"
#endif
		bytes += 1;
	}

	return hash;
}

// ===========================================
//
//
// Hash Table
//
//
// ===========================================

HeroResult _hero_hash_table_resize_cap(HeroHashTable* hash_table, Uptr new_cap, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align) {
	Uptr cap = hash_table->cap;
	HERO_DEBUG_ASSERT(HERO_IS_POWER_OF_TWO(new_cap), "the hash table's new_cap must be a power of two");

	HeroHash* hashes = hero_alloc(alctor, tag, new_cap * sizeof(HeroHash), alignof(HeroHash));
	if (hashes == NULL) return HERO_ERROR(ALLOCATION_FAILURE);

	void* entries = hero_alloc(alctor, tag, new_cap * entry_size, entry_align);
	if (entries == NULL) {
		hero_dealloc(alctor, tag, hashes, new_cap * sizeof(HeroHash), alignof(HeroHash));
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	// set all hashes in the new buffer to HERO_HASH_TABLE_HASH_EMPTY
	HERO_ZERO_ELMT_MANY(hashes, new_cap);

	if (cap) {
		Uptr new_buckets_count = new_cap >> HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
		for (Uptr i = 0; i < cap; i += 1) {
			HeroHash hash = hash_table->hashes[i];
			if (hash >= HERO_HASH_TABLE_HASH_START) {
				Uptr new_bucket_idx, step, pos;
				//
				// found a entry so locate it's new bucket.
				new_bucket_idx = hash & (new_buckets_count - 1);
				step = 1;
				while (1) {
					pos = new_bucket_idx << HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
					for (Uptr j = pos; j < pos + HERO_HASH_TABLE_BUCKET_ENTRIES_COUNT; j += 1) {
						switch (hashes[j]) {
							case HERO_HASH_TABLE_HASH_EMPTY:
							case HERO_HASH_TABLE_HASH_TOMBSTONE:
								//
								// found a new location, copy the value over
								hashes[j] = hash;
								memcpy(HERO_PTR_ADD(entries, j * entry_size), HERO_PTR_ADD(hash_table->entries, i * entry_size), entry_size);
								goto INSERT_SUCCESS;
						}
					}

					//
					// that bucket was full. so move to the next bucket.
					// quadratic probing to help reduce clustering of entries
					new_bucket_idx += step;
					step += 1;
					new_bucket_idx &= (new_buckets_count - 1);
				}
			}
INSERT_SUCCESS: {}
		}
	}

	hash_table->hashes = hashes;
	hash_table->entries = entries;
	hash_table->cap = new_cap;
	return HERO_SUCCESS;
}

HeroResult _hero_hash_table_init(HeroHashTable* hash_table, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align) {
	return _hero_hash_table_resize_cap(hash_table, HERO_HASH_TABLE_INIT_CAP, alctor, tag, entry_size, entry_align);
}

void _hero_hash_table_deinit(HeroHashTable* hash_table, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align) {
	hero_dealloc(alctor, tag, hash_table->hashes, hash_table->cap * sizeof(HeroHash), alignof(HeroHash));
	hero_dealloc(alctor, tag, hash_table->entries, hash_table->cap * entry_size, entry_align);
	HERO_ZERO_ELMT(hash_table);
}

HeroResult _hero_hash_table_find(HeroHashTable* hash_table, const void* key, Uptr key_size, Uptr entry_size, void** entry_ptr_out) {
	if (HERO_UNLIKELY(hash_table->cap == 0)) return HERO_ERROR(DOES_NOT_EXIST);
	//
	// hash the key and ensure it is no one of the special marker hash values.
	HeroHash hash = hero_hash(key, key_size, HERO_HASH_INIT);
	if (hash < HERO_HASH_TABLE_HASH_START) hash += HERO_HASH_TABLE_HASH_START;

	//
	// divide the entry capacity by the number of entries in a bucket
	// and see what bucket we should start our search in.
	Uptr buckets_count = hash_table->cap >> HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
	Uptr bucket_idx = hash & (buckets_count - 1);

	Uptr step = 1;
	while (1) {
		//
		// multiply by the number of entries in a bucket to get the position in the entry arrays.
		Uptr pos = bucket_idx << HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
		for (Uptr i = pos; i < pos + HERO_HASH_TABLE_BUCKET_ENTRIES_COUNT; i += 1) {
			HeroHash existing_hash = existing_hash = hash_table->hashes[i];
			if (existing_hash == hash) {
				//
				// hash matches, check if the key matches
				void* entry_ptr = HERO_PTR_ADD(hash_table->entries, i * entry_size);
				if (hero_cmp(key, entry_ptr, key_size)) {
					// key matches, success!
					*entry_ptr_out = entry_ptr;
					return HERO_SUCCESS;
				}
			} else if (existing_hash == HERO_HASH_TABLE_HASH_EMPTY) {
				// found an empty hash, no other entries have been inserted past this point
				*entry_ptr_out = NULL;
				return HERO_ERROR(DOES_NOT_EXIST);
			}
		}

		//
		// quadratic probing to help reduce clustering of entries
		bucket_idx += step;
		step += 1;
		bucket_idx &= (buckets_count - 1);
	}
}

HeroResult _hero_hash_table_find_or_insert(HeroHashTable* hash_table, const void* key, Uptr key_size, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align, void** entry_ptr_out) {
	//
	// hash the key and ensure it is no one of the special marker hash values.
	HeroHash hash = hero_hash(key, key_size, HERO_HASH_INIT);
	if (hash < HERO_HASH_TABLE_HASH_START) hash += HERO_HASH_TABLE_HASH_START;

	if (HERO_UNLIKELY(hash_table->cap == 0) || hash_table->count / 8 * 7 >= hash_table->cap) { // extend when 87.5% full
		Uptr new_cap = HERO_UNLIKELY(hash_table->cap == 0) ? HERO_HASH_TABLE_INIT_CAP : hash_table->cap * 2;
		HeroResult result = _hero_hash_table_resize_cap(hash_table, new_cap, alctor, tag, entry_size, entry_align);
		if (result < 0) return result;
	}

	//
	// divide the entry capacity by the number of entries in a bucket
	// and see what bucket we should start our search in.
	Uptr buckets_count = hash_table->cap >> HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
	Uptr bucket_idx = hash & (buckets_count - 1);

	Uptr step = 1;
	while (1) {
		//
		// multiply by the number of entries in a bucket to get the position in the entry arrays.
		Uptr pos = bucket_idx << HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
		for (Uptr i = pos; i < pos + HERO_HASH_TABLE_BUCKET_ENTRIES_COUNT; i += 1) {
			HeroHash existing_hash = hash_table->hashes[i];
			if (existing_hash == hash) {
				// hash matches, check if the key matches
				void* entry_ptr = HERO_PTR_ADD(hash_table->entries, i * entry_size);
				if (hero_cmp(key, entry_ptr, key_size)) {
					// key matches, success!
					*entry_ptr_out = entry_ptr;
					return HERO_SUCCESS;
				}
			} else if (existing_hash == HERO_HASH_TABLE_HASH_EMPTY || existing_hash == HERO_HASH_TABLE_HASH_TOMBSTONE) {
				//
				// found an empty or tombstone hash that we can use for our value.
				void* entry_ptr = HERO_PTR_ADD(hash_table->entries, i * entry_size);
				hash_table->count += 1;
				*entry_ptr_out = entry_ptr;
				hash_table->hashes[i] = hash;
				memcpy(entry_ptr, key, key_size ? key_size : sizeof(HeroString));
				return HERO_SUCCESS_IS_NEW;
			}
		}

		//
		// quadratic probing to help reduce clustering of entries
		bucket_idx += step;
		step += 1;
		bucket_idx &= (buckets_count - 1);
	}
}

HeroResult _hero_hash_table_remove(HeroHashTable* hash_table, const void* key, Uptr key_size, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align) {
	if (HERO_UNLIKELY(hash_table->count == 0)) return HERO_ERROR(DOES_NOT_EXIST);

	//
	// hash the key and ensure it is no one of the special marker hash values.
	HeroHash hash = hero_hash(key, key_size, HERO_HASH_INIT);
	if (hash < HERO_HASH_TABLE_HASH_START) hash += HERO_HASH_TABLE_HASH_START;

	if (hash_table->count >= hash_table->cap / 4) { // shrink when 25.0% empty
		HeroResult result = _hero_hash_table_resize_cap(hash_table, hash_table->cap / 2, alctor, tag, entry_size, entry_align);
		if (result < 0) return result;
	}

	//
	// divide the entry capacity by the number of entries in a bucket
	// and see what bucket we should start our search in.
	Uptr buckets_count = hash_table->cap >> HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
	Uptr bucket_idx = hash & (buckets_count - 1);

	Uptr step = 1;
	while (1) {
		//
		// multiply by the number of entries in a bucket to get the position in the entry arrays.
		Uptr pos = bucket_idx << HERO_HASH_TABLE_BUCKET_ENTRIES_SHIFT;
		for (Uptr i = pos; i < pos + HERO_HASH_TABLE_BUCKET_ENTRIES_COUNT; i += 1) {
			HeroHash existing_hash = hash_table->hashes[i];
			if (existing_hash == hash) {
				// hash matches, check if the key matches
				void* entry_ptr = HERO_PTR_ADD(hash_table->entries, i * entry_size);
				if (hero_cmp(key, entry_ptr, key_size)) {
					// key matches, success! replace the hash with a tombstone
					hash_table->hashes[i] = HERO_HASH_TABLE_HASH_TOMBSTONE;
					return HERO_SUCCESS;
				}
			} else if (existing_hash == HERO_HASH_TABLE_HASH_EMPTY) {
				// found an empty hash, no other entries have been inserted past this point
				return HERO_ERROR(DOES_NOT_EXIST);
			}
		}

		//
		// quadratic probing to help reduce clustering of entries
		bucket_idx += step;
		step += 1;
		bucket_idx &= (buckets_count - 1);
	}
}

HeroResult _hero_hash_table_iter_next(HeroHashTable* hash_table, Uptr entry_size, Uptr* next_entry_idx_mut, void** entry_ptr_out) {
	for (Uptr i = *next_entry_idx_mut; i < hash_table->cap; i += 1) {
		if (hash_table->hashes[i] >= HERO_HASH_TABLE_HASH_START) {
			*entry_ptr_out = HERO_PTR_ADD(hash_table->entries, i * entry_size);
			*next_entry_idx_mut = i + 1;
			return HERO_SUCCESS;
		}
	}

	*next_entry_idx_mut = hash_table->cap;
	return HERO_SUCCESS_FINISHED;
}

HeroResult _hero_hash_table_clear(HeroHashTable* hash_table, HeroIAlctor alctor, HeroAllocTag tag, Uptr entry_size, Uptr entry_align) {
	if (hash_table->cap == HERO_HASH_TABLE_INIT_CAP) {
		memset(hash_table->hashes, 0, sizeof(*hash_table->hashes) * hash_table->cap);
		hash_table->count = 0;
	} else {
		_hero_hash_table_deinit(hash_table, alctor, tag, entry_size, entry_align);
		HeroResult result = _hero_hash_table_init(hash_table, alctor, tag, entry_size, entry_align);
		if (result < 0) {
			return result;
		}
	}

	return HERO_SUCCESS;
}

bool hero_cmp(const void* a, const void* b, Uptr size) {
	if (size == 0) {
		const HeroString* a_ = a;
		const HeroString* b_ = b;
		return a_->size == b_->size && strncmp((const char*)a_->data, (const char*)b_->data, a_->size) == 0;
	} else {
		return memcmp(a, b, size) == 0;
	}
}

// ===========================================
//
//
// Object Pool
//
//
// ===========================================

HeroResult _hero_object_pool_init(HeroObjectPool* object_pool, U32 cap, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align) {
	HERO_ASSERT(cap, "the object pool must be allocated with at least 1 entry");

	Uptr size = (Uptr)cap * elmt_size;
	void* data = hero_alloc(alctor, tag, size, elmt_align);
	if (data == NULL) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	object_pool->data = data;
	object_pool->count = 0;
	object_pool->cap = cap; // we have a null entry, so deduct that from the capacity.
	object_pool->free_list_head_idx = cap;
	object_pool->allocated_list_head_idx = cap;
	object_pool->allocated_list_tail_idx = cap;

	return HERO_SUCCESS;
}

void _hero_object_pool_deinit(HeroObjectPool* object_pool, HeroIAlctor alctor, HeroAllocTag tag, Uptr elmt_size, Uptr elmt_align) {
	hero_dealloc(alctor, tag, object_pool->data, (Uptr)object_pool->cap * elmt_size, elmt_align);
	HERO_ZERO_ELMT(object_pool);
}

HeroResult _hero_object_pool_alloc(HeroObjectPool* object_pool, Uptr elmt_size, U32 idx_bits_count, void** ptr_out, HeroObjectIdRaw* raw_id_out) {
	HeroObjectHeader* object;
	U32 object_idx;
	U32 object_counter;

	HERO_ASSERT(object_pool->cap, "object pool must be initialized before allocating any objects");

	if (object_pool->free_list_head_idx == object_pool->cap) {
		//
		// the free list is empty so we get our newly allocated object from the end of the array.
		if (object_pool->count == object_pool->cap) {
			return HERO_ERROR(ALLOCATION_FAILURE);
		}
		object = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, (Uptr)object_pool->count * elmt_size);
		object_idx = object_pool->count;
		object_counter = object_idx ? 0 : 1; // the first object's counter starts on 1 so we can have a null id
	} else {
		//
		// grab the newly allocated object from the free list head.
		object_idx = object_pool->free_list_head_idx;
		object = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, (Uptr)object_pool->free_list_head_idx * elmt_size);
		object_counter = _hero_object_id_raw_counter(object->next_id, idx_bits_count);

		// set the free list head to where our newly allocated object was pointing to next.
		object_pool->free_list_head_idx = _hero_object_id_raw_idx(object->next_id, idx_bits_count);
	}

	// zero the object and setup the header to be at the end of the allocated list
	memset(object, 0, elmt_size);
	object->next_id = _hero_object_id_raw_init(object_pool->cap, object_counter, idx_bits_count);
	object->prev_idx = object_pool->allocated_list_tail_idx;

	if (object_pool->allocated_list_tail_idx != object_pool->cap) {
		//
		// we have allocated objects, make the tail object point to our newly allocated object
		HeroObjectHeader* old_tail_object = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, (Uptr)object_pool->allocated_list_tail_idx * elmt_size);
		_hero_object_id_raw_set_idx(&old_tail_object->next_id, object_idx, idx_bits_count);
	} else {
		//
		// we have not allocated objects, so this object is also at the had of the allocated list.
		object_pool->allocated_list_head_idx = object_idx;
	}

	// make our object the tail of the allocated list
	object_pool->allocated_list_tail_idx = object_idx;
	object_pool->count += 1;

	*ptr_out = object;
	*raw_id_out = _hero_object_id_raw_init(object_idx, object_counter, idx_bits_count);
	return HERO_SUCCESS;
}

HeroResult _hero_object_pool_dealloc(HeroObjectPool* object_pool, HeroObjectIdRaw raw_id, Uptr elmt_size, U32 idx_bits_count) {
	HeroObjectHeader* object;
	HeroResult result = _hero_object_pool_get(object_pool, raw_id, elmt_size, idx_bits_count, (void**)&object);
	if (result < 0) return result;

	//
	// remove from the allocated list
	{
		U32 next_idx = _hero_object_id_raw_idx(object->next_id, idx_bits_count);
		if (object->prev_idx != object_pool->cap) {
			//
			// make the previous object point to the deallocated object's next object
			HeroObjectHeader* prev = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, (Uptr)object->prev_idx * elmt_size);
			_hero_object_id_raw_set_idx(&prev->next_id, next_idx, idx_bits_count);
		} else {
			//
			// deallocated object is at the head of the allocated list,
			// make the next object the new allocated list head.
			object_pool->allocated_list_head_idx = next_idx;
		}

		if (next_idx != object_pool->cap) {
			//
			// make the next object point to the deallocated object's previous object
			HeroObjectHeader* next = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, (Uptr)next_idx * elmt_size);
			next->prev_idx = object->prev_idx;
		} else {
			//
			// deallocated object is at the tail of the allocated list,
			// make the previous object the new allocated list tail.
			object_pool->allocated_list_tail_idx = object->prev_idx;
		}
	}

	//
	// increment the counter of the deallocated object
	U32 idx = _hero_object_id_raw_idx(raw_id, idx_bits_count);
	U32 counter = _hero_object_id_raw_counter(raw_id, idx_bits_count);
	counter += 1;
	if (HERO_UNLIKELY(counter > HERO_OBJECT_ID_COUNTER_MASK(idx_bits_count))) {
		counter = idx ? 0 : 1; // the first object's counter starts on 1 so we can have a null id
	}

	//
	// add the deallocated object to the free list
	object->next_id = _hero_object_id_raw_init(object_pool->free_list_head_idx, counter, idx_bits_count);
	object_pool->free_list_head_idx = idx;

	object_pool->count -= 1;
	return HERO_SUCCESS;
}

HeroResult _hero_object_pool_get(HeroObjectPool* object_pool, HeroObjectIdRaw raw_id, Uptr elmt_size, U32 idx_bits_count, void** ptr_out) {
	if (raw_id == 0) {
		return HERO_ERROR(OBJECT_ID_IS_NULL);
	}

	U32 idx = _hero_object_id_raw_idx(raw_id, idx_bits_count);
	if (idx > object_pool->cap) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	HeroObjectHeader* object = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, (Uptr)(idx * elmt_size));
	if (_hero_object_id_raw_counter(object->next_id, idx_bits_count) != _hero_object_id_raw_counter(raw_id, idx_bits_count)) {
		return HERO_ERROR(USE_AFTER_FREE);
	}

	*ptr_out = object;
	return HERO_SUCCESS;
}

HeroResult _hero_object_pool_get_id(HeroObjectPool* object_pool, void* object, Uptr elmt_size, U32 idx_bits_count, HeroObjectIdRaw* raw_id_out) {
	if (object <= object_pool->data || object > HERO_PTR_ADD(object_pool->data, (Uptr)object_pool->cap * elmt_size)) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	HeroObjectHeader* obj = (HeroObjectHeader*)object;
	*raw_id_out = _hero_object_id_raw_init(HERO_PTR_DIFF(object, object_pool->data) / elmt_size, _hero_object_id_raw_counter(obj->next_id, idx_bits_count), idx_bits_count);
	return HERO_SUCCESS;
}

void _hero_object_pool_clear(HeroObjectPool* object_pool) {
	object_pool->count = 0;
	object_pool->free_list_head_idx = object_pool->cap;
	object_pool->allocated_list_head_idx = object_pool->cap;
}

HeroResult _hero_object_pool_iter_next(HeroObjectPool* object_pool, HeroObjectIdRaw* raw_id_mut, Uptr elmt_size, U32 idx_bits_count, void** ptr_out) {
	HeroObjectIdRaw raw_id = *raw_id_mut;

	HeroObjectHeader* object;
	if (HERO_UNLIKELY(raw_id == 0)) {
		if (HERO_UNLIKELY(object_pool->allocated_list_head_idx == object_pool->cap)) {
			// pool has no allocated objects
			return HERO_SUCCESS_FINISHED;
		}

		//
		// return the head of the allocated object list
		object = HERO_PTR_ADD(object_pool->data, (Uptr)object_pool->allocated_list_head_idx * elmt_size);
		*raw_id_mut = _hero_object_id_raw_init(object_pool->allocated_list_head_idx, _hero_object_id_raw_counter(object->next_id, idx_bits_count), idx_bits_count);
		*ptr_out = object;
		return HERO_SUCCESS;
	}

	//
	// get the next object
	U32 idx = _hero_object_id_raw_idx(*raw_id_mut, idx_bits_count);
	object = HERO_PTR_ADD(object_pool->data, (Uptr)idx * elmt_size);
	U32 next_idx = _hero_object_id_raw_idx(object->next_id, idx_bits_count);
	if (HERO_UNLIKELY(next_idx == object_pool->cap)) {
		//
		// there is no next object we have reached the end of the allocated list
		*raw_id_mut = 0;
		*ptr_out = NULL;
		return HERO_SUCCESS_FINISHED;
	}

	//
	// get the identifier of the next object and pass back out the object's pointer
	HeroObjectHeader* next = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, next_idx * elmt_size);
	*raw_id_mut = _hero_object_id_raw_init(next_idx, _hero_object_id_raw_counter(next->next_id, idx_bits_count), idx_bits_count);
	*ptr_out = next;
	return HERO_SUCCESS;
}

HeroResult _hero_object_pool_iter_prev(HeroObjectPool* object_pool, HeroObjectIdRaw* raw_id_mut, Uptr elmt_size, U32 idx_bits_count, void** ptr_out) {
	HeroObjectIdRaw raw_id = *raw_id_mut;

	HeroObjectHeader* object;
	if (HERO_UNLIKELY(raw_id == 0)) {
		if (HERO_UNLIKELY(object_pool->allocated_list_tail_idx == object_pool->cap)) {
			// pool has no allocated objects
			return HERO_SUCCESS_FINISHED;
		}

		//
		// return the tail of the allocated object list
		object = HERO_PTR_ADD(object_pool->data, (Uptr)object_pool->allocated_list_tail_idx * elmt_size);
		*raw_id_mut = _hero_object_id_raw_init(object_pool->allocated_list_tail_idx, _hero_object_id_raw_counter(object->next_id, idx_bits_count), idx_bits_count);
		*ptr_out = object;
		return HERO_SUCCESS;
	}

	//
	// get the previous object
	U32 idx = _hero_object_id_raw_idx(*raw_id_mut, idx_bits_count);
	object = HERO_PTR_ADD(object_pool->data, (Uptr)idx * elmt_size);
	if (HERO_UNLIKELY(object->prev_idx == object_pool->cap)) {
		//
		// there is no previous object we have reached the end of the allocated list
		*raw_id_mut = 0;
		*ptr_out = NULL;
		return HERO_SUCCESS_FINISHED;
	}

	//
	// get the identifier of the previous object and pass back out the object's pointer
	HeroObjectHeader* prev = (HeroObjectHeader*)HERO_PTR_ADD(object_pool->data, object->prev_idx * elmt_size);
	*raw_id_mut = _hero_object_id_raw_init(object->prev_idx, _hero_object_id_raw_counter(prev->next_id, idx_bits_count), idx_bits_count);
	*ptr_out = prev;
	return HERO_SUCCESS;
}

// ===========================================
//
//
// Virtual Memory Abstraction
//
//
// ===========================================

HeroResult _hero_os_get_last_error() {
#ifdef __linux__
	switch (errno) {
		case ENOENT:
			return HERO_ERROR(DOES_NOT_EXIST);
		case EEXIST:
			return HERO_ERROR(ALREADY_EXISTS);
		default: HERO_ABORT("unhandled error: %d 0x%x", errno, errno);
	}
#elif _WIN32
	DWORD errnum = GetLastError();
	switch (errnum) {
		default: HERO_ABORT("unhandled error: %d 0x%x", errnum, errnum);
	}
#else
#error "unimplemented platform"
#endif
}

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
static int _hero_virt_mem_prot_unix(HeroVirtMemProtection prot) {
	switch (prot) {
		case HERO_VIRT_MEM_PROTECTION_NO_ACCESS: return 0;
		case HERO_VIRT_MEM_PROTECTION_READ: return PROT_READ;
		case HERO_VIRT_MEM_PROTECTION_READ_WRITE: return PROT_READ | PROT_WRITE;
		case HERO_VIRT_MEM_PROTECTION_EXEC_READ: return PROT_EXEC | PROT_READ;
		case HERO_VIRT_MEM_PROTECTION_EXEC_READ_WRITE: return PROT_READ | PROT_WRITE | PROT_EXEC;
	}
	return 0;
}
#elif _WIN32
static DWORD _hero_virt_mem_prot_windows(HeroVirtMemProtection prot) {
	switch (prot) {
		case HERO_VIRT_MEM_PROTECTION_NO_ACCESS: return PAGE_NOACCESS;
		case HERO_VIRT_MEM_PROTECTION_READ: return PAGE_READONLY;
		case HERO_VIRT_MEM_PROTECTION_READ_WRITE: return PAGE_READWRITE;
		case HERO_VIRT_MEM_PROTECTION_EXEC_READ: return PAGE_EXECUTE_READ;
		case HERO_VIRT_MEM_PROTECTION_EXEC_READ_WRITE: return PAGE_EXECUTE_READWRITE;
	}
	return 0;
}
#else
#error "unimplemented virtual memory API for this platform"
#endif

HeroResult hero_virt_mem_page_size_get(Uptr* page_size_out, Uptr* reserve_align_out) {
#ifdef __linux__
	long page_size = sysconf(_SC_PAGESIZE);
	if (page_size ==  (long)-1)
		return _hero_os_get_last_error();

	*page_size_out = page_size;
	*reserve_align_out = page_size;
#elif _WIN32
	SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
	*page_size_out = si.dwPageSize;
	*reserve_align_out = si.dwAllocationGranularity;
#else
#error "unimplemented virtual memory API for this platform"
#endif

	return HERO_SUCCESS;
}

HeroResult hero_virt_mem_reserve(void* requested_addr, Uptr size, void** addr_out) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	// memory is automatically commited on Unix based OSs,
	// so we will restrict the memory from being accessed on reserved.
	int prot = 0;

	// MAP_ANON = means map physical memory and not a file. it also means the memory will be initialized to zero
	// MAP_PRIVATE = keep memory private so child process cannot access them
	// MAP_NORESERVE = do not reserve any swap space for this mapping
	void* addr = mmap(requested_addr, size, prot, MAP_ANON | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
	if (addr == MAP_FAILED)
		return _hero_os_get_last_error();
#elif _WIN32
	void* addr = VirtualAlloc(requested_addr, size, MEM_RESERVE, PAGE_NOACCESS);
	if (addr == NULL)
		return _hero_os_get_last_error();
#else
#error "TODO implement virtual memory for this platform"
#endif

	*addr_out = addr;
	return HERO_SUCCESS;
}

HeroResult hero_virt_mem_commit(void* addr, Uptr size, HeroVirtMemProtection protection) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	// memory is automatically commited on Unix based OSs,
	// memory is restricted from being accessed in our hero_virt_mem_reserve.
	// so lets just apply the protection for the address space.
	// and then advise the OS that these pages will be used soon.
	// TODO: madvise MADV_WILLNEED is up to the operating system to choose
	// when it wants to assign pages to the address space. what might be
	// better is to loop over the pages and write a full cacheline to the
	// start of each page to let the OS know we need these pages now plz.
	int prot = _hero_virt_mem_prot_unix(protection);
	if (mprotect(addr, size, prot) != 0) return _hero_os_get_last_error();
	if (madvise(addr, size, MADV_WILLNEED) != 0) return _hero_os_get_last_error();
#elif _WIN32
	DWORD prot = _hero_virt_mem_prot_windows(protection);
	if (VirtualAlloc(addr, size, MEM_COMMIT, prot) == NULL)
		return _hero_os_get_last_error();
#else
#error "TODO implement virtual memory for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_virt_mem_protection_set(void* addr, Uptr size, HeroVirtMemProtection protection) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int prot = _hero_virt_mem_prot_unix(protection);
	if (mprotect(addr, size, prot) != 0)
		return _hero_os_get_last_error();
#elif _WIN32
	DWORD prot = _hero_virt_mem_prot_windows(protection);
	DWORD old_prot; // unused
	if (!VirtualProtect(addr, size, prot, &old_prot))
		return _hero_os_get_last_error();
#else
#error "TODO implement virtual memory for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_virt_mem_decommit(void* addr, Uptr size) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))

	//
	// advise the OS that these pages will not be needed.
	// the OS will zero fill these pages before you next access them.
	if (madvise(addr, size, MADV_DONTNEED) != 0)
		return _hero_os_get_last_error();

	// memory is automatically commited on Unix based OSs,
	// so we will restrict the memory from being accessed when we "decommit.
	int prot = 0;
	if (mprotect(addr, size, prot) != 0)
		return _hero_os_get_last_error();
#elif _WIN32
	if (VirtualFree(addr, size, MEM_DECOMMIT) == 0)
		return _hero_os_get_last_error();
#else
#error "TODO implement virtual memory for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_virt_mem_release(void* addr, Uptr size) {
#ifdef __linux__
	if (munmap(addr, size) != 0)
		return _hero_os_get_last_error();
#elif _WIN32
	//
	// unfortunately on Windows all memory must be release at once
	// that was reserved with VirtualAlloc.
	if (!VirtualFree(addr, 0, MEM_RELEASE))
		return _hero_os_get_last_error();
#else
#error "TODO implement virtual memory for this platform"
#endif
	return HERO_SUCCESS;
}

// ===========================================
//
//
// File Utilities
//
//
// ===========================================

HeroResult hero_file_open(const char* path, HeroFileFlags flags, HeroFileHandle* file_handle_out) {
	HERO_ASSERT(
		(flags & (HERO_FILE_FLAGS_READ | HERO_FILE_FLAGS_WRITE | HERO_FILE_FLAGS_APPEND)),
		"HERO_FILE_FLAGS_{READ, WRITE OR APPEND} must be set when opening a file"
	);

	HERO_ASSERT(
		(flags & (HERO_FILE_FLAGS_WRITE | HERO_FILE_FLAGS_APPEND))
			|| !(flags & (HERO_FILE_FLAGS_CREATE_IF_NOT_EXIST | HERO_FILE_FLAGS_CREATE_NEW | HERO_FILE_FLAGS_TRUNCATE)),
		"file must be opened with HERO_FILE_FLAGS_{WRITE or APPEND} IF HERO_FILE_FLAGS_{CREATE_IF_NOT_EXIST, CREATE_NEW or TRUNCATE} exists"
	);

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int fd_flags = O_CLOEXEC;

	if (flags & HERO_FILE_FLAGS_CREATE_NEW) {
		fd_flags |= O_CREAT | O_EXCL;
	} else {
		if (flags & HERO_FILE_FLAGS_CREATE_IF_NOT_EXIST) {
			fd_flags |= O_CREAT;
		}

		if (flags & HERO_FILE_FLAGS_TRUNCATE) {
			fd_flags |= O_TRUNC;
		}
	}

	if ((flags & HERO_FILE_FLAGS_READ) && (flags & HERO_FILE_FLAGS_WRITE)) {
		fd_flags |= O_RDWR;
		if (flags & HERO_FILE_FLAGS_APPEND) {
			fd_flags |= O_APPEND;
		}
	} else if (flags & HERO_FILE_FLAGS_READ) {
		fd_flags |= O_RDONLY;
	} else if (flags & HERO_FILE_FLAGS_WRITE) {
		fd_flags |= O_WRONLY;
		if (flags & HERO_FILE_FLAGS_APPEND) {
			fd_flags |= O_APPEND;
		}
	}

	mode_t mode = 0600; // TODO allow the user to supply a mode.
	int fd = open(path, fd_flags, mode);
	if (fd == -1) return _hero_os_get_last_error();

	*file_handle_out = (HeroFileHandle) { .raw = fd };
#elif _WIN32
	DWORD win_access = 0;
	DWORD win_creation = 0;

	if (flags & HERO_FILE_FLAGS_CREATE_NEW) {
		win_creation |= CREATE_NEW;
	} else {
		if ((flags & HERO_FILE_FLAGS_CREATE_IF_NOT_EXIST) && (flags & HERO_FILE_FLAGS_TRUNCATE)) {
			win_creation |= CREATE_ALWAYS;
		} else if (flags & HERO_FILE_FLAGS_CREATE_IF_NOT_EXIST) {
			win_creation |= OPEN_ALWAYS;
		} else if (flags & HERO_FILE_FLAGS_TRUNCATE) {
			win_creation |= TRUNCATE_EXISTING;
		}
	}

	if (win_creation == 0)
		win_creation = OPEN_EXISTING;

	if (flags & HERO_FILE_FLAGS_READ) {
		win_access |= GENERIC_READ;
	}

	if (flags & HERO_FILE_FLAGS_WRITE) {
		if (flags & HERO_FILE_FLAGS_APPEND) {
			win_access |= FILE_GENERIC_WRITE & ~FILE_WRITE_DATA;
		} else {
			win_access |= GENERIC_WRITE;
		}
	}

	HANDLE handle = CreateFileA(path, win_access, 0, NULL, win_creation, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle == INVALID_HANDLE_VALUE)
		return _hero_os_get_last_error();

	*file_handle_out = (HeroFileHandle) { .raw = handle };
#endif

	return HERO_SUCCESS;
}

HeroResult hero_file_close(HeroFileHandle handle) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	if (close(handle.raw) != 0)
		return _hero_os_get_last_error();
#elif _WIN32
	if (CloseHandle(handle.raw) == 0)
		return _hero_os_get_last_error();
#else
#error "unimplemented file API for this platform"
#endif

	return HERO_SUCCESS;
}

HeroResult hero_file_size(HeroFileHandle handle, U64* size_out) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	// get the size of the file
	struct stat s = {0};
	if (fstat(handle.raw, &s) != 0) return _hero_os_get_last_error();
	*size_out = s.st_size;
#elif _WIN32
	LARGE_INTEGER size;
	if (!GetFileSizeEx(handle.raw, &size)) return _hero_os_get_last_error();
	*size_out = size.QuadPart;
#else
#error "unimplemented file API for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_file_read(HeroFileHandle handle, void* data_out, Uptr size, Uptr* bytes_read_out) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	ssize_t bytes_read = read(handle.raw, data_out, size);
	if (bytes_read == (ssize_t)-1)
		return _hero_os_get_last_error();
#elif _WIN32
	size = hero_min_u(size, UINT32_MAX);
	DWORD bytes_read;
	if (ReadFile(handle.raw, data_out, size, &bytes_read, NULL) == 0)
		return _hero_os_get_last_error();
#else
#error "unimplemented file API for this platform"
#endif

	*bytes_read_out = bytes_read;
	return HERO_SUCCESS;
}

HeroResult hero_file_read_exact(HeroFileHandle handle, void* data_out, Uptr size, Uptr* bytes_read_out) {
	Uptr og_size = size;
	while (size) {
		Uptr bytes_read;
		HeroResult result = hero_file_read(handle, data_out, size, &bytes_read);
		if (result < 0) return result;

		if (bytes_read == 0)
			break;

		data_out = HERO_PTR_ADD(data_out, bytes_read);
		size -= bytes_read;
	}

	*bytes_read_out = og_size - size;
	return HERO_SUCCESS;
}

HeroResult hero_file_write(HeroFileHandle handle, void* data, Uptr size, Uptr* bytes_written_out) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	ssize_t bytes_written = write(handle.raw, data, size);
	if (bytes_written == (ssize_t)-1)
		return _hero_os_get_last_error();
#elif _WIN32
	size = hero_min_u(size, UINT32_MAX);
	DWORD bytes_written;
	if (WriteFile(handle.raw, data, size, &bytes_written, NULL) == 0)
		return _hero_os_get_last_error();
#else
#error "unimplemented file API for this platform"
#endif

	*bytes_written_out = bytes_written;
	return HERO_SUCCESS;
}

HeroResult hero_file_write_exact(HeroFileHandle handle, void* data, Uptr size, Uptr* bytes_written_out) {
	Uptr og_size = size;
	while (size) {
		Uptr bytes_written;
		HeroResult result = hero_file_write(handle, data, size, &bytes_written);
		if (result < 0) return result;

		if (bytes_written == 0)
			break;

		data = HERO_PTR_ADD(data, bytes_written);
		size -= bytes_written;
	}

	*bytes_written_out = og_size - size;
	return HERO_SUCCESS;
}

HeroResult hero_file_seek(HeroFileHandle handle, S64 offset, HeroFileSeekFrom from, U64* cursor_offset_out) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	int whence = 0;
	switch (from) {
		case HERO_FILE_SEEK_FROM_START: whence = SEEK_SET; break;
		case HERO_FILE_SEEK_FROM_CURRENT: whence = SEEK_CUR; break;
		case HERO_FILE_SEEK_FROM_END: whence = SEEK_END; break;
	}
	off_t cursor_offset = lseek(handle.raw, offset, whence);
	if (cursor_offset == (off_t)-1)
		return _hero_os_get_last_error();
	if (cursor_offset_out) {
		*cursor_offset_out = cursor_offset;
	}
#elif _WIN32
	int move_method = 0;
	switch (from) {
		case HERO_FILE_SEEK_FROM_START: move_method = FILE_BEGIN; break;
		case HERO_FILE_SEEK_FROM_CURRENT: move_method = FILE_CURRENT; break;
		case HERO_FILE_SEEK_FROM_END: move_method = FILE_END; break;
	}
	LARGE_INTEGER distance_to_move;
	distance_to_move.QuadPart = offset;
	LARGE_INTEGER new_offset = {0};

	if (!SetFilePointerEx(handle.raw, distance_to_move, &new_offset, move_method))
		return _hero_os_get_last_error();

	if (cursor_offset_out) {
		*cursor_offset_out = new_offset.QuadPart;
	}
#else
#error "unimplemented file API for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_file_flush(HeroFileHandle handle) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	if (fsync(handle.raw) != 0)
		return _hero_os_get_last_error();
#elif _WIN32
	if (FlushFileBuffers(handle.raw) == 0)
		return _hero_os_get_last_error();
#else
#error "unimplemented file API for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_file_rename(const char* dst_path, const char* src_path) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	if (rename(src_path, dst_path) != 0) {
		return _hero_os_get_last_error();
	}
#else
#error "unimplemented file API for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_file_safe_full_write_start(HeroFileHandle* handle_mut, const char* path, char** tmp_path_out) {
	HeroResult result;
	if (handle_mut->raw != HERO_FILE_HANDLE_NULL.raw) {
		result = hero_file_close(*handle_mut);
		HERO_RESULT_ASSERT(result);
	}

	// TODO use a temporary allocator
	HeroString tmp_path;
	hero_string_fmt(&tmp_path, hero_system_alctor, HERO_ALLOC_TAG_FILE_SAFE_FULL_WRITE_PATH, "%s.tmp", path);
	*tmp_path_out = (char*)tmp_path.data;

	return hero_file_open((char*)tmp_path.data, HERO_FILE_FLAGS_WRITE | HERO_FILE_FLAGS_CREATE_NEW, handle_mut);
}

HeroResult hero_file_safe_full_write_end(HeroFileHandle* handle_mut, const char* path, char* tmp_path) {
	HeroResult result = hero_file_flush(*handle_mut);
	HERO_RESULT_ASSERT(result);

	result = hero_file_close(*handle_mut);
	HERO_RESULT_ASSERT(result);

	result = hero_file_rename(path, tmp_path);
	HERO_RESULT_ASSERT(result);

	return hero_file_open(path, HERO_FILE_FLAGS_READ, handle_mut);
}

HeroResult hero_file_path_canonicalize(const char* path, HeroAllocTag tag, HeroString* path_out) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	char* rpath = realpath(path, NULL);
	if (rpath == NULL) {
		return _hero_os_get_last_error();
	}
	path_out->data = (U8*)rpath;
	path_out->size = strlen(rpath);
#else
#error "unimplemented file API for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_dir_make(const char* path) {
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	mode_t mode = 0700; // TODO allow the user to supply a mode.
	if (mkdir(path, mode) != 0) {
		return _hero_os_get_last_error();
	}
#else
#error "unimplemented file API for this platform"
#endif
	return HERO_SUCCESS;
}

HeroResult hero_file_read_all(const char* path, HeroIAlctor alctor, HeroAllocTag tag, U8** bytes_out, Uptr* file_size_out) {
	HeroFileHandle file_handle;
    HeroResult result = hero_file_open(path, HERO_FILE_FLAGS_READ, &file_handle);
    if (result < 0) { return result; }

	U64 file_size;
	result = hero_file_size(file_handle, &file_size);
    if (result < 0) { goto ERR; }

	U8* bytes = hero_alloc(alctor, tag, file_size, 1);
	if (bytes == NULL) {
		result = HERO_ERROR_ALLOCATION_FAILURE;
		goto ERR;
	}

	Uptr bytes_read;
	result = hero_file_read_exact(file_handle, bytes, file_size, &bytes_read);
    if (result < 0) { goto ERR; }

	hero_file_close(file_handle);

	*bytes_out = bytes;
	*file_size_out = file_size;
    return HERO_SUCCESS;

ERR: {}
	hero_file_close(file_handle);
    return result;
}

// ===========================================
//
//
// Free Range
//
//
// ===========================================

HeroResult hero_free_ranges_init(HeroFreeRanges* r, Uptr init_cap, HeroIAlctor alctor, HeroAllocTag tag) {
	hero_stack(HeroRange, resize_cap)(&r->ranges, init_cap, alctor, tag);
	return HERO_SUCCESS;
}

void hero_free_ranges_deinit(HeroFreeRanges* r, HeroIAlctor alctor, HeroAllocTag tag) {
	hero_stack(HeroRange, deinit)(&r->ranges, alctor, tag);
}

HeroResult hero_free_ranges_give(HeroFreeRanges* r, Uptr idx, HeroIAlctor alctor, HeroAllocTag tag) {
	bool have_inserted = false;
	Uptr range_idx = 0;

	if (r->ranges.count == 0) {
		goto INSERT;
	}

	HeroRange* range;
	for (range_idx = 0; range_idx < r->ranges.count; range_idx += 1) {
		range = &r->ranges.data[range_idx];

		if (range->start_idx == idx + 1) {
			range->start_idx -= 1;
			goto INSERT_INTO_EXISTING;
		}

		if (range->end_idx == idx) {
			range->end_idx += 1;
			goto INSERT_INTO_EXISTING;
		}

		if (idx < range->start_idx) {
			goto INSERT;
		}

		HERO_ASSERT(range->end_idx < idx, "index '%zu' has already been freed", idx);
	}

	return HERO_SUCCESS;

INSERT_INTO_EXISTING: {}
	//
	// we only have to merge with next on inserts since the
	// ranges are ordered and the end index of a range is checked before the next range's start index.
	Uptr next_range_idx = range_idx + 1;
	if (next_range_idx < r->ranges.count) {
		HeroRange* next_range = &r->ranges.data[next_range_idx];
		if (next_range->start_idx == range->end_idx) {
			next_range->start_idx = range->start_idx;
			hero_stack(HeroRange, remove_shift)(&r->ranges, range_idx);
			return HERO_SUCCESS;
		}
	}

	return HERO_SUCCESS;

INSERT: {}
	HeroRange insert_range = { .start_idx = idx, .end_idx = idx + 1 };
	return hero_stack(HeroRange, insert_value)(&r->ranges, range_idx, alctor, tag, insert_range);
}

HeroResult hero_free_ranges_take(HeroFreeRanges* r, Uptr* idx_out) {
	if (r->ranges.count == 0) {
		return HERO_ERROR(ALLOCATION_FAILURE);
	}

	HeroRange* first_range = &r->ranges.data[0];
	*idx_out = first_range->start_idx;
	first_range->start_idx += 1;

	if (first_range->start_idx == first_range->end_idx) {
		hero_stack(HeroRange, remove_shift)(&r->ranges, 0);
	}

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Time Utilities
//
//
// ===========================================

HeroTime hero_time_now(HeroTimeMode mode) {
	int m;
	switch (mode) {
		case HERO_TIME_MODE_REALTIME: m = CLOCK_REALTIME; break;
		case HERO_TIME_MODE_MONOTONIC: m = CLOCK_MONOTONIC; break;
	}

	struct timespec start;
	clock_gettime(m, &start);
	return (HeroTime) { .secs = start.tv_sec, .nanosecs = start.tv_nsec };
}

HeroDuration hero_time_elapsed(HeroTime time, HeroTimeMode mode) {
	int m;
	switch (mode) {
		case HERO_TIME_MODE_REALTIME: m = CLOCK_REALTIME; break;
		case HERO_TIME_MODE_MONOTONIC: m = CLOCK_MONOTONIC; break;
	}

	struct timespec elapsed, now;
	clock_gettime(m, &now);

	if (now.tv_nsec < time.nanosecs) {
		elapsed.tv_sec = now.tv_sec - time.secs - 1;
		elapsed.tv_nsec = 1000000000 + now.tv_nsec - time.nanosecs;
	} else {
		elapsed.tv_sec = now.tv_sec - time.secs;
		elapsed.tv_nsec = now.tv_nsec - time.nanosecs;
	}

	return (HeroDuration) { .secs = elapsed.tv_sec, .nanosecs = elapsed.tv_nsec };
}

HeroDuration hero_time_diff(HeroTime to, HeroTime from) {
	HERO_ASSERT(to.secs > from.secs || (to.secs == from.secs && to.nanosecs > from.nanosecs), "'to' time must come after 'from' time");
	HeroDuration d = {0};
	if (to.nanosecs < from.nanosecs) {
		d.secs = to.secs - from.secs - 1;
		d.nanosecs = 1000000000 + to.nanosecs - from.nanosecs;
	} else {
		d.secs = to.secs - from.secs;
		d.nanosecs = to.nanosecs - from.nanosecs;
	}

	return d;
}

bool hero_duration_has_secs(HeroDuration duration) {
	return duration.secs > 0;
}

bool hero_duration_has_millisecs(HeroDuration duration) {
	return duration.secs > 0 || duration.nanosecs >= 1000000;
}

bool hero_duration_has_microsecs(HeroDuration duration) {
	return duration.secs > 0 || duration.nanosecs >= 1000;
}

bool hero_duration_has_nanosecs(HeroDuration duration) {
	return duration.secs > 0 || duration.nanosecs > 0;
}

F32 hero_duration_to_f32_secs(HeroDuration duration) {
    return (F32)duration.secs + 1.0e-9 * (F32)duration.nanosecs;
}

F64 hero_duration_to_f64_secs(HeroDuration duration) {
    return (F64)duration.secs + 1.0e-9 * (F64)duration.nanosecs;
}

F32 hero_duration_to_f32_millisecs(HeroDuration duration) {
    return (F32)1.0e-3 * (F32)duration.secs + (F32)1.0e-6 * (F32)duration.nanosecs;
}

F64 hero_duration_to_f64_millisecs(HeroDuration duration) {
    return (F64)1.0e-3 * (F64)duration.secs + (F64)1.0e-6 * (F64)duration.nanosecs;
}

F32 hero_duration_to_f32_microsecs(HeroDuration duration) {
    return (F32)1.0e-6 * (F32)duration.secs + (F32)1.0e-3 * (F32)duration.nanosecs;
}

F64 hero_duration_to_f64_microsecs(HeroDuration duration) {
    return (F64)1.0e-6 * (F64)duration.secs + (F64)1.0e-3 * (F64)duration.nanosecs;
}

U64 hero_duration_secs(HeroDuration duration) {
	return duration.secs;
}

U64 hero_duration_millisecs(HeroDuration duration) {
	return (duration.secs * 1000) + (duration.nanosecs / 1000000);
}

U64 hero_duration_microsecs(HeroDuration duration) {
	return (duration.secs * 1000000) + (duration.nanosecs / 1000);
}

U64 hero_duration_nanosecs(HeroDuration duration) {
	return (duration.secs * 1000000000) + duration.nanosecs;
}

U64 hero_duration_frame_to_fps(HeroDuration duration) {
	return 1000000000 / hero_duration_nanosecs(duration);
}

HeroDuration hero_duration_add(HeroDuration a, HeroDuration b) {
	HeroDuration res;
	res.secs = a.secs + b.secs;
	res.nanosecs = a.nanosecs + b.nanosecs;
	if (res.nanosecs >= 1000000000) {
		res.secs += 1;
		res.nanosecs -= 1000000000;
	}
	return res;
}

HeroDuration hero_duration_sub(HeroDuration a, HeroDuration b) {
	HeroDuration res;
	if (a.secs < b.secs) {
		res.secs = 0;
		res.nanosecs = 0;
		return res;
	}

	res.secs = a.secs - b.secs;

	if (a.nanosecs < b.nanosecs) {
		res.secs -= 1;
		res.nanosecs = 1000000000 - (b.nanosecs - a.nanosecs);
	} else {
		res.nanosecs = a.nanosecs - b.nanosecs;
	}
	return res;
}

bool hero_duration_lt(HeroDuration a, HeroDuration b) {
	if (a.secs == b.secs) {
		return a.nanosecs < b.nanosecs;
	} else {
		return a.secs < b.secs;
	}
}

bool hero_duration_gt(HeroDuration a, HeroDuration b) {
	if (a.secs == b.secs) {
		return a.nanosecs > b.nanosecs;
	} else {
		return a.secs > b.secs;
	}
}

// ===========================================
//
//
// Text Reader
//
//
// ===========================================

void hero_text_reader_init(HeroTextReader* reader, U8* string, U32 size) {
	reader->string = string;
	reader->size = size;
	reader->position = 0;
	reader->row = 1;
	reader->column = 1;
}

U32 hero_text_reader_remaining_size(HeroTextReader* reader) {
	return reader->size - reader->position;
}

bool hero_text_reader_has_content(HeroTextReader* reader) {
	return reader->position < reader->size;
}

bool hero_text_reader_is_empty(HeroTextReader* reader) {
	return reader->position >= reader->size;
}

U32 hero_text_reader_consume_whitespace(HeroTextReader* reader) {
	U32 i;
	for (i = 0; reader->position < reader->size; i += 1) {
		U8 rbyte = reader->string[reader->position];
		if (rbyte != ' ' && rbyte != '\t') break;

		reader->position += 1;
		if (rbyte == '\n') {
			reader->row += 1;
			reader->column = 1;
		} else {
			reader->column += 1;
		}
	}
	return i;
}

U32 hero_text_reader_consume_whitespace_and_newlines(HeroTextReader* reader) {
	U32 i;
	for (i = 0; reader->position < reader->size; i += 1) {
		U8 rbyte = reader->string[reader->position];
		if (rbyte != ' ' && rbyte != '\t' && rbyte != '\n' && rbyte != '\r') break;

		reader->position += 1;
		if (rbyte == '\n') {
			reader->row += 1;
			reader->column = 1;
		} else {
			reader->column += 1;
		}
	}
	return i;
}

U32 hero_text_reader_consume_until_any_byte(HeroTextReader* reader, HeroString bytes) {
	U32 position = reader->position;
	U32 row = reader->row;
	U32 column = reader->column;
	while (position < reader->size) {
		U8 rbyte = reader->string[position];
		for (U32 j = 0; j < bytes.size; j += 1) {
			if (rbyte == bytes.data[j]) {
				U32 read_size = position - reader->position;
				reader->position = position;
				reader->row = row;
				reader->column = column;
				return read_size;
			}
		}

		position += 1;
		if (rbyte == '\n') {
			row += 1;
			column = 1;
		} else {
			column += 1;
		}
	}

	return 0;
}

bool hero_text_reader_consume_byte(HeroTextReader* reader, U8 byte) {
	if (reader->position >= reader->size) {
		return false;
	}

	U8 rbyte = reader->string[reader->position];
	if (rbyte != byte) {
		return false;
	}

	reader->position += 1;
	if (rbyte == '\n') {
		reader->row += 1;
		reader->column = 1;
	} else {
		reader->column += 1;
	}

	return true;
}

// ===========================================
//
//
// Input
//
//
// ===========================================

const char* HeroKeyCode_strings[HERO_KEY_CODE_COUNT] = {
	[HERO_KEY_CODE_NULL] = "NULL",

	[HERO_KEY_CODE_A] = "A",
	[HERO_KEY_CODE_B] = "B",
	[HERO_KEY_CODE_C] = "C",
	[HERO_KEY_CODE_D] = "D",
	[HERO_KEY_CODE_E] = "E",
	[HERO_KEY_CODE_F] = "F",
	[HERO_KEY_CODE_G] = "G",
	[HERO_KEY_CODE_H] = "H",
	[HERO_KEY_CODE_I] = "I",
	[HERO_KEY_CODE_J] = "J",
	[HERO_KEY_CODE_K] = "K",
	[HERO_KEY_CODE_L] = "L",
	[HERO_KEY_CODE_M] = "M",
	[HERO_KEY_CODE_N] = "N",
	[HERO_KEY_CODE_O] = "O",
	[HERO_KEY_CODE_P] = "P",
	[HERO_KEY_CODE_Q] = "Q",
	[HERO_KEY_CODE_R] = "R",
	[HERO_KEY_CODE_S] = "S",
	[HERO_KEY_CODE_T] = "T",
	[HERO_KEY_CODE_U] = "U",
	[HERO_KEY_CODE_V] = "V",
	[HERO_KEY_CODE_W] = "W",
	[HERO_KEY_CODE_X] = "X",
	[HERO_KEY_CODE_Y] = "Y",
	[HERO_KEY_CODE_Z] = "Z",

	[HERO_KEY_CODE_1] = "1",
	[HERO_KEY_CODE_2] = "2",
	[HERO_KEY_CODE_3] = "3",
	[HERO_KEY_CODE_4] = "4",
	[HERO_KEY_CODE_5] = "5",
	[HERO_KEY_CODE_6] = "6",
	[HERO_KEY_CODE_7] = "7",
	[HERO_KEY_CODE_8] = "8",
	[HERO_KEY_CODE_9] = "9",
	[HERO_KEY_CODE_0] = "0",

	[HERO_KEY_CODE_ENTER] = "ENTER",
	[HERO_KEY_CODE_ESCAPE] = "ESCAPE",
	[HERO_KEY_CODE_BACKSPACE] = "BACKSPACE",
	[HERO_KEY_CODE_TAB] = "TAB",
	[HERO_KEY_CODE_SPACE] = "SPACE",

	[HERO_KEY_CODE_MINUS] = "MINUS",
	[HERO_KEY_CODE_EQUALS] = "EQUALS",
	[HERO_KEY_CODE_LEFT_BRACKET] = "LEFT_BRACKET",
	[HERO_KEY_CODE_RIGHT_BRACKET] = "RIGHT_BRACKET",
	[HERO_KEY_CODE_BACKSLASH] = "BACKSLASH",
	[HERO_KEY_CODE_NON_US_HASH] = "NON_US_HASH",
	[HERO_KEY_CODE_SEMICOLON] = "SEMICOLON",
	[HERO_KEY_CODE_APOSTROPHE] = "APOSTROPHE",
	[HERO_KEY_CODE_GHERO] = "GHERO",
	[HERO_KEY_CODE_COMMA] = "COMMA",
	[HERO_KEY_CODE_FULL_STOP] = "FULL_STOP",
	[HERO_KEY_CODE_FORWARDSLASH] = "FORWARDSLASH",
	[HERO_KEY_CODE_CAPS_LOCK] = "CAPS_LOCK",

	[HERO_KEY_CODE_F1] = "F1",
	[HERO_KEY_CODE_F2] = "F2",
	[HERO_KEY_CODE_F3] = "F3",
	[HERO_KEY_CODE_F4] = "F4",
	[HERO_KEY_CODE_F5] = "F5",
	[HERO_KEY_CODE_F6] = "F6",
	[HERO_KEY_CODE_F7] = "F7",
	[HERO_KEY_CODE_F8] = "F8",
	[HERO_KEY_CODE_F9] = "F9",
	[HERO_KEY_CODE_F10] = "F10",
	[HERO_KEY_CODE_F11] = "F11",
	[HERO_KEY_CODE_F12] = "F12",

	[HERO_KEY_CODE_PRINT_SCREEN] = "PRINT_SCREEN",
	[HERO_KEY_CODE_SCROLL_LOCK] = "SCROLL_LOCK",
	[HERO_KEY_CODE_PAUSE] = "PAUSE",
	[HERO_KEY_CODE_INSERT] = "INSERT",
	[HERO_KEY_CODE_HOME] = "HOME",
	[HERO_KEY_CODE_PAGE_UP] = "PAGE_UP",
	[HERO_KEY_CODE_DELETE] = "DELETE",
	[HERO_KEY_CODE_END] = "END",
	[HERO_KEY_CODE_PAGE_DOWN] = "PAGE_DOWN",
	[HERO_KEY_CODE_RIGHT] = "RIGHT",
	[HERO_KEY_CODE_LEFT] = "LEFT",
	[HERO_KEY_CODE_DOWN] = "DOWN",
	[HERO_KEY_CODE_UP] = "UP",

	[HERO_KEY_CODE_NUM_LOCK_OR_CLEAR] = "NUM_LOCK_OR_CLEAR",
	[HERO_KEY_CODE_KP_DIVIDE] = "KP_DIVIDE",
	[HERO_KEY_CODE_KP_MULTIPLY] = "KP_MULTIPLY",
	[HERO_KEY_CODE_KP_MINUS] = "KP_MINUS",
	[HERO_KEY_CODE_KP_PLUS] = "KP_PLUS",
	[HERO_KEY_CODE_KP_ENTER] = "KP_ENTER",
	[HERO_KEY_CODE_KP_1] = "KP_1",
	[HERO_KEY_CODE_KP_2] = "KP_2",
	[HERO_KEY_CODE_KP_3] = "KP_3",
	[HERO_KEY_CODE_KP_4] = "KP_4",
	[HERO_KEY_CODE_KP_5] = "KP_5",
	[HERO_KEY_CODE_KP_6] = "KP_6",
	[HERO_KEY_CODE_KP_7] = "KP_7",
	[HERO_KEY_CODE_KP_8] = "KP_8",
	[HERO_KEY_CODE_KP_9] = "KP_9",
	[HERO_KEY_CODE_KP_0] = "KP_0",
	[HERO_KEY_CODE_KP_FULL_STOP] = "KP_FULL_STOP",

	[HERO_KEY_CODE_NON_US_BACKSLASH] = "NON_US_BACKSLASH",
	[HERO_KEY_CODE_APPLICATION] = "APPLICATION",
	[HERO_KEY_CODE_POWER] = "POWER",
	[HERO_KEY_CODE_KP_EQUALS] = "KP_EQUALS",
	[HERO_KEY_CODE_F13] = "F13",
	[HERO_KEY_CODE_F14] = "F14",
	[HERO_KEY_CODE_F15] = "F15",
	[HERO_KEY_CODE_F16] = "F16",
	[HERO_KEY_CODE_F17] = "F17",
	[HERO_KEY_CODE_F18] = "F18",
	[HERO_KEY_CODE_F19] = "F19",
	[HERO_KEY_CODE_F20] = "F20",
	[HERO_KEY_CODE_F21] = "F21",
	[HERO_KEY_CODE_F22] = "F22",
	[HERO_KEY_CODE_F23] = "F23",
	[HERO_KEY_CODE_F24] = "F24",
	[HERO_KEY_CODE_EXECUTE] = "EXECUTE",
	[HERO_KEY_CODE_HELP] = "HELP",
	[HERO_KEY_CODE_MENU] = "MENU",
	[HERO_KEY_CODE_SELECT] = "SELECT",
	[HERO_KEY_CODE_STOP] = "STOP",
	[HERO_KEY_CODE_AGAIN] = "AGAIN",
	[HERO_KEY_CODE_UNDO] = "UNDO",
	[HERO_KEY_CODE_CUT] = "CUT",
	[HERO_KEY_CODE_COPY] = "COPY",
	[HERO_KEY_CODE_PASTE] = "PASTE",
	[HERO_KEY_CODE_FIND] = "FIND",
	[HERO_KEY_CODE_MUTE] = "MUTE",
	[HERO_KEY_CODE_VOLUME_UP] = "VOLUME_UP",
	[HERO_KEY_CODE_VOLUME_DOWN] = "VOLUME_DOWN",

	[HERO_KEY_CODE_KP_COMMA] = "KP_COMMA",
	[HERO_KEY_CODE_KP_EQUALSAS400] = "KP_EQUALSAS400",

	[HERO_KEY_CODE_INTERNATIONAL_1] = "INTERNATIONAL_1",
	[HERO_KEY_CODE_INTERNATIONAL_2] = "INTERNATIONAL_2",
	[HERO_KEY_CODE_INTERNATIONAL_3] = "INTERNATIONAL_3",
	[HERO_KEY_CODE_INTERNATIONAL_4] = "INTERNATIONAL_4",
	[HERO_KEY_CODE_INTERNATIONAL_5] = "INTERNATIONAL_5",
	[HERO_KEY_CODE_INTERNATIONAL_6] = "INTERNATIONAL_6",
	[HERO_KEY_CODE_INTERNATIONAL_7] = "INTERNATIONAL_7",
	[HERO_KEY_CODE_INTERNATIONAL_8] = "INTERNATIONAL_8",
	[HERO_KEY_CODE_INTERNATIONAL_9] = "INTERNATIONAL_9",
	[HERO_KEY_CODE_LANG_1] = "LANG_1",
	[HERO_KEY_CODE_LANG_2] = "LANG_2",
	[HERO_KEY_CODE_LANG_3] = "LANG_3",
	[HERO_KEY_CODE_LANG_4] = "LANG_4",
	[HERO_KEY_CODE_LANG_5] = "LANG_5",
	[HERO_KEY_CODE_LANG_6] = "LANG_6",
	[HERO_KEY_CODE_LANG_7] = "LANG_7",
	[HERO_KEY_CODE_LANG_8] = "LANG_8",
	[HERO_KEY_CODE_LANG_9] = "LANG_9",

	[HERO_KEY_CODE_ALT_ERASE] = "ALT_ERASE",
	[HERO_KEY_CODE_SYS_REQ] = "SYS_REQ",
	[HERO_KEY_CODE_CANCEL] = "CANCEL",
	[HERO_KEY_CODE_CLEAR] = "CLEAR",
	[HERO_KEY_CODE_PRIOR] = "PRIOR",
	[HERO_KEY_CODE_ENTER_2] = "ENTER_2",
	[HERO_KEY_CODE_SEPARATOR] = "SEPARATOR",
	[HERO_KEY_CODE_OUT] = "OUT",
	[HERO_KEY_CODE_OPER] = "OPER",
	[HERO_KEY_CODE_CLEARAGAIN] = "CLEARAGAIN",
	[HERO_KEY_CODE_CRSEL] = "CRSEL",
	[HERO_KEY_CODE_EXSEL] = "EXSEL",

	[HERO_KEY_CODE_KP_00] = "KP_00",
	[HERO_KEY_CODE_KP_000] = "KP_000",
	[HERO_KEY_CODE_THOUSANDSSEPARATOR] = "THOUSANDSSEPARATOR",
	[HERO_KEY_CODE_DECIMALSEPARATOR] = "DECIMALSEPARATOR",
	[HERO_KEY_CODE_CURRENCYUNIT] = "CURRENCYUNIT",
	[HERO_KEY_CODE_CURRENCYSUBUNIT] = "CURRENCYSUBUNIT",
	[HERO_KEY_CODE_KP_LEFT_PAREN] = "KP_LEFT_PAREN",
	[HERO_KEY_CODE_KP_RIGHT_PAREN] = "KP_RIGHT_PAREN",
	[HERO_KEY_CODE_KP_LEFT_BRACE] = "KP_LEFT_BRACE",
	[HERO_KEY_CODE_KP_RIGHT_BRACE] = "KP_RIGHT_BRACE",
	[HERO_KEY_CODE_KP_TAB] = "KP_TAB",
	[HERO_KEY_CODE_KP_BACKSPACE] = "KP_BACKSPACE",
	[HERO_KEY_CODE_KP_A] = "KP_A",
	[HERO_KEY_CODE_KP_B] = "KP_B",
	[HERO_KEY_CODE_KP_C] = "KP_C",
	[HERO_KEY_CODE_KP_D] = "KP_D",
	[HERO_KEY_CODE_KP_E] = "KP_E",
	[HERO_KEY_CODE_KP_F] = "KP_F",
	[HERO_KEY_CODE_KP_XOR] = "KP_XOR",
	[HERO_KEY_CODE_KP_POWER] = "KP_POWER",
	[HERO_KEY_CODE_KP_PERCENT] = "KP_PERCENT",
	[HERO_KEY_CODE_KP_LESS] = "KP_LESS",
	[HERO_KEY_CODE_KP_GREATER] = "KP_GREATER",
	[HERO_KEY_CODE_KP_AMPERSAND] = "KP_AMPERSAND",
	[HERO_KEY_CODE_KP_DBLAMPERSAND] = "KP_DBLAMPERSAND",
	[HERO_KEY_CODE_KP_VERTICALBAR] = "KP_VERTICALBAR",
	[HERO_KEY_CODE_KP_DBLVERTICALBAR] = "KP_DBLVERTICALBAR",
	[HERO_KEY_CODE_KP_COLON] = "KP_COLON",
	[HERO_KEY_CODE_KP_HASH] = "KP_HASH",
	[HERO_KEY_CODE_KP_SPACE] = "KP_SPACE",
	[HERO_KEY_CODE_KP_AT] = "KP_AT",
	[HERO_KEY_CODE_KP_EXCLAM] = "KP_EXCLAM",
	[HERO_KEY_CODE_KP_MEMSTORE] = "KP_MEMSTORE",
	[HERO_KEY_CODE_KP_MEMRECALL] = "KP_MEMRECALL",
	[HERO_KEY_CODE_KP_MEMCLEAR] = "KP_MEMCLEAR",
	[HERO_KEY_CODE_KP_MEMADD] = "KP_MEMADD",
	[HERO_KEY_CODE_KP_MEMSUBTRACT] = "KP_MEMSUBTRACT",
	[HERO_KEY_CODE_KP_MEMMULTIPLY] = "KP_MEMMULTIPLY",
	[HERO_KEY_CODE_KP_MEMDIVIDE] = "KP_MEMDIVIDE",
	[HERO_KEY_CODE_KP_PLUS_MINUS] = "KP_PLUS_MINUS",
	[HERO_KEY_CODE_KP_CLEAR] = "KP_CLEAR",
	[HERO_KEY_CODE_KP_CLEARENTRY] = "KP_CLEARENTRY",
	[HERO_KEY_CODE_KP_BINARY] = "KP_BINARY",
	[HERO_KEY_CODE_KP_OCTAL] = "KP_OCTAL",
	[HERO_KEY_CODE_KP_DECIMAL] = "KP_DECIMAL",
	[HERO_KEY_CODE_KP_HEXADECIMAL] = "KP_HEXADECIMAL",

	[HERO_KEY_CODE_LEFT_CTRL] = "LEFT_CTRL",
	[HERO_KEY_CODE_LEFT_SHIFT] = "LEFT_SHIFT",
	[HERO_KEY_CODE_LEFT_ALT] = "LEFT_ALT",
	[HERO_KEY_CODE_LEFT_META] = "LEFT_META",
	[HERO_KEY_CODE_RIGHT_CTRL] = "RIGHT_CTRL",
	[HERO_KEY_CODE_RIGHT_SHIFT] = "RIGHT_SHIFT",
	[HERO_KEY_CODE_RIGHT_ALT] = "RIGHT_ALT",
	[HERO_KEY_CODE_RIGHT_META] = "RIGHT_META",
	[HERO_KEY_CODE_AUDIO_NEXT] = "AUDIO_NEXT",
	[HERO_KEY_CODE_AUDIO_PREV] = "AUDIO_PREV",
	[HERO_KEY_CODE_AUDIO_STOP] = "AUDIO_STOP",
	[HERO_KEY_CODE_AUDIO_PLAY] = "AUDIO_PLAY",
	[HERO_KEY_CODE_AUDIO_MUTE] = "AUDIO_MUTE",
	[HERO_KEY_CODE_MEDIASELECT] = "MEDIASELECT",
	[HERO_KEY_CODE_WWW] = "WWW",
	[HERO_KEY_CODE_MAIL] = "MAIL",
	[HERO_KEY_CODE_CALCULATOR] = "CALCULATOR",
	[HERO_KEY_CODE_COMPUTER] = "COMPUTER",
	[HERO_KEY_CODE_AC_SEARCH] = "AC_SEARCH",
	[HERO_KEY_CODE_AC_HOME] = "AC_HOME",
	[HERO_KEY_CODE_AC_BACK] = "AC_BACK",
	[HERO_KEY_CODE_AC_FORWARD] = "AC_FORWARD",
	[HERO_KEY_CODE_AC_STOP] = "AC_STOP",
	[HERO_KEY_CODE_AC_REFRESH] = "AC_REFRESH",
	[HERO_KEY_CODE_AC_BOOKMARKS] = "AC_BOOKMARKS",

	[HERO_KEY_CODE_BRIGHTNESS_DOWN] = "BRIGHTNESS_DOWN",
	[HERO_KEY_CODE_BRIGHTNESS_UP] = "BRIGHTNESS_UP",
	[HERO_KEY_CODE_DISPLAY_SWITCH] = "DISPLAY_SWITCH",
	[HERO_KEY_CODE_KBD_ILLUM_TOGGLE] = "KBD_ILLUM_TOGGLE",
	[HERO_KEY_CODE_KBD_ILLUM_DOWN] = "KBD_ILLUM_DOWN",
	[HERO_KEY_CODE_KBD_ILLUM_UP] = "KBD_ILLUM_UP",
	[HERO_KEY_CODE_EJECT] = "EJECT",
	[HERO_KEY_CODE_SLEEP] = "SLEEP",

	[HERO_KEY_CODE_APP_1] = "APP_1",
	[HERO_KEY_CODE_APP_2] = "APP_2",

	[HERO_KEY_CODE_AUDIO_REWIND] = "AUDIO_REWIND",
	[HERO_KEY_CODE_AUDIO_FASTFORWARD] = "AUDIO_FASTFORWARD",

};

// ===========================================
//
//
// Float16
//
//
// ===========================================

typedef union _cgm_F32_uint _cgm_F32_uint;
union _cgm_F32_uint {
	F32 f;
	U32 u;
};

F32 hero_f16_to_f32(F16 v) {
	if ((v.bits & 0x7c00) == 0x7c00) { // inf, -inf or nan
		if (v.bits & 0x03ff) return NAN;
		else if (v.bits & 0x8000) return -INFINITY;
		else return INFINITY;
	}

	_cgm_F32_uint t1;
	U32 t2;
	U32 t3;

	t1.u = v.bits & 0x7fff;      // non-sign bits
	t2 = v.bits & 0x8000;        // sign bit
	t3 = v.bits & 0x7c00;        // exponent

	t1.u <<= 13;                 // align mantissa on MSB
	t2 <<= 16;                   // shift sign bit into position

	t1.u += 0x38000000;          // adjust bias

	t1.u = (t3 == 0 ? 0 : t1.u); // denormals-as-zero

	t1.u |= t2;                  // re-insert sign bit

	return t1.f;
}

F16 hero_f16_from_f32(F32 v) {
	if (isinf(v)) return (F16) { .bits = v < 0.0 ? 0xfc00 : 0x7c00 };
	if (isnan(v)) return (F16) { .bits = 0xffff };

	_cgm_F32_uint vu = { .f = v };
	U32 t1;
	U32 t2;
	U32 t3;

	t1 = vu.u & 0x7fffffff;                // non-sign bits
	t2 = vu.u & 0x80000000;                // sign bit
	t3 = vu.u & 0x7f800000;                // exponent

	t1 >>= 13;                             // align mantissa on MSB
	t2 >>= 16;                             // shift sign bit into position

	t1 -= 0x1c000;                         // adjust bias

	t1 = (t3 < 0x38800000) ? 0 : t1;       // flush-to-zero
	t1 = (t3 > 0x8e000000) ? 0x7bff : t1;  // clamp-to-max
	t1 = (t3 == 0 ? 0 : t1);               // denormals-as-zero

	t1 |= t2;                              // re-insert sign bit

	return (F16) { .bits = t1 };
}

bool hero_f16_is_nan(F16 v) {
	return (v.bits & 0x7c00) == 0x7c00 && v.bits & 0x03ff;
}

bool hero_f16_is_inf(F16 v) {
	return (v.bits & 0x7c00) == 0x7c00 && (v.bits & 0x03ff) == 0;

}

// ===========================================
//
//
// Vectors
//
//
// ===========================================

bool vec2_eq(Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }
Vec2 vec2_copysign(Vec2 v, Vec2 copy) { return VEC2_INIT(copysignf(v.x, copy.x), copysignf(v.y, copy.y)); }

Vec2 vec2_add(Vec2 a, Vec2 b) { return VEC2_INIT(a.x + b.x, a.y + b.y); }
Vec2 vec2_sub(Vec2 a, Vec2 b) { return VEC2_INIT(a.x - b.x, a.y - b.y); }
Vec2 vec2_mul(Vec2 a, Vec2 b) { return VEC2_INIT(a.x * b.x, a.y * b.y); }
Vec2 vec2_div(Vec2 a, Vec2 b) { return VEC2_INIT(a.x / b.x, a.y / b.y); }
Vec2 vec2_add_scalar(Vec2 v, F32 by) { return VEC2_INIT(v.x + by, v.y + by); }
Vec2 vec2_sub_scalar(Vec2 v, F32 by) { return VEC2_INIT(v.x - by, v.y - by); }
Vec2 vec2_mul_scalar(Vec2 v, F32 by) { return VEC2_INIT(v.x * by, v.y * by); }
Vec2 vec2_div_scalar(Vec2 v, F32 by) { return VEC2_INIT(v.x / by, v.y / by); }

Vec2 vec2_neg(Vec2 v) { return VEC2_INIT(-v.x, -v.y); }
F32 vec2_len(Vec2 v) { return sqrtf((v.x * v.x) + (v.y * v.y)); }
Vec2 vec2_norm(Vec2 v) {
	if (v.x == 0 && v.y == 0) return v;
	F32 k = 1.0 / sqrtf((v.x * v.x) + (v.y * v.y));
	return VEC2_INIT(v.x * k, v.y * k);
}

F32 vec2_dot(Vec2 a, Vec2 b) {
	F32 p = 0.0;
	p += a.x * b.x;
	p += a.y * b.y;
	return p;
}

F32 vec2_angle(Vec2 v) {
	return atan2f(-v.y, v.x);
}

Vec2 vec2_cross_scalar(Vec2 v, F32 s) {
	return VEC2_INIT(v.y * s, v.x * s);
}

F32 vec2_cross_vec(Vec2 a, Vec2 b) {
	return (a.x * b.y) - (a.y * b.x);
}

Vec2 vec2_rotate(Vec2 v, F32 angle) {
	F32 co = cosf(angle);
	F32 si = sinf(angle);
	return VEC2_INIT(
		(v.x * si) - (v.y * co),
		(v.x * co) + (v.y * si)
	);
}

Vec2 vec2_perp_left(Vec2 v) { return VEC2_INIT(v.y, -v.x); }
Vec2 vec2_perp_right(Vec2 v) { return VEC2_INIT(-v.y, v.x); }

Vec2 vec2_min(Vec2 a, Vec2 b) { return VEC2_INIT(HERO_MIN(a.x, b.x), HERO_MIN(a.y, b.y)); }
Vec2 vec2_max(Vec2 a, Vec2 b) { return VEC2_INIT(HERO_MAX(a.x, b.x), HERO_MAX(a.y, b.y)); }
Vec2 vec2_clamp(Vec2 v, Vec2 min, Vec2 max) {
	return VEC2_INIT(HERO_CLAMP(v.x, min.x, max.x), HERO_CLAMP(v.y, min.y, max.y));
}
Vec2 vec2_lerp_scalar(Vec2 from, Vec2 to, F32 t) {
	return VEC2_INIT(hero_lerp(from.x, to.x, t), hero_lerp(from.y, to.y, t));
}
Vec2 vec2_lerp_vec(Vec2 from, Vec2 to, Vec2 t) {
	return VEC2_INIT(hero_lerp(from.x, to.x, t.x), hero_lerp(from.y, to.y, t.y));
}
Vec2 vec2_sign(Vec2 v) {
	return VEC2_INIT(hero_sign(v.x), hero_sign(v.y));
}
Vec2 vec2_abs(Vec2 v) { return VEC2_INIT(fabs(v.x), fabs(v.y)); }
Vec2 vec2_floor(Vec2 v) { return VEC2_INIT(floorf(v.x), floorf(v.y)); }
Vec2 vec2_ceil(Vec2 v) { return VEC2_INIT(ceilf(v.x), ceilf(v.y)); }
Vec2 vec2_round(Vec2 v) { return VEC2_INIT(roundf(v.x), roundf(v.y)); }
bool vec2_approx_eq(Vec2 a, Vec2 b) { return hero_approx_eq(a.x, b.x) && hero_approx_eq(a.y, b.y); }

bool vec3_eq(Vec3 a, Vec3 b) { return a.x == b.x && a.y == b.y; }
Vec3 vec3_copysign(Vec3 v, Vec3 copy) { return VEC3_INIT(copysignf(v.x, copy.x), copysignf(v.y, copy.y), copysignf(v.z, copy.z)); }

Vec3 vec3_add(Vec3 a, Vec3 b) { return VEC3_INIT(a.x + b.x, a.y + b.y, a.z + b.z); }
Vec3 vec3_sub(Vec3 a, Vec3 b) { return VEC3_INIT(a.x - b.x, a.y - b.y, a.z - b.z); }
Vec3 vec3_mul(Vec3 a, Vec3 b) { return VEC3_INIT(a.x * b.x, a.y * b.y, a.z * b.z); }
Vec3 vec3_div(Vec3 a, Vec3 b) { return VEC3_INIT(a.x / b.x, a.y / b.y, a.z / b.z); }
Vec3 vec3_add_scalar(Vec3 v, F32 by) { return VEC3_INIT(v.x + by, v.y + by, v.z + by); }
Vec3 vec3_sub_scalar(Vec3 v, F32 by) { return VEC3_INIT(v.x - by, v.y - by, v.z - by); }
Vec3 vec3_mul_scalar(Vec3 v, F32 by) { return VEC3_INIT(v.x * by, v.y * by, v.z * by); }
Vec3 vec3_div_scalar(Vec3 v, F32 by) { return VEC3_INIT(v.x / by, v.y / by, v.z / by); }

Vec3 vec3_neg(Vec3 v) { return VEC3_INIT(-v.x, -v.y, -v.z); }
F32 vec3_len(Vec3 v) { return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z)); }
Vec3 vec3_norm(Vec3 v) {
	if (v.x == 0 && v.y == 0) return v;
	F32 k = 1.0 / sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return VEC3_INIT(v.x * k, v.y * k, v.z * k);
}

F32 vec3_dot(Vec3 a, Vec3 b) {
	F32 p = 0.0;
	p += a.x * b.x;
	p += a.y * b.y;
	p += a.z * b.z;
	return p;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
	return VEC3_INIT((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x));
}

Vec3 vec3_perp_y_forward(Vec3 v) { return VEC3_INIT(v.x, -v.z, v.y); }
Vec3 vec3_perp_y_backward(Vec3 v) { return VEC3_INIT(v.x, v.z, v.y); }
Vec3 vec3_perp_y_left(Vec3 v) { return VEC3_INIT(v.y, -v.x, v.z); }
Vec3 vec3_perp_y_right(Vec3 v) { return VEC3_INIT(-v.y, v.x, v.z); }
Vec3 vec3_perp_z_left(Vec3 v) { return VEC3_INIT(-v.z, v.y, v.x); }
Vec3 vec3_perp_z_right(Vec3 v) { return VEC3_INIT(v.z, v.y, -v.x); }

Vec3 vec3_min(Vec3 a, Vec3 b) { return VEC3_INIT(HERO_MIN(a.x, b.x), HERO_MIN(a.y, b.y), HERO_MIN(a.z, b.z)); }
Vec3 vec3_max(Vec3 a, Vec3 b) { return VEC3_INIT(HERO_MAX(a.x, b.x), HERO_MAX(a.y, b.y), HERO_MAX(a.z, b.z)); }
Vec3 vec3_clamp(Vec3 v, Vec3 min, Vec3 max) {
	return VEC3_INIT(HERO_CLAMP(v.x, min.x, max.x), HERO_CLAMP(v.y, min.y, max.y), HERO_CLAMP(v.z, min.z, max.z));
}
Vec3 vec3_lerp_scalar(Vec3 from, Vec3 to, F32 t) {
	return VEC3_INIT(hero_lerp(from.x, to.x, t), hero_lerp(from.y, to.y, t), hero_lerp(from.z, to.z, t));
}

Vec3 vec3_lerp_vec(Vec3 from, Vec3 to, Vec3 t) {
	return VEC3_INIT(hero_lerp(from.x, to.x, t.x), hero_lerp(from.y, to.y, t.y), hero_lerp(from.z, to.z, t.z));
}
Vec3 vec3_sign(Vec3 v) {
	return VEC3_INIT(hero_sign(v.x), hero_sign(v.y), hero_sign(v.z));
}

Vec3 vec3_abs(Vec3 v) { return VEC3_INIT(fabs(v.x), fabs(v.y), fabs(v.z)); }
Vec3 vec3_floor(Vec3 v) { return VEC3_INIT(floorf(v.x), floorf(v.y), floorf(v.z)); }
Vec3 vec3_ceil(Vec3 v) { return VEC3_INIT(ceilf(v.x), ceilf(v.y), ceilf(v.z)); }
Vec3 vec3_round(Vec3 v) { return VEC3_INIT(roundf(v.x), roundf(v.y), roundf(v.z)); }
bool vec3_approx_eq(Vec3 a, Vec3 b) { return hero_approx_eq(a.x, b.x) && hero_approx_eq(a.y, b.y) && hero_approx_eq(a.z, b.z); }

// ===========================================
//
//
// Quaternion
//
//
// ===========================================

Quat quat_norm(Quat q) {
	if (q.x == 0 && q.y == 0 && q.z == 0 && q.w == 0) return q;
	F32 k = 1.0 / sqrtf((q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w));
	return QUAT_INIT(q.x * k, q.y * k, q.z * k, q.w * k);
}

Quat quat_add(Quat a, Quat b) { return QUAT_INIT(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
Quat quat_sub(Quat a, Quat b) { return QUAT_INIT(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
Quat quat_add_scalar(Quat q, F32 by) { return QUAT_INIT(q.x + by, q.y + by, q.z + by, q.w + by); }
Quat quat_sub_scalar(Quat q, F32 by) { return QUAT_INIT(q.x - by, q.y - by, q.z - by, q.w - by); }

Quat quat_mul(Quat a, Quat b) {
	Vec3 av = VEC3_INIT(a.x, a.y, a.z);
	Vec3 bv = VEC3_INIT(b.x, b.y, b.z);

	Vec3 r = vec3_cross(av, bv);
	Vec3 wv = vec3_mul_scalar(av, b.w);
	r = vec3_add(r, wv);
	wv = vec3_mul_scalar(bv, a.w);
	r = vec3_add(r, wv);
	F32 w = (a.w * b.w) - ((av.x * bv.x) + (av.y * bv.y) + (av.z * bv.z));

	return QUAT_INIT(r.x, r.y, r.z, w);
}

Vec3 quat_mul_vec3(Quat q, Vec3 vec) {
	Vec3 u = VEC3_INIT(q.x, q.y, q.z);

	Vec3 t = vec3_cross(u, vec);
	t = vec3_mul_scalar(t, 2.f);

	u = vec3_cross(u, t);
	t = vec3_mul_scalar(t, q.w);

	Vec3 r = vec3_add(vec, t);
	return vec3_add(r, u);
}

Quat quat_conj(Quat q) {
	return QUAT_INIT(-q.x, -q.y, -q.z, q.w);
}

Quat quat_rotate(F32 angle, Vec3 axis) {
	F32 s = sinf(angle / 2.f);
	F32 c = cosf(angle / 2.f);

	return QUAT_INIT(
		axis.x * s,
		axis.y * s,
		axis.z * s,
		c);
}

F32 quat_euler_roll(Quat q) {
	F32 sinr_cosp = 2.f * (q.w * q.x + q.y * q.z);
	F32 cosr_cosp = 1.f - 2.f * (q.x * q.x + q.y * q.y);
	return atan2(sinr_cosp, cosr_cosp);
}

F32 quat_euler_pitch(Quat q) {
	F32 sinp = 2.f * (q.w * q.y - q.z * q.x);
	if (fabsf(sinp) >= 1.f) {
		return copysign(M_PI / 2.f, sinp); // use 90 degrees if out of range
	} else {
		return asinf(sinp);
	}
}

F32 quat_euler_yaw(Quat q) {
	F32 siny_cosp = 2.f * (q.w * q.z + q.x * q.y);
	F32 cosy_cosp = 1.f - 2.f * (q.y * q.y + q.z * q.z);
	return atan2f(siny_cosp, cosy_cosp);
}

Quat quat_from_euler(F32 roll, F32 pitch, F32 yaw) {
	F32 cy = cosf(yaw * 0.5f);
	F32 sy = sinf(yaw * 0.5f);
	F32 cp = cosf(pitch * 0.5f);
	F32 sp = sinf(pitch * 0.5f);
	F32 cr = cosf(roll * 0.5f);
	F32 sr = sinf(roll * 0.5f);

	Quat q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;

	return q;
}

// ===========================================
//
//
// Matrices - column major order
//
//
// ===========================================


void mat3x2_identity(Mat3x2* out) {
	out->col[0] = VEC2_INIT(1.0, 0.0);
	out->col[1] = VEC2_INIT(0.0, 1.0);
	out->col[2] = VEC2_INIT(0.0, 0.0);
}

void mat3x2_identity_translate(Mat3x2* out, Vec2 v) {
	out->col[0] = VEC2_INIT(1.0, 0.0);
	out->col[1] = VEC2_INIT(0.0, 1.0);
	out->col[2] = v;
}

void mat3x2_identity_scale(Mat3x2* out, Vec2 v) {
	out->col[0] = VEC2_INIT(v.x, 0.0);
	out->col[1] = VEC2_INIT(0.0, v.y);
	out->col[2] = VEC2_INIT(0.0, 0.0);
}

void mat3x2_identity_rotate(Mat3x2* out, F32 angle) {
	F32 c = cosf(angle);
	F32 s = sinf(angle);
	out->col[0] = VEC2_INIT(c, -s);
	out->col[1] = VEC2_INIT(s, c);
	out->col[2] = VEC2_INIT(0.0, 0.0);
}

Vec2 mat3x2_row(Mat3x2* m, U32 row_idx) {
	HERO_ASSERT(row_idx < 3, "col index must be less than 3 but got %u", row_idx);
	return m->col[row_idx];
}

Vec3 mat3x2_column(Mat3x2* m, U32 column_idx) {
	HERO_ASSERT(column_idx < 2, "column index must be less than 2 but got %u", column_idx);
	return VEC3_INIT(
		m->a[0 * 2 + column_idx],
		m->a[1 * 2 + column_idx],
		m->a[2 * 2 + column_idx]
	);
}

void mat3x2_mul(Mat3x2* out, Mat3x2* a, Mat3x2* b) {
	out->col[0] = VEC2_INIT(
		(a->col[0].x * b->col[0].x) + (a->col[0].y * b->col[1].x),
		(a->col[0].x * b->col[0].y) + (a->col[0].y * b->col[1].y)
	);

	out->col[1] = VEC2_INIT(
		(a->col[1].x * b->col[0].x) + (a->col[1].y * b->col[1].x),
		(a->col[1].x * b->col[0].y) + (a->col[1].y * b->col[1].y)
	);

	out->col[2] = VEC2_INIT(
		(a->col[2].x * b->col[0].x) + (a->col[2].y * b->col[1].x) + b->col[2].x,
		(a->col[2].x * b->col[0].y) + (a->col[2].y * b->col[1].y) + b->col[2].y
	);
}

Vec2 mat3x2_mul_point(Mat3x2* m, Vec2 pt) {
	return VEC2_INIT(
		(pt.x * m->col[0].x) + (pt.y * m->col[1].x) + m->col[2].x,
		(pt.x * m->col[0].y) + (pt.y * m->col[1].y) + m->col[2].y
	);
}

Vec2 mat3x2_mul_vector(Mat3x2* m, Vec2 v) {
	return VEC2_INIT(
		(v.x * m->col[0].x) + (v.y * m->col[1].x),
		(v.x * m->col[0].y) + (v.y * m->col[1].y)
	);
}


void mat4x4_identity(Mat4x4* out) {
	out->col[0] = VEC4_INIT(1.0, 0.0, 0.0, 0.0);
	out->col[1] = VEC4_INIT(0.0, 1.0, 0.0, 0.0);
	out->col[2] = VEC4_INIT(0.0, 0.0, 1.0, 0.0);
	out->col[3] = VEC4_INIT(0.0, 0.0, 0.0, 1.0);
}

void mat4x4_identity_scale(Mat4x4* out, Vec3 v) {
	mat4x4_identity(out);
	out->col[0].x = v.x;
	out->col[1].y = v.y;
	out->col[2].z = v.z;
}

void mat4x4_identity_rotate(Mat4x4* out, Vec3 v, F32 angle) {
	F32 xx = v.x * v.x;
	F32 yy = v.y * v.y;
	F32 zz = v.z * v.z;

	F32 t = angle / 2.0;
	F32 ts = sinf(t);
	F32 tc = cosf(t);
	F32 sc = ts * tc;
	F32 sq = ts * ts;

	out->col[0] = VEC4_INIT(
		1.0 - (2.0 * (yy + zz) * sq),
		2.0 * ((v.x * v.y * sq) + (v.z * sc)),
		2.0 * ((v.x * v.z * sq) - (v.y * sc)),
		0.0
	);

	out->col[1] = VEC4_INIT(
		2.0 * ((v.x * v.y * sq) - (v.z * sc)),
		1.0 - (2.0 * (xx + zz) * sq),
		2.0 * ((v.y * v.z * sq) + (v.x * sc)),

		0.0
	);

	out->col[2] = VEC4_INIT(
		2.0 * ((v.x * v.z * sq) + (v.y * sc)),
		2.0 * ((v.y * v.z * sq) - (v.x * sc)),
		1.0 - (2.0 * (xx + yy) * sq),
		0.0
	);

	out->col[3] = VEC4_INIT(0.0, 0.0, 0.0, 1.0);
}

void mat4x4_identity_rotate_x(Mat4x4* out, F32 angle) {
	F32 s = sinf(angle);
	F32 c = cosf(angle);

	*out = (Mat4x4){0};
	out->col[0].x = 1.f;
	out->col[1].y = c;
	out->col[1].z = s;
	out->col[2].y = -s;
	out->col[2].z = c;
	out->col[3].w = 1.f;
}

void mat4x4_identity_rotate_y(Mat4x4* out, F32 angle) {
	F32 s = sinf(angle);
	F32 c = cosf(angle);

	*out = (Mat4x4){0};
	out->col[0].x = c;
	out->col[0].z = -s;
	out->col[1].y = 1.f;
	out->col[2].x = s;
	out->col[2].z = c;
	out->col[3].w = 1.f;
}

void mat4x4_identity_rotate_z(Mat4x4* out, F32 angle) {
	F32 s = sinf(angle);
	F32 c = cosf(angle);

	*out = (Mat4x4){0};
	out->col[0].x = c;
	out->col[0].y = s;
	out->col[1].x = -s;
	out->col[1].y = c;
	out->col[2].z = 1.f;
	out->col[3].w = 1.f;
}

void mat4x4_identity_translate(Mat4x4* out, Vec3 v) {
	mat4x4_identity(out);
	out->col[3].x = v.x;
	out->col[3].y = v.y;
	out->col[3].z = v.z;
}

void mat4x4_from_quat(Mat4x4* out, Quat quat) {
	float a = quat.w;
	float b = quat.x;
	float c = quat.y;
	float d = quat.z;
	float a2 = a * a;
	float b2 = b * b;
	float c2 = c * c;
	float d2 = d * d;

	out->col[0] = VEC4_INIT(
		a2 + b2 - c2 - d2,
		2.f*(b*c + a*d),
		2.f*(b*d - a*c),
		0.f
	);

	out->col[1] = VEC4_INIT(
		2.f*(b*c - a*d),
		a2 - b2 + c2 - d2,
		2.f*(c*d + a*b),
		0.f
	);

	out->col[2] = VEC4_INIT(
		2.f*(b*d + a*c),
		2.f*(c*d - a*b),
		a2 - b2 - c2 + d2,
		0.f
	);

	out->col[3] = VEC4_INIT(0.f, 0.f, 0.f, 1.f);
}

void mat4x4_scale(Mat4x4* m, Vec3 v) {
	Mat4x4 mat;
	mat4x4_identity_scale(&mat, v);
	mat4x4_mul(m, &mat, m);
}

void mat4x4_rotate(Mat4x4* m, Vec3 v, F32 angle) {
	Mat4x4 mat;
	mat4x4_identity_rotate(&mat, v, angle);
	mat4x4_mul(m, &mat, m);
}

void mat4x4_rotate_x(Mat4x4* m, F32 angle) {
	Mat4x4 mat;
	mat4x4_identity_rotate_x(&mat, angle);
	mat4x4_mul(m, &mat, m);
}

void mat4x4_rotate_y(Mat4x4* m, F32 angle) {
	Mat4x4 mat;
	mat4x4_identity_rotate_y(&mat, angle);
	mat4x4_mul(m, &mat, m);
}

void mat4x4_rotate_z(Mat4x4* m, F32 angle) {
	Mat4x4 mat;
	mat4x4_identity_rotate_z(&mat, angle);
	mat4x4_mul(m, &mat, m);
}

void mat4x4_translate(Mat4x4* m, Vec3 v) {
	Mat4x4 mat;
	mat4x4_identity_translate(&mat, v);
	mat4x4_mul(m, &mat, m);
}

void mat4x4_from_3x2(Mat4x4* out, Mat3x2* m) {
	out->col[0] = VEC4_INIT(m->col[0].x, m->col[0].y, 0.0, 0.0);
	out->col[1] = VEC4_INIT(m->col[1].x, m->col[1].y, 0.0, 0.0);
	out->col[2] = VEC4_INIT(0.0, 0.0, 1.0, 0.0);
	out->col[3] = VEC4_INIT(m->col[2].x, m->col[2].y, 0.0, 1.0);
}

Vec4 mat4x4_row(Mat4x4* m, U32 row_idx) {
	HERO_ASSERT(row_idx < 4, "row index must be less than 4 but got %u", row_idx);
	return VEC4_INIT(
		m->a[0 * 4 + row_idx],
		m->a[1 * 4 + row_idx],
		m->a[2 * 4 + row_idx],
		m->a[3 * 4 + row_idx]
	);
}

Vec4 mat4x4_column(Mat4x4* m, U32 column_idx) {
	HERO_ASSERT(column_idx < 4, "column index must be less than 4 but got %u", column_idx);
	return m->col[column_idx];
}

void mat4x4_ortho(Mat4x4* out, F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far) {
	F32 diff_x = right - left;
	F32 diff_y = bottom - top;
	F32 diff_z = far - near;

	F32 tx = -((right + left) / diff_x);
	F32 ty = -((top + bottom) / diff_y);
	F32 tz = -((far + near) / diff_z);

	out->col[0] = VEC4_INIT(2.f / diff_x, 0.0, 0.0, 0.0);
	out->col[1] = VEC4_INIT(0.0, 2.f / diff_y, 0.0, 0.0);
	out->col[2] = VEC4_INIT(0.0, 0.0, -2.f / diff_z, 0.0);
	out->col[3] = VEC4_INIT(tx, ty, tz, 1.0);
}

void mat4x4_perspective(Mat4x4* out, F32 fovy, F32 aspect_ratio, F32 z_near, F32 z_far) {
	HERO_ASSERT(aspect_ratio != 0.0, "aspect_ratio cannot be 0.0");
	HERO_ASSERT(z_far != z_near, "z_near and z_far cannot be equal");

	float tan_half_fovy = tanf(fovy / 2.0);
	float a = 1.0 / tan_half_fovy;

	*out = (Mat4x4){0};
	out->col[0].x = a / aspect_ratio;
	out->col[1].y = a;
	out->col[2].z = -((z_far + z_near) / (z_far - z_near));
	out->col[2].w = -1.0;
	out->col[3].z = -((2.0 * z_far * z_near) / (z_far - z_near));
}

void mat4x4_mul(Mat4x4* out, Mat4x4* a, Mat4x4* b) {
	out->col[0] = VEC4_INIT(
		a->col[0].x * b->col[0].x  +  a->col[0].y * b->col[1].x  +  a->col[0].z * b->col[2].x  +  a->col[0].w * b->col[3].x,
		a->col[0].x * b->col[0].y  +  a->col[0].y * b->col[1].y  +  a->col[0].z * b->col[2].y  +  a->col[0].w * b->col[3].y,
		a->col[0].x * b->col[0].z  +  a->col[0].y * b->col[1].z  +  a->col[0].z * b->col[2].z  +  a->col[0].w * b->col[3].z,
		a->col[0].x * b->col[0].w  +  a->col[0].y * b->col[1].w  +  a->col[0].z * b->col[2].w  +  a->col[0].w * b->col[3].w
	);

	out->col[1] = VEC4_INIT(
		a->col[1].x * b->col[0].x  +  a->col[1].y * b->col[1].x  +  a->col[1].z * b->col[2].x  +  a->col[1].w * b->col[3].x,
		a->col[1].x * b->col[0].y  +  a->col[1].y * b->col[1].y  +  a->col[1].z * b->col[2].y  +  a->col[1].w * b->col[3].y,
		a->col[1].x * b->col[0].z  +  a->col[1].y * b->col[1].z  +  a->col[1].z * b->col[2].z  +  a->col[1].w * b->col[3].z,
		a->col[1].x * b->col[0].w  +  a->col[1].y * b->col[1].w  +  a->col[1].z * b->col[2].w  +  a->col[1].w * b->col[3].w
	);

	out->col[2] = VEC4_INIT(
		a->col[2].x * b->col[0].x  +  a->col[2].y * b->col[1].x  +  a->col[2].z * b->col[2].x  +  a->col[2].w * b->col[3].x,
		a->col[2].x * b->col[0].y  +  a->col[2].y * b->col[1].y  +  a->col[2].z * b->col[2].y  +  a->col[2].w * b->col[3].y,
		a->col[2].x * b->col[0].z  +  a->col[2].y * b->col[1].z  +  a->col[2].z * b->col[2].z  +  a->col[2].w * b->col[3].z,
		a->col[2].x * b->col[0].w  +  a->col[2].y * b->col[1].w  +  a->col[2].z * b->col[2].w  +  a->col[2].w * b->col[3].w
	);

	out->col[3] = VEC4_INIT(
		a->col[3].x * b->col[0].x  +  a->col[3].y * b->col[1].x  +  a->col[3].z * b->col[2].x  +  a->col[3].w * b->col[3].x,
		a->col[3].x * b->col[0].y  +  a->col[3].y * b->col[1].y  +  a->col[3].z * b->col[2].y  +  a->col[3].w * b->col[3].y,
		a->col[3].x * b->col[0].z  +  a->col[3].y * b->col[1].z  +  a->col[3].z * b->col[2].z  +  a->col[3].w * b->col[3].z,
		a->col[3].x * b->col[0].w  +  a->col[3].y * b->col[1].w  +  a->col[3].z * b->col[2].w  +  a->col[3].w * b->col[3].w
	);
}

void mat4x4_mul_quat(Mat4x4* out, Mat4x4* m, Quat q) {
	Vec3 col_0 = quat_mul_vec3(q, VEC3_INIT(m->col[0].x, m->col[0].y, m->col[0].z));
	Vec3 col_1 = quat_mul_vec3(q, VEC3_INIT(m->col[1].x, m->col[1].y, m->col[1].z));
	Vec3 col_2 = quat_mul_vec3(q, VEC3_INIT(m->col[2].x, m->col[2].y, m->col[2].z));

	out->col[0] = VEC4_INIT(col_0.x, col_0.y, col_0.z, m->col[0].w);
	out->col[1] = VEC4_INIT(col_1.x, col_1.y, col_1.z, m->col[1].w);
	out->col[2] = VEC4_INIT(col_2.x, col_2.y, col_2.z, m->col[2].w);
	out->col[3] = m->col[3];
}

Vec3 mat4x4_mul_point(Mat4x4* m, Vec3 pt) {
	return VEC3_INIT(
		(pt.x * m->col[0].x) + (pt.y * m->col[1].y) + (pt.z * m->col[2].z) + m->col[3].x,
		(pt.x * m->col[1].x) + (pt.y * m->col[1].y) + (pt.z * m->col[2].z) + m->col[3].y,
		(pt.x * m->col[2].x) + (pt.y * m->col[1].y) + (pt.z * m->col[2].z) + m->col[3].z
	);
}

Vec3 mat4x4_mul_vector(Mat4x4* m, Vec3 v) {
	return VEC3_INIT(
		(v.x * m->col[0].x) + (v.y * m->col[1].y) + (v.z * m->col[2].z),
		(v.x * m->col[1].x) + (v.y * m->col[1].y) + (v.z * m->col[2].z),
		(v.x * m->col[2].x) + (v.y * m->col[1].y) + (v.z * m->col[2].z)
	);
}

// ===========================================
//
//
// Aabb
//
//
// ===========================================

HeroAabb hero_aabb_cut_left(HeroAabb* parent, float len) {
	float x = parent->x;
	parent->x = HERO_MIN(parent->ex, parent->x + len);
	return (HeroAabb){ x, parent->y, parent->x, parent->ey };
}

HeroAabb hero_aabb_cut_right(HeroAabb* parent, float len) {
	float ex = parent->ex;
	parent->ex = HERO_MAX(parent->x, parent->ex - len);
	return (HeroAabb){ parent->ex, parent->y, ex, parent->ey };
}

HeroAabb hero_aabb_cut_top(HeroAabb* parent, float len) {
	float y = parent->y;
	parent->y = HERO_MIN(parent->ey, parent->y + len);
	return (HeroAabb){ parent->x, y, parent->ex, parent->y };
}

HeroAabb hero_aabb_cut_bottom(HeroAabb* parent, float len) {
	float ey = parent->ey;
	parent->ey = HERO_MAX(parent->y, parent->ey - len);
	return (HeroAabb){ parent->x, parent->ey, parent->ex, ey };
}

HeroAabb hero_aabb_cut_center_horizontal(HeroAabb* parent, float len) {
	float width = parent->ex - parent->x;
	float side_remove_len = HERO_MAX((width - len) / 2.f, 0.f);
	float x = parent->x + side_remove_len;
	float ex = parent->ex - side_remove_len;
	return (HeroAabb){ x, parent->y, ex, parent->ey };
}

HeroAabb hero_aabb_cut_center_vertical(HeroAabb* parent, float len) {
	float height = parent->ey - parent->y;
	float side_remove_len = HERO_MAX((height - len) / 2.f, 0.f);
	float y = parent->y + side_remove_len;
	float ey = parent->ey - side_remove_len;
	return (HeroAabb){ parent->x, y, parent->ex, ey };
}

HeroAabb hero_aabb_keep_overlapping(HeroAabb* a, HeroAabb* b) {
    if (
        a->x >= b->ex || a->y >= b->ey ||
        b->x >= a->ex || b->y >= a->ey
    ) {
        return ((HeroAabb){0});
    }

    return ((HeroAabb){
		.x = HERO_MAX(a->x, b->x),
		.y = HERO_MAX(a->y, b->y),
		.ex = HERO_MIN(a->ex, b->ex),
		.ey = HERO_MIN(a->ey, b->ey),
	});
}

bool hero_aabb_intersects_pt(HeroAabb* a, Vec2 pt) {
	return a->x <= pt.x && a->ex >= pt.x &&
		a->y <= pt.y && a->ey >= pt.y;
}

void hero_aabb_print(HeroAabb* aabb, const char* name) {
	printf("%s: { x: %f, y: %f, ex: %f, ey: %f\n", name, aabb->x, aabb->y, aabb->ex, aabb->ey);
}

void hero_uaabb_print(HeroUAabb* aabb, const char* name) {
	printf("%s: { x: %d, y: %d, ex: %d, ey: %d\n", name, aabb->x, aabb->y, aabb->ex, aabb->ey);
}

// ==========================================
//
//
// Dynamic Library Loading
//
//
// ==========================================

HeroResult hero_dll_open(HeroDLL* dll, const char* path) {
#ifdef __unix__
	void* handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
#else
#error "unimplemented"
#endif
	if (handle == NULL) return HERO_ERROR(OPENING_DLL);
	dll->handle = handle;
	return HERO_SUCCESS;
}

void hero_dll_close(HeroDLL* dll) {
#ifdef __unix__
	dlclose(dll->handle);
#else
#error "unimplemented"
#endif
	dll->handle = NULL;
}

HeroResult hero_dll_resolve_symbol(HeroDLL* dll, const char* symbol_name, void** addr_out) {
#ifdef __unix__
	void* addr = dlsym(dll->handle, symbol_name);
	if (addr == NULL) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}
	*addr_out = addr;
	return HERO_SUCCESS;
#else
#error "unimplemented"
#endif
}

const char* hero_dll_error(void) {
#ifdef __unix__
	return dlerror();
#else
#error "unimplemented"
#endif
}

// ==========================================
//
//
// Hero
//
//
// ==========================================

Hero hero;

HeroResult hero_init(HeroSetup* setup) {
	hero.enum_group_strings = setup->enum_group_strings;

	return HERO_SUCCESS;
}

