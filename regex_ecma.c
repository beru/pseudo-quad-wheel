/*
 * Copyright (c) 2003
 *      David Leonard.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of David Leonard nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uregex.h"
#include "misc.h"

#if 0
#if linux
#define _STRING_ALLOCA(interp, ch, statesz)  (ch *)alloca(statesz)
#define _STRING_ALLOCAFREE(q, p)  
#else
#define _STRING_ALLOCA(interp, ch, statesz)  (ch *)malloc(statesz)
#define _STRING_ALLOCAFREE(q, p)  free(p)
#endif

#define _NEW(p, t)			malloc(sizeof(t))
#define NEW1(rec, t)			malloc(sizeof(t))
#define _MALLOC(p,t)                malloc(t)
#define _FREE1(p,q)                  free(q)

#else

static void* (*_localmalloc)(void* c, unsigned int size);
static void (*_localfree)(void* c, void* p);

#define _STRING_ALLOCA(interp, ch, statesz)  (ch *)_localmalloc(interp, statesz)
#define _STRING_ALLOCAFREE(ec,q)  _localfree(ec,q)

#define _NEW(p, t)			_localmalloc(p, sizeof(t))
#define NEW1(rec, t)			_localmalloc(rec->interpreter, sizeof(t))
#define _MALLOC(p,t)                _localmalloc(p, t)
#define _FREE1(rec,q)                  _localfree(rec->interpreter, q)

#endif

struct capture
{
  unsigned int cap_start, cap_end;
};

typedef unsigned int _unicode_t;


int
syntaxerror ()
{
  abort ();
}

int
internalerror ()
{
  abort ();
}

#define _INPUT_NEXT(i)	(*(i)->inputclass->next)(i)
#define _INPUT_CLOSE(i)	(*(i)->inputclass->close)(i)
int
UNICODE_TOUPPER (int ch)
{
  if ('a' <= ch && ch <= 'z')
    ch = ch - 'a' + 'A';
  return ch;
}

/*
 * Regular expression engine.
 *
 * This module contains a parser to compile ECMA-262 regular expressions
 * into 'p-code', and a matcher engine that runs the p-code against
 * strings.
 *
 * NOTE: ECMA-262 "regular" expressions are not actually regular in
 * a technical sense, since the presence of non-greedy and zero-width 
 * lookahead patterns make the matching process an NP-complete algorithm
 * rather than a linear process implied by context-free finite automata.
 * (See the NP-complete regex discussion at <http://perl.plover.com/NPC/>)
 *
 * The regex parser generates a 'struct ecma_regex' which contains the bytecode 
 * ('p-code'), that when executed will try to match the pattern against the
 * given string, commencing at the given index into the string.
 */

#ifndef NDEBUG
int _regex_debug = 0;
#endif

#define _COMPAT_JS(a,b,c) 1

#define	OP_FAIL		 0	/* match failed */
#define	OP_SUCCEED	 1	/* match succeeded */
#define	OP_CHAR		 2	/* match a char class instance */
#define	OP_ZERO		 3	/* reset counter */
#define	OP_REACH	 4	/* test counter over */
#define	OP_NREACH	 5	/* test counter under */
#define	OP_START	 6	/* enter a group */
#define	OP_END		 7	/* exit a group */
#define	OP_UNDEF	 8	/* reset a group */
#define	OP_MARK		 9	/* record a position */
#define	OP_FDIST	10	/* position test */
#define	OP_RDIST	11	/* position and counter test */
#define	OP_MNEXT	12	/* max-loop */
#define	OP_RNEXT	13	/* reach-loop */
#define	OP_GOTO		14	/* branch */
#define	OP_GS		15	/* greedy success */
#define	OP_NS		16	/* non-greedy success */
#define	OP_GF		17	/* greedy fail */
#define	OP_NF		18	/* non-greedy fail */
#define	OP_AS		19	/* assert success */
#define	OP_AF		20	/* assert fail */
#define	OP_BOL		21	/* test beginning of line */
#define	OP_EOL		22	/* test end of line */
#define	OP_BRK		23	/* test word-break */
#define	OP_NBRK		24	/* test non-word-break */
#define	OP_BACKREF	25	/* backreference match */

struct charclassrange
{
  struct charclassrange *next;
  _unicode_t lo, hi;		/* simple range of chars, eg [a-z] */
};

struct charclass
{
  struct charclassrange *ranges;	/* linked list of character ranges */
};

#define _INPUT_BADCHAR	((_unicode_t)0x100000)

struct _growable
{
  void **data_ptr;		/* Reference to base pointer */
  unsigned int *length_ptr;	/* Reference to element use count */
  size_t element_size;		/* Size of an element */
  size_t allocated;		/* Bytes of storage addressed by *data_ptr */
  unsigned int is_string:1;	/* Use SEE_malloc_string */
};
struct execcontext;
/* Sets the new length of a growable array */
static void _grow_to (struct execcontext *i, struct _growable *grow,
		      unsigned int new_len);
#define _GROW_INIT(i,g,ptr,len) do {			\
	(ptr) = 0; (len) = 0;				\
	(g)->data_ptr = (void **)&(ptr);		\
	(g)->length_ptr = &(len);			\
	(g)->element_size = sizeof (ptr)[0];		\
	(g)->allocated = 0;				\
	(g)->is_string = 0;				\
    } while (0)


/* A slightly faster variant of SEE_grow_to() */
#define _GROW_TO(i,g,l) do {				\
	if ((l) > (g)->allocated / (g)->element_size)	\
	    _grow_to(i, g, l);			\
	else						\
	    *(g)->length_ptr = (l);			\
    } while (0)


#define GROW_INITIAL_SIZE   64	/* bytes */
#define GROW_MAXIMUM_SIZE (0x7fffffff-128)


static void
_grow_to (interp, grow, new_len)
     struct execcontext *interp;
     struct _growable *grow;
     unsigned int new_len;
{
  size_t new_alloc;
  void *new_ptr;

  if (new_len >= GROW_MAXIMUM_SIZE / grow->element_size)
    internalerror ();
  new_alloc = grow->allocated;
  while (new_len * grow->element_size > new_alloc)
    if (new_alloc < GROW_INITIAL_SIZE / 2)
      new_alloc = GROW_INITIAL_SIZE;
    else if (new_alloc >= GROW_MAXIMUM_SIZE / 2)
      new_alloc = GROW_MAXIMUM_SIZE;
    else
      new_alloc *= 2;

  if (new_alloc > grow->allocated)
    {
      if (grow->is_string)
	new_ptr = _MALLOC (interp, new_alloc);
      else
	new_ptr = _MALLOC (interp, new_alloc);
      if (*grow->length_ptr)
	memcpy (new_ptr, *grow->data_ptr,
		*grow->length_ptr * grow->element_size);
      *grow->data_ptr = new_ptr;
      grow->allocated = new_alloc;
    }
  *grow->length_ptr = new_len;
}


struct _string
{
	int stringclass;
	int interpreter;
	int flags;
	int length;
	const unsigned short* data;
};

struct _input;

struct _inputclass
{
  /* Returns the next character on the stream. Invalid if !eof */
  _unicode_t (*next) (struct _input *);
  /* Releases system resources allocated to input */
  void (*close) (struct _input *);
};

struct _input
{
  struct execcontext *interpreter;
  struct _inputclass *inputclass;
  int first_lineno;
  int lookahead;
  int eof;
};

struct _input_string
{
  struct _input inp;
  const unsigned short *cur, *end;
};

struct _input_lookahead
{
  struct _input input;
  struct _input *sub;
  int max, ptr;
  struct
  {
    _unicode_t ch;
    int eof;
  } buf[1];
};

static _unicode_t _input_utf8_next (struct _input *);
static void _input_utf8_close (struct _input *);

static struct _inputclass input_utf8_class = {
  _input_utf8_next,
  _input_utf8_close
};

struct input_utf8
{
  struct _input inp;
  const unsigned char *s;
};

static _unicode_t
_input_utf8_next (inp)
     struct _input *inp;
{
  struct input_utf8 *inpu = (struct input_utf8 *) inp;
  _unicode_t next, c;
  int i, j, bytes;
  static unsigned char mask[] = { 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };
  static _unicode_t safe[] = { 0, 0x80, 0x800, 0x10000, 0x200000,
    0x4000000, 0x80000000
  };

  next = inpu->inp.lookahead;

