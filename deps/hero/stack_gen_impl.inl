#ifndef _HERO_CORE_H_
#error "core.h must be included before this file"
#endif

#ifndef HERO_STACK_ELMT_TYPE
#error "HERO_STACK_ELMT_TYPE must be defined with the type of the stack's element"
#endif

#ifndef HERO_STACK_ELMT_NAME
#define HERO_STACK_ELMT_NAME HERO_STACK_ELMT_TYPE
#endif

#define HERO_STACK_FN(NAME) HERO_CONCAT(hero_stack_##NAME##_,HERO_STACK_ELMT_NAME)

#define HERO_STACK HERO_CONCAT(HeroStack_,HERO_STACK_ELMT_NAME)

static inline void HERO_STACK_FN(deinit)(HERO_STACK* stack, HeroIAlctor alctor, HeroAllocTag tag) {
	_hero_stack_deinit((HeroStack*)stack, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE));
}

static inline HERO_STACK_ELMT_TYPE* HERO_STACK_FN(get)(HERO_STACK* stack, Uptr idx) {
	return _hero_stack_get((HeroStack*)stack, idx, sizeof(HERO_STACK_ELMT_TYPE));
}

static inline HERO_STACK_ELMT_TYPE* HERO_STACK_FN(get_last)(HERO_STACK* stack) {
	return _hero_stack_get((HeroStack*)stack, stack->count - 1, sizeof(HERO_STACK_ELMT_TYPE));
}

static inline HeroResult HERO_STACK_FN(resize_cap)(HERO_STACK* stack, Uptr new_cap, HeroIAlctor alctor, HeroAllocTag tag) {
	return _hero_stack_resize_cap((HeroStack*)stack, new_cap, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE));
}

static inline HeroResult HERO_STACK_FN(push)(HERO_STACK* stack, HeroIAlctor alctor, HeroAllocTag tag, HERO_STACK_ELMT_TYPE** elmt_ptr_out) {
	return _hero_stack_push((HeroStack*)stack, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE), (void**)elmt_ptr_out);
}

static inline HeroResult HERO_STACK_FN(push_value)(HERO_STACK* stack, HeroIAlctor alctor, HeroAllocTag tag, HERO_STACK_ELMT_TYPE elmt) {
	HERO_STACK_ELMT_TYPE* elmt_ptr;
	HeroResult result = _hero_stack_push((HeroStack*)stack, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE), (void**)&elmt_ptr);
	if (result < 0) {
		return result;
	}
	*elmt_ptr = elmt;
	return HERO_SUCCESS;
}

static inline HeroResult HERO_STACK_FN(push_many)(HERO_STACK* stack, Uptr amount, HeroIAlctor alctor, HeroAllocTag tag, HERO_STACK_ELMT_TYPE** elmts_ptr_out) {
	return _hero_stack_push_many((HeroStack*)stack, amount, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE), (void**)elmts_ptr_out);
}

static inline HeroResult HERO_STACK_FN(insert)(HERO_STACK* stack, Uptr idx, HeroIAlctor alctor, HeroAllocTag tag, HERO_STACK_ELMT_TYPE** elmt_ptr_out) {
	return _hero_stack_insert((HeroStack*)stack, idx, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE), (void**)elmt_ptr_out);
}

static inline HeroResult HERO_STACK_FN(insert_value)(HERO_STACK* stack, Uptr idx, HeroIAlctor alctor, HeroAllocTag tag, HERO_STACK_ELMT_TYPE elmt) {
	HERO_STACK_ELMT_TYPE* elmt_ptr;
	HeroResult result = _hero_stack_insert((HeroStack*)stack, idx, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE), (void**)&elmt_ptr);
	if (result < 0) {
		return result;
	}
	*elmt_ptr = elmt;
	return HERO_SUCCESS;
}

static inline HeroResult HERO_STACK_FN(insert_many)(HERO_STACK* stack, Uptr idx, Uptr amount, HeroIAlctor alctor, HeroAllocTag tag, HERO_STACK_ELMT_TYPE** elmts_ptr_out) {
	return _hero_stack_insert_many((HeroStack*)stack, idx, amount, alctor, tag, sizeof(HERO_STACK_ELMT_TYPE), alignof(HERO_STACK_ELMT_TYPE), (void**)elmts_ptr_out);
}

static inline void HERO_STACK_FN(remove_shift)(HERO_STACK* stack, Uptr idx) {
	_hero_stack_remove_shift((HeroStack*)stack, idx, sizeof(HERO_STACK_ELMT_TYPE));
}

static inline HeroResult HERO_STACK_FN(remove_shift_range)(HERO_STACK* stack, Uptr start_idx, Uptr end_idx) {
	return _hero_stack_remove_shift_range((HeroStack*)stack, start_idx, end_idx, sizeof(HERO_STACK_ELMT_TYPE));
}

static inline HeroResult HERO_STACK_FN(pop)(HERO_STACK* stack) {
	return _hero_stack_pop((HeroStack*)stack);
}

#undef HERO_STACK_ELMT_TYPE
#undef HERO_STACK_ELMT_NAME
#undef HERO_STACK_FN
#undef HERO_STACK


