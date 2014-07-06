/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     IDENTIFIER = 259,
     IF = 260,
     ELSE = 261,
     FOR = 262,
     IN = 263,
     WHILE = 264,
     DO = 265,
     CONTINUE = 266,
     SWITCH = 267,
     CASE = 268,
     DEFAULT = 269,
     BREAK = 270,
     FUNC = 271,
     RETURN = 272,
     LOCAL = 273,
     NEW = 274,
     DELETE = 275,
     TRY = 276,
     CATCH = 277,
     FINALLY = 278,
     THROW = 279,
     WITH = 280,
     UNDEF = 281,
     _TRUE = 282,
     _FALSE = 283,
     _THIS = 284,
     ARGUMENTS = 285,
     FNUMBER = 286,
     REGEXP = 287,
     __DEBUG = 288,
     MIN_PRI = 289,
     ARGCOMMA = 290,
     DIVAS = 291,
     BXORAS = 292,
     BORAS = 293,
     BANDAS = 294,
     URSHFAS = 295,
     RSHFAS = 296,
     LSHFAS = 297,
     MODAS = 298,
     MULAS = 299,
     MNSAS = 300,
     ADDAS = 301,
     OR = 302,
     AND = 303,
     NNEQ = 304,
     EEQU = 305,
     NEQ = 306,
     EQU = 307,
     INSTANCEOF = 308,
     GEQ = 309,
     LEQ = 310,
     URSHF = 311,
     RSHF = 312,
     LSHF = 313,
     VOID = 314,
     TYPEOF = 315,
     DEC = 316,
     INC = 317,
     NEG = 318,
     MAX_PRI = 319
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



