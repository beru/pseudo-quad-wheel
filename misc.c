#include "misc.h"
#include <stdio.h>
#include <stdlib.h>

void dprintf(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	va_end(ap);
}

