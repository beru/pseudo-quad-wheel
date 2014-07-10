#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pstate.h"
#include "regexp.h"
#include "value.h"
#include "func.h"
#include "error.h"

#if USE_UREGEX

regex_t*
regex_u_new (void* ps, const unsigned short* str, int len, int compflag)
{
	regex_t* reg = psmalloc (sizeof (regex_t));
	if (!reg) {
		return 0;			// die("Out of memory\n");
	}
	reg->interp = ps;

	if (regcomp_u (reg, str, len, compflag)) {
		return 0;			// die("Invalid regex string'\n");
	}

	return reg;
}

regex_t*
regex_new (PSTATE* ps, const char* str, int compflag)
{
	regex_t* reg = psmalloc (sizeof (regex_t));
	int ulen = strlen (str);
	unsigned short* ustr = psmalloc ((ulen + 1) * 2);
	if (!reg) {
		psfree(ustr);
		return 0;			// die("Out of memory\n");
	}
	int i;
	for (i=0; i<ulen; i++) {
		ustr[i] = (unsigned char) str[i];
	}
	ustr[i] = 0;
	if (regcomp_u (reg, ustr, ulen, compflag)) {
		return 0;			//  die("Invalid regex string'\n");
	}
	return reg;
}


#else // #if USE_UREGEX

regex_t*
regex_new (PSTATE* ps, const char* str, int compflag)
{
	regex_t* reg = malloc (sizeof (regex_t));
	if (!reg) {
		die ("Out of memory\n");
	}
	if (regcomp (reg, str, compflag)) {
		die ("Invalid regex string'\n");
	}
	return reg;
}

#endif // #if USE_UREGEX
