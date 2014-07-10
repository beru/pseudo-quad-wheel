#ifndef __PROTO_H__
#define __PROTO_H__

/* prototype init to global naming space, return Top_object - the default 'this' value */
void proto_init(PState* ps, Value* global);

/* extern struct Value* Object_prototype;
extern Value* Function_prototype;
extern Value* String_prototype;
extern Value* Number_prototype;
extern Value* Boolean_prototype;
extern Value* Array_prototype;
extern Value* RegExp_prototype;
extern Value* Top_object;*/

void fcall_shared_arguments(PState*, Value* args, strs* argnames);
void fcall_set_callee(PState*, Value* args, Value* tocall);

#endif
