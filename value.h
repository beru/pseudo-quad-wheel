#ifndef __VALUE_H__
#define __VALUE_H__

#include "memcontext.h"
#include "regexp.h"

#include "unichar.h"
#include "rbtree.h"
#include "scope.h"
#include "func.h"
#include "number.h"

typedef struct PSTATE PSTATE;

typedef enum {		// type			constructor	Data in Value	Implicit prototype
	VT_UNDEF,		// undefined	none		none			none 			
	VT_NULL,		// null			none		none			none 			
	VT_BOOL,		// boolean		Boolean		d.val			none 			
	VT_NUMBER,		// number		Number		d.num			Number.prototype 
	VT_STRING,		// string		String		d.str			String.prototype 
	VT_OBJECT,		// object		Object		d.obj			Object.prototype 
	VT_VARIABLE		// lvalue		none		d.lval			none 			
} vtype;

typedef unsigned int udid;

// note: UserData example, see filesys.ex.c
typedef struct {
	udid id;
	void* data;
} UserData;

typedef void (*SSUDFree)(void* ps, void* data);
typedef int (*SSUDIsTrue)(void* data);
typedef int (*SSUDIsEqu)(void* data1, void* data2);

#define MAX_UDTYPE	1024
typedef struct {
	const char* name;
	SSUDFree freefun;
	SSUDIsTrue istrue;
	SSUDIsEqu isequ;
} UserDataReg;

#define OM_READONLY		0x1		// ecma read-only
#define OM_DONTEMU		0x2		// ecma emumerable
#define OM_DONTDEL		0x4		// ecma configurable

#define OM_INNERSHARED	0x10	// shared the same value insider

#define bits_set(who, mask)		((who) |= (mask))
#define bits_unset(who, mask)	((who) &= (~(mask)))
#define bits_get(who, mask)		((who) & (mask))

/* with an ObjKey, here is how it store
 *
 *         - an unichar * pointed here
 *         | ObjKey also pointed here
 *         | 
 * | 4 | 4 | unichars |
 *   |   |
 *   |   - len of unichars
 *   - flag of objkey
 */
typedef unichar ObjKey;

#define KEYFLAG(ok) (*((int *)(((int)(ok)) - 2 * sizeof(int))))
#define OBJKEY(_len) struct{int flag;UNISTR(_len) str;}

// Scope chain
typedef struct {
	Value** chains;	// values(objects)
	int chains_cnt;			// count
} ScopeChain;

// Function obj
// a FuncObj is a raw function with own scope chain
typedef struct {
	Func* func;
	ScopeChain* scope;
} FuncObj;

// IterObj, use only in for-in statement
typedef struct {
	ObjKey** keys;
	int size;
	int count;
	int iter;
} IterObj;

typedef enum {
	OT_OBJECT,		// common object, not use d
	OT_BOOL,		// Boolean object, use d.val
	OT_NUMBER,		// Number object, use d.num
	OT_STRING,		// String object, use d.str
	OT_FUNCTION,	// Function object, use d.fobj
	OT_REGEXP,		// RegExp object, use d.robj
	OT_ITER,		// Iter object, use d.iobj
	OT_USERDEF		// UserDefined object, use d.uobj
} otype;

typedef struct Object {
	otype ot;					// object type
	union {						// see above
		int val;
		double num;
		unichar* str;
		FuncObj* fobj;
		regex_t* robj;
		IterObj* iobj;
		UserData* uobj;
	} d;
	// struct Value *acc_length;	 for array object, faster access 
	// int acc_length_flag;		 keyflag of length value 

	// faster access keys of object
	Value* _acc_values[8];
	ObjKey* _acc_keys[8];
	
	rbtree tree;				// store key-value
	int __refcnt;				// reference count
	struct Value* __proto__;	// implicit prototype
} Object;

typedef struct Value {
	vtype vt;					// value type
	union {						// see above
		int val;
		double num;
		unichar* str;
		Object* obj;
		Value* lval;
	} d;
} Value;

#define objref_inc(obj) do {					\
	(obj)->__refcnt++;							\
} while(0)

#define objref_dec(obj) do {										\
    if (--((obj)->__refcnt) <= 0) object_free(ps,(obj));		\
} while(0)

#define value_erase(v) do {											\
    if ((v).vt == VT_STRING) unifree(ps,(v).d.str);			\
	else if ((v).vt == VT_OBJECT) objref_dec((v).d.obj);			\
	(v).vt = VT_UNDEF;												\
} while(0)

#define value_copy(to, from) do {									\
	(to) = (from);													\
	if ((to).vt == VT_STRING) (to).d.str = unistrdup(ps,(to).d.str); \
	else if ((to).vt == VT_OBJECT) objref_inc((to).d.obj);			\
} while(0)

