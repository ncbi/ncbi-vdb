/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* "%code top" blocks.  */

    #define YYDEBUG 1

    #include <stdio.h>
    #include "schema-ast.hpp"

    #define AST_lex NextToken
    static int NextToken ( YYSTYPE * p_token, ParseTreeScanner & p_sb )
    {
        return p_sb . NextToken ( p_token -> tok );
    }

    void AST_error ( ctx_t ctx, void * p_parser, ASTBuilder & p_builder, ParseTreeScanner & p_sb, const char * p_msg )
    {
        INTERNAL_ERROR ( xcUnexpected, "%s: %s", p_sb . GetSourceFileName (), p_msg );
    }


/* Substitute the variable and function names.  */
#define yyparse         AST_parse
#define yylex           AST_lex
#define yyerror         AST_error
#define yydebug         AST_debug
#define yynerrs         AST_nerrs


# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "zz_schema-ast.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of source"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_UNRECOGNIZED = 3,               /* UNRECOGNIZED  */
  YYSYMBOL_ELLIPSIS = 4,                   /* ELLIPSIS  */
  YYSYMBOL_INCREMENT = 5,                  /* INCREMENT  */
  YYSYMBOL_DECIMAL = 6,                    /* DECIMAL  */
  YYSYMBOL_OCTAL = 7,                      /* OCTAL  */
  YYSYMBOL_HEX = 8,                        /* HEX  */
  YYSYMBOL_FLOAT_ = 9,                     /* FLOAT_  */
  YYSYMBOL_EXP_FLOAT = 10,                 /* EXP_FLOAT  */
  YYSYMBOL_STRING = 11,                    /* STRING  */
  YYSYMBOL_ESCAPED_STRING = 12,            /* ESCAPED_STRING  */
  YYSYMBOL_IDENTIFIER_1_0 = 13,            /* IDENTIFIER_1_0  */
  YYSYMBOL_PHYSICAL_IDENTIFIER_1_0 = 14,   /* PHYSICAL_IDENTIFIER_1_0  */
  YYSYMBOL_VERSION = 15,                   /* VERSION  */
  YYSYMBOL_UNTERM_STRING = 16,             /* UNTERM_STRING  */
  YYSYMBOL_UNTERM_ESCAPED_STRING = 17,     /* UNTERM_ESCAPED_STRING  */
  YYSYMBOL_VERS_1_0 = 18,                  /* VERS_1_0  */
  YYSYMBOL_VERS_2_0 = 19,                  /* VERS_2_0  */
  YYSYMBOL_KW___no_header = 20,            /* KW___no_header  */
  YYSYMBOL_KW___row_length = 21,           /* KW___row_length  */
  YYSYMBOL_KW___untyped = 22,              /* KW___untyped  */
  YYSYMBOL_KW_alias = 23,                  /* KW_alias  */
  YYSYMBOL_KW_column = 24,                 /* KW_column  */
  YYSYMBOL_KW_const = 25,                  /* KW_const  */
  YYSYMBOL_KW_control = 26,                /* KW_control  */
  YYSYMBOL_KW_database = 27,               /* KW_database  */
  YYSYMBOL_KW_decode = 28,                 /* KW_decode  */
  YYSYMBOL_KW_default = 29,                /* KW_default  */
  YYSYMBOL_KW_encode = 30,                 /* KW_encode  */
  YYSYMBOL_KW_extern = 31,                 /* KW_extern  */
  YYSYMBOL_KW_false = 32,                  /* KW_false  */
  YYSYMBOL_KW_fmtdef = 33,                 /* KW_fmtdef  */
  YYSYMBOL_KW_function = 34,               /* KW_function  */
  YYSYMBOL_KW_include = 35,                /* KW_include  */
  YYSYMBOL_KW_limit = 36,                  /* KW_limit  */
  YYSYMBOL_KW_physical = 37,               /* KW_physical  */
  YYSYMBOL_KW_read = 38,                   /* KW_read  */
  YYSYMBOL_KW_readonly = 39,               /* KW_readonly  */
  YYSYMBOL_KW_return = 40,                 /* KW_return  */
  YYSYMBOL_KW_schema = 41,                 /* KW_schema  */
  YYSYMBOL_KW_static = 42,                 /* KW_static  */
  YYSYMBOL_KW_table = 43,                  /* KW_table  */
  YYSYMBOL_KW_template = 44,               /* KW_template  */
  YYSYMBOL_KW_trigger = 45,                /* KW_trigger  */
  YYSYMBOL_KW_true = 46,                   /* KW_true  */
  YYSYMBOL_KW_type = 47,                   /* KW_type  */
  YYSYMBOL_KW_typedef = 48,                /* KW_typedef  */
  YYSYMBOL_KW_typeset = 49,                /* KW_typeset  */
  YYSYMBOL_KW_validate = 50,               /* KW_validate  */
  YYSYMBOL_KW_version = 51,                /* KW_version  */
  YYSYMBOL_KW_view = 52,                   /* KW_view  */
  YYSYMBOL_KW_virtual = 53,                /* KW_virtual  */
  YYSYMBOL_KW_void = 54,                   /* KW_void  */
  YYSYMBOL_KW_write = 55,                  /* KW_write  */
  YYSYMBOL_PT_ASTLIST = 56,                /* PT_ASTLIST  */
  YYSYMBOL_PT_PARSE = 57,                  /* PT_PARSE  */
  YYSYMBOL_PT_SOURCE = 58,                 /* PT_SOURCE  */
  YYSYMBOL_PT_VERSION_1_0 = 59,            /* PT_VERSION_1_0  */
  YYSYMBOL_PT_VERSION_2 = 60,              /* PT_VERSION_2  */
  YYSYMBOL_PT_SCHEMA_1_0 = 61,             /* PT_SCHEMA_1_0  */
  YYSYMBOL_PT_SCHEMA_2_0 = 62,             /* PT_SCHEMA_2_0  */
  YYSYMBOL_PT_INCLUDE = 63,                /* PT_INCLUDE  */
  YYSYMBOL_PT_TYPEDEF = 64,                /* PT_TYPEDEF  */
  YYSYMBOL_PT_FQN = 65,                    /* PT_FQN  */
  YYSYMBOL_PT_IDENT = 66,                  /* PT_IDENT  */
  YYSYMBOL_PT_PHYSIDENT = 67,              /* PT_PHYSIDENT  */
  YYSYMBOL_PT_UINT = 68,                   /* PT_UINT  */
  YYSYMBOL_PT_TYPESET = 69,                /* PT_TYPESET  */
  YYSYMBOL_PT_TYPESETDEF = 70,             /* PT_TYPESETDEF  */
  YYSYMBOL_PT_FORMAT = 71,                 /* PT_FORMAT  */
  YYSYMBOL_PT_CONST = 72,                  /* PT_CONST  */
  YYSYMBOL_PT_ALIAS = 73,                  /* PT_ALIAS  */
  YYSYMBOL_PT_EXTERN = 74,                 /* PT_EXTERN  */
  YYSYMBOL_PT_FUNCTION = 75,               /* PT_FUNCTION  */
  YYSYMBOL_PT_UNTYPED = 76,                /* PT_UNTYPED  */
  YYSYMBOL_PT_ROWLENGTH = 77,              /* PT_ROWLENGTH  */
  YYSYMBOL_PT_FUNCDECL = 78,               /* PT_FUNCDECL  */
  YYSYMBOL_PT_EMPTY = 79,                  /* PT_EMPTY  */
  YYSYMBOL_PT_SCHEMASIG = 80,              /* PT_SCHEMASIG  */
  YYSYMBOL_PT_SCHEMAFORMAL = 81,           /* PT_SCHEMAFORMAL  */
  YYSYMBOL_PT_RETURNTYPE = 82,             /* PT_RETURNTYPE  */
  YYSYMBOL_PT_FACTSIG = 83,                /* PT_FACTSIG  */
  YYSYMBOL_PT_FUNCSIG = 84,                /* PT_FUNCSIG  */
  YYSYMBOL_PT_FUNCPARAMS = 85,             /* PT_FUNCPARAMS  */
  YYSYMBOL_PT_FORMALPARAM = 86,            /* PT_FORMALPARAM  */
  YYSYMBOL_PT_ELLIPSIS = 87,               /* PT_ELLIPSIS  */
  YYSYMBOL_PT_FUNCPROLOGUE = 88,           /* PT_FUNCPROLOGUE  */
  YYSYMBOL_PT_RETURN = 89,                 /* PT_RETURN  */
  YYSYMBOL_PT_PRODSTMT = 90,               /* PT_PRODSTMT  */
  YYSYMBOL_PT_PRODTRIGGER = 91,            /* PT_PRODTRIGGER  */
  YYSYMBOL_PT_SCHEMA = 92,                 /* PT_SCHEMA  */
  YYSYMBOL_PT_VALIDATE = 93,               /* PT_VALIDATE  */
  YYSYMBOL_PT_PHYSICAL = 94,               /* PT_PHYSICAL  */
  YYSYMBOL_PT_PHYSPROLOGUE = 95,           /* PT_PHYSPROLOGUE  */
  YYSYMBOL_PT_PHYSSTMT = 96,               /* PT_PHYSSTMT  */
  YYSYMBOL_PT_PHYSBODYSTMT = 97,           /* PT_PHYSBODYSTMT  */
  YYSYMBOL_PT_TABLE = 98,                  /* PT_TABLE  */
  YYSYMBOL_PT_TABLEPARENTS = 99,           /* PT_TABLEPARENTS  */
  YYSYMBOL_PT_TABLEBODY = 100,             /* PT_TABLEBODY  */
  YYSYMBOL_PT_FUNCEXPR = 101,              /* PT_FUNCEXPR  */
  YYSYMBOL_PT_FACTPARMS = 102,             /* PT_FACTPARMS  */
  YYSYMBOL_PT_COLUMN = 103,                /* PT_COLUMN  */
  YYSYMBOL_PT_COLUMNEXPR = 104,            /* PT_COLUMNEXPR  */
  YYSYMBOL_PT_COLDECL = 105,               /* PT_COLDECL  */
  YYSYMBOL_PT_TYPEDCOL = 106,              /* PT_TYPEDCOL  */
  YYSYMBOL_PT_COLSTMT = 107,               /* PT_COLSTMT  */
  YYSYMBOL_PT_DFLTVIEW = 108,              /* PT_DFLTVIEW  */
  YYSYMBOL_PT_PHYSMBR = 109,               /* PT_PHYSMBR  */
  YYSYMBOL_PT_PHYSCOL = 110,               /* PT_PHYSCOL  */
  YYSYMBOL_PT_PHYSCOLDEF = 111,            /* PT_PHYSCOLDEF  */
  YYSYMBOL_PT_COLSCHEMAPARMS = 112,        /* PT_COLSCHEMAPARMS  */
  YYSYMBOL_PT_COLSCHEMAPARAM = 113,        /* PT_COLSCHEMAPARAM  */
  YYSYMBOL_PT_COLUNTYPED = 114,            /* PT_COLUNTYPED  */
  YYSYMBOL_PT_DATABASE = 115,              /* PT_DATABASE  */
  YYSYMBOL_PT_TYPEEXPR = 116,              /* PT_TYPEEXPR  */
  YYSYMBOL_PT_DBBODY = 117,                /* PT_DBBODY  */
  YYSYMBOL_PT_DBDAD = 118,                 /* PT_DBDAD  */
  YYSYMBOL_PT_DBMEMBER = 119,              /* PT_DBMEMBER  */
  YYSYMBOL_PT_TBLMEMBER = 120,             /* PT_TBLMEMBER  */
  YYSYMBOL_PT_NOHEADER = 121,              /* PT_NOHEADER  */
  YYSYMBOL_PT_CASTEXPR = 122,              /* PT_CASTEXPR  */
  YYSYMBOL_PT_CONSTVECT = 123,             /* PT_CONSTVECT  */
  YYSYMBOL_PT_NEGATE = 124,                /* PT_NEGATE  */
  YYSYMBOL_PT_UNARYPLUS = 125,             /* PT_UNARYPLUS  */
  YYSYMBOL_PT_VERSNAME = 126,              /* PT_VERSNAME  */
  YYSYMBOL_PT_ARRAY = 127,                 /* PT_ARRAY  */
  YYSYMBOL_PT_PHYSENCREF = 128,            /* PT_PHYSENCREF  */
  YYSYMBOL_PT_TYPEDCOLEXPR = 129,          /* PT_TYPEDCOLEXPR  */
  YYSYMBOL_PT_VIEW = 130,                  /* PT_VIEW  */
  YYSYMBOL_PT_VIEWPARAM = 131,             /* PT_VIEWPARAM  */
  YYSYMBOL_PT_VIEWPARENTS = 132,           /* PT_VIEWPARENTS  */
  YYSYMBOL_PT_VIEWPARENT = 133,            /* PT_VIEWPARENT  */
  YYSYMBOL_PT_MEMBEREXPR = 134,            /* PT_MEMBEREXPR  */
  YYSYMBOL_PT_JOINEXPR = 135,              /* PT_JOINEXPR  */
  YYSYMBOL_PT_ALIASMEMBER = 136,           /* PT_ALIASMEMBER  */
  YYSYMBOL_PT_VIEWSPEC = 137,              /* PT_VIEWSPEC  */
  YYSYMBOL_138_ = 138,                     /* '@'  */
  YYSYMBOL_139_ = 139,                     /* '('  */
  YYSYMBOL_140_ = 140,                     /* ')'  */
  YYSYMBOL_141_ = 141,                     /* ';'  */
  YYSYMBOL_142_ = 142,                     /* ','  */
  YYSYMBOL_143_ = 143,                     /* '['  */
  YYSYMBOL_144_ = 144,                     /* ']'  */
  YYSYMBOL_145_ = 145,                     /* '*'  */
  YYSYMBOL_146_ = 146,                     /* '{'  */
  YYSYMBOL_147_ = 147,                     /* '}'  */
  YYSYMBOL_148_ = 148,                     /* '='  */
  YYSYMBOL_149_ = 149,                     /* '<'  */
  YYSYMBOL_150_ = 150,                     /* '>'  */
  YYSYMBOL_151_ = 151,                     /* '|'  */
  YYSYMBOL_152_ = 152,                     /* '+'  */
  YYSYMBOL_153_ = 153,                     /* '-'  */
  YYSYMBOL_154_ = 154,                     /* '/'  */
  YYSYMBOL_155_ = 155,                     /* '.'  */
  YYSYMBOL_156_ = 156,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 157,                 /* $accept  */
  YYSYMBOL_parse = 158,                    /* parse  */
  YYSYMBOL_source = 159,                   /* source  */
  YYSYMBOL_version_1 = 160,                /* version_1  */
  YYSYMBOL_version_2 = 161,                /* version_2  */
  YYSYMBOL_schema_1 = 162,                 /* schema_1  */
  YYSYMBOL_schema_2 = 163,                 /* schema_2  */
  YYSYMBOL_schema_decls = 164,             /* schema_decls  */
  YYSYMBOL_schema_decl = 165,              /* schema_decl  */
  YYSYMBOL_typedef = 166,                  /* typedef  */
  YYSYMBOL_new_type_names = 167,           /* new_type_names  */
  YYSYMBOL_typespec = 168,                 /* typespec  */
  YYSYMBOL_arrayspec = 169,                /* arrayspec  */
  YYSYMBOL_dim = 170,                      /* dim  */
  YYSYMBOL_typeset = 171,                  /* typeset  */
  YYSYMBOL_typeset_spec = 172,             /* typeset_spec  */
  YYSYMBOL_fmtdef = 173,                   /* fmtdef  */
  YYSYMBOL_const = 174,                    /* const  */
  YYSYMBOL_alias = 175,                    /* alias  */
  YYSYMBOL_function = 176,                 /* function  */
  YYSYMBOL_func_decl = 177,                /* func_decl  */
  YYSYMBOL_schema_sig_opt = 178,           /* schema_sig_opt  */
  YYSYMBOL_schema_formals = 179,           /* schema_formals  */
  YYSYMBOL_schema_formal = 180,            /* schema_formal  */
  YYSYMBOL_return_type = 181,              /* return_type  */
  YYSYMBOL_fact_sig = 182,                 /* fact_sig  */
  YYSYMBOL_param_sig = 183,                /* param_sig  */
  YYSYMBOL_param_signature = 184,          /* param_signature  */
  YYSYMBOL_formals_list = 185,             /* formals_list  */
  YYSYMBOL_formals = 186,                  /* formals  */
  YYSYMBOL_formal = 187,                   /* formal  */
  YYSYMBOL_vararg = 188,                   /* vararg  */
  YYSYMBOL_prologue = 189,                 /* prologue  */
  YYSYMBOL_script_prologue = 190,          /* script_prologue  */
  YYSYMBOL_script_stmts = 191,             /* script_stmts  */
  YYSYMBOL_script_stmt = 192,              /* script_stmt  */
  YYSYMBOL_production = 193,               /* production  */
  YYSYMBOL_extern_function = 194,          /* extern_function  */
  YYSYMBOL_script = 195,                   /* script  */
  YYSYMBOL_script_decl = 196,              /* script_decl  */
  YYSYMBOL_validate = 197,                 /* validate  */
  YYSYMBOL_physical = 198,                 /* physical  */
  YYSYMBOL_phys_return_type = 199,         /* phys_return_type  */
  YYSYMBOL_phys_prologue = 200,            /* phys_prologue  */
  YYSYMBOL_phys_body = 201,                /* phys_body  */
  YYSYMBOL_phys_body_stmt = 202,           /* phys_body_stmt  */
  YYSYMBOL_table = 203,                    /* table  */
  YYSYMBOL_parents_opt = 204,              /* parents_opt  */
  YYSYMBOL_tbl_parents = 205,              /* tbl_parents  */
  YYSYMBOL_tbl_body = 206,                 /* tbl_body  */
  YYSYMBOL_tbl_stmts = 207,                /* tbl_stmts  */
  YYSYMBOL_tbl_stmt = 208,                 /* tbl_stmt  */
  YYSYMBOL_column_decl = 209,              /* column_decl  */
  YYSYMBOL_col_modifiers_opt = 210,        /* col_modifiers_opt  */
  YYSYMBOL_col_modifiers = 211,            /* col_modifiers  */
  YYSYMBOL_col_modifier = 212,             /* col_modifier  */
  YYSYMBOL_col_decl = 213,                 /* col_decl  */
  YYSYMBOL_phys_enc_ref = 214,             /* phys_enc_ref  */
  YYSYMBOL_typed_col = 215,                /* typed_col  */
  YYSYMBOL_col_ident = 216,                /* col_ident  */
  YYSYMBOL_col_body_opt = 217,             /* col_body_opt  */
  YYSYMBOL_col_body = 218,                 /* col_body  */
  YYSYMBOL_col_stmt = 219,                 /* col_stmt  */
  YYSYMBOL_physmbr_decl = 220,             /* physmbr_decl  */
  YYSYMBOL_phys_coldef = 221,              /* phys_coldef  */
  YYSYMBOL_col_schema_parms_opt = 222,     /* col_schema_parms_opt  */
  YYSYMBOL_col_schema_parms = 223,         /* col_schema_parms  */
  YYSYMBOL_col_schema_parm = 224,          /* col_schema_parm  */
  YYSYMBOL_col_schema_value = 225,         /* col_schema_value  */
  YYSYMBOL_database = 226,                 /* database  */
  YYSYMBOL_dbdad_opt = 227,                /* dbdad_opt  */
  YYSYMBOL_dbbody = 228,                   /* dbbody  */
  YYSYMBOL_db_members = 229,               /* db_members  */
  YYSYMBOL_db_member = 230,                /* db_member  */
  YYSYMBOL_template_opt = 231,             /* template_opt  */
  YYSYMBOL_view_spec = 232,                /* view_spec  */
  YYSYMBOL_view_parms = 233,               /* view_parms  */
  YYSYMBOL_include = 234,                  /* include  */
  YYSYMBOL_cond_expr = 235,                /* cond_expr  */
  YYSYMBOL_cond_chain = 236,               /* cond_chain  */
  YYSYMBOL_expr = 237,                     /* expr  */
  YYSYMBOL_func_expr = 238,                /* func_expr  */
  YYSYMBOL_schema_parts_opt = 239,         /* schema_parts_opt  */
  YYSYMBOL_schema_parms = 240,             /* schema_parms  */
  YYSYMBOL_schema_parm = 241,              /* schema_parm  */
  YYSYMBOL_factory_parms_opt = 242,        /* factory_parms_opt  */
  YYSYMBOL_factory_parms_list = 243,       /* factory_parms_list  */
  YYSYMBOL_factory_parms = 244,            /* factory_parms  */
  YYSYMBOL_func_parms_opt = 245,           /* func_parms_opt  */
  YYSYMBOL_expr_list = 246,                /* expr_list  */
  YYSYMBOL_uint_expr = 247,                /* uint_expr  */
  YYSYMBOL_float_expr = 248,               /* float_expr  */
  YYSYMBOL_string_expr = 249,              /* string_expr  */
  YYSYMBOL_const_vect_expr = 250,          /* const_vect_expr  */
  YYSYMBOL_bool_expr = 251,                /* bool_expr  */
  YYSYMBOL_negate_expr = 252,              /* negate_expr  */
  YYSYMBOL_cast_expr = 253,                /* cast_expr  */
  YYSYMBOL_type_expr = 254,                /* type_expr  */
  YYSYMBOL_member_expr = 255,              /* member_expr  */
  YYSYMBOL_join_expr = 256,                /* join_expr  */
  YYSYMBOL_fqn = 257,                      /* fqn  */
  YYSYMBOL_qualnames = 258,                /* qualnames  */
  YYSYMBOL_ident = 259,                    /* ident  */
  YYSYMBOL_fqn_opt_vers = 260,             /* fqn_opt_vers  */
  YYSYMBOL_fqn_vers = 261,                 /* fqn_vers  */
  YYSYMBOL_view = 262,                     /* view  */
  YYSYMBOL_view_parm = 263,                /* view_parm  */
  YYSYMBOL_view_body_opt = 264,            /* view_body_opt  */
  YYSYMBOL_view_body = 265,                /* view_body  */
  YYSYMBOL_view_member = 266,              /* view_member  */
  YYSYMBOL_view_parents_opt = 267,         /* view_parents_opt  */
  YYSYMBOL_view_parents = 268,             /* view_parents  */
  YYSYMBOL_view_parent = 269,              /* view_parent  */
  YYSYMBOL_view_parent_parms = 270         /* view_parent_parms  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1180

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  157
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  114
/* YYNRULES -- Number of rules.  */
#define YYNRULES  249
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  879

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   392


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     139,   140,   145,   152,   142,   153,   155,   154,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   156,   141,
     149,   148,   150,     2,   138,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   143,     2,   144,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   146,   151,   147,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   272,   272,   273,   277,   278,   279,   283,   287,   291,
     292,   296,   297,   302,   303,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     327,   332,   333,   337,   338,   342,   346,   347,   351,   356,
     357,   361,   362,   366,   370,   374,   378,   379,   380,   385,
     386,   390,   391,   395,   396,   400,   401,   405,   406,   410,
     414,   415,   416,   417,   418,   422,   426,   427,   431,   432,
     436,   437,   441,   442,   443,   447,   451,   452,   456,   457,
     461,   462,   466,   469,   470,   474,   479,   488,   493,   494,
     497,   499,   504,   505,   509,   511,   513,   515,   522,   527,
     528,   532,   533,   537,   538,   542,   543,   547,   548,   549,
     550,   551,   552,   553,   554,   555,   559,   563,   564,   568,
     569,   573,   574,   575,   579,   580,   584,   586,   587,   591,
     592,   593,   597,   598,   602,   603,   607,   608,   612,   613,
     614,   615,   619,   621,   623,   625,   630,   634,   635,   639,
     640,   644,   645,   649,   650,   655,   659,   660,   663,   664,
     668,   669,   670,   671,   675,   677,   679,   684,   685,   689,
     694,   695,   701,   707,   711,   712,   716,   717,   718,   719,
     720,   721,   722,   723,   724,   725,   726,   727,   728,   729,
     733,   738,   739,   742,   743,   747,   748,   749,   753,   754,
     758,   762,   763,   767,   768,   772,   773,   777,   778,   779,
     783,   784,   788,   789,   793,   797,   798,   802,   805,   809,
     810,   814,   815,   827,   829,   844,   848,   849,   853,   857,
     858,   862,   867,   873,   874,   878,   882,   883,   887,   888,
     892,   893,   894,   898,   899,   903,   904,   908,   913,   914
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of source\"", "error", "\"invalid token\"", "UNRECOGNIZED",
  "ELLIPSIS", "INCREMENT", "DECIMAL", "OCTAL", "HEX", "FLOAT_",
  "EXP_FLOAT", "STRING", "ESCAPED_STRING", "IDENTIFIER_1_0",
  "PHYSICAL_IDENTIFIER_1_0", "VERSION", "UNTERM_STRING",
  "UNTERM_ESCAPED_STRING", "VERS_1_0", "VERS_2_0", "KW___no_header",
  "KW___row_length", "KW___untyped", "KW_alias", "KW_column", "KW_const",
  "KW_control", "KW_database", "KW_decode", "KW_default", "KW_encode",
  "KW_extern", "KW_false", "KW_fmtdef", "KW_function", "KW_include",
  "KW_limit", "KW_physical", "KW_read", "KW_readonly", "KW_return",
  "KW_schema", "KW_static", "KW_table", "KW_template", "KW_trigger",
  "KW_true", "KW_type", "KW_typedef", "KW_typeset", "KW_validate",
  "KW_version", "KW_view", "KW_virtual", "KW_void", "KW_write",
  "PT_ASTLIST", "PT_PARSE", "PT_SOURCE", "PT_VERSION_1_0", "PT_VERSION_2",
  "PT_SCHEMA_1_0", "PT_SCHEMA_2_0", "PT_INCLUDE", "PT_TYPEDEF", "PT_FQN",
  "PT_IDENT", "PT_PHYSIDENT", "PT_UINT", "PT_TYPESET", "PT_TYPESETDEF",
  "PT_FORMAT", "PT_CONST", "PT_ALIAS", "PT_EXTERN", "PT_FUNCTION",
  "PT_UNTYPED", "PT_ROWLENGTH", "PT_FUNCDECL", "PT_EMPTY", "PT_SCHEMASIG",
  "PT_SCHEMAFORMAL", "PT_RETURNTYPE", "PT_FACTSIG", "PT_FUNCSIG",
  "PT_FUNCPARAMS", "PT_FORMALPARAM", "PT_ELLIPSIS", "PT_FUNCPROLOGUE",
  "PT_RETURN", "PT_PRODSTMT", "PT_PRODTRIGGER", "PT_SCHEMA", "PT_VALIDATE",
  "PT_PHYSICAL", "PT_PHYSPROLOGUE", "PT_PHYSSTMT", "PT_PHYSBODYSTMT",
  "PT_TABLE", "PT_TABLEPARENTS", "PT_TABLEBODY", "PT_FUNCEXPR",
  "PT_FACTPARMS", "PT_COLUMN", "PT_COLUMNEXPR", "PT_COLDECL",
  "PT_TYPEDCOL", "PT_COLSTMT", "PT_DFLTVIEW", "PT_PHYSMBR", "PT_PHYSCOL",
  "PT_PHYSCOLDEF", "PT_COLSCHEMAPARMS", "PT_COLSCHEMAPARAM",
  "PT_COLUNTYPED", "PT_DATABASE", "PT_TYPEEXPR", "PT_DBBODY", "PT_DBDAD",
  "PT_DBMEMBER", "PT_TBLMEMBER", "PT_NOHEADER", "PT_CASTEXPR",
  "PT_CONSTVECT", "PT_NEGATE", "PT_UNARYPLUS", "PT_VERSNAME", "PT_ARRAY",
  "PT_PHYSENCREF", "PT_TYPEDCOLEXPR", "PT_VIEW", "PT_VIEWPARAM",
  "PT_VIEWPARENTS", "PT_VIEWPARENT", "PT_MEMBEREXPR", "PT_JOINEXPR",
  "PT_ALIASMEMBER", "PT_VIEWSPEC", "'@'", "'('", "')'", "';'", "','",
  "'['", "']'", "'*'", "'{'", "'}'", "'='", "'<'", "'>'", "'|'", "'+'",
  "'-'", "'/'", "'.'", "':'", "$accept", "parse", "source", "version_1",
  "version_2", "schema_1", "schema_2", "schema_decls", "schema_decl",
  "typedef", "new_type_names", "typespec", "arrayspec", "dim", "typeset",
  "typeset_spec", "fmtdef", "const", "alias", "function", "func_decl",
  "schema_sig_opt", "schema_formals", "schema_formal", "return_type",
  "fact_sig", "param_sig", "param_signature", "formals_list", "formals",
  "formal", "vararg", "prologue", "script_prologue", "script_stmts",
  "script_stmt", "production", "extern_function", "script", "script_decl",
  "validate", "physical", "phys_return_type", "phys_prologue", "phys_body",
  "phys_body_stmt", "table", "parents_opt", "tbl_parents", "tbl_body",
  "tbl_stmts", "tbl_stmt", "column_decl", "col_modifiers_opt",
  "col_modifiers", "col_modifier", "col_decl", "phys_enc_ref", "typed_col",
  "col_ident", "col_body_opt", "col_body", "col_stmt", "physmbr_decl",
  "phys_coldef", "col_schema_parms_opt", "col_schema_parms",
  "col_schema_parm", "col_schema_value", "database", "dbdad_opt", "dbbody",
  "db_members", "db_member", "template_opt", "view_spec", "view_parms",
  "include", "cond_expr", "cond_chain", "expr", "func_expr",
  "schema_parts_opt", "schema_parms", "schema_parm", "factory_parms_opt",
  "factory_parms_list", "factory_parms", "func_parms_opt", "expr_list",
  "uint_expr", "float_expr", "string_expr", "const_vect_expr", "bool_expr",
  "negate_expr", "cast_expr", "type_expr", "member_expr", "join_expr",
  "fqn", "qualnames", "ident", "fqn_opt_vers", "fqn_vers", "view",
  "view_parm", "view_body_opt", "view_body", "view_member",
  "view_parents_opt", "view_parents", "view_parent", "view_parent_parms", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-676)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      92,   138,   280,    84,  -676,    12,   183,   347,  -676,   238,
     208,   217,   221,   246,  -676,    15,    13,   266,  -676,   328,
     356,   396,   273,   275,  -676,   282,  -676,   380,   417,   298,
     301,   302,   303,   304,   305,   306,   308,   310,   311,   313,
     314,   315,   316,  -676,   240,  -676,  -676,  -676,  -676,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,   396,  -676,   297,   317,   404,   409,   412,   429,   439,
     443,   442,   440,   434,   426,   441,   436,   450,   433,  -676,
    -676,   294,   346,   351,   476,   430,   430,   430,     7,   430,
     421,   299,   117,   422,   257,   373,   373,   373,  -676,  -676,
    -676,   361,   364,   451,   438,   -12,   367,   430,  -676,  -676,
     430,   369,   371,   374,   375,   377,   437,   379,   381,   383,
    -676,   384,    20,   389,   161,   101,   370,  -676,   454,   390,
     391,   392,   393,   430,   368,   394,  -676,   509,   515,   257,
    -676,   398,   257,  -676,   505,   395,   401,   402,  -676,   373,
     430,  -676,   403,   445,  -676,   407,   431,   487,   408,   111,
       7,   405,  -676,   410,   406,    32,   414,   430,   430,   470,
    -676,   470,   477,   500,   -15,   538,     2,   544,   413,   424,
     416,   427,   420,   428,   552,  -676,   502,   148,  -676,   513,
    -676,   206,  -676,  -676,  -676,  -676,  -676,  -676,  -676,   432,
     435,   444,   446,   448,   449,   452,   453,  -676,   455,  -676,
    -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,  -676,
    -676,   458,   459,    49,    49,   460,   461,   462,   464,  -676,
     466,   470,  -676,   465,   478,   467,   516,   447,    49,   463,
    -676,     1,   468,   471,   472,     7,   473,  -676,   475,  -676,
     376,   474,   482,   479,   423,   425,   512,   512,   484,   485,
     488,  -676,     2,  -676,     2,   257,   494,  -676,   430,  -676,
     489,   481,   492,   493,  -676,   495,   523,   496,   -29,   498,
     499,   254,  -676,  -676,  -676,   567,   501,  -676,     7,   503,
     504,   506,   507,   525,    49,    63,   526,    32,    32,     5,
     497,  -676,   508,   510,   527,   527,   470,   514,   255,  -676,
     491,  -676,   239,   173,  -676,    49,   517,   469,  -676,   518,
     511,   571,    49,   519,     1,   520,  -676,  -676,   259,  -676,
    -676,  -676,  -676,   522,   152,   524,   528,   530,   531,   532,
     512,   534,  -676,  -676,   529,   539,   547,    49,    21,   533,
     494,   430,  -676,   535,   536,   558,   521,   262,  -676,   179,
     537,   223,  -676,   540,   512,   -23,  -676,  -676,  -676,   541,
       7,    -6,  -676,   542,   543,  -676,    32,    32,  -676,  -676,
    -676,   545,   548,   546,   550,   553,   551,  -676,   555,   556,
       2,   512,   512,   557,  -676,   559,   -21,   560,   562,   563,
     564,    49,   566,   568,   569,   570,   572,   573,  -676,  -676,
     132,  -676,  -676,   574,   576,   577,   578,  -676,   165,  -676,
    -676,   579,  -676,   581,   486,   582,  -676,  -676,   263,  -676,
    -676,  -676,   490,    17,   583,  -676,   270,  -676,    32,     9,
     239,   204,  -676,   549,  -676,   527,   584,   585,  -676,  -676,
     587,   586,    26,  -676,   561,   575,  -676,  -676,    63,   607,
     203,   591,   312,   632,  -676,  -676,  -676,   142,   142,   636,
     580,  -676,  -676,  -676,   565,   177,   588,   589,    -6,   599,
     592,  -676,   590,  -676,   554,    32,   110,  -676,   593,   512,
     594,   595,   606,   430,   539,  -676,  -676,   598,   168,  -676,
     597,   596,   586,   602,   604,   -36,  -676,   609,   512,   512,
    -676,   605,   601,   160,   619,    14,   603,  -676,  -676,   639,
     620,   457,   610,   617,   608,  -676,   611,   612,  -676,  -676,
     614,    32,   615,   616,  -676,  -676,    32,  -676,   621,   622,
    -676,   618,   613,   624,   626,   278,  -676,   627,   628,   586,
     168,  -676,    24,   625,  -676,   629,   623,   630,   296,   634,
     635,   640,   631,   637,   642,   619,   643,   430,    49,    49,
     638,   512,  -676,   641,    98,   644,  -676,    32,   279,  -676,
    -676,  -676,  -676,  -676,   300,   645,   646,    -2,  -676,   598,
    -676,   671,   168,   647,   648,   649,   650,   651,   652,   300,
     534,   534,  -676,  -676,  -676,   271,  -676,    45,  -676,   653,
      32,    16,  -676,   654,  -676,   656,   512,   512,    49,   633,
     655,   658,   659,  -676,    88,  -676,  -676,  -676,   283,  -676,
     660,   -20,  -676,  -676,  -676,   662,     7,   665,  -676,   663,
     664,  -676,   430,   661,   666,  -676,  -676,   109,   667,   668,
     670,  -676,   672,   128,   128,    32,   673,   678,   674,   689,
    -676,   675,   676,   677,   600,   679,   681,   286,  -676,     7,
     686,  -676,  -676,   657,    32,   697,   669,  -676,  -676,   705,
     682,  -676,  -676,   684,   680,   683,   685,   687,   688,  -676,
     -27,   691,   692,   693,   694,   695,   698,   724,   131,   100,
     696,   699,   700,   703,  -676,    49,   701,   655,   512,     7,
     702,  -676,   534,   704,   706,  -676,   707,   710,   713,   708,
    -676,  -676,   734,   719,   152,    77,    77,  -676,  -676,   709,
    -676,   198,  -676,   711,    49,   712,   534,   714,  -676,  -676,
     716,   715,  -676,  -676,   717,   512,  -676,   718,  -676,  -676,
     720,   722,   723,   726,   728,   729,   730,  -676,   232,  -676,
     725,  -676,   731,   534,   727,   152,  -676,   733,  -676,     1,
     737,   534,   732,   735,  -676,   300,   300,  -676,    -6,  -676,
    -676,   738,   202,   534,  -676,   736,   750,   739,   741,   287,
     743,   742,   534,  -676,   113,   135,   290,  -676,   745,  -676,
     721,   744,   746,   748,  -676,  -676,   740,   512,   749,   747,
     751,   752,   753,   104,   754,   755,  -676,   119,   756,  -676,
     291,  -676,   757,   760,   761,    49,   763,  -676,    67,  -676,
    -676,  -676,   765,   -10,  -676,  -676,  -676,  -676,  -676,   758,
     512,  -676,   766,   767,   152,   258,  -676,  -676,   430,   759,
    -676,   770,  -676,  -676,  -676,   771,   764,   768,   769,   772,
     773,  -676,  -676,   775,   534,   534,   200,  -676,   774,   777,
     778,   781,   782,   783,   784,  -676,  -676,  -676,  -676
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     0,     0,     0,     2,     0,
       0,     0,     0,     0,    10,     0,     0,     0,     3,     0,
       0,     0,     0,     0,    12,     0,     4,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,     0,    13,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
       5,     0,     6,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     9,
      14,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    11,     7,
       8,     0,     0,     0,     0,     0,     0,     0,    34,    33,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      49,     0,     0,     0,     0,     0,     0,   172,     0,     0,
       0,     0,     0,     0,     0,     0,    82,     0,     0,     0,
      45,     0,     0,    83,     0,     0,     0,     0,    88,     0,
       0,    99,     0,     0,   156,     0,     0,     0,     0,     0,
       0,     0,    41,     0,     0,     0,     0,     0,     0,     0,
      84,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   225,     0,     0,    31,     0,
      42,     0,   210,   211,   212,   213,   177,   216,   215,     0,
       0,     0,     0,     0,     0,     0,     0,   178,     0,   179,
     180,   181,   182,   183,   184,   185,   187,   188,   189,   176,
      44,     0,     0,     0,     0,     0,     0,     0,     0,   219,
       0,     0,    57,     0,     0,     0,     0,     0,     0,     0,
     155,     0,     0,     0,     0,     0,     0,    37,     0,    36,
       0,   191,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   229,     0,   230,     0,     0,     0,    55,     0,    56,
       0,     0,     0,     0,   231,     0,   103,     0,     0,     0,
       0,     0,   170,   233,   226,     0,     0,    32,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    43,     0,     0,     0,     0,     0,     0,     0,    51,
       0,    89,     0,     0,    87,     0,     0,     0,   157,     0,
       0,     0,     0,     0,     0,     0,    30,    39,     0,    35,
     207,   209,   208,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    46,    47,     0,     0,     0,     0,     0,     0,
       0,     0,    60,     0,     0,     0,     0,     0,   101,     0,
       0,     0,   159,     0,     0,     0,   171,   234,   227,     0,
       0,     0,   198,     0,     0,   199,     0,     0,   217,   186,
     222,     0,     0,     0,     0,     0,     0,    74,     0,     0,
       0,     0,     0,     0,    52,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   115,   107,
       0,   105,   108,     0,     0,     0,     0,   161,     0,   160,
     228,     0,   243,     0,     0,     0,    40,   196,     0,   193,
     197,   195,     0,     0,     0,   205,     0,   221,     0,     0,
       0,     0,    48,     0,    85,     0,     0,     0,    50,   220,
       0,     0,     0,    58,     0,     0,   100,   102,     0,     0,
       0,     0,     0,     0,   104,   106,    98,     0,     0,     0,
       0,   163,   162,   235,     0,     0,     0,     0,     0,     0,
       0,   203,     0,   218,     0,     0,     0,   174,     0,     0,
       0,     0,     0,     0,     0,    53,    54,     0,     0,    70,
       0,     0,     0,     0,     0,     0,    92,     0,     0,     0,
     117,     0,     0,     0,     0,     0,     0,   168,   167,     0,
       0,     0,     0,     0,     0,   236,     0,     0,   192,   194,
       0,     0,     0,     0,   206,   173,     0,   224,     0,     0,
      72,     0,     0,     0,     0,     0,    66,     0,     0,     0,
       0,    61,     0,     0,    93,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   158,     0,     0,     0,    38,     0,     0,   190,
     214,   175,   223,    59,     0,     0,     0,     0,    65,     0,
      62,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   121,   122,   123,     0,   119,     0,   116,     0,
       0,     0,   112,     0,   111,     0,     0,     0,     0,     0,
       0,     0,     0,   242,     0,   238,   232,   201,     0,   204,
       0,     0,    76,    79,    73,     0,     0,     0,    67,     0,
       0,    63,     0,     0,     0,    94,    91,     0,     0,     0,
       0,   120,     0,     0,     0,     0,     0,     0,     0,     0,
     113,     0,     0,     0,     0,     0,     0,     0,   245,     0,
       0,   237,   239,     0,     0,     0,     0,    77,    86,     0,
       0,    71,    64,     0,     0,     0,     0,     0,     0,   118,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   166,     0,     0,     0,     0,     0,
       0,   202,     0,     0,     0,    68,     0,     0,     0,     0,
      80,    81,     0,     0,     0,     0,     0,   124,   125,     0,
     109,     0,   147,     0,     0,     0,     0,     0,   164,   165,
       0,     0,   244,   246,     0,     0,   200,     0,    75,    69,
       0,     0,     0,     0,     0,     0,     0,   133,     0,   132,
       0,   110,     0,     0,     0,     0,   142,     0,   114,     0,
       0,     0,     0,     0,    97,     0,     0,    90,     0,   128,
     127,     0,     0,     0,   143,     0,     0,     0,     0,     0,
       0,     0,     0,    78,     0,     0,     0,   131,     0,   134,
       0,     0,     0,     0,   146,   144,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   145,     0,     0,   248,
       0,   240,     0,     0,     0,     0,     0,   138,     0,   136,
     129,   130,     0,     0,   149,   152,   154,   153,   169,     0,
       0,   241,     0,     0,     0,     0,   135,   137,     0,     0,
     150,     0,   249,    95,    96,     0,     0,     0,     0,     0,
       0,   247,   126,     0,     0,     0,     0,   148,     0,     0,
       0,     0,     0,     0,     0,   151,   141,   139,   140
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -676,  -676,  -676,  -676,  -676,   797,  -676,   790,    30,  -676,
    -676,   -83,  -362,  -676,  -676,  -676,  -676,  -676,  -676,   801,
    -676,  -106,  -676,   329,  -143,  -232,  -284,   320,  -404,  -676,
     235,  -461,   331,   690,  -583,  -613,  -266,  -676,  -676,   776,
    -676,  -676,  -676,  -676,  -676,   330,  -676,  -676,  -676,  -676,
    -676,   483,  -676,  -676,  -676,   248,  -676,  -676,   191,   130,
    -676,  -676,    29,  -458,   201,  -676,  -676,    39,    -3,  -676,
    -676,  -676,  -676,   762,   480,  -676,   136,  -676,  -586,  -676,
    -162,  -676,  -676,   147,   456,  -675,   205,  -676,  -676,   399,
    -361,  -676,  -676,  -676,  -676,  -676,  -676,  -282,  -676,  -676,
     -85,  -676,  -245,  -207,   -89,  -676,   779,  -676,  -676,   307,
    -676,  -676,   207,  -676
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     7,    15,    16,    17,    25,    44,    45,    46,
     187,   229,   108,   248,    47,   328,    48,    49,    50,    51,
     115,   122,   308,   309,   148,   234,   345,   354,   452,   545,
     546,   503,   386,   387,   631,   632,   633,    52,    53,   118,
      54,    55,   149,   273,   505,   506,    56,   153,   357,   317,
     410,   411,   412,   512,   605,   606,   560,   654,   693,   758,
     800,   828,   829,   564,   659,   734,   833,   834,   835,    57,
     156,   182,   418,   419,   519,   571,   281,    58,   383,   486,
     435,   209,   294,   428,   429,   374,   375,   628,   482,   436,
     210,   211,   212,   213,   214,   215,   216,   230,   217,   218,
     109,   159,   282,   262,   263,    59,   283,   526,   624,   625,
     424,   667,   668,   820
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     103,   104,   105,   208,   110,   107,   124,   125,   126,   427,
     430,   299,   300,   335,   648,   649,   647,   264,   677,   339,
     132,   346,   134,   488,   636,   135,   223,   319,   224,   249,
     304,   277,   305,   169,   677,   450,   171,   547,   102,   227,
     657,   192,   193,   194,   195,   594,   196,   498,   164,   756,
     102,   565,   595,   102,   596,   102,   422,   566,   199,   102,
     176,   504,   199,   102,   197,   177,   392,   279,   391,   630,
     402,   403,   102,   480,    80,    23,    13,   188,   198,   366,
     219,   232,   221,   222,     5,   233,   102,   334,   270,   593,
     787,   757,    24,   409,    14,   381,   481,   102,   550,   123,
     199,   228,   146,   832,   553,   499,   219,   613,   358,   423,
     102,    80,   106,   500,   102,   364,   427,   430,   320,   421,
     676,   106,   722,   563,   451,   106,   747,   658,   102,   131,
     849,   640,   280,   200,   106,   337,   338,   228,   261,   261,
     390,   147,     6,   279,   409,   592,   446,   447,   106,     1,
     767,   116,     8,   261,   201,   202,   203,   204,   445,   306,
     340,   494,   287,   347,   489,   597,   205,   206,   501,   855,
     207,   502,   106,   652,   826,   123,   508,   785,   621,   228,
     154,   677,   677,   310,   102,   791,   517,   199,   621,   561,
     106,   622,   794,   795,   457,   117,   562,   801,   630,   402,
     403,   622,   630,   402,   403,   327,   809,   846,   827,   261,
     732,   826,   219,   219,   434,   192,   193,   194,   195,   155,
     196,   518,   402,   403,   630,   402,   403,   510,   671,   623,
     261,   372,   832,   524,   691,   404,   405,   261,   197,   623,
     151,   735,   406,   733,   538,   827,   407,   499,   736,   686,
     535,   185,   198,   810,   373,   500,   525,   692,   798,   511,
     152,   536,   261,   556,   557,   102,   395,   186,   199,   402,
     403,   102,   464,   408,   199,   811,   487,     3,   869,   870,
       4,   799,   404,   405,   414,   415,   431,   426,   244,   406,
     245,   219,   219,   407,   856,   650,   857,    11,    12,    13,
     602,   416,   603,    29,    30,   470,   471,   200,   858,    31,
     604,    32,    33,    34,    35,    36,   261,    14,   352,   355,
     408,   356,     9,   534,   353,   602,   619,   603,   201,   202,
     203,   204,    37,    38,    39,   604,   120,   121,    40,   762,
     205,   206,   414,   415,   207,   491,   763,    10,    18,   514,
     492,   247,   493,   219,   515,    41,    19,    29,    30,   416,
      20,   616,   617,    31,   417,    32,    33,    34,    35,    36,
      42,   662,   663,   781,   581,   112,   113,   114,   782,    27,
      79,    43,   290,   291,   292,    21,    37,    38,    39,   630,
     402,   403,    40,   431,   323,   349,   324,   350,    63,   369,
     219,   370,   400,   477,   401,   478,    26,    28,   542,    41,
     484,   664,   485,    60,    61,   627,   427,   430,   588,   629,
     589,   485,    62,   673,    42,   674,   706,   806,   707,   324,
     812,   839,   478,   840,    98,    43,    64,    65,    82,    84,
      66,    67,    68,    69,    70,    71,   219,    72,   656,    73,
      74,   219,    75,    76,    77,    78,   836,    85,    83,    29,
      30,    86,    87,   744,    88,    31,    89,    32,    33,    34,
      35,    36,   836,    90,    91,    92,    93,    96,    94,    95,
     759,   759,   615,   261,   261,    97,    99,   101,    37,    38,
      39,   100,   219,   695,    40,   102,    36,   119,   741,   123,
     772,   127,   868,   128,   637,   836,   133,   129,   130,   136,
     137,    41,   711,   138,   139,   117,   165,   140,   142,   157,
     158,   143,   144,   145,   653,   219,    42,   765,   150,   160,
     161,   167,   162,   261,   163,   166,   168,    43,   170,   172,
     174,   175,   178,   183,   173,   179,   180,   184,   181,   191,
     190,   189,   146,   679,   220,   225,   226,   683,   231,   235,
     240,   236,   819,   237,   238,   242,   239,   241,   243,   246,
     219,   250,   275,   272,   251,   307,   297,   298,   279,   316,
     325,   333,   336,   252,   363,   253,   708,   254,   255,   219,
     382,   256,   257,   276,   570,   852,   258,   259,   260,   265,
     266,   724,   267,   268,   271,   723,   269,   274,   284,   278,
     285,   344,   288,   286,   398,   513,   360,   399,   844,   289,
     261,   295,   296,   293,   301,   302,   745,   385,   303,   311,
     312,   313,   475,   314,   315,   388,   318,   321,   322,   479,
     341,   326,   450,   329,   330,   351,   331,   332,   342,   261,
     343,   362,   509,   348,   516,   530,   359,   361,   504,   521,
     368,   371,   541,   569,   376,   555,   568,   377,   384,   365,
     378,   379,   380,   573,   396,   639,   609,   413,   680,   394,
     420,   432,   433,   393,   544,   437,   397,   438,   425,   440,
     439,   442,   441,   431,   443,   492,   444,   448,   533,   449,
     453,   454,   455,   699,   456,   458,   559,   459,   460,   461,
     709,   462,   463,   523,   466,   467,   468,   469,   714,   473,
     474,   507,   476,   483,   495,   496,   497,   522,   563,   527,
     532,   531,   837,   537,   539,   540,   548,   712,   731,   528,
     261,   549,   551,   552,   558,   643,   644,   574,   837,   703,
     572,   567,   576,   577,   585,   579,   580,   584,   575,   740,
     490,   582,   583,   859,   586,   587,   751,   590,   599,   752,
     591,   600,   598,   607,   665,   608,   611,   618,   601,   610,
     620,   837,   612,   614,   626,   634,   635,   641,   666,   658,
     754,   645,   646,   790,   660,   661,   642,   669,   670,   675,
     684,   655,   678,   681,   682,   685,   803,   710,   687,   688,
     689,   690,    22,   698,   696,   700,   713,   701,   702,   704,
     705,   373,   715,   716,   638,   543,   717,   720,   721,   718,
     725,   726,   719,   727,   728,   554,   729,   737,   730,   738,
     739,   742,   746,   199,   748,   694,   749,   750,   753,   761,
     764,    81,   766,   651,   768,   769,   760,   847,   697,   773,
     774,   775,   776,   871,   770,   771,   777,   778,   814,   779,
     780,   784,   850,   783,   788,   793,   786,   802,   797,   804,
     792,   805,   807,   808,   813,   815,   816,   817,   822,   821,
     818,   111,   141,   465,   830,   831,   838,   841,   823,   824,
     842,   843,   845,   825,   848,   789,   853,   854,   851,   860,
     861,   862,   863,   867,   743,   872,   864,   865,   873,   874,
     866,   875,   876,   877,   878,   796,     0,     0,   755,     0,
     578,   672,     0,     0,   529,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   520,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   389,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   367,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     472
};