  /*
   * Decode a UTF8 string, returning EOF if we reach a nul character.
   * This is a non-strict decoder.
   */
  if (*inpu->s == '\0')
    {
      inpu->inp.eof = 1;
    }
  else if ((*inpu->s & 0x80) == 0)
    {
      inpu->inp.lookahead = *inpu->s++;
      inpu->inp.eof = 0;
    }
  else
    {
      for (bytes=1; bytes<6; bytes++)
	if ((*inpu->s & mask[bytes]) == mask[bytes - 1])
	  break;
      if (bytes < 6)
	{
	  c = *inpu->s++ & ~mask[bytes];
	  for (i=bytes, j=0; i--; j++)
	    {
	      if ((*inpu->s & 0xc0) != 0x80)
		{
		  goto bad;
		}
	      c = (c << 6) | (*inpu->s++ & 0x3f);
	    }
#define _UNICODE_MAX 0x10ffff
	  if (c > _UNICODE_MAX)
	    inpu->inp.lookahead = _INPUT_BADCHAR;
	  else if (c < safe[bytes])
	    inpu->inp.lookahead = _INPUT_BADCHAR;
	  else
	    inpu->inp.lookahead = c;

	  inpu->inp.eof = 0;
	}
      else
	{
	bad:
	  inpu->inp.lookahead = _INPUT_BADCHAR;
	  inpu->inp.eof = 0;
	  while ((*inpu->s & 0x80))
	    inpu->s++;
	}
    }
  return next;
}

static void
_input_utf8_close (inp)
     struct _input *inp;
{
  /* struct input_utf8 *inpu = (struct input_utf8 *)inp; */
  /* nothing */
}

struct _input *
_input_utf8_new (interp, s)
     struct execcontext *interp;
     const char *s;
{
  struct input_utf8 *inpu;

  inpu = _NEW (interp, struct input_utf8);
  inpu->inp.interpreter = interp;
  inpu->inp.inputclass = &input_utf8_class;
  /*      inpu->inp.filename = NULL;
     inpu->inp.first_lineno = 1; */
  inpu->s = (const unsigned char *) s;
  _INPUT_NEXT ((struct _input *) inpu);	/* prime */
  return (struct _input *) inpu;
}

static _unicode_t _input_string_next (struct _input *inp);
static void _input_string_close (struct _input *inp);

static struct _inputclass input_string_class = {
  _input_string_next,
  _input_string_close
};


static _unicode_t
_input_string_next (inp)
     struct _input *inp;
{
  struct _input_string *inps = (struct _input_string *) inp;
  _unicode_t next, c, c2;

  next = inps->inp.lookahead;
  if (inps->cur >= inps->end)
    {
      inps->inp.eof = 1;
    }
  else
    {
      c = *inps->cur++;
      if ((c & 0xfc00) == 0xd800 && inps->cur < inps->end)
	{
	  c2 = *inps->cur;
	  if ((c2 & 0xfc00) == 0xdc00)
	    {
	      inps->cur++;
	      c = (((c & 0x3ff) << 10) | (c2 & 0x3ff)) + 0x10000;
	    }
	  else
	    {
	      c = _INPUT_BADCHAR;
	    }
	}
      inps->inp.lookahead = c;
      inps->inp.eof = 0;
    }
  return next;
}

static void
_input_string_close (inp)
     struct _input *inp;
{
  struct _input_string *inps = (struct _input_string *) inp;

  inps->cur = NULL;
  inps->end = NULL;
}

struct _input *
_input_string_new (interp, s)
     struct execcontext *interp;
     struct _string *s;
{
  struct _input_string *inps;

  inps = _NEW (interp, struct _input_string);
  inps->cur = s->data;
  inps->end = s->data + s->length;
  inps->inp.interpreter = interp;
  inps->inp.inputclass = &input_string_class;
  inps->inp.first_lineno = 1;
  _INPUT_NEXT ((struct _input *) inps);
  return (struct _input *) inps;
}


/*
 * an n-character lookahead input filter
 */

static _unicode_t la_next (struct _input *);
static void la_close (struct _input *);

static struct _inputclass la_inputclass = {
  la_next,
  la_close
};

static _unicode_t
la_next (inp)
     struct _input *inp;
{
  struct _input_lookahead *la = (struct _input_lookahead *) inp;
  _unicode_t next = inp->lookahead;
  struct _input *sub = la->sub;

  inp->lookahead = la->buf[la->ptr].ch;
  inp->eof = la->buf[la->ptr].eof;
  la->buf[la->ptr].ch = sub->lookahead;
  la->buf[la->ptr].eof = sub->eof;
  if (!sub->eof)
    _INPUT_NEXT (sub);
  la->ptr = (la->ptr + 1) % la->max;
  return next;
}

/*
 * Return the lookahead buffer that we have available
 */
int
_input_lookahead_copy (inp, buf, buflen)
     struct _input *inp;
     _unicode_t *buf;
     int buflen;
{
  struct _input_lookahead *la = (struct _input_lookahead *) inp;
  int i;

  if (buflen <= 0 || inp->eof)
    return 0;
  buf[0] = inp->lookahead;
  for (i=0; i<la->max && i+1<buflen && !la->buf[(la->ptr + i) % la->max].eof; i++) {
    buf[i + 1] = la->buf[(la->ptr + i) % la->max].ch;
  }
  return i + 1;
}

static void
la_close (inp)
     struct _input *inp;
{
  struct _input_lookahead *la = (struct _input_lookahead *) inp;

  _INPUT_CLOSE (la->sub);
}

struct _input *
_input_lookahead_new (sub, max)
     struct _input *sub;
     int max;
{
  struct _input_lookahead *la;
  int i;

  la = (struct _input_lookahead *) _MALLOC (sub->interpreter,
					    sizeof (struct _input_lookahead) +
					    (max - 1) * sizeof la->buf[0]);
  la->input.inputclass = &la_inputclass;
  la->input.first_lineno = sub->first_lineno;
  la->input.interpreter = sub->interpreter;
  la->sub = sub;
  la->ptr = 0;
  la->max = max;
  for (i=0; i<max+1; i++)
    la_next ((struct _input *) la);
  return (struct _input *) la;
}



struct ecma_regex
{
  struct uregex regex;
  int ncaptures, ncounters, nmarks, maxref;
  int statesz;
  unsigned char *code;
  unsigned int codelen;
  struct _growable codegrow;
  struct charclass **cc;
  unsigned int cclen;
  struct _growable ccgrow;
  int flags;
};

#define REGEX_CAST(aregex)   ((struct ecma_regex *)(aregex))

struct recontext
{
  struct execcontext *interpreter;
  struct _input *input;
  struct ecma_regex *regex;
};


#define NEXT			(recontext->input->lookahead)
#define SKIP			_INPUT_NEXT(recontext->input)
#define ATEOF			(recontext->input->eof)
#define LOOKAHEAD(buf, len)	_input_lookahead_copy(	\
					recontext->input, buf, len)



#if 0
#define SYNTAX_ERROR						\
	SEE_error_throw_string(recontext->interpreter,		\
		recontext->interpreter->SyntaxError, 		\
		STR(regex_syntax_error))
#else
#define SYNTAX_ERROR		syntaxerror()
#endif


#define EXPECT(c)    		do { if (ATEOF || NEXT != (c)) 	\
					SYNTAX_ERROR; 		\
				     SKIP; } while (0)

#define RELADDR(base, addr)	(addr) - (base)
#define ASSERT(x)		_ASSERT(recontext->interpreter, x)
#define CODE_ADD(c)		code_add(recontext, c)
#define CODE_INSERT(pos, n)	code_insert(recontext, pos, n)
#define CODE_POS		recontext->regex->codelen
#define CODE_PATCH(pos, c)	recontext->regex->code[pos] = c
#define CODE_ADDI(i) 						\
    do { unsigned int _i = (i);					\
	 CODE_ADD((_i >> 8) & 0xff); 				\
	 CODE_ADD(_i & 0xff); } while (0)
#define CODE_ADDA(i)		CODE_ADDI(RELADDR(CODE_POS, i))
#define CODE_PATCHI(pos, i) 					\
    do { CODE_PATCH(pos, ((i) >> 8) & 0xff); 			\
	 CODE_PATCH((pos)+1, (i) & 0xff); } while (0)
#define CODE_PATCHA(addr, i)	CODE_PATCHI(addr, RELADDR(addr, i))
#define CODE_SZA	2
#define CODE_SZI	2

#define CODE_MAKEI(code, addr)  ((code[addr] << 8) | code[(addr)+1])
#define CODE_MAKEA(code, addr)  ((CODE_MAKEI(code, addr) + (addr)) & 0xffff)

#define CC_NEW()                cc_new(recontext)
#define CC_ADDRANGE(cc, l, h)   cc_add_range(recontext, cc, l, (h)+1)
#define CC_ADDCHAR(cc, ch)      CC_ADDRANGE(cc, ch, ch)
#define CC_INVERT(cc)	        cc_invert(recontext, cc)
#define CC_ADDCC(dst, src)      cc_add_cc(recontext, dst, src)
#define CC_INTERN(cc)	        cc_intern(recontext, cc)

#define UNDEFINED 	(-1)
#undef INFINITY			/* XXX fixme - rename INFINITY in this file maybe? */
#define INFINITY	(-1)

