#ifndef __PROTO_H__
#define __PROTO_H__

struct Value;
struct strs;

/* prototype init to global naming space, return Top_object - the default 'this' value */
void proto_init(struct PSTATE* ps, struct Value* global);

/* extern struct Value* Object_prototype;
extern struct Value* Function_prototype;
extern struct Value* String_prototype;
extern struct Value* Number_prototype;
extern struct Value* Boolean_prototype;
extern struct Value* Array_prototype;
extern struct Value* RegExp_prototype;
extern struct Value* Top_object;*/

void fcall_shared_arguments(struct PSTATE*, struct Value* args, struct strs* argnames);
void fcall_set_callee(struct PSTATE*, Value* args, Value* tocall);

#endif
