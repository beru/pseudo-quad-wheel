#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"
#include "error.h"
#include "proto.h"
#include "mempool.h"
#include "pstate.h"

static Mempool* object_pool;
static Mempool* value_pool;

// length unichar constant
static const
UNISTR (6)
  LENGTH =
{
  6,
  {'l', 'e', 'n', 'g', 't', 'h'}
};

ObjKey*
objkey_new (void* ps, const unichar* strkey, int flag)
{
	void* p = mm_alloc (ps, unistrlen (strkey) * sizeof (unichar) + sizeof (int) * 2);
	ObjKey* ok = (ObjKey*) (((int) p) + sizeof (int) * 2);

	unistrcpy ((unichar*) ok, strkey);
	KEYFLAG (ok) = flag;

	return ok;
}

ObjKey*
objkey_new2 (void* ps, const char* strkey, int flag)
{
	size_t slen = strlen(strkey);
	void* p = mm_alloc (ps, slen * sizeof (unichar) + sizeof (int) * 2);
	ObjKey* ok = (ObjKey*) (((int) p) + sizeof (int) * 2);

	strcpyuni ((unichar*) ok, strkey, slen);
	KEYFLAG (ok) = flag;

	return ok;
}

ObjKey*
objkey_dup (void* ps, const ObjKey* ori)
{
	int size = unistrlen (ori) * sizeof (unichar) + sizeof (int) * 2;
	int* p = mm_alloc (ps, size);
	memcpy (p, (void*) (((int) ori) - sizeof (int) * 2), size);

	return (ObjKey*) (&p[2]);
}

static int
objkey_compare (void* left, void* right)
{
	ObjKey* a = left;
	ObjKey* b = right;
	return unistrcmp (b, a);
}

static void
objkey_free (void* ps, void* data)
{
	// printf("Free key: "); 
	//   _uniprint(data);
	psfree ((void*) (((int) data) - sizeof (int) * 2));
}

IterObj*
iterobj_new (PState* ps)
{
	IterObj* io = psmalloc (sizeof (IterObj));
	memset (io, 0, sizeof (IterObj));
	return io;
}

void
iterobj_free (PState* ps, IterObj* iobj)
{
	for (int i=0; i<iobj->count; i++) {
		objkey_free (ps, iobj->keys[i]);
	}
	psfree (iobj->keys);
	psfree (iobj);
}

static void
iterobj_insert (void* ps, IterObj* io, ObjKey* key)
{
	if (io->count >= io->size) {
		io->size += 20;
		io->keys = psrealloc (io->keys, io->size * sizeof (ObjKey*));
	}
	io->keys[io->count] = objkey_dup (ps, key);
	io->count++;
}

FuncObj*
funcobj_new (PState* ps, Func* func)
{
	FuncObj* f = mm_alloc (ps, sizeof (FuncObj));
	memset (f, 0, sizeof (FuncObj));
	f->func = func;
	return f;
}

void
funcobj_free (PState* ps, FuncObj* fobj)
{
	if (fobj->scope) {
		scope_chain_free (ps, fobj->scope);
	}
	// Do not free fobj->func, always constant
	psfree (fobj);
}

// raw object
Object*
object_new (PState* ps)
{
	Object* obj = mpool_alloc (object_pool);
	memset (obj, 0, sizeof (Object));
	obj->tree = rbtree_create (ps);
	return obj;
}

void
object_free (PState* ps, Object* obj)
{
	// printf("Free obj: %x\n", (int)obj);
	switch (obj->ot) {
	case OT_STRING:
		unifree (ps, obj->d.str);
		break;
	case OT_FUNCTION:
		funcobj_free (ps, obj->d.fobj);
		break;
	case OT_ITER:
		iterobj_free (ps, obj->d.iobj);
		break;
	case OT_USERDEF:
		userdata_free (ps, obj->d.uobj);
		break;
	// todo regex destroy
	default:
		break;
	}
	rbtree_destroy (ps, obj->tree);
	mpool_free (obj, object_pool);
}

