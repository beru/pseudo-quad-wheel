/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 1 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"
#include "pstate.h"
#include "func.h"
#include "scope.h"
#include "lexer.h"
#include "parser.h"
#include "unichar.h"

typedef struct ArgList {
	unichar *argname;
	struct ArgList *tail;
	struct ArgList *next;
} ArgList;

static ArgList *arglist_new(PSTATE*ps,const unichar *name)
{
  ArgList *a = psmalloc(sizeof(ArgList));
	memset(a, 0, sizeof(ArgList));
	a->argname = unistrdup(ps,name);
	a->tail = a;
	return a;
}

static ArgList *arglist_insert(PSTATE*ps,ArgList *a, const unichar *name)
{
  ArgList *b = psmalloc(sizeof(ArgList));
	memset(b, 0, sizeof(ArgList));
	b->argname = unistrdup(ps,name);
	a->tail->next = b;
	a->tail = b;
	return a;
}

typedef struct ForinVar {
	unichar *varname;
	OpCodes *local;
	OpCodes *lval;
} ForinVar;

ForinVar *forinvar_new(PSTATE*ps,unichar *varname, OpCodes *local, OpCodes *lval)
{
  ForinVar *r = psmalloc(sizeof(ForinVar));
	r->varname = varname;
	r->local = local;
	r->lval = lval;
	return r;
}

static OpCodes *make_forin(PSTATE*ps,OpCodes *lval, OpCodes *expr, OpCodes *stat, const unichar *label)
{
  OpCodes *init = codes_join(ps,expr, code_key(ps));
	OpCodes *cond = codes_join3(ps,lval, code_next(ps),
				    code_jfalse(ps,stat->code_len + 2));
	OpCodes *stat_jmp = code_jmp(ps, -(cond->code_len + stat->code_len));
	code_reserved_replace(ps,stat, 1, 0, label, 2);
	return codes_join3(ps,codes_join(ps,init, cond), 
			   codes_join(ps,stat, stat_jmp), code_pop(ps,2));
}

typedef struct CaseExprStat {
	OpCodes *expr;
	OpCodes *stat;
	int isdefault;
} CaseExprStat;

CaseExprStat *exprstat_new(PSTATE*ps,OpCodes *expr, OpCodes *stat, int isdef)
{
  CaseExprStat *r = psmalloc(sizeof(CaseExprStat));
	r->expr = expr;
	r->stat = stat;
	r->isdefault = isdef;
	return r;
}

typedef struct CaseList {
	CaseExprStat *es;
	int off;
	struct CaseList *tail;
	struct CaseList *next;
} CaseList;

static CaseList *caselist_new(PSTATE*ps,CaseExprStat *es)
{
  CaseList *a = psmalloc(sizeof(CaseList));
	memset(a, 0, sizeof(CaseList));
	a->es = es;
	a->tail = a;
	return a;
}

static CaseList *caselist_insert(PSTATE*ps,CaseList *a, CaseExprStat *es)
{
  CaseList *b = psmalloc(sizeof(CaseList));
	memset(b, 0, sizeof(CaseList));
	b->es = es;
	a->tail->next = b;
	a->tail = b;
	return a;
}

static OpCodes *opassign(PSTATE*ps,OpCodes *lval, OpCodes *oprand, OpCodes *op)
{
	OpCodes *ret;
	if (((OpCodes *)lval)->lvalue_flag == 1) {
		ret = codes_join3(ps,lval, 
							 codes_join3(ps,code_push_top(ps), oprand, op),
				  code_assign(ps,1));
	} else {
		ret = codes_join3(ps,lval,
				  codes_join4(ps,code_push_top2(ps), code_subscript(ps,1), oprand, op),
				  code_assign(ps,2));
	}
	return ret;
}

#define yyfilename ps->lexer->codename
#define yylineno ps->lexer->cur_line



