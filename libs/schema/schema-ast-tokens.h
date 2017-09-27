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

#ifndef YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED
# define YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int AST_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END_SOURCE = 0,
    UNRECOGNIZED = 258,
    ELLIPSIS = 259,
    INCREMENT = 260,
    DECIMAL = 261,
    OCTAL = 262,
    HEX = 263,
    FLOAT = 264,
    EXP_FLOAT = 265,
    STRING = 266,
    ESCAPED_STRING = 267,
    IDENTIFIER_1_0 = 268,
    PHYSICAL_IDENTIFIER_1_0 = 269,
    VERSION = 270,
    UNTERM_STRING = 271,
    UNTERM_ESCAPED_STRING = 272,
    VERS_1_0 = 273,
    VERS_2_0 = 274,
    KW___no_header = 275,
    KW___row_length = 276,
    KW___untyped = 277,
    KW_alias = 278,
    KW_column = 279,
    KW_const = 280,
    KW_control = 281,
    KW_database = 282,
    KW_decode = 283,
    KW_default = 284,
    KW_encode = 285,
    KW_extern = 286,
    KW_false = 287,
    KW_fmtdef = 288,
    KW_function = 289,
    KW_include = 290,
    KW_limit = 291,
    KW_physical = 292,
    KW_read = 293,
    KW_readonly = 294,
    KW_return = 295,
    KW_schema = 296,
    KW_static = 297,
    KW_table = 298,
    KW_template = 299,
    KW_trigger = 300,
    KW_true = 301,
    KW_type = 302,
    KW_typedef = 303,
    KW_typeset = 304,
    KW_validate = 305,
    KW_version = 306,
    KW_view = 307,
    KW_virtual = 308,
    KW_void = 309,
    KW_write = 310,
    PT_ASTLIST = 311,
    PT_PARSE = 312,
    PT_SOURCE = 313,
    PT_VERSION_1_0 = 314,
    PT_VERSION_2 = 315,
    PT_SCHEMA_1_0 = 316,
    PT_INCLUDE = 317,
    PT_TYPEDEF = 318,
    PT_FQN = 319,
    PT_IDENT = 320,
    PT_PHYSIDENT = 321,
    PT_UINT = 322,
    PT_TYPESET = 323,
    PT_TYPESETDEF = 324,
    PT_FORMAT = 325,
    PT_CONST = 326,
    PT_ALIAS = 327,
    PT_EXTERN = 328,
    PT_FUNCTION = 329,
    PT_UNTYPED = 330,
    PT_ROWLENGTH = 331,
    PT_FUNCDECL = 332,
    PT_EMPTY = 333,
    PT_SCHEMASIG = 334,
    PT_SCHEMAFORMAL = 335,
    PT_RETURNTYPE = 336,
    PT_FACTSIG = 337,
    PT_FUNCSIG = 338,
    PT_FUNCPARAMS = 339,
    PT_FORMALPARAM = 340,
    PT_ELLIPSIS = 341,
    PT_FUNCPROLOGUE = 342,
    PT_RETURN = 343,
    PT_PRODSTMT = 344,
    PT_PRODTRIGGER = 345,
    PT_SCHEMA = 346,
    PT_VALIDATE = 347,
    PT_PHYSICAL = 348,
    PT_PHYSPROLOGUE = 349,
    PT_PHYSSTMT = 350,
    PT_PHYSBODYSTMT = 351,
    PT_TABLE = 352,
    PT_TABLEPARENTS = 353,
    PT_TABLEBODY = 354,
    PT_FUNCEXPR = 355,
    PT_FACTPARMS = 356,
    PT_COLUMN = 357,
    PT_COLUMNEXPR = 358,
    PT_COLDECL = 359,
    PT_TYPEDCOL = 360,
    PT_COLSTMT = 361,
    PT_DFLTVIEW = 362,
    PT_PHYSMBR = 363,
    PT_PHYSCOL = 364,
    PT_PHYSCOLDEF = 365,
    PT_COLSCHEMAPARMS = 366,
    PT_COLSCHEMAPARAM = 367,
    PT_COLUNTYPED = 368,
    PT_DATABASE = 369,
    PT_TYPEEXPR = 370,
    PT_DBBODY = 371,
    PT_DBDAD = 372,
    PT_DBMEMBER = 373,
    PT_TBLMEMBER = 374,
    PT_NOHEADER = 375,
    PT_CASTEXPR = 376,
    PT_CONSTVECT = 377,
    PT_NEGATE = 378,
    PT_UNARYPLUS = 379,
    PT_VERSNAME = 380,
    PT_ARRAY = 381,
    PT_PHYSENCREF = 382,
    PT_TYPEDCOLEXPR = 383,
    PT_VIEW = 384,
    PT_VIEWPARAM = 385,
    PT_VIEWPARENTS = 386
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 57 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast.y" /* yacc.c:1909  */

  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;

#line 194 "/home/boshkins/devel/ncbi-vdb/libs/schema/schema-ast-tokens.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int AST_parse (AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

#endif /* !YY_AST_HOME_BOSHKINS_DEVEL_NCBI_VDB_LIBS_SCHEMA_SCHEMA_AST_TOKENS_H_INCLUDED  */
