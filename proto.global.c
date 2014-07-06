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
#include "pstate.h"
#include "utils.h"

static int
global_exit (PSTATE * ps, Value * args, Value * _this, Value * ret, int asc)
{
  int err = 0;
  if (asc)
    die ("Can not call exit as a constructor\n");
  if (value_get_length (args) > 0)
    {
      Value *v = value_object_lookup_array (args, 0, NULL);
      if (v && is_number (v))
	err = (int) v->d.num;
    }
  exit (err);
  return 0;
}

int
global_print (PSTATE * ps, Value * args, Value * _this, Value * ret, int asc);

static int
global_gc (PSTATE * ps, Value * args, Value * _this, Value * ret, int asc)
{
  return 0;
}

void
proto_global_init (PSTATE * ps, Value * global, int argc, char **argv)
{
  value_object_utils_insert (ps, global, tounichars (ps, "exit"),
			     func_utils_make_func_value (ps, global_exit), 0,
			     0, 0);

  /* for testing */
  value_object_utils_insert (ps, global, tounichars (ps, "print"),
			     func_utils_make_func_value (ps, global_print), 0,
			     0, 0);
  value_object_utils_insert (ps, global, tounichars (ps, "gc"),
			     func_utils_make_func_value (ps, global_gc), 0,
			     0, 0);

}