/* Line 268 of yacc.c  */
#line 197 "parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     IDENTIFIER = 259,
     IF = 260,
     ELSE = 261,
     FOR = 262,
     IN = 263,
     WHILE = 264,
     DO = 265,
     CONTINUE = 266,
     SWITCH = 267,
     CASE = 268,
     DEFAULT = 269,
     BREAK = 270,
     FUNC = 271,
     RETURN = 272,
     LOCAL = 273,
     NEW = 274,
     DELETE = 275,
     TRY = 276,
     CATCH = 277,
     FINALLY = 278,
     THROW = 279,
     WITH = 280,
     UNDEF = 281,
     _TRUE = 282,
     _FALSE = 283,
     _THIS = 284,
     ARGUMENTS = 285,
     FNUMBER = 286,
     REGEXP = 287,
     __DEBUG = 288,
     MIN_PRI = 289,
     ARGCOMMA = 290,
     DIVAS = 291,
     BXORAS = 292,
     BORAS = 293,
     BANDAS = 294,
     URSHFAS = 295,
     RSHFAS = 296,
     LSHFAS = 297,
     MODAS = 298,
     MULAS = 299,
     MNSAS = 300,
     ADDAS = 301,
     OR = 302,
     AND = 303,
     NNEQ = 304,
     EEQU = 305,
     NEQ = 306,
     EQU = 307,
     INSTANCEOF = 308,
     GEQ = 309,
     LEQ = 310,
     URSHF = 311,
     RSHF = 312,
     LSHF = 313,
     VOID = 314,
     TYPEOF = 315,
     DEC = 316,
     INC = 317,
     NEG = 318,
     MAX_PRI = 319
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 316 "parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  100
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1867

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  166
/* YYNRULES -- Number of states.  */
#define YYNSTATES  335

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    73,     2,     2,     2,    72,    55,     2,
      82,    87,    70,    68,    35,    69,    80,    71,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    50,    84,
      61,    37,    60,    49,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    81,     2,    88,    54,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    85,    53,    86,    74,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      36,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    51,    52,    56,    57,    58,    59,    62,    63,
      64,    65,    66,    67,    75,    76,    77,    78,    79,    83
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     9,    11,    13,    16,    18,
      20,    24,    27,    29,    31,    35,    39,    43,    46,    50,
      54,    56,    58,    60,    64,    66,    72,    75,    77,    79,
      81,    83,    84,    86,    87,    90,    92,    95,   101,   109,
     118,   120,   122,   125,   130,   134,   142,   147,   157,   159,
     163,   165,   169,   173,   179,   187,   197,   206,   207,   210,
     212,   214,   217,   221,   222,   224,   225,   227,   234,   242,
     248,   255,   256,   258,   260,   264,   268,   271,   273,   275,
     277,   281,   286,   290,   293,   296,   299,   302,   305,   308,
     312,   316,   320,   324,   328,   331,   334,   337,   340,   344,
     348,   352,   356,   360,   364,   368,   372,   376,   380,   384,
     388,   392,   396,   400,   404,   408,   412,   416,   420,   424,
     428,   432,   436,   440,   444,   448,   452,   456,   458,   461,
     464,   469,   472,   478,   484,   492,   498,   504,   509,   516,
     524,   531,   536,   538,   540,   542,   547,   551,   552,   554,
     556,   560,   562,   564,   566,   568,   570,   572,   574,   576,
     580,   581,   583,   587,   591,   595,   599
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      90,     0,    -1,    -1,    91,    -1,    91,   121,    -1,   121,
      -1,    92,    -1,    91,    92,    -1,    96,    -1,    93,    -1,
       4,    50,    93,    -1,   121,    84,    -1,   109,    -1,   108,
      -1,    15,    97,    84,    -1,    11,    97,    84,    -1,    17,
     121,    84,    -1,    17,    84,    -1,    18,   106,    84,    -1,
      24,   121,    84,    -1,   105,    -1,   100,    -1,    84,    -1,
      85,    91,    86,    -1,    94,    -1,    95,    82,   118,    87,
     120,    -1,    16,     4,    -1,   110,    -1,   115,    -1,   116,
      -1,   101,    -1,    -1,     4,    -1,    -1,     4,    50,    -1,
      92,    -1,    85,    86,    -1,    25,    82,   102,    87,    99,
      -1,    98,    12,    82,   102,    87,    85,    86,    -1,    98,
      12,    82,   102,    87,    85,   103,    86,    -1,   121,    -1,
     104,    -1,   103,   104,    -1,    13,   102,    50,    91,    -1,
      14,    50,    91,    -1,    21,   120,    22,    82,     4,    87,
     120,    -1,    21,   120,    23,   120,    -1,    21,   120,    22,
      82,     4,    87,   120,    23,   120,    -1,   107,    -1,   106,
      35,   107,    -1,     4,    -1,     4,    37,   121,    -1,    20,
     123,    84,    -1,     5,    82,   102,    87,    99,    -1,     5,
      82,   102,    87,    99,     6,    99,    -1,    98,     7,    82,
     112,   113,    84,   114,    87,    99,    -1,    98,     7,    82,
     111,     8,   121,    87,    99,    -1,    -1,    18,     4,    -1,
     123,    -1,    84,    -1,   102,    84,    -1,    18,   106,    84,
      -1,    -1,   102,    -1,    -1,   121,    -1,    98,     9,    82,
     102,    87,    99,    -1,    98,    10,    99,     9,    82,   102,
      87,    -1,    16,    82,   118,    87,   120,    -1,    16,     4,
      82,   118,    87,   120,    -1,    -1,   119,    -1,     4,    -1,
     119,    35,     4,    -1,    85,    91,    86,    -1,    85,    86,
      -1,   126,    -1,   117,    -1,   123,    -1,   121,    35,   121,
      -1,   121,    81,   121,    88,    -1,   121,    80,     4,    -1,
      69,   121,    -1,    68,   121,    -1,    74,   121,    -1,    73,
     121,    -1,    76,   121,    -1,    75,   121,    -1,   121,    70,
     121,    -1,   121,    71,   121,    -1,   121,    72,   121,    -1,
     121,    68,   121,    -1,   121,    69,   121,    -1,   123,    78,
      -1,   123,    77,    -1,    78,   123,    -1,    77,   123,    -1,
      82,   121,    87,    -1,   121,    52,   121,    -1,   121,    51,
     121,    -1,   121,    61,   121,    -1,   121,    60,   121,    -1,
     121,    64,   121,    -1,   121,    63,   121,    -1,   121,    59,
     121,    -1,   121,    58,   121,    -1,   121,    57,   121,    -1,
     121,    56,   121,    -1,   121,    55,   121,    -1,   121,    53,
     121,    -1,   121,    54,   121,    -1,   121,    67,   121,    -1,
     121,    66,   121,    -1,   121,    65,   121,    -1,   123,    37,
     121,    -1,   123,    48,   121,    -1,   123,    47,   121,    -1,
     123,    46,   121,    -1,   123,    45,   121,    -1,   123,    44,
     121,    -1,   123,    43,   121,    -1,   123,    42,   121,    -1,
     123,    41,   121,    -1,   123,    40,   121,    -1,   123,    39,
     121,    -1,   123,    38,   121,    -1,   122,    -1,    19,   126,
      -1,    19,   123,    -1,    19,    82,   121,    87,    -1,    19,
     117,    -1,    19,   126,    82,   124,    87,    -1,    19,   123,
      82,   124,    87,    -1,    19,    82,   121,    87,    82,   124,
      87,    -1,    19,   117,    82,   124,    87,    -1,   121,    49,
     121,    50,   121,    -1,    33,    82,   121,    87,    -1,   121,
      80,     4,    82,   124,    87,    -1,   121,    81,   121,    88,
      82,   124,    87,    -1,    82,   121,    87,    82,   124,    87,
      -1,   123,    82,   124,    87,    -1,     4,    -1,    30,    -1,
      29,    -1,   123,    81,   121,    88,    -1,   123,    80,     4,
      -1,    -1,   125,    -1,   121,    -1,   125,    35,   121,    -1,
       3,    -1,    26,    -1,    27,    -1,    28,    -1,    31,    -1,
      32,    -1,   127,    -1,   130,    -1,    85,   128,    86,    -1,
      -1,   129,    -1,   128,    35,   129,    -1,     4,    50,   121,
      -1,     3,    50,   121,    -1,    81,   125,    88,    -1,    81,
      88,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   191,   191,   192,   195,   198,   203,   204,   209,   210,
     211,   215,   216,   217,   218,   219,   220,   221,   222,   223,
     224,   225,   226,   227,   228,   231,   242,   251,   252,   253,
     254,   257,   258,   261,   262,   268,   269,   273,   279,   280,
     323,   327,   328,   332,   333,   337,   344,   351,   361,   362,
     366,   375,   387,   395,   399,   408,   419,   436,   437,   440,
     448,   449,   450,   453,   454,   457,   458,   462,   472,   482,
     486,   492,   493,   506,   507,   510,   511,   515,   516,   517,
     521,   522,   523,   524,   525,   526,   527,   528,   529,   530,
     531,   532,   533,   534,   535,   539,   543,   547,   551,   552,
     556,   560,   561,   562,   563,   564,   565,   566,   567,   568,
     569,   570,   571,   572,   573,   574,   575,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   585,   586,   588,   589,
     592,   593,   594,   599,   607,   612,   617,   621,   625,   631,
     637,   642,   662,   667,   668,   669,   674,   681,   682,   686,
     687,   694,   695,   696,   697,   698,   699,   700,   701,   705,
     709,   710,   711,   719,   720,   724,   725
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "STRING", "IDENTIFIER", "IF", "ELSE",
  "FOR", "IN", "WHILE", "DO", "CONTINUE", "SWITCH", "CASE", "DEFAULT",
  "BREAK", "FUNC", "RETURN", "LOCAL", "NEW", "DELETE", "TRY", "CATCH",
  "FINALLY", "THROW", "WITH", "UNDEF", "_TRUE", "_FALSE", "_THIS",
  "ARGUMENTS", "FNUMBER", "REGEXP", "__DEBUG", "MIN_PRI", "','",
  "ARGCOMMA", "'='", "DIVAS", "BXORAS", "BORAS", "BANDAS", "URSHFAS",
  "RSHFAS", "LSHFAS", "MODAS", "MULAS", "MNSAS", "ADDAS", "'?'", "':'",
  "OR", "AND", "'|'", "'^'", "'&'", "NNEQ", "EEQU", "NEQ", "EQU", "'>'",
  "'<'", "INSTANCEOF", "GEQ", "LEQ", "URSHF", "RSHF", "LSHF", "'+'", "'-'",
  "'*'", "'/'", "'%'", "'!'", "'~'", "VOID", "TYPEOF", "DEC", "INC", "NEG",
  "'.'", "'['", "'('", "MAX_PRI", "';'", "'{'", "'}'", "')'", "']'",
  "$accept", "file", "statements", "statement", "comonstatement",
  "func_statement", "func_prefix", "iterstatement", "identifier_opt",
  "label_opt", "statement_or_empty", "with_statement", "switch_statement",
  "condexpr", "cases", "case", "try_statement", "vardecs", "vardec",
  "delete_statement", "if_statement", "for_statement", "for_var",
  "for_init", "for_cond", "expr_opt", "while_statement", "do_statement",
  "func_expr", "args_opt", "args", "func_statement_block", "expr",
  "fcall_exprs", "lvalue", "exprlist_opt", "exprlist", "value", "object",
  "items", "item", "array", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,    44,   290,    61,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,    63,
      58,   302,   303,   124,    94,    38,   304,   305,   306,   307,
      62,    60,   308,   309,   310,   311,   312,   313,    43,    45,
      42,    47,    37,    33,   126,   314,   315,   316,   317,   318,
      46,    91,    40,   319,    59,   123,   125,    41,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    90,    90,    91,    91,    92,    92,
      92,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    94,    95,    96,    96,    96,
      96,    97,    97,    98,    98,    99,    99,   100,   101,   101,
     102,   103,   103,   104,   104,   105,   105,   105,   106,   106,
     107,   107,   108,   109,   109,   110,   110,   111,   111,   111,
     112,   112,   112,   113,   113,   114,   114,   115,   116,   117,
     117,   118,   118,   119,   119,   120,   120,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   122,   122,
     122,   122,   123,   123,   123,   123,   123,   124,   124,   125,
     125,   126,   126,   126,   126,   126,   126,   126,   126,   127,
     128,   128,   128,   129,   129,   130,   130
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     2,     1,     1,     2,     1,     1,
       3,     2,     1,     1,     3,     3,     3,     2,     3,     3,
       1,     1,     1,     3,     1,     5,     2,     1,     1,     1,
       1,     0,     1,     0,     2,     1,     2,     5,     7,     8,
       1,     1,     2,     4,     3,     7,     4,     9,     1,     3,
       1,     3,     3,     5,     7,     9,     8,     0,     2,     1,
       1,     2,     3,     0,     1,     0,     1,     6,     7,     5,
       6,     0,     1,     1,     3,     3,     2,     1,     1,     1,
       3,     4,     3,     2,     2,     2,     2,     2,     2,     3,
       3,     3,     3,     3,     2,     2,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     2,     2,
       4,     2,     5,     5,     7,     5,     5,     4,     6,     7,
       6,     4,     1,     1,     1,     4,     3,     0,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       0,     1,     3,     3,     3,     3,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      33,   151,   142,     0,    31,    31,     0,     0,     0,     0,
       0,     0,     0,     0,   152,   153,   154,   144,   143,   155,
     156,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,    33,     0,    33,     6,     9,    24,     0,
       8,     0,    21,    30,    20,    13,    12,    27,    28,    29,
      78,     5,   127,    79,    77,   157,   158,    34,     0,    32,
       0,     0,    26,    71,   142,     0,    17,   160,     0,    50,
       0,    48,     0,   131,   129,   128,     0,    33,     0,     0,
       0,     0,    84,    83,    86,    85,    88,    87,    97,    96,
     166,   149,     0,     0,   151,   142,    33,     0,     0,   161,
       1,     7,     4,    71,     0,     0,    33,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    11,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    95,    94,     0,     0,
     147,    10,     0,    40,    15,    14,    71,    73,     0,    72,
       0,     0,     0,    16,     0,     0,    18,     0,   147,   147,
     147,    52,    76,    33,     0,     0,    19,     0,     0,     0,
     165,    98,     0,    34,    23,     0,   159,     0,    57,     0,
      33,    35,     0,     0,    80,     0,   100,    99,   110,   111,
     109,   108,   107,   106,   105,   102,   101,   104,   103,   114,
     113,   112,    92,    93,    89,    90,    91,    82,     0,   115,
     126,   125,   124,   123,   122,   121,   120,   119,   118,   117,
     116,   146,     0,     0,   148,    33,     0,     0,     0,     0,
      51,    49,   130,     0,     0,     0,    75,     0,    46,    33,
     137,   150,   147,   164,   163,   162,     0,     0,    60,     0,
       0,    63,    79,     0,    36,     0,     0,     0,   147,    81,
     145,   141,    53,     0,    69,    74,   163,   147,   135,   133,
     132,     0,    37,     0,    25,    50,     0,    61,     0,    64,
       0,    33,     0,     0,   136,     0,   147,    33,    70,     0,
       0,   140,    62,     0,    65,    67,     0,     0,   138,     0,
      54,   134,    45,    33,     0,    66,    68,     0,     0,    38,
       0,    41,   139,     0,    56,    33,     0,    33,    39,    42,
      47,    55,    33,    33,    33
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    34,    96,   191,    37,    38,    39,    40,    60,    41,
     192,    42,    43,   152,   320,   321,    44,    70,    71,    45,
      46,    47,   260,   261,   290,   314,    48,    49,    50,   158,
     159,    78,    97,    52,    53,   233,   234,    54,    55,    98,
      99,    56
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -156
static const yytype_int16 yypact[] =
{
     268,  -156,    -7,   -32,    65,    65,    -3,  1087,    70,    26,
      18,    -4,  1105,     5,  -156,  -156,  -156,  -156,  -156,  -156,
    -156,    20,  1105,  1105,  1105,  1105,  1105,  1105,    18,    18,
     370,  1105,  -156,   571,    78,   351,  -156,  -156,  -156,    27,
    -156,    61,  -156,  -156,  -156,  -156,  -156,  -156,  -156,  -156,
    -156,  1424,  -156,   587,  -156,  -156,  -156,   887,  1105,  -156,
      32,    35,  -156,    81,  -156,     1,  -156,    15,  1462,    73,
     -33,  -156,  1105,    38,   -57,    39,    19,   688,    67,  1500,
    1105,  1105,   -49,   -49,   -49,   -49,   -49,   -49,    17,    17,
    -156,  1643,   -29,  1268,    97,   111,   772,  1424,   -25,  -156,
    -156,  -156,  1424,    81,    83,    84,   918,    85,  1105,  1105,
    1105,  1105,  1105,  1105,  1105,  1105,  1105,  1105,  1105,  1105,
    1105,  1105,  1105,  1105,  1105,  1105,  1105,  1105,  1105,  1105,
    1105,   165,  1105,  -156,  1105,  1105,  1105,  1105,  1105,  1105,
    1105,  1105,  1105,  1105,  1105,  1105,  -156,  -156,   166,  1105,
    1105,  -156,    87,  1576,  -156,  -156,    81,  -156,    90,   136,
      98,    97,   122,  -156,  1105,    70,  -156,  1307,  1105,  1105,
    1105,  -156,  -156,   803,    99,    -4,  -156,    92,  1346,  1105,
    -156,   104,  1105,   887,  -156,    15,  -156,   101,  1020,  1105,
     655,  -156,   180,  1105,  1643,  1538,  1704,  1733,  1153,  1760,
    1786,   162,   162,   162,   162,   179,   179,   179,   179,    -5,
      -5,    -5,    34,    34,   -49,   -49,   -49,   110,  1192,  1643,
    1643,  1643,  1643,  1643,  1643,  1643,  1643,  1643,  1643,  1643,
    1643,  -156,  1230,   107,   160,   918,   112,    -4,   194,  1105,
    1643,  -156,   119,   117,   118,   121,  -156,   202,  -156,   918,
    -156,  1643,  1105,  1643,  1609,  -156,    -4,   205,  -156,   126,
     203,  1105,  1158,   125,  -156,   131,   127,  1105,  1105,   133,
    -156,  -156,   211,    -4,  -156,  -156,  1643,  1105,  -156,  -156,
    -156,   134,  -156,   148,  -156,     9,   -22,  -156,  1105,  -156,
     152,   918,  1105,   135,  1674,   150,  1105,   918,  -156,   151,
      -4,  -156,  -156,  1385,  1105,  -156,   167,    -2,  -156,   168,
    -156,  -156,   216,   918,   169,  1576,  -156,  1105,   207,  -156,
       2,  -156,  -156,    -4,  -156,   918,   208,  1001,  -156,  -156,
    -156,  -156,  1001,   456,   487
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -156,  -156,     3,     0,   -23,  -156,  -156,  -156,   256,  -156,
     -73,  -156,  -156,   -76,  -156,   -58,  -156,     6,   100,  -156,
    -156,  -156,  -156,  -156,  -156,  -156,  -156,  -156,   255,   -96,
    -156,  -155,    14,  -156,    -1,   -77,   237,   260,  -156,  -156,
      89,  -156
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -161
static const yytype_int16 yytable[] =
{
      36,    62,   165,    35,   177,   160,   179,   187,    74,    76,
     185,   317,   318,   165,    51,   317,   318,   -58,   161,   162,
     248,    68,    64,   148,   149,   169,    79,    88,    89,     1,
      64,   131,   132,    36,   151,   101,    82,    83,    84,    85,
      86,    87,    65,    57,    91,    93,   164,    17,    18,   102,
      58,   166,    14,    15,    16,    17,    18,    19,    20,   180,
     236,   186,   302,   126,   127,   128,   129,   130,   104,    59,
     105,   106,   153,   107,    69,   131,   132,    36,   100,    63,
     173,    77,   274,    63,   319,   157,   167,    80,   328,   174,
     175,   243,   244,   245,   153,   178,   101,   148,   149,   148,
     149,   284,    81,   171,   128,   129,   130,    30,    72,   103,
     164,    67,   259,   263,   131,   132,   154,   266,   298,   155,
     168,   170,   194,   195,   196,   197,   198,   199,   200,   201,
     202,   203,   204,   205,   206,   207,   208,   209,   210,   211,
     212,   213,   214,   215,   216,   312,   218,   182,   219,   220,
     221,   222,   223,   224,   225,   226,   227,   228,   229,   230,
     151,   183,   272,   232,    91,   188,   189,   193,   330,   217,
     231,   238,   239,   101,   235,   283,   282,   237,   240,   249,
     156,   247,    91,    91,    91,   289,   252,   262,   256,   265,
      36,   295,   268,   251,   271,   179,   253,   254,   275,   273,
     299,   277,   153,   153,   278,   279,   281,   153,   280,   285,
     287,   288,   291,   292,   293,   296,   306,   297,   305,   309,
     307,   300,   119,   120,   310,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   301,   304,   308,   311,   323,
     324,   326,   131,   132,   123,   124,   125,   126,   127,   128,
     129,   130,   331,   276,   316,   322,   325,   327,   332,   131,
     132,    61,   329,   286,    73,   241,    91,    92,    -2,    75,
       0,     1,     2,     3,   255,   153,     0,     0,     0,     4,
       0,   294,    91,     5,     6,     7,     8,     9,    10,    11,
       0,    91,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,   303,     0,     0,     0,   153,     0,     0,     0,
      91,     0,     0,     0,     0,     0,     0,     0,   315,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
     333,   153,    36,   101,   101,   334,    22,    23,     0,     0,
       0,    24,    25,    26,    27,    28,    29,     0,     0,    30,
      31,    -3,    32,    33,     1,     2,     3,     0,     0,     0,
       0,     0,     4,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     1,    64,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,     0,    65,     0,     0,     9,
       0,     0,     0,     0,     0,     0,    14,    15,    16,    17,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
      23,     0,     0,     0,    24,    25,    26,    27,    28,    29,
       0,     0,    30,    31,     0,    32,    33,     0,    22,    23,
       0,     0,     0,    24,    25,    26,    27,    28,    29,     0,
       0,    30,    31,     0,     0,    67,     0,     0,    90,     1,
       2,     3,     0,     0,     0,     0,     0,     4,     0,   -44,
     -44,     5,     6,     7,     8,     9,    10,    11,     0,     0,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
       1,     2,     3,     0,     0,     0,     0,     0,     4,     0,
     -43,   -43,     5,     6,     7,     8,     9,    10,    11,     0,
       0,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,     0,     0,     0,    22,    23,     0,     0,     0,    24,
      25,    26,    27,    28,    29,     0,     0,    30,    31,     0,
      32,    33,   -44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,    23,     0,     0,     0,
      24,    25,    26,    27,    28,    29,     0,     0,    30,    31,
       0,    32,    33,   -43,    94,    95,     3,     0,     0,     0,
       0,     0,     4,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     0,     0,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,     0,  -160,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,     0,     0,     0,    22,
      23,     0,     0,     0,    24,    25,    26,    27,    28,    29,
       0,     0,    30,    31,     0,    32,    33,  -160,    94,    95,
       3,     0,     0,     0,   146,   147,     4,   148,   149,   150,
       5,     6,     7,     8,     9,    10,    11,     0,     0,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,     0,
    -160,     1,     2,     3,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,     0,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,     0,    22,    23,     0,     0,     0,    24,    25,
      26,    27,    28,    29,     0,     0,    30,    31,     0,    32,
      33,   264,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,    27,    28,    29,     0,     0,    30,
      31,     0,    32,    33,   172,     1,     2,     3,     0,     0,
       0,     0,     0,     4,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,     0,     0,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,     1,     2,     3,     0,
       0,     0,     0,     0,     4,     0,     0,     0,     5,     6,
       7,     8,     9,    10,    11,     0,     0,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,     0,     0,     0,
      22,    23,     0,     0,     0,    24,    25,    26,    27,    28,
      29,     0,     0,    30,    31,     0,    32,    33,   184,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    22,    23,     0,     0,     0,    24,    25,    26,    27,
      28,    29,     0,     0,    30,    31,     0,    32,    33,   246,
       1,    64,     3,     0,     0,     0,     0,     0,     4,     0,
       0,     0,     5,     6,     7,     8,     9,    10,    11,     0,
       0,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,     1,     2,     3,     0,     0,     0,     0,     0,     4,
       0,     0,     0,     5,     6,     7,     8,     9,    10,    11,
       0,     0,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,     0,     0,     0,    22,    23,     0,     0,     0,
      24,    25,    26,    27,    28,    29,     0,     0,    30,    31,
       0,    32,    33,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,    27,    28,    29,     0,     0,    30,
      31,     0,    32,   190,     1,     2,     3,     0,     0,     0,
       0,     0,     4,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,     1,    64,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,     0,    65,     0,   257,     9,
       0,     0,     0,     0,     0,     0,    14,    15,    16,    17,
      18,    19,    20,    21,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    22,
      23,     0,     0,     0,    24,    25,    26,    27,    28,    29,
       0,     0,    30,    31,     0,    32,    33,     0,    22,    23,
       1,    64,     0,    24,    25,    26,    27,    28,    29,     0,
       0,    30,    31,    65,   258,    67,     9,     0,     1,    64,
       0,     0,     0,    14,    15,    16,    17,    18,    19,    20,
      21,    65,     0,     0,     9,     0,     0,     0,     0,     0,
       0,    14,    15,    16,    17,    18,    19,    20,    21,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    22,    23,     0,     0,     0,
      24,    25,    26,    27,    28,    29,   -59,     0,    30,    31,
       0,    66,    67,    22,    23,     0,     0,     0,    24,    25,
      26,    27,    28,    29,     0,     0,    30,    31,     0,     0,
      67,     0,     0,     0,     0,   134,   135,   136,   137,   138,
     139,   140,   141,   142,   143,   144,   145,   113,   114,   115,
     116,   117,   118,   119,   120,     0,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,     0,   108,     0,     0,
       0,     0,     0,   131,   132,   146,   147,     0,   148,   149,
     150,   109,     0,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,     0,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   108,     0,     0,     0,     0,
       0,     0,   131,   132,     0,     0,     0,     0,     0,   109,
     269,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,     0,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   108,     0,     0,     0,     0,     0,     0,
     131,   132,     0,     0,     0,     0,     0,   109,   270,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
       0,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,     0,   108,     0,     0,     0,     0,     0,   131,   132,
       0,     0,     0,     0,     0,   181,   109,     0,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,     0,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
       0,   108,     0,     0,     0,     0,     0,   131,   132,     0,
       0,     0,     0,     0,   242,   109,     0,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,     0,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
     108,     0,     0,     0,     0,     0,   131,   132,     0,     0,
       0,     0,     0,   250,   109,     0,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,     0,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,     0,   108,
       0,     0,     0,     0,     0,   131,   132,     0,     0,     0,
       0,     0,   313,   109,     0,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,     0,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   108,     0,     0,
       0,     0,     0,     0,   131,   132,     0,     0,   133,     0,
       0,   109,     0,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,     0,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   108,     0,     0,     0,     0,
       0,     0,   131,   132,     0,     0,   163,     0,     0,   109,
       0,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,     0,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   108,     0,     0,     0,     0,     0,     0,
     131,   132,     0,     0,   176,     0,     0,   109,   267,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
       0,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   108,     0,     0,     0,     0,     0,     0,   131,   132,
       0,     0,     0,     0,     0,   109,     0,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,     0,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
       0,     0,     0,     0,     0,     0,   131,   132,   109,     0,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,     0,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,     0,     0,     0,     0,     0,     0,     0,   131,
     132,     0,   109,   133,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,     0,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,     0,     0,     0,     0,
       0,     0,     0,   131,   132,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,     0,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,     0,     0,     0,
       0,     0,     0,     0,   131,   132,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,     0,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,     0,     0,     0,
       0,     0,     0,     0,   131,   132,   112,   113,   114,   115,
     116,   117,   118,   119,   120,     0,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,     0,     0,     0,     0,
       0,     0,     0,   131,   132,   114,   115,   116,   117,   118,
     119,   120,     0,   121,   122,   123,   124,   125,   126,   127,
     128,   129,   130,     0,     0,     0,     0,     0,     0,     0,
     131,   132,   115,   116,   117,   118,   119,   120,     0,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,     0,
       0,     0,     0,     0,     0,     0,   131,   132
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-156))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,     4,    35,     0,    80,     4,    35,   103,     9,    10,
      35,    13,    14,    35,     0,    13,    14,     8,     3,     4,
     175,     7,     4,    80,    81,    82,    12,    28,    29,     3,
       4,    80,    81,    33,    57,    35,    22,    23,    24,    25,
      26,    27,    16,    50,    30,    31,    37,    29,    30,    35,
      82,    84,    26,    27,    28,    29,    30,    31,    32,    88,
     156,    86,    84,    68,    69,    70,    71,    72,     7,     4,
       9,    10,    58,    12,     4,    80,    81,    77,     0,    82,
      77,    85,   237,    82,    86,     4,    72,    82,    86,    22,
      23,   168,   169,   170,    80,    81,    96,    80,    81,    80,
      81,   256,    82,    84,    70,    71,    72,    81,    82,    82,
      37,    85,   188,   189,    80,    81,    84,   193,   273,    84,
      82,    82,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   300,   132,    50,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     183,    50,   235,   149,   150,    82,    82,    82,   323,     4,
       4,    35,    50,   173,    87,   252,   249,    87,   164,    87,
      82,    82,   168,   169,   170,   261,    82,   188,    87,     9,
     190,   268,    82,   179,    87,    35,   182,   183,     4,    87,
     277,    82,   188,   189,    87,    87,     4,   193,    87,     4,
      84,     8,    87,    82,    87,    82,   292,     6,   291,   296,
      85,    87,    60,    61,   297,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    87,    84,    87,    87,    23,
     313,   317,    80,    81,    65,    66,    67,    68,    69,    70,
      71,    72,   325,   239,    87,    87,    87,    50,    50,    80,
      81,     5,   320,   257,     9,   165,   252,    30,     0,     9,
      -1,     3,     4,     5,   185,   261,    -1,    -1,    -1,    11,
      -1,   267,   268,    15,    16,    17,    18,    19,    20,    21,
      -1,   277,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,   288,    -1,    -1,    -1,   292,    -1,    -1,    -1,
     296,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   304,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   327,    -1,    -1,
     327,   317,   332,   333,   334,   332,    68,    69,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    -1,    -1,    81,
      82,     0,    84,    85,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    11,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    20,    21,     3,     4,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    16,    -1,    -1,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      -1,    -1,    81,    82,    -1,    84,    85,    -1,    68,    69,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    -1,
      -1,    81,    82,    -1,    -1,    85,    -1,    -1,    88,     3,
       4,     5,    -1,    -1,    -1,    -1,    -1,    11,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    -1,    -1,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    11,    -1,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    -1,
      -1,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    -1,    -1,    -1,    68,    69,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    -1,    -1,    81,    82,    -1,
      84,    85,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    -1,    -1,    81,    82,
      -1,    84,    85,    86,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    11,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    20,    21,    -1,    -1,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    -1,    -1,    68,
      69,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      -1,    -1,    81,    82,    -1,    84,    85,    86,     3,     4,
       5,    -1,    -1,    -1,    77,    78,    11,    80,    81,    82,
      15,    16,    17,    18,    19,    20,    21,    -1,    -1,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      35,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,    11,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    68,    69,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    -1,    -1,    81,    82,    -1,    84,
      85,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    -1,    -1,    81,
      82,    -1,    84,    85,    86,     3,     4,     5,    -1,    -1,
      -1,    -1,    -1,    11,    -1,    -1,    -1,    15,    16,    17,
      18,    19,    20,    21,    -1,    -1,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,     3,     4,     5,    -1,
      -1,    -1,    -1,    -1,    11,    -1,    -1,    -1,    15,    16,
      17,    18,    19,    20,    21,    -1,    -1,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    -1,    -1,    -1,
      68,    69,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    -1,    -1,    81,    82,    -1,    84,    85,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    68,    69,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    -1,    -1,    81,    82,    -1,    84,    85,    86,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    11,    -1,
      -1,    -1,    15,    16,    17,    18,    19,    20,    21,    -1,
      -1,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,    11,
      -1,    -1,    -1,    15,    16,    17,    18,    19,    20,    21,
      -1,    -1,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    -1,    -1,    -1,    68,    69,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    -1,    -1,    81,    82,
      -1,    84,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    -1,    -1,    81,
      82,    -1,    84,    85,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    11,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    20,    21,     3,     4,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    16,    -1,    18,    19,
      -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,
      69,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      -1,    -1,    81,    82,    -1,    84,    85,    -1,    68,    69,
       3,     4,    -1,    73,    74,    75,    76,    77,    78,    -1,
      -1,    81,    82,    16,    84,    85,    19,    -1,     3,     4,
      -1,    -1,    -1,    26,    27,    28,    29,    30,    31,    32,
      33,    16,    -1,    -1,    19,    -1,    -1,    -1,    -1,    -1,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    68,    69,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,     8,    -1,    81,    82,
      -1,    84,    85,    68,    69,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    -1,    -1,    81,    82,    -1,    -1,
      85,    -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    54,    55,    56,
      57,    58,    59,    60,    61,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    35,    -1,    -1,
      -1,    -1,    -1,    80,    81,    77,    78,    -1,    80,    81,
      82,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    -1,    -1,    -1,    -1,    -1,    49,
      88,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    -1,    -1,    -1,    -1,    -1,    49,    88,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    35,    -1,    -1,    -1,    -1,    -1,    80,    81,
      -1,    -1,    -1,    -1,    -1,    87,    49,    -1,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    -1,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      -1,    35,    -1,    -1,    -1,    -1,    -1,    80,    81,    -1,
      -1,    -1,    -1,    -1,    87,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    80,    81,    -1,    -1,
      -1,    -1,    -1,    87,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    -1,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    80,    81,    -1,    -1,    -1,
      -1,    -1,    87,    49,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    35,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    -1,    -1,    84,    -1,
      -1,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    -1,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    35,    -1,    -1,    -1,    -1,
      -1,    -1,    80,    81,    -1,    -1,    84,    -1,    -1,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    35,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    -1,    -1,    84,    -1,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      -1,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    35,    -1,    -1,    -1,    -1,    -1,    -1,    80,    81,
      -1,    -1,    -1,    -1,    -1,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    -1,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    80,
      81,    -1,    49,    84,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    80,    81,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    -1,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    80,    81,    55,    56,    57,    58,    59,
      60,    61,    -1,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      80,    81,    56,    57,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    80,    81
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,    11,    15,    16,    17,    18,    19,
      20,    21,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    68,    69,    73,    74,    75,    76,    77,    78,
      81,    82,    84,    85,    90,    91,    92,    93,    94,    95,
      96,    98,   100,   101,   105,   108,   109,   110,   115,   116,
     117,   121,   122,   123,   126,   127,   130,    50,    82,     4,
      97,    97,     4,    82,     4,    16,    84,    85,   121,     4,
     106,   107,    82,   117,   123,   126,   123,    85,   120,   121,
      82,    82,   121,   121,   121,   121,   121,   121,   123,   123,
      88,   121,   125,   121,     3,     4,    91,   121,   128,   129,
       0,    92,   121,    82,     7,     9,    10,    12,    35,    49,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    80,    81,    84,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    77,    78,    80,    81,
      82,    93,   102,   121,    84,    84,    82,     4,   118,   119,
       4,     3,     4,    84,    37,    35,    84,   121,    82,    82,
      82,    84,    86,    91,    22,    23,    84,   102,   121,    35,
      88,    87,    50,    50,    86,    35,    86,   118,    82,    82,
      85,    92,    99,    82,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,     4,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,     4,   121,   124,   125,    87,   118,    87,    35,    50,
     121,   107,    87,   124,   124,   124,    86,    82,   120,    87,
      87,   121,    82,   121,   121,   129,    87,    18,    84,   102,
     111,   112,   123,   102,    86,     9,   102,    50,    82,    88,
      88,    87,    99,    87,   120,     4,   121,    82,    87,    87,
      87,     4,    99,   124,   120,     4,   106,    84,     8,   102,
     113,    87,    82,    87,   121,   124,    82,     6,   120,   124,
      87,    87,    84,   121,    84,    99,   102,    85,    87,   124,
      99,    87,   120,    87,   114,   121,    87,    13,    14,    86,
     103,   104,    87,    23,    99,    87,   102,    50,    86,   104,
     120,    99,    50,    91,    91
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (&yylloc, ps, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, &yylloc, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, &yylloc, ps)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location, ps); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, PSTATE *ps)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, ps)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    PSTATE *ps;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
  YYUSE (ps);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, PSTATE *ps)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp, ps)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
    PSTATE *ps;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, ps);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, PSTATE *ps)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule, ps)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
    PSTATE *ps;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       , ps);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule, ps); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, PSTATE *ps)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp, ps)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
    PSTATE *ps;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (ps);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (PSTATE *ps);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (PSTATE *ps)