/* Prototypes */
static struct charclass *cc_new (struct recontext *);
static void cc_add_range (struct recontext *, struct charclass *,
			  _unicode_t, _unicode_t);
static void cc_invert (struct recontext *, struct charclass *);
static void cc_add_cc (struct recontext *, struct charclass *,
		       struct charclass *);
static int cc_issingle (struct charclass *);
static unsigned int cc_count (struct charclass *);
static int cc_cmp (struct charclass *, struct charclass *);
static int cc_intern (struct recontext *, struct charclass *);
static int cc_contains (struct charclass *, _unicode_t);
static struct ecma_regex *ecma_regex_new (struct recontext *);
static void code_add (struct recontext *, int);
static void code_insert (struct recontext *, int, int);

static void Disjunction_parse (struct recontext *);
static void Alternative_parse (struct recontext *);
static void Term_parse (struct recontext *);
static int Quantifier_is_next (struct recontext *);
static int Integer_parse (struct recontext *);
static void Atom_parse (struct recontext *);
static unsigned char HexDigit_parse (struct recontext *);
static struct charclass *ClassEscape_parse (struct recontext *);
static struct charclass *ClassAtom_parse (struct recontext *);
static struct charclass *CanonicalizeClass (struct recontext *,
					    struct charclass *);
static struct charclass *CharacterClass_parse (struct recontext *);

#ifndef NDEBUG
static void dprint_ch (_unicode_t);
static void dprint_cc (struct charclass *);
static int dprint_code (struct ecma_regex *, int);
static void dprint_regex (struct ecma_regex *);
#endif



static _unicode_t Canonicalize (struct ecma_regex *, _unicode_t);
static int pcode_run (struct execcontext *, struct ecma_regex *,
		      unsigned int, struct _string *, char *);
static void optimize_regex (struct execcontext *, struct ecma_regex *);

/*------------------------------------------------------------
 * charclass
 */

/* Create a new, empty charclass */
static struct charclass *
cc_new (recontext)
     struct recontext *recontext;
{
  struct charclass *c;

  c = NEW1 (recontext, struct charclass);
  c->ranges = NULL;
  return c;
}

/* Add a range to a charclass */
static void
cc_add_range (recontext, c, lo, hi)
     struct recontext *recontext;
     struct charclass *c;
     _unicode_t lo, hi;
{
  struct charclassrange *s, *newr = NULL;
  struct charclassrange **rp;
  for (rp = &c->ranges; *rp; rp = &(*rp)->next)
    if (lo <= (*rp)->hi)
      break;

  if (!*rp || hi < (*rp)->lo)
    {
      newr = NEW1 (recontext, struct charclassrange);
      newr->lo = lo;
      newr->hi = hi;
      newr->next = *rp;
      *rp = newr;
    }
  else
    {
      if (lo < (*rp)->lo)
	(*rp)->lo = lo;
      if (hi > (*rp)->hi)
	{
	  (*rp)->hi = hi;
	  s = (*rp)->next;
	  while (s && s->hi < hi)
	    (*rp)->next = s = s->next;
	  if (s && s->lo <= hi)
	    {
	      (*rp)->hi = s->hi;
	      (*rp)->next = s->next;
	    }
	}
    }
}

/* Invert a charclass */
static void
cc_invert (recontext, c)
     struct recontext *recontext;
     struct charclass *c;
{
  struct charclassrange *l, *newlist, *r;
  r = c->ranges;
  if (r && r->lo == 0 && r->hi == ~0)
    {
      c->ranges = NULL;
      return;
    }
  l = newlist = NEW1 (recontext, struct charclassrange);
  if (r && r->lo == 0)
    {
      l->lo = r->hi;
      r = r->next;
    }
  else
    l->lo = 0;
  for (; r; r = r->next)
    {
      l->hi = r->lo;
      if (r->hi == ~0)
	{
	  l->next = NULL;
	  l = NULL;
	  break;
	}
      l = (l->next = NEW1 (recontext, struct charclassrange));
      l->lo = r->hi;
    }
  if (l)
    {
      l->hi = ~0;
      l->next = NULL;
    }
  c->ranges = newlist;
}

static void
cc_add_cc (recontext, dst, src)
     struct recontext *recontext;
     struct charclass *dst, *src;
{
  struct charclassrange *r;

  /* XXX very inefficient */
  for (r = src->ranges; r; r = r->next)
    cc_add_range (recontext, dst, r->lo, r->hi);
}

static int
cc_issingle (c)
     struct charclass *c;
{
  struct charclassrange *r = c->ranges;
  return r != NULL && r->next == NULL && r->lo + 1 == r->hi;
}

/* Return the number of characters in the class */
static unsigned int
cc_count (c)
     struct charclass *c;
{
  unsigned int count = 0;
  struct charclassrange *r;

  for (r = c->ranges; r; r = r->next)
    count += r->hi - r->lo;
  return count;
}

/* Return 0 if two charclasses are identical */
static int
cc_cmp (c1, c2)
     struct charclass *c1, *c2;
{
  struct charclassrange *r1, *r2;

  r1 = c1->ranges;
  r2 = c2->ranges;
  while (r1 && r2)
    {
      if (r1->lo != r2->lo)
	return r1->lo - r2->lo;
      if (r1->hi != r2->hi)
	return r1->hi - r2->hi;
      r1 = r1->next;
      r2 = r2->next;
    }
  if (r1)
    return 1;
  if (r2)
    return -1;
  return 0;
}

/* Insert charclass into recontext, returning unique ID */
static int
cc_intern (recontext, c)
     struct recontext *recontext;
     struct charclass *c;
{
  struct ecma_regex* regex = recontext->regex;
  struct execcontext* interp = recontext->interpreter;
  unsigned int i;

  for (i = 0; i < regex->cclen; i++)
    if (cc_cmp (c, regex->cc[i]) == 0)
      return i;
  i = regex->cclen;
  _grow_to (interp, &regex->ccgrow, regex->cclen + 1);
  regex->cc[i] = c;
  return i;
}

/* Return true if charclass c contains character ch */
static int
cc_contains (c, ch)
     struct charclass *c;
     _unicode_t ch;
{
  struct charclassrange *r;

  for (r = c->ranges; r; r = r->next)
    {
      if (ch >= r->lo && ch < r->hi)
	return 1;
      if (ch < r->lo)
	return 0;
    }
  return 0;
}

#ifndef NDEBUG
/* Print a character in a readable form. */
static void
dprint_ch (ch)
     _unicode_t ch;
{
  switch (ch)
    {
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case '-':
    case '.':
    case '^':
    case '$':
    case '|':
    case '?':
    case '*':
    case '+':
    case '\\':
      DPRINTF (("\\%c", ch & 0x7f));
      break;
    case 0x0000:
      DPRINTF (("\\0"));
      break;
    case 0x0009:
      DPRINTF (("\\t"));
      break;
    case 0x000a:
      DPRINTF (("\\n"));
      break;
    case 0x000b:
      DPRINTF (("\\v"));
      break;
    case 0x000c:
      DPRINTF (("\\f"));
      break;
    case 0x000d:
      DPRINTF (("\\r"));
      break;
    default:
      if (ch >= ' ' && ch <= '~')
	DPRINTF (("%c", ch & 0x7f));
      else if (ch < 0x100)
	DPRINTF (("\\x%02x", ch & 0xff));
      else
	DPRINTF (("\\u%04x", ch));
    }
}
#endif

#ifndef NDEBUG
/* Print a character class in a readable form. */
static void
dprint_cc (c)
     struct charclass *c;
{
  struct charclassrange *r;

  DPRINTF (("["));
  if (c->ranges && c->ranges->lo == 0)
    {
      DPRINTF (("^"));
      for (r = c->ranges; r; r = r->next)
	{
	  if (r->next)
	    {
	      dprint_ch (r->hi);
	      if (r->next->lo != r->hi + 1)
		{
		  DPRINTF (("-"));
		  dprint_ch (r->next->lo - 1);
		}
	    }
	  else if (r->hi != ~0)
	    {
	      dprint_ch (r->hi);
	      DPRINTF (("-"));
	      dprint_ch (~0);
	    }
	}
    }
  else
    for (r = c->ranges; r; r = r->next)
      {
	dprint_ch (r->lo);
	if (r->hi != r->lo + 1)
	  {
	    DPRINTF (("-"));
	    dprint_ch (r->hi - 1);
	  }
      }
  DPRINTF (("]"));
}
#endif

/*------------------------------------------------------------
 * regex and pcode construction
 */

/*
 * Allocate a new regex structure. This will contain everything needed 
 * to run and match a string, independent of the original pattern text.
 */
static struct ecma_regex *
ecma_regex_new (recontext)
     struct recontext *recontext;
{
  struct ecma_regex *regex;

