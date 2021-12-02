#ifndef _HERO_CORE_H_
#error "core.h must be included before this file"
#endif

#ifndef HERO_STACK_ELMT_TYPE
#error "HERO_STACK_ELMT_TYPE must be defined with the type of the stack's element"
#endif

#ifndef HERO_STACK_ELMT_NAME
#define HERO_STACK_ELMT_NAME HERO_STACK_ELMT_TYPE
#endif

#define HERO_STACK HERO_CONCAT(HeroStack_,HERO_STACK_ELMT_NAME)

typedef struct HERO_STACK HERO_STACK;
struct HERO_STACK {
	HERO_STACK_ELMT_TYPE* data;
	Uptr count;
	Uptr cap;
};

#undef HERO_STACK_ELMT_NAME
#undef HERO_STACK

#ifndef HERO_NO_UNDEF
#undef HERO_STACK_ELMT_TYPE
#endif // HERO_NO_UNDEF
