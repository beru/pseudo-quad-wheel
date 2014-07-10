#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "func.h"
#include "error.h"
#include "value.h"
#include "scope.h"
#include "unichar.h"
#include "pstate.h"

Func*
func_make_static (PState* ps, strs* args, strs* localvar, OpCodes* ops)
{
	Func* f = psmalloc (sizeof (Func));
	memset (f, 0, sizeof (Func));
	f->type = FC_NORMAL;
	f->exec.opcodes = ops;
	f->argnames = args;
	f->localnames = localvar;
	return f;
}

void
func_init_localvar (PState* ps, Value* arguments, Func* who)
{
	if (who->localnames) {
		for (int i=0; i<who->localnames->count; ++i) {
			const unichar* argkey = strs_get (ps, who->localnames, i);
			if (argkey) {
				ObjKey* strkey = objkey_new (ps, argkey, OM_DONTEMU);
				value_object_insert (ps, arguments, strkey, value_new (ps));
			}
		}
	}
}

static FuncObj*
func_make_internal (PState* ps, SSFunc callback)
{
	Func* f = psmalloc (sizeof (Func));
	memset (f, 0, sizeof (Func));
	f->type = FC_BUILTIN;
	f->exec.callback = callback;
	return funcobj_new (ps, f);
}

Value*
func_utils_make_func_value (PState* ps, SSFunc callback)
{
	Object* o = object_new (ps);
	Value* v;
	o->ot = OT_FUNCTION;
	o->d.fobj = func_make_internal (ps, callback);

	v = value_new (ps);
	value_make_object (*v, o);
	return v;
}