  regex = NEW1 (recontext, struct ecma_regex);
  regex->ncaptures = 0;
  regex->maxref = 0;
  regex->ncounters = 0;
  regex->nmarks = 0;
  regex->statesz = 0;
  _GROW_INIT (recontext->interpreter, &regex->codegrow,
	      regex->code, regex->codelen);
  regex->codegrow.is_string = 1;
  _GROW_INIT (recontext->interpreter, &regex->ccgrow,
	      regex->cc, regex->cclen);
  regex->flags = 0;
  return regex;
}

/* add to the end of the p-code array, resizing as needed */
static void
code_add (recontext, c)
     struct recontext *recontext;
     int c;
{
  struct ecma_regex *regex = recontext->regex;
  struct execcontext *interp = recontext->interpreter;
  unsigned int i;

  i = regex->codelen;
  _grow_to (interp, &regex->codegrow, i + 1);
  regex->code[i] = c;
}

/* insert some bytes into the middle of the p-code, resizing as needed */
static void
code_insert (recontext, pos, n)
     struct recontext *recontext;
     int pos, n;
{
  struct ecma_regex *regex = recontext->regex;
  int i;

  for (i = 0; i < n; i++)
    code_add (recontext, 0);
  for (i = regex->codelen - n; i > pos; i--)
    regex->code[i - 1 + n] = regex->code[i - 1];
}

/*------------------------------------------------------------
 * Parser
 *
 * This recursive descent parser builds a p-code array as it runs.
 * During recursion, the p-code array is sometimes 'back-patched'
 * because branch distances weren't known in advance. In some
 * cases, p-code segments are also shifted. This all means that we
 * have to be very careful that our p-code is quite relocatable,
 * and not dependendent on absolute addresses.
 */

/* parse a source pattern, and return a filled-in regex structure */
struct uregex *
_ecma_regex_compile (interp, source, flags)
     struct execcontext *interp;
     struct _string *source;
     int flags;
{
  struct recontext *recontext;
  struct ecma_regex *regex;

  recontext = _NEW (interp, struct recontext);
  recontext->interpreter = interp;
  recontext->input =
    _input_lookahead_new (_input_string_new (interp, source), 24);
  recontext->regex = regex = ecma_regex_new (recontext);
  regex->flags = flags;
  regex->regex.interp = interp;
  /*      regex->regex.engine = &_SEE_ecma_regex_engine; */

  regex->ncaptures = 1;
  Disjunction_parse (recontext);
  if (!ATEOF)
    SYNTAX_ERROR;

  CODE_ADD (OP_SUCCEED);

  /* Check that no backreferences were too big */
  if (regex->maxref >= regex->ncaptures)
    SYNTAX_ERROR;

  /* XXX - should this close be enclosed in a 'finally'? */
  _INPUT_CLOSE (recontext->input);

  /* compute the size of a captures context */
  regex->statesz =
    regex->ncaptures * sizeof (struct capture) +
    regex->ncounters * sizeof (int) + regex->nmarks * sizeof (int);

  optimize_regex (interp, regex);

#ifndef NDEBUG
  if (_regex_debug)
    {
      DPRINTF (("regex:"));
      dprint_regex (regex);
      DPRINTF ((".\n"));
    }
#endif

  return &regex->regex;
}

struct uregex *
ecma_regex_compile (struct execcontext *interp, const unsigned short *source,
		    int len, int flags)
{
  struct _string s;
  s.stringclass = 0;
  s.interpreter = 0;
  s.flags = 0;
  s.data = source;
  s.length = len;
  return _ecma_regex_compile (interp, &s, flags);
}

/* Returns the number of capture parentheses in the compiled regex */
int
ecma_regex_count_captures (aregex)
     struct uregex *aregex;
{
  struct ecma_regex *regex = REGEX_CAST (aregex);

  return regex->ncaptures;
}

/* Returns the flags of the regex object */
int
ecma_regex_get_flags (aregex)
     struct uregex *aregex;
{
  struct ecma_regex *regex = REGEX_CAST (aregex);

  return regex->flags;
}

/*
 * Disjunction :: Alternative
 *		  Alternative | Disjunction
 */

static void
Disjunction_parse (recontext)
     struct recontext *recontext;
{
  int pos;

  pos = CODE_POS;
  Alternative_parse (recontext);
  if (!ATEOF && NEXT == '|')
    {
      int p = pos, p1, p2, x1, x2;
      int insert = 1 + CODE_SZA;

      SKIP;

      CODE_INSERT (pos, insert);
      CODE_PATCH (p, OP_GF);
      p++;			/* GF x1 */
      p1 = p;
      p += CODE_SZA;
      ASSERT (p == pos + insert);	/* (a) */
      CODE_ADD (OP_GOTO);	/* GOTO x2 */
      p2 = CODE_POS;
      CODE_ADDA (0);
      x1 = CODE_POS;		/* x1: (b) */
      Disjunction_parse (recontext);
      x2 = CODE_POS;		/* x2: */

      CODE_PATCHA (p1, x1);
      CODE_PATCHA (p2, x2);
    }
}

/*
 * Alternative :: [empty]		-- lookahead in ) |
 *		  Term
 *		  Term Alternative
 */

static void
Alternative_parse (recontext)
     struct recontext *recontext;
{

  while (!(ATEOF || NEXT == /*( */ ')' || NEXT == '|'))
    Term_parse (recontext);
}

/*------------------------------------------------------------
 * Term ::
 *	Assertion		la = ^ $ \b \B
 *	Atom
 *	Atom Quantifier
 */

/* We have 24 bytes of lookahead, which is sufficient to
 * scan for {2147483647,2147483647}. Anything larger will
 * overflow the signed int type on 32 bit systems.
 */

static int
Quantifier_is_next (recontext)
     struct recontext *recontext;
{
  int pos, len;
  _unicode_t lookahead[24];

  if (NEXT != '{')
    return 0;

  /*
   * Strict ECMA-262 says that '{' is NOT a Pattern character,
   * but Mozilla allows it 
   */
  /*EXT:24 */ if (!_COMPAT_JS (recontext->interpreter, >=, JS11))
    return 1;

  len = LOOKAHEAD (lookahead, 24);
  pos = 1;

  while (pos < len && lookahead[pos] >= '0' && lookahead[pos] <= '9')
    pos++;

  if (pos < len && lookahead[pos] == ',')
    pos++;
  else if (pos < len && lookahead[pos] == '}')
    return pos > 1;
  else
    return 0;

  while (pos < len && lookahead[pos] >= '0' && lookahead[pos] <= '9')
    pos++;

  if (pos < len && lookahead[pos] == '}')
    pos++;
  else
    return 0;

  return 1;
}

static void
Term_parse (recontext)
     struct recontext *recontext;
{
  int min, max, greedy, pos;
  int oparen, cparen;
  int lookahead_len;
  _unicode_t lookahead[2];

  /*
   * parse Assertion inline since it is a bit special
   * in terms of its lookahead
   */
  switch (NEXT)
    {
    case '\\':
      lookahead_len = LOOKAHEAD (lookahead, 2);
      if (lookahead_len > 1 && lookahead[1] == 'b')
	{
	  SKIP;
	  SKIP;
	  CODE_ADD (OP_BRK);
	  return;
	}
      if (lookahead_len > 1 && lookahead[1] == 'B')
	{
	  SKIP;
	  SKIP;
	  CODE_ADD (OP_NBRK);
	  return;
	}
      /* some other kind of escape */
      break;

    case '^':
      SKIP;
      CODE_ADD (OP_BOL);
      return;

    case '$':
      SKIP;
      CODE_ADD (OP_EOL);
      return;

      /* Lookaheads forbidden by the Atom production */
    case '*':
    case '+':
    case '?':
    case ')':
    case ']':
    case '{':
    case '}':
    case '|':
      SYNTAX_ERROR;
    }

  pos = CODE_POS;
  oparen = recontext->regex->ncaptures;
  Atom_parse (recontext);
  cparen = recontext->regex->ncaptures;

  /*
   * parse Quantifier inline to save my sanity
   */
  if (ATEOF)
    {
      min = max = 1;
    }
  else if (NEXT == '*')
    {
      SKIP;
      min = 0;
      max = INFINITY;
    }
  else if (NEXT == '+')
    {
      SKIP;
      min = 1;
      max = INFINITY;
    }
  else if (NEXT == '?')
    {
      SKIP;
      min = 0;
      max = 1;
    }
  else if (Quantifier_is_next (recontext))
    {
      SKIP;
      min = Integer_parse (recontext);
      if (!ATEOF && NEXT == ',')
	{
	  EXPECT (',');		/*{ */
	  if (!ATEOF && NEXT == '}')
	    max = INFINITY;
	  else
	    max = Integer_parse (recontext);	/*{ */
	}
      else
	max = min;
      EXPECT ('}');
    }
  else
    {
      min = max = 1;
    }
  if (!ATEOF && NEXT == '?')
    {
      SKIP;
      greedy = 0;
    }
  else
    greedy = 1;

