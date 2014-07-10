%{
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
	unichar* argname;
	struct ArgList* tail;
	struct ArgList* next;
} ArgList;

static ArgList* arglist_new(PSTATE* ps,const unichar* name)
{
	ArgList* a = psmalloc(sizeof(ArgList));
	memset(a, 0, sizeof(ArgList));
	a->argname = unistrdup(ps,name);
	a->tail = a;
	return a;
}

static ArgList* arglist_insert(PSTATE* ps, ArgList* a, const unichar* name)
{
	ArgList* b = psmalloc(sizeof(ArgList));
	memset(b, 0, sizeof(ArgList));
	b->argname = unistrdup(ps,name);
	a->tail->next = b;
	a->tail = b;
	return a;
}

typedef struct ForinVar {
	unichar* varname;
	OpCodes* local;
	OpCodes* lval;
} ForinVar;

ForinVar* forinvar_new(PSTATE* ps, unichar* varname, OpCodes* local, OpCodes* lval)
{
	ForinVar* r = psmalloc(sizeof(ForinVar));
	r->varname = varname;
	r->local = local;
	r->lval = lval;
	return r;
}

static OpCodes* make_forin(PSTATE* ps, OpCodes* lval, OpCodes* expr, OpCodes* stat, const unichar* label)
{
	OpCodes* init = codes_join(ps, expr, code_key(ps));
	OpCodes* cond = codes_join3(ps, lval, code_next(ps),
				    code_jfalse(ps, stat->code_len + 2));
	OpCodes* stat_jmp = code_jmp(ps, -(cond->code_len + stat->code_len));
	code_reserved_replace(ps,stat, 1, 0, label, 2);
	return
		codes_join3(
			ps,
			codes_join(ps, init, cond),
			codes_join(ps, stat, stat_jmp),
			code_pop(ps, 2)
		);
}

typedef struct CaseExprStat {
	OpCodes* expr;
	OpCodes* stat;
	int isdefault;
} CaseExprStat;

CaseExprStat* exprstat_new(PSTATE* ps, OpCodes* expr, OpCodes* stat, int isdef)
{
	CaseExprStat* r = psmalloc(sizeof(CaseExprStat));
	r->expr = expr;
	r->stat = stat;
	r->isdefault = isdef;
	return r;
}

typedef struct CaseList {
	CaseExprStat* es;
	int off;
	struct CaseList* tail;
	struct CaseList* next;
} CaseList;

static CaseList* caselist_new(PSTATE* ps, CaseExprStat* es)
{
	CaseList* a = psmalloc(sizeof(CaseList));
	memset(a, 0, sizeof(CaseList));
	a->es = es;
	a->tail = a;
	return a;
}

static CaseList* caselist_insert(PSTATE* ps, CaseList* a, CaseExprStat* es)
{
	CaseList* b = psmalloc(sizeof(CaseList));
	memset(b, 0, sizeof(CaseList));
	b->es = es;
	a->tail->next = b;
	a->tail = b;
	return a;
}

static OpCodes* opassign(PSTATE* ps, OpCodes* lval, OpCodes* oprand, OpCodes* op)
{
	OpCodes* ret;
	if (((OpCodes*)lval)->lvalue_flag == 1) {
		ret =
			codes_join3(
				ps,
				lval, 
				codes_join3(ps, code_push_top(ps), oprand, op),
				code_assign(ps,1)
			);
	} else {
		ret =
			codes_join3(
				ps,
				lval,
				codes_join4(ps, code_push_top2(ps), code_subscript(ps,1), oprand, op),
				code_assign(ps, 2)
			);
	}
	return ret;
}

#define yyfilename ps->lexer->codename
#define yylineno ps->lexer->cur_line

%}

%locations			/* location proccess */
%pure-parser		/* re-entence */
%parse-param	{PSTATE* ps}
%lex-param		{PSTATE* ps}
%error-verbose
%expect 6			/* if-else shift/reduce
					   lvalue shift/reduce 
					   ',' shift/reduce
					   empty statement '{''}' empty object shift/reduct */