#else
int
yyparse (ps)
    PSTATE *ps;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval = 0;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 191 "parser.y"
    { ps->opcodes = code_nop(ps); }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 192 "parser.y"
    {
		ps->opcodes = (yyvsp[(1) - (1)]);
	}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 195 "parser.y"
    {
	  ps->opcodes = codes_join3(ps,(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]), code_ret(ps,1));
	}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 198 "parser.y"
    {	/* for json */
	  ps->opcodes = codes_join(ps,(yyvsp[(1) - (1)]), code_ret(ps,1));
	}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 203 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 204 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 209 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 210 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 211 "parser.y"
    { (yyval) = (yyvsp[(3) - (3)]); }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 215 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(1) - (2)]), code_pop(ps,1)); codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 216 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 217 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]);  codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 218 "parser.y"
    { (yyval) = code_reserved(ps,RES_BREAK, (yyvsp[(2) - (3)]));  codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 219 "parser.y"
    { (yyval) = code_reserved(ps,RES_CONTINUE, (yyvsp[(2) - (3)]));  codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 220 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (3)]), code_ret(ps,1));  codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 221 "parser.y"
    { (yyval) = code_ret(ps,0); codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 222 "parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 223 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (3)]), code_throw(ps));  codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 224 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 225 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 226 "parser.y"
    { (yyval) = code_nop(ps); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 227 "parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 228 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 231 "parser.y"
    {
	  OpCodes *ret = codes_join4(ps,code_push_index(ps,(yyvsp[(1) - (5)])),
				     code_push_func(ps,func_make_static(ps,(yyvsp[(3) - (5)]), scope_get_varlist(ps->lexer), (yyvsp[(5) - (5)]))),
				     code_assign(ps,1), code_pop(ps,1));
	  if (ps->eval_flag) ret = codes_join(ps,code_local(ps,(yyvsp[(1) - (5)])), ret);
	  scope_pop(ps->lexer);
		(yyval) = ret;
	}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 242 "parser.y"
    {
		if (!ps->eval_flag) {
		  scope_add_var(ps->lexer,(yyvsp[(2) - (2)]));
		}
		(yyval) = (yyvsp[(2) - (2)]);
	}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 251 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 252 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 253 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 254 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 257 "parser.y"
    { (yyval) = NULL; }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 258 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 261 "parser.y"
    { (yyval) = NULL; }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 262 "parser.y"
    {
		(yyval) = (yyvsp[(1) - (2)]);
	}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 268 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 269 "parser.y"
    { (yyval) = code_nop(ps); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 273 "parser.y"
    { 
	  (yyval) = codes_join4(ps,(yyvsp[(3) - (5)]), code_with(ps,((OpCodes *)(yyvsp[(5) - (5)]))->code_len + 1), (yyvsp[(5) - (5)]), code_ewith(ps));
	}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 279 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(4) - (7)]), code_pop(ps,1)); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 280 "parser.y"
    { OpCodes *ophead;
		CaseList *cl = (yyvsp[(7) - (8)]);
		OpCodes *allstats = codes_new(ps,3);
		CaseList *cldefault = NULL;
		CaseList *head = NULL;
		CaseList *t;

		while (cl) {
			cl->off = allstats->code_len;
			allstats = codes_join(ps,allstats, cl->es->stat);

			t = cl;
			cl = cl->next;
			
			if (t->es->isdefault) {
			  if (cldefault) yyerror(&(yylsp[(8) - (8)]), ps, "More then one switch default\n");
				cldefault = t;
			} else {
				t->next = head;

				head = t;
			}
		}
		code_reserved_replace(ps,allstats, 0, 1, (yyvsp[(1) - (8)]), 1);
		
		ophead = code_jmp(ps,allstats->code_len + 1);
		if (cldefault) {
		  ophead = codes_join(ps,code_jmp(ps,ophead->code_len + cldefault->off + 1), ophead);
		  psfree(cldefault);
		}
		while (head) {
		  CaseList *t;
		  OpCodes *e = codes_join4(ps,code_push_top(ps), head->es->expr, 
					   code_equal(ps), code_jtrue(ps,ophead->code_len + head->off + 1));
		  ophead = codes_join(ps,e, ophead);
			t = head;
			head = head->next;
			psfree(t); 
		}
		(yyval) = codes_join4(ps,codes_join(ps,(yyvsp[(4) - (8)]), code_unref(ps)), ophead, allstats, code_pop(ps,1));
	}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 323 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); codes_lineno((yyval), yyfilename, yylineno); }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 327 "parser.y"
    { (yyval) = caselist_new(ps,(yyvsp[(1) - (1)])); }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 328 "parser.y"
    { (yyval) = caselist_insert(ps,(yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 332 "parser.y"
    { (yyval) = exprstat_new(ps,(yyvsp[(2) - (4)]), (yyvsp[(4) - (4)]), 0); }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 333 "parser.y"
    { (yyval) = exprstat_new(ps,NULL, (yyvsp[(3) - (3)]), 1); }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 337 "parser.y"
    {
	  OpCodes *catchblock = codes_join3(ps,code_scatch(ps,(yyvsp[(5) - (7)])), (yyvsp[(7) - (7)]), code_ecatch(ps));
		OpCodes *finallyblock = codes_join(ps,code_sfinal(ps), code_efinal(ps));
		OpCodes *tryblock = codes_join(ps,(yyvsp[(2) - (7)]), code_etry(ps));
		(yyval) = codes_join4(ps,code_stry(ps,tryblock->code_len, catchblock->code_len, finallyblock->code_len),
							tryblock, catchblock, finallyblock);
	}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 344 "parser.y"
    {
	  OpCodes *catchblock = codes_join(ps,code_scatch(ps,NULL), code_ecatch(ps));
		OpCodes *finallyblock = codes_join3(ps,code_sfinal(ps), (yyvsp[(4) - (4)]), code_efinal(ps));
		OpCodes *tryblock = codes_join(ps,(yyvsp[(2) - (4)]), code_etry(ps));
		(yyval) = codes_join4(ps,code_stry(ps,tryblock->code_len, catchblock->code_len, finallyblock->code_len),
							tryblock, catchblock, finallyblock);
	}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 352 "parser.y"
    {
	  OpCodes *catchblock = codes_join3(ps,code_scatch(ps,(yyvsp[(5) - (9)])), (yyvsp[(7) - (9)]), code_ecatch(ps));
		OpCodes *finallyblock = codes_join3(ps,code_sfinal(ps), (yyvsp[(9) - (9)]), code_efinal(ps));
		OpCodes *tryblock = codes_join(ps,(yyvsp[(2) - (9)]), code_etry(ps));
		(yyval) = codes_join4(ps,code_stry(ps,tryblock->code_len, catchblock->code_len, finallyblock->code_len),
							tryblock, catchblock, finallyblock);
	}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 361 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 362 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 366 "parser.y"
    {
	  OpCodes *ret = codes_join4(ps,code_push_index(ps,(yyvsp[(1) - (1)])),
							code_push_undef(ps),
				     code_assign(ps,1),
				     code_pop(ps,1));
		if (!ps->eval_flag)	scope_add_var(ps->lexer,(yyvsp[(1) - (1)]));
		else ret = codes_join(ps,code_local(ps,(yyvsp[(1) - (1)])), ret);
		(yyval) = ret;
	}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 375 "parser.y"
    {
	  OpCodes *ret = codes_join4(ps,code_push_index(ps,(yyvsp[(1) - (3)])),
							(yyvsp[(3) - (3)]),
				     code_assign(ps,1),
				     code_pop(ps,1));
		if (!ps->eval_flag) scope_add_var(ps->lexer,(yyvsp[(1) - (3)]));
		else ret = codes_join(ps,code_local(ps,(yyvsp[(1) - (3)])), ret);
		(yyval) = ret;
	}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 387 "parser.y"
    {
		if (((OpCodes *)(yyvsp[(2) - (3)]))->lvalue_flag == 2) {
		  (yyval) = codes_join(ps,(yyvsp[(2) - (3)]), code_delete(ps,2));
		} else (yyval) = code_delete(ps,1);
	}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 395 "parser.y"
    {
		int offset = ((OpCodes *)(yyvsp[(5) - (5)]))->code_len;
		(yyval) = codes_join3(ps,(yyvsp[(3) - (5)]), code_jfalse(ps,offset + 1), (yyvsp[(5) - (5)]));
	}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 399 "parser.y"
    {
		int len_block2 = ((OpCodes *)(yyvsp[(7) - (7)]))->code_len;
		OpCodes *block1 = codes_join(ps,(yyvsp[(5) - (7)]), code_jmp(ps,len_block2 + 1));
		OpCodes *condi = codes_join(ps,(yyvsp[(3) - (7)]), code_jfalse(ps,block1->code_len + 1));
		(yyval) = codes_join3(ps,condi, block1, (yyvsp[(7) - (7)]));
	}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 408 "parser.y"
    {
		OpCodes *init = (yyvsp[(4) - (9)]);
		OpCodes *cond = (yyvsp[(5) - (9)]);
		OpCodes *step = ((yyvsp[(7) - (9)]) ? codes_join(ps,(yyvsp[(7) - (9)]), code_pop(ps,1)) : code_nop(ps));
		OpCodes *stat = (yyvsp[(9) - (9)]);
		OpCodes *cont_jmp = code_jfalse(ps,step->code_len + stat->code_len + 2);
		OpCodes *step_jmp = code_jmp(ps,-(cond->code_len + step->code_len + stat->code_len + 1));
		code_reserved_replace(ps,stat, step->code_len + 1, 0, (yyvsp[(1) - (9)]), 0);
		(yyval) = codes_join(ps,codes_join3(ps,init, cond, cont_jmp),
						   codes_join3(ps,stat, step, step_jmp));
	}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 419 "parser.y"
    { OpCodes *ret;
		ForinVar *fv = (yyvsp[(4) - (8)]);
		OpCodes *lval;
		if (fv->varname) lval = code_push_index(ps,fv->varname);
		else lval = fv->lval;
		
		ret = make_forin(ps,lval, (yyvsp[(6) - (8)]), (yyvsp[(8) - (8)]), (yyvsp[(1) - (8)]));
		if (fv->varname && fv->local) {
			if (!ps->eval_flag) {
			  scope_add_var(ps->lexer,fv->varname);
			  codes_free(ps,fv->local);
			} else ret = codes_join(ps,fv->local, ret);
		}
		(yyval) = ret;
	}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 437 "parser.y"
    {
	  (yyval) = forinvar_new(ps,(yyvsp[(2) - (2)]), code_local(ps,(yyvsp[(2) - (2)])), NULL);
	}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 440 "parser.y"
    {
		if (((OpCodes *)(yyvsp[(1) - (1)]))->lvalue_flag == 2) 
		  (yyval) = forinvar_new(ps,NULL, NULL, codes_join(ps,(yyvsp[(1) - (1)]), code_subscript(ps,0)));
		else (yyval) = forinvar_new(ps,NULL, NULL, (yyvsp[(1) - (1)]));
	}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 448 "parser.y"
    { (yyval) = code_nop(ps); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 449 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(1) - (2)]), code_pop(ps,1)); }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 450 "parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 453 "parser.y"
    { (yyval) = code_push_bool(ps,1); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 454 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 457 "parser.y"
    { (yyval) = NULL; }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 458 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 462 "parser.y"
    {
		OpCodes *cond = (yyvsp[(4) - (6)]);
		OpCodes *stat = (yyvsp[(6) - (6)]);
		code_reserved_replace(ps,stat, 1, 0, (yyvsp[(1) - (6)]), 0);
		(yyval) = codes_join4(ps,cond, code_jfalse(ps,stat->code_len + 2), stat,
				 code_jmp(ps,-(stat->code_len + cond->code_len + 1)));
	}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 472 "parser.y"
    {
		OpCodes *stat = (yyvsp[(3) - (7)]);
		OpCodes *cond = (yyvsp[(6) - (7)]);
		code_reserved_replace(ps,stat, cond->code_len + 1, 0, (yyvsp[(1) - (7)]), 0);
		(yyval) = codes_join3(ps,stat, cond,
				 code_jtrue(ps,-(stat->code_len + cond->code_len)));
	}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 482 "parser.y"
    {
	  (yyval) = code_push_func(ps,func_make_static(ps,(yyvsp[(3) - (5)]), scope_get_varlist(ps->lexer), (yyvsp[(5) - (5)])));
		scope_pop(ps->lexer);
	}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 486 "parser.y"
    {
	  (yyval) = code_push_func(ps,func_make_static(ps,(yyvsp[(4) - (6)]), scope_get_varlist(ps->lexer), (yyvsp[(6) - (6)])));
		scope_pop(ps->lexer);
	}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 492 "parser.y"
    { scope_push(ps->lexer); (yyval) = strs_new(ps); }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 493 "parser.y"
    { ArgList *a; strs *s;
	  scope_push(ps->lexer);
		a = (yyvsp[(1) - (1)]);
		s = strs_new(ps);
		while (a) {
		  strs_push(ps,s, a->argname);
			a = a->next;
		}
		(yyval) = s;
	}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 506 "parser.y"
    { (yyval) = arglist_new(ps,(yyvsp[(1) - (1)])); }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 507 "parser.y"
    { (yyval) = arglist_insert(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 510 "parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 511 "parser.y"
    { (yyval) = code_nop(ps); }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 515 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 516 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 517 "parser.y"
    { 
	  if (((OpCodes *)(yyvsp[(1) - (1)]))->lvalue_flag == 2) (yyval) = codes_join(ps,(yyvsp[(1) - (1)]), code_subscript(ps,1)); 
		else (yyval) = (yyvsp[(1) - (1)]);
	}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 521 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), code_pop(ps,1), (yyvsp[(3) - (3)])); }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 522 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (4)]), (yyvsp[(3) - (4)]), code_subscript(ps,1)); }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 523 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), code_push_string(ps,(yyvsp[(3) - (3)])), code_subscript(ps,1)); }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 524 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_neg(ps)); }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 525 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_pos(ps)); }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 526 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_bnot(ps)); }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 527 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_not(ps)); }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 528 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_typeof(ps)); }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 529 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(2) - (2)]), code_pop(ps,1), code_push_undef(ps)); }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 530 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_mul(ps)); }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 531 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_div(ps)); }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 532 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_mod(ps)); }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 533 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_add(ps)); }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 534 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_sub(ps)); }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 535 "parser.y"
    {
	  if (((OpCodes *)(yyvsp[(1) - (2)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(1) - (2)]), code_subscript(ps,0), code_inc(ps,1));
	  else (yyval) = codes_join(ps,(yyvsp[(1) - (2)]), code_inc(ps,1));
 	}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 539 "parser.y"
    { 
	  if (((OpCodes *)(yyvsp[(1) - (2)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(1) - (2)]), code_subscript(ps,0), code_dec(ps,1));
	  else (yyval) = codes_join(ps,(yyvsp[(1) - (2)]), code_dec(ps,1)); 
	}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 543 "parser.y"
    {
	  if (((OpCodes *)(yyvsp[(2) - (2)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(2) - (2)]), code_subscript(ps,0), code_inc(ps,0));
	  else (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_inc(ps,0));
	}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 547 "parser.y"
    { 
	  if (((OpCodes *)(yyvsp[(2) - (2)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(2) - (2)]), code_subscript(ps,0), code_dec(ps,0));
	  else (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_dec(ps,0));
	}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 551 "parser.y"
    { (yyval) = (yyvsp[(2) - (3)]); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 552 "parser.y"
    {
	  OpCodes *expr2 = codes_join(ps,code_pop(ps,1), (yyvsp[(3) - (3)]));
	  (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), code_jfalse_np(ps,expr2->code_len + 1), expr2);
	}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 556 "parser.y"
    {
	  OpCodes *expr2 = codes_join(ps,code_pop(ps,1), (yyvsp[(3) - (3)]));
	  (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), code_jtrue_np(ps,expr2->code_len + 1), expr2);
	}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 560 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_less(ps)); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 561 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_greater(ps)); }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 562 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_lessequ(ps)); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 563 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_greaterequ(ps)); }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 564 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_equal(ps)); }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 565 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_notequal(ps)); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 566 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_eequ(ps));	}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 567 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_nneq(ps)); }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 568 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_band(ps)); }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 569 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_bor(ps)); }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 570 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_bxor(ps)); }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 571 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_shf(ps,0)); }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 572 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_shf(ps,1)); }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 573 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_shf(ps,2)); }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 574 "parser.y"
    { (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_assign(ps,((OpCodes *)(yyvsp[(1) - (3)]))->lvalue_flag)); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 575 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_add(ps)); }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 576 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_sub(ps)); }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 577 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_mul(ps)); }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 578 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_mod(ps)); }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 579 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_shf(ps,0)); }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 580 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_shf(ps,1)); }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 581 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_shf(ps,2)); }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 582 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_band(ps)); }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 583 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_bor(ps)); }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 584 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_bxor(ps)); }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 585 "parser.y"
    { (yyval) = opassign(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]), code_div(ps)); }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 586 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 588 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_newfcall(ps,0)); }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 589 "parser.y"
    { 
		if (((OpCodes *)(yyvsp[(2) - (2)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(2) - (2)]), code_subscript(ps,1), code_newfcall(ps,0));
 		else (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_newfcall(ps,0));}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 592 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(3) - (4)]), code_newfcall(ps,0)); }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 593 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (2)]), code_newfcall(ps,0)); }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 594 "parser.y"
    { 
		OpCodes *opl = (yyvsp[(4) - (5)]);
		int expr_cnt = opl ? opl->expr_counter:0;
 		(yyval) = codes_join3(ps,(yyvsp[(2) - (5)]), (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 599 "parser.y"
    {
		OpCodes *opl = (yyvsp[(4) - (5)]);
		int expr_cnt = opl ? opl->expr_counter:0;
		OpCodes *lv = NULL;
		if (((OpCodes *)(yyvsp[(2) - (5)]))->lvalue_flag == 2) lv = codes_join(ps,(yyvsp[(2) - (5)]), code_subscript(ps,1));
		else lv = (yyvsp[(2) - (5)]);
		(yyval) = codes_join3(ps,lv, (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 607 "parser.y"
    { 
		OpCodes *opl = (yyvsp[(6) - (7)]);
		int expr_cnt = opl ? opl->expr_counter:0;
		(yyval) = codes_join3(ps,(yyvsp[(3) - (7)]), (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 612 "parser.y"
    {
		OpCodes *opl = (yyvsp[(4) - (5)]);
		int expr_cnt = opl ? opl->expr_counter:0;
 		(yyval) = codes_join3(ps,(yyvsp[(2) - (5)]), (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 617 "parser.y"
    {
	  OpCodes *expr2 = codes_join(ps,(yyvsp[(3) - (5)]), code_jmp(ps,((OpCodes *)(yyvsp[(5) - (5)]))->code_len + 1));
	  (yyval) = codes_join4(ps,(yyvsp[(1) - (5)]), code_jfalse(ps,expr2->code_len + 1), expr2, (yyvsp[(5) - (5)]));
	}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 621 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(3) - (4)]), code_debug(ps)); }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 625 "parser.y"
    {
	  OpCodes *ff = codes_join4(ps,(yyvsp[(1) - (6)]), code_push_string(ps,(yyvsp[(3) - (6)])), code_chthis(ps,1), code_subscript(ps,1));
		OpCodes *opl = (yyvsp[(5) - (6)]);
		int expr_cnt = opl ? opl->expr_counter:0;
 		(yyval) = codes_join3(ps,ff, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
	}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 631 "parser.y"
    {
	  OpCodes *ff = codes_join4(ps,(yyvsp[(1) - (7)]), (yyvsp[(3) - (7)]), code_chthis(ps,1), code_subscript(ps,1));
		OpCodes *opl = (yyvsp[(6) - (7)]);
		int expr_cnt = opl ? opl->expr_counter:0;
 		(yyval) = codes_join3(ps,ff, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
	}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 637 "parser.y"
    {
		OpCodes *opl = (yyvsp[(5) - (6)]);
		int expr_cnt = opl ? opl->expr_counter:0;
 		(yyval) = codes_join4(ps,(yyvsp[(2) - (6)]), code_chthis(ps,0), (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
	}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 642 "parser.y"
    {
		OpCodes *opl = (yyvsp[(3) - (4)]);
		int expr_cnt = opl ? opl->expr_counter:0;
		OpCodes *pref;
		OpCodes *lval = (yyvsp[(1) - (4)]);
		if (lval->lvalue_flag == 2) {
		  pref = codes_join3(ps,(yyvsp[(1) - (4)]), code_chthis(ps,1), code_subscript(ps,1));
		  (yyval) = codes_join3(ps,pref, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
		} else {
		  if (lval->lvalue_name && unistrcmp(lval->lvalue_name, tounichars(ps,"eval")) == 0) {
			  (yyval) = codes_join(ps,(opl ? opl : code_nop(ps)), code_eval(ps,expr_cnt));
			} else {
			  pref = codes_join(ps,(yyvsp[(1) - (4)]), code_chthis(ps,0));
				(yyval) = codes_join3(ps,pref, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
			}
		}
	}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 662 "parser.y"
    {
	  (yyval) = code_push_index(ps,(yyvsp[(1) - (1)])); 
		((OpCodes *)(yyval))->lvalue_flag = 1; 
		((OpCodes *)(yyval))->lvalue_name = (yyvsp[(1) - (1)]); 
	}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 667 "parser.y"
    { (yyval) = code_push_args(ps); ((OpCodes *)(yyval))->lvalue_flag = 1; }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 668 "parser.y"
    { (yyval) = code_push_this(ps); ((OpCodes *)(yyval))->lvalue_flag = 1; }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 669 "parser.y"
    {
		if (((OpCodes *)(yyvsp[(1) - (4)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(1) - (4)]), code_subscript(ps,1), (yyvsp[(3) - (4)])); 
		else (yyval) = codes_join(ps,(yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); 
		((OpCodes *)(yyval))->lvalue_flag = 2;
	}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 674 "parser.y"
    {
		if (((OpCodes *)(yyvsp[(1) - (3)]))->lvalue_flag == 2) (yyval) = codes_join3(ps,(yyvsp[(1) - (3)]), code_subscript(ps,1), code_push_string(ps,(yyvsp[(3) - (3)]))); 
		else (yyval) = codes_join(ps,(yyvsp[(1) - (3)]), code_push_string(ps,(yyvsp[(3) - (3)])));
		((OpCodes *)(yyval))->lvalue_flag = 2;
	}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 681 "parser.y"
    { (yyval) = NULL; }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 682 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 686 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ((OpCodes *)(yyval))->expr_counter = 1; }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 687 "parser.y"
    { 
		int exprcnt = ((OpCodes *)(yyvsp[(1) - (3)]))->expr_counter + 1;
		(yyval) = codes_join(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
		((OpCodes *)(yyval))->expr_counter = exprcnt;
	}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 694 "parser.y"
    { (yyval) = code_push_string(ps,(yyvsp[(1) - (1)])); }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 695 "parser.y"
    { (yyval) = code_push_undef(ps); }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 696 "parser.y"
    { (yyval) = code_push_bool(ps,1); }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 697 "parser.y"
    { (yyval) = code_push_bool(ps,0); }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 698 "parser.y"
    { (yyval) = code_push_num(ps,(yyvsp[(1) - (1)])); }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 699 "parser.y"
    { (yyval) = code_push_regex(ps,(yyvsp[(1) - (1)])); }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 700 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 701 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 705 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (3)]), code_object(ps,((OpCodes *)(yyvsp[(2) - (3)]))->expr_counter)); }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 709 "parser.y"
    { (yyval) = code_nop(ps); ((OpCodes *)(yyval))->expr_counter = 0; }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 710 "parser.y"
    { (yyval) = (yyvsp[(1) - (1)]); ((OpCodes *)(yyval))->expr_counter = 1; }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 711 "parser.y"
    {
		int cnt = ((OpCodes *)(yyvsp[(1) - (3)]))->expr_counter + 1;
		(yyval) = codes_join(ps,(yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
		((OpCodes *)(yyval))->expr_counter = cnt;
	}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 719 "parser.y"
    { (yyval) = codes_join(ps,code_push_string(ps,(yyvsp[(1) - (3)])), (yyvsp[(3) - (3)])); }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 720 "parser.y"
    { (yyval) = codes_join(ps,code_push_string(ps,(yyvsp[(1) - (3)])), (yyvsp[(3) - (3)])); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 724 "parser.y"
    { (yyval) = codes_join(ps,(yyvsp[(2) - (3)]), code_array(ps,((OpCodes *)(yyvsp[(2) - (3)]))->expr_counter)); }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 725 "parser.y"
    { (yyval) = code_array(ps,0); }
    break;



/* Line 1806 of yacc.c  */
#line 3619 "parser.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (&yylloc, ps, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (&yylloc, ps, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc, ps);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[1] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp, ps);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, ps, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, ps);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp, ps);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 729 "parser.y"



