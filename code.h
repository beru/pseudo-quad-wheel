#ifndef __CODE_H__
#define __CODE_H__

#include "regexp.h"

#include "unichar.h"

/* stack change */
/* 0  nothing change */
/* +1 push */
/* -1 pop */
typedef enum {		/* SC 	type of data	comment 							*/
	OP_NOP,			/* 0 */
	OP_PUSHNUM,		/* +1 	*double			number 								*/
	OP_PUSHSTR,		/* +1 	*unichar		string 								*/
	OP_PUSHVAR,		/* +1 	*FastVar		variable name	 					*/
	OP_PUSHUND,		/* +1 	-				undefined 							*/
	OP_PUSHBOO,		/* +1 	int				bool 								*/
	OP_PUSHFUN,		/* +1 	*Func			function 							*/
	OP_PUSHREG,		/* +1 	*regex_t		regex 								*/
	OP_PUSHARG,		/* +1	-				push arguments(cur scope)			*/
	OP_PUSHTHS,		/* +1 	-				push this 							*/
	OP_PUSHTOP,		/* +1 	-				duplicate top 						*/
	OP_PUSHTOP2,	/* +2	-				duplicate toq and top				*/
	OP_UNREF,		/* 0	-				make top be right value				*/
	OP_POP,			/* -n 	int				pop n elements 						*/
	OP_LOCAL,		/* 0  	*unichar		add a var to current scope 			*/
	OP_NEG,			/* 0 	-				make top = - top 					*/
	OP_POS,			/* 0	-				make top = + top, (conv to number)	*/
	OP_NOT,			/* 0	-				reserve top 						*/
	OP_BNOT,		/* 0	-				bitwise not							*/
	OP_ADD,			/* -1	-				all math opr pop 2 elem from stack,	*/
	OP_SUB,			/* -1	-				 calc and push back in to the stack */
	OP_MUL,			/* -1	-													*/
	OP_DIV,			/* -1	-													*/
	OP_MOD,			/* -1	-													*/
	OP_LESS,		/* -1	-				logical opr, same as math opr 		*/
	OP_GREATER,		/* -1	-													*/
	OP_LESSEQU,		/* -1	-													*/
	OP_GREATEREQU,	/* -1	-													*/
	OP_EQUAL,		/* -1	-													*/
	OP_NOTEQUAL,	/* -1	-													*/
	OP_STRICTEQU,	/* -1	-													*/
	OP_STRICTNEQ,	/* -1	-													*/
	OP_BAND,		/* -1	-				bitwise and							*/
	OP_BOR,			/* -1	-				bitwise or							*/
	OP_BXOR,		/* -1	-				bitwise xor							*/
	OP_SHF,			/* -1	int(right)		signed shift left or shift right	*/
	
	OP_ASSIGN,		/* -n	int				if n = 1, assign to lval,			*/
					/*						n = 2, assign to object member 		*/
	OP_SUBSCRIPT,	/* -1	-				do subscript TOQ[TOP]				*/
	OP_INC,			/* 0	int				data indicate prefix/postfix inc/dec 				*/
	OP_DEC,			/* 0	int 																*/
	OP_KEY,			/* +1	-				push an iter object that contain all key in top 	*/
	OP_NEXT,		/* -1	-				assign next key to top, make top be res of this opr */
	OP_JTRUE,		/* -1	int				jmp to offset if top is true, 						*/
	OP_JFALSE,		/* -1	int				jmp to offset if top is false,						*/
	OP_JTRUE_NP,	/* 0	int				jtrue no pop version 								*/
	OP_JFALSE_NP,	/* 0	int				jfalse no pop version 								*/
	OP_JMP,			/* 0	int				jmp to offset 										*/
	OP_JMPPOP,		/* -n	*JmpPopInfo		jmp to offset with pop n 							*/
	OP_FCALL,		/* -n+1	int				call func with n args, pop then, make ret to be top */
	OP_NEWFCALL,	/* -n+1	int				same as fcall, call as a constructor 				*/
	OP_RET,			/* -n	int				n = 0|1, return with arg 							*/
	OP_DELETE,		/* -n 	int				n = 1, delete var, n = 2, delete object member 		*/
	OP_CHTHIS,		/* 0,	-				make toq as new 'this'								*/
	OP_OBJECT,		/* -n*2+1	int			create object from stack, and push back in 			*/
	OP_ARRAY,		/* -n+1	int				create array object from stack, and push back in 	*/
	OP_EVAL,		/* -n+1	int				eval can not be assign to other var 				*/
	OP_STRY,		/* 0	*TryInfo		push try statment poses info to trylist 			*/
	OP_ETRY,		/* 0	-				end of try block, jmp to finally 					*/
	OP_SCATCH,		/* 0	*unichar		create new scope, assign to current excption 		*/
	OP_ECATCH,		/* 0	-				jmp to finally 										*/
	OP_SFINAL,		/* 0	-				restore scope chain create by Scatch 				*/
	OP_EFINAL,		/* 0	-				end of finally, any unfinish code in catch, do it 	*/
	OP_THROW,		/* 0	-				make top be last exception, pop trylist till catched*/
	OP_WITH,		/* -1	-				make top be top of scopechain, add to trylist 		*/
	OP_EWITH,		/* 0	-				pop trylist 										*/
	OP_TYPEOF, /*  0 */
	OP_RESERVED,	/* 0	ReservedInfo*	reserved, be replaced by iterstat by jmp/jmppop 	*/
	OP_DEBUG,		/* 0	-				DEBUG OPCODE, output top 							*/
	OP_LASTOP		/* 0	-				END OF OPCODE 										*/
} Eopcode;

