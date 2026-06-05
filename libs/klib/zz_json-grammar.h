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

#ifndef YY_JSON_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_KLIB_ZZ_JSON_GRAMMAR_H_INCLUDED
# define YY_JSON_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_KLIB_ZZ_JSON_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef JSON_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define JSON_DEBUG 1
#  else
#   define JSON_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define JSON_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined JSON_DEBUG */
#if JSON_DEBUG
extern int Json_debug;
#endif

/* Token kinds.  */
#ifndef JSON_TOKENTYPE
# define JSON_TOKENTYPE
  enum Json_tokentype
  {
    JSON_EMPTY = -2,
    END_SOURCE = 0,                /* "end of source"  */
    JSON_error = 256,              /* error  */
    JSON_UNDEF = 257,              /* "invalid token"  */
    UNRECOGNIZED = 258,            /* "unrecognized token"  */
    jsonSTRING = 259,              /* "string"  */
    jsonNUMBER = 260,              /* "number"  */
    jsonTRUE = 261,                /* "true"  */
    jsonFALSE = 262,               /* "false"  */
    jsonNULL = 263                 /* "null"  */
  };
  typedef enum Json_tokentype Json_token_kind_t;
#endif

/* Value type.  */
#if ! defined JSON_STYPE && ! defined JSON_STYPE_IS_DECLARED
typedef JsonToken JSON_STYPE;
# define JSON_STYPE_IS_TRIVIAL 1
# define JSON_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined JSON_LTYPE && ! defined JSON_LTYPE_IS_DECLARED
typedef struct JSON_LTYPE JSON_LTYPE;
struct JSON_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define JSON_LTYPE_IS_DECLARED 1
# define JSON_LTYPE_IS_TRIVIAL 1
#endif




int Json_parse (struct KJsonValue ** root, struct JsonScanBlock* sb);


#endif /* !YY_JSON_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_KLIB_ZZ_JSON_GRAMMAR_H_INCLUDED  */
