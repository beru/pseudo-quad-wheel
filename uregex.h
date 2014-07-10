/* Copyright (c) 2003, David Leonard. All rights reserved. */

#ifndef _u_h_regex_
#define _u_h_regex_

#define FLAG_GLOBAL     0x01	/* 'g'-flag */
#define FLAG_IGNORECASE 0x02	/* 'i'-flag */
#define FLAG_MULTILINE  0x04	/* 'm'-flag */


typedef struct
{
	unsigned int rm_so;  /* Byte offset from string's start to substring's start.  */
	unsigned int rm_eo;  /* Byte offset from string's start to substring's end.  */
} regmatch_t;

#define CAPTURE_IS_UNDEFINED(c)	((c).cap_end == -1)
#define CAPTURE_SIZE(r) ((r)->statez)

struct uregex {
	struct execcontext* interp;
	struct ecma_regex* pat;
};

typedef struct uregex regex_t;

/* POSIX compatibility.  */
#define REG_ICASE FLAG_IGNORECASE
#define REG_NEWLINE FLAG_MULTILINE
#define REG_NOMATCH (!0)
#define REG_EXTENDED 0x80

extern int regcomp_u (regex_t* __preg,  const unsigned short* __pattern, int len, int __cflags);

extern int regexec_u (const regex_t* __preg, const unsigned short* __string,  int len, size_t __nmatch, regmatch_t __pmatch[], int __eflags);

extern size_t regerror (int __errcode, const regex_t* __preg, char* __errbuf, size_t __errbuf_size);

extern void regfree (regex_t* __preg);

void reg_init(
	void* (*localmalloc)(void* c, unsigned int size),
	void (*localfree)(void* c, void* p)
);

#endif /* _u_h_regex_ */
