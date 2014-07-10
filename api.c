#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "pstate.h"
#include "parser.h"
#include "regexp.h"
#include "code.h"
#include "value.h"
#include "eval.h"
#include "func.h"
#include "utils.h"
#include "proto.h"
#include "filesys.ex.h"
#include "error.h"
#include "mempool.h"

static
unsigned short*
cstr2uniname (char* p, unsigned short* q)
{
	int len = strlen (p);
	*(int*) q = len;
	q += 2;
	unsigned short* ret = q;
	while (*p) {
		*q++ = *p++;
	}
	*q = 0;
	return ret;
}

static
char*
uniname2cstrdup (void* ps, unsigned short* p)
{
	int len = unistrlen (p);
	char* q = psmalloc (len + 1);
	char* ret = q;
	int i = 0;
	while (i < len) {
		*q++ = *p++;
		i++;
	}
	*q = 0;
	return ret;
}

// get value

int
js_utils_get_var_int_value (PState* ps, Value* csc, unichar* unistr, int* d)
{
	ObjKey* nk = objkey_new (ps, unistr, OM_READONLY);
	Value* v = value_object_lookup (csc, nk, 0);
	if (v != 0 && v->vt == VT_BOOL) {
		*d = v->d.val;
		return 0;
	}
	return -1;
}

int
js_utils_get_var_double_value (PState* ps, ScopeChain* sc, Value* csc, unichar* unistr, double* d)
{
	ObjKey* nk = objkey_new (ps, unistr, OM_READONLY);
	Value* v = value_object_lookup (csc, nk, 0);
	if (v == 0 && sc) {
		v = scope_chain_object_lookup (ps,sc, nk);
	}
	if (v != 0 && v->vt == VT_NUMBER) {
		*d = v->d.num;
		return 0;
	}
	return -1;
}

int
js_utils_get_var_userdata_value (PState* ps, Value* csc, unichar* unistr, udid userdataid, void** d)
{
	ObjKey* nk = objkey_new (ps, unistr, OM_READONLY);
	Value* v = value_object_lookup (csc, nk, 0);
	if (v != 0 && v->vt == VT_OBJECT) {
		*d = userdata_get (ps,v->d.obj, userdataid);
		return 0;
	}
	return -1;
}

int
js_utils_get_var_string_value (PState* ps, Value* csc, unichar* unistr, unsigned char** d)
{
	ObjKey* nk = objkey_new (ps, unistr, OM_READONLY);
	Value* v = value_object_lookup (csc, nk, 0);
	if (v != 0 && v->vt == VT_STRING) {
		*d = uniname2cstrdup (ps,v->d.str);
		return 0;
	}

	return -1;
}


// load


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "pstate.h"
#include "parser.h"
#include "regexp.h"
#include "code.h"
#include "value.h"
#include "eval.h"
#include "func.h"
#include "utils.h"
#include "proto.h"
#include "filesys.ex.h"
#include "error.h"
#include "mempool.h"
#include "memcontext.h"

extern int yyparse (PState* ps);

typedef struct js_context
{
	Value* scope;
	ScopeChain* scopechain;
	memcontext* memcontext;
	execctx ec;
	PState* pstate;
} js_context;

js_context*
js_context_new ()
{
	js_context* ctx = malloc (sizeof (js_context));
	return ctx;
}

void
js_context_free (js_context* p)
{
	free (p);
}

int
js_context_exec_first (js_context* globalcontext, int argc, char** argv, char* program)
{
	// subsystem init
	memcontext* mc = malloc (sizeof (memcontext));
	globalcontext->memcontext = mc;
#if USE_DLMALLOC
	mc->mymspace = create_mspace (0, 0);
#endif

	mpool_init (mc);		// general mempool
	objects_init (mc);
	reg_init(&mm_alloc, &mm_free);

	PState* ps = pstate_new_from_string (program, mc, "-");
	int jmpbufsp = ps->ec.jmpbufsp;
	if (setjmp (ps->ec.jmpbuf[ps->ec.jmpbufsp++]) == 0) {
		yyparse (ps);

		if (!ps->err_count) {
			// current scope, also global
			Value* csc = value_new (ps);
			globalcontext->scope = csc;

			value_make_object (*csc, object_new (ps));

			// top this and prototype chain
			proto_init (ps, csc);

			// global funtion, debugger, etc
			utils_init (ps, csc, argc, argv);

			// file system extern init
			filesys_init (ps, csc);
			load_ex_init (ps, csc);

			// initial scope chain, nothing
			ScopeChain* gsc = scope_chain_new (ps, 0);
			globalcontext->scopechain = gsc;

#ifdef DEBUG
			codes_print (ps->opcodes);
			printf ("------------------------\n");
#endif
			Value ret;
			if (eval (ps, ps->opcodes, gsc, csc, csc, &ret)) {
				die ("Uncatched error");
			}else {
				if (ps->ec.sp != 0) {
					bug ("Stack not ballence after execute script\n");
				}
			}
			//              scope_chain_free(gsc);
			//              value_free(csc);
		}
		ps->ec.jmpbufsp = jmpbufsp;
	}else {
		ps->ec.jmpbufsp = jmpbufsp;
	}
	memcpy (&globalcontext->ec, &ps->ec, sizeof (execctx));
	pstate_free (ps);
	return 0;
}

