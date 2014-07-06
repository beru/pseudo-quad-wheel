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

extern int yyparse (PSTATE * ps);

int
Usage ()
{
  fprintf (stderr, "Usage: smallscript [input file] [arguments]\n");
  return -1;
}

int
main (int argc, char **argv)
{
  FILE *input = stdin;
  PSTATE *ps;
  struct memcontext *mc;
  int fid = 0;

  argv++;
  argc--;

  if (argc > 0)
    {
      input = fopen (argv[0], "r");
      if (!input)
	{
	  fprintf (stderr, "Can not open '%s'\n", argv[0]);
	  return Usage ();
	}
      argv++;
      argc--;
    }

  /* subsystem init */
  mc = malloc (sizeof (struct memcontext));
  mc->mymspace = create_mspace(0, 0);

  mpool_init (mc);		/* general mempool */
  reg_init(mm_alloc, mm_free);
  objects_init (mc);


  ps = pstate_new_from_file (input, mc, argv[0]);
  yyparse (ps);

  if (!ps->err_count)
    {
      Value ret;
      ScopeChain *gsc;

      /* current scope, also global */
      Value *csc = value_new (ps);
      value_make_object (*csc, object_new (ps));

      /* top this and prototype chain */
      proto_init (ps, csc);

      /* global funtion, debugger, etc */
      utils_init (ps, csc, argc, argv);
      proto_global_init (ps, csc, argc, argv);
      load_ex_init (ps, csc);

      /* file system extern init */
      filesys_init (ps, csc);

      /* initial scope chain, nothing */
      gsc = scope_chain_new (ps, 0);

#ifdef DEBUG
      codes_print (ps, ps->opcodes);
      printf ("------------------------\n");
#endif
      if (eval (ps, ps->opcodes, gsc, csc, csc, &ret))
	{
	  die ("Uncaught error");
	}
      else
	{
	  if (ps->ec.sp != 0)
	    {
	      bug ("Stack invalid after execute script\n");
	    }
	}
      scope_chain_free (ps, gsc);
      value_free (ps, csc);
    }
  fclose (input);
  pstate_free (ps);
  return 0;
}
