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

#include <sys/stat.h>

extern int yyparse (PState* ps);

int
Usage ()
{
	fprintf (stderr, "Usage: smallscript [input file] [arguments]\n");
	return -1;
}

int
_utils_global_load (
	PState* ps,
    const char* fn,
    ScopeChain* sc,
    Value* _this,
    Value* ret,
    int asc
);

int
main (int argc, char** argv)
{
	FILE* input = stdin;
	PState* ps;
	struct memcontext* mc;
	int fid = 0;

	argv++;
	argc--;

	/* subsystem init */
	mc = malloc (sizeof (struct memcontext));
	mc->mymspace = (void*)create_mspace (0, 0);

	mpool_init (mc);		/* general mempool */
	reg_init(mm_alloc, mm_free);
	objects_init (mc);

	ps = pstate_new_from_string("", mc, "-");
	{
		Value ret;
		ScopeChain* gsc;
		/* current scope, also global */
		Value* csc = value_new (ps);
		value_make_object (*csc, object_new (ps));

		/* top this and prototype chain */
		proto_init (ps, csc);

		/* global funtion, debugger, etc */
		utils_init (ps, csc, argc, argv);
		proto_global_init (ps, csc);
		load_ex_init (ps, csc);

		/* file system extern init */
		filesys_init (ps, csc);

		/* initial scope chain, nothing */
		gsc = scope_chain_new (ps, 0);

		while (argc > 0) {
			_utils_global_load (ps, argv[fid], gsc, csc, &ret, 0);
			fid++;
			argc--;
		}

		scope_chain_free (ps, gsc);
		value_free (ps, csc);
	}

	pstate_free (ps);
	return 0;
}

