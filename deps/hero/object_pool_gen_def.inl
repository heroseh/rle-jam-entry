#ifndef _HERO_CORE_H_
#error "core.h must be included before this file"
#endif

#ifndef HERO_OBJECT_TYPE
#error "HERO_OBJECT_TYPE must be defined with the type of the object pool's element"
#endif

#ifndef HERO_OBJECT_NAME
#define HERO_OBJECT_NAME HERO_OBJECT_TYPE
#endif

#define HERO_OBJECT_POOL HERO_CONCAT(HeroObjectPool_,HERO_OBJECT_NAME)

typedef struct HERO_OBJECT_POOL HERO_OBJECT_POOL;
struct HERO_OBJECT_POOL {
	HERO_OBJECT_TYPE* data;
	U32 count;
	U32 cap;
	U32 free_list_head_idx;
	U32 allocated_list_head_idx;
	U32 allocated_list_tail_idx;
};

#undef HERO_OBJECT_NAME
#undef HERO_OBJECT_POOL

#ifndef HERO_NO_UNDEF
#undef HERO_OBJECT_TYPE
#endif // HERO_NO_UNDEF