%token STRING
%token IDENTIFIER
%token IF
%token ELSE
%token FOR
%token IN
%token WHILE
%token DO
%token CONTINUE
%token SWITCH
%token CASE
%token DEFAULT
%token BREAK
%token FUNC
%token RETURN
%token LOCAL
%token NEW
%token DELETE
%token TRY
%token CATCH
%token FINALLY
%token THROW
%token WITH
%token UNDEF
%token _TRUE
%token _FALSE
%token _THIS
%token ARGUMENTS
%token FNUMBER
%token REGEXP
%token __DEBUG

%left MIN_PRI
%left ','
%left ARGCOMMA						/* comma in argument list */
%right '=' ADDAS MNSAS MULAS MODAS LSHFAS RSHFAS URSHFAS BANDAS BORAS BXORAS DIVAS
/*           +=    -=    *=    %=   <<=     >>=   >>>=     &=     |=    ^=    /= */
%left '?' ':'
%left OR							/* || */
%left AND							/* && */
%left '|'							/* | */
%left '^'							/* ^ */
%left '&'							/* & */
%left EQU NEQ EEQU NNEQ				/* == != === !== */
%left '>' '<' LEQ GEQ INSTANCEOF	/* <= >= instanceof */
%left LSHF RSHF URSHF				/* << >> >>> */
%left '+' '-'
%left '*' '/' '%'
%left NEG '!' INC DEC '~' TYPEOF VOID	/* - ++ -- typeof */
%left NEW								/* new */
%left '.' '[' '('
%left MAX_PRI

%%

file:	{ ps->opcodes = code_nop(ps); }
	| statements {
		ps->opcodes = $1;
	}
	| statements expr {
	  ps->opcodes = codes_join3(ps, $1, $2, code_ret(ps,1));
	}
	| expr {	/* for json */
	  ps->opcodes = codes_join(ps, $1, code_ret(ps,1));
	}
;

statements:	statement		{ $$ = $1; }
| statements statement	{ $$ = codes_join(ps, $1, $2); }
;

/* todo, ';' auto gen */
statement: 
	iterstatement		{ $$ = $1; }
	| comonstatement	{ $$ = $1; }
	| IDENTIFIER ':' comonstatement	{ $$ = $3; }
;

comonstatement:
expr ';' { $$ = codes_join(ps, $1, code_pop(ps,1)); codes_lineno($$, yyfilename, yylineno); }
	| if_statement 	{ $$ = $1; }
	| delete_statement 	{ $$ = $1;  codes_lineno($$, yyfilename, yylineno); }
| BREAK identifier_opt ';'		{ $$ = code_reserved(ps,RES_BREAK, $2);  codes_lineno($$, yyfilename, yylineno); }
| CONTINUE identifier_opt ';'	{ $$ = code_reserved(ps,RES_CONTINUE, $2);  codes_lineno($$, yyfilename, yylineno); }
| RETURN expr ';'   { $$ = codes_join(ps, $2, code_ret(ps,1));  codes_lineno($$, yyfilename, yylineno); }
| RETURN ';'		{ $$ = code_ret(ps,0); codes_lineno($$, yyfilename, yylineno); }
	| LOCAL vardecs ';' { $$ = $2; codes_lineno($$, yyfilename, yylineno); }
| THROW expr ';'	{ $$ = codes_join(ps, $2, code_throw(ps));  codes_lineno($$, yyfilename, yylineno); }
	| try_statement		{ $$ = $1; }
	| with_statement	{ $$ = $1; }
| ';'					{ $$ = code_nop(ps); }
	| '{' statements '}'	{ $$ = $2; }
	| func_statement 		{ $$ = $1; }
;
func_statement:
	func_prefix '(' args_opt ')' func_statement_block {
	  OpCodes *ret = codes_join4(ps, code_push_index(ps,$1),
				     code_push_func(ps,func_make_static(ps,$3, scope_get_varlist(ps->lexer), $5)),
				     code_assign(ps,1), code_pop(ps,1));
	  if (ps->eval_flag) ret = codes_join(ps, code_local(ps,$1), ret);
	  scope_pop(ps->lexer);
		$$ = ret;
	}
