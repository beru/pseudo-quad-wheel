#ifndef __EVAL_H__
#define __EVAL_H__

#include "code.h"

/* excute opcodes
 * 1. ps, program execution context
 * 2. opcodes, codes to be executed
 * 3. scope, current scopechain, not include current scope
 * 4. currentScope, current scope
 * 5. _this, where 'this' indicated
 * 6. vret, return value
 */
int eval(PState* ps,
		OpCodes* opcodes, 
		 ScopeChain* scope,
		 Value* currentScope,
		 Value* _this,
		 Value* vret);
		 
void eval_print(PState* ps);

#endif
