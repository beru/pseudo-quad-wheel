#ifndef __REGEXP_H__
#define __REGEXP_H__

#define USE_UREGEX 1
#if USE_UREGEX

#include "uregex.h"
#include "unichar.h"

regex_t *regex_u_new(void*ec,const unsigned short *str, int len, int compflag);

#else
#include "regex.h"
#include "unichar.h"

regex_t *regex_new(const char *str, int compflag);
#endif

#endif

