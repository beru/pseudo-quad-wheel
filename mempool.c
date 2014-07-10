#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "config.h"
#include "error.h"
#include "mempool.h"
#include "memcontext.h"

#if USE_DLMALLOC
#include "malloc/dlmalloc.h"
#endif	//

void*
mc_malloc (void* mc, unsigned int size) 
{
#if USE_DLMALLOC
  memcontext* ctx = mc;
  if (ctx->mymspace) {
    return (void *)mspace_malloc (ctx->mymspace, size);
  }else {
    return malloc (size);
  }
#else	// #if USE_DLMALLOC
  return malloc (size);
#endif	// #if USE_DLMALLOC
}


void
mc_free (void* mc, void* p)
{
  
#if USE_DLMALLOC
  memcontext* ctx = mc;
  if (ctx->mymspace) {
    mspace_free (ctx->mymspace, p);
  }else {
    free(p);
  }
#else	// #if USE_DLMALLOC
    free(p);
#endif	// #if USE_DLMALLOC
}
 
#ifndef DONT_USE_POOL
static void 
pool_extern (Mempool* mp) 
{
  int size = sizeof (Memblock) + ((mp->elemsize + sizeof (Memnode)) * MP_BLOCK_SIZE);
  void* mc = mp->mc;
  Memblock* mb = mc_malloc (mc, size);
   if (!mb) {
    xdie ("Out of memory\n");
   }
   
  // push to block head
  mb->next = mp->blockhead;
  mp->blockhead = mb;
  char* p = (char*) mb;		// raw byte proccess
  p += sizeof (Memblock);	// p pointed to (Memnode+elesize) * BLOCKSIZE
  for (int i=0; i<MP_BLOCK_SIZE; ++i) {
      Memnode* n = (Memnode *) p;
      n->esize = mp->elemsize;
      n->next = mp->nodehead;
      mp->nodehead = n;
      p += (mp->elemsize + sizeof (Memnode));
  }
}
#endif	// #ifndef DONT_USE_POOL

Mempool* mpool_create (memcontext* mc, unsigned int elemsize)
{
  Mempool* mp = mc_malloc (mc, sizeof (Mempool));
  if (!mp) {
    xdie ("Out of memory\n");
  }
  memset (mp, 0, sizeof (Mempool*));
  if (elemsize % 4 != 0) {
    xbug ("Problem, create a non-align mempool, failed");
  }
  mp->elemsize = elemsize;
  mp->mc = mc;
   
#ifndef DONT_USE_POOL
    pool_extern (mp);
#endif	//
    return mp;
}

void*
mpool_alloc (Mempool* mp) 
{
   
#ifndef DONT_USE_POOL
  if (!mp->nodehead) {		// running out of cache
    pool_extern (mp);
  }
  char* ret = (char*) mp->nodehead;
  mp->nodehead = mp->nodehead->next;
  return (void*) (ret + (int) sizeof (Memnode));
   
#else	//
    return malloc (mp->elemsize);
#endif	//
}

void 
mpool_free (void* p, Mempool* mp) 
{
   
#ifndef DONT_USE_POOL
  Memnode* n = (Memnode *) (((char *) p) - sizeof (Memnode));
  void* mc = mp->mc;
  if (n->esize != mp->elemsize) {
      xbug ("mismatch");
  }
  n->next = mp->nodehead;
  mp->nodehead = n;
#else	//
  free (p);
   
#endif	//
}  

#ifndef DONT_USE_POOL
// Create pools to replace malloc */ 
static const unsigned int gpools_sizes[POOL_COUNT] = 
  { 8, 12, 16, 24, 32, 64, 128, 256 
};

static const unsigned char sizeindexes[129] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, // 0 - 8
    1, 1, 1, 1, // 9 - 12
    2, 2, 2, 2, // 13 - 16
    3, 3, 3, 3, 3, 3, 3, 3, // 17 - 24
    4, 4, 4, 4, 4, 4, 4, 4, // 25 - 32
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, // 33 - 64
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 // 65 - 128
};

void** pstate_get_general_pools (void* ps);
 
#define roundsize(n) (((n)<=128)?sizeindexes[n]:(((n)>256)?-1:7))
#endif	//
 void 
mpool_init (memcontext* ctx) 
{
  
#ifndef DONT_USE_POOL
  if (!ctx) {
	  return;
  }
  for (int i=0; i<POOL_COUNT; ++i) {
      if (gpools_sizes[i]) {
	  ctx->general_pools[i] = mpool_create (ctx, gpools_sizes[i]);
	}
  }
  
#else	//

  if (ctx) {
    int i;
    for (i=0; i<POOL_COUNT; ++i)
    {
      ctx->general_pools[i] = 0;
    }
  }
  
#endif	//
}

void* _ps_memctx_malloc(void*, unsigned int);
void _ps_memctx_free(void*, void*);

void*
mm_alloc (void* ec, unsigned int size) // size_t ...
{
   
#ifndef DONT_USE_POOL
  int poolindex = roundsize (size);
  void* ps = ec;
   if (poolindex >= 0) {
      void** pools = pstate_get_general_pools (ec);
      if (pools == 0) {
		  return malloc(size);
	  }
      return mpool_alloc (pools[poolindex]);
    }
   
    // block that bigger then 256 bytes, use malloc
    // still add a Memnode struct before the allocated memory
  Memnode* n = _ps_memctx_malloc (ps, size + sizeof (Memnode));
  n->esize = size;
  n++;
  return n;
   
#else	//
    return malloc (size);
  
#endif	//
}


void*
mm_realloc (void* ec, void* p, unsigned int size) // size_t ...
{
   
#ifndef DONT_USE_POOL
  void* q = mm_alloc(ec, size);
  if (p != 0) {
    Memnode* oldp = (((Memnode*)(p))-1);
    int oldsize = oldp->esize;
    memcpy(q, p, oldsize > size ? size : oldsize);
    mm_free(ec, p);
  }
  return q;
#else	//
  return realloc (p, size);
  
#endif	//
}

void
mm_free (void* ec, void* p) 
{
   
#ifndef DONT_USE_POOL
    Memnode* n = (Memnode*) (((char*) p) - sizeof (Memnode));
  void *ps = ec;
  int poolindex = roundsize (n->esize);
  if (poolindex >= 0) {
      void** pools = pstate_get_general_pools (ec);
      if (pools == 0) {
		free(p);
		return;
	  }
      mpool_free (p, pools[poolindex]);
      return;
    }
   
    // more then 256 bytes, use free
    _ps_memctx_free (ps, n);
   
#else	//
    free (p);
   
#endif	//
}   

