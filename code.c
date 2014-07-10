#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "code.h"

const char* op_names[OP_LASTOP] = {
	"NOP",
	"PUSHNUM",
	"PUSHSTR",
	"PUSHVAR",
	"PUSHUND",
	"PUSHBOO",
	"PUSHFUN",
	"PUSHREG",
	"PUSHARG",
	"PUSHTHS",
	"PUSHTOP",
	"PUSHTOP2",
	"UNREF",
	"POP",
	"LOCAL",
	"NEG",
	"POS",
	"NOT",
	"BNOT",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"MOD",
	"LESS",
	"GREATER",
	"LESSEQU",
	"GREATEREQU",
	"EQUAL",
	"NOTEQUAL",
	"STRICTEQU",
	"STRICTNEQ",
	"BAND",
	"BOR",
	"BXOR",
	"SHF",
	"ASSIGN",
	"SUBSCRIPT",
	"INC",
	"DEC",
	"KEY",
	"NEXT",
	"JTRUE",
	"JFALSE",
	"JTRUE_NP",
	"JFALSE_NP",
	"JMP",
	"JMPPOP",
	"FCALL",
	"NEWFCALL",
	"RET",
	"DELETE",
	"CHTHIS",
	"OBJECT",
	"ARRAY",
	"EVAL",
	"STRY",
	"ETRY",
	"SCATCH",
	"ECATCH",
	"SFINAL",
	"EFINAL",
	"THROW",
	"WITH",
	"EWITH",
	"TYPEOF",
	"RESERVED",
	"DEBUG",
};

#include "pstate.h"

OpCodes*
codes_new (PState* ps, int size)
{
	OpCodes* ret = psmalloc (sizeof (OpCodes));
	memset (ret, 0, sizeof (OpCodes));
	ret->codes = psmalloc (sizeof (OpCode) * size);
	ret->code_size = size;
	return ret;
}

static int
codes_insert (PState* ps, OpCodes* c, Eopcode code, void* extra)
{
	if (c->code_size - c->code_len <= 0) {
		c->code_size += 100;
		c->codes = psrealloc (c->codes, c->code_size * sizeof (OpCode));
	}
	c->codes[c->code_len].op = code;
	c->codes[c->code_len].data = extra;
	c->codes[c->code_len].lineno = 0;
	c->code_len++;
	return 0;
}

OpCodes* codes_lineno (OpCodes* c, char* codename, int lineno)
{
	if (c->code_len == 0) {
		return;
	}
	c->codes[c->code_len-1].codename = codename;
	c->codes[c->code_len-1].lineno = lineno;
	return c;
}


OpCodes*
codes_join (PState* ps, OpCodes* a, OpCodes* b)
{
	OpCodes* ret = codes_new (ps, a->code_len + b->code_len);
	memcpy (ret->codes, a->codes, a->code_len * sizeof (OpCode));
	memcpy (&ret->codes[a->code_len], b->codes, b->code_len * sizeof (OpCode));
	ret->code_size = a->code_len + b->code_len;
	ret->code_len = ret->code_size;
	ret->expr_counter = a->expr_counter + b->expr_counter;
	psfree (a->codes);
	psfree (b->codes);
	psfree (a);
	psfree (b);
	return ret;
}

OpCodes*
codes_join3 (PState* ps, OpCodes* a, OpCodes* b, OpCodes* c)
{
	return codes_join (ps, codes_join (ps, a, b), c);
}

OpCodes*
codes_join4 (PState* ps, OpCodes* a, OpCodes* b, OpCodes* c, OpCodes* d)
{
	return codes_join (ps, codes_join (ps, a, b), codes_join (ps, c, d));
}

#define NEW_CODES(code, extra) do {						\
		OpCodes* r = codes_new(ps, 3);					\
		codes_insert(ps, r, (code), (void*)(extra));	\
		return r;										\
	} while(0)

OpCodes*
code_push_undef (PState* ps)
{
	NEW_CODES (OP_PUSHUND, 0);
}

OpCodes*
code_push_bool (PState* ps, int v)
{
	NEW_CODES (OP_PUSHBOO, v);
}