;

func_prefix:
	FUNC IDENTIFIER %prec MAX_PRI {
		if (!ps->eval_flag) {
		  scope_add_var(ps->lexer,$2);
		}
		$$ = $2;
	}
;

iterstatement:
	for_statement	{ $$ = $1; }
	| while_statement	{ $$ = $1; }
	| do_statement		{ $$ = $1; }
	| switch_statement	{ $$ = $1; }
;

identifier_opt:	{ $$ = NULL; }
	| IDENTIFIER { $$ = $1; }
;

label_opt: { $$ = NULL; }
	| IDENTIFIER ':' {
		$$ = $1;
	}
;

statement_or_empty:
	statement	{ $$ = $1; }
	| '{' '}'	{ $$ = code_nop(ps); }
;

with_statement:
	WITH '(' condexpr ')' statement_or_empty { 
	  $$ = codes_join4(ps, $3, code_with(ps,((OpCodes *)$5)->code_len + 1), $5, code_ewith(ps));
	}
;

switch_statement: 
label_opt SWITCH '(' condexpr ')' '{' '}' { $$ = codes_join(ps, $4, code_pop(ps,1)); }
| label_opt SWITCH '(' condexpr ')' '{' cases '}'	{ OpCodes *ophead;
		CaseList* cl = $7;
		OpCodes* allstats = codes_new(ps,3);
		CaseList* cldefault = NULL;
		CaseList* head = NULL;
		CaseList* t;

		while (cl) {
			cl->off = allstats->code_len;
			allstats = codes_join(ps, allstats, cl->es->stat);

0			t = cl;
			cl = cl->next;
			
			if (t->es->isdefault) {
				if (cldefault) {
					yyerror(&@8, ps, "More then one switch default\n");
				}
				cldefault = t;
			} else {
				t->next = head;

				head = t;
			}
		}
		code_reserved_replace(ps,allstats, 0, 1, $1, 1);
		
		ophead = code_jmp(ps,allstats->code_len + 1);
		if (cldefault) {
			ophead = codes_join(ps, code_jmp(ps,ophead->code_len + cldefault->off + 1), ophead);
			psfree(cldefault);
		}
		while (head) {
			CaseList* t;
			OpCodes* e =
				codes_join4(
					ps,
					code_push_top(ps),
					head->es->expr, 
					code_equal(ps),
					code_jtrue(ps,ophead->code_len + head->off + 1)
				);
			ophead = codes_join(ps, e, ophead);
			t = head;
			head = head->next;
			psfree(t); 
		}
		$$ = codes_join4(ps, codes_join(ps, $4, code_unref(ps)), ophead, allstats, code_pop(ps,1));
	}
;

condexpr: expr { $$ = $1; codes_lineno($$, yyfilename, yylineno); }
;

cases:
 case			{ $$ = caselist_new(ps,$1); }
| cases case	{ $$ = caselist_insert(ps,$1, $2); }
;

case:
	CASE condexpr ':' statements	{ $$ = exprstat_new(ps,$2, $4, 0); }
	| DEFAULT ':' statements	{ $$ = exprstat_new(ps,NULL, $3, 1); }
;

