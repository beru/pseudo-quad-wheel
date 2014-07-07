#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unichar.h"
#include "mempool.h"
#include "pstate.h"

#define psmalloc(size)		mm_alloc(ps,size)
#define psrealloc(p,size)	mm_realloc(ps,p,size)
#define psfree(p)			mm_free(ps,p)

unichar*
unistrdup (void* ec, const unichar* str)
{
	int len = unistrlen (str);
	unichar* r = mm_alloc (ec, (len + 1) * sizeof (unichar) + sizeof (int));
	unichar* rr = (unichar*) ((int) r + sizeof (int));
	*((int*) r) = len;
	memcpy (rr, str, (len) * sizeof (unichar));
	return rr;
}

unichar*
unistrdup_str (void* ec, const char* str)
{
	int len = strlen (str);
	unichar* r = mm_alloc (ec, (len + 1) * sizeof (unichar) + sizeof (int));
	unichar* rr = (unichar*) ((int) r + sizeof (int));
	*((int*) r) = len;
	for (int i = 0; i < len; ++i) {
		rr[i] = str[i];
	}
	return rr;
}

unichar*
unisubstrdup (void* ec, const unichar* a, int start, int len)
{
	if (len == 0) {
		return unistrdup_str (ec, "");
	}
	int lenofa = unistrlen (a);
	while (start < 0) {
		start += lenofa;
	}
	if (start >= lenofa) {
		return unistrdup_str (ec, "");
	}
	int maxcpy = lenofa - start;

	if (len > 0) {
		maxcpy = maxcpy < len ? maxcpy : len;
	}

	unichar* r = mm_alloc (ec, (maxcpy + 1) * sizeof (unichar) + sizeof (int));
	unichar* rr = (unichar*) ((int) r + sizeof (int));
	*((int*) r) = maxcpy;
	memcpy (rr, a + start, maxcpy * sizeof (unichar));
	return rr;
}

void
unistrcpy (unichar* to, const unichar* from)
{
	int len = unistrlen (from);
	for (int i = 0; i < len; ++i) {
		to[i] = from[i];
	}
	unistrlen (to) = len;
}

void
_uniprint (unichar* s)
{
	int len = unistrlen (s);
	for (int i = 0; i < len; ++i) {
		printf ("%c", s[i]);
	}
	printf ("\n");
}

void
unifree (void* ec, unichar* d)
{
	void* ps = ec;
	psfree ((void*) (((int) d) - sizeof (int)));
}

int
unistrchr (const unichar* str, int c)
{
	int len = unistrlen (str);
	for (int i=0; i<len; ++i) {
		if (str[i] == c) {
			return 1;
		}
	}
	return 0;
}

const char*
tochars (void* ps, const unichar* str)
{
	char* buf = (char*)pstate_getbuf(ps);
	int len = unistrlen (str);
	int i;
	for (i = 0; i < len && i < 65530; ++i) {
		buf[i] = (char) str[i];
	}
	buf[i] = 0;
	return buf;
}

const unichar*
tounichars (void* ps, const char* str)
{
	unichar* buf = (unichar*)pstate_getbuf(ps);
	int* len = (int*) buf;
	unichar* b = (unichar*) ((int)buf + sizeof (int));
	int i;
	for (i = 0; str[i] && i < 65530; ++i) {
		b[i] = str[i];
	}
	*len = i;
	return b;
}

int
unistrcmp (const unichar* str1, const unichar* str2)
{
	int len1 = unistrlen (str1);
	int len2 = unistrlen (str2);
	if (len1 != len2) {
		return len1 - len2;
	}
	int r;
	for (int i = 0; i < len1; ++i) {
		if ((r = str1[i] - str2[i])) {
			return r;
		}
	}
	return 0;
}

unichar*
unistrcat (void* ec, const unichar* str1, const unichar* str2)
{
	int len = unistrlen (str1) + unistrlen (str2);
	unichar* r = mm_alloc (ec, (len + 1) * sizeof (unichar) + sizeof (int));
	unichar* rr = (unichar*) ((int) r + sizeof (int));
	*((int*) r) = len;
	memcpy (rr, str1, unistrlen (str1) * sizeof (unichar));
	memcpy (rr + unistrlen (str1), str2, unistrlen (str2) * sizeof (unichar));
	return rr;
}

int
unistrpos (unichar* str, int start, unichar* s2)
{
	unichar* s1 = str;
	int l1 = unistrlen (s1);
	int l2 = unistrlen (s2);
	s1 += start;
	l1 -= start;
	while (l1 >= l2) {
		if (memcmp (s1, s2, l2 * sizeof (unichar)) == 0) {
			return s1 - str;
		}
		s1++;
		l1--;
	}
	return -1;
}

/* strdup impletement */
char*
c_strdup (void* ec, const char* buf)
{
	int len = strlen (buf);
	char* ret = mm_alloc (ec, len + 1);
	memcpy (ret, buf, len + 1);
	return ret;
}

void
c_strfree (void* ec, char* buf)
{
	void* ps = ec;
	psfree ( buf);
}

