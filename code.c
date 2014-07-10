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
codes_new (PSTATE* ps, int size)
{
	OpCodes* ret = psmalloc (sizeof (OpCodes));
	memset (ret, 0, sizeof (OpCodes));
	ret->codes = psmalloc (sizeof (OpCode) * size);
	ret->code_size = size;
	return ret;
}

static int
codes_insert (PSTATE* ps, OpCodes* c, Eopcode code, void* extra)
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
codes_join (PSTATE* ps, OpCodes* a, OpCodes* b)
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
codes_join3 (PSTATE* ps, OpCodes* a, OpCodes* b, OpCodes* c)
{
	return codes_join (ps, codes_join (ps, a, b), c);
}

OpCodes*
codes_join4 (PSTATE* ps, OpCodes* a, OpCodes* b, OpCodes* c, OpCodes* d)
{
	return codes_join (ps, codes_join (ps, a, b), codes_join (ps, c, d));
}

#define NEW_CODES(code, extra) do {						\
		OpCodes* r = codes_new(ps, 3);					\
		codes_insert(ps, r, (code), (void*)(extra));	\
		return r;										\
	} while(0)

OpCodes*
code_push_undef (PSTATE* ps)
{
	NEW_CODES (OP_PUSHUND, 0);
}

OpCodes*
code_push_bool (PSTATE* ps, int v)
{
	NEW_CODES (OP_PUSHBOO, v);
}

OpCodes*
code_push_num (PSTATE* ps, double* v)
{
	NEW_CODES (OP_PUSHNUM, v);
}

OpCodes*
code_push_string (PSTATE* ps, const unichar* str)
{
	NEW_CODES (OP_PUSHSTR, str);
}

OpCodes*
code_push_index (PSTATE* ps, unichar* varname)
{
	FastVar* n = psmalloc (sizeof (FastVar));
	memset (n, 0, sizeof (FastVar));
	n->context_id = -1;
	n->var.varname = varname;
	NEW_CODES (OP_PUSHVAR, n);
}

OpCodes*
code_push_this (PSTATE* ps)
{
	NEW_CODES (OP_PUSHTHS, 0);
}

OpCodes*
code_push_top (PSTATE* ps)
{
	NEW_CODES (OP_PUSHTOP, 0);
}

OpCodes*
code_push_top2 (PSTATE* ps)
{
	NEW_CODES (OP_PUSHTOP2, 0);
}

OpCodes*
code_unref (PSTATE* ps)
{
	NEW_CODES (OP_UNREF, 0);
}

OpCodes*
code_push_args (PSTATE* ps)
{
	NEW_CODES (OP_PUSHARG, 0);
}

OpCodes*
code_push_func (PSTATE* ps, Func* fun)
{
	NEW_CODES (OP_PUSHFUN, fun);
}

OpCodes*
code_push_regex (PSTATE* ps, regex_t* reg)
{
	NEW_CODES (OP_PUSHREG, reg);
}

OpCodes*
code_local (PSTATE* ps, const unichar* varname)
{
	NEW_CODES (OP_LOCAL, varname);
}

OpCodes*
code_nop (PSTATE* ps)
{
	NEW_CODES (OP_NOP, 0);
}

OpCodes*
code_neg (PSTATE* ps)
{
	NEW_CODES (OP_NEG, 0);
}

OpCodes*
code_pos (PSTATE* ps)
{
	NEW_CODES (OP_POS, 0);
}

OpCodes*
code_bnot (PSTATE* ps)
{
	NEW_CODES (OP_BNOT, 0);
}

OpCodes*
code_not (PSTATE* ps)
{
	NEW_CODES (OP_NOT, 0);
}

OpCodes*
code_mul (PSTATE* ps)
{
	NEW_CODES (OP_MUL, 0);
}

OpCodes*
code_div (PSTATE* ps)
{
	NEW_CODES (OP_DIV, 0);
}

OpCodes*
code_mod (PSTATE* ps)
{
	NEW_CODES (OP_MOD, 0);
}

OpCodes*
code_add (PSTATE* ps)
{
	NEW_CODES (OP_ADD, 0);
}

OpCodes*
code_sub (PSTATE* ps)
{
	NEW_CODES (OP_SUB, 0);
}

OpCodes*
code_less (PSTATE* ps)
{
	NEW_CODES (OP_LESS, 0);
}

OpCodes*
code_greater (PSTATE* ps)
{
	NEW_CODES (OP_GREATER, 0);
}

OpCodes*
code_lessequ (PSTATE* ps)
{
	NEW_CODES (OP_LESSEQU, 0);
}

OpCodes*
code_greaterequ (PSTATE* ps)
{
	NEW_CODES (OP_GREATEREQU, 0);
}

OpCodes*
code_equal (PSTATE* ps)
{
	NEW_CODES (OP_EQUAL, 0);
}

OpCodes*
code_notequal (PSTATE* ps)
{
	NEW_CODES (OP_NOTEQUAL, 0);
}

OpCodes*
code_eequ (PSTATE* ps)
{
	NEW_CODES (OP_STRICTEQU, 0);
}

OpCodes*
code_nneq (PSTATE* ps)
{
	NEW_CODES (OP_STRICTNEQ, 0);
}

OpCodes*
code_band (PSTATE* ps)
{
	NEW_CODES (OP_BAND, 0);
}

