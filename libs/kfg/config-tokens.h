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

#ifndef YY_KFG_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_KFG_CONFIG_TOKENS_H_INCLUDED
# define YY_KFG_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_KFG_CONFIG_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int KFG_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    kfgEND_INPUT = 0,
    kfgEND_LINE = 258,
    kfgUNTERM_STRING = 259,
    kfgUNTERM_ESCAPED_STRING = 260,
    kfgUNRECOGNIZED = 261,
    kfgVAR_REF = 262,
    kfgUNTERM_VAR_REF = 263,
    kfgASSIGN = 264,
    kfgSTRING = 265,
    kfgESCAPED_STRING = 266,
    kfgABS_PATH = 267,
    kfgREL_PATH = 268
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


    KFGToken                pb;
    const struct VNamelist* namelist;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int KFG_parse (KFGParseBlock* pb, KFGScanBlock* sb);

#endif /* !YY_KFG_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_KFG_CONFIG_TOKENS_H_INCLUDED  */