#define value_make_object(v, o) do {			\
	(v).vt = VT_OBJECT;							\
	(v).d.obj = (o);							\
	objref_inc((v).d.obj);								\
} while(0)

#define value_make_number(v, n) do {			\
	(v).vt = VT_NUMBER;							\
	(v).d.num = (n);							\
} while(0)

#define value_make_bool(v, b) do {				\
	(v).vt = VT_BOOL;							\
	(v).d.val = (b);							\
} while(0)

#define value_make_string(v, s) do {			\
	(v).vt = VT_STRING;							\
	(v).d.str = (s);							\
} while(0)

#define value_make_null(v) do {					\
	(v).vt = VT_NULL;							\
} while(0)

#define value_make_undef(v) do {				\
	(v).vt = VT_UNDEF;							\
} while(0)

#define is_integer(n) 	(ieee_isnormal(n) && (double)((int)(n)) == (n))
#define is_number(pv) 	((pv)->vt == VT_NUMBER)
#define is_string(pv) 	((pv)->vt == VT_STRING)
#define is_bool(pv) 	((pv)->vt == VT_BOOL)
#define is_undef(pv)	((pv)->vt == VT_UNDEF)
#define is_null(pv)		((pv)->vt == VT_NULL)
#define obj_isarray(o)	((o)->ot == OT_OBJECT && object_get_length(o) >= 0)

ObjKey* objkey_new(void*, const unichar* strkey, int flag);
ObjKey* objkey_new2(void*, const char* strkey, int flag);
ObjKey* objkey_dup(void*, const ObjKey* ori);

IterObj* iterobj_new(PSTATE*);
FuncObj* funcobj_new(PSTATE*, Func* func);

Object* object_new(PSTATE*);
void object_free(PSTATE*, Object* obj);
Object* object_make(PSTATE*, Value* items, int count);
Object* object_make_array(PSTATE*, Value* items, int count);

Value* value_new(PSTATE*);
Value* value_dup(PSTATE*, Value* v);
void value_free(PSTATE*, void* data);
void value_toprimitive(PSTATE*, Value* v);
void value_tostring(PSTATE*, Value* v);
void value_tonumber(PSTATE*, Value* v);
void value_toint32(PSTATE*, Value* v);
void value_toobject(PSTATE*, Value* v);
int value_istrue(Value* v);

void object_insert(PSTATE*, Object* obj, ObjKey* key, Value* value);
void value_object_insert(PSTATE*, Value* target, ObjKey* key, Value* value);
void object_try_extern(PSTATE*, Object* obj, int inserted_index);
Value* object_lookup(Object *obj, ObjKey *key, int *flag);
Value* value_object_lookup(Value* target, ObjKey* key, int* flag);
Value* value_object_key_assign(PSTATE*, Value* target, Value* key, Value* value, int flag);
void value_object_delete(PSTATE*, Value* target, Value* key);
void value_subscript(PSTATE*, Value* target, Value* key, Value* ret, int right_val);
int value_key_present(Value* target, ObjKey* k);
void value_object_getkeys(PSTATE*, Value* target, Value* ret);

ScopeChain* scope_chain_new(PSTATE*, int cnt);
Value* scope_chain_object_lookup(PSTATE*, ScopeChain* sc, ObjKey* key);
ScopeChain* scope_chain_dup_next(PSTATE*, ScopeChain* sc, Value* next);
void scope_chain_free(PSTATE*, ScopeChain* sc);

void object_set_length(PSTATE*, Object* obj, int len);
int object_get_length(Object* obj);
int value_get_length(Value* v);
Value* value_object_lookup_array(Value* args, int index, int* flag);

Value* value_object_utils_new_object(PSTATE*);
void object_utils_insert(PSTATE*, Object* obj, const unichar* key, Value* val, int deletable, int writable, int emuable);
void object_utils_insert2(PSTATE*, Object* obj, const char* key, Value* val, int deletable, int writable, int emuable);
void value_object_utils_insert(PSTATE*, Value* target, const unichar* key, Value* val, int deletable, int writable, int emuable);
void value_object_utils_insert2(PSTATE*, Value* target, const char* key, Value* val, int deletable, int writable, int emuable);

void object_utils_insert_array(PSTATE*, Object* obj, int key, Value* val, int deletable, int writable, int emuable);
void value_object_utils_insert_array(PSTATE*, Value* target, int key, Value* val, int deletable, int writable, int emuable);

udid userdata_register(PSTATE*, UserDataReg* udreg);
UserData* userdata_new(PSTATE*, udid id, void* data);
void userdata_free(PSTATE*, UserData* ud);
void userdata_set(PSTATE*, Object* obj, udid id, void* data);
void* userdata_get(PSTATE*, Object* obj, udid id);
int userdata_istrue(UserData* ud);

void objects_init(memcontext* mc);

#endif