OpCodes*
code_bor (PSTATE* ps)
{
	NEW_CODES (OP_BOR, 0);
}

OpCodes*
code_bxor (PSTATE* ps)
{
	NEW_CODES (OP_BXOR, 0);
}

OpCodes*
code_shf (PSTATE* ps, int right)
{
	NEW_CODES (OP_SHF, right);
}

OpCodes*
code_assign (PSTATE* ps, int h)
{
	NEW_CODES (OP_ASSIGN, h);
}

OpCodes*
code_subscript (PSTATE* ps, int right_val)
{
	NEW_CODES (OP_SUBSCRIPT, right_val);
}

OpCodes*
code_inc (PSTATE* ps, int e)
{
	NEW_CODES (OP_INC, e);
}

OpCodes*
code_dec (PSTATE* ps, int e)
{
	NEW_CODES (OP_DEC, e);
}

OpCodes*
code_fcall (PSTATE* ps, int argc)
{
	NEW_CODES (OP_FCALL, argc);
}

OpCodes*
code_newfcall (PSTATE* ps, int argc)
{
	NEW_CODES (OP_NEWFCALL, argc);
}

OpCodes*
code_ret (PSTATE* ps, int n)
{
	NEW_CODES (OP_RET, n);
}

OpCodes*
code_delete (PSTATE* ps, int n)
{
	NEW_CODES (OP_DELETE, n);
}

OpCodes*
code_chthis (PSTATE* ps, int n)
{
	NEW_CODES (OP_CHTHIS, n);
}

OpCodes*
code_pop (PSTATE* ps, int n)
{
	NEW_CODES (OP_POP, n);
}

OpCodes*
code_jfalse (PSTATE* ps, int off)
{
	NEW_CODES (OP_JFALSE, off);
}

OpCodes*
code_jtrue (PSTATE* ps, int off)
{
	NEW_CODES (OP_JTRUE, off);
}

OpCodes*
code_jfalse_np (PSTATE* ps, int off)
{
	NEW_CODES (OP_JFALSE_NP, off);
}

OpCodes*
code_jtrue_np (PSTATE* ps, int off)
{
	NEW_CODES (OP_JTRUE_NP, off);
}

OpCodes*
code_jmp (PSTATE* ps, int off)
{
	NEW_CODES (OP_JMP, off);
}

OpCodes*
code_object (PSTATE* ps, int c)
{
	NEW_CODES (OP_OBJECT, c);
}

OpCodes*
code_array (PSTATE* ps, int c)
{
	NEW_CODES (OP_ARRAY, c);
}

OpCodes*
code_key (PSTATE* ps)
{
	NEW_CODES (OP_KEY, 0);
}

OpCodes*
code_next (PSTATE* ps)
{
	NEW_CODES (OP_NEXT, 0);
}

OpCodes*
code_eval (PSTATE* ps, int argc)
{
	NEW_CODES (OP_EVAL, argc);
}

OpCodes*
code_stry (PSTATE* ps, int trylen, int catchlen, int finlen)
{
	TryInfo* ti = psmalloc (sizeof (TryInfo));
	ti->trylen = trylen;
	ti->catchlen = catchlen;
	ti->finallen = finlen;
	NEW_CODES (OP_STRY, ti);
}

OpCodes*
code_etry (PSTATE* ps)
{
	NEW_CODES (OP_ETRY, 0);
}

OpCodes*
code_scatch (PSTATE* ps, const unichar* var)
{
	NEW_CODES (OP_SCATCH, var);
}

OpCodes*
code_ecatch (PSTATE* ps)
{
	NEW_CODES (OP_ECATCH, 0);
}

OpCodes*
code_sfinal (PSTATE* ps)
{
	NEW_CODES (OP_SFINAL, 0);
}

OpCodes*
code_efinal (PSTATE* ps)
{
	NEW_CODES (OP_EFINAL, 0);
}

OpCodes*
code_throw (PSTATE* ps)
{
	NEW_CODES (OP_THROW, 0);
}

OpCodes*
code_with (PSTATE* ps, int withlen)
{
	NEW_CODES (OP_WITH, withlen);
}

OpCodes*
code_ewith (PSTATE* ps)
{
	NEW_CODES (OP_EWITH, 0);
}

OpCodes*
code_typeof (PSTATE* ps)
{
	NEW_CODES (OP_TYPEOF, 0);
}

OpCodes*
code_debug (PSTATE* ps)
{
	NEW_CODES (OP_DEBUG, 0);
}

OpCodes*
code_reserved (PSTATE* ps, int type, unichar* id)
{
	ReservedInfo* ri = psmalloc (sizeof (ReservedInfo));
	ri->type = type;
	ri->label = id;
	ri->topop = 0;
	NEW_CODES (OP_RESERVED, ri);
}

JmpPopInfo*
jpinfo_new (PSTATE* ps, int off, int topop)
{
	JmpPopInfo* r = psmalloc (sizeof (JmpPopInfo));
	r->off = off;
	r->topop = topop;
	return r;
}

void
code_reserved_replace (PSTATE* ps, OpCodes* ops, int step_len, int break_only, const unichar* desire_label, int topop)
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
code_decode (PSTATE* ps, OpCode* op, int currentip)
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
codes_free (PSTATE* ps, OpCodes* ops)
{
	// TODO
	psfree (ops->codes);
	psfree (ops);
}

void
codes_print (PSTATE* ps, OpCodes* ops)
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

