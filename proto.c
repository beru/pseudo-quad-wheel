#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pstate.h"
#include "error.h"
#include "value.h"
#include "proto.h"
#include "regexp.h"
#include "eval.h"

#include "proto.string.h"
#include "proto.number.h"
#include "proto.array.h"

// Object constructor
static int
Object_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		// new oprator will do the rest
		return 0;
	}
	if (value_get_length (args) <= 0) {
		Object* o = object_new (ps);
		o->__proto__ = Object_prototype;
		value_make_object (*ret, o);
		return 0;
	}
	Value* v = value_object_lookup_array (args, 0, NULL);
	if (!v || v->vt == VT_UNDEF || v->vt == VT_NULL) {
		Object* o = object_new (ps);
		o->__proto__ = Object_prototype;
		value_make_object (*ret, o);
		return 0;
	}
	value_copy (*ret, *v);
	value_toobject (ps, ret);
	return 0;
}

// Function.prototype pointed to a empty function
static int
Function_prototype_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	return 0;
}

static int
Function_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		// todo, parse the argument, return the new function obj
		_this->d.obj->ot = OT_FUNCTION;
		return 0;
	}
	Object* o = object_new (ps);
	o->ot = OT_FUNCTION;
	o->__proto__ = Function_prototype;
	value_make_object (*ret, o);
	return 0;
}

// delete array[0], array[1]->array[0]
static void
value_array_shift (PSTATE* ps, Value* v)
{
	if (v->vt != VT_OBJECT) {
		xbug ("value_array_shift, target is not object\n");
	}

	int len = object_get_length (v->d.obj);
	if (len <= 0) {
		return;
	}

	Value* v0 = value_object_lookup_array (v, 0, NULL);
	if (!v0) {
		return;
	}

	value_erase (*v0);

	Value* last = v0;
	for (int i=1; i<len; ++i) {
		Value* t = value_object_lookup_array (v, i, NULL);
		if (!t) {
			return;
		}
		value_copy (*last, *t);
		value_erase (*t);
		last = t;
	}

	object_set_length (ps, v->d.obj, len - 1);
}

void
fcall_shared_arguments (PSTATE* ps, Value* args, strs* argnames)
{
	if (!argnames) {
		return;
	}
	for (int i=0; i<argnames->count; ++i) {
		const unichar* argkey = strs_get (ps, argnames, i);
		if (!argkey) {
			break;
		}
		Value* v = value_object_lookup_array (args, i, NULL);
		if (v) {
			ObjKey* strkey = objkey_new (ps, argkey, OM_DONTEMU | OM_INNERSHARED);
			value_object_insert (ps, args, strkey, v);
		}else {
			ObjKey* strkey = objkey_new (ps, argkey, OM_DONTEMU);
			value_object_insert (ps, args, strkey, value_new (ps));
		}
	}
}

const static
UNISTR (8)
  _CALLEE_ =
{
  8,
  {
1, 'c', 'a', 'l', 'l', 'e', 'e', 1}};

const static
UNISTR (0)
  EMPTY =
{
	0,
	{0}
};

void
fcall_set_callee (PSTATE* ps, Value* args, Value* tocall)
{
	Value* callee = value_new (ps);
	value_copy (*callee, *tocall);
	value_object_utils_insert (ps, args, _CALLEE_.unistr, callee, 0, 0, 0);
}

// Function.prototype.call
static int
Function_prototype_call (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		die ("Execute call as constructor\n");
	}

	Value* tocall = _this;
	if (tocall->vt != VT_OBJECT || tocall->d.obj->ot != OT_FUNCTION) {
		die ("can not execute expression, expression is not a function\n");
	}

	if (!tocall->d.obj->d.fobj) {
		// empty function
		return 0;
	}

	// func to call
	Func* fstatic = tocall->d.obj->d.fobj->func;

	// new this
	Value newthis = { 0 };
	Value* arg1 = NULL;
	if ((arg1 = value_object_lookup_array (args, 0, NULL))) {
		value_copy (newthis, *arg1);
		value_toobject (ps, &newthis);
	}else {
		value_copy (newthis, *Top_object);
	}

	// prepare args
	value_array_shift (ps, args);
	fcall_shared_arguments (ps, args, fstatic->argnames);

	func_init_localvar (ps, args, fstatic);
	fcall_set_callee (ps, args, tocall);

	int res = 0;
	if (fstatic->type == FC_NORMAL) {
		res = eval (ps, fstatic->exec.opcodes, tocall->d.obj->d.fobj->scope, args, &newthis, ret);
	}else {
		res = fstatic->exec.callback (ps, args, &newthis, ret, 0);
	}
	value_erase (newthis);
	return res;
}