// extern Value *Object_prototype;

Object*
object_make (PState* ps, Value* items, int count)
{
	Object* obj = object_new (ps);
	for (int i=0; i<count; i+=2) {
		if (items[i].vt != VT_STRING) {
			bug ("Making object\n");
		}
		ObjKey* ok = objkey_new (ps, items[i].d.str, 0);
		Value* v = value_new (ps);
		value_copy (*v, items[i + 1]);
		rbtree_insert (ps, obj->tree, ok, v);
	}
	obj->__proto__ = Object_prototype;
	return obj;
}

// extern Value *Array_prototype;
Object*
object_make_array (PState* ps, Value* items, int count)
{
	Object* obj = object_new (ps);
	UNISTR (12) unibuf;
	for (int i=0; i<count; ++i) {
		num_itoa10 (i, unibuf.unistr);
		ObjKey* ok = objkey_new (ps, unibuf.unistr, 0);
		Value* v = value_new (ps);
		value_copy (*v, items[i]);
		rbtree_insert (ps, obj->tree, ok, v);
	}
	object_set_length (ps, obj, count);
	obj->__proto__ = Array_prototype;
	return obj;
}

Value*
value_new (PState* ps)
{
	Value* v = mpool_alloc (value_pool);
	memset (v, 0, sizeof (Value));
	return v;
}

Value*
value_dup (PState* ps, Value* v)
{
	Value* r = value_new (ps);
	value_copy (*r, *v);
	return r;
}

void
value_free (PState* ps, void* data)
{
	Value* v = data;
	value_erase (*v);
	mpool_free (v, value_pool);
}

// far diff away from ecma, but behavior is the same
void
value_toprimitive (PState* ps, Value* v)
{
	if (v->vt == VT_OBJECT) {
		Value res;
		Object* obj = v->d.obj;
		switch (obj->ot) {
		case OT_BOOL:
			value_make_bool (res, obj->d.val);
			break;
		case OT_NUMBER:
			value_make_number (res, obj->d.num);
			break;
		case OT_STRING:
			value_make_string (res, unistrdup (ps, obj->d.str));
			break;
		default:
			value_make_string (res, unistrdup_str (ps, "[object Object]"));
			break;
		}
		value_erase (*v);
		*v = res;
	}
}

static const
UNISTR (4)
  USTRUE =
{
	4,
	L"true"
};

static const
UNISTR (5)
  USFALSE =
{
	5,
	L"false"
};

static const
UNISTR (4)
  USNULL =
{
	4,
	L"null"
};

static const
UNISTR (3)
  USNAN =
{
	3,
	L"NaN"
};

static const
UNISTR (8)
  USINF =
{
	8,
	L"Infinity"
};

static const
UNISTR (9)
  USNINF =
{
	9,
	L"-Infinity"
};

static const
UNISTR (15)
  USOBJ =
{
	15,
	L"[object Object]"
};

static const
UNISTR (9)
  USUNDEF =
{
	9,
	L"undefined"
};