  if (min == max && !greedy)
    {
      /*
       * XXX should we warn that the greedy modifiers to
       * 'a{n,n}?' and 'a{n}?' are technically meaningless?
       * We speed up our code by using greedy mode anyway.
       */
      greedy = 1;
    }

  /* Don't allow stupid ranges, such as 'a{7,3}' */
  if (max != INFINITY && min > max)
    SYNTAX_ERROR;

  if (min == 1 && max == 1)	/* a */
    return;

  if (max == 0)
    {				/* a{0} */
      /* Undo! */
      CODE_POS = pos;
      return;
    }

  if (oparen != cparen)
    {
      /*
       * If the atom introduces capture parentheses,
       * then insert code to reset them before each
       * iteration.
       */
      CODE_INSERT (pos, 1 + 2 * CODE_SZI);
      CODE_PATCH (pos, OP_UNDEF);
      CODE_PATCHI (pos + 1, oparen);
      CODE_PATCHI (pos + 1 + CODE_SZI, cparen);
    }

  /*
   * The following code generators all generate looping
   * matchers.  While every corresponding pattern could 
   * be written as the generalised 'a{n,m}' (where m could 
   * be INFINITY), some efficiency is gained by selecting cases
   * where general code that would be a no-op is not emitted.
   */

  if (min == max)
    {				/* a{m} */
      int p = pos, px;
      int insert = 1 + CODE_SZI;
      int c = recontext->regex->ncounters++;

      CODE_INSERT (pos, insert);
      CODE_PATCH (p, OP_ZERO);
      p++;			/* ZERO c; */
      CODE_PATCHI (p, c);
      p += CODE_SZI;
      px = p;			/* x: */
      ASSERT (p == pos + insert);	/*  (a) */
      CODE_ADD (OP_RNEXT);	/* RNEXT c,m,x */
      CODE_ADDI (c);
      CODE_ADDI (max);
      CODE_ADDA (px);
      return;
    }

  if (min == 0 && max == 1)
    {				/* a? */
      int p = pos, p1, px;
      int insert = 1 + CODE_SZA;

      CODE_INSERT (pos, insert);
      CODE_PATCH (p, greedy ? OP_GF : OP_NF);	/* GF x */
      p++;
      p1 = p;
      p += CODE_SZA;
      ASSERT (p == pos + insert);	/* (a) */
      px = CODE_POS;		/* x: */

      CODE_PATCHA (p1, px);
      return;
    }

  if (min == 0 && max == INFINITY)
    {				/* a* */
      int p = pos, px, py, p1;
      int insert = 2 + CODE_SZA + CODE_SZI;
      int m = recontext->regex->nmarks++;

      CODE_INSERT (pos, insert);
      px = p;			/* x: GF y */
      CODE_PATCH (p, greedy ? OP_GF : OP_NF);
      p++;
      p1 = p;
      p += CODE_SZA;
      CODE_PATCH (p, OP_MARK);
      p++;			/* MARK m */
      CODE_PATCHI (p, m);
      p += CODE_SZI;
      ASSERT (p == pos + insert);	/* (a) */
      CODE_ADD (OP_FDIST);	/* FDIST m */
      CODE_ADDI (m);
      CODE_ADD (OP_GOTO);	/* GOTO x */
      CODE_ADDA (px);
      py = CODE_POS;		/* y: */

      CODE_PATCHA (p1, py);

      return;
    }

  {				/* a{n,m} */
    int p = pos, px, py, p1;
    int insert = 3 + CODE_SZI * 2 + CODE_SZA;
    int c = recontext->regex->ncounters++;
    int k = recontext->regex->nmarks++;

    CODE_INSERT (pos, insert);
    CODE_PATCH (p, OP_ZERO);
    p++;			/* ZERO c */
    CODE_PATCHI (p, c);
    p += CODE_SZI;
    px = p;			/* x: GF y */
    CODE_PATCH (p, greedy ? OP_GF : OP_NF);
    p++;
    p1 = p;
    p += CODE_SZA;
    CODE_PATCH (p, OP_MARK);
    p++;			/* MARK k */
    CODE_PATCHI (p, k);
    p += CODE_SZI;
    ASSERT (p == pos + insert);	/* (a) */

    if (min)
      {
	CODE_ADD (OP_RDIST);	/* RDIST k,c,n */
	CODE_ADDI (k);
	CODE_ADDI (c);
	CODE_ADDI (min);
      }
    else
      {
	CODE_ADD (OP_FDIST);	/* FDIST k */
	CODE_ADDI (k);
      }
    if (max != INFINITY)
      {
	CODE_ADD (OP_RNEXT);	/* RNEXT c,m,x */
	CODE_ADDI (c);
	CODE_ADDI (max);
	CODE_ADDA (px);
      }
    else
      {
	CODE_ADD (OP_MNEXT);	/* MNEXT c,n,x */
	CODE_ADDI (c);
	CODE_ADDI (min);
	CODE_ADDA (px);
      }
    py = CODE_POS;		/* y: */
    if (min)
      {
	CODE_ADD (OP_REACH);	/* REACH c,n */
	CODE_ADDI (c);
	CODE_ADDI (min);
      }

    CODE_PATCHA (p1, py);
    return;
  }
}

/* Parse a simple integer. Used for repetition ranges */
static int
Integer_parse (recontext)
     struct recontext *recontext;
{
  int val;
  int hasdig = 0;

  val = 0;
  while (!ATEOF && NEXT >= '0' && NEXT <= '9')
    {
      val = 10 * val + (NEXT - '0');
      hasdig = 1;
      SKIP;
    }
  if (!hasdig)
    SYNTAX_ERROR;
  return val;
}

/*
 * Atom::			la != ^ $     * + ?   )   ] { } |
 *	pattern character	la != ^ $ \ . * + ? ( ) [ ] { } |
 *	.
 *	\ AtomEscape
 *	[ CharacterClass ]
 *	( Disjunction )
 *	( ?: Disjunction )
 *	( ?= Disjunction )
 *	( ?! Disjunction )
 */

static void
Atom_parse (recontext)
     struct recontext *recontext;
{
  struct charclass *c;
  int i;

  if (NEXT == '(')
    {
      SKIP;
      if (!ATEOF && NEXT == '?')
	{			/* (?... */
	  SKIP;
	  if (!ATEOF && NEXT == ':')
	    {			/* (?:... */
	      SKIP;
	      Disjunction_parse (recontext);
	    }
	  else if (!ATEOF && (NEXT == '=' || NEXT == '!'))
	    {
	      int px, p1;	/* (?=... */
	      int neg = (NEXT == '!');	/* (?!... */

	      SKIP;
	      CODE_ADD (neg ? OP_AF : OP_AS);	/* AS x */
	      p1 = CODE_POS;
	      CODE_ADDI (0);
	      Disjunction_parse (recontext);	/* (a) */
	      CODE_ADD (OP_SUCCEED);	/* SUCCEED */
	      px = CODE_POS;	/* x: */

	      CODE_PATCHA (p1, px);
	    }
	  else
	    SYNTAX_ERROR;
	}
      else
	{			/* (...) */
	  i = recontext->regex->ncaptures++;
	  CODE_ADD (OP_START);	/* START i */
	  CODE_ADDI (i);
	  Disjunction_parse (recontext);	/* (a) */
	  CODE_ADD (OP_END);	/* END i */
	  CODE_ADDI (i);
	}
      EXPECT (')');
      return;
    }

  /*
   * All other atoms compile to simple character class matches
   * (or backreferences)
   */

  switch (NEXT)
    {
    case '\\':
      SKIP;
      if (ATEOF)
	SYNTAX_ERROR;
      if (NEXT >= '1' && NEXT <= '9')
	{
	  i = 0;
	  while (!ATEOF && (NEXT >= '0' && NEXT <= '9'))
	    {
	      i = 10 * i + NEXT - '0';
	      SKIP;
	    }
	  CODE_ADD (OP_BACKREF);
	  CODE_ADDI (i);
	  if (i > recontext->regex->maxref)
	    recontext->regex->maxref = i;
	  return;
	}
      c = ClassEscape_parse (recontext);
      break;
    case '[':
      c = CharacterClass_parse (recontext);
      break;
    case '.':
      SKIP;
      c = CC_NEW ();
      CC_ADDCHAR (c, 0x000a);
      CC_ADDCHAR (c, 0x000d);
      CC_ADDCHAR (c, 0x2028);
      CC_ADDCHAR (c, 0x2029);
      CC_INVERT (c);
      break;
    default:
      c = CC_NEW ();
      CC_ADDCHAR (c, Canonicalize (recontext->regex, NEXT));
      SKIP;
      break;
    }

  i = CC_INTERN (c);
  CODE_ADD (OP_CHAR);
  CODE_ADDI (i);

}

