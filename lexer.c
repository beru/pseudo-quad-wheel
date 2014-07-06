#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "regexp.h"

#include "lexer.h"
#include "parser.h"
#include "error.h"
#include "regexp.h"

#define COMMENT (-128)

static int
lexer_getchar (Lexer * lex)
{
  int c = 0;
  if (!lex)
    lexbug ("No lexer init");
  if (lex->ltype == LT_FILE)
    {
      c = fgetc (lex->d.fp);
      if (c == EOF)
	c = 0;
    }
  else
    {
      c = lex->d.str[lex->cur];
      if (c != 0)
	lex->cur++;
    }
  if (c == '\n')
    {
      lex->cur_line++;
      lex->cur_char = 0;
    }
  lex->cur_char++;
  return c;
}

static void
lexer_ungetc (int c, Lexer * lex)
{
  if (!lex)
    lexbug ("No lexer init");
  if (!c)
    return;

  if (lex->ltype == LT_FILE)
    {
      ungetc (c, lex->d.fp);
    }
  else
    {
      lex->cur--;
      if (c != lex->d.str[lex->cur])
	abort ();
    }
      if (c == '\n')
        lex->cur_line--;
}

static int
iskey (const char *word)
{
  static struct st_kw
  {
    const char *name;
    int value;
  } keywords[] =
  {
    {
    "if", IF},
    {
    "else", ELSE},
    {
    "for", FOR},
    {
    "in", IN},
    {
    "while", WHILE},
    {
    "do", DO},
    {
    "continue", CONTINUE},
    {
    "switch", SWITCH},
    {
    "case", CASE},
    {
    "default", DEFAULT},
    {
    "break", BREAK},
    {
    "function", FUNC},
    {
    "return", RETURN},
    {
    "var", LOCAL},
    {
    "new", NEW},
    {
    "delete", DELETE},
    {
    "try", TRY},
    {
    "catch", CATCH},
    {
    "typeof", TYPEOF},
    {
    "throw", THROW},
    {
    "finally", FINALLY},
    {
    "with", WITH},
    {
    "undefined", UNDEF},
    {
    "true", _TRUE},
    {
    "false", _FALSE},
    {
    "this", _THIS},
    {
    "arguments", ARGUMENTS},
    {
    "void", VOID},
    {
    "__debug", __DEBUG}
  };
  int i;
  for (i = 0; i < sizeof (keywords) / sizeof (struct st_kw); ++i)
    {
      if (strcmp (word, keywords[i].name) == 0)
	return keywords[i].value;
    }
  return 0;
}

unichar *
do_string (Lexer * lex)
{
  int c = lexer_getchar (lex);
  int endchar = c;

  UNISTR (65536) unibuf;

  unichar *buf = unibuf.unistr;
  int bufi = 0;

  while (bufi < 65530)
    {
      c = lexer_getchar (lex);
      if (c == EOF || c == 0)
	{
	  lexdie ("Unexpected EOF parsing string.\n");
	}
      if (c == '\\')
	{
	  int n = lexer_getchar (lex);
	  switch (n)
	    {
	    case 'b':
	      buf[bufi++] = '\b';
	      break;
	    case 'f':
	      buf[bufi++] = '\f';
	      break;
	    case 'n':
	      buf[bufi++] = '\n';
	      break;
	    case 'r':
	      buf[bufi++] = '\r';
	      break;
	    case 't':
	      buf[bufi++] = '\t';
	      break;
	    case EOF:
	    case 0:
	      lexdie ("Unexpected EOF parsing string.\n");
	    default:
	      buf[bufi++] = n;
	    }
	}
      else
	{
	  buf[bufi++] = c;
	}
      if (c == endchar)
	{
	  bufi--;
	  break;
	}
    }
  buf[bufi] = 0;
  unibuf.len = bufi;
  return unistrdup (lex->pstate,buf);
}


char *
do_regex (Lexer * lex, int *flag)
{
  char buf[65536];
  int bufi = 0;
  char *ret;

  lexer_getchar (lex);		/* first '/' */
  while (bufi < 65530)
    {
      int c = lexer_getchar (lex);
      if (c == EOF || c == 0)
	{
	  lexdie ("Unexpected EOF parsing regular expression.\n");
	}
      if (c == '\\')
	{
	  int n = lexer_getchar (lex);
	  if (n == EOF || c == 0)
	    lexdie ("Unexpected EOF parsing regular expression.\n");

	  buf[bufi++] = c;
	  buf[bufi++] = n;
	}
      else if (c == '/')
	{
	  buf[bufi] = 0;
	  while (1)
	    {
	      c = lexer_getchar (lex);
	      if (!isalnum (c))
		break;
	      if (c == 'i')
		*flag |= REG_ICASE;
	    }
	  lexer_ungetc (c, lex);
	  break;
	}
      else
	{
	  buf[bufi++] = c;
	}
    }
  ret = c_strdup (lex->pstate,buf);
  return ret;
}