void
value_tostring (PState* ps, Value* v)
{
	const unichar* ntxt = NULL;
	UNISTR (100) unibuf;
	switch (v->vt) {
	case VT_BOOL:
		ntxt = v->d.val ? USTRUE.unistr : USFALSE.unistr;
		break;
	case VT_NULL:
		ntxt = USNULL.unistr;
		break;
	case VT_NUMBER: {
		if (is_integer (v->d.num)) {
			num_itoa10 ((int) v->d.num, unibuf.unistr);
			ntxt = unibuf.unistr;
		}else if (ieee_isnormal (v->d.num)) {
			num_dtoa2 (v->d.num, unibuf.unistr, 10);
			ntxt = unibuf.unistr;
		}else if (ieee_isnan (v->d.num)) {
			ntxt = USNAN.unistr;
		}else {
			int s = ieee_infinity (v->d.num);
			if (s > 0) {
				ntxt = USINF.unistr;
			}else if (s < 0) {
				ntxt = USNINF.unistr;
			}else {
				xbug ("Ieee function got problem");
			}
		}
		break;
	}
	case VT_OBJECT: {
		Object* obj = v->d.obj;
		switch (obj->ot) {
		case OT_BOOL:
			ntxt = v->d.val ? USTRUE.unistr : USFALSE.unistr;
			break;
		case OT_NUMBER:
			if (is_integer (obj->d.num)) {
				num_itoa10 ((int) obj->d.num, unibuf.unistr);
				ntxt = unibuf.unistr;
			}else if (ieee_isnormal (obj->d.num)) {
				num_dtoa2 (obj->d.num, unibuf.unistr, 10);
				ntxt = unibuf.unistr;
			}else if (ieee_isnan (obj->d.num)) {
				ntxt = USNAN.unistr;
			}else {
				int s = ieee_infinity (obj->d.num);
				if (s > 0) {
					ntxt = USINF.unistr;
				}else if (s < 0) {
					ntxt = USNINF.unistr;
				}else {
					xbug ("Ieee function got problem");
				}
			}
			break;
		case OT_STRING:
			ntxt = obj->d.str;
			break;
		default:
			ntxt = USOBJ.unistr;
			break;
		}
		break;
	}
	case VT_STRING:
		return;
	case VT_UNDEF:
		ntxt = USUNDEF.unistr;
		break;
	default:
		xbug ("Convert a unknown type: to string\n");
		break;
	}
	value_erase (*v);		// may cause problem, gc multithread: erased, but still dup ntxt
	value_make_string (*v, unistrdup (ps, ntxt));
}

void
value_tonumber (PState* ps, Value* v)
{
	double a = 0;
	switch (v->vt) {
	case VT_BOOL:
		a = (double) (v->d.val ? 1.0 : 0);
		break;
	case VT_NULL:
		a = 0;
		break;
	case VT_OBJECT: {
		Object* obj = v->d.obj;
		switch (obj->ot) {
		case OT_BOOL:
			a = (double) (obj->d.val ? 1.0 : 0);
			break;
		case OT_NUMBER:
			a = obj->d.num;
			break;
		case OT_STRING:
			a = atof (tochars (ps, obj->d.str));
			break;
		default:
			a = 0;
			break;
		}
		break;
	}
	case VT_UNDEF:
		a = ieee_makenan ();
		break;
	case VT_NUMBER:
		return;
	case VT_STRING:		// todo, NaN
		a = atof (tochars (ps, v->d.str));
		break;
	default:
		xbug ("Convert a unknown type: to number\n");
		break;
	}
	value_erase (*v);
	value_make_number (*v, a);
}

void
value_toint32 (PState* ps, Value* v)
{
	double d = 0.0;
	value_tonumber (ps, v);
	if (ieee_isnormal (v->d.num)) {
		d = v->d.num;
	}
	// todo, not standard procedure
	v->d.num = (double) ((int) d);
}

void
value_toobject (PState* ps, Value* v)
{
	if (v->vt == VT_OBJECT) {
		return;
	}
	Object* o = object_new (ps);
	switch (v->vt) {
	case VT_UNDEF:
	case VT_NULL:
		die ("Can not convert a undefined/null value to object\n");
	case VT_BOOL: {
		o->d.val = v->d.val;
		o->ot = OT_BOOL;
		o->__proto__ = Boolean_prototype;
		break;
	}
	case VT_NUMBER: {
		o->d.num = v->d.num;
		o->ot = OT_NUMBER;
		o->__proto__ = Number_prototype;
		break;
	}
	case VT_STRING: {
		o->d.str = unistrdup (ps, v->d.str);
		o->ot = OT_STRING;
		o->__proto__ = String_prototype;
		int len = unistrlen (o->d.str);
		for (int i=0; i<len; ++i) {
			Value* v = value_new (ps);
			value_make_string (*v, unisubstrdup (ps, o->d.str, i, 1));
			object_utils_insert_array (ps, o, i, v, 0, 0, 1);
		}
		break;
	}
	default:
		bug ("toobject, not suppose to reach here\n");
	}
	value_erase (*v);
	value_make_object (*v, o);
}

