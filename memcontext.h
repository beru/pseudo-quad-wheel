
#ifndef MEMCONTEXT_H
#define MEMCONTEXT_H
#define POOL_COUNT	8

#include "config.h"

#if USE_DLMALLOC
#include "malloc/dlmalloc.h"
#endif

struct memcontext {
#if USE_DLMALLOC
	void* mymspace;
#endif
	void* general_pools[POOL_COUNT];  
};
#endif

