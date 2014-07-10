#ifndef __FUNC_H__
#define __FUNC_H__

#include "scope.h"
#include "code.h"

typedef struct PSTATE PSTATE;
typedef struct OpCodes OpCodes;
typedef struct Value Value;

/* System func callback type */
typedef int (*SSFunc)(PSTATE* ps, Value* args, Value* _this, Value* ret, int asconstructor);

/* raw function data, with script function or system SSFunc */
typedef struct Func {
	enum {
		FC_NORMAL,
		FC_BUILDIN
	} type;							/* type */
	union {
		OpCodes* opcodes;	/* FC_NORMAL, codes of this function */
		SSFunc callback;			/* FC_BUILDIN, callback */
	} exec;
	strs* argnames;					/* FC_NORMAL, argument names */
	strs* localnames;				/* FC_NORMAL, local var names */
} Func;

Func* func_make_static(PSTATE*, strs* args, strs* localvar, OpCodes* ops);

/* Make a function value from SSFunc */
Value* func_utils_make_func_value(PSTATE*, SSFunc callback);
void func_init_localvar(PSTATE*, Value* arguments, Func* who);

#endif