// also toBoolean here, in ecma
int
value_istrue (Value* v)
{
	switch (v->vt) {
	case VT_UNDEF:
	case VT_NULL:
		return 0;
	case VT_BOOL:
		return v->d.val ? 1 : 0;
	case VT_NUMBER:
		if (v->d.num == 0.0 || ieee_isnan (v->d.num)) {
			return 0;
		}
		return 1;
	case VT_STRING:
		return unistrlen (v->d.str) ? 1 : 0;
	case VT_OBJECT: {
		Object* o = v->d.obj;
		if (o->ot == OT_USERDEF) {
			return userdata_istrue (o->d.uobj);
		}
		return 1;
	}
	default:
		return 0;			// xbug ("TOP is type incorrect\n");
	}
	return 0;
}

void
object_insert (PState* ps, Object* obj, ObjKey* key, Value* value)
{
	int ret = rbtree_insert (ps, obj->tree, key, value);
	if (ret < 0) {
		warn ("Can not assign to a read-only key\n");
	}
}

void
value_object_insert (PState* ps, Value* target, ObjKey* key, Value* value)
{
	if (target->vt != VT_OBJECT) {
		warn ("Target is not object\n");
		return;
	}
	object_insert (ps, target->d.obj, key, value);
}

void
object_try_extern (PState* ps, Object* obj, int inserted_index)
{
	int len = object_get_length (obj);
	if (len < 0) {
		return;
	}
	if (len < inserted_index + 1) {
		object_set_length (ps, obj, inserted_index + 1);
	}
}

Value*
object_lookup (Object* obj, ObjKey* key, int* flag)
{
	// object faster accesser
	// == these codes can be removed ==
	int len = unistrlen (key);
	if (len >= 0 && len < 8) {
		ObjKey* tocmp = obj->_acc_keys[len];
		if (tocmp && unistrcmp (key, tocmp) == 0) {
			if (flag) {
				*flag = KEYFLAG (tocmp);
			}
			return obj->_acc_values[len];
		}
	}
	// == end of removable ==

	ObjKey* realok = NULL;
	Value* v = rbtree_lookup (obj->tree, key, &realok);
	if (v) {
		if (!realok) {
			// xbug ("Object has value, but no key?");
			return 0;
		}
		if (flag) {
			*flag = KEYFLAG (realok);
		}
		// == ==
		if (len >= 0 && len < 8) {
			obj->_acc_values[len] = v;
			obj->_acc_keys[len] = realok;
		}
		// == ==
	}
	return v;
}

Value*
value_object_lookup (Value* target, ObjKey* key, int* flag)
{
	if (target->vt != VT_OBJECT) {
		//      warn ("Target is not object\n");
		return NULL;
	}
	return object_lookup (target->d.obj, key, flag);
}

Value*
value_object_key_assign (PState* ps, Value* target, Value* key, Value* value, int flag)
{
	int arrayindex = -1;
	if (is_number (key) && is_integer (key->d.num) && key->d.num >= 0) {
		arrayindex = (int) key->d.num;
	}
	// todo: array["1"] also extern the length of array
	
	value_tostring (ps, key);
	ObjKey* ok = objkey_new (ps, key->d.str, flag);
	
	Value* v = value_new (ps);
	value_copy (*v, *value);
	value_object_insert (ps, target, ok, v);
	if (arrayindex >= 0) {
		object_try_extern (ps, target->d.obj, arrayindex);
	}
	return v;
}

void
value_object_delete (PState* ps, Value* target, Value* key)
{
	if (target->vt != VT_OBJECT) {
		return;
	}
	
	value_tostring (ps, key);
	
	int flag = 0;
	object_lookup (target->d.obj, key->d.str, &flag);
	if (bits_get (flag, OM_DONTDEL)) {
		return;
	}
	
	// all reset to NULL
	for (int i=0; i<8; ++i) {
		target->d.obj->_acc_values[i] = NULL;
		target->d.obj->_acc_keys[i] = NULL;
	}
	rbtree_delete (ps, target->d.obj->tree, key->d.str);
}

