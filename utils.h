#ifndef __UTILS_H__
#define __UTILS_H__

#include "unichar.h"

void utils_init(PState* ps, Value* global, int argc, char** argv);
int utils_global_eval(PState* ps, const char* program,
					  ScopeChain* scope, Value* currentScope,
					  Value* _this, Value* ret, char* codename);

#endif