static unsigned char
HexDigit_parse (recontext)
     struct recontext *recontext;
{
  _unicode_t c;

  if (ATEOF)
    SYNTAX_ERROR;
  c = NEXT;
  SKIP;
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  SYNTAX_ERROR;
  return 0;
}

static struct charclass *
ClassEscape_parse (recontext)
     struct recontext *recontext;
{
  struct charclass *c;
  _unicode_t ch, lookahead[3];
  int i;

  /* EXPECT('\\'); *//* backslash already skipped */

  c = CC_NEW ();

  if (NEXT >= '0' && NEXT <= '9')
    {

      /* \0oo - 3 digit octal escapes */
/*EXT:25*/ if (_COMPAT_JS (recontext->interpreter, >=, JS11) &&
	       NEXT == '0' && LOOKAHEAD (lookahead, 3) >= 2 &&
	       lookahead[1] >= '0' && lookahead[1] < '8' &&
	       lookahead[2] >= '0' && lookahead[2] < '8')
	{
	  ch = (lookahead[1] - '0') * 8 + (lookahead[2] - '0');
	  CC_ADDCHAR (c, ch);
	  SKIP;
	  SKIP;
	  SKIP;
	  return c;
	}

      i = 0;
      while (!ATEOF && NEXT >= '0' && NEXT <= '9')
	{
	  i = 10 * i + NEXT - '0';
	  SKIP;
	}

      /*
       * 15.10.2.1.9: "Using a backreference inside a ClassAtom
       * causes an error"
       */
      if (i != 0)
	SYNTAX_ERROR;

      CC_ADDCHAR (c, i);
      return c;
    }

  ch = NEXT;
  SKIP;

  switch (ch)
    {
    case 'b':
      CC_ADDCHAR (c, 0x0008);
      break;
    case 't':
      CC_ADDCHAR (c, 0x0009);
      break;
    case 'n':
      CC_ADDCHAR (c, 0x000a);
      break;
    case 'v':
      CC_ADDCHAR (c, 0x000b);
      break;
    case 'f':
      CC_ADDCHAR (c, 0x000c);
      break;
    case 'r':
      CC_ADDCHAR (c, 0x000d);
      break;
    case 'D':
    case 'd':
      CC_ADDRANGE (c, '0', '9');
      if (ch == 'D')
	CC_INVERT (c);
      break;
    case 'W':
    case 'w':
      CC_ADDRANGE (c, 'a', 'z');
      CC_ADDRANGE (c, 'A', 'Z');
      CC_ADDRANGE (c, '0', '9');
      CC_ADDCHAR (c, '_');
      if (ch == 'W')
	CC_INVERT (c);
      break;
    case 'S':
    case 's':
#if WITH_UNICODE_TABLES
      for (i = 0; i < SEE_unicode_Zscodeslen; i++)
	CC_ADDCHAR (c, SEE_unicode_Zscodes[i]);
#else
      CC_ADDCHAR (c, 0x0020);
#endif
      if (ch == 'S')
	CC_INVERT (c);
      break;
    case 'c':
      if (ATEOF)
	SYNTAX_ERROR;
      ch = NEXT;
      SKIP;
      if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
	CC_ADDCHAR (c, ch % 32);
      else
	SYNTAX_ERROR;
      break;
    case 'u':
    case 'x':
      i = (ch == 'x' ? 2 : 4);
      ch = 0;
      while (i--)
	ch = (ch << 4) | HexDigit_parse (recontext);
      CC_ADDCHAR (c, ch);
      break;
    default:
      CC_ADDCHAR (c, ch);
      break;
    }

  return c;
}

/*
 * ClassAtom :
 *	'\' ClassEscape
 *	anychar
 */
static struct charclass *
ClassAtom_parse (recontext)
     struct recontext *recontext;
{
  struct charclass *c;

  if (ATEOF)
    SYNTAX_ERROR;
  if (NEXT == '\\')
    {
      SKIP;
      return ClassEscape_parse (recontext);
    }

  c = CC_NEW ();
  CC_ADDCHAR (c, NEXT);
  SKIP;
  return c;
}

/*
 * Convert the charclass into a canonicalised version. (SLOW!)
 * !!!Every character in the class has to be individually canonicalized!!!
 * !!!OUCH!!!OUCH!!!I!HATE!YOU!ALL!!!
 */
static struct charclass *
CanonicalizeClass (recontext, c)
     struct recontext *recontext;
     struct charclass *c;
{
  struct charclass *ccanon;
  struct charclassrange *r;
  _unicode_t ch, uch;

  if (cc_count (c) > (unsigned int) ~0 / 2)
    {
      CC_INVERT (c);
      ccanon = CanonicalizeClass (recontext, c);
      CC_INVERT (ccanon);
      return ccanon;
    }

  /*
   * Evil hack: if the charclass range includes ['A'-0xEFFFF], then
   * there is no need to canonicalize because every uppercase character
   * is already there.
   */
  for (r = c->ranges; r; r = r->next)
    if (r->lo <= 'A' && r->hi > 0xf0000)
      return c;

  ccanon = CC_NEW ();
  for (r = c->ranges; r; r = r->next)
    for (ch = r->lo; ch < r->hi; ch++)
      {
	uch = UNICODE_TOUPPER (ch);
	CC_ADDCHAR (ccanon, uch);
      }
  return ccanon;
}

/*
 * CharacterClass ::
 *	'[' [^] ( ClassAtom | ClassAtom '-' ClassAtom ) * ']'
 */
static struct charclass *
CharacterClass_parse (recontext)
     struct recontext *recontext;
{
  struct charclass *c = CC_NEW (), *a, *b;
  int invertflag = 0;

  EXPECT ('[');
  if (!ATEOF && NEXT == '^')
    {
      invertflag = 1;
      SKIP;
    }

  while (!ATEOF && NEXT != ']')
    {
      a = ClassAtom_parse (recontext);
      if (!ATEOF && NEXT == '-')
	{
	  SKIP;
	  /* Treat '-' literally if at end of ClassRanges 15.10.2.16 */
	  if (!ATEOF && NEXT == ']')
	    {
	      CC_ADDCHAR (a, '-');
	      goto out;
	    }
	  if (!cc_issingle (a))
	    SYNTAX_ERROR;
	  b = ClassAtom_parse (recontext);
	  if (!cc_issingle (b))
	    SYNTAX_ERROR;
	  if (b->ranges->lo < a->ranges->lo)
	    SYNTAX_ERROR;
	  a->ranges->hi = b->ranges->hi;
	  _FREE1(recontext, b);
	}
    out:CC_ADDCC (c, a);
      /* free(a) */
    }
  EXPECT (']');

  if (recontext->regex->flags & FLAG_IGNORECASE)
    {
      c = CanonicalizeClass (recontext, c);
      _FREE1(recontext, c);      /* free(c) -- i.e the old c */
    }

  if (invertflag)
    CC_INVERT (c);
  return c;
}

/*------------------------------------------------------------
 * P-code
 */

#ifndef NDEBUG
static int
dprint_code (regex, addr)
     struct ecma_regex *regex;
     int addr;
{
  int i;
  const char *op = "", *opc;
  unsigned char *code = regex->code;

  DPRINTF (("0x%04x: ", addr));
  switch (code[addr])
    {
    case OP_FAIL:
      DPRINTF (("FAIL"));
      op = "";
      break;
    case OP_SUCCEED:
      DPRINTF (("SUCCEED"));
      op = "";
      break;
    case OP_CHAR:
      DPRINTF (("CHAR"));
      op = "i";
      break;
    case OP_ZERO:
      DPRINTF (("ZERO"));
      op = "i";
      break;
    case OP_REACH:
      DPRINTF (("REACH"));
      op = "ii";
      break;
    case OP_NREACH:
      DPRINTF (("NREACH"));
      op = "ii";
      break;
    case OP_START:
      DPRINTF (("START"));
      op = "i";
      break;
    case OP_END:
      DPRINTF (("END"));
      op = "i";
      break;
    case OP_UNDEF:
      DPRINTF (("UNDEF"));
      op = "ii";
      break;
    case OP_MARK:
      DPRINTF (("MARK"));
      op = "i";
      break;
    case OP_FDIST:
      DPRINTF (("FDIST"));
      op = "i";
      break;
    case OP_RDIST:
      DPRINTF (("RDIST"));
      op = "iii";
      break;
    case OP_MNEXT:
      DPRINTF (("MNEXT"));
      op = "iia";
      break;
    case OP_RNEXT:
      DPRINTF (("RNEXT"));
      op = "iia";
      break;
    case OP_GOTO:
      DPRINTF (("GOTO"));
      op = "a";
      break;
    case OP_GS:
      DPRINTF (("GS"));
      op = "a";
      break;
    case OP_NS:
      DPRINTF (("NS"));
      op = "a";
      break;
    case OP_GF:
      DPRINTF (("GF"));
      op = "a";
      break;
    case OP_NF:
      DPRINTF (("NF"));
      op = "a";
      break;
    case OP_AS:
      DPRINTF (("AS"));
      op = "a";
      break;
    case OP_AF:
      DPRINTF (("AF"));
      op = "a";
      break;
    case OP_BOL:
      DPRINTF (("BOL"));
      op = "";
      break;
    case OP_EOL:
      DPRINTF (("EOL"));
      op = "";
      break;
    case OP_BRK:
      DPRINTF (("BRK"));
      op = "";
      break;
    case OP_NBRK:
      DPRINTF (("NBRK"));
      op = "";
      break;
    case OP_BACKREF:
      DPRINTF (("BACKREF"));
      op = "i";
      break;

    default:
      DPRINTF (("*** %d", code[addr]));
    }
  addr++;
  for (opc = op; *opc; opc++)
    {
      if (opc != op)
	DPRINTF ((","));
      DPRINTF ((" "));
      switch (*opc)
	{
	case 'a':
	  i = CODE_MAKEA (code, addr);
	  DPRINTF (("0x%04x", i));
	  i = CODE_MAKEI (code, addr);
	  DPRINTF ((" [0x%04x]", i));
	  addr += CODE_SZA;
	  break;
	case 'i':
	  i = CODE_MAKEI (code, addr);
	  addr += CODE_SZI;
	  DPRINTF (("%d", i));
	  break;
	case 'c':
	  i = CODE_MAKEI (code, addr);
	  addr += CODE_SZI;
	  DPRINTF (("%d=", i));
	  if (i > regex->cclen)
	    DPRINTF (("**BAD**"));
	  else
	    dprint_cc (regex->cc[i]);
	  break;
	}
    }
  DPRINTF (("\n"));
  return addr;
}
#endif