OpCodes*
code_push_num (PState* ps, double* v)
{
	NEW_CODES (OP_PUSHNUM, v);
}

OpCodes*
code_push_string (PState* ps, const unichar* str)
{
	NEW_CODES (OP_PUSHSTR, str);
}

OpCodes*
code_push_index (PState* ps, unichar* varname)
{
	FastVar* n = psmalloc (sizeof (FastVar));
	memset (n, 0, sizeof (FastVar));
	n->context_id = -1;
	n->var.varname = varname;
	NEW_CODES (OP_PUSHVAR, n);
}

OpCodes*
code_push_this (PState* ps)
{
	NEW_CODES (OP_PUSHTHS, 0);
}

OpCodes*
code_push_top (PState* ps)
{
	NEW_CODES (OP_PUSHTOP, 0);
}

OpCodes*
code_push_top2 (PState* ps)
{
	NEW_CODES (OP_PUSHTOP2, 0);
}

OpCodes*
code_unref (PState* ps)
{
	NEW_CODES (OP_UNREF, 0);
}

OpCodes*
code_push_args (PState* ps)
{
	NEW_CODES (OP_PUSHARG, 0);
}

OpCodes*
code_push_func (PState* ps, Func* fun)
{
	NEW_CODES (OP_PUSHFUN, fun);
}

OpCodes*
code_push_regex (PState* ps, regex_t* reg)
{
	NEW_CODES (OP_PUSHREG, reg);
}

OpCodes*
code_local (PState* ps, const unichar* varname)
{
	NEW_CODES (OP_LOCAL, varname);
}

OpCodes*
code_nop (PState* ps)
{
	NEW_CODES (OP_NOP, 0);
}

OpCodes*
code_neg (PState* ps)
{
	NEW_CODES (OP_NEG, 0);
}

OpCodes*
code_pos (PState* ps)
{
	NEW_CODES (OP_POS, 0);
}

OpCodes*
code_bnot (PState* ps)
{
	NEW_CODES (OP_BNOT, 0);
}

OpCodes*
code_not (PState* ps)
{
	NEW_CODES (OP_NOT, 0);
}

OpCodes*
code_mul (PState* ps)
{
	NEW_CODES (OP_MUL, 0);
}

OpCodes*
code_div (PState* ps)
{
	NEW_CODES (OP_DIV, 0);
}

OpCodes*
code_mod (PState* ps)
{
	NEW_CODES (OP_MOD, 0);
}

OpCodes*
code_add (PState* ps)
{
	NEW_CODES (OP_ADD, 0);
}

OpCodes*
code_sub (PState* ps)
{
	NEW_CODES (OP_SUB, 0);
}

OpCodes*
code_less (PState* ps)
{
	NEW_CODES (OP_LESS, 0);
}

OpCodes*
code_greater (PState* ps)
{
	NEW_CODES (OP_GREATER, 0);
}

OpCodes*
code_lessequ (PState* ps)
{
	NEW_CODES (OP_LESSEQU, 0);
}

OpCodes*
code_greaterequ (PState* ps)
{
	NEW_CODES (OP_GREATEREQU, 0);
}

OpCodes*
code_equal (PState* ps)
{
	NEW_CODES (OP_EQUAL, 0);
}

OpCodes*
code_notequal (PState* ps)
{
	NEW_CODES (OP_NOTEQUAL, 0);
}

OpCodes*
code_eequ (PState* ps)
{
	NEW_CODES (OP_STRICTEQU, 0);
}

OpCodes*
code_nneq (PState* ps)
{
	NEW_CODES (OP_STRICTNEQ, 0);
}

OpCodes*
code_band (PState* ps)
{
	NEW_CODES (OP_BAND, 0);
}

OpCodes*
code_bor (PState* ps)
{
	NEW_CODES (OP_BOR, 0);
}

OpCodes*
code_bxor (PState* ps)
{
	NEW_CODES (OP_BXOR, 0);
}

OpCodes*
code_shf (PState* ps, int right)
{
	NEW_CODES (OP_SHF, right);
}

OpCodes*
code_assign (PState* ps, int h)
{
	NEW_CODES (OP_ASSIGN, h);
}

