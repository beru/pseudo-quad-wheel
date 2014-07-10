#ifndef __FUNC_H__
#define __FUNC_H__

#include "scope.h"
#include "code.h"

typedef struct PState PState;
typedef struct OpCodes OpCodes;
typedef struct Value Value;

/* System func callback type */
typedef int (*SSFunc)(PState* ps, Value* args, Value* _this, Value* ret, int asconstructor);

/* raw function data, with script function or system SSFunc */
typedef struct Func {
	enum {
		FC_NORMAL,
		FC_BUILTIN
	} type;							/* type */
	union {
		OpCodes* opcodes;			/* FC_NORMAL, codes of this function */
		SSFunc callback;			/* FC_BUILTIN, callback */
	} exec;
	strs* argnames;					/* FC_NORMAL, argument names */
	strs* localnames;				/* FC_NORMAL, local var names */
} Func;

Func* func_make_static(PState*, strs* args, strs* localvar, OpCodes* ops);

/* Make a function value from SSFunc */
Value* func_utils_make_func_value(PState*, SSFunc callback);
void func_init_localvar(PState*, Value* arguments, Func* who);

#endif
