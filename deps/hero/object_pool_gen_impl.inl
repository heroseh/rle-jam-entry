#ifndef _HERO_CORE_H_
#error "core.h must be included before this file"
#endif

#ifndef HERO_OBJECT_ID_TYPE
#error "HERO_OBJECT_ID_TYPE must be defined with the name for the object identifier"
#endif

#ifndef HERO_OBJECT_TYPE
#error "HERO_OBJECT_TYPE must be defined with the type of the object pool's element"
#endif

HERO_STATIC_ASSERT(offsetof(HERO_OBJECT_TYPE, header) == 0, "header must be the first field in the HERO_OBJECT_TYPE structure");
HERO_STATIC_ASSERT(sizeof(((HERO_OBJECT_TYPE*)0)->header) == sizeof(HeroObjectHeader), "header must be of type HeroObjectHeader");

#ifndef HERO_OBJECT_ID_IDX_BITS
#define HERO_OBJECT_ID_IDX_BITS 20
#endif

#ifndef HERO_OBJECT_ID_COUNTER_BITS
#define HERO_OBJECT_ID_COUNTER_BITS 12
#endif

#ifndef HERO_OBJECT_ID_USER_BITS
#define HERO_OBJECT_ID_USER_BITS 0
#endif

#if HERO_OBJECT_ID_IDX_BITS + HERO_OBJECT_ID_COUNTER_BITS + HERO_OBJECT_ID_USER_BITS != 32
#error "the index, counter and user bits must be 32 in total"
#endif

#ifndef HERO_OBJECT_NAME
#define HERO_OBJECT_NAME HERO_OBJECT_TYPE
#endif

#define HERO_OBJECT_ID_FN(NAME) HERO_CONCAT(hero_object_id_##NAME##_,HERO_OBJECT_ID_TYPE)
#define HERO_OBJECT_POOL_FN(NAME) HERO_CONCAT(hero_object_pool_##NAME##_,HERO_OBJECT_NAME)

#define HERO_OBJECT_POOL HERO_CONCAT(HeroObjectPool_,HERO_OBJECT_NAME)

static inline U32 HERO_OBJECT_ID_FN(idx)(HERO_OBJECT_ID_TYPE id) {
	return (id.raw >> HERO_OBJECT_ID_IDX_SHIFT) & HERO_OBJECT_ID_IDX_MASK(HERO_OBJECT_ID_IDX_BITS);
}

static inline U32 HERO_OBJECT_ID_FN(counter)(HERO_OBJECT_ID_TYPE id) {
	return (id.raw >> HERO_OBJECT_ID_COUNTER_SHIFT(HERO_OBJECT_ID_IDX_BITS)) & HERO_OBJECT_ID_COUNTER_MASK(HERO_OBJECT_ID_COUNTER_BITS);
}

static inline U32 HERO_OBJECT_ID_FN(user_bits)(HERO_OBJECT_ID_TYPE id) {
#if HERO_OBJECT_ID_USER_BITS == 0
	return 0;
#else
	return (id.raw >> HERO_OBJECT_ID_USER_SHIFT(HERO_OBJECT_ID_IDX_BITS, HERO_OBJECT_ID_COUNTER_BITS)) & HERO_OBJECT_ID_USER_MASK(HERO_OBJECT_ID_USER_BITS);
#endif
}

static inline HeroResult HERO_OBJECT_POOL_FN(init)(HERO_OBJECT_POOL* object_pool, U32 cap, HeroIAlctor alctor, HeroAllocTag tag) {
	return _hero_object_pool_init((HeroObjectPool*)object_pool, cap, alctor, tag, sizeof(HERO_OBJECT_TYPE), alignof(HERO_OBJECT_TYPE));
}

static inline void HERO_OBJECT_POOL_FN(deinit)(HERO_OBJECT_POOL* object_pool, HeroIAlctor alctor, HeroAllocTag tag) {
	_hero_object_pool_deinit((HeroObjectPool*)object_pool, alctor, tag, sizeof(HERO_OBJECT_TYPE), alignof(HERO_OBJECT_TYPE));
}

static inline HeroResult HERO_OBJECT_POOL_FN(alloc)(HERO_OBJECT_POOL* object_pool, U32 user_bits, HERO_OBJECT_TYPE** ptr_out, HERO_OBJECT_ID_TYPE* id_out) {
	return _hero_object_pool_alloc((HeroObjectPool*)object_pool, user_bits, sizeof(HERO_OBJECT_TYPE), HERO_OBJECT_ID_IDX_BITS, HERO_OBJECT_ID_COUNTER_BITS, HERO_OBJECT_ID_USER_BITS, (void**)ptr_out, &id_out->raw);
}

static inline HeroResult HERO_OBJECT_POOL_FN(dealloc)(HERO_OBJECT_POOL* object_pool, HERO_OBJECT_ID_TYPE id) {
	return _hero_object_pool_dealloc((HeroObjectPool*)object_pool, id.raw, sizeof(HERO_OBJECT_TYPE), HERO_OBJECT_ID_IDX_BITS, HERO_OBJECT_ID_COUNTER_BITS);
}

static inline HeroResult HERO_OBJECT_POOL_FN(get)(HERO_OBJECT_POOL* object_pool, HERO_OBJECT_ID_TYPE id, HERO_OBJECT_TYPE** ptr_out) {
	return _hero_object_pool_get((HeroObjectPool*)object_pool, id.raw, sizeof(HERO_OBJECT_TYPE), HERO_OBJECT_ID_IDX_BITS, HERO_OBJECT_ID_COUNTER_BITS, (void**)ptr_out);
}

static inline HeroResult HERO_OBJECT_POOL_FN(get_id)(HERO_OBJECT_POOL* object_pool, HERO_OBJECT_TYPE* object, HERO_OBJECT_ID_TYPE* id_out) {
	return _hero_object_pool_get_id((HeroObjectPool*)object_pool, object, sizeof(HERO_OBJECT_TYPE), HERO_OBJECT_ID_IDX_BITS, &id_out->raw);
}

static inline void HERO_OBJECT_POOL_FN(clear)(HERO_OBJECT_POOL* object_pool) {
	_hero_object_pool_clear((HeroObjectPool*)object_pool);
}

static inline HeroResult HERO_OBJECT_POOL_FN(iter_next)(HERO_OBJECT_POOL* object_pool, HERO_OBJECT_ID_TYPE* id_mut, HERO_OBJECT_TYPE** ptr_out) {
	return _hero_object_pool_iter_next((HeroObjectPool*)object_pool, &id_mut->raw, sizeof(HERO_OBJECT_TYPE), HERO_OBJECT_ID_IDX_BITS, (void**)ptr_out);
}

static inline HeroResult HERO_OBJECT_POOL_FN(iter_prev)(HERO_OBJECT_POOL* object_pool, HERO_OBJECT_ID_TYPE* id_mut, HERO_OBJECT_TYPE** ptr_out) {
	return _hero_object_pool_iter_prev((HeroObjectPool*)object_pool, &id_mut->raw, sizeof(HERO_OBJECT_TYPE), HERO_OBJECT_ID_IDX_BITS, (void**)ptr_out);
}

#undef HERO_OBJECT_ID_TYPE
#undef HERO_OBJECT_TYPE
#undef HERO_OBJECT_ID_USER_BITS
#undef HERO_OBJECT_ID_COUNTER_BITS
#undef HERO_OBJECT_ID_IDX_BITS
#undef HERO_OBJECT_NAME
#undef HERO_OBJECT_ID_FN
#undef HERO_OBJECT_POOL_FN
#undef HERO_OBJECT_POOL