unichar *
do_regex_u (Lexer * lex, int *flag)
{
  unsigned short u[65536];
  unichar *buf = ushort2unistr(u);
  int bufi = 0;
  unichar *ret;

  lexer_getchar (lex);		/* first '/' */
  while (bufi < 65530)
    {
      int c = lexer_getchar (lex);
      if (c == EOF || c == 0)
	{
	  lexdie ("Unexpected EOF parsing regular expression.\n");
	}
      if (c == '\\')
	{
	  int n = lexer_getchar (lex);
	  if (n == EOF || c == 0)
	    lexdie ("Unexpected EOF parsing regular expression.\n");

	  buf[bufi++] = c;
	  buf[bufi++] = n;
	}
      else if (c == '/')
	{
	  buf[bufi] = 0;
	  while (1)
	    {
	      c = lexer_getchar (lex);
	      if (!isalnum (c))
		break;
	      if (c == 'i')
		*flag |= REG_ICASE;
	    }
	  lexer_ungetc (c, lex);
	  break;
	}
      else
	{
	  buf[bufi++] = c;
	}
    }
  unistrlen(buf) = bufi;
  ret = unistrdup (lex->pstate,buf);
  return ret;
}

int
do_sign (Lexer * lex)
{
  static struct st_sn
  {
    const char *name;
    int len;
    int value;
  } signs[] =
  {
    {
    ">>>=", 4, URSHFAS},
    {
    "<<=", 3, LSHFAS},
    {
    ">>=", 3, RSHFAS},
    {
    "===", 3, EEQU},
    {
    "!==", 3, NNEQ},
    {
    ">>>", 3, URSHF},
    {
    "==", 2, EQU},
    {
    "!=", 2, NEQ},
    {
    "<=", 2, LEQ},
    {
    ">=", 2, GEQ},
    {
    "++", 2, INC},
    {
    "--", 2, DEC},
    {
    "&&", 2, AND},
    {
    "||", 2, OR},
    {
    "+=", 2, ADDAS},
    {
    "-=", 2, MNSAS},
    {
    "*=", 2, MULAS},
    {
    "/=", 2, DIVAS},
    {
    "%=", 2, MODAS},
    {
    "&=", 2, BANDAS},
    {
    "|=", 2, BORAS},
    {
    "^=", 2, BXORAS},
    {
    "<<", 2, LSHF},
    {
    ">>", 2, RSHF}
  };

  int bufi;
  char buf[4];
  int i;
  for (bufi = 0; bufi < 4; ++bufi)
    {
      int c = lexer_getchar (lex);
      if (c == 0 || c == '\n')
	{
	  buf[bufi++] = c;
	  break;
	}
      buf[bufi] = c;

    }
  if (!bufi)
    return 0;

  for (i = 0; i < sizeof (signs) / sizeof (struct st_sn); ++i)
    {
      if (bufi < signs[i].len)
	continue;
      if (strncmp (buf, signs[i].name, signs[i].len) == 0)
	{
	  int j;
	  for (j = bufi - 1; j >= signs[i].len; --j)
	    lexer_ungetc (buf[j], lex);

	  return signs[i].value;
	}
    }

  for (i = bufi - 1; i >= 1; --i)
    lexer_ungetc (buf[i], lex);

  return buf[0];
}

#define LOCATION_START(loc, lex) do { 		\
	(loc)->first_line = (lex)->cur_line;	\
	(loc)->first_column = (lex)->cur_char;	\
	} while(0)
#define LOCATION_END(loc, lex) do {			\
	(loc)->last_line = (lex)->cur_line;		\
	(loc)->last_column = (lex)->cur_char;	\
	} while(0)

static void
eat_comment (Lexer * lex)
{
  int c;
  while ((c = lexer_getchar (lex)))
    {
      if (c == '*')
	{
	  c = lexer_getchar (lex);
	  if (c == '/')
	    return;
	  lexer_ungetc (c, lex);
	}
    }
  lexdie ("Comment reach end of file\n");
}

