#ifndef __PROTO_GLOBAL_H__
#define __PROTO_GLOBAL_H__

struct Value;

/* Global.prototype */
void proto_global_init (PSTATE* ps, Value* global, int argc, char** argv);

#endif
