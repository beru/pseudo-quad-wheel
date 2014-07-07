#ifndef __UTILS_H__
#define __UTILS_H__

#include "unichar.h"

struct Value;
struct PSTATE;
struct ScopeChain;

void utils_init(PSTATE* ps, struct Value* global, int argc, char** argv);
int utils_global_eval(struct PSTATE* ps, const char* program,
					  struct ScopeChain* scope, struct Value* currentScope,
					  struct Value* _this, struct Value* ret, char* codename);

#endif
