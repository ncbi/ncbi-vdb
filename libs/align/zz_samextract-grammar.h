/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_SAM_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_ALIGN_ZZ_SAMEXTRACT_GRAMMAR_H_INCLUDED
# define YY_SAM_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_ALIGN_ZZ_SAMEXTRACT_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef SAM_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define SAM_DEBUG 1
#  else
#   define SAM_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define SAM_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined SAM_DEBUG */
#if SAM_DEBUG
extern int Sam_debug;
#endif

/* Token kinds.  */
#ifndef SAM_TOKENTYPE
# define SAM_TOKENTYPE
  enum Sam_tokentype
  {
    SAM_EMPTY = -2,
    END = 0,                       /* "end of file"  */
    SAM_error = 256,               /* error  */
    SAM_UNDEF = 257,               /* "invalid token"  */
    HEADER = 258,                  /* HEADER  */
    SEQUENCE = 259,                /* SEQUENCE  */
    READGROUP = 260,               /* READGROUP  */
    PROGRAM = 261,                 /* PROGRAM  */
    COMMENT = 262,                 /* COMMENT  */
    VALUE = 263,                   /* VALUE  */
    QNAME = 264,                   /* QNAME  */
    FLAG = 265,                    /* FLAG  */
    RNAME = 266,                   /* RNAME  */
    POS = 267,                     /* POS  */
    MAPQ = 268,                    /* MAPQ  */
    CIGAR = 269,                   /* CIGAR  */
    RNEXT = 270,                   /* RNEXT  */
    PNEXT = 271,                   /* PNEXT  */
    TLEN = 272,                    /* TLEN  */
    SEQ = 273,                     /* SEQ  */
    QUAL = 274,                    /* QUAL  */
    OPTTAG = 275,                  /* OPTTAG  */
    OPTITAG = 276,                 /* OPTITAG  */
    OPTZTAG = 277,                 /* OPTZTAG  */
    OPTBTAG = 278,                 /* OPTBTAG  */
    OPTATYPE = 279,                /* OPTATYPE  */
    OPTITYPE = 280,                /* OPTITYPE  */
    OPTFTYPE = 281,                /* OPTFTYPE  */
    OPTZTYPE = 282,                /* OPTZTYPE  */
    OPTHTYPE = 283,                /* OPTHTYPE  */
    OPTBTYPE = 284,                /* OPTBTYPE  */
    OPTAVALUE = 285,               /* OPTAVALUE  */
    OPTIVALUE = 286,               /* OPTIVALUE  */
    OPTFVALUE = 287,               /* OPTFVALUE  */
    OPTZVALUE = 288,               /* OPTZVALUE  */
    OPTHVALUE = 289,               /* OPTHVALUE  */
    OPTBVALUE = 290,               /* OPTBVALUE  */
    HDVN = 291,                    /* HDVN  */
    HDSO = 292,                    /* HDSO  */
    HDGO = 293,                    /* HDGO  */
    RGID = 294,                    /* RGID  */
    RGCN = 295,                    /* RGCN  */
    RGDS = 296,                    /* RGDS  */
    RGDT = 297,                    /* RGDT  */
    RGFO = 298,                    /* RGFO  */
    RGKS = 299,                    /* RGKS  */
    RGLB = 300,                    /* RGLB  */
    RGPG = 301,                    /* RGPG  */
    RGPI = 302,                    /* RGPI  */
    RGPL = 303,                    /* RGPL  */
    RGPM = 304,                    /* RGPM  */
    RGPU = 305,                    /* RGPU  */
    RGSM = 306,                    /* RGSM  */
    PGID = 307,                    /* PGID  */
    PGPN = 308,                    /* PGPN  */
    PGCL = 309,                    /* PGCL  */
    PGPP = 310,                    /* PGPP  */
    PGDS = 311,                    /* PGDS  */
    PGVN = 312,                    /* PGVN  */
    SQSN = 313,                    /* SQSN  */
    SQLN = 314,                    /* SQLN  */
    SQAS = 315,                    /* SQAS  */
    SQM5 = 316,                    /* SQM5  */
    SQSP = 317,                    /* SQSP  */
    SQUR = 318,                    /* SQUR  */
    TAB = 319,                     /* TAB  */
    CONTROLCHAR = 320,             /* CONTROLCHAR  */
    EOL = 321                      /* EOL  */
  };
  typedef enum Sam_tokentype Sam_token_kind_t;
#endif

/* Value type.  */
#if ! defined SAM_STYPE && ! defined SAM_STYPE_IS_DECLARED
union SAM_STYPE
{

 char * strval;


};
typedef union SAM_STYPE SAM_STYPE;
# define SAM_STYPE_IS_TRIVIAL 1
# define SAM_STYPE_IS_DECLARED 1
#endif


extern SAM_STYPE Sam_lval;


int Sam_parse (SAMExtractor * state);


#endif /* !YY_SAM_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_ALIGN_ZZ_SAMEXTRACT_GRAMMAR_H_INCLUDED  */