#ifndef NDEBUG
static void
dprint_regex (regex)
     struct ecma_regex *regex;
{
  int i, addr;
  struct charclassrange *r;

  DPRINTF (("regex %p\n", regex));
  DPRINTF (("\tncaptures = %d\n", regex->ncaptures));
  DPRINTF (("\tcodelen = %d\n", regex->codelen));
  DPRINTF (("\tcclen = %d\n", regex->cclen));
  DPRINTF (("\tflags = 0x%x\n", regex->flags));
  DPRINTF (("\tcc:\n"));
  for (i = 0; i < regex->cclen; i++)
    {
      DPRINTF (("\t\t%d = ", i));
      dprint_cc (regex->cc[i]);
      DPRINTF (("\n\t\t  = { "));
      for (r = regex->cc[i]->ranges; r; r = r->next)
	DPRINTF (("%x:%x ", r->lo, r->hi));
      DPRINTF (("}\n"));
    }
  DPRINTF (("\tcode:\n"));

  addr = 0;
  while (addr < regex->codelen)
    addr = dprint_code (regex, addr);
}
#endif

/*------------------------------------------------------------
 * pcode-execution
 */

/* 15.10.2.8 */
static _unicode_t
Canonicalize (regex, ch)
     struct ecma_regex *regex;
     _unicode_t ch;
{
  if (regex->flags & FLAG_IGNORECASE)
    return UNICODE_TOUPPER (ch);
  else
    return ch;
}

static int
pcode_run (interp, regex, addr, text, state)
     struct execcontext *interp;
     struct ecma_regex *regex;
     unsigned int addr;
     struct _string *text;
     char *state;
{
  int result;
  int i = 0, i2 = 0, i3 = 0;
  unsigned int a = 0;
  unsigned char op;
  _unicode_t ch;
  struct capture *capture;
  int *counter, *mark, statesz;
  unsigned int newaddr;
  char *newstate;

  /*      if (SEE_system.periodic)
     /        (*SEE_system.periodic)(interp); */

  /* Compute the offsets into the state structure */
  statesz = 0;
  capture = (struct capture *) state;
  statesz += regex->ncaptures * sizeof (struct capture);
  counter = (int *) (state + statesz);
  statesz += regex->ncounters * sizeof (int);
  mark = (int *) (state + statesz);
  statesz += regex->nmarks * sizeof (int);

  _ASSERT (interp, statesz == regex->statesz);

  newstate = _STRING_ALLOCA (interp, char, statesz);

#define index (capture[0].cap_end)

  for (;;)
    {

      /* Catch bad branches */
      if (addr >= regex->codelen)
	{
	  internalerror (interp);
	}

      /* Read the opcode and its arguments */
      op = regex->code[addr];

#ifndef NDEBUG
      if (_regex_debug)
	{
	  int x;
	  struct _string mys;

	  mys.stringclass = 0;
	  mys.interpreter = 0;
	  mys.flags = 0;
	  DPRINTF (("index=%d captures=[", index));
	  for (x = 0; x < regex->ncaptures; x++)
	    {
	      if (x)
		DPRINTF ((","));
	      if (capture[x].cap_start == -1)
		DPRINTF (("undef"));
	      else if (capture[x].cap_start + capture[x].cap_end >
		       text->length)
		{
		  DPRINTF (("bad<%x:%x>",
			    capture[x].cap_start, capture[x].cap_end));
		}
	      else
		{
		  int end = capture[x].cap_end;
		  if (end == -1)
		    end = index;
		  mys.length = end - capture[x].cap_start;
		  mys.data = text->data + capture[x].cap_start;
		  DPRINTF (("\""));
		  /*                      dprints(&mys); */
		  DPRINTF (("\""));
		  if (capture[x].cap_end == -1)
		    DPRINTF (("+"));
		}
	    }
	  DPRINTF (("]"));
	  if (op == OP_ZERO || op == OP_REACH || op == OP_NREACH ||
	      op == OP_MNEXT || op == OP_RNEXT)
	    {
	      DPRINTF ((" counter=["));
	      for (x = 0; x < regex->ncounters; x++)
		{
		  if (x)
		    DPRINTF ((","));
		  DPRINTF (("%d", counter[x]));
		}
	      DPRINTF (("]"));
	    }
	  if (op == OP_MARK || op == OP_FDIST || op == OP_RDIST)
	    {
	      DPRINTF ((" mark=["));
	      for (x = 0; x < regex->nmarks; x++)
		{
		  if (x)
		    DPRINTF ((","));
		  DPRINTF (("%d", mark[x]));
		}
	      DPRINTF (("]"));
	    }
	  if (regex->code[addr] == OP_CHAR && index < text->length)
	    DPRINTF ((" ch='%c'", Canonicalize (regex, text->data[index])));
	  DPRINTF (("\n"));
	  (void) dprint_code (regex, addr);
	}
#endif

      addr++;

      switch (op)
	{
	case OP_FAIL:
	case OP_SUCCEED:
	case OP_BOL:
	case OP_EOL:
	case OP_BRK:
	case OP_NBRK:
	  break;
	case OP_CHAR:
	case OP_ZERO:
	case OP_START:
	case OP_END:
	case OP_MARK:
	case OP_FDIST:
	case OP_BACKREF:
	  i = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  break;
	case OP_REACH:
	case OP_NREACH:
	case OP_UNDEF:
	  i = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  i2 = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  break;
	case OP_RDIST:
	  i = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  i2 = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  i3 = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  break;
	case OP_MNEXT:
	case OP_RNEXT:
	  i = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  i2 = CODE_MAKEI (regex->code, addr);	/* integer arg */
	  addr += CODE_SZI;
	  a = CODE_MAKEA (regex->code, addr);	/* address arg */
	  addr += CODE_SZA;
	  break;
	case OP_GOTO:
	case OP_GS:
	case OP_NS:
	case OP_GF:
	case OP_NF:
	case OP_AS:
	case OP_AF:
	  a = CODE_MAKEA (regex->code, addr);	/* address arg */
	  addr += CODE_SZA;
	  break;
	default:
	  /* error */
	  internalerror (interp);
	}

      switch (op)
	{

	case OP_FAIL:
	  return 0;
	case OP_SUCCEED:
	  return 1;

	  /* succeed if current character matches charclass. index++ */
	case OP_CHAR:
	  if (index < text->length)
	    {
	      ch = text->data[index++];
	      /* N.B. strings are UTF-16 encoded! */
	      if ((ch & 0xfc00) == 0xd800 &&
		  index < text->length &&
		  (text->data[index] & 0xfc00) == 0xdc00)
		ch = (((ch & 0x3ff) << 10) |
		      (text->data[index++] & 0x3ff)) + 0x10000;
	      ch = Canonicalize (regex, ch);
	      if (!cc_contains (regex->cc[i], ch))
		return 0;
	    }
	  else
	    return 0;
	  break;

	  /* reset an iteration counter */
	case OP_ZERO:
	  counter[i] = 0;
	  break;

	  /* fail if we havent reached a particular count */
	case OP_REACH:
	  if (counter[i] < i2)
	    return 0;
	  break;

	  /* fail if we reached a particular count */
	case OP_NREACH:
	  if (counter[i] >= i2)
	    return 0;
	  break;

	  /* start a capture group at current index */
	case OP_START:
	  capture[i].cap_start = index;
	  capture[i].cap_end = -1;
	  break;

	  /* finish a capture group at current index */
	case OP_END:
	  capture[i].cap_end = index;
	  break;

	  /* reset the given captures - usually done at a loop start */
	case OP_UNDEF:
	  while (i < i2)
	    {
	      capture[i].cap_start = -1;
	      capture[i].cap_end = -1;
	      i++;
	    }
	  break;

	  /* Set a mark to the current index */
	case OP_MARK:
	  mark[i] = index;
	  break;

	  /* fail if we haven't advanced past the mark */
	case OP_FDIST:
	  if (mark[i] == index)
	    return 0;
	  break;

	  /* fail if haven't advanced past mark AND counter has reached 
	   * a limit */
	case OP_RDIST:
	  if (mark[i] == index && counter[i2] >= i3)
	    return 0;
	  break;

	  /* increment counter if it is less than n. always branch */
	case OP_MNEXT:
	  if (counter[i] < i2)
	    counter[i]++;
	  addr = a;
	  break;

	  /* increment counter. if it is less than n, then branch */
	case OP_RNEXT:
	  counter[i]++;
	  if (counter[i] < i2)
	    addr = a;
	  break;

	case OP_GOTO:
	  addr = a;
	  break;

	  /* operations that push state and branch for backtracking */
	case OP_GS:		/* greedy success */
	case OP_NS:		/* non-greedy success */
	case OP_GF:		/* greedy fail */
	case OP_NF:		/* greedy fail */
	case OP_AS:		/* assert success */
	case OP_AF:		/* assert fail */
	  newaddr = (op == OP_NS || op == OP_NF) ? a : addr;
	  memcpy (newstate, state, statesz);
	  result = pcode_run (interp, regex, newaddr, text, newstate);
	  if (result)
	    {
	      if (op == OP_GF || op == OP_NF)
		{
		  memcpy (state, newstate, statesz);
		  return 1;
		}
	      else if (op == OP_AF)
		return 0;
	      else if (op == OP_AS)
		{
		  int index_save = index;
		  memcpy (state, newstate, statesz);
		  index = index_save;
		  addr = a;
		}
	      else if (op == OP_GS)
		addr = a;
	    }
	  else
	    {
	      if (op == OP_GS || op == OP_NS || op == OP_AS)
		return 0;
	      else if (op == OP_GF || op == OP_AF)
		addr = a;
	    }
	  break;

	  /* succeed if we are at the beginning of a line */
	  /* See 15.10.2.6 */
	case OP_BOL:
	  if (index == 0)	/* ^ */
	    ;			/* succeed */
	  else if ((regex->flags & FLAG_MULTILINE) == 0)
	    return 0;
	  else if (text->data[index - 1] == 0x000a /*LF*/
		   || text->data[index - 1] == 0x000d /*CR*/
		   || text->data[index - 1] == 0x2028 /*LS*/
		   || text->data[index - 1] == 0x2029)
	     /*PS*/;		/* succeed */
	  else
	    return 0;
	  break;

	  /* succeed if we are at the end of a line */
	case OP_EOL:
	  if (index == text->length)	/* $ */
	    ;			/* succeed */
	  else if ((regex->flags & FLAG_MULTILINE) == 0)
	    return 0;
	  else if (text->data[index] == 0x000a /*LF*/
		   || text->data[index] == 0x000d /*CR*/
		   || text->data[index] == 0x2028 /*LS*/
		   || text->data[index] == 0x2029)
	     /*PS*/;		/* succeed */
	  else
	    return 0;
	  break;

#define IsWordChar(e)	((e) >= 0 && (e) < text->length && (		  \
			   (text->data[e] >= 'a' && text->data[e] <= 'z') \
			|| (text->data[e] >= 'A' && text->data[e] <= 'Z') \
			|| (text->data[e] >= '0' && text->data[e] <= '9') \
			|| text->data[e] == '_'))

	  /* succeed if we are at a word break */
	case OP_BRK:
	case OP_NBRK:
	  {
	    int a = IsWordChar (index - 1);
	    int b = IsWordChar (index);
	    if (op == OP_BRK)
	      {
		if (a == b)
		  return 0;
	      }
	    else
	      {
		if (a != b)
		  return 0;
	      }
	    break;
	  }

	  /* succeed if we match a backreference */
	case OP_BACKREF:
	  if (!CAPTURE_IS_UNDEFINED (capture[i]))
	    {
	      int br = capture[i].cap_start;
	      int len = capture[i].cap_end - br;
	      if (len + index > text->length) {
			return 0;
		  }
	      for (int x = 0; x < len; x++) {
			if (Canonicalize (regex, text->data[br + x]) != Canonicalize (regex, text->data[index + x])) {
			  return 0;
			}
		  }
	      index += len;
	    }
	  break;

	  /* catch unexpected instructions */
	default:
	  internalerror (interp);
	}
    }
}

