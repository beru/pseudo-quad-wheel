#ifndef __ERROR_H__
#define __ERROR_H__

#if 0
#if WIN32
#define die(format) \
	do { fprintf(stderr, "[Fatal] "format); exit(1); }while(0)

#define warn(format) \
	do { fprintf(stderr, "[Warning:%s:%d] "format, __FILE__, __LINE__); }while(0)

#define info(format) \
	do { fprintf(stderr, "[Info:%s:%d] "format, __FILE__, __LINE__); }while(0)

#define bug(format) \
	do { fprintf(stderr, "[Bug:%s:%d] "format"\nplease contact\n", \
		__FILE__, __LINE__); exit(1); }while(0)

#define todo(format) \
	do { fprintf(stderr, "[TODO:%s:%d] "format"\nunfinish version, sorry\n", \
		__FILE__, __LINE__); exit(1); }while(0)
#else

#define die(format,args...) \
  do { fprintf(stderr, "[Fatal] "format, ##args); ps; exit(1); }while(0)

#define warn(format,args...) \
	do { fprintf(stderr, "[Warning:%s:%d] "format, __FILE__, __LINE__, ##args); }while(0)

#define info(format,args...) \
	do { fprintf(stderr, "[Info:%s:%d] "format, __FILE__, __LINE__, ##args); }while(0)

#define bug(format,args...) \
	do { fprintf(stderr, "[Bug:%s:%d] "format"\nplease contact\n", \
		     __FILE__, __LINE__, ##args); ps; exit(1); }while(0)

#define todo(format,args...) \
	do { fprintf(stderr, "[TODO:%s:%d] "format"\nunfinish version, sorry\n", \
		     __FILE__, __LINE__, ##args); ps; exit(1); }while(0)
#endif

#else

#define die(s) pstate_svc(ps,'d',s)
#define warn(s) pstate_svc(ps,'w',s)
#define info(s) pstate_svc(ps,'i',s)
#define bug(s) pstate_svc(ps,'b',s)
#define todo(s) pstate_svc(ps,'t',s)
#define lexdie(s) pstate_svc(lex->pstate,'l',s)
#define lexbug(s) pstate_svc(lex->pstate,'u',s)
#define lextodo(s) pstate_svc(lex->pstate,'o',s)
#define xdie(s) pstate_svc(0,'d',s)
#define xbug(s) pstate_svc(0,'d',s)

void pstate_svc(void*,int, char*);

/*
#define die(format) \
  do { fprintf(stderr, "[Fatal] "format); longjmp(ps->ec.jmpbuf,1); }while(0)

#define warn(format) \
	do { fprintf(stderr, "[Warning:%s:%d] "format, __FILE__, __LINE__); }while(0)

#define info(format) \
	do { fprintf(stderr, "[Info:%s:%d] "format, __FILE__, __LINE__); }while(0)

#define bug(format) \
	do { fprintf(stderr, "[Bug:%s:%d] "format"\nplease contact\n", \
		     __FILE__, __LINE__); longjmp(ps->ec.jmpbuf,2);  }while(0)

#define todo(format) \
	do { fprintf(stderr, "[TODO:%s:%d] "format"\nunfinish version, sorry\n", \
		     __FILE__, __LINE__); longjmp(ps->ec.jmpbuf,2);  }while(0)

#define lexdie(format) \
  do { fprintf(stderr, "[Fatal] "format); longjmp(lex->pstate->ec.jmpbuf,1); }while(0)

#define warn(format) \
	do { fprintf(stderr, "[Warning:%s:%d] "format, __FILE__, __LINE__); }while(0)

#define info(format) \
	do { fprintf(stderr, "[Info:%s:%d] "format, __FILE__, __LINE__); }while(0)

#define lexbug(format) \
	do { fprintf(stderr, "[Bug:%s:%d] "format"\nplease contact\n", \
		     __FILE__, __LINE__); longjmp(lex->pstate->ec.jmpbuf,2);  }while(0)

#define lextodo(format) \
	do { fprintf(stderr, "[TODO:%s:%d] "format"\nunfinish version, sorry\n", \
		     __FILE__, __LINE__); longjmp(lex->pstate->ec.jmpbuf,2);  }while(0)

#define xbug(format) \
	do { fprintf(stderr, "[Bug:%s:%d] "format"\nplease contact\n", \
		     __FILE__, __LINE__);  }while(0)

#define xdie(format) \
	do { fprintf(stderr, "[Die:%s:%d] "format"\nplease contact\n", \
		     __FILE__, __LINE__);  }while(0)
*/

#endif

#endif
