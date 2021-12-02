#ifndef _HERO_CORE_H_
#error "core.h must be included before this file"
#endif

#ifndef HERO_HASH_TABLE_KEY_TYPE
#error "HERO_HASH_TABLE_KEY_TYPE must be defined with the type of the hash table's key"
#endif

#ifndef HERO_HASH_TABLE_VALUE_TYPE
#error "HERO_HASH_TABLE_VALUE_TYPE must be defined with the type of the hash table's value"
#endif

#ifndef HERO_HASH_TABLE_KEY_IS_ARRAY
#define HERO_HASH_TABLE_KEY_IS_ARRAY false
#endif

#define HERO_HASH_TABLE_KEY_SIZE (HERO_HASH_TABLE_KEY_IS_ARRAY ? 0 : sizeof(HERO_HASH_TABLE_KEY_TYPE))

#ifndef HERO_HASH_TABLE_KEY_NAME
#define HERO_HASH_TABLE_KEY_NAME HERO_HASH_TABLE_KEY_TYPE
#endif

#ifndef HERO_HASH_TABLE_VALUE_NAME
#define HERO_HASH_TABLE_VALUE_NAME HERO_HASH_TABLE_VALUE_TYPE
#endif

#define HERO_HASH_TABLE_FN(NAME) HERO_CONCAT(HERO_CONCAT(HERO_CONCAT(hero_hash_table_##NAME##_,HERO_HASH_TABLE_KEY_NAME),_),HERO_HASH_TABLE_VALUE_NAME)

#define HERO_HASH_TABLE_ENTRY HERO_CONCAT(HERO_CONCAT(HERO_CONCAT(HeroHashTableEntry_,HERO_HASH_TABLE_KEY_NAME),_),HERO_HASH_TABLE_VALUE_NAME)

#define HERO_HASH_TABLE HERO_CONCAT(HERO_CONCAT(HERO_CONCAT(HeroHashTable_,HERO_HASH_TABLE_KEY_NAME),_),HERO_HASH_TABLE_VALUE_NAME)

static inline HeroResult HERO_HASH_TABLE_FN(init)(HERO_HASH_TABLE* hash_table, HeroIAlctor alctor, HeroAllocTag tag) {
	return _hero_hash_table_init((HeroHashTable*)hash_table, alctor, tag, sizeof(HERO_HASH_TABLE_ENTRY), alignof(HERO_HASH_TABLE_ENTRY));
}

static inline void HERO_HASH_TABLE_FN(deinit)(HERO_HASH_TABLE* hash_table, HeroIAlctor alctor, HeroAllocTag tag) {
	_hero_hash_table_deinit((HeroHashTable*)hash_table, alctor, tag, sizeof(HERO_HASH_TABLE_ENTRY), alignof(HERO_HASH_TABLE_ENTRY));
}

static inline HeroResult HERO_HASH_TABLE_FN(find)(HERO_HASH_TABLE* hash_table, const HERO_HASH_TABLE_KEY_TYPE* key, HERO_HASH_TABLE_ENTRY** entry_ptr_out) {
	return _hero_hash_table_find((HeroHashTable*)hash_table, key, HERO_HASH_TABLE_KEY_SIZE, sizeof(HERO_HASH_TABLE_ENTRY), (void**)entry_ptr_out);
}

static inline HeroResult HERO_HASH_TABLE_FN(find_or_insert)(HERO_HASH_TABLE* hash_table, const HERO_HASH_TABLE_KEY_TYPE* key, HeroIAlctor alctor, HeroAllocTag tag, HERO_HASH_TABLE_ENTRY** entry_ptr_out) {
	return _hero_hash_table_find_or_insert((HeroHashTable*)hash_table, key, HERO_HASH_TABLE_KEY_SIZE, alctor, tag, sizeof(HERO_HASH_TABLE_ENTRY), alignof(HERO_HASH_TABLE_ENTRY), (void**)entry_ptr_out);
}

static inline HeroResult HERO_HASH_TABLE_FN(remove)(HERO_HASH_TABLE* hash_table, const HERO_HASH_TABLE_KEY_TYPE* key, HeroIAlctor alctor, HeroAllocTag tag) {
	return _hero_hash_table_remove((HeroHashTable*)hash_table, key, HERO_HASH_TABLE_KEY_SIZE, alctor, tag, sizeof(HERO_HASH_TABLE_ENTRY), alignof(HERO_HASH_TABLE_ENTRY));
}

static inline HeroResult HERO_HASH_TABLE_FN(iter_next)(HERO_HASH_TABLE* hash_table, Uptr* next_entry_idx_mut, HERO_HASH_TABLE_ENTRY** entry_ptr_out) {
	return _hero_hash_table_iter_next((HeroHashTable*)hash_table, sizeof(HERO_HASH_TABLE_ENTRY), next_entry_idx_mut, (void**)entry_ptr_out);
}

static inline HeroResult HERO_HASH_TABLE_FN(clear)(HERO_HASH_TABLE* hash_table, HeroIAlctor alctor, HeroAllocTag tag) {
	return _hero_hash_table_clear((HeroHashTable*)hash_table, alctor, tag, sizeof(HERO_HASH_TABLE_ENTRY), alignof(HERO_HASH_TABLE_ENTRY));
}

#undef HERO_HASH_TABLE_KEY_TYPE
#undef HERO_HASH_TABLE_VALUE_TYPE
#undef HERO_HASH_TABLE_KEY_IS_ARRAY
#undef HERO_HASH_TABLE_KEY_SIZE
#undef HERO_HASH_TABLE_KEY_NAME
#undef HERO_HASH_TABLE_VALUE_NAME
#undef HERO_HASH_TABLE_FN
#undef HERO_HASH_TABLE_ENTRY
#undef HERO_HASH_TABLE



