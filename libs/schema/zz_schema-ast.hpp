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

#ifndef YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_AST_HPP_INCLUDED
# define YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_AST_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int AST_debug;
#endif
/* "%code requires" blocks.  */


    #include "ASTBuilder.hpp"
    #include "AST_Expr.hpp"
    using namespace ncbi::SchemaParser;



/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    END_SOURCE = 0,                /* "end of source"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    UNRECOGNIZED = 258,            /* UNRECOGNIZED  */
    ELLIPSIS = 259,                /* ELLIPSIS  */
    INCREMENT = 260,               /* INCREMENT  */
    DECIMAL = 261,                 /* DECIMAL  */
    OCTAL = 262,                   /* OCTAL  */
    HEX = 263,                     /* HEX  */
    FLOAT_ = 264,                  /* FLOAT_  */
    EXP_FLOAT = 265,               /* EXP_FLOAT  */
    STRING = 266,                  /* STRING  */
    ESCAPED_STRING = 267,          /* ESCAPED_STRING  */
    IDENTIFIER_1_0 = 268,          /* IDENTIFIER_1_0  */
    PHYSICAL_IDENTIFIER_1_0 = 269, /* PHYSICAL_IDENTIFIER_1_0  */
    VERSION = 270,                 /* VERSION  */
    UNTERM_STRING = 271,           /* UNTERM_STRING  */
    UNTERM_ESCAPED_STRING = 272,   /* UNTERM_ESCAPED_STRING  */
    VERS_1_0 = 273,                /* VERS_1_0  */
    VERS_2_0 = 274,                /* VERS_2_0  */
    KW___no_header = 275,          /* KW___no_header  */
    KW___row_length = 276,         /* KW___row_length  */
    KW___untyped = 277,            /* KW___untyped  */
    KW_alias = 278,                /* KW_alias  */
    KW_column = 279,               /* KW_column  */
    KW_const = 280,                /* KW_const  */
    KW_control = 281,              /* KW_control  */
    KW_database = 282,             /* KW_database  */
    KW_decode = 283,               /* KW_decode  */
    KW_default = 284,              /* KW_default  */
    KW_encode = 285,               /* KW_encode  */
    KW_extern = 286,               /* KW_extern  */
    KW_false = 287,                /* KW_false  */
    KW_fmtdef = 288,               /* KW_fmtdef  */
    KW_function = 289,             /* KW_function  */
    KW_include = 290,              /* KW_include  */
    KW_limit = 291,                /* KW_limit  */
    KW_physical = 292,             /* KW_physical  */
    KW_read = 293,                 /* KW_read  */
    KW_readonly = 294,             /* KW_readonly  */
    KW_return = 295,               /* KW_return  */
    KW_schema = 296,               /* KW_schema  */
    KW_static = 297,               /* KW_static  */
    KW_table = 298,                /* KW_table  */
    KW_template = 299,             /* KW_template  */
    KW_trigger = 300,              /* KW_trigger  */
    KW_true = 301,                 /* KW_true  */
    KW_type = 302,                 /* KW_type  */
    KW_typedef = 303,              /* KW_typedef  */
    KW_typeset = 304,              /* KW_typeset  */
    KW_validate = 305,             /* KW_validate  */
    KW_version = 306,              /* KW_version  */
    KW_view = 307,                 /* KW_view  */
    KW_virtual = 308,              /* KW_virtual  */
    KW_void = 309,                 /* KW_void  */
    KW_write = 310,                /* KW_write  */
    PT_ASTLIST = 311,              /* PT_ASTLIST  */
    PT_PARSE = 312,                /* PT_PARSE  */
    PT_SOURCE = 313,               /* PT_SOURCE  */
    PT_VERSION_1_0 = 314,          /* PT_VERSION_1_0  */
    PT_VERSION_2 = 315,            /* PT_VERSION_2  */
    PT_SCHEMA_1_0 = 316,           /* PT_SCHEMA_1_0  */
    PT_SCHEMA_2_0 = 317,           /* PT_SCHEMA_2_0  */
    PT_INCLUDE = 318,              /* PT_INCLUDE  */
    PT_TYPEDEF = 319,              /* PT_TYPEDEF  */
    PT_FQN = 320,                  /* PT_FQN  */
    PT_IDENT = 321,                /* PT_IDENT  */
    PT_PHYSIDENT = 322,            /* PT_PHYSIDENT  */
    PT_UINT = 323,                 /* PT_UINT  */
    PT_TYPESET = 324,              /* PT_TYPESET  */
    PT_TYPESETDEF = 325,           /* PT_TYPESETDEF  */
    PT_FORMAT = 326,               /* PT_FORMAT  */
    PT_CONST = 327,                /* PT_CONST  */
    PT_ALIAS = 328,                /* PT_ALIAS  */
    PT_EXTERN = 329,               /* PT_EXTERN  */
    PT_FUNCTION = 330,             /* PT_FUNCTION  */
    PT_UNTYPED = 331,              /* PT_UNTYPED  */
    PT_ROWLENGTH = 332,            /* PT_ROWLENGTH  */
    PT_FUNCDECL = 333,             /* PT_FUNCDECL  */
    PT_EMPTY = 334,                /* PT_EMPTY  */
    PT_SCHEMASIG = 335,            /* PT_SCHEMASIG  */
    PT_SCHEMAFORMAL = 336,         /* PT_SCHEMAFORMAL  */
    PT_RETURNTYPE = 337,           /* PT_RETURNTYPE  */
    PT_FACTSIG = 338,              /* PT_FACTSIG  */
    PT_FUNCSIG = 339,              /* PT_FUNCSIG  */
    PT_FUNCPARAMS = 340,           /* PT_FUNCPARAMS  */
    PT_FORMALPARAM = 341,          /* PT_FORMALPARAM  */
    PT_ELLIPSIS = 342,             /* PT_ELLIPSIS  */
    PT_FUNCPROLOGUE = 343,         /* PT_FUNCPROLOGUE  */
    PT_RETURN = 344,               /* PT_RETURN  */
    PT_PRODSTMT = 345,             /* PT_PRODSTMT  */
    PT_PRODTRIGGER = 346,          /* PT_PRODTRIGGER  */
    PT_SCHEMA = 347,               /* PT_SCHEMA  */
    PT_VALIDATE = 348,             /* PT_VALIDATE  */
    PT_PHYSICAL = 349,             /* PT_PHYSICAL  */
    PT_PHYSPROLOGUE = 350,         /* PT_PHYSPROLOGUE  */
    PT_PHYSSTMT = 351,             /* PT_PHYSSTMT  */
    PT_PHYSBODYSTMT = 352,         /* PT_PHYSBODYSTMT  */
    PT_TABLE = 353,                /* PT_TABLE  */
    PT_TABLEPARENTS = 354,         /* PT_TABLEPARENTS  */
    PT_TABLEBODY = 355,            /* PT_TABLEBODY  */
    PT_FUNCEXPR = 356,             /* PT_FUNCEXPR  */
    PT_FACTPARMS = 357,            /* PT_FACTPARMS  */
    PT_COLUMN = 358,               /* PT_COLUMN  */
    PT_COLUMNEXPR = 359,           /* PT_COLUMNEXPR  */
    PT_COLDECL = 360,              /* PT_COLDECL  */
    PT_TYPEDCOL = 361,             /* PT_TYPEDCOL  */
    PT_COLSTMT = 362,              /* PT_COLSTMT  */
    PT_DFLTVIEW = 363,             /* PT_DFLTVIEW  */
    PT_PHYSMBR = 364,              /* PT_PHYSMBR  */
    PT_PHYSCOL = 365,              /* PT_PHYSCOL  */
    PT_PHYSCOLDEF = 366,           /* PT_PHYSCOLDEF  */
    PT_COLSCHEMAPARMS = 367,       /* PT_COLSCHEMAPARMS  */
    PT_COLSCHEMAPARAM = 368,       /* PT_COLSCHEMAPARAM  */
    PT_COLUNTYPED = 369,           /* PT_COLUNTYPED  */
    PT_DATABASE = 370,             /* PT_DATABASE  */
    PT_TYPEEXPR = 371,             /* PT_TYPEEXPR  */
    PT_DBBODY = 372,               /* PT_DBBODY  */
    PT_DBDAD = 373,                /* PT_DBDAD  */
    PT_DBMEMBER = 374,             /* PT_DBMEMBER  */
    PT_TBLMEMBER = 375,            /* PT_TBLMEMBER  */
    PT_NOHEADER = 376,             /* PT_NOHEADER  */
    PT_CASTEXPR = 377,             /* PT_CASTEXPR  */
    PT_CONSTVECT = 378,            /* PT_CONSTVECT  */
    PT_NEGATE = 379,               /* PT_NEGATE  */
    PT_UNARYPLUS = 380,            /* PT_UNARYPLUS  */
    PT_VERSNAME = 381,             /* PT_VERSNAME  */
    PT_ARRAY = 382,                /* PT_ARRAY  */
    PT_PHYSENCREF = 383,           /* PT_PHYSENCREF  */
    PT_TYPEDCOLEXPR = 384,         /* PT_TYPEDCOLEXPR  */
    PT_VIEW = 385,                 /* PT_VIEW  */
    PT_VIEWPARAM = 386,            /* PT_VIEWPARAM  */
    PT_VIEWPARENTS = 387,          /* PT_VIEWPARENTS  */
    PT_VIEWPARENT = 388,           /* PT_VIEWPARENT  */
    PT_MEMBEREXPR = 389,           /* PT_MEMBEREXPR  */
    PT_JOINEXPR = 390,             /* PT_JOINEXPR  */
    PT_ALIASMEMBER = 391,          /* PT_ALIASMEMBER  */
    PT_VIEWSPEC = 392              /* PT_VIEWSPEC  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{

  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;


};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int AST_parse (ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb);

/* "%code provides" blocks.  */

    extern const char * AST_symbol_name( enum yytokentype t );


#endif /* !YY_AST_HOME_BOSHKINS_NCBI_DEVEL_NCBI_VDB_LIBS_SCHEMA_ZZ_SCHEMA_AST_HPP_INCLUDED  */
