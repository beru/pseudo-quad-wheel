#ifndef __PSTAT_H__
#define __PSTAT_H__

#include <stdio.h>
#include "code.h"

#include <setjmp.h>

#include "config.h"
#if USE_DLMALLOC
#include "malloc/dlmalloc.h"
#endif
#include "mempool.h"
#include "memcontext.h"
#include "code.h"
#include "value.h"

typedef struct OpCode OpCode;

typedef struct execctx {
	Value* Object_prototype;
	Value* Function_prototype_prototype;
	Value* Function_prototype;
	Value* String_prototype;
	Value* Number_prototype;
	Value* Boolean_prototype;
	Value* Array_prototype;
	Value* RegExp_prototype;
	Value* Top_object;
	Value* File_prototype;

	/* stacks */
	Value stack[65536];

	/* with a function call, opcode chthis will make obj_this assign to new this */
	Value obj_this[65536];

	int sp;

	struct {
		OpCodes* opcodes;
		OpCode* ip;
	} callstack[1024];
	int callstacksp;

	jmp_buf jmpbuf[64];
	int jmpbufsp;
	memcontext *memcontext;
} execctx; /* execution context */

/* Program state(context) */
typedef struct PSTATE {
	int err_count;				/* error count after parse */
	int eval_flag;				/* currently execute in eval function */
 	OpCodes* opcodes;	/* opcodes result(parsing result) */
	Lexer* lexer;		/* seq provider */

	int _context_id;			/* used in FastVar-locating */
	Value last_exception;		/* exception */

	execctx ec;			/* execution context */

	char buf[1024];
} PSTATE;

PSTATE* pstate_new_from_file(FILE* fp, memcontext* memcontext, char*);
PSTATE* pstate_new_from_string(const char* str, memcontext* memcontext, char*);
void pstate_free(PSTATE* ps);

#define Object_prototype ps->ec.Object_prototype
#define Function_prototype_prototype ps->ec.Function_prototype_prototype
#define Function_prototype ps->ec.Function_prototype
#define String_prototype ps->ec.String_prototype
#define Number_prototype ps->ec.Number_prototype
#define Boolean_prototype ps->ec.Boolean_prototype
#define Array_prototype ps->ec.Array_prototype
#define RegExp_prototype ps->ec.RegExp_prototype
#define Top_object ps->ec.Top_object
#define File_prototype ps->ec.File_prototype

#define psmalloc(size)   mm_alloc(ps,size)
#define psrealloc(p,size)   mm_realloc(ps,p,size)
#define psfree(p) mm_free(ps,p)

#if 0
#if USE_DLMALLOC
#define l_malloc(size)   mspace_malloc(((PSTATE*)ps)->ec.mymspace, size)
#define l_realloc(mem, size)   mspace_realloc(((PSTATE*)ps-)>ec.mymspace, me, size)
#define l_free(p) mspace_free(((PSTATE*)ps)->ec.mymspace, p)
#else
#define l_malloc(size)   malloc(size)
#define l_realloc(mem, size)  realloc(mem,size)
#define l_free(p) free(p)
#endif
#endif

void pstate_svc(void* ps, int i, char* s);
char* pstate_getbuf(void* p);

void* _psdxmalloc (void* ps, int size);
void _psdxfree (void* ps, void *p);

#endif
