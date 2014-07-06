#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pstate.h"
#include "error.h"
#include "value.h"
#include "proto.h"
#include "regexp.h"
#include "pstate.h"


#define MAX_SUBREGEX	256

/* substr */
static int
strpto_substr (PSTATE * ps, Value * args, Value * _this, Value * ret, int asc)
{
  int ilen;
  unichar *v;
  Value *start;
  Value *len;
  int istart;
  if (asc)
    die ("Execute String.prototype.substr as constructor\n");
  if (_this->vt != VT_OBJECT || _this->d.obj->ot != OT_STRING)
    {
      die ("apply String.prototype.substr to a non-string object\n");
    }
  v = _this->d.obj->d.str;
  start = value_object_lookup_array (args, 0, NULL);
  len = value_object_lookup_array (args, 1, NULL);
  if (!start || !is_number (start))
    {
      value_make_string (*ret, unistrdup (ps, v));
      return 0;
    }
  istart = (int) start->d.num;
  if (!len || !is_number (len))
    {
      value_make_string (*ret, unisubstrdup (ps, v, istart, -1));
      return 0;
    }
  ilen = (int) len->d.num;
  if (ilen <= 0)
    {
      value_make_string (*ret, unistrdup_str (ps, ""));
    }
  else
    {
      value_make_string (*ret, unisubstrdup (ps, v, istart, ilen));
    }
  return 0;
}


/* indexOf */
/* todo regex */
static int
strpto_indexOf (PSTATE * ps, Value * args, Value * _this, Value * ret,
		int asc)
{
  int r;
  unichar *v;
  Value *seq;
  Value *start;
  unichar *vseq;
  int istart = 0;
  if (asc)
    die ("Execute String.prototype.indexOf as constructor\n");
  if (_this->vt != VT_OBJECT || _this->d.obj->ot != OT_STRING)
    {
      die ("apply String.prototype.indexOf to a non-string object\n");
    }
  v = _this->d.obj->d.str;
  seq = value_object_lookup_array (args, 0, NULL);
  start = value_object_lookup_array (args, 1, NULL);
  if (!seq)
    {
      value_make_number (*ret, -1);
      return 0;
    }
  value_tostring (ps, seq);
  vseq = seq->d.str;
  if (start && is_number (start))
    {
      istart = (int) start->d.num;
      if (istart < 0)
	istart = 0;
    }
  r = unistrpos (v, istart, vseq);
  value_make_number (*ret, r);
  return 0;
}

static
UNISTR (5)
  INDEX =
{
  5,
  {
'i', 'n', 'd', 'e', 'x'}};

static
UNISTR (5)
  INPUT =
{
  5,
  {
'i', 'n', 'p', 'u', 't'}};


/* match */
static int
strpto_match (PSTATE * ps, Value * args, Value * _this, Value * ret, int asc)
{
  int i;
  regex_t *reg;
  Object *obj;
  unichar *v;
  Value *seq;
  Value *vind;
  int r;
  regmatch_t pos[MAX_SUBREGEX];
  Value *vinput;
  if (asc)
    die ("Execute String.prototype.match as constructor\n");
  if (_this->vt != VT_OBJECT || _this->d.obj->ot != OT_STRING)
    {
      die ("apply String.prototype.match to a non-string object\n");
    }
  v = _this->d.obj->d.str;
  seq = value_object_lookup_array (args, 0, NULL);
  if (!seq || seq->vt != VT_OBJECT || seq->d.obj->ot != OT_REGEXP)
    {
      value_make_null (*ret);
      return 0;
    }
  reg = seq->d.obj->d.robj;
  memset (&pos, 0, MAX_SUBREGEX * sizeof (regmatch_t));

#if USE_UREGEX
  if ((r = regexec_u (reg, v, unistrlen (v), MAX_SUBREGEX, pos, 0)) != 0)
    {
      if (r == REG_NOMATCH)
	{
	  value_make_null (*ret);
	  return 0;
	}
      else
	die ("Out of memory\n");
    }

#else /*  */
  if ((r = regexec (reg, tochars (v), MAX_SUBREGEX, pos, 0)) != 0)
    {
      if (r == REG_NOMATCH)
	{
	  value_make_null (*ret);
	  return 0;
	}
      else
	die ("Out of memory\n");
    }

#endif /*  */
  obj = object_new (ps);
  obj->__proto__ = Array_prototype;
  value_make_object (*ret, obj);
  object_set_length (ps, ret->d.obj, 0);
  for (i = 0; i < MAX_SUBREGEX; ++i)
    {
      Value *val;
      if (pos[i].rm_so <= 0 && pos[i].rm_eo <= 0)
	break;
      val = value_new (ps);
      value_make_string (*val,
			 unisubstrdup (ps, v, pos[i].rm_so,
				       pos[i].rm_eo - pos[i].rm_so));
      value_object_utils_insert_array (ps, ret, i, val, 1, 1, 1);
    }
  vind = value_new (ps);
  value_make_number (*vind, pos[0].rm_so);
  value_object_utils_insert (ps, ret, INDEX.unistr, vind, 1, 1, 1);
  vinput = value_new (ps);
  value_make_string (*vinput, unistrdup (ps, v));
  value_object_utils_insert (ps, ret, INPUT.unistr, vinput, 1, 1, 1);
  return 0;
}


/* charCodeAt */
static int
strpto_charCodeAt (PSTATE * ps, Value * args, Value * _this, Value * ret,
		   int asc)
{
  int slen;
  int pos;
  Value *vpos;
  Value target = {
    0
  };
  if (asc)
    die ("Execute String.prototype.charCodeAt as constructor\n");
  value_copy (target, *_this);
  value_tostring (ps, &target);
  slen = unistrlen (target.d.str);
  pos = 0;
  if ((vpos = value_object_lookup_array (args, 0, NULL)))
    {
      value_toint32 (ps, vpos);
      pos = (int) vpos->d.num;
    }
  if (pos < 0 || pos >= slen)
    {
      value_make_number (*ret, ieee_makenan ());
    }
  else
    {
      value_make_number (*ret, target.d.str[pos]);
    }
  value_erase (target);
  return 0;
}

static struct st_strpro_tab
{
  const char *name;
  SSFunc func;
} strpro_funcs[] =
{

  {
  "substr", strpto_substr},
  {
  "indexOf", strpto_indexOf},
  {
  "match", strpto_match},
  {
"charCodeAt", strpto_charCodeAt}};

void
proto_string_init (PSTATE * ps, Value * global)
{
  int i;
  if (!String_prototype)
    bug ("proto init failed?");
  for (i = 0; i < sizeof (strpro_funcs) / sizeof (struct st_strpro_tab); ++i)
    {
      Value *n = func_utils_make_func_value (ps, strpro_funcs[i].func);
      n->d.obj->__proto__ = Function_prototype;
      value_object_utils_insert (ps, String_prototype,
				 tounichars (ps, strpro_funcs[i].name), n, 0,
				 0, 0);
}}