void
value_subscript (PState* ps, Value* target, Value* key, Value* ret, int right_val)
{
	if (!target) {
		value_make_undef (*ret);
		return;
	}

	if (target->vt != VT_OBJECT) {
		xbug ("subscript operand is not object\n");
	}

	// faster string[i] access
	// == these codes can be removed ==
	if (right_val && target->d.obj->ot == OT_STRING && is_number (key) && is_integer (key->d.num)) {
		int ti = (int) key->d.num;
		unichar* s = target->d.obj->d.str;
		int len = unistrlen (s);
		if (ti >= 0 && ti < len) {
			value_make_string (*ret, unisubstrdup (ps, s, ti, 1));
			return;
		}
	}
	// == end of removable codes ==

	value_tostring (ps, key);

	int flag = 0;
	Value* r = value_object_lookup (target, (ObjKey *) key->d.str, &flag);
	if (!r) {
		// query from prototype, always no right_val
		if (target->d.obj->__proto__) {
			value_subscript (ps, target->d.obj->__proto__, key, ret, 1);
		}
		if (right_val == 0) {			// need a lvalue
			Value* n = value_new (ps);
			value_copy (*n, *ret);	// copy from prototype

			ObjKey* nk = objkey_new (ps, key->d.str, 0);
			value_object_insert (ps, target, nk, n);

			value_erase (*ret);
			ret->vt = VT_VARIABLE;
			ret->d.lval = n;
		}
	}else {
		if (right_val || bits_get (flag, OM_READONLY)) {
			value_copy (*ret, *r);
		}else {
			ret->vt = VT_VARIABLE;
			ret->d.lval = r;
		}
	}
}

int
value_key_present (Value* target, ObjKey* k)
{
	if (target->vt != VT_OBJECT) {
		return 0;
	}

	if (value_object_lookup (target, k, NULL)) {
		return 1;
	}
	if (!target->d.obj->__proto__) {
		return 0;
	}
	return value_key_present (target->d.obj->__proto__, k);
}

static int
_object_getkeys_callback (void* ps, void* key, void* value, void* userdata)
{
	ObjKey* ok = key;
	int flag = KEYFLAG (ok);

	if (!bits_get (flag, OM_DONTEMU)) {
		IterObj* io = userdata;
		iterobj_insert (ps, io, ok);
	}
	return 0;
}

static void
_object_getkeys (PState* ps, Value* target, IterObj* iterobj)
{
	if (!target) {
		return;
	}
	if (target->vt != VT_OBJECT) {
		warn ("operand is not a object\n");
		return;
	}
	rbtree_walk (ps, target->d.obj->tree, iterobj, _object_getkeys_callback);
	_object_getkeys (ps, target->d.obj->__proto__, iterobj);
}

void
value_object_getkeys (PState* ps, Value* target, Value* ret)
{
	IterObj* io = iterobj_new (ps);
	_object_getkeys (ps, target, io);
	Object* r = object_new (ps);
	r->ot = OT_ITER;
	r->d.iobj = io;

	value_make_object (*ret, r);
}

ScopeChain*
scope_chain_new (PState* ps, int cnt)
{
	ScopeChain* r = mm_alloc (ps, sizeof (ScopeChain));
	memset (r, 0, sizeof (ScopeChain));
	r->chains = mm_alloc (ps, cnt * sizeof (Value *));
	memset (r->chains, 0, cnt * sizeof (Value *));
	r->chains_cnt = cnt;
	return r;
}

Value*
scope_chain_object_lookup (PState* ps, ScopeChain* sc, ObjKey* key)
{
	Value* ret;
	for (int i=sc->chains_cnt-1; i>=0; --i) {
		if ((ret = value_object_lookup (sc->chains[i], key, NULL))) {
			return ret;
		}
	}
	return NULL;
}