static const yytype_int16 yycheck[] =
{
      85,    86,    87,   165,    89,    88,    95,    96,    97,   371,
     371,   256,   257,   295,   600,   601,   599,   224,   631,    14,
     105,   305,   107,    14,    26,   110,   169,    56,   171,   191,
     262,   238,   264,   139,   647,    56,   142,   498,    65,    54,
      24,     9,    10,    11,    12,    21,    14,   451,   133,   724,
      65,    37,    28,    65,    30,    65,    79,   515,    68,    65,
     149,    97,    68,    65,    32,   150,   348,    66,    47,    89,
      90,    91,    65,    56,    44,    62,    61,   160,    46,   324,
     165,    79,   167,   168,     0,    83,    65,   294,   231,   550,
     765,    14,    79,   359,    79,   340,    79,    65,   502,   126,
      68,   116,    82,   113,   140,    79,   191,   565,   315,   132,
      65,    81,   127,    87,    65,   322,   478,   478,   147,   364,
     140,   127,   149,   109,   145,   127,   712,   111,    65,   141,
     140,   592,   131,   101,   127,   297,   298,   116,   223,   224,
     347,   121,    58,    66,   410,   549,   391,   392,   127,    57,
     736,    34,   140,   238,   122,   123,   124,   125,   390,   265,
     155,   445,   245,   306,   155,   141,   134,   135,   142,   844,
     138,   145,   127,   128,   107,   126,   458,   763,    90,   116,
      79,   794,   795,   268,    65,   771,    44,    68,    90,    29,
     127,   103,   775,   776,   401,    78,    36,   783,    89,    90,
      91,   103,    89,    90,    91,   288,   792,   140,   141,   294,
      79,   107,   297,   298,   376,     9,    10,    11,    12,   118,
      14,    79,    90,    91,    89,    90,    91,    24,   140,   141,
     315,    79,   113,    56,   106,   103,   104,   322,    32,   141,
      79,   141,   110,   112,   489,   141,   114,    79,   148,   140,
     140,   140,    46,   140,   102,    87,    79,   129,    56,    56,
      99,   151,   347,   508,   509,    65,   351,   156,    68,    90,
      91,    65,   140,   141,    68,   140,   438,   139,   864,   865,
       0,    79,   103,   104,   119,   120,   371,   370,   140,   110,
     142,   376,   377,   114,    36,    24,    38,    59,    60,    61,
      29,   136,    31,    63,    64,   140,   141,   101,    50,    69,
      39,    71,    72,    73,    74,    75,   401,    79,    79,   146,
     141,   148,   139,   485,    85,    29,   571,    31,   122,   123,
     124,   125,    92,    93,    94,    39,    79,    80,    98,   141,
     134,   135,   119,   120,   138,   141,   148,     0,   140,    37,
     146,   145,   148,   438,    42,   115,   139,    63,    64,   136,
     139,   568,   569,    69,   141,    71,    72,    73,    74,    75,
     130,   616,   617,   141,   536,    76,    77,    78,   146,    51,
     140,   141,     6,     7,     8,   139,    92,    93,    94,    89,
      90,    91,    98,   478,   140,   140,   142,   142,    18,   140,
     485,   142,   140,   140,   142,   142,   140,    51,   493,   115,
     140,   618,   142,   140,   139,   577,   778,   778,   140,   140,
     142,   142,   140,   140,   130,   142,   140,   140,   142,   142,
     140,   140,   142,   142,   140,   141,    19,   139,   141,    35,
     139,   139,   139,   139,   139,   139,   531,   139,   610,   139,
     139,   536,   139,   139,   139,   139,   817,    48,   141,    63,
      64,    49,    33,   708,    25,    69,    23,    71,    72,    73,
      74,    75,   833,    31,    34,    41,    50,    27,    37,    43,
     725,   726,   567,   568,   569,    52,   140,    11,    92,    93,
      94,   140,   577,   655,    98,    65,    75,    75,   705,   126,
     745,   140,   863,   139,   587,   866,   139,    56,    70,   140,
     139,   115,   674,   139,   139,    78,   148,   140,   139,   149,
      66,   140,   139,   139,   607,   610,   130,   734,   139,   139,
     139,    22,   140,   618,   141,   141,    21,   141,   140,    34,
     139,   139,   139,    56,   149,   100,   139,   139,   117,   143,
     140,   146,    82,   636,   140,    78,    56,   642,    20,    15,
     140,   148,   807,   139,   148,    13,   139,   139,    66,    56,
     655,   139,    56,    95,   139,    81,   153,   152,    66,    56,
      13,    56,    56,   139,    13,   139,   669,   139,   139,   674,
      56,   139,   139,   146,   137,   840,   141,   139,   139,   139,
     139,   690,   140,   139,   139,   690,   140,   140,   140,   146,
     139,    84,   139,   141,    56,    24,   147,    96,   825,   144,
     705,   139,   143,   149,   140,   140,   709,    88,   140,   140,
     149,   139,   146,   140,   139,    88,   140,   139,   139,   149,
     143,   140,    56,   140,   140,   154,   140,   140,   140,   734,
     140,   140,    45,   139,    22,    56,   139,   139,    97,    23,
     140,   139,    56,    43,   140,    56,    27,   139,   139,   150,
     140,   140,   140,    56,   139,     4,    36,   140,    13,   350,
     140,   139,   139,   150,    86,   140,   150,   139,   147,   139,
     144,   140,   139,   778,   139,   146,   140,   140,   144,   140,
     140,   139,   139,    14,   140,   139,   105,   139,   139,   139,
      24,   139,   139,   148,   140,   139,   139,   139,    13,   140,
     139,   146,   140,   140,   140,   140,   139,   147,   109,   141,
     140,   139,   817,   140,   140,   140,   139,    40,    14,   150,
     825,   145,   140,   139,   139,    96,    96,   139,   833,   149,
     140,   148,   140,   139,   141,   140,   140,   139,   147,    56,
     440,   140,   140,   848,   140,   139,    56,   140,   139,    56,
     142,   148,   147,   139,   141,   140,   139,   139,   148,   148,
     139,   866,   140,   140,   140,   140,   140,   140,   133,   111,
      56,   140,   140,    56,   140,   139,   148,   139,   139,   139,
     139,   148,   140,   140,   140,   139,    56,   150,   141,   141,
     140,   139,    15,   139,   141,   140,   147,   141,   141,   140,
     139,   102,   140,   139,   589,   494,   146,   140,   140,   146,
     139,   139,   147,   140,   140,   505,   141,   141,   140,   140,
     140,   140,   140,    68,   140,   654,   140,   140,   140,   140,
     139,    61,   140,   605,   140,   139,   726,   828,   657,   141,
     140,   139,   139,   866,   149,   148,   140,   139,   147,   140,
     140,   140,   833,   148,   141,   140,   149,   141,   140,   140,
     148,   140,   139,   141,   139,   141,   140,   139,   141,   140,
     150,    90,   116,   410,   140,   140,   140,   140,   147,   147,
     140,   140,   139,   150,   139,   769,   140,   140,   150,   150,
     140,   140,   148,   140,   707,   141,   148,   148,   141,   141,
     148,   140,   140,   140,   140,   778,    -1,    -1,   723,    -1,
     531,   624,    -1,    -1,   478,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   468,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   346,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   324,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     418
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    57,   158,   139,     0,     0,    58,   159,   140,   139,
       0,    59,    60,    61,    79,   160,   161,   162,   140,   139,
     139,   139,   162,    62,    79,   163,   140,    51,    51,    63,
      64,    69,    71,    72,    73,    74,    75,    92,    93,    94,
      98,   115,   130,   141,   164,   165,   166,   171,   173,   174,
     175,   176,   194,   195,   197,   198,   203,   226,   234,   262,
     140,   139,   140,    18,    19,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   140,
     165,   164,   141,   141,    35,    48,    49,    33,    25,    23,
      31,    34,    41,    50,    37,    43,    27,    52,   140,   140,
     140,    11,    65,   257,   257,   257,   127,   168,   169,   257,
     257,   176,    76,    77,    78,   177,    34,    78,   196,    75,
      79,    80,   178,   126,   261,   261,   261,   140,   139,    56,
      70,   141,   257,   139,   257,   257,   140,   139,   139,   139,
     140,   196,   139,   140,   139,   139,    82,   121,   181,   199,
     139,    79,    99,   204,    79,   118,   227,   149,    66,   258,
     139,   139,   140,   141,   257,   148,   141,    22,    21,   178,
     140,   178,    34,   149,   139,   139,   261,   257,   139,   100,
     139,   117,   228,    56,   139,   140,   156,   167,   168,   146,
     140,   143,     9,    10,    11,    12,    14,    32,    46,    68,
     101,   122,   123,   124,   125,   134,   135,   138,   237,   238,
     247,   248,   249,   250,   251,   252,   253,   255,   256,   257,
     140,   257,   257,   181,   181,    78,    56,    54,   116,   168,
     254,    20,    79,    83,   182,    15,   148,   139,   148,   139,
     140,   139,    13,    66,   140,   142,    56,   145,   170,   237,
     139,   139,   139,   139,   139,   139,   139,   139,   141,   139,
     139,   257,   260,   261,   260,   139,   139,   140,   139,   140,
     181,   139,    95,   200,   140,    56,   146,   260,   146,    66,
     131,   233,   259,   263,   140,   139,   141,   168,   139,   144,
       6,     7,     8,   149,   239,   139,   143,   153,   152,   259,
     259,   140,   140,   140,   182,   182,   178,    81,   179,   180,
     257,   140,   149,   139,   140,   139,    56,   206,   140,    56,
     147,   139,   139,   140,   142,    13,   140,   168,   172,   140,
     140,   140,   140,    56,   260,   254,    56,   237,   237,    14,
     155,   143,   140,   140,    84,   183,   183,   181,   139,   140,
     142,   154,    79,    85,   184,   146,   148,   205,   260,   139,
     147,   139,   140,    13,   260,   150,   259,   263,   140,   140,
     142,   139,    79,   102,   242,   243,   140,   139,   140,   140,
     140,   259,    56,   235,   139,    88,   189,   190,    88,   190,
     260,    47,   254,   150,   180,   257,   139,   150,    56,    96,
     140,   142,    90,    91,   103,   104,   110,   114,   141,   193,
     207,   208,   209,   140,   119,   120,   136,   141,   229,   230,
     140,   259,    79,   132,   267,   147,   168,   169,   240,   241,
     247,   257,   139,   139,   237,   237,   246,   140,   139,   144,
     139,   139,   140,   139,   140,   182,   259,   259,   140,   140,
      56,   145,   185,   140,   139,   139,   140,   260,   139,   139,
     139,   139,   139,   139,   140,   208,   140,   139,   139,   139,
     140,   141,   230,   140,   139,   146,   140,   140,   142,   149,
      56,    79,   245,   140,   140,   142,   236,   237,    14,   155,
     184,   141,   146,   148,   183,   140,   140,   139,   185,    79,
      87,   142,   145,   188,    97,   201,   202,   146,   254,    45,
      24,    56,   210,    24,    37,    42,    22,    44,    79,   231,
     231,    23,   147,   148,    56,    79,   264,   141,   150,   241,
      56,   139,   140,   144,   237,   140,   151,   140,   259,   140,
     140,    56,   257,   189,    86,   186,   187,   188,   139,   145,
     185,   140,   139,   140,   202,    56,   259,   259,   139,   105,
     213,    29,    36,   109,   220,    37,   220,   148,    27,    43,
     137,   232,   140,    56,   139,   147,   140,   139,   246,   140,
     140,   237,   140,   140,   139,   141,   140,   139,   140,   142,
     140,   142,   185,   188,    21,    28,    30,   141,   147,   139,
     148,   148,    29,    31,    39,   211,   212,   139,   140,    36,
     148,   139,   140,   220,   140,   257,   260,   260,   139,   259,
     139,    90,   103,   141,   265,   266,   140,   237,   244,   140,
      89,   191,   192,   193,   140,   140,    26,   168,   187,     4,
     188,   140,   148,    96,    96,   140,   140,   191,   235,   235,
      24,   212,   128,   168,   214,   148,   237,    24,   111,   221,
     140,   139,   259,   259,   260,   141,   133,   268,   269,   139,
     139,   140,   266,   140,   142,   139,   140,   192,   140,   168,
      13,   140,   140,   257,   139,   139,   140,   141,   141,   140,
     139,   106,   129,   215,   215,   237,   141,   221,   139,    14,
     140,   141,   141,   149,   140,   139,   140,   142,   168,    24,
     150,   237,    40,   147,    13,   140,   139,   146,   146,   147,
     140,   140,   149,   257,   261,   139,   139,   140,   140,   141,
     140,    14,    79,   112,   222,   141,   148,   141,   140,   140,
      56,   260,   140,   269,   259,   168,   140,   235,   140,   140,
     140,    56,    56,   140,    56,   243,   242,    14,   216,   259,
     216,   140,   141,   148,   139,   260,   140,   235,   140,   139,
     149,   148,   259,   141,   140,   139,   139,   140,   139,   140,
     140,   141,   146,   148,   140,   235,   149,   242,   141,   233,
      56,   235,   148,   140,   191,   191,   240,   140,    56,    79,
     217,   235,   141,    56,   140,   140,   140,   139,   141,   235,
     140,   140,   140,   139,   147,   141,   140,   139,   150,   259,
     270,   140,   141,   147,   147,   150,   107,   141,   218,   219,
     140,   140,   113,   223,   224,   225,   247,   257,   140,   140,
     142,   140,   140,   140,   260,   139,   140,   219,   139,   140,
     224,   150,   259,   140,   140,   242,    36,    38,    50,   257,
     150,   140,   140,   148,   148,   148,   148,   140,   247,   235,
     235,   225,   141,   141,   141,   140,   140,   140,   140
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   157,   158,   158,   159,   159,   159,   160,   161,   162,
     162,   163,   163,   164,   164,   165,   165,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   165,   165,
     166,   167,   167,   168,   168,   169,   170,   170,   171,   172,
     172,   173,   173,   174,   175,   176,   177,   177,   177,   178,
     178,   179,   179,   180,   180,   181,   181,   182,   182,   183,
     184,   184,   184,   184,   184,   185,   186,   186,   187,   187,
     188,   188,   189,   189,   189,   190,   191,   191,   192,   192,
     193,   193,   194,   195,   195,   196,   197,   198,   199,   199,
     200,   200,   201,   201,   202,   202,   202,   202,   203,   204,
     204,   205,   205,   206,   206,   207,   207,   208,   208,   208,
     208,   208,   208,   208,   208,   208,   209,   210,   210,   211,
     211,   212,   212,   212,   213,   213,   214,   214,   214,   215,
     215,   215,   216,   216,   217,   217,   218,   218,   219,   219,
     219,   219,   220,   220,   220,   220,   221,   222,   222,   223,
     223,   224,   224,   225,   225,   226,   227,   227,   228,   228,
     229,   229,   229,   229,   230,   230,   230,   231,   231,   232,
     233,   233,   234,   235,   236,   236,   237,   237,   237,   237,
     237,   237,   237,   237,   237,   237,   237,   237,   237,   237,
     238,   239,   239,   240,   240,   241,   241,   241,   242,   242,
     243,   244,   244,   245,   245,   246,   246,   247,   247,   247,
     248,   248,   249,   249,   250,   251,   251,   252,   253,   254,
     254,   255,   255,   256,   256,   257,   258,   258,   259,   260,
     260,   261,   262,   233,   233,   263,   264,   264,   265,   265,
     266,   266,   266,   267,   267,   268,   268,   269,   270,   270
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     4,     5,     4,     5,     5,     6,     6,     4,
       1,     4,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
      10,     1,     3,     1,     1,     7,     1,     1,    15,     1,
       3,     6,     7,     9,     7,     5,     7,     7,     9,     1,
       9,     1,     3,     5,     5,     4,     4,     1,     6,     6,
       1,     5,     6,     7,     8,     4,     1,     3,     5,     6,
       1,     5,     4,     6,     1,     9,     1,     2,     6,     1,
       8,     8,     5,     5,     6,     9,    17,     9,     1,     5,
      13,     9,     1,     2,     4,    13,    13,     8,    12,     1,
       8,     1,     3,     0,     4,     1,     2,     1,     1,     8,
       9,     5,     5,     6,     9,     1,     5,     1,     5,     1,
       2,     1,     1,     1,     5,     5,    11,     5,     5,     7,
       7,     5,     1,     1,     1,     4,     1,     2,     1,     7,
       7,     7,     6,     7,     8,     9,     6,     1,     9,     1,
       2,     6,     1,     1,     1,     7,     1,     5,     9,     5,
       1,     1,     2,     2,     8,     8,     7,     1,     1,    10,
       1,     3,     5,     4,     1,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     5,     1,     1,     1,
       9,     0,     6,     1,     3,     1,     1,     1,     1,     1,
       9,     1,     3,     1,     4,     1,     3,     4,     4,     4,
       1,     1,     1,     1,     9,     1,     1,     5,     7,     1,
       6,     6,     5,     9,     8,     4,     4,     6,     4,     1,
       1,     5,    15,     1,     3,     5,     1,     4,     1,     2,
       8,     9,     1,     1,     8,     1,     3,    10,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (ctx, p_ast, p_builder, p_sb, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, ctx, p_ast, p_builder, p_sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (ctx);
  YY_USE (p_ast);
  YY_USE (p_builder);
  YY_USE (p_sb);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, ctx, p_ast, p_builder, p_sb);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], ctx, p_ast, p_builder, p_sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, ctx, p_ast, p_builder, p_sb); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
  YY_USE (yyvaluep);
  YY_USE (ctx);
  YY_USE (p_ast);
  YY_USE (p_builder);
  YY_USE (p_sb);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (ctx_t ctx, AST*& p_ast, ASTBuilder& p_builder, ParseTreeScanner& p_sb)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, p_sb);
    }

  if (yychar <= END_SOURCE)
    {
      yychar = END_SOURCE;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* parse: PT_PARSE '(' "end of source" ')'  */
                                                    { p_ast = AST :: Make ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 3: /* parse: PT_PARSE '(' source "end of source" ')'  */
                                                    { p_ast = (yyvsp[-2].node); }
    break;

  case 4: /* source: PT_SOURCE '(' schema_1 ')'  */
                                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 5: /* source: PT_SOURCE '(' version_1 schema_1 ')'  */
                                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[-2].tok) ); }
    break;

  case 6: /* source: PT_SOURCE '(' version_2 schema_2 ')'  */
                                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[-2].tok) ); }
    break;

  case 7: /* version_1: PT_VERSION_1_0 '(' KW_version VERS_1_0 ';' ')'  */
                                                        { (yyval.tok) = (yyvsp[-5].tok); }
    break;

  case 8: /* version_2: PT_VERSION_2 '(' KW_version VERS_2_0 ';' ')'  */
                                                       { (yyval.tok) = (yyvsp[-5].tok); }
    break;

  case 9: /* schema_1: PT_SCHEMA_1_0 '(' schema_decls ')'  */
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-1].node) ); }
    break;

  case 10: /* schema_1: PT_EMPTY  */
                                                    { (yyval.node) = AST :: Make ( ctx, PT_SCHEMA_1_0 ); }
    break;

  case 11: /* schema_2: PT_SCHEMA_2_0 '(' schema_decls ')'  */
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-1].node) ); }
    break;

  case 12: /* schema_2: PT_EMPTY  */
                                                    { (yyval.node) = AST :: Make ( ctx, PT_SCHEMA_2_0 ); }
    break;

  case 13: /* schema_decls: schema_decl  */
                                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 14: /* schema_decls: schema_decls schema_decl  */
                                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 15: /* schema_decl: typedef  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 16: /* schema_decl: typeset  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 17: /* schema_decl: fmtdef  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 18: /* schema_decl: const  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 19: /* schema_decl: alias  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 20: /* schema_decl: function  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 21: /* schema_decl: extern_function  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 22: /* schema_decl: script  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 23: /* schema_decl: validate  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 24: /* schema_decl: physical  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 25: /* schema_decl: table  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 26: /* schema_decl: database  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 27: /* schema_decl: include  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 28: /* schema_decl: view  */
                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 29: /* schema_decl: ';'  */
                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 30: /* typedef: PT_TYPEDEF '(' KW_typedef fqn PT_ASTLIST '(' new_type_names ')' ';' ')'  */
                                            { (yyval.node) = p_builder . TypeDef ( ctx, (yyvsp[-9].tok), (yyvsp[-6].fqn), (yyvsp[-3].node) ); }
    break;

  case 31: /* new_type_names: typespec  */
                                       { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 32: /* new_type_names: new_type_names ',' typespec  */
                                       { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 33: /* typespec: fqn  */
                { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 34: /* typespec: arrayspec  */
                { (yyval.node) = (yyvsp[0].node); }
    break;

  case 35: /* arrayspec: PT_ARRAY '(' fqn '[' dim ']' ')'  */
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].fqn), (yyvsp[-2].node) ); }
    break;

  case 36: /* dim: expr  */
             { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 37: /* dim: '*'  */
             { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 38: /* typeset: PT_TYPESET '(' KW_typeset fqn PT_TYPESETDEF '(' '{' PT_ASTLIST '(' typeset_spec ')' '}' ')' ';' ')'  */
                { (yyval.node) = p_builder . TypeSet ( ctx, (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-5].node) ); }
    break;

  case 39: /* typeset_spec: typespec  */
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 40: /* typeset_spec: typeset_spec ',' typespec  */
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 41: /* fmtdef: PT_FORMAT '(' KW_fmtdef fqn ';' ')'  */
                                                { (yyval.node) = p_builder . FmtDef ( ctx, (yyvsp[-5].tok), (yyvsp[-2].fqn), 0 ); }
    break;

  case 42: /* fmtdef: PT_FORMAT '(' KW_fmtdef fqn fqn ';' ')'  */
                                                { (yyval.node) = p_builder . FmtDef ( ctx, (yyvsp[-6].tok), (yyvsp[-2].fqn), (yyvsp[-3].fqn) ); }
    break;

  case 43: /* const: PT_CONST '(' KW_const typespec fqn '=' expr ';' ')'  */
                                                                 { (yyval.node) = p_builder . ConstDef ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-2].expr) ); }
    break;

  case 44: /* alias: PT_ALIAS '(' KW_alias fqn fqn ';' ')'  */
                                             { (yyval.node) = p_builder . AliasDef ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].fqn) ); }
    break;

  case 45: /* function: PT_FUNCTION '(' KW_function func_decl ')'  */
                                                { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 46: /* func_decl: PT_UNTYPED '(' KW___untyped fqn '(' ')' ')'  */
                                                       { (yyval.node) = p_builder . UntypedFunctionDecl ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
    break;

  case 47: /* func_decl: PT_ROWLENGTH '(' KW___row_length fqn '(' ')' ')'  */
                                                       { (yyval.node) = p_builder . RowlenFunctionDecl ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn) ); }
    break;

  case 48: /* func_decl: PT_FUNCDECL '(' schema_sig_opt return_type fqn_opt_vers fact_sig param_sig prologue ')'  */
                { (yyval.node) = p_builder . FunctionDecl ( ctx, (yyvsp[-8].tok), false, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 49: /* schema_sig_opt: PT_EMPTY  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 50: /* schema_sig_opt: PT_SCHEMASIG '(' '<' PT_ASTLIST '(' schema_formals ')' '>' ')'  */
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 51: /* schema_formals: schema_formal  */
                                        { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 52: /* schema_formals: schema_formals ',' schema_formal  */
                                        { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 53: /* schema_formal: PT_SCHEMAFORMAL '(' KW_type ident ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-1].node) ); }
    break;

  case 54: /* schema_formal: PT_SCHEMAFORMAL '(' type_expr ident ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 55: /* return_type: PT_RETURNTYPE '(' KW_void ')'  */
                                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 56: /* return_type: PT_RETURNTYPE '(' type_expr ')'  */
                                        { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 57: /* fact_sig: PT_EMPTY  */
                                                 { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 58: /* fact_sig: PT_FACTSIG '(' '<' param_signature '>' ')'  */
                                                 { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 59: /* param_sig: PT_FUNCSIG '(' '(' param_signature ')' ')'  */
                                                  { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 60: /* param_signature: PT_EMPTY  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 61: /* param_signature: PT_FUNCPARAMS '(' formals_list vararg ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), AST :: Make ( ctx ), (yyvsp[-1].node) ); }
    break;

  case 62: /* param_signature: PT_FUNCPARAMS '(' '*' formals_list vararg ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), AST :: Make ( ctx ), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 63: /* param_signature: PT_FUNCPARAMS '(' formals_list '*' formals_list vararg ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 64: /* param_signature: PT_FUNCPARAMS '(' formals_list ',' '*' formals_list vararg ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 65: /* formals_list: PT_ASTLIST '(' formals ')'  */
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 66: /* formals: formal  */
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 67: /* formals: formals ',' formal  */
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 68: /* formal: PT_FORMALPARAM '(' typespec IDENTIFIER_1_0 ')'  */
                                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), AST :: Make ( ctx, (yyvsp[-1].tok) ), AST :: Make ( ctx ) ); }
    break;

  case 69: /* formal: PT_FORMALPARAM '(' KW_control typespec IDENTIFIER_1_0 ')'  */
                                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-2].node), AST :: Make ( ctx, (yyvsp[-1].tok) ), AST :: Make ( ctx, (yyvsp[-3].tok) ) ); }
    break;

  case 70: /* vararg: PT_EMPTY  */
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 71: /* vararg: PT_ELLIPSIS '(' ',' ELLIPSIS ')'  */
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok) ); }
    break;

  case 72: /* prologue: PT_FUNCPROLOGUE '(' ';' ')'  */
                                            { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 73: /* prologue: PT_FUNCPROLOGUE '(' '=' fqn ';' ')'  */
                                            { (yyval.node) = (yyvsp[-2].fqn); }
    break;

  case 75: /* script_prologue: PT_FUNCPROLOGUE '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')'  */
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 76: /* script_stmts: script_stmt  */
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 77: /* script_stmts: script_stmts script_stmt  */
                                { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 78: /* script_stmt: PT_RETURN '(' KW_return cond_expr ';' ')'  */
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-2].expr) ); }
    break;

  case 79: /* script_stmt: production  */
                                                    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 80: /* production: PT_PRODSTMT '(' type_expr ident '=' cond_expr ';' ')'  */
                                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
    break;

  case 81: /* production: PT_PRODTRIGGER '(' KW_trigger ident '=' cond_expr ';' ')'  */
                                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
    break;

  case 82: /* extern_function: PT_EXTERN '(' KW_extern function ')'  */
                                              { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 83: /* script: PT_SCHEMA '(' KW_schema script_decl ')'  */
                                                          { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 84: /* script: PT_SCHEMA '(' KW_schema KW_function script_decl ')'  */
                                                          { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 85: /* script_decl: PT_FUNCDECL '(' schema_sig_opt return_type fqn_opt_vers fact_sig param_sig script_prologue ')'  */
                { (yyval.node) = p_builder . FunctionDecl ( ctx, (yyvsp[-8].tok), true, (yyvsp[-6].node), (yyvsp[-5].node), (yyvsp[-4].fqn), (yyvsp[-3].node), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 86: /* validate: PT_VALIDATE '(' KW_validate PT_FUNCTION '(' KW_function PT_FUNCDECL '(' schema_sig_opt return_type fqn_opt_vers fact_sig param_sig prologue ')' ')' ')'  */
        { (yyval.node) = p_builder . FunctionDecl ( ctx, (yyvsp[-16].tok), false, (yyvsp[-8].node), (yyvsp[-7].node), (yyvsp[-6].fqn), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].node) ); }
    break;

  case 87: /* physical: PT_PHYSICAL '(' KW_physical schema_sig_opt phys_return_type fqn_vers fact_sig phys_prologue ')'  */
            { (yyval.node) = p_builder . PhysicalDecl ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 88: /* phys_return_type: return_type  */
                                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 89: /* phys_return_type: PT_NOHEADER '(' KW___no_header return_type ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-1].node) ); }
    break;

  case 90: /* phys_prologue: PT_PHYSPROLOGUE '(' '=' PT_PHYSSTMT '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')' ')'  */
                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok), (yyvsp[-4].node) ); }
    break;

  case 91: /* phys_prologue: PT_PHYSPROLOGUE '(' '{' PT_ASTLIST '(' phys_body ')' '}' ')'  */
                { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 92: /* phys_body: phys_body_stmt  */
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 93: /* phys_body: phys_body phys_body_stmt  */
                                { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 94: /* phys_body_stmt: PT_PHYSBODYSTMT '(' ';' ')'  */
        { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 95: /* phys_body_stmt: PT_PHYSBODYSTMT '(' KW_decode PT_PHYSSTMT '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-10].tok), (yyvsp[-4].node) ) ; }
    break;

  case 96: /* phys_body_stmt: PT_PHYSBODYSTMT '(' KW_encode PT_PHYSSTMT '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-10].tok), (yyvsp[-4].node) ); }
    break;

  case 97: /* phys_body_stmt: PT_PHYSBODYSTMT '(' KW___row_length '=' fqn '(' ')' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn) ); }
    break;

  case 98: /* table: PT_TABLE '(' KW_table fqn_vers parents_opt PT_TABLEBODY '(' '{' tbl_body '}' ')' ')'  */
                { (yyval.node) = p_builder . TableDef ( ctx, (yyvsp[-11].tok), (yyvsp[-8].fqn), (yyvsp[-7].node), (yyvsp[-3].node) ); }
    break;

  case 99: /* parents_opt: PT_EMPTY  */
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 100: /* parents_opt: PT_TABLEPARENTS '(' '=' PT_ASTLIST '(' tbl_parents ')' ')'  */
                                                                    { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 101: /* tbl_parents: fqn_opt_vers  */
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 102: /* tbl_parents: tbl_parents ',' fqn_opt_vers  */
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 103: /* tbl_body: %empty  */
                                    { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 104: /* tbl_body: PT_ASTLIST '(' tbl_stmts ')'  */
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 105: /* tbl_stmts: tbl_stmt  */
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 106: /* tbl_stmts: tbl_stmts tbl_stmt  */
                            { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 107: /* tbl_stmt: production  */
                                                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 108: /* tbl_stmt: column_decl  */
                                                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 109: /* tbl_stmt: PT_COLUMNEXPR '(' KW_column KW_limit '=' expr ';' ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-2].expr) ); }
    break;

  case 110: /* tbl_stmt: PT_COLUMNEXPR '(' KW_column KW_default KW_limit '=' expr ';' ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-2].expr) ); }
    break;

  case 111: /* tbl_stmt: PT_PHYSCOL '(' KW_static physmbr_decl ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-2].tok), (yyvsp[-1].node) ); }
    break;

  case 112: /* tbl_stmt: PT_PHYSCOL '(' KW_physical physmbr_decl ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-2].tok), (yyvsp[-1].node) ); }
    break;

  case 113: /* tbl_stmt: PT_PHYSCOL '(' KW_static KW_physical physmbr_decl ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok), (yyvsp[-1].node) ); }
    break;

  case 114: /* tbl_stmt: PT_COLUNTYPED '(' KW___untyped '=' fqn '(' ')' ';' ')'  */
                                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-4].fqn) ); }
    break;

  case 115: /* tbl_stmt: ';'  */
                                                                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 116: /* column_decl: PT_COLUMN '(' col_modifiers_opt col_decl ')'  */
                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 117: /* col_modifiers_opt: KW_column  */
                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 118: /* col_modifiers_opt: PT_ASTLIST '(' col_modifiers KW_column ')'  */
                                                    { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 119: /* col_modifiers: col_modifier  */
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 120: /* col_modifiers: col_modifiers col_modifier  */
                                    { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 121: /* col_modifier: KW_default  */
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 122: /* col_modifier: KW_extern  */
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 123: /* col_modifier: KW_readonly  */
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 124: /* col_decl: PT_COLDECL '(' typespec typed_col ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 125: /* col_decl: PT_COLDECL '(' phys_enc_ref typed_col ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 126: /* phys_enc_ref: PT_PHYSENCREF '(' '<' PT_ASTLIST '(' schema_parms ')' '>' fqn_opt_vers factory_parms_opt ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-10].tok), (yyvsp[-5].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 127: /* phys_enc_ref: PT_PHYSENCREF '(' fqn_vers factory_parms_opt ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 128: /* phys_enc_ref: PT_PHYSENCREF '(' fqn factory_parms_list ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 129: /* typed_col: PT_TYPEDCOL '(' col_ident '{' col_body_opt '}' ')'  */
                                                            {  (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].node) ); }
    break;

  case 130: /* typed_col: PT_TYPEDCOLEXPR '(' col_ident '=' cond_expr ';' ')'  */
                                                            {  (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
    break;

  case 131: /* typed_col: PT_TYPEDCOL '(' col_ident ';' ')'  */
                                                            {  (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].node) ); }
    break;

  case 132: /* col_ident: ident  */
                                { (yyval.node) = (yyvsp[0].node); }
    break;

  case 133: /* col_ident: PHYSICAL_IDENTIFIER_1_0  */
                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 134: /* col_body_opt: PT_EMPTY  */
                                    { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 135: /* col_body_opt: PT_ASTLIST '(' col_body ')'  */
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 136: /* col_body: col_stmt  */
                        { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 137: /* col_body: col_body col_stmt  */
                        { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 138: /* col_stmt: ';'  */
                                                        { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 139: /* col_stmt: PT_COLSTMT '(' KW_read '=' cond_expr ';' ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 140: /* col_stmt: PT_COLSTMT '(' KW_validate '=' cond_expr ';' ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 141: /* col_stmt: PT_COLSTMT '(' KW_limit '=' uint_expr ';' ')'  */
                                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 142: /* physmbr_decl: PT_PHYSMBR '(' phys_coldef PHYSICAL_IDENTIFIER_1_0 ';' ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].node), AST :: Make ( ctx, (yyvsp[-2].tok) ) ); }
    break;

  case 143: /* physmbr_decl: PT_PHYSMBR '(' KW_column phys_coldef PHYSICAL_IDENTIFIER_1_0 ';' ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-3].node), AST :: Make ( ctx, (yyvsp[-2].tok) ) ); }
    break;

  case 144: /* physmbr_decl: PT_PHYSMBR '(' phys_coldef PHYSICAL_IDENTIFIER_1_0 '=' cond_expr ';' ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 145: /* physmbr_decl: PT_PHYSMBR '(' KW_column phys_coldef PHYSICAL_IDENTIFIER_1_0 '=' cond_expr ';' ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), AST :: Make ( ctx, (yyvsp[-4].tok) ), (yyvsp[-2].expr) ); }
    break;

  case 146: /* phys_coldef: PT_PHYSCOLDEF '(' col_schema_parms_opt fqn_opt_vers factory_parms_opt ')'  */
                                                                                   { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].node), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 147: /* col_schema_parms_opt: PT_EMPTY  */
                                                                            { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 148: /* col_schema_parms_opt: PT_COLSCHEMAPARMS '(' '<' PT_ASTLIST '(' col_schema_parms ')' '>' ')'  */
                                                                            { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 149: /* col_schema_parms: col_schema_parm  */
                                        { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 150: /* col_schema_parms: col_schema_parms col_schema_parm  */
                                        { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 151: /* col_schema_parm: PT_COLSCHEMAPARAM '(' fqn '=' col_schema_value ')'  */
                                                            { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].node) ); }
    break;

  case 152: /* col_schema_parm: col_schema_value  */
                                                            { (yyval.node) = (yyvsp[0].node); }
    break;

  case 153: /* col_schema_value: fqn  */
                { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 154: /* col_schema_value: uint_expr  */
                { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 155: /* database: PT_DATABASE '(' KW_database fqn_vers dbdad_opt dbbody ')'  */
                                                                { (yyval.node) = p_builder . DatabaseDef ( ctx, (yyvsp[-6].tok), (yyvsp[-3].fqn), (yyvsp[-2].node), (yyvsp[-1].node) ); }
    break;

  case 156: /* dbdad_opt: PT_EMPTY  */
                                           { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 157: /* dbdad_opt: PT_DBDAD '(' '=' fqn_opt_vers ')'  */
                                           { (yyval.node) = (yyvsp[-1].fqn); }
    break;

  case 158: /* dbbody: PT_DBBODY '(' '{' PT_ASTLIST '(' db_members ')' '}' ')'  */
                                                                { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 159: /* dbbody: PT_DBBODY '(' '{' '}' ')'  */
                                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok) ); }
    break;

  case 160: /* db_members: db_member  */
                              { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 161: /* db_members: ';'  */
                              { (yyval.node) = AST :: Make ( ctx ); }
    break;

  case 162: /* db_members: db_members db_member  */
                              { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 163: /* db_members: db_members ';'  */
                              { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 164: /* db_member: PT_DBMEMBER '(' template_opt KW_database fqn_opt_vers ident ';' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].node) ); }
    break;

  case 165: /* db_member: PT_TBLMEMBER '(' template_opt KW_table fqn_opt_vers ident ';' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-3].fqn), (yyvsp[-2].node) ); }
    break;

  case 166: /* db_member: PT_ALIASMEMBER '(' KW_alias view_spec ident ';' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-6].tok), (yyvsp[-3].node), (yyvsp[-2].node) ); }
    break;

  case 167: /* template_opt: PT_EMPTY  */
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 168: /* template_opt: KW_template  */
                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 169: /* view_spec: PT_VIEWSPEC '(' fqn_opt_vers '<' PT_ASTLIST '(' view_parms ')' '>' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok) ); (yyval.node) -> AddNode ( ctx, (yyvsp[-7].fqn) ); (yyval.node) -> AddNode ( ctx, (yyvsp[-3].node) ); }
    break;

  case 170: /* view_parms: ident  */
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 171: /* view_parms: view_parms ',' ident  */
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 172: /* include: PT_INCLUDE '(' KW_include STRING ')'  */
                                           { (yyval.node) = p_builder . Include ( ctx, (yyvsp[-2].tok), (yyvsp[-1].tok) ); }
    break;

  case 173: /* cond_expr: PT_ASTLIST '(' cond_chain ')'  */
                                    { (yyval.expr) = (yyvsp[-1].expr); }
    break;

  case 174: /* cond_chain: expr  */
                             { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].expr) ); }
    break;

  case 175: /* cond_chain: cond_chain '|' expr  */
                             { (yyval.expr) = (yyvsp[-2].expr); (yyval.expr) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 176: /* expr: fqn  */
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].fqn) ); }
    break;

  case 177: /* expr: PHYSICAL_IDENTIFIER_1_0  */
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 178: /* expr: '@'  */
                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 179: /* expr: func_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 180: /* expr: uint_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 181: /* expr: float_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 182: /* expr: string_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 183: /* expr: const_vect_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 184: /* expr: bool_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 185: /* expr: negate_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 186: /* expr: PT_UNARYPLUS '(' '+' expr ')'  */
                                    { (yyval.expr) = (yyvsp[-1].expr); }
    break;

  case 187: /* expr: cast_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 188: /* expr: member_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 189: /* expr: join_expr  */
                                    { (yyval.expr) = (yyvsp[0].expr); }
    break;

  case 190: /* func_expr: PT_FUNCEXPR '(' schema_parts_opt fqn_opt_vers factory_parms_opt '(' func_parms_opt ')' ')'  */
        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-5].fqn) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-4].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-2].node) ); }
    break;

  case 191: /* schema_parts_opt: %empty  */
                                                { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 192: /* schema_parts_opt: '<' PT_ASTLIST '(' schema_parms ')' '>'  */
                                                { (yyval.node) = (yyvsp[-2].node); }
    break;

  case 193: /* schema_parms: schema_parm  */
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 194: /* schema_parms: schema_parms ',' schema_parm  */
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 195: /* schema_parm: fqn  */
                    { (yyval.node) = (yyvsp[0].fqn); }
    break;

  case 196: /* schema_parm: arrayspec  */
                    { (yyval.node) = (yyvsp[0].node); }
    break;

  case 197: /* schema_parm: uint_expr  */
                    { (yyval.node) = (yyvsp[0].expr); }
    break;

  case 198: /* factory_parms_opt: PT_EMPTY  */
                            { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 199: /* factory_parms_opt: factory_parms_list  */
                            { (yyval.node) = (yyvsp[0].node); }
    break;

  case 200: /* factory_parms_list: PT_FACTPARMS '(' '<' PT_ASTLIST '(' factory_parms ')' '>' ')'  */
                                                                        { (yyval.node) = (yyvsp[-3].node); }
    break;

  case 201: /* factory_parms: expr  */
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 202: /* factory_parms: factory_parms ',' expr  */
                                { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 203: /* func_parms_opt: PT_EMPTY  */
                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 204: /* func_parms_opt: PT_ASTLIST '(' expr_list ')'  */
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 205: /* expr_list: expr  */
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 206: /* expr_list: expr_list ',' expr  */
                            { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].expr) ); }
    break;

  case 207: /* uint_expr: PT_UINT '(' DECIMAL ')'  */
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 208: /* uint_expr: PT_UINT '(' HEX ')'  */
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 209: /* uint_expr: PT_UINT '(' OCTAL ')'  */
                                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 210: /* float_expr: FLOAT_  */
                 { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 211: /* float_expr: EXP_FLOAT  */
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 212: /* string_expr: STRING  */
                        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 213: /* string_expr: ESCAPED_STRING  */
                        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 214: /* const_vect_expr: PT_CONSTVECT '(' '[' PT_ASTLIST '(' expr_list ')' ']' ')'  */
                                                                    { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); }
    break;

  case 215: /* bool_expr: KW_true  */
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 216: /* bool_expr: KW_false  */
                { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 217: /* negate_expr: PT_NEGATE '(' '-' expr ')'  */
                                 { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-4].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].expr) ); }
    break;

  case 218: /* cast_expr: PT_CASTEXPR '(' '(' type_expr ')' expr ')'  */
                                                  { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-6].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].expr) ); }
    break;

  case 219: /* type_expr: typespec  */
                                        { (yyval.node) = (yyvsp[0].node); }
    break;

  case 220: /* type_expr: PT_TYPEEXPR '(' fqn '/' fqn ')'  */
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-5].tok), (yyvsp[-3].fqn), (yyvsp[-1].fqn) ); }
    break;

  case 221: /* member_expr: PT_MEMBEREXPR '(' ident '.' ident ')'  */
                                                            { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-5].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].node) ); }
    break;

  case 222: /* member_expr: PT_MEMBEREXPR '(' ident PHYSICAL_IDENTIFIER_1_0 ')'  */
        {   /* remove leading '.'*/
            (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-4].tok) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-2].node) );
            AST * ident = AST :: Make ( ctx, PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( ctx, & t );
            (yyval.expr) -> AddNode ( ctx, ident );
        }
    break;

  case 223: /* join_expr: PT_JOINEXPR '(' ident '[' cond_expr ']' '.' ident ')'  */
        { (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-8].tok) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-6].node) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-4].expr) ); (yyval.expr) -> AddNode ( ctx, (yyvsp[-1].node) ); }
    break;

  case 224: /* join_expr: PT_JOINEXPR '(' ident '[' cond_expr ']' PHYSICAL_IDENTIFIER_1_0 ')'  */
        {   /* remove leading '.'*/
            (yyval.expr) = AST_Expr :: Make ( ctx, (yyvsp[-7].tok) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-5].node) );
            (yyval.expr) -> AddNode ( ctx, (yyvsp[-3].expr) );
            AST * ident = AST :: Make ( ctx, PT_IDENT );
            Token t ( IDENTIFIER_1_0, (yyvsp[-1].tok) -> GetValue() + 1, (yyval.expr) -> GetLocation() );
            ident -> AddNode ( ctx, & t );
            (yyval.expr) -> AddNode ( ctx, ident );
        }
    break;

  case 225: /* fqn: PT_FQN '(' qualnames ')'  */
                                { (yyval.fqn) = (yyvsp[-1].fqn); }
    break;

  case 226: /* qualnames: PT_IDENT '(' IDENTIFIER_1_0 ')'  */
                                                        { (yyval.fqn) = AST_FQN :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 227: /* qualnames: qualnames ':' PT_IDENT '(' IDENTIFIER_1_0 ')'  */
                                                        { (yyval.fqn) = (yyvsp[-5].fqn); (yyval.fqn) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 228: /* ident: PT_IDENT '(' IDENTIFIER_1_0 ')'  */
                                        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-3].tok) ); (yyval.node) -> AddNode ( ctx, (yyvsp[-1].tok) ); }
    break;

  case 229: /* fqn_opt_vers: fqn  */
                { (yyval.fqn) = (yyvsp[0].fqn); }
    break;

  case 230: /* fqn_opt_vers: fqn_vers  */
                { (yyval.fqn) = (yyvsp[0].fqn); }
    break;

  case 231: /* fqn_vers: PT_VERSNAME '(' fqn VERSION ')'  */
                                        { (yyval.fqn) = (yyvsp[-2].fqn); (yyval.fqn) -> SetVersion ( (yyvsp[-1].tok) -> GetValue () ); }
    break;

  case 232: /* view: PT_VIEW '(' KW_view fqn_vers '<' PT_ASTLIST '(' view_parms ')' '>' view_parents_opt '{' view_body_opt '}' ')'  */
        { (yyval.node) = p_builder . ViewDef ( ctx, (yyvsp[-14].tok), (yyvsp[-11].fqn), (yyvsp[-7].node), (yyvsp[-4].node), (yyvsp[-2].node) ); }
    break;

  case 233: /* view_parms: view_parm  */
                                { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 234: /* view_parms: view_parms ',' view_parm  */
                                { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 235: /* view_parm: PT_VIEWPARAM '(' fqn_opt_vers ident ')'  */
                                                { (yyval.node) = AST :: Make ( ctx, (yyvsp[-4].tok), (yyvsp[-2].fqn), (yyvsp[-1].node) ); }
    break;

  case 236: /* view_body_opt: PT_EMPTY  */
                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 237: /* view_body_opt: PT_ASTLIST '(' view_body ')'  */
                                    { (yyval.node) = (yyvsp[-1].node); }
    break;

  case 238: /* view_body: view_member  */
                            { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 239: /* view_body: view_body view_member  */
                            { (yyval.node) = (yyvsp[-1].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 240: /* view_member: PT_PRODSTMT '(' typespec ident '=' cond_expr ';' ')'  */
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
    break;

  case 241: /* view_member: PT_COLUMN '(' KW_column typespec ident '=' cond_expr ';' ')'  */
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-8].tok), (yyvsp[-5].node), (yyvsp[-4].node), (yyvsp[-2].expr) ); }
    break;

  case 242: /* view_member: ';'  */
                                                                    { (yyval.node) = AST :: Make ( ctx, PT_EMPTY ); }
    break;

  case 243: /* view_parents_opt: PT_EMPTY  */
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[0].tok) ); }
    break;

  case 244: /* view_parents_opt: PT_VIEWPARENTS '(' '=' PT_ASTLIST '(' view_parents ')' ')'  */
                                                                    { (yyval.node) = AST :: Make ( ctx, (yyvsp[-7].tok), (yyvsp[-2].node) ); }
    break;

  case 245: /* view_parents: view_parent  */
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 246: /* view_parents: view_parents ',' view_parent  */
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 247: /* view_parent: PT_VIEWPARENT '(' fqn_opt_vers '<' PT_ASTLIST '(' view_parent_parms ')' '>' ')'  */
        { (yyval.node) = AST :: Make ( ctx, (yyvsp[-9].tok), (yyvsp[-7].fqn), (yyvsp[-3].node) ); }
    break;

  case 248: /* view_parent_parms: ident  */
                                    { (yyval.node) = AST :: Make ( ctx ); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;

  case 249: /* view_parent_parms: view_parent_parms ',' ident  */
                                    { (yyval.node) = (yyvsp[-2].node); (yyval.node) -> AddNode ( ctx, (yyvsp[0].node) ); }
    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (ctx, p_ast, p_builder, p_sb, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= END_SOURCE)
        {
          /* Return failure if at end of input.  */
          if (yychar == END_SOURCE)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, ctx, p_ast, p_builder, p_sb);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, ctx, p_ast, p_builder, p_sb);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (ctx, p_ast, p_builder, p_sb, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, ctx, p_ast, p_builder, p_sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, ctx, p_ast, p_builder, p_sb);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}



const char * AST_symbol_name( enum yytokentype t )
{
    return yytname[ t - 255 ];
}

