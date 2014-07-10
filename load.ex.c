#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "pstate.h"
#include "error.h"
#include "value.h"
#include "func.h"
#include "proto.h"
#include "eval.h"

int utils_global_eval (
	PSTATE* ps,
	const char* program,
	ScopeChain* scope,
	Value* currentScope,
	Value* _this,
	Value* ret,
	const char* codename
);

#include <sys/stat.h>

int
_utils_global_load (
	PSTATE* ps, 
	const char* fn,
	ScopeChain* sc, 
	Value* _this, 
	Value* ret,
	int asc
	)
{
	FILE* fp = fopen (fn, "rb");
	if (fp == 0) {
		return -2;
	}
	
	struct stat st;
	stat (fn, &st);
	char* program = psmalloc (st.st_size + 1);
	fread (program, 1, st.st_size, fp);
	fclose (fp);
	
	program[st.st_size] = 0;
	int r = utils_global_eval (ps, program, sc, _this, _this, ret, fn);
	psfree (program);
	return r;
}


int
_utils_global_require (
	PSTATE* ps, 
	const char* mn,
	ScopeChain* sc, 
	Value* _this, 
	Value* ret,
	int asc
	)
{
	char fn[1024];
	sprintf(fn, "modules/%s.js", mn);
	FILE* fp =  fopen (fn, "rb");;
	if (fp == 0) {
		return -2;
	}
	
	struct stat st;
	stat (fn, &st);
	char* program = psmalloc (st.st_size + 1);
	fread (program, 1, st.st_size, fp);
	fclose (fp);
	program[st.st_size] = 0;

	char* wrap = psmalloc (st.st_size + 1 + 256);
	sprintf(wrap, "return (function(){\nvar exports = {};\nvar module = {};\nmodule.exports = exports;\n%s;\nreturn module.exports;\n})();\n", program);
	int r = utils_global_eval (ps, wrap, sc, _this, _this, ret, fn);
	psfree (program);
	return r;
}

int
utils_global_load (
	PSTATE* ps, 
	Value* args, 
	Value* _this, 
	Value* ret,
	int asc
	)
{
	Value* v = value_object_lookup_array (args, 0, NULL);
	if (v && is_string (v)) {
		// nothing
		;
	}else {
		return -1;
	}

	{
		const char* fn = tochars (ps, v->d.str);
		ScopeChain* gsc = scope_chain_new (ps, 0);
		int r = _utils_global_load (ps, fn, gsc, _this, ret, 0);
		scope_chain_free (ps, gsc);
		return r;
	}
}

int
utils_global_require (
	PSTATE* ps,
	Value* args,
	Value* _this,
	Value* ret,
	int asc
	)
{
	Value* v = value_object_lookup_array (args, 0, NULL);
	if (v && is_string (v)) {
		// nothing
		;
	}else {
		return -1;
	}

	{
		const char *mn = tochars (ps, v->d.str);
		ScopeChain* gsc = scope_chain_new (ps, 0);
		int r = _utils_global_require (ps, mn, gsc, _this, ret, 0);
		scope_chain_free (ps, gsc);
		return r;
	}
}

#define GLOBAL_LOAD 1

#if GLOBAL_LOAD
void
load_ex_init (PSTATE* ps, Value* global)
{
	value_object_utils_insert2 (ps, global, "load", func_utils_make_func_value (ps,utils_global_load), 0, 0, 0);
	value_object_utils_insert2 (ps, global, "include", func_utils_make_func_value (ps,utils_global_load), 0, 0, 0);
	value_object_utils_insert2 (ps, global, "require", func_utils_make_func_value (ps,utils_global_require), 0, 0, 0);
}
#endif