int
js_context_exec_next (js_context* globalcontext, char* program)
{
	Value* csc = globalcontext->scope;
	ScopeChain* gsc = globalcontext->scopechain;
	
	PState* ps = pstate_new_from_string (program, globalcontext->memcontext, "-");
	memcpy (&ps->ec, &globalcontext->ec, sizeof (execctx));
	int jmpbufsp = ps->ec.jmpbufsp;
	if (setjmp (ps->ec.jmpbuf[ps->ec.jmpbufsp++]) == 0) {
		yyparse (ps);

		if (!ps->err_count) {
			Value ret;
			if (eval (ps, ps->opcodes, gsc, csc, csc, &ret)) {
				die ("Uncatched error");
			}else {
				if (ps->ec.sp != 0) {
					bug ("Stack not ballence after execute script\n");
				}
			}
		}
		ps->ec.jmpbufsp = jmpbufsp;
	}else {
		ps->ec.jmpbufsp = jmpbufsp;
	}
	pstate_free (ps);
	return 0;
}


void
js_context_get_var_int_value (js_context* glbl, char* name, int* val)
{
	unsigned short nameunistr[256];
	unsigned short* uniname = cstr2uniname (name, nameunistr);
	if (js_utils_get_var_int_value (glbl->pstate,glbl->scope, uniname, val) == 0) {
		return;
	}
	*val = 0;
}

void
js_context_get_var_double_value (js_context* glbl, char* name, double* val)
{
	unsigned short nameunistr[256];
	unsigned short* uniname = cstr2uniname (name, nameunistr);
	if (js_utils_get_var_double_value (glbl->pstate,glbl->scopechain, glbl->scope, uniname, val) == 0) {
		return;
	}
	*val = 0;
}

void
js_context_get_var_string_value (js_context* glbl, char* name, unsigned char** val)
{
	unsigned short nameunistr[256];
	unsigned short* uniname = cstr2uniname (name, nameunistr);
	if (js_utils_get_var_string_value (glbl->pstate,glbl->scope, uniname, val) == 0) {
		return;
	}
	*val = 0;
}

int
js_context_done (js_context* globalcontext)
{
	scope_chain_free (globalcontext->pstate, globalcontext->scopechain);
	value_free (globalcontext->pstate, globalcontext->scope);
}

#if 0
// slot
#include <pthread.h>

static pthread_mutex_t mutex;
void
js_utils_interp_lock_init ()
{
	pthread_mutex_init (&mutex, NULL);
}

void
js_utils_interp_lock ()
{
	pthread_mutex_lock (&mutex);
}

void
js_utils_interp_unlock ()
{
	pthread_mutex_unlock (&mutex);
}

/*
struct slot
{
  int used;
  union
  {
    void *p;
    int n;
    double d;
  } u;
};
#define N_SLOT 32
static struct slot slots[N_SLOT];
static pthread_mutex_t mutex;

void
js_utils_slot_init ()
{
  pthread_mutex_init (&mutex, NULL);
}

int
js_utils_slot_alloc ()
{
  int r = -1;
	int i;
  pthread_mutex_lock (&mutex);
  for (i=0; i<N_SLOT; i++)
    {
      if (slots[i].used == 0)
	{
	  slots[i].used = 1;
	  r = i;
	  break;
	}
    }
  pthread_mutex_unlock (&mutex);
  return r;
}

int
js_utils_slot_free (int idx)
{
  int r = -1;
  pthread_mutex_lock (&mutex);
  if (0 <= idx && idx < N_SLOT)
    {
      if (slots[idx].used == 1)
	r = idx;
      slots[idx].used = 0;
    }
  pthread_mutex_unlock (&mutex);
  return r;
}

int
js_utils_slot_put_pointer (int idx, void *p)
{
  slots[idx].u.p = p;
  return idx;
}

void *js_utils_slot_get_pointer (int idx)
{
  return slots[idx].u.p;
}
*/
#endif