ScopeChain*
scope_chain_dup_next (PState* ps, ScopeChain* sc, Value* next)
{
	if (!sc) {
		ScopeChain* nr = scope_chain_new (ps, 1);
		nr->chains[0] = value_new (ps);
		value_copy (*(nr->chains[0]), *next);
		nr->chains_cnt = 1;
		return nr;
	}
	ScopeChain* r = scope_chain_new (ps, sc->chains_cnt + 1);
	int i;
	for (i=0; i<sc->chains_cnt; ++i) {
		r->chains[i] = value_new (ps);
		value_copy (*(r->chains[i]), *(sc->chains[i]));
	}
	r->chains[i] = value_new (ps);
	value_copy (*(r->chains[i]), *next);
	r->chains_cnt = i + 1;
	return r;
}

void
scope_chain_free (PState* ps, ScopeChain* sc)
{
	for (int i=0; i<sc->chains_cnt; ++i) {
		value_free (ps, sc->chains[i]);
	}
	psfree (sc->chains);
	psfree (sc);
}

// quick set length of an object
void
object_set_length (PState* ps, Object* obj, int len)
{
	int flag = 0;
	Value* r = object_lookup (obj, (ObjKey*)LENGTH.unistr, &flag);
	if (!r) {
		Value* n = value_new (ps);
		value_make_number (*n, len);
		ObjKey* nk = objkey_new (ps, LENGTH.unistr, OM_DONTDEL | OM_DONTEMU | OM_READONLY);
		object_insert (ps, obj, nk, n);
	}else {
		value_make_number (*r, len);
	}
}

int
object_get_length (Object* obj)
{
	int flag;
	Value* r = object_lookup (obj, (ObjKey*)LENGTH.unistr, &flag);
	if (r && is_number (r)) {
		if (is_integer (r->d.num)) {
			return (int) r->d.num;
		}
	}
	return -1;
}

int
value_get_length (Value* v)
{
	if (v->vt != VT_OBJECT) {
		return -1;
	}
	return object_get_length (v->d.obj);
}

// get argv[i]
Value*
value_object_lookup_array (Value* args, int index, int* flag)
{
	UNISTR (12) unibuf;
	num_itoa10 (index, unibuf.unistr);
	return object_lookup (args->d.obj, unibuf.unistr, flag);
}

Value*
value_object_utils_new_object (PState* ps)
{
	Value* n = value_new (ps);
	value_make_object (*n, object_new (ps));
	return n;
}

void
object_utils_insert (PState* ps, Object* obj, const unichar* key, Value* val, int deletable, int writable, int emuable)
{
	int flag = 0;
	if (!deletable) {
		flag |= OM_DONTDEL;
	}
	if (!writable) {
		flag |= OM_READONLY;
	}
	if (!emuable) {
		flag |= OM_DONTEMU;
	}
	ObjKey* ok = objkey_new (ps, key, flag);
	object_insert (ps, obj, ok, val);
}

void
object_utils_insert2 (PState* ps, Object* obj, const char* key, Value* val, int deletable, int writable, int emuable)
{
	int flag = 0;
	if (!deletable) {
		flag |= OM_DONTDEL;
	}
	if (!writable) {
		flag |= OM_READONLY;
	}
	if (!emuable) {
		flag |= OM_DONTEMU;
	}
	ObjKey* ok = objkey_new2 (ps, key, flag);
	object_insert (ps, obj, ok, val);
}


void
value_object_utils_insert (PState* ps, Value* target, const unichar* key, Value* val, int deletable, int writable, int emuable)
{
	if (target->vt != VT_OBJECT) {
		warn ("Target is not object\n");
		return;
	}
	object_utils_insert (ps, target->d.obj, key, val, deletable, writable, emuable);
}

void
value_object_utils_insert2 (PState* ps, Value* target, const char* key, Value* val, int deletable, int writable, int emuable)
{
	if (target->vt != VT_OBJECT) {
		warn ("Target is not object\n");
		return;
	}
	object_utils_insert2 (ps, target->d.obj, key, val, deletable, writable, emuable);
}

