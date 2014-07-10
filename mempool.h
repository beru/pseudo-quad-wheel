#ifndef __MEM_POOL_H__
#define __MEM_POOL_H__
#include "memcontext.h"

/* 
 * struct relationship:
 * |------------------------MemBlock------------------------|
 * |                        ________        __________      |
 * |                 next* /        \      /          \     |
 * |struct Memblock|Memnode|UserData|Memnode|UserData|...   |
 *         next* |         /\
 *               |          Return to user
 * |struct Memblock|
 *              ...
 *
 * define DONT_USE_POOL to disable mempool and use native c mem funcs
 * Note:
 *    all allocated mem from pool is not zero filled
 */

//#define DONT_USE_POOL

/* how many node in one block */
#define MP_BLOCK_SIZE	1024

/* per node size is (sizeof(Memnode) + mp->elemsize); */
typedef struct Memnode {
	struct Memnode* next;
	unsigned int esize;
} Memnode;

typedef struct Memblock {
	struct Memblock* next;
} Memblock;

typedef struct Mempool {
	unsigned int elemsize;

	Memblock* blockhead;
	Memnode* nodehead;
	void* mc;
} Mempool;

void mpool_init(struct memcontext* mc);

/* create a pool, with size and initial element count */
/* return pool handle */
Mempool* mpool_create(struct memcontext* mc,unsigned int elemsize);

/* allocate an element from pool */
/* same as malloc */
void* mpool_alloc(Mempool* mpool);

/* free an element to pool */
/* same as free */
void mpool_free(void* p, Mempool* mpool);

/* general malloc/free replacement
 * cutting down calling times of malloc and free,
 * but some times slower then native malloc/free, 
 * don't know why, if so
 * simply USE DONT_USE_POOL to disable mempool
 */
void* mm_alloc(void* ps, unsigned int size);
void* mm_realloc(void* ps, void* p, unsigned int size);
void mm_free(void* ps, void* p);

#define POOL_COUNT 8

void* mc_malloc(void* mc, unsigned int size);
void mc_free(void* mc, void* p);

#endif