#define RES_CONTINUE	1
#define RES_BREAK		2

extern const char* op_names[OP_LASTOP];

typedef struct OpCode {
	Eopcode op;
	void* data;
	char* codename;
	int lineno;
} OpCode;

typedef struct OpCodes {
	OpCode* codes;
	int code_len;
	int code_size;
	
	int expr_counter;			/* context related expr count */
	int lvalue_flag;			/* left value count/flag */
	const unichar* lvalue_name;	/* left value name */
} OpCodes;

typedef struct Value Value;

typedef struct FastVar {
	int context_id;
	struct {
		unichar* varname;
		Value* lval;
	} var;
} FastVar;

typedef struct TryInfo {
	int trylen;
	int catchlen;
	int finallen;
} TryInfo;

typedef struct ReservedInfo {
	int type;
	const unichar* label;
	int topop;
} ReservedInfo;

typedef struct JmpPopInfo {
	int off;
	int topop;
} JmpPopInfo;

typedef struct PSTATE PSTATE;
typedef struct Func Func;

OpCodes* code_push_undef(PSTATE*);
OpCodes* code_push_bool(PSTATE*, int v);
OpCodes* code_push_num(PSTATE*, double* v);
OpCodes* code_push_string(PSTATE*, const unichar* str);
OpCodes* code_push_index(PSTATE*, unichar* varname);
OpCodes* code_push_args(PSTATE*);
OpCodes* code_push_this(PSTATE*);
OpCodes* code_push_func(PSTATE*, Func* fun);
OpCodes* code_push_regex(PSTATE*, regex_t* reg);
OpCodes* code_push_top(PSTATE*);
OpCodes* code_push_top2(PSTATE*);
OpCodes* code_unref(PSTATE*);
OpCodes* code_local(PSTATE*, const unichar* varname);