void
object_utils_insert_array (PState* ps, Object* obj, int key, Value* val, int deletable, int writable, int emuable)
{
	int flag = 0;
	UNISTR (12) unibuf;
	if (!deletable) {
		flag |= OM_DONTDEL;
	}
	if (!writable) {
		flag |= OM_READONLY;
	}
	if (!emuable) {
		flag |= OM_DONTEMU;
	}
	num_itoa10 (key, unibuf.unistr);

	ObjKey* ok = objkey_new (ps, unibuf.unistr, flag);
	object_insert (ps, obj, ok, val);
	object_try_extern (ps, obj, key);
}

void
value_object_utils_insert_array (PState* ps, Value* target, int key, Value* val, int deletable, int writable, int emuable)
{
	if (target->vt != VT_OBJECT) {
		warn ("Target is not object\n");
		return;
	}
	object_utils_insert_array (ps, target->d.obj, key, val, deletable, writable, emuable);
}

static UserDataReg* global_userdataregs[MAX_UDTYPE];
static int global_userdataregs_cnt;

udid
userdata_register (PState* ps, UserDataReg* udreg)
{
	int i = global_userdataregs_cnt;
	if (i >= MAX_UDTYPE) {
		return -1;
	}
	global_userdataregs[i] = udreg;
	global_userdataregs_cnt++;
	return i;
}

UserData*
userdata_new (PState* ps, udid id, void* data)
{
	UserData* ud = mm_alloc (ps, sizeof (UserData));
	ud->id = id;
	ud->data = data;
	return ud;
}

void
userdata_free (PState* ps, UserData* ud)
{
	udid id = ud->id;
	if (id < 0 || id >= global_userdataregs_cnt) {
		xdie ("UDID error\n");
	}
	if (global_userdataregs[id]->freefun) {
		global_userdataregs[id]->freefun (ps, ud->data);
	}
	psfree (ud);
}

void
userdata_set (PState* ps, Object* obj, udid id, void* data)
{
	if (obj->ot != OT_OBJECT) {
		xbug ("userdata_assign to a non raw object\n");
	}
	obj->d.uobj = userdata_new (ps, id, data);
	obj->ot = OT_USERDEF;
}

void*
userdata_get (PState* ps, Object* obj, udid id)
{
	if (obj->ot != OT_USERDEF) {
		warn ("Object not userdefined type\n");
		return NULL;
	}
	UserData* ud = obj->d.uobj;
	if (ud->id != id) {
		warn ("Get_userdata, id not match\n");
		return NULL;
	}
	return ud->data;
}

int
userdata_istrue (UserData* ud)
{
	int id = ud->id;
	if (id < 0 || id >= global_userdataregs_cnt) {
		// xdie ("UDID error\n");
		return 0;
	}
	if (global_userdataregs[id]->istrue) {
		return global_userdataregs[id]->istrue (ud->data);
	}
	return 1;
}

static void
objvalue_free (void* ps, void* key, void* value)
{
	int flag = KEYFLAG (key);
	if (bits_get (flag, OM_INNERSHARED)) {
		return;
	}
	// printf("Free value of key: ");
	// _uniprint(key);
	value_free (ps, value);
}

static void
objvalue_vreplace (void* ps, void* target, void* from)
{
	Value* tv = target;
	Value* fv = from;
	value_erase (*tv);
	value_copy (*tv, *fv);
}

static void
objvalue_lookup_helper (void* key, void* userdata)
{
	if (!userdata) {
		return;
	}
	ObjKey** outkey = userdata;
	*outkey = key;
}

static int
objvalue_insert_helper (void* key)
{
	int flag = KEYFLAG (key);
	if (bits_get (flag, OM_READONLY)) {
		return -1;
	}
	return 0;
}

void
objects_init (memcontext* mc)
{
	object_pool = mpool_create (mc, sizeof (Object));
	value_pool = mpool_create (mc, sizeof (Value));
	rbtree_module_init (mc,
				objkey_compare, objkey_free,
				objvalue_free, objvalue_vreplace,
				objvalue_lookup_helper, objvalue_insert_helper);
}