// Function.prototype.apply
static int
Function_prototype_apply (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		die ("Execute apply as constructor\n");
	}

	Value* tocall = _this;
	if (tocall->vt != VT_OBJECT || tocall->d.obj->ot != OT_FUNCTION) {
		die ("can not execute expression, expression is not a function\n");
	}

	if (!tocall->d.obj->d.fobj) {
		// empty function
		return 0;
	}

	// func to call
	Func* fstatic = tocall->d.obj->d.fobj->func;

	// new this
	Value newthis = { 0 };
	//Value newthis = {0};
	Value* arg1 = NULL;
	if ((arg1 = value_object_lookup_array (args, 0, NULL))) {
		value_copy (newthis, *arg1);
		value_toobject (ps, &newthis);
	}else {
		value_copy (newthis, *Top_object);
	}

	// prepare args
	Value* newscope = value_object_lookup_array (args, 1, NULL);
	if (newscope) {
		if (newscope->vt != VT_OBJECT || !obj_isarray (newscope->d.obj)) {
			die("second argument to Function.prototype.apply must be an array\n");
		}
	}else {
		newscope = value_object_utils_new_object (ps);
		object_set_length (ps, newscope->d.obj, 0);
	}

	fcall_shared_arguments (ps, newscope, fstatic->argnames);
	func_init_localvar (ps, newscope, fstatic);
	fcall_set_callee (ps, newscope, tocall);

	int res = 0;
	if (fstatic->type == FC_NORMAL) {
		res = eval (ps, fstatic->exec.opcodes, tocall->d.obj->d.fobj->scope, newscope, &newthis, ret);
	}else {
		res = fstatic->exec.callback (ps, newscope, &newthis, ret, 0);
	}
	value_erase (newthis);
	return res;
}

static int
String_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		const unichar* nv = EMPTY.unistr;
		if (value_get_length (args) > 0) {
			Value* v = value_object_lookup_array (args, 0, NULL);
			if (v) {
				value_tostring (ps, v);
				nv = v->d.str;
			}
		}
		_this->d.obj->ot = OT_STRING;
		_this->d.obj->d.str = unistrdup (ps, nv);
		object_set_length (ps, _this->d.obj, 0);

		int len = unistrlen (nv);
		for (int i=0; i<len; ++i) {
			Value* v = value_new (ps);
			value_make_string (*v, unisubstrdup (ps, nv, i, 1));
			object_utils_insert_array (ps, _this->d.obj, i, v, 0, 0, 1);
		}

		return 0;
	}
	if (value_get_length (args) > 0) {
		Value* v = value_object_lookup_array (args, 0, NULL);
		if (v) {
			value_copy (*ret, *v);
			value_tostring (ps, ret);
			return 0;
		}
	}
	value_make_string (*ret, unistrdup (ps, EMPTY.unistr));
	return 0;
}

static int
String_fromCharCode (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	UNISTR (4096) unibuf;
	if (asc) {
		die ("Can not execute String.fromCharCode as a constructor\n");
	}
	
	int len = value_get_length (args);
	if (len > 4096) {
		len = 4096;
	}
	
	unibuf.len = len;
	unichar* u = unibuf.unistr;
	
	for (int i=0; i<len; ++i) {
		Value* v = value_object_lookup_array (args, i, NULL);
		if (!v) {
			bug ("Arguments error\n");
		}
		value_tonumber (ps, v);
		u[i] = (unichar) v->d.num;
	}
	value_make_string (*ret, unistrdup (ps, unibuf.unistr));
	return 0;
}