OpCodes*
code_subscript (PState* ps, int right_val)
{
	NEW_CODES (OP_SUBSCRIPT, right_val);
}

OpCodes*
code_inc (PState* ps, int e)
{
	NEW_CODES (OP_INC, e);
}

OpCodes*
code_dec (PState* ps, int e)
{
	NEW_CODES (OP_DEC, e);
}

OpCodes*
code_fcall (PState* ps, int argc)
{
	NEW_CODES (OP_FCALL, argc);
}

OpCodes*
code_newfcall (PState* ps, int argc)
{
	NEW_CODES (OP_NEWFCALL, argc);
}

OpCodes*
code_ret (PState* ps, int n)
{
	NEW_CODES (OP_RET, n);
}

OpCodes*
code_delete (PState* ps, int n)
{
	NEW_CODES (OP_DELETE, n);
}

OpCodes*
code_chthis (PState* ps, int n)
{
	NEW_CODES (OP_CHTHIS, n);
}

OpCodes*
code_pop (PState* ps, int n)
{
	NEW_CODES (OP_POP, n);
}

OpCodes*
code_jfalse (PState* ps, int off)
{
	NEW_CODES (OP_JFALSE, off);
}

OpCodes*
code_jtrue (PState* ps, int off)
{
	NEW_CODES (OP_JTRUE, off);
}

OpCodes*
code_jfalse_np (PState* ps, int off)
{
	NEW_CODES (OP_JFALSE_NP, off);
}

OpCodes*
code_jtrue_np (PState* ps, int off)
{
	NEW_CODES (OP_JTRUE_NP, off);
}

OpCodes*
code_jmp (PState* ps, int off)
{
	NEW_CODES (OP_JMP, off);
}

OpCodes*
code_object (PState* ps, int c)
{
	NEW_CODES (OP_OBJECT, c);
}

OpCodes*
code_array (PState* ps, int c)
{
	NEW_CODES (OP_ARRAY, c);
}

OpCodes*
code_key (PState* ps)
{
	NEW_CODES (OP_KEY, 0);
}

OpCodes*
code_next (PState* ps)
{
	NEW_CODES (OP_NEXT, 0);
}

OpCodes*
code_eval (PState* ps, int argc)
{
	NEW_CODES (OP_EVAL, argc);
}

OpCodes*
code_stry (PState* ps, int trylen, int catchlen, int finlen)
{
	TryInfo* ti = psmalloc (sizeof (TryInfo));
	ti->trylen = trylen;
	ti->catchlen = catchlen;
	ti->finallen = finlen;
	NEW_CODES (OP_STRY, ti);
}

OpCodes*
code_etry (PState* ps)
{
	NEW_CODES (OP_ETRY, 0);
}

OpCodes*
code_scatch (PState* ps, const unichar* var)
{
	NEW_CODES (OP_SCATCH, var);
}

OpCodes*
code_ecatch (PState* ps)
{
	NEW_CODES (OP_ECATCH, 0);
}

OpCodes*
code_sfinal (PState* ps)
{
	NEW_CODES (OP_SFINAL, 0);
}

OpCodes*
code_efinal (PState* ps)
{
	NEW_CODES (OP_EFINAL, 0);
}

OpCodes*
code_throw (PState* ps)
{
	NEW_CODES (OP_THROW, 0);
}

OpCodes*
code_with (PState* ps, int withlen)
{
	NEW_CODES (OP_WITH, withlen);
}

OpCodes*
code_ewith (PState* ps)
{
	NEW_CODES (OP_EWITH, 0);
}

OpCodes*
code_typeof (PState* ps)
{
	NEW_CODES (OP_TYPEOF, 0);
}

OpCodes*
code_debug (PState* ps)
{
	NEW_CODES (OP_DEBUG, 0);
}

OpCodes*
code_reserved (PState* ps, int type, unichar* id)
{
	ReservedInfo* ri = psmalloc (sizeof (ReservedInfo));
	ri->type = type;
	ri->label = id;
	ri->topop = 0;
	NEW_CODES (OP_RESERVED, ri);
}

JmpPopInfo*
jpinfo_new (PState* ps, int off, int topop)
{
	JmpPopInfo* r = psmalloc (sizeof (JmpPopInfo));
	r->off = off;
	r->topop = topop;
	return r;
}

