#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pstate.h"
#include "error.h"
#include "value.h"
#include "proto.h"

static struct st_numpro_tab
{
	const char* name;
	SSFunc func;
} numpro_funcs[] = {
	{"", 0}
};

void
proto_number_init (PState* ps, Value* global)
{
	if (!Number_prototype) {
		bug ("proto init failed?");
	}
	for (int i=0; i<sizeof (numpro_funcs) / sizeof (struct st_numpro_tab); ++i) {
		Value* n = func_utils_make_func_value (ps, numpro_funcs[i].func);
		n->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, Number_prototype, numpro_funcs[i].name, n, 0, 0, 0);
	}
	Value* NaN = value_new (ps);
	value_make_number (*NaN, ieee_makenan ());
	Value* Inf = value_new (ps);
	value_make_number (*Inf, ieee_makeinf (1));
	value_object_utils_insert2 (ps, global, "NaN", NaN, 0, 0, 0);
	value_object_utils_insert2 (ps, global, "Infinity", Inf, 0, 0, 0);
}