static int
Number_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		double nv = 0.0;
		if (value_get_length (args) > 0) {
			Value* v = value_object_lookup_array (args, 0, NULL);
			if (v) {
				value_tonumber (ps, v);
				nv = v->d.num;
			}
		}
		_this->d.obj->ot = OT_NUMBER;
		_this->d.obj->d.num = nv;
		return 0;
	}
	if (value_get_length (args) > 0) {
		Value* v = value_object_lookup_array (args, 0, NULL);
		if (v) {
			value_copy (*ret, *v);
			value_tonumber (ps, ret);
			return 0;
		}
	}
	value_make_number (*ret, 0.0);
	return 0;
}

static int
Array_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	Value* target;

	if (asc) {
		target = _this;
	}else {
		Object* o = object_new (ps);
		o->__proto__ = Array_prototype;
		value_make_object (*ret, o);
		target = ret;
	}

	int argc = value_get_length (args);
	if (argc == 1) {
		Value* v = value_object_lookup_array (args, 0, NULL);
		if (v && is_number (v)) {
			if (!is_integer (v->d.num) || v->d.num < 0) {
				die ("Invalid array length\n");
			}
			object_set_length (ps, target->d.obj, (int) v->d.num);
			return 0;
		}
	}

	object_set_length (ps, target->d.obj, 0);

	for (int i=0; i<argc; ++i) {
		Value* v = value_new (ps);
		Value* argv = value_object_lookup_array (args, i, NULL);

		value_copy (*v, *argv);

		value_object_utils_insert_array (ps, _this, i, v, 1, 1, 1);
	}
	return 0;
}

static int
Boolean_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	if (asc) {
		int nv = 0;
		if (value_get_length (args) > 0) {
			Value* v = value_object_lookup_array (args, 0, NULL);
			if (v) {
				nv = value_istrue (v);
			}
		}
		_this->d.obj->ot = OT_BOOL;
		_this->d.obj->d.val = nv;
		return 0;
	}
	if (value_get_length (args) > 0) {
		Value* v = value_object_lookup_array (args, 0, NULL);
		if (v) {
			value_make_bool (*ret, value_istrue (v));
			return 0;
		}
	}
	value_make_bool (*ret, 0);
	return 0;
}

static int
RegExp_constructor (PSTATE* ps, Value* args, Value* _this, Value* ret, int asc)
{
	Value* target;
	if (asc) {
		target = _this;
	}else {
		Object* o = object_new (ps);
		o->__proto__ = RegExp_prototype;
		value_make_object (*ret, o);
		target = ret;
	}

	Value* v = value_object_lookup_array (args, 0, NULL);
	const unichar* regtxt = EMPTY.unistr;
	if (v) {
		if (v->vt == VT_OBJECT && v->d.obj->ot == OT_REGEXP) {
			value_copy (*target, *v);
			return 0;
		}else if (v->vt == VT_STRING) {
			regtxt = v->d.str;
		}			// todo tostring
	}

	int flag = REG_EXTENDED;
	Value* f = value_object_lookup_array (args, 1, NULL);
	if (f && f->vt == VT_STRING) {
		if (unistrchr (f->d.str, 'i')) {
			flag |= REG_ICASE;
		}
	}

#if USE_UREGEX
	target->d.obj->d.robj = regex_u_new (ps, regtxt, unistrlen (regtxt), flag);;
#else
	target->d.obj->d.robj = regex_new (ps, tochars (ps, regtxt), flag);
#endif
	target->d.obj->ot = OT_REGEXP;
	return 0;
}