#if 0
static int
_yylex (YYSTYPE * yylvalp, YYLTYPE * yyllocp, Lexer * lex)
{
  int c;
  double *db;
  UNISTR (1024) unibuf;
  int r;

  unichar *word = unibuf.unistr;
  int wi = 0;

  LOCATION_START (yyllocp, lex);
  while ((c = lexer_getchar (lex)) == ' ' || c == '\t' || c == '\n'
	 || c == '\r');

  if (isdigit (c))
    {
      int fnum = 0;
      word[wi++] = c;
      while (wi < 1020)
	{
	  c = lexer_getchar (lex);
	  if (isdigit (c))
	    word[wi++] = c;
	  else if (c == '.')
	    {
	      if (fnum)
		lexdie ("Number format error");
	      fnum = 1;
	      word[wi++] = c;
	    }
	  else
	    {
	      lexer_ungetc (c, lex);
	      break;
	    }
	}
      LOCATION_END (yyllocp, lex);
      word[wi] = 0;
      unibuf.len = wi;
      db = malloc (sizeof (double));
      sscanf (tochars (word), "%lf", db);
      *yylvalp = db;
      return FNUMBER;
    }
  else if (c == '"' || c == '\'')
    {
      lexer_ungetc (c, lex);
      *yylvalp = do_string (lex);
      LOCATION_END (yyllocp, lex);
      return STRING;
    }
  else if (isalpha (c) || c == '_' || c == '$')
    {
      int r;
      lexer_ungetc (c, lex);
      while (wi < 1020)
	{
	  c = lexer_getchar (lex);
	  if (!isalnum (c) && c != '_' && c != '$')
	    break;
	  word[wi++] = c;
	}
      lexer_ungetc (c, lex);

      word[wi] = 0;
      unibuf.len = wi;
      r = iskey (tochars (word));
      if (r)
	return r;
      *yylvalp = unistrdup (ps,word);
      LOCATION_END (yyllocp, lex);
      return IDENTIFIER;
    }
  else if (c == '/')
    {
      int flag;
      char *regtxt;
      int d = lexer_getchar (lex);
      if (d == '/')
	{
	  while ((d = lexer_getchar (lex)) != '\r' && d != '\n' && d != 0);
	  return COMMENT;
	}
      else if (d == '*')
	{
	  eat_comment (lex);
	  return COMMENT;
	}
      else
	lexer_ungetc (d, lex);

      if (lex->last_token != FNUMBER && lex->last_token != STRING &&
	  lex->last_token != REGEXP && lex->last_token != UNDEF &&
	  lex->last_token != _TRUE && lex->last_token != _FALSE &&
	  lex->last_token != ARGUMENTS && lex->last_token != _THIS &&
	  lex->last_token != IDENTIFIER)
	{
	  lexer_ungetc (c, lex);
	  flag = REG_EXTENDED;
	  regtxt = do_regex (lex, &flag);
	  *yylvalp = regex_new (regtxt, flag);
	  c_strfree (ps,regtxt);
	  return REGEXP;
	}
    }

  lexer_ungetc (c, lex);

  r = do_sign (lex);
  LOCATION_END (yyllocp, lex);
  return r;
}
#else
static __inline int
_yylex (YYSTYPE * yylvalp, YYLTYPE * yyllocp, Lexer * lex)
{
  int c;
  double *db;
  UNISTR (1024) unibuf;
  int r;

  unichar *word = unibuf.unistr;
  int wi = 0;
  void *ps = lex->pstate;

  LOCATION_START (yyllocp, lex);
  while ((c = lexer_getchar (lex)) == ' ' || c == '\t' || c == '\n'
	 || c == '\r');

  switch (c)
    {
    case '0':
	{
	c = lexer_getchar (lex);
	if (c == 'x' || c == 'X') {
		int num = 0; int val;
		while (wi < 1020) {
		c = lexer_getchar (lex);
	    	if (isdigit (c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) {
		  word[wi++] = c;
	          num++;	
                } else {	
		  lexer_ungetc (c, lex);
		  goto end;
		}
		}
end:;
	        if (num == 0) lexdie("Number format error");
         	LOCATION_END (yyllocp, lex);
	        word[wi] = 0;
	        unibuf.len = wi;
         	db = psmalloc (sizeof (double));
        	sscanf (tochars (lex->pstate,word), "%x", &val); 
		*db = val;
        	*yylvalp = db;
        	return FNUMBER;
	} else {
		lexer_ungetc (c, lex);
		c = '0';
	}
     	} 
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
	int fnum = 0;
	word[wi++] = c;
	while (wi < 1020)
	  {
	    c = lexer_getchar (lex);
	    if (isdigit (c))
	      word[wi++] = c;
	    else if (c == '.')
	      {
		if (fnum)
		  lexdie ("Number format error");
		fnum = 1;
		word[wi++] = c;
	      }
	    else if (c == 'e' || c == 'E')
	      {
		fnum = 1;
		word[wi++] = c;
		  c = lexer_getchar (lex);
		    if (c == '+' || c == '-' || isdigit(c)) {
		      word[wi++] = c;
		    } else {
		      lexer_ungetc (c, lex);
			break;
		    }
	      }
	    else
	      {
		lexer_ungetc (c, lex);
		break;
	      }
	  }
	LOCATION_END (yyllocp, lex);
	word[wi] = 0;
	unibuf.len = wi;
	db = psmalloc (sizeof (double));
	sscanf (tochars (lex->pstate,word), "%lf", db);
	*yylvalp = db;
	return FNUMBER;
      }
    case '"':
    case '\'':
      {
	lexer_ungetc (c, lex);
	*yylvalp = do_string (lex);
	LOCATION_END (yyllocp, lex);
	return STRING;
      }
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case '_':
    case '$':
      {
	int r;
	lexer_ungetc (c, lex);
	while (wi < 1020)
	  {
	    c = lexer_getchar (lex);
	    if (!isalnum (c) && c != '_' && c != '$')
	      break;
	    word[wi++] = c;
	  }
	lexer_ungetc (c, lex);

	word[wi] = 0;
	unibuf.len = wi;
	r = iskey (tochars (lex->pstate,word));
	if (r)
	  return r;
	*yylvalp = unistrdup (lex->pstate,word);
	LOCATION_END (yyllocp, lex);
	return IDENTIFIER;
      }
    case '/':
      {
	int flag;
#if USE_UREGEX
	unichar *regtxt;
#else
	char *regtxt;
#endif
	int d = lexer_getchar (lex);
	if (d == '/')
	  {
	    while ((d = lexer_getchar (lex)) != '\r' && d != '\n' && d != 0);
	    return COMMENT;
	  }
	else if (d == '*')
	  {
	    eat_comment (lex);
	    return COMMENT;
	  }
	else
	  lexer_ungetc (d, lex);

	if (lex->last_token != FNUMBER && lex->last_token != STRING &&
	    lex->last_token != REGEXP && lex->last_token != UNDEF &&
	    lex->last_token != _TRUE && lex->last_token != _FALSE &&
	    lex->last_token != ARGUMENTS && lex->last_token != _THIS &&
	    lex->last_token != IDENTIFIER &&
	    lex->last_token != ')') 
	  {
	    lexer_ungetc (c, lex);
	    flag = REG_EXTENDED;
#if USE_UREGEX
	    regtxt = do_regex_u (lex, &flag);
	    *yylvalp = regex_u_new (lex->pstate, regtxt, unistrlen(regtxt), flag);
	    unifree (lex->pstate,regtxt);
#else
	    regtxt = do_regex (lex, &flag);
	    *yylvalp = regex_new (regtxt, flag);
	    c_strfree (lex->pstate,regtxt);
#endif
	    return REGEXP;
	  }
	break;
      }
    case ':':
    case ';':
    case '{':
    case '}':
    case '(':
    case ')':
    case ',':
    case '?':
    case '.':
    case 0:
      return c;
    default:
      break;
    }

  lexer_ungetc (c, lex);

  r = do_sign (lex);
  LOCATION_END (yyllocp, lex);
  return r;
}
#endif

int
yylex (YYSTYPE * yylvalp, YYLTYPE * yyllocp, PSTATE * pstate)
{
  int ret;
  do
    {
      ret = _yylex (yylvalp, yyllocp, pstate->lexer);
    }
  while (ret == COMMENT);
/*
	if (ret < 128 && ret > 0) printf("%c\n", ret);
	else printf("%d\n", ret);
*/
  pstate->lexer->last_token = ret;
  return ret;
}

char *lexer_codename(Lexer *lexer);

void
yyerror (YYLTYPE * yylloc, PSTATE * ps, const char *msg)
{
  fprintf (stderr, "%s:%d:[%d-%d]:%s\n", 
	   lexer_codename(ps->lexer),
    yylloc->first_line,
	   yylloc->first_column, yylloc->last_column, msg);
  ps->err_count++;
}

char *lexer_codename(Lexer *lexer) { return lexer->codename; }