OpCodes* code_nop(PSTATE*);
OpCodes* code_neg(PSTATE*);
OpCodes* code_pos(PSTATE*);
OpCodes* code_bnot(PSTATE*);
OpCodes* code_not(PSTATE*);
OpCodes* code_mul(PSTATE*);
OpCodes* code_div(PSTATE*);
OpCodes* code_mod(PSTATE*);
OpCodes* code_add(PSTATE*);
OpCodes* code_sub(PSTATE*);
OpCodes* code_less(PSTATE*);
OpCodes* code_greater(PSTATE*);
OpCodes* code_lessequ(PSTATE*);
OpCodes* code_greaterequ(PSTATE*);
OpCodes* code_equal(PSTATE*); 
OpCodes* code_notequal(PSTATE*);
OpCodes* code_eequ(PSTATE*);
OpCodes* code_nneq(PSTATE*);
OpCodes* code_band(PSTATE*);
OpCodes* code_bor(PSTATE*);
OpCodes* code_bxor(PSTATE*);
OpCodes* code_shf(PSTATE*, int right);

OpCodes* code_assign(PSTATE*, int h);
OpCodes* code_subscript(PSTATE*, int right_val);
OpCodes* code_inc(PSTATE*, int e);
OpCodes* code_dec(PSTATE*, int e);

OpCodes* code_fcall(PSTATE*, int argc);
OpCodes* code_newfcall(PSTATE*, int argc);
OpCodes* code_pop(PSTATE*, int n);
OpCodes* code_ret(PSTATE*, int n);
OpCodes* code_object(PSTATE*, int c);
OpCodes* code_array(PSTATE*, int c);
OpCodes* code_key(PSTATE*);
OpCodes* code_next(PSTATE*);
OpCodes* code_delete(PSTATE*, int n);
OpCodes* code_chthis(PSTATE*, int n);

OpCodes* code_jfalse(PSTATE*, int off);
OpCodes* code_jtrue(PSTATE*, int off);
OpCodes* code_jfalse_np(PSTATE*, int off);
OpCodes* code_jtrue_np(PSTATE*, int off);
OpCodes* code_jmp(PSTATE*, int off);
OpCodes* code_eval(PSTATE*, int argc);

OpCodes* code_throw(PSTATE*);
OpCodes* code_stry(PSTATE*, int trylen, int catchlen, int finlen);
OpCodes* code_etry(PSTATE*);
OpCodes* code_scatch(PSTATE*, const unichar* var);
OpCodes* code_ecatch(PSTATE*);
OpCodes* code_sfinal(PSTATE*);
OpCodes* code_efinal(PSTATE*);
OpCodes* code_throw(PSTATE*);
OpCodes* code_with(PSTATE*, int withlen);
OpCodes* code_ewith(PSTATE*);
OpCodes* code_typeof(PSTATE*);

OpCodes* code_debug(PSTATE*);
OpCodes* code_reserved(PSTATE*, int type, unichar* id);

OpCodes* codes_join(PSTATE*, OpCodes* a, OpCodes* b);
OpCodes* codes_join3(PSTATE*, OpCodes* a, OpCodes* b, OpCodes* c);
OpCodes* codes_join4(PSTATE*, OpCodes* a, OpCodes* b, OpCodes* c, OpCodes* d);

/* replace continue/break(coded as OP_RESERVED) jmp
 * |------------------| \
 * |                  | \\ where 'continue' jmp (jmp to step code)
 * |       ops        |  / 
 * |                  | / \
 * |------------------|    \ where 'break' jmp (jmp after step code)
 * |                  |    /
 * |       step       |   /
 * |                  |  /
 * |------------------| /
 * 1. break_only used only in swith
 * 2. desire_label, only replace if current iter statement has the same label with opcode
 * 3. topop, if not replace in current iter statment, make sure when jmp out of this loop/switch
 *	  corrent stack elems poped(for in always has 2 elem, while switch has 1)
 */
void code_reserved_replace(PSTATE*, OpCodes* ops, int step_len, int break_only, const unichar* desire_label, int topop);

void code_decode(PSTATE*, OpCode* op, int currentip);
void codes_print(PSTATE*, OpCodes* ops);
OpCodes* codes_new(PSTATE*, int size);
void codes_free(PSTATE*, OpCodes* ops);

#endif