void
proto_init (PSTATE* ps, Value* global)
{
	// object_prototype the start of protochain
	Object_prototype = value_object_utils_new_object (ps);

	// Top, the default "this" value, pointed to global, is an object
	Top_object = global;
	Top_object->d.obj->__proto__ = Object_prototype;

	// Function.prototype.prototype is a common object
	Function_prototype_prototype = value_object_utils_new_object (ps);
	Function_prototype_prototype->d.obj->__proto__ = Object_prototype;

	// Function.prototype.__proto__ pointed to Object.prototype
	Function_prototype = func_utils_make_func_value (ps, Function_prototype_constructor);
	value_object_utils_insert2 (ps, Function_prototype, "prototype", Function_prototype_prototype, 0, 0, 0);
	Function_prototype->d.obj->__proto__ = Object_prototype;

	// Function prototype.call
	{
		Value* _Function_p_call = func_utils_make_func_value (ps, Function_prototype_call);
		value_object_utils_insert2 (ps, Function_prototype, "call", _Function_p_call, 0, 0, 0);
		_Function_p_call->d.obj->__proto__ = Function_prototype;
	}
	// Function prototype.apply
	{
		Value* _Function_p_apply = func_utils_make_func_value (ps, Function_prototype_apply);
		value_object_utils_insert2 (ps, Function_prototype, "apply", _Function_p_apply, 0, 0, 0);
		_Function_p_apply->d.obj->__proto__ = Function_prototype;
	}
	// Object.__proto__ pointed to Function.prototype
	{
		Value* _Object = func_utils_make_func_value (ps, Object_constructor);
		value_object_utils_insert2 (ps, _Object, "prototype", Object_prototype, 0, 0, 0);
		_Object->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, global, "Object", _Object, 1, 1, 0);
	}
	// both Function.prototype,__proto__ pointed to Function.prototype
	{
		Value* _Function = func_utils_make_func_value (ps, Function_constructor);
		value_object_utils_insert2 (ps, _Function, "prototype", Function_prototype, 0, 0, 0);
		_Function->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, global, "Function", _Function, 1, 1, 0);
	}
	// String.prototype is a common object
	String_prototype = value_object_utils_new_object (ps);
	String_prototype->d.obj->__proto__ = Object_prototype;
	{
		Value* _String = func_utils_make_func_value (ps, String_constructor);
		value_object_utils_insert2 (ps, _String, "prototype", String_prototype, 0, 0, 0);
		_String->d.obj->__proto__ = Function_prototype;
		{
			Value* _String_fcc = func_utils_make_func_value (ps, String_fromCharCode);
			_String_fcc->d.obj->__proto__ = Function_prototype;
			value_object_utils_insert2 (ps, _String, "fromCharCode", _String_fcc, 0, 0, 0);
			value_object_utils_insert2 (ps, global, "String", _String, 1, 1, 0);
		}
	}
	Number_prototype = value_object_utils_new_object (ps);
	Number_prototype->d.obj->__proto__ = Object_prototype;
	{
		Value* _Number = func_utils_make_func_value (ps, Number_constructor);
		value_object_utils_insert2 (ps, _Number, "prototype", Number_prototype, 0, 0, 0);
		_Number->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, global, "Number", _Number, 1, 1, 0);
	}
	Boolean_prototype = value_object_utils_new_object (ps);
	Boolean_prototype->d.obj->__proto__ = Object_prototype;
	{
		Value* _Boolean = func_utils_make_func_value (ps, Boolean_constructor);
		value_object_utils_insert2 (ps, _Boolean, "prototype", Boolean_prototype, 0, 0, 0);
		_Boolean->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, global, "Boolean", _Boolean, 1, 1, 0);
	}
	Array_prototype = value_object_utils_new_object (ps);
	Array_prototype->d.obj->__proto__ = Object_prototype;
	object_set_length (ps, Array_prototype->d.obj, 0);
	{
		Value* _Array = func_utils_make_func_value (ps, Array_constructor);
		value_object_utils_insert2 (ps, _Array, "prototype", Array_prototype, 0, 0, 0);
		_Array->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, global, "Array", _Array, 1, 1, 0);
	}
	RegExp_prototype = value_object_utils_new_object (ps);
	RegExp_prototype->d.obj->__proto__ = Object_prototype;
	{
		Value* _RegExp = func_utils_make_func_value (ps, RegExp_constructor);
		value_object_utils_insert2 (ps, _RegExp, "prototype", RegExp_prototype, 0, 0, 0);
		_RegExp->d.obj->__proto__ = Function_prototype;
		value_object_utils_insert2 (ps, global, "RegExp", _RegExp, 1, 1, 0);
	}
	proto_string_init (ps, global);
	proto_number_init (ps, global);
	proto_array_init (ps, global);
}

