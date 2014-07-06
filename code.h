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

extern const char *op_names[OP_LASTOP];

typedef struct {
	Eopcode op;
	void *data;
	char *codename;
	int lineno;
} OpCode;

typedef struct OpCodes {
	OpCode *codes;
	int code_len;
	int code_size;
	
	int expr_counter;			/* context related expr count */
	int lvalue_flag;			/* left value count/flag */
	const unichar *lvalue_name;	/* left value name */
} OpCodes;

struct Func;
struct Value;

typedef struct FastVar {
	int context_id;
	struct {
		unichar *varname;
		struct Value *lval;
	} var;
} FastVar;

typedef struct TryInfo {
	int trylen;
	int catchlen;
	int finallen;
} TryInfo;

typedef struct ReservedInfo {
	int type;
	const unichar *label;
	int topop;
} ReservedInfo;

typedef struct JmpPopInfo {
	int off;
	int topop;
} JmpPopInfo;


struct PSTATE;


OpCodes *code_push_undef(struct PSTATE*);
OpCodes *code_push_bool(struct PSTATE*,int v);
OpCodes *code_push_num(struct PSTATE*,double *v);
OpCodes *code_push_string(struct PSTATE*,const unichar *str);
OpCodes *code_push_index(struct PSTATE*,unichar *varname);
OpCodes *code_push_args(struct PSTATE*);
OpCodes *code_push_this(struct PSTATE*);
OpCodes *code_push_func(struct PSTATE*,struct Func *fun);
OpCodes *code_push_regex(struct PSTATE*,regex_t *reg);
OpCodes *code_push_top(struct PSTATE*);
OpCodes *code_push_top2(struct PSTATE*);
OpCodes *code_unref(struct PSTATE*);
OpCodes *code_local(struct PSTATE*,const unichar *varname);

OpCodes *code_nop(struct PSTATE*);
OpCodes *code_neg(struct PSTATE*);
OpCodes *code_pos(struct PSTATE*);
OpCodes *code_bnot(struct PSTATE*);
OpCodes *code_not(struct PSTATE*);
OpCodes *code_mul(struct PSTATE*);
OpCodes *code_div(struct PSTATE*);
OpCodes *code_mod(struct PSTATE*);
OpCodes *code_add(struct PSTATE*);
OpCodes *code_sub(struct PSTATE*);
OpCodes *code_less(struct PSTATE*);
OpCodes *code_greater(struct PSTATE*);
OpCodes *code_lessequ(struct PSTATE*);
OpCodes *code_greaterequ(struct PSTATE*);
OpCodes *code_equal(struct PSTATE*); 
OpCodes *code_notequal(struct PSTATE*);
OpCodes *code_eequ(struct PSTATE*);
OpCodes *code_nneq(struct PSTATE*);
OpCodes *code_band(struct PSTATE*);
OpCodes *code_bor(struct PSTATE*);
OpCodes *code_bxor(struct PSTATE*);
OpCodes *code_shf(struct PSTATE*,int right);

OpCodes *code_assign(struct PSTATE*,int h);
OpCodes *code_subscript(struct PSTATE*,int right_val);
OpCodes *code_inc(struct PSTATE*,int e);
OpCodes *code_dec(struct PSTATE*,int e);

OpCodes *code_fcall(struct PSTATE*,int argc);
OpCodes *code_newfcall(struct PSTATE*,int argc);
OpCodes *code_pop(struct PSTATE*,int n);
OpCodes *code_ret(struct PSTATE*,int n);
OpCodes *code_object(struct PSTATE*,int c);
OpCodes *code_array(struct PSTATE*,int c);
OpCodes *code_key(struct PSTATE*);
OpCodes *code_next(struct PSTATE*);
OpCodes *code_delete(struct PSTATE*,int n);
OpCodes *code_chthis(struct PSTATE*,int n);

OpCodes *code_jfalse(struct PSTATE*,int off);
OpCodes *code_jtrue(struct PSTATE*,int off);
OpCodes *code_jfalse_np(struct PSTATE*,int off);
OpCodes *code_jtrue_np(struct PSTATE*,int off);
OpCodes *code_jmp(struct PSTATE*,int off);
OpCodes *code_eval(struct PSTATE*,int argc);

OpCodes *code_throw(struct PSTATE*);
OpCodes *code_stry(struct PSTATE*,int trylen, int catchlen, int finlen);
OpCodes *code_etry(struct PSTATE*);
OpCodes *code_scatch(struct PSTATE*,const unichar *var);
OpCodes *code_ecatch(struct PSTATE*);
OpCodes *code_sfinal(struct PSTATE*);
OpCodes *code_efinal(struct PSTATE*);
OpCodes *code_throw(struct PSTATE*);
OpCodes *code_with(struct PSTATE*,int withlen);
OpCodes *code_ewith(struct PSTATE*);
OpCodes *code_typeof(struct PSTATE*);

OpCodes *code_debug(struct PSTATE*);
OpCodes *code_reserved(struct PSTATE*,int type, unichar *id);

OpCodes *codes_join(struct PSTATE*,OpCodes *a, OpCodes *b);
OpCodes *codes_join3(struct PSTATE*,OpCodes *a, OpCodes *b, OpCodes *c);
OpCodes *codes_join4(struct PSTATE*,OpCodes *a, OpCodes *b, OpCodes *c, OpCodes *d);

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
void code_reserved_replace(struct PSTATE*,OpCodes *ops, int step_len, int break_only,
						   const unichar *desire_label, int topop);

void code_decode(struct PSTATE*,OpCode *op, int currentip);
void codes_print(struct PSTATE*,OpCodes *ops);
OpCodes *codes_new(struct PSTATE*,int size);
void codes_free(struct PSTATE *,OpCodes *ops);
#endif
