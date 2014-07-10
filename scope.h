#ifndef __SCOPE_H__
#define __SCOPE_H__

#include "unichar.h"

typedef struct strs {
	unichar** strings;
	int count;
	int _size;
} strs;

strs* strs_new(void* ec);
void strs_push(void* ec, strs* ss, const unichar* string);
void strs_free(void* ec,strs* ss);
const unichar* strs_get(void* ec, strs* ss, int i);

/* what lexical scope means:
 * --------------
 * var a;						// this is first level of scope
 * function foo() {				// parsing function make lexical scope to push
 *     var b;					// this is the second level
 *     var c = function() {		// push again
 *         var d;				// third level
 *     }						// end of an function, pop scope
 * }							// return to first scope
 * --------------
 */
#define MAX_SCOPE	4096

typedef struct Lexer Lexer;

void scope_push(Lexer* lexer);
void scope_pop(Lexer* lexer);
void scope_add_var(Lexer* lexer, const unichar* str);
strs* scope_get_varlist(Lexer* lexer);

#endif
