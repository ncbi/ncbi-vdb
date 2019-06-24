/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_SAM_2T_DATA_ALTDEVEL_NCBI_VDB_LIBS_ALIGN_SAMEXTRACT_TOKENS_H_INCLUDED
# define YY_SAM_2T_DATA_ALTDEVEL_NCBI_VDB_LIBS_ALIGN_SAMEXTRACT_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int SAMdebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 0,
    HEADER = 258,
    SEQUENCE = 259,
    READGROUP = 260,
    PROGRAM = 261,
    COMMENT = 262,
    VALUE = 263,
    QNAME = 264,
    FLAG = 265,
    RNAME = 266,
    POS = 267,
    MAPQ = 268,
    CIGAR = 269,
    RNEXT = 270,
    PNEXT = 271,
    TLEN = 272,
    SEQ = 273,
    QUAL = 274,
    OPTTAG = 275,
    OPTITAG = 276,
    OPTZTAG = 277,
    OPTBTAG = 278,
    OPTATYPE = 279,
    OPTITYPE = 280,
    OPTFTYPE = 281,
    OPTZTYPE = 282,
    OPTHTYPE = 283,
    OPTBTYPE = 284,
    OPTAVALUE = 285,
    OPTIVALUE = 286,
    OPTFVALUE = 287,
    OPTZVALUE = 288,
    OPTHVALUE = 289,
    OPTBVALUE = 290,
    HDVN = 291,
    HDSO = 292,
    HDGO = 293,
    RGID = 294,
    RGCN = 295,
    RGDS = 296,
    RGDT = 297,
    RGFO = 298,
    RGKS = 299,
    RGLB = 300,
    RGPG = 301,
    RGPI = 302,
    RGPL = 303,
    RGPM = 304,
    RGPU = 305,
    RGSM = 306,
    PGID = 307,
    PGPN = 308,
    PGCL = 309,
    PGPP = 310,
    PGDS = 311,
    PGVN = 312,
    SQSN = 313,
    SQLN = 314,
    SQAS = 315,
    SQM5 = 316,
    SQSP = 317,
    SQUR = 318,
    TAB = 319,
    CONTROLCHAR = 320,
    EOL = 321
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


 char * strval;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE SAMlval;

int SAMparse (SAMExtractor * state);

#endif /* !YY_SAM_2T_DATA_ALTDEVEL_NCBI_VDB_LIBS_ALIGN_SAMEXTRACT_TOKENS_H_INCLUDED  */
