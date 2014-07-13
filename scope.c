#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "error.h"
#include "scope.h"
#include "pstate.h"

#define MAX_SCOPE	4096

strs* strs_new (void* ps)
{
	strs* ret = psmalloc(sizeof(strs));
	memset(ret, 0, sizeof(strs));
	return ret;
}

void strs_push(void* ps, strs* ss, const unichar* string)
{
	if (ss->count >= ss->_size) {
		ss->_size += 5;
		ss->strings = psrealloc(ss->strings, (ss->_size) * sizeof(unichar*));
	}
	ss->strings[ss->count] = unistrdup(ps, string);
	ss->count++;
}

strs* strs_dup(void* ps, strs* ss)
{
	strs* n = strs_new(ps);
	if (!ss) {
		return n;
	}
	for (int i=0; i<ss->count; ++i) {
		strs_push(ps, n, ss->strings[i]);
	}
	return n;
}

const unichar* strs_get(void* ps, strs* ss, int i)
{
	if (i < 0 || i >= ss->count) {
		return NULL;
	}
	return ss->strings[i];
}

void strs_free (void* ps, strs* ss)
{
	if (!ss) {
		return;
	}
	for (int i=0; i<ss->count; ++i) {
		unifree(ps, ss->strings[i]);
	}
	psfree(ss->strings);
	psfree(ss);
}

// lexical scope

#include "lexer.h"
#define scopes lex->lc.scopes
#define cur_scope lex->lc.cur_scope

void scope_push (Lexer* lex)
{
	if (cur_scope >= MAX_SCOPE - 1) {
		lexbug("Scope chain to short\n");
	}
	cur_scope++;
}

void scope_pop(Lexer* lex)
{
	if (cur_scope <= 0) {
		lexbug("No more scope to pop\n");
	}
	strs_free(lex->pstate, scopes[cur_scope]);
	scopes[cur_scope] = NULL;
	cur_scope--;
}

void scope_add_var(Lexer* lex, const unichar* str)
{
	if (scopes[cur_scope] == NULL) {
		scopes[cur_scope] = strs_new(lex->pstate);
	}
	for (int i=0; i<scopes[cur_scope]->count; ++i) {
		if (unistrcmp(str, scopes[cur_scope]->strings[i]) == 0) {
			return;
		}
	}
	strs_push(lex->pstate, scopes[cur_scope], str);
}

strs* scope_get_varlist(Lexer* lex)
{
	return strs_dup(lex->pstate, scopes[cur_scope]);
}