try_statement:
	TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block {
	  OpCodes *catchblock = codes_join3(ps, code_scatch(ps,$5), $7, code_ecatch(ps));
		OpCodes *finallyblock = codes_join(ps, code_sfinal(ps), code_efinal(ps));
		OpCodes *tryblock = codes_join(ps, $2, code_etry(ps));
		$$ = codes_join4(ps, code_stry(ps, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
							tryblock, catchblock, finallyblock);
	}
	| TRY func_statement_block FINALLY func_statement_block {
	  OpCodes *catchblock = codes_join(ps, code_scatch(ps,NULL), code_ecatch(ps));
		OpCodes *finallyblock = codes_join3(ps, code_sfinal(ps), $4, code_efinal(ps));
		OpCodes *tryblock = codes_join(ps, $2, code_etry(ps));
		$$ = codes_join4(ps, code_stry(ps,tryblock->code_len, catchblock->code_len, finallyblock->code_len),
							tryblock, catchblock, finallyblock);
	}
	| TRY func_statement_block CATCH '(' IDENTIFIER ')' func_statement_block 
		FINALLY func_statement_block {
	  OpCodes *catchblock = codes_join3(ps, code_scatch(ps,$5), $7, code_ecatch(ps));
		OpCodes *finallyblock = codes_join3(ps, code_sfinal(ps), $9, code_efinal(ps));
		OpCodes *tryblock = codes_join(ps, $2, code_etry(ps));
		$$ = codes_join4(ps,code_stry(ps, tryblock->code_len, catchblock->code_len, finallyblock->code_len),
							tryblock, catchblock, finallyblock);
	}
;
vardecs:
	vardec					{ $$ = $1; }
	| vardecs ',' vardec 	{ $$ = codes_join(ps, $1, $3); }
;

vardec:
	IDENTIFIER				{
	  OpCodes *ret = codes_join4(ps, code_push_index(ps,$1),
							code_push_undef(ps),
				     code_assign(ps,1),
				     code_pop(ps,1));
		if (!ps->eval_flag)	scope_add_var(ps->lexer,$1);
		else ret = codes_join(ps, code_local(ps,$1), ret);
		$$ = ret;
	}
	| IDENTIFIER '=' expr	{
	  OpCodes *ret = codes_join4(ps, code_push_index(ps,$1),
							$3,
				     code_assign(ps,1),
				     code_pop(ps,1));
		if (!ps->eval_flag) scope_add_var(ps->lexer,$1);
		else ret = codes_join(ps, code_local(ps,$1), ret);
		$$ = ret;
	}
;

delete_statement:
	DELETE lvalue ';'			{
		if (((OpCodes *)$2)->lvalue_flag == 2) {
		  $$ = codes_join(ps, $2, code_delete(ps,2));
		} else $$ = code_delete(ps,1);
	}
;

if_statement:
	IF '(' condexpr ')' statement_or_empty {
		int offset = ((OpCodes *)$5)->code_len;
		$$ = codes_join3(ps, $3, code_jfalse(ps,offset + 1), $5);
	}
	| IF '(' condexpr ')' statement_or_empty ELSE statement_or_empty {
		int len_block2 = ((OpCodes *)$7)->code_len;
		OpCodes *block1 = codes_join(ps, $5, code_jmp(ps,len_block2 + 1));
		OpCodes *condi = codes_join(ps, $3, code_jfalse(ps,block1->code_len + 1));
		$$ = codes_join3(ps, condi, block1, $7);
	}
;

for_statement:
	label_opt FOR '(' for_init for_cond ';' expr_opt ')' statement_or_empty {
		OpCodes *init = $4;
		OpCodes *cond = $5;
		OpCodes *step = ($7 ? codes_join(ps, $7, code_pop(ps,1)) : code_nop(ps));
		OpCodes *stat = $9;
		OpCodes *cont_jmp = code_jfalse(ps,step->code_len + stat->code_len + 2);
		OpCodes *step_jmp = code_jmp(ps,-(cond->code_len + step->code_len + stat->code_len + 1));
		code_reserved_replace(ps,stat, step->code_len + 1, 0, $1, 0);
		$$ = codes_join(ps, codes_join3(ps, init, cond, cont_jmp),
						   codes_join3(ps, stat, step, step_jmp));
	}
| label_opt FOR '(' for_var IN expr ')' statement_or_empty { OpCodes *ret;
		ForinVar *fv = $4;
		OpCodes *lval;
		if (fv->varname) lval = code_push_index(ps,fv->varname);
		else lval = fv->lval;
		
		ret = make_forin(ps,lval, $6, $8, $1);
		if (fv->varname && fv->local) {
			if (!ps->eval_flag) {
			  scope_add_var(ps->lexer,fv->varname);
			  codes_free(ps,fv->local);
			} else ret = codes_join(ps, fv->local, ret);
		}
		$$ = ret;
	}
;

for_var:
	| LOCAL IDENTIFIER {
	  $$ = forinvar_new(ps,$2, code_local(ps,$2), NULL);
	}
	| lvalue {
		if (((OpCodes *)$1)->lvalue_flag == 2) 
		  $$ = forinvar_new(ps,NULL, NULL, codes_join(ps, $1, code_subscript(ps,0)));
		else $$ = forinvar_new(ps,NULL, NULL, $1);
	}
;

for_init:
	';'					{ $$ = code_nop(ps); }
| condexpr ';'			{ $$ = codes_join(ps, $1, code_pop(ps,1)); }
	| LOCAL vardecs ';' { $$ = $2; }
;

for_cond:				{ $$ = code_push_bool(ps,1); }
	| condexpr				{ $$ = $1; }
;

expr_opt:				{ $$ = NULL; }
	| expr				{ $$ = $1; }
;

while_statement:
	label_opt WHILE '(' condexpr ')' statement_or_empty {
		OpCodes *cond = $4;
		OpCodes *stat = $6;
		code_reserved_replace(ps,stat, 1, 0, $1, 0);
		$$ = codes_join4(ps, cond, code_jfalse(ps,stat->code_len + 2), stat,
				 code_jmp(ps,-(stat->code_len + cond->code_len + 1)));
	}
;

do_statement:
	label_opt DO statement_or_empty WHILE '(' condexpr ')' {
		OpCodes *stat = $3;
		OpCodes *cond = $6;
		code_reserved_replace(ps,stat, cond->code_len + 1, 0, $1, 0);
		$$ = codes_join3(ps, stat, cond,
				 code_jtrue(ps,-(stat->code_len + cond->code_len)));
	}
;

func_expr:
	FUNC '(' args_opt ')' func_statement_block {
	  $$ = code_push_func(ps,func_make_static(ps,$3, scope_get_varlist(ps->lexer), $5));
		scope_pop(ps->lexer);
	}
	| FUNC IDENTIFIER '(' args_opt ')' func_statement_block {
	  $$ = code_push_func(ps,func_make_static(ps,$4, scope_get_varlist(ps->lexer), $6));
		scope_pop(ps->lexer);
	}
;

args_opt: { scope_push(ps->lexer); $$ = strs_new(ps); }
| args { ArgList *a; strs *s;
	  scope_push(ps->lexer);
		a = $1;
		s = strs_new(ps);
		while (a) {
		  strs_push(ps,s, a->argname);
			a = a->next;
		}
		$$ = s;
	}
;

args:
IDENTIFIER	{ $$ = arglist_new(ps,$1); }
| args ',' IDENTIFIER { $$ = arglist_insert(ps,$1, $3); }
;

func_statement_block: '{' statements '}'	{ $$ = $2; }
	| '{' '}'								{ $$ = code_nop(ps); }
;

expr:
	value					{ $$ = $1; }
	| func_expr				{ $$ = $1; }
	| lvalue				{ 
	  if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join(ps, $1, code_subscript(ps,1)); 
		else $$ = $1;
	}
	| expr ',' expr			{ $$ = codes_join3(ps, $1, code_pop(ps,1), $3); }
	| expr '[' expr ']'		{ $$ = codes_join3(ps, $1, $3, code_subscript(ps,1)); }
	| expr '.' IDENTIFIER	{ $$ = codes_join3(ps, $1, code_push_string(ps,$3), code_subscript(ps,1)); }
	| '-' expr %prec NEG 	{ $$ = codes_join(ps, $2, code_neg(ps)); }
	| '+' expr %prec NEG 	{ $$ = codes_join(ps, $2, code_pos(ps)); }
	| '~' expr				{ $$ = codes_join(ps, $2, code_bnot(ps)); }
	| '!' expr				{ $$ = codes_join(ps, $2, code_not(ps)); }
	| TYPEOF expr			{ $$ = codes_join(ps, $2, code_typeof(ps)); }
	| VOID expr				{ $$ = codes_join3(ps, $2, code_pop(ps,1), code_push_undef(ps)); }
	| expr '*' expr 		{ $$ = codes_join3(ps, $1, $3, code_mul(ps)); }
	| expr '/' expr 		{ $$ = codes_join3(ps, $1, $3, code_div(ps)); }
	| expr '%' expr 		{ $$ = codes_join3(ps, $1, $3, code_mod(ps)); }
	| expr '+' expr 		{ $$ = codes_join3(ps, $1, $3, code_add(ps)); }
	| expr '-' expr 		{ $$ = codes_join3(ps, $1, $3, code_sub(ps)); }
	| lvalue INC			{
	  if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3(ps, $1, code_subscript(ps,0), code_inc(ps,1));
	  else $$ = codes_join(ps, $1, code_inc(ps,1));
 	}
	| lvalue DEC			{ 
	  if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3(ps, $1, code_subscript(ps,0), code_dec(ps,1));
	  else $$ = codes_join(ps, $1, code_dec(ps,1)); 
	}
	| INC lvalue			{
	  if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3(ps, $2, code_subscript(ps,0), code_inc(ps,0));
	  else $$ = codes_join(ps, $2, code_inc(ps,0));
	}
	| DEC lvalue 			{ 
	  if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3(ps, $2, code_subscript(ps,0), code_dec(ps,0));
	  else $$ = codes_join(ps, $2, code_dec(ps,0));
	}
	| '(' expr ')'			{ $$ = $2; }
	| expr AND expr			{
	  OpCodes *expr2 = codes_join(ps, code_pop(ps,1), $3);
	  $$ = codes_join3(ps, $1, code_jfalse_np(ps,expr2->code_len + 1), expr2);
	}
	| expr OR expr			{
	  OpCodes *expr2 = codes_join(ps, code_pop(ps,1), $3);
	  $$ = codes_join3(ps, $1, code_jtrue_np(ps,expr2->code_len + 1), expr2);
	}
	| expr '<' expr			{ $$ = codes_join3(ps, $1, $3, code_less(ps)); }
	| expr '>' expr			{ $$ = codes_join3(ps, $1, $3, code_greater(ps)); }
	| expr LEQ expr			{ $$ = codes_join3(ps, $1, $3, code_lessequ(ps)); }
	| expr GEQ expr			{ $$ = codes_join3(ps, $1, $3, code_greaterequ(ps)); }
	| expr EQU expr			{ $$ = codes_join3(ps, $1, $3, code_equal(ps)); }
	| expr NEQ expr			{ $$ = codes_join3(ps, $1, $3, code_notequal(ps)); }
	| expr EEQU expr		{ $$ = codes_join3(ps, $1, $3, code_eequ(ps));	}
	| expr NNEQ expr		{ $$ = codes_join3(ps, $1, $3, code_nneq(ps)); }
	| expr '&' expr			{ $$ = codes_join3(ps, $1, $3, code_band(ps)); }
	| expr '|' expr			{ $$ = codes_join3(ps, $1, $3, code_bor(ps)); }
	| expr '^' expr			{ $$ = codes_join3(ps, $1, $3, code_bxor(ps)); }
	| expr LSHF expr		{ $$ = codes_join3(ps, $1, $3, code_shf(ps,0)); }
	| expr RSHF expr		{ $$ = codes_join3(ps, $1, $3, code_shf(ps,1)); }
	| expr URSHF expr		{ $$ = codes_join3(ps, $1, $3, code_shf(ps,2)); }
	| lvalue '=' expr 		{ $$ = codes_join3(ps, $1, $3, code_assign(ps,((OpCodes *)$1)->lvalue_flag)); }
	| lvalue ADDAS expr		{ $$ = opassign(ps,$1, $3, code_add(ps)); }
	| lvalue MNSAS expr		{ $$ = opassign(ps,$1, $3, code_sub(ps)); }
	| lvalue MULAS expr		{ $$ = opassign(ps,$1, $3, code_mul(ps)); }
	| lvalue MODAS expr		{ $$ = opassign(ps,$1, $3, code_mod(ps)); }
	| lvalue LSHFAS expr	{ $$ = opassign(ps,$1, $3, code_shf(ps,0)); }
	| lvalue RSHFAS expr	{ $$ = opassign(ps,$1, $3, code_shf(ps,1)); }
	| lvalue URSHFAS expr	{ $$ = opassign(ps,$1, $3, code_shf(ps,2)); }
	| lvalue BANDAS expr	{ $$ = opassign(ps,$1, $3, code_band(ps)); }
	| lvalue BORAS expr		{ $$ = opassign(ps,$1, $3, code_bor(ps)); }
	| lvalue BXORAS expr	{ $$ = opassign(ps,$1, $3, code_bxor(ps)); }
	| lvalue DIVAS expr		{ $$ = opassign(ps,$1, $3, code_div(ps)); }
	| fcall_exprs			{ $$ = $1; }
	
	| NEW value				{ $$ = codes_join(ps, $2, code_newfcall(ps,0)); }
	| NEW lvalue			{ 
		if (((OpCodes *)$2)->lvalue_flag == 2) $$ = codes_join3(ps, $2, code_subscript(ps,1), code_newfcall(ps,0));
 		else $$ = codes_join(ps, $2, code_newfcall(ps,0));}
	| NEW '(' expr ')'		{ $$ = codes_join(ps, $3, code_newfcall(ps,0)); }
	| NEW func_expr			{ $$ = codes_join(ps, $2, code_newfcall(ps,0)); }
	| NEW value '(' exprlist_opt ')'		{ 
		OpCodes *opl = $4;
		int expr_cnt = opl ? opl->expr_counter:0;
 		$$ = codes_join3(ps, $2, (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
	| NEW lvalue '(' exprlist_opt ')'		{
		OpCodes *opl = $4;
		int expr_cnt = opl ? opl->expr_counter:0;
		OpCodes *lv = NULL;
		if (((OpCodes *)$2)->lvalue_flag == 2) lv = codes_join(ps, $2, code_subscript(ps,1));
		else lv = $2;
		$$ = codes_join3(ps, lv, (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
	| NEW '(' expr ')' '(' exprlist_opt ')'	{ 
		OpCodes *opl = $6;
		int expr_cnt = opl ? opl->expr_counter:0;
		$$ = codes_join3(ps, $3, (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
	| NEW func_expr '(' exprlist_opt ')'	{
		OpCodes *opl = $4;
		int expr_cnt = opl ? opl->expr_counter:0;
 		$$ = codes_join3(ps, $2, (opl ? opl : code_nop(ps)), code_newfcall(ps,expr_cnt));
	}
	| expr '?' expr ':' expr {
	  OpCodes *expr2 = codes_join(ps, $3, code_jmp(ps,((OpCodes *)$5)->code_len + 1));
	  $$ = codes_join4(ps, $1, code_jfalse(ps,expr2->code_len + 1), expr2, $5);
	}
	| __DEBUG '(' expr ')' { $$ = codes_join(ps, $3, code_debug(ps)); }
;

fcall_exprs:
	expr '.' IDENTIFIER '(' exprlist_opt ')' {
	  OpCodes *ff = codes_join4(ps, $1, code_push_string(ps,$3), code_chthis(ps,1), code_subscript(ps,1));
		OpCodes *opl = $5;
		int expr_cnt = opl ? opl->expr_counter:0;
 		$$ = codes_join3(ps, ff, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
	}
	| expr '[' expr ']' '(' exprlist_opt ')' {
	  OpCodes *ff = codes_join4(ps, $1, $3, code_chthis(ps,1), code_subscript(ps,1));
		OpCodes *opl = $6;
		int expr_cnt = opl ? opl->expr_counter:0;
 		$$ = codes_join3(ps, ff, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
	}
	| '(' expr ')' '(' exprlist_opt ')' {
		OpCodes *opl = $5;
		int expr_cnt = opl ? opl->expr_counter:0;
 		$$ = codes_join4(ps, $2, code_chthis(ps,0), (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
	}
	| lvalue '(' exprlist_opt ')' {
		OpCodes *opl = $3;
		int expr_cnt = opl ? opl->expr_counter:0;
		OpCodes *pref;
		OpCodes *lval = $1;
		if (lval->lvalue_flag == 2) {
		  pref = codes_join3(ps, $1, code_chthis(ps,1), code_subscript(ps,1));
		  $$ = codes_join3(ps, pref, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
		} else {
		  if (lval->lvalue_name && unistrcmp(lval->lvalue_name, tounichars(ps,"eval")) == 0) {
			  $$ = codes_join(ps, (opl ? opl : code_nop(ps)), code_eval(ps,expr_cnt));
			} else {
			  pref = codes_join(ps, $1, code_chthis(ps,0));
				$$ = codes_join3(ps, pref, (opl ? opl : code_nop(ps)), code_fcall(ps,expr_cnt));
			}
		}
	}
;

lvalue:
	IDENTIFIER 				{
	  $$ = code_push_index(ps,$1); 
		((OpCodes *)$$)->lvalue_flag = 1; 
		((OpCodes *)$$)->lvalue_name = $1; 
	}
	| ARGUMENTS 			{ $$ = code_push_args(ps); ((OpCodes *)$$)->lvalue_flag = 1; }
	| _THIS					{ $$ = code_push_this(ps); ((OpCodes *)$$)->lvalue_flag = 1; }
	| lvalue '[' expr ']'	{
		if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3(ps, $1, code_subscript(ps,1), $3); 
		else $$ = codes_join(ps, $1, $3); 
		((OpCodes *)$$)->lvalue_flag = 2;
	}
	| lvalue '.' IDENTIFIER	{
		if (((OpCodes *)$1)->lvalue_flag == 2) $$ = codes_join3(ps, $1, code_subscript(ps,1), code_push_string(ps,$3)); 
		else $$ = codes_join(ps, $1, code_push_string(ps,$3));
		((OpCodes *)$$)->lvalue_flag = 2;
	}
;

exprlist_opt: 	{ $$ = NULL; }
	| exprlist 	{ $$ = $1; }
;

exprlist:
	expr %prec ARGCOMMA { $$ = $1; ((OpCodes *)$$)->expr_counter = 1; }
	| exprlist ',' expr %prec ARGCOMMA { 
		int exprcnt = ((OpCodes *)$1)->expr_counter + 1;
		$$ = codes_join(ps, $1, $3);
		((OpCodes *)$$)->expr_counter = exprcnt;
	}
;

value: STRING { $$ = code_push_string(ps,$1); }
	| UNDEF { $$ = code_push_undef(ps); }
	| _TRUE { $$ = code_push_bool(ps,1); }
	| _FALSE { $$ = code_push_bool(ps,0); }
	| FNUMBER { $$ = code_push_num(ps,$1); }
	| REGEXP { $$ = code_push_regex(ps,$1); }
	| object { $$ = $1; }
	| array { $$ = $1; }
;

object:
'{' items '}' 	{ $$ = codes_join(ps, $2, code_object(ps,((OpCodes *)$2)->expr_counter)); }

;

items:		{ $$ = code_nop(ps); ((OpCodes *)$$)->expr_counter = 0; }
	| item 	{ $$ = $1; ((OpCodes *)$$)->expr_counter = 1; }
	| items ',' item {
		int cnt = ((OpCodes *)$1)->expr_counter + 1;
		$$ = codes_join(ps, $1, $3);
		((OpCodes *)$$)->expr_counter = cnt;
	}
;

item:
IDENTIFIER ':' expr	{ $$ = codes_join(ps, code_push_string(ps,$1), $3); }
| STRING ':' expr	{ $$ = codes_join(ps, code_push_string(ps,$1), $3); }
;

array:
'[' exprlist ']' { $$ = codes_join(ps, $2, code_array(ps,((OpCodes *)$2)->expr_counter)); }
| '[' ']' { $$ = code_array(ps,0); }
;


%%

