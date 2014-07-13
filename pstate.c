#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "pstate.h"
#include "mempool.h"

static
PState* pstate_new(void* mc)
{
	PState* ps = mc_malloc(mc, sizeof(PState));
	memset(ps, 0, sizeof(PState));
	((PState*)ps)->ec.memcontext = mc;
	return ps;
}

PState* pstate_new_from_file(FILE* fp, memcontext* mc, char* codename)
{
	PState* ps = pstate_new(mc);
	Lexer* l= mm_alloc(ps, sizeof (Lexer));
	memset(l, 0, sizeof(Lexer));
	ps->lexer = l;
	l->pstate = ps;
	l->ltype = LT_FILE;
	l->d.fp = fp;
	l->codename = codename ? _strdup(codename): codename;
	rewind(fp);
	l->cur_line = 1;
	return ps;
}

PState* pstate_new_from_string(const char* str, memcontext* mc, char* codename)
{
	PState* ps = pstate_new(mc);
	Lexer* l= psmalloc(sizeof(Lexer));
	memset(l, 0, sizeof(Lexer));
	ps->lexer = l;
	l->pstate = ps;
	l->ltype = LT_STRING;
	l->d.str = str;
	l->cur_line = 1;
	l->codename = codename ? _strdup(codename): codename;
	return ps;
}

void pstate_free(PState* ps)
{
	// todo: free opcodes
	psfree(ps->lexer);

	mc_free(((PState*)ps)->ec.memcontext, ps);
}

/*
void** get_general_pools(void* ps)
{
#if USE_DLMALLOC
	if (((PState*) ps)->ec == 0) return 0;
	return (((PState*)ps)->ec.general_pools);
#else
	return 0;
#endif
}*/

void* _ps_memctx_malloc(void* ps, int size)
{
	if (((PState*)ps)->ec.memcontext == 0) return malloc(size);
	return mc_malloc(((PState *) ps)->ec.memcontext, size);
}

void _ps_memctx_free(void* ps, void* p)
{
	if (((PState*)ps)->ec.memcontext == 0) {
		free(p);
		return;
	}else {
		mc_free(((PState*)ps)->ec.memcontext, p);
	}
}

void pstate_svc(void* ps, int i, char* s)
{
	PState* p = (PState*)ps;
	int sp = p->ec.callstacksp-1;
	int count = 0;

	if (sp < 0) {
		printf("%s:%d:%c:%s\n", "-", 0, i, s); 
	}

	while (sp >= 0) {
		OpCode* ip = p->ec.callstack[sp].ip;
		OpCodes* opcodes = p->ec.callstack[sp].opcodes;
		OpCode* end = &opcodes->codes[opcodes->code_len];
		char* codename = 0; 
		int no = 0; 
		while (ip < end) {
			if (ip->lineno > 0) {
				codename = ip->codename, no = ip->lineno; 
				break;
			} 
			ip++;
		} 
		if (count == 0) {
			printf("%s:%d:%c:%s\n", codename, no, i, s); 
		}else {
			printf("\t%s:%d\n", codename, no);
		}
		sp--;
		count++;
	}

	if (((PState*)ps)->ec.jmpbufsp <= 0) {
		abort();
	}
	longjmp(((PState*)ps)->ec.jmpbuf[((PState*)ps)->ec.jmpbufsp-1], 1);
}


void** pstate_get_general_pools(void* ec)
{
	PState* ps = ec;
	if (((PState*)ps)->ec.memcontext == 0) {
		return 0;
	}
	return ps->ec.memcontext->general_pools;
}

char* pstate_getbuf(void* p)
{
	PState* ps = p;
	return ps->buf;
}

