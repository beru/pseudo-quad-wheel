#include <stdarg.h>

#if NDEBUG

void dprintf(const char *fmt, ...);

#define DPRINTF(a)  do{dprintf a;}while(0)

#define _ASSERT(a,b)

#else

#define DPRINTF(a)  do{}while(0)

#define _ASSERT(a,b)

#endif