#undef index

/*
 * Executes the regex on the text beginning at index.
 * Returns true of a match was successful.
 */
int
_ecma_regex_match (interp, aregex, text, index, capture_ret)
     struct execcontext *interp;
     struct uregex *aregex;
     struct _string *text;
     unsigned int index;
     struct capture *capture_ret;
{
  struct ecma_regex *regex = REGEX_CAST (aregex);
  int i, success;
  char *state = _STRING_ALLOCA (interp, char, regex->statesz);
  struct capture *capture = (struct capture *) state;

#ifndef NDEBUG
  memset (state, 0xd0, regex->statesz);	/* catch bugs */
#endif

  capture[0].cap_start = index;
  capture[0].cap_end = index;
  for (i=1; i<regex->ncaptures; i++)
    {
      capture[i].cap_start = -1;
      capture[i].cap_end = -1;
    }
  success = pcode_run (interp, regex, 0, text, state);
#ifndef NDEBUG
  if (_regex_debug)
    {
      DPRINTF ((". %s\n", success ? "success" : "failure"));
    }
#endif
  if (success)
    memcpy (capture_ret, capture, regex->ncaptures * sizeof (struct capture));

  _STRING_ALLOCAFREE(interp, state);
  return success;
}

int
ecma_regex_match (interp, aregex, source, len, index, capture_ret)
     struct execcontext *interp;
     struct uregex *aregex;
     unsigned short *source;
     int len;
     unsigned int index;
     struct capture *capture_ret;
{
  struct _string s;
  s.stringclass = 0;
  s.interpreter = 0;
  s.flags = 0;
  s.data = source;
  s.length = len;
  return _ecma_regex_match (interp, aregex, &s, index, capture_ret);
}

/*------------------------------------------------------------
 * optimizer
 */

static void
optimize_regex (interp, regex)
     struct execcontext *interp;
     struct ecma_regex *regex;
{
  /*
   * (nothing here yet)
   *
   * possible optimisations include branch short-cuts,
   * and compiling the p-code to native machine instructions.
   */
}


/*
main()
{
  struct ecma_regex *p = ecma_regex_compile(0, "^ba", 0);
  void *q = alloca(p->statesz);
  int i = ecma_regex_match(0, p, "abac", 1, q);

  printf("%d\n", i);

  }*/

/* POSIX compatibility.  */




extern int
regcomp_u (regex_t * preg, const unsigned short *pattern, int len, int cflags)
{
  preg->pat =
    (struct ecma_regex *) ecma_regex_compile (preg->interp, pattern, len, cflags);
  return 0;
}

extern int
regexec_u (const regex_t * preg,
	   const unsigned short *string,
	   int len, size_t nmatch, regmatch_t pmatch[], int eflags)
{
  void *q = _STRING_ALLOCA (preg->interp, char, preg->pat->statesz);
  int i;
  int ret;
  int n;
  for (i=0; i<len; i++)
    {
      ret = ecma_regex_match (preg->interp, preg->pat, string, len, i, q);
      if (ret == 1)
	goto MATCH;
    }
  _STRING_ALLOCAFREE(preg->interp, q);
  return REG_NOMATCH;
MATCH:
#define MIN(a,b) ((a<b)?a:b)
  n = MIN (nmatch, preg->pat->ncaptures);
  memcpy (pmatch, q, sizeof (regmatch_t) * n);

  _STRING_ALLOCAFREE(preg->interp, q);
  return 0;
}

extern size_t
regerror_u (int errcode, const regex_t * preg,
	    char *errbuf, size_t errbuf_size)
{
  return 0;
}

extern void
regfree_u (regex_t * preg)
{
}

void reg_init(
	void* (*localmalloc)(void* c, unsigned int size),
	void (*localfree)(void* c, void* p)
	)
{
  _localmalloc = localmalloc;
  _localfree = localfree;
}