void
code_reserved_replace (PState* ps, OpCodes* ops, int step_len, int break_only, const unichar* desire_label, int topop)
{
	for (int i=0; i<ops->code_len; ++i) {
		ReservedInfo* ri;
		if (ops->codes[i].op != OP_RESERVED) {
			continue;
		}
		ri = ops->codes[i].data;

		if (ri->label) {
			if (!desire_label || unistrcmp (ri->label, desire_label) != 0) {
				ri->topop += topop;
				continue;
			}
		}
		if (ri->type == RES_CONTINUE) {
			if (break_only) {
				ri->topop += topop;
				continue;
			}else {
				int topop = ri->topop;
				psfree (ri);	// kill reserved info, replace with other opcode
				if (topop) {
					ops->codes[i].data = jpinfo_new (ps, ops->code_len - i, topop);
					ops->codes[i].op = OP_JMPPOP;
				}else {
					ops->codes[i].data = (void *) (ops->code_len - i);
					ops->codes[i].op = OP_JMP;
				}
			}
		}else if (ri->type == RES_BREAK) {
			int topop = ri->topop;
			psfree (ri);
			if (topop)
			{
			ops->codes[i].data =
			jpinfo_new (ps, step_len + ops->code_len - i, topop);
			ops->codes[i].op = OP_JMPPOP;
			}
			else
			{
			ops->codes[i].data = (void *) (step_len + ops->code_len - i);
			ops->codes[i].op = OP_JMP;
			}
		}
	}
}

void
code_decode (PState* ps, OpCode* op, int currentip)
{
	if (op->op < 0 || op->op >= OP_LASTOP) {
		printf ("Bad opcode[%d] at %d\n", op->op, currentip);
	}
	printf ("%d:\t%s", currentip, op_names[op->op]);
	switch (op->op) {
	case OP_PUSHBOO:
	case OP_FCALL:
	case OP_EVAL:
	case OP_POP:
	case OP_ASSIGN:
	case OP_RET:
	case OP_NEWFCALL:
	case OP_DELETE:
	case OP_CHTHIS:
	case OP_OBJECT:
	case OP_ARRAY:
	case OP_SHF:
	case OP_INC:
	case OP_DEC:
		printf ("\t%d\n", (int) op->data);
		break;
	case OP_PUSHNUM:
		printf ("\t%g\n", *((double *) op->data));
		break;
	case OP_PUSHSTR:
	case OP_LOCAL:
	case OP_SCATCH:
		printf ("\t\"%s\"\n", tochars (ps, op->data ? op->data : "(NoCatch)"));
		break;
	case OP_PUSHVAR:
		printf ("\tvar: \"%s\"\n", tochars (ps, ((FastVar *) op->data)->var.varname));
		break;
	case OP_PUSHFUN:
		printf ("\tfunc: 0x%x\n", (int) op->data);
		break;
	case OP_JTRUE:
	case OP_JFALSE:
	case OP_JTRUE_NP:
	case OP_JFALSE_NP:
	case OP_JMP:
		printf ("\t{%d}\t#%d\n", (int) op->data, currentip + (int) op->data);
		break;
	case OP_JMPPOP: {
		JmpPopInfo* jp = op->data;
		printf ("\t{%d},%d\t#%d\n", jp->off, jp->topop, currentip + jp->off);
		break;
	}
	case OP_STRY: {
		TryInfo* t = (TryInfo *) op->data;
		printf ("\t{try:%d, catch:%d, final:%d}\n", t->trylen, t->catchlen,
		t->finallen);
		break;
	}
	default:
		printf ("\n");
		break;
	}
}

void
codes_free (PState* ps, OpCodes* ops)
{
	// TODO
	psfree (ops->codes);
	psfree (ops);
}

void
codes_print (PState* ps, OpCodes* ops)
{
	int i = 0;
	OpCode* opcodes = ops->codes;
	int opcodesi = ops->code_len;

	printf ("opcodes count = %d\n", opcodesi);

	while (i < opcodesi) {
		code_decode (ps, &opcodes[i], i);
		i++;
	}
}

