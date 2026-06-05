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

/* Substitute the type names.  */
#define YYSTYPE         SCHEMA_STYPE
#define YYLTYPE         SCHEMA_LTYPE
/* Substitute the variable and function names.  */
#define yyparse         Schema_parse
#define yylex           Schema_lex
#define yyerror         Schema_error
#define yydebug         Schema_debug
#define yynerrs         Schema_nerrs

/* First part of user prologue.  */

    #define YYDEBUG 1

    #include <stdio.h>

    #include "ParseTree.hpp"
    #include "ErrorReport.hpp"

    using namespace ncbi::SchemaParser;

    #include "schema-grammar.hpp"
    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    void Schema_error ( YYLTYPE *                   p_llocp,
                        ctx_t                       ctx,
                        ParseTree **                p_root,
                        ErrorReport *               p_errors,
                        struct SchemaScanBlock *    p_sb,
                        const char *                p_msg )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        /* send message to the C++ parser for proper display and recovery */
        Token :: Location loc ( p_sb -> file_name, p_llocp -> first_line, p_llocp -> first_column );
        p_errors -> ReportError ( ctx, loc, "%s", p_msg);
    }

    extern "C"
    {
        extern enum Schema_tokentype SchemaScan_yylex ( SCHEMA_STYPE *lvalp, SCHEMA_LTYPE *llocp, SchemaScanBlock* sb );
    }

    static
    ParseTree*
    P ( SchemaToken & p_prod )
    {
        assert ( p_prod . subtree );
        return ( ParseTree * ) p_prod . subtree;
    }

    static
    ParseTree*
    T ( ctx_t ctx, SchemaToken & p_term )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );//TODO: catch
        assert ( p_term . subtree == 0 );
        return ParseTree::Make ( ctx, p_term );
    }

    /* Create production node */
    static
    ParseTree *
    MakeTree ( ctx_t ctx,
               int p_token,
               ParseTree * p_ch1 = 0,
               ParseTree * p_ch2 = 0,
               ParseTree * p_ch3 = 0,
               ParseTree * p_ch4 = 0,
               ParseTree * p_ch5 = 0,
               ParseTree * p_ch6 = 0,
               ParseTree * p_ch7 = 0,
               ParseTree * p_ch8 = 0,
               ParseTree * p_ch9 = 0
             )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        ParseTree * ret = ParseTree :: Make ( ctx, Token ( p_token ) );
        if ( p_ch1 != 0 ) ret -> AddChild ( ctx, p_ch1 );
        if ( p_ch2 != 0 ) ret -> AddChild ( ctx, p_ch2 );
        if ( p_ch3 != 0 ) ret -> AddChild ( ctx, p_ch3 );
        if ( p_ch4 != 0 ) ret -> AddChild ( ctx, p_ch4 );
        if ( p_ch5 != 0 ) ret -> AddChild ( ctx, p_ch5 );
        if ( p_ch6 != 0 ) ret -> AddChild ( ctx, p_ch6 );
        if ( p_ch7 != 0 ) ret -> AddChild ( ctx, p_ch7 );
        if ( p_ch8 != 0 ) ret -> AddChild ( ctx, p_ch8 );
        if ( p_ch9 != 0 ) ret -> AddChild ( ctx, p_ch9 );
        return ret;
    }

    /* Create a flat list */
    static
    ParseTree *
    MakeList ( ctx_t ctx, SchemaToken & p_prod )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        ParseTree * ret = ParseTree :: Make ( ctx, Token ( PT_ASTLIST ) );
        ret -> AddChild ( ctx, P ( p_prod ) );
        return ret;
    }

    /* Add to a flat list node */
    static
    ParseTree *
    AddToList ( ctx_t ctx, ParseTree * p_root, ParseTree * p_br1, ParseTree * p_br2 = 0 )
    {
        FUNC_ENTRY( ctx, rcSRA, rcSchema, rcParsing );
        assert ( p_br1 != 0 );
        p_root -> AddChild ( ctx, p_br1 );
        if ( p_br2 != 0 )
        {
            p_root -> AddChild ( ctx, p_br2 );
        }
        return p_root;
    }



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

#include "zz_schema-grammar.hpp"
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
  YYSYMBOL_138_ = 138,                     /* ';'  */
  YYSYMBOL_139_ = 139,                     /* ','  */
  YYSYMBOL_140_ = 140,                     /* '{'  */
  YYSYMBOL_141_ = 141,                     /* '}'  */
  YYSYMBOL_142_ = 142,                     /* '['  */
  YYSYMBOL_143_ = 143,                     /* ']'  */
  YYSYMBOL_144_ = 144,                     /* '*'  */
  YYSYMBOL_145_ = 145,                     /* '='  */
  YYSYMBOL_146_ = 146,                     /* '('  */
  YYSYMBOL_147_ = 147,                     /* ')'  */
  YYSYMBOL_148_ = 148,                     /* '<'  */
  YYSYMBOL_149_ = 149,                     /* '>'  */
  YYSYMBOL_150_ = 150,                     /* '|'  */
  YYSYMBOL_151_ = 151,                     /* '@'  */
  YYSYMBOL_152_ = 152,                     /* '-'  */
  YYSYMBOL_153_ = 153,                     /* '+'  */
  YYSYMBOL_154_ = 154,                     /* '/'  */
  YYSYMBOL_155_ = 155,                     /* '.'  */
  YYSYMBOL_156_ = 156,                     /* ':'  */
  YYSYMBOL_YYACCEPT = 157,                 /* $accept  */
  YYSYMBOL_parse = 158,                    /* parse  */
  YYSYMBOL_source = 159,                   /* source  */
  YYSYMBOL_version_1_0 = 160,              /* version_1_0  */
  YYSYMBOL_schema_1_0_opt = 161,           /* schema_1_0_opt  */
  YYSYMBOL_schema_1_0 = 162,               /* schema_1_0  */
  YYSYMBOL_schema_1_0_decl = 163,          /* schema_1_0_decl  */
  YYSYMBOL_typedef_1_0_decl = 164,         /* typedef_1_0_decl  */
  YYSYMBOL_typedef_1_0_new_name_list = 165, /* typedef_1_0_new_name_list  */
  YYSYMBOL_typeset_1_0_decl = 166,         /* typeset_1_0_decl  */
  YYSYMBOL_typeset_1_0_new_name = 167,     /* typeset_1_0_new_name  */
  YYSYMBOL_typeset_1_0_def = 168,          /* typeset_1_0_def  */
  YYSYMBOL_typespec_1_0_list = 169,        /* typespec_1_0_list  */
  YYSYMBOL_typespec_1_0 = 170,             /* typespec_1_0  */
  YYSYMBOL_dim_1_0 = 171,                  /* dim_1_0  */
  YYSYMBOL_format_1_0_decl = 172,          /* format_1_0_decl  */
  YYSYMBOL_format_1_0_new_name = 173,      /* format_1_0_new_name  */
  YYSYMBOL_format_1_0_name = 174,          /* format_1_0_name  */
  YYSYMBOL_const_1_0_decl = 175,           /* const_1_0_decl  */
  YYSYMBOL_alias_1_0_decl = 176,           /* alias_1_0_decl  */
  YYSYMBOL_alias_1_0_new_name = 177,       /* alias_1_0_new_name  */
  YYSYMBOL_function_1_0_decl = 178,        /* function_1_0_decl  */
  YYSYMBOL_func_1_0_decl = 179,            /* func_1_0_decl  */
  YYSYMBOL_untyped_func_1_0_decl = 180,    /* untyped_func_1_0_decl  */
  YYSYMBOL_row_length_func_1_0_decl = 181, /* row_length_func_1_0_decl  */
  YYSYMBOL_opt_func_1_0_schema_sig = 182,  /* opt_func_1_0_schema_sig  */
  YYSYMBOL_func_1_0_schema_sig = 183,      /* func_1_0_schema_sig  */
  YYSYMBOL_func_1_0_schema_formals = 184,  /* func_1_0_schema_formals  */
  YYSYMBOL_func_1_0_schema_formal = 185,   /* func_1_0_schema_formal  */
  YYSYMBOL_func_1_0_return_type = 186,     /* func_1_0_return_type  */
  YYSYMBOL_opt_func_1_0_fact_sig = 187,    /* opt_func_1_0_fact_sig  */
  YYSYMBOL_func_1_0_fact_sig = 188,        /* func_1_0_fact_sig  */
  YYSYMBOL_func_1_0_fact_signature = 189,  /* func_1_0_fact_signature  */
  YYSYMBOL_func_1_0_fact_params = 190,     /* func_1_0_fact_params  */
  YYSYMBOL_fact_param_1_0 = 191,           /* fact_param_1_0  */
  YYSYMBOL_func_1_0_param_sig = 192,       /* func_1_0_param_sig  */
  YYSYMBOL_func_1_0_param_signature = 193, /* func_1_0_param_signature  */
  YYSYMBOL_func_1_0_formal_params = 194,   /* func_1_0_formal_params  */
  YYSYMBOL_formal_param_1_0 = 195,         /* formal_param_1_0  */
  YYSYMBOL_func_1_0_vararg_formals = 196,  /* func_1_0_vararg_formals  */
  YYSYMBOL_func_1_0_prologue = 197,        /* func_1_0_prologue  */
  YYSYMBOL_script_1_0_stmt_seq = 198,      /* script_1_0_stmt_seq  */
  YYSYMBOL_script_1_0_stmt = 199,          /* script_1_0_stmt  */
  YYSYMBOL_extern_1_0_decl = 200,          /* extern_1_0_decl  */
  YYSYMBOL_ext_func_1_0_decl = 201,        /* ext_func_1_0_decl  */
  YYSYMBOL_script_1_0_decl = 202,          /* script_1_0_decl  */
  YYSYMBOL_validate_1_0_decl = 203,        /* validate_1_0_decl  */
  YYSYMBOL_physical_1_0_decl = 204,        /* physical_1_0_decl  */
  YYSYMBOL_phys_1_0_return_type = 205,     /* phys_1_0_return_type  */
  YYSYMBOL_phys_1_0_prologue = 206,        /* phys_1_0_prologue  */
  YYSYMBOL_phys_1_0_body = 207,            /* phys_1_0_body  */
  YYSYMBOL_phys_1_0_body_stmt = 208,       /* phys_1_0_body_stmt  */
  YYSYMBOL_phys_1_0_stmt = 209,            /* phys_1_0_stmt  */
  YYSYMBOL_table_1_0_decl = 210,           /* table_1_0_decl  */
  YYSYMBOL_opt_tbl_1_0_parents = 211,      /* opt_tbl_1_0_parents  */
  YYSYMBOL_tbl_1_0_parents = 212,          /* tbl_1_0_parents  */
  YYSYMBOL_tbl_1_0_body = 213,             /* tbl_1_0_body  */
  YYSYMBOL_tbl_1_0_stmt_seq = 214,         /* tbl_1_0_stmt_seq  */
  YYSYMBOL_tbl_1_0_stmt = 215,             /* tbl_1_0_stmt  */
  YYSYMBOL_production_1_0_stmt = 216,      /* production_1_0_stmt  */
  YYSYMBOL_col_1_0_modifiers = 217,        /* col_1_0_modifiers  */
  YYSYMBOL_col_1_0_modifier_seq = 218,     /* col_1_0_modifier_seq  */
  YYSYMBOL_col_1_0_modifier = 219,         /* col_1_0_modifier  */
  YYSYMBOL_col_1_0_decl = 220,             /* col_1_0_decl  */
  YYSYMBOL_phys_enc_ref = 221,             /* phys_enc_ref  */
  YYSYMBOL_typed_column_decl_1_0 = 222,    /* typed_column_decl_1_0  */
  YYSYMBOL_col_ident = 223,                /* col_ident  */
  YYSYMBOL_phys_ident = 224,               /* phys_ident  */
  YYSYMBOL_opt_column_body_1_0 = 225,      /* opt_column_body_1_0  */
  YYSYMBOL_column_body_1_0 = 226,          /* column_body_1_0  */
  YYSYMBOL_column_stmt_1_0 = 227,          /* column_stmt_1_0  */
  YYSYMBOL_default_view_1_0_decl = 228,    /* default_view_1_0_decl  */
  YYSYMBOL_physmbr_1_0_decl = 229,         /* physmbr_1_0_decl  */
  YYSYMBOL_phys_coldef_1_0 = 230,          /* phys_coldef_1_0  */
  YYSYMBOL_opt_col_schema_parms_1_0 = 231, /* opt_col_schema_parms_1_0  */
  YYSYMBOL_col_schema_parms_1_0 = 232,     /* col_schema_parms_1_0  */
  YYSYMBOL_col_schema_parm_1_0 = 233,      /* col_schema_parm_1_0  */
  YYSYMBOL_col_schema_value_1_0 = 234,     /* col_schema_value_1_0  */
  YYSYMBOL_cond_expr_1_0 = 235,            /* cond_expr_1_0  */
  YYSYMBOL_expression_1_0 = 236,           /* expression_1_0  */
  YYSYMBOL_primary_expr_1_0 = 237,         /* primary_expr_1_0  */
  YYSYMBOL_func_expr_1_0 = 238,            /* func_expr_1_0  */
  YYSYMBOL_schema_parms_1_0 = 239,         /* schema_parms_1_0  */
  YYSYMBOL_schema_parm_1_0 = 240,          /* schema_parm_1_0  */
  YYSYMBOL_opt_factory_parms_1_0 = 241,    /* opt_factory_parms_1_0  */
  YYSYMBOL_factory_parms_1_0 = 242,        /* factory_parms_1_0  */
  YYSYMBOL_factory_parms = 243,            /* factory_parms  */
  YYSYMBOL_opt_func_1_0_parms = 244,       /* opt_func_1_0_parms  */
  YYSYMBOL_func_1_0_parms = 245,           /* func_1_0_parms  */
  YYSYMBOL_uint_expr_1_0 = 246,            /* uint_expr_1_0  */
  YYSYMBOL_float_expr_1_0 = 247,           /* float_expr_1_0  */
  YYSYMBOL_string_expr_1_0 = 248,          /* string_expr_1_0  */
  YYSYMBOL_const_vect_expr_1_0 = 249,      /* const_vect_expr_1_0  */
  YYSYMBOL_opt_const_vect_exprlist_1_0 = 250, /* opt_const_vect_exprlist_1_0  */
  YYSYMBOL_const_vect_exprlist_1_0 = 251,  /* const_vect_exprlist_1_0  */
  YYSYMBOL_bool_expr_1_0 = 252,            /* bool_expr_1_0  */
  YYSYMBOL_type_expr_1_0 = 253,            /* type_expr_1_0  */
  YYSYMBOL_member_expr_2_0 = 254,          /* member_expr_2_0  */
  YYSYMBOL_join_expr_2_0 = 255,            /* join_expr_2_0  */
  YYSYMBOL_database_1_0_decl = 256,        /* database_1_0_decl  */
  YYSYMBOL_opt_database_dad_1_0 = 257,     /* opt_database_dad_1_0  */
  YYSYMBOL_database_body_1_0 = 258,        /* database_body_1_0  */
  YYSYMBOL_database_members_1_0 = 259,     /* database_members_1_0  */
  YYSYMBOL_database_member_1_0 = 260,      /* database_member_1_0  */
  YYSYMBOL_opt_template_1_0 = 261,         /* opt_template_1_0  */
  YYSYMBOL_db_member_1_0 = 262,            /* db_member_1_0  */
  YYSYMBOL_table_member_1_0 = 263,         /* table_member_1_0  */
  YYSYMBOL_alias_member_1_0 = 264,         /* alias_member_1_0  */
  YYSYMBOL_view_spec = 265,                /* view_spec  */
  YYSYMBOL_view_parms = 266,               /* view_parms  */
  YYSYMBOL_include_directive = 267,        /* include_directive  */
  YYSYMBOL_fqn_1_0 = 268,                  /* fqn_1_0  */
  YYSYMBOL_ident_1_0 = 269,                /* ident_1_0  */
  YYSYMBOL_empty = 270,                    /* empty  */
  YYSYMBOL_fqn_vers = 271,                 /* fqn_vers  */
  YYSYMBOL_fqn_opt_vers = 272,             /* fqn_opt_vers  */
  YYSYMBOL_version_2_0 = 273,              /* version_2_0  */
  YYSYMBOL_schema_2_0_opt = 274,           /* schema_2_0_opt  */
  YYSYMBOL_schema_2_0 = 275,               /* schema_2_0  */
  YYSYMBOL_schema_2_0_decl = 276,          /* schema_2_0_decl  */
  YYSYMBOL_view_2_0_decl = 277,            /* view_2_0_decl  */
  YYSYMBOL_view_parm = 278,                /* view_parm  */
  YYSYMBOL_opt_view_body = 279,            /* opt_view_body  */
  YYSYMBOL_view_body = 280,                /* view_body  */
  YYSYMBOL_view_member = 281,              /* view_member  */
  YYSYMBOL_opt_view_parents = 282,         /* opt_view_parents  */
  YYSYMBOL_view_parents = 283,             /* view_parents  */
  YYSYMBOL_view_parent = 284,              /* view_parent  */
  YYSYMBOL_view_parent_parms = 285         /* view_parent_parms  */
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
         || (defined SCHEMA_LTYPE_IS_TRIVIAL && SCHEMA_LTYPE_IS_TRIVIAL \
             && defined SCHEMA_STYPE_IS_TRIVIAL && SCHEMA_STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  68
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   837

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  157
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  129
/* YYNRULES -- Number of rules.  */
#define YYNRULES  280
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  540

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
     146,   147,   144,   153,   139,   152,   155,   154,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   156,   138,
     148,   145,   149,     2,   151,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   142,     2,   143,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   140,   150,   141,     2,     2,     2,     2,
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

#if SCHEMA_DEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   315,   315,   316,   320,   321,   322,   328,   332,   333,
     337,   338,   342,   343,   344,   345,   346,   347,   348,   349,
     350,   351,   352,   353,   354,   355,   361,   369,   370,   376,
     381,   385,   392,   393,   397,   398,   402,   403,   409,   411,
     416,   420,   427,   434,   439,   445,   449,   450,   451,   461,
     466,   471,   472,   476,   480,   481,   485,   486,   490,   491,
     495,   496,   500,   504,   505,   507,   509,   511,   516,   517,
     521,   525,   529,   530,   532,   534,   536,   541,   542,   546,
     547,   552,   553,   557,   559,   561,   566,   567,   571,   572,
     579,   584,   591,   593,   601,   608,   618,   619,   623,   625,
     630,   631,   635,   637,   639,   641,   646,   653,   661,   662,
     666,   667,   671,   673,   678,   679,   683,   684,   685,   687,
     689,   690,   691,   692,   693,   695,   699,   701,   706,   707,
     711,   712,   716,   717,   718,   722,   724,   729,   731,   733,
     738,   740,   742,   747,   748,   752,   756,   757,   761,   762,
     766,   767,   768,   769,   773,   778,   780,   782,   784,   789,
     794,   795,   799,   800,   804,   805,   809,   810,   817,   818,
     822,   823,   827,   828,   829,   830,   831,   832,   833,   834,
     835,   836,   837,   838,   839,   843,   850,   859,   860,   864,
     865,   866,   870,   871,   875,   879,   880,   884,   885,   889,
     890,   894,   895,   896,   900,   901,   905,   906,   910,   914,
     915,   919,   920,   924,   925,   929,   930,   934,   935,   939,
     941,   948,   956,   957,   961,   962,   966,   967,   971,   972,
     973,   974,   978,   979,   983,   988,   993,   998,  1003,  1004,
    1011,  1019,  1020,  1022,  1023,  1024,  1025,  1026,  1027,  1028,
    1029,  1033,  1037,  1041,  1045,  1046,  1052,  1056,  1057,  1061,
    1062,  1066,  1067,  1071,  1076,  1077,  1081,  1085,  1086,  1090,
    1091,  1095,  1097,  1099,  1103,  1104,  1108,  1109,  1113,  1118,
    1119
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
  "PT_ALIASMEMBER", "PT_VIEWSPEC", "';'", "','", "'{'", "'}'", "'['",
  "']'", "'*'", "'='", "'('", "')'", "'<'", "'>'", "'|'", "'@'", "'-'",
  "'+'", "'/'", "'.'", "':'", "$accept", "parse", "source", "version_1_0",
  "schema_1_0_opt", "schema_1_0", "schema_1_0_decl", "typedef_1_0_decl",
  "typedef_1_0_new_name_list", "typeset_1_0_decl", "typeset_1_0_new_name",
  "typeset_1_0_def", "typespec_1_0_list", "typespec_1_0", "dim_1_0",
  "format_1_0_decl", "format_1_0_new_name", "format_1_0_name",
  "const_1_0_decl", "alias_1_0_decl", "alias_1_0_new_name",
  "function_1_0_decl", "func_1_0_decl", "untyped_func_1_0_decl",
  "row_length_func_1_0_decl", "opt_func_1_0_schema_sig",
  "func_1_0_schema_sig", "func_1_0_schema_formals",
  "func_1_0_schema_formal", "func_1_0_return_type",
  "opt_func_1_0_fact_sig", "func_1_0_fact_sig", "func_1_0_fact_signature",
  "func_1_0_fact_params", "fact_param_1_0", "func_1_0_param_sig",
  "func_1_0_param_signature", "func_1_0_formal_params", "formal_param_1_0",
  "func_1_0_vararg_formals", "func_1_0_prologue", "script_1_0_stmt_seq",
  "script_1_0_stmt", "extern_1_0_decl", "ext_func_1_0_decl",
  "script_1_0_decl", "validate_1_0_decl", "physical_1_0_decl",
  "phys_1_0_return_type", "phys_1_0_prologue", "phys_1_0_body",
  "phys_1_0_body_stmt", "phys_1_0_stmt", "table_1_0_decl",
  "opt_tbl_1_0_parents", "tbl_1_0_parents", "tbl_1_0_body",
  "tbl_1_0_stmt_seq", "tbl_1_0_stmt", "production_1_0_stmt",
  "col_1_0_modifiers", "col_1_0_modifier_seq", "col_1_0_modifier",
  "col_1_0_decl", "phys_enc_ref", "typed_column_decl_1_0", "col_ident",
  "phys_ident", "opt_column_body_1_0", "column_body_1_0",
  "column_stmt_1_0", "default_view_1_0_decl", "physmbr_1_0_decl",
  "phys_coldef_1_0", "opt_col_schema_parms_1_0", "col_schema_parms_1_0",
  "col_schema_parm_1_0", "col_schema_value_1_0", "cond_expr_1_0",
  "expression_1_0", "primary_expr_1_0", "func_expr_1_0",
  "schema_parms_1_0", "schema_parm_1_0", "opt_factory_parms_1_0",
  "factory_parms_1_0", "factory_parms", "opt_func_1_0_parms",
  "func_1_0_parms", "uint_expr_1_0", "float_expr_1_0", "string_expr_1_0",
  "const_vect_expr_1_0", "opt_const_vect_exprlist_1_0",
  "const_vect_exprlist_1_0", "bool_expr_1_0", "type_expr_1_0",
  "member_expr_2_0", "join_expr_2_0", "database_1_0_decl",
  "opt_database_dad_1_0", "database_body_1_0", "database_members_1_0",
  "database_member_1_0", "opt_template_1_0", "db_member_1_0",
  "table_member_1_0", "alias_member_1_0", "view_spec", "view_parms",
  "include_directive", "fqn_1_0", "ident_1_0", "empty", "fqn_vers",
  "fqn_opt_vers", "version_2_0", "schema_2_0_opt", "schema_2_0",
  "schema_2_0_decl", "view_2_0_decl", "view_parm", "opt_view_body",
  "view_body", "view_member", "opt_view_parents", "view_parents",
  "view_parent", "view_parent_parms", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-369)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-255)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     521,  -369,     8,     8,     8,    98,     8,    59,   168,    57,
      52,     8,     8,     8,    98,   392,  -369,   244,   248,   699,
     699,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,   635,  -369,    -8,  -369,     8,
     -49,    -9,   118,  -369,  -369,   114,     8,    -6,     8,     8,
      12,  -369,  -369,  -369,    76,  -369,  -369,  -369,   200,    59,
    -369,   120,    -8,   149,   125,  -369,   173,   220,  -369,  -369,
    -369,   699,  -369,  -369,     8,  -369,  -369,  -369,   635,  -369,
    -369,   597,   232,   125,   -14,   107,  -369,     8,   187,  -369,
    -369,   278,   125,    42,    89,     8,  -369,   189,  -369,     8,
      80,  -369,     8,  -369,    76,  -369,     8,  -369,     8,   195,
    -369,   -40,  -369,     8,   281,  -369,  -369,   193,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,   486,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
     486,  -369,     8,   386,  -369,   486,   486,   212,  -369,  -369,
    -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,  -369,    34,
       2,  -369,   277,    -9,  -369,    61,  -369,  -369,   280,   299,
    -369,    12,  -369,  -369,     8,   282,  -369,   282,   313,  -369,
     279,  -369,  -369,     8,   -98,  -369,  -369,     8,   318,  -369,
     323,   328,  -369,   321,   203,  -369,  -369,   -33,  -369,  -369,
    -369,  -369,   486,     8,   486,   325,  -369,  -369,     8,  -369,
    -369,  -369,    78,  -369,    36,  -369,  -369,  -369,  -369,  -369,
    -369,  -369,   125,    29,   326,  -369,  -369,   -12,     8,   324,
     378,   421,  -369,    27,  -369,    38,     8,  -369,  -369,     8,
     578,  -369,  -369,    19,   291,  -369,  -369,  -369,     8,  -369,
     217,   218,     8,  -369,  -369,  -369,   486,   486,   386,     8,
     107,   272,  -369,  -369,  -369,   222,   486,     8,   329,  -369,
    -369,     8,     8,     8,   461,   327,   141,  -369,  -369,    45,
     295,    66,   338,  -369,  -369,     8,   443,   335,   470,   334,
     386,  -369,   474,     8,  -369,    27,  -369,   345,   357,  -369,
    -369,   386,   430,  -369,   430,    18,   277,  -369,  -369,  -369,
    -369,     8,   359,  -369,  -369,  -369,  -369,   277,   363,    -1,
     486,   486,  -369,  -369,   360,   369,  -369,   371,     8,     8,
       8,   372,  -369,  -369,    53,     8,  -369,  -369,     8,   182,
     499,   366,   257,  -369,  -369,  -369,    84,     8,  -369,   370,
     338,   338,  -369,    -4,  -369,    84,  -369,   216,   375,   486,
     376,   502,   230,  -369,  -369,  -369,    86,   300,   277,  -369,
     486,   486,   234,  -369,   296,  -369,  -369,  -369,  -369,  -369,
     237,  -369,     8,  -369,   377,   380,  -369,  -369,     8,  -369,
    -369,  -369,   486,  -369,   242,   385,   389,   122,  -369,  -369,
       8,  -369,   372,   511,   390,  -369,  -369,    14,   182,  -369,
     486,    21,  -369,     8,    87,     8,  -369,  -369,  -369,  -369,
      51,   381,   486,   393,  -369,   317,   386,  -369,   386,  -369,
     486,  -369,   112,   128,     8,  -369,     0,   486,   382,   394,
    -369,     7,   486,  -369,  -369,  -369,  -369,  -369,   372,  -369,
    -369,   211,  -369,   182,  -369,   390,   134,  -369,  -369,   397,
    -369,   241,  -369,   396,   399,  -369,  -369,   486,  -369,  -369,
     125,   136,  -369,  -369,   277,   400,   402,   406,  -369,   412,
       0,  -369,  -369,   156,     8,     8,     8,  -369,     8,  -369,
     416,     7,  -369,   391,  -369,   390,  -369,  -369,   486,   413,
    -369,  -369,   162,  -369,  -369,   457,   486,   486,  -369,  -369,
    -369,  -369,   251,  -369,     8,   414,  -369,  -369,  -369,  -369,
     175,  -369,  -369,   423,   196,   209,     8,  -369,   418,   486,
    -369,  -369,  -369,  -369,  -369,   486,   210,   213,  -369,  -369
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     0,   252,     0,   252,
     252,     0,     0,     0,     0,     0,    25,     0,     0,   252,
       4,    10,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,   252,   251,     0,   241,     0,
      34,     0,   252,    91,    90,     0,     0,    40,     0,     0,
       0,    45,    46,    47,     0,    52,    51,   240,     0,   252,
      92,   252,     0,     0,    30,    94,     0,     0,     1,     3,
       5,     8,     9,    11,     0,   261,   258,     6,   257,   259,
     262,     0,     0,    44,     0,     0,   253,     0,     0,   222,
      38,     0,    40,     0,     0,     0,   215,     0,    54,     0,
      34,    58,     0,    59,     0,    96,     0,    93,     0,     0,
     108,     0,    27,     0,     0,     7,   256,     0,   260,   243,
     244,   245,   246,   247,   248,   249,   250,   242,    43,     0,
     201,   203,   202,   204,   205,   206,   207,   145,   214,   213,
     252,    37,     0,     0,   174,     0,     0,     0,   173,    36,
     170,   175,   176,   177,   178,   179,   180,   183,   184,   172,
     241,   255,   252,   254,   223,   252,   221,    39,     0,     0,
      56,     0,    53,    57,     0,   252,    97,   252,   109,   110,
       0,   107,    26,     0,     0,    32,    29,     0,     0,   211,
       0,   210,   209,     0,     0,   187,   191,   189,   181,   182,
      35,   218,     0,     0,     0,     0,   193,   192,     0,   233,
     231,   224,   252,   226,     0,   228,   229,   230,   232,    50,
      49,    55,   216,   252,     0,    61,    60,     0,     0,     0,
     128,   132,   133,   252,   134,   252,     0,   125,   113,     0,
       0,   114,   116,     0,     0,   130,   123,    28,     0,    31,
       0,   241,     0,   264,    42,   208,     0,     0,     0,     0,
       0,     0,   168,   217,   195,     0,   252,     0,     0,   225,
     227,     0,     0,     0,     0,     0,   252,    68,    63,   252,
       0,     0,     0,    95,   111,     0,     0,     0,     0,   252,
       0,   121,     0,     0,   160,   252,   120,     0,     0,   112,
     115,     0,     0,   117,     0,    34,   252,   129,   132,   131,
      33,     0,   252,   266,   212,   171,   188,   252,     0,     0,
       0,     0,   194,   199,     0,   198,   197,     0,     0,     0,
       0,   252,    70,    62,     0,     0,    64,    81,     0,     0,
       0,     0,   252,    77,    72,    83,     0,     0,    48,     0,
       0,     0,   102,     0,   100,     0,    98,     0,     0,     0,
       0,     0,     0,   162,   165,   167,   166,     0,   252,   122,
       0,     0,     0,   135,     0,   144,   143,   136,   139,   138,
     241,   265,     0,   274,     0,     0,   190,   220,     0,   169,
     196,   186,     0,   236,     0,     0,     0,     0,    65,    82,
       0,    69,   252,     0,   252,    79,    71,     0,     0,    73,
       0,     0,    86,     0,     0,     0,   103,   104,    99,   101,
       0,     0,     0,     0,   154,     0,     0,   161,     0,   155,
       0,   159,     0,     0,     0,   142,   252,     0,     0,   275,
     276,   252,   252,   219,   200,   237,   234,   235,   252,    66,
      80,     0,    74,     0,    78,   252,     0,    85,    87,     0,
      84,     0,   106,     0,     0,   118,   157,     0,   163,   164,
     166,     0,   127,   126,   252,     0,     0,     0,   153,     0,
     147,   148,   146,     0,     0,     0,     0,   273,     0,   267,
       0,   268,   269,     0,    67,   252,    75,    88,     0,     0,
     124,   119,     0,   156,   137,     0,     0,     0,   140,   149,
     141,   279,     0,   277,     0,     0,   263,   270,   185,    76,
       0,   105,   158,     0,     0,     0,     0,   278,     0,     0,
      89,   152,   150,   151,   280,     0,     0,     0,   271,   272
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -369,  -369,  -369,  -369,  -369,   546,   197,  -369,  -369,  -369,
    -369,  -369,  -369,    50,   306,  -369,   522,  -369,  -369,  -369,
    -369,   262,    44,  -369,  -369,   558,  -369,  -369,   403,   -35,
     398,  -369,  -369,  -206,  -289,  -369,  -369,  -262,  -368,  -312,
    -369,   221,  -106,  -369,  -369,  -369,  -369,  -369,  -369,  -369,
    -369,   224,   110,  -369,  -369,  -369,  -369,  -369,   333,  -369,
    -369,  -369,   336,  -369,  -369,   275,  -369,   -93,  -369,  -369,
     101,  -369,  -213,   293,  -369,  -369,   163,   158,  -342,    83,
    -369,  -369,   287,   332,  -303,   288,  -369,   150,  -369,  -131,
    -369,  -369,  -369,  -369,  -369,  -369,   160,  -369,  -369,  -369,
    -369,  -369,  -369,   383,  -369,  -369,  -369,  -369,  -369,   268,
    -369,    -2,   283,   142,     4,   -32,  -369,  -369,  -369,   519,
    -369,   290,  -369,  -369,   108,  -369,  -369,   113,  -369
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    17,    18,    19,    70,    20,    21,    22,   111,    23,
      63,   114,   184,    96,   147,    24,    45,    46,    25,    26,
      82,    27,    51,    52,    53,    54,    55,    97,    98,   102,
     224,   225,   275,   276,   277,   280,   341,   342,   343,   336,
     348,   411,   412,    28,    44,    29,    30,    31,   106,   283,
     353,   354,   356,    32,   109,   178,   181,   240,   241,   242,
     243,   244,   245,   303,   304,   373,   374,   148,   479,   480,
     481,   246,   291,   292,   293,   362,   363,   364,   261,   262,
     150,   151,   194,   195,   205,   206,   265,   324,   325,   152,
     153,   154,   155,   190,   191,   156,   413,   157,   158,    33,
      88,   166,   212,   213,   214,   215,   216,   217,   267,   250,
      34,   159,    38,   337,   161,   162,    35,    77,    78,    79,
      80,   253,   490,   491,   492,   384,   439,   440,   512
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    40,    41,   379,    47,    36,    86,   -41,    42,    41,
      62,    64,   196,   387,   385,    61,   201,   349,   399,   398,
      36,    36,   296,   105,   350,    36,   351,    36,   432,   433,
     409,   486,    36,    86,    36,    83,   475,    84,   476,   454,
     338,   248,    36,   249,    92,   401,    93,    94,   100,    86,
     477,   289,   100,    39,    60,   164,   100,   399,    36,    95,
      40,   410,   289,   271,    36,   431,    36,   331,   456,   176,
     175,   338,    41,    48,    49,   295,   179,   404,   117,   272,
      48,    49,   369,   454,   208,   163,    59,   349,   471,    36,
     449,   410,   452,    85,   350,   483,   351,    36,   182,   183,
     163,   208,   100,   107,    41,   209,   163,    81,   401,   260,
     177,    40,   112,   130,   131,   132,   133,   134,   135,   136,
      36,   137,   209,    81,   410,   502,   399,   196,   281,   402,
     101,   129,     7,   282,   352,    36,   494,   418,   478,   138,
     100,   197,    81,   496,   202,   487,   455,    81,    81,    56,
      81,    56,    56,   139,   388,   252,   520,   203,   453,   365,
      85,    72,   457,   185,   524,   525,   204,   301,   149,   100,
     196,   504,   222,   273,    81,   290,   268,    76,    40,    57,
    -254,    40,  -254,   519,    89,   163,   290,   536,   168,   339,
      81,   495,   462,   537,   448,    36,   284,   400,    81,   210,
      50,    56,   211,   110,   352,    50,   163,    50,   338,   375,
      99,   375,   188,    36,   103,   399,   210,    73,   103,   269,
     104,    40,    85,   189,    36,   460,   163,   317,   198,   199,
     239,   428,    75,   247,   174,   169,    81,   338,    40,   329,
     330,   305,    81,    81,    68,    81,    40,   306,    69,   140,
     472,   141,    90,   142,   101,   143,   197,   163,   144,   145,
     146,   368,   320,    87,   103,   108,   473,    43,    73,   163,
     163,    40,   497,   274,   503,    75,    65,    40,   320,   252,
     334,    81,   192,   357,   320,   335,   320,   264,   366,   113,
     239,   163,    36,   302,   510,   365,   252,   365,   310,   197,
     522,   229,   193,   230,   207,   458,   320,   218,   231,   163,
     232,   115,   320,   530,   458,   307,   233,   226,   234,   226,
     308,   235,   232,   274,   236,   320,   163,   165,   171,   340,
     234,    99,    40,    40,   532,   180,    40,    40,   172,   314,
     315,   187,   258,   149,   100,   414,   320,   533,   538,   323,
     438,   539,   259,   100,   218,   200,   311,  -238,   116,   320,
     320,   321,   421,   320,   127,   278,   312,  -238,   160,   426,
     128,   322,    81,   258,   523,   294,  -239,   294,   170,   427,
     163,   311,   173,   434,   274,   274,  -239,   499,   403,   340,
     526,   445,   130,   131,   132,    40,   407,    81,    40,    36,
     527,   408,   474,   389,   390,    40,    40,   286,   326,   100,
      66,    67,   160,   461,   287,   319,   167,   237,   100,   186,
     238,   344,   320,   160,   366,   204,   470,   219,   160,   160,
     223,   294,   163,   345,   435,   346,   436,   294,   429,    40,
     347,   437,   423,    36,   137,   430,   220,   274,   207,    40,
     274,    40,   228,   438,   383,   466,   254,   340,   340,   207,
     416,   417,   467,   130,   131,   132,   255,   256,   257,   285,
     251,   266,   279,   288,   332,   444,   333,   328,   355,   358,
     359,   360,   290,   163,    40,   160,   263,   160,   367,    40,
     370,   488,   130,   131,   132,   133,   134,   135,   136,    36,
     137,   340,   371,   340,   382,   464,   386,   391,   392,   393,
     207,   397,   405,   406,   424,   415,   425,   441,   138,   297,
     422,     1,   298,   446,   450,   323,   442,   447,   463,   451,
     484,   465,   139,   485,   500,   313,   514,   501,   518,   160,
     160,   488,   498,   160,     2,   505,     3,   506,     4,   160,
     327,   507,     5,   508,     6,     7,     8,   516,     9,   529,
     521,   531,    10,   535,    11,    71,   318,    58,    91,    12,
      13,    14,    15,   300,   221,   227,   420,   419,   482,   377,
     309,   509,   361,   489,   326,   376,   469,   376,   372,   468,
     316,    36,   493,   378,   380,   270,   394,   118,   513,   517,
     229,   381,   230,   160,   160,     0,     0,   231,     0,   232,
      36,   251,   395,   396,     0,   233,   207,   234,     0,     0,
     235,     0,     0,   236,   119,   120,     0,   121,   140,     0,
       0,     0,   142,     0,   143,   122,     0,   144,   145,   146,
     123,     0,   160,     0,   124,     0,     0,     0,     0,   125,
       0,     0,   126,   160,   160,     0,     0,     0,     2,    16,
       3,     0,     4,     0,     0,     0,     5,     0,     6,     7,
       8,   443,     9,     0,     0,   160,    10,     0,    11,     0,
       0,     0,     0,    12,    13,    14,     0,    74,     0,     0,
       0,     0,     0,   160,     0,     0,   459,     0,     0,     0,
       0,     0,     0,     0,     0,   160,     0,     0,     0,     0,
       0,     0,     0,   160,     0,     0,   237,     0,     0,   299,
     160,     0,     2,     0,     3,   160,     4,     0,     0,     0,
       5,     0,     6,     7,     8,     0,     9,     0,     0,     0,
      10,     0,    11,     0,     0,     0,     0,    12,    13,    14,
     160,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   511,     0,     0,
       0,   515,     0,    16,     0,     0,     0,     0,     0,     0,
       0,   160,     0,     0,     0,     0,     0,     0,     0,   160,
     160,     0,     0,     0,     0,     0,     0,   528,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   534,
       0,     0,   160,     0,     0,     0,     0,     0,   160,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    16
};

static const yytype_int16 yycheck[] =
{
       2,     3,     4,   306,     6,    13,    15,    13,     4,    11,
      12,    13,   143,    14,   317,    11,    14,    21,     4,   331,
      13,    13,   235,    58,    28,    13,    30,    13,   370,   371,
     342,    24,    13,    15,    13,    37,    36,    39,    38,   407,
      26,   139,    13,   141,    46,   334,    48,    49,    50,    15,
      50,    24,    54,     3,    10,    87,    58,     4,    13,    47,
      62,    40,    24,    27,    13,   368,    13,   273,   410,   104,
     102,    26,    74,    21,    22,    37,   108,   339,    74,    43,
      21,    22,   295,   451,    23,    87,    34,    21,   430,    13,
     402,    40,   404,   142,    28,   437,    30,    13,   138,   139,
     102,    23,   104,    59,   106,    44,   108,   156,   397,   142,
     106,   113,    62,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    44,   156,    40,   467,     4,   258,   140,   335,
      54,   145,    34,   145,   138,    13,   448,   141,   138,    32,
     142,   143,   156,   455,   142,   138,   408,   156,   156,     7,
     156,     9,    10,    46,   155,   187,   498,   155,   144,   290,
     142,    19,   141,   113,   506,   507,   148,   148,    85,   171,
     301,   474,   174,   144,   156,   148,   208,    35,   180,    11,
     146,   183,   148,   495,    42,   187,   148,   529,   146,   144,
     156,   453,   141,   535,   400,    13,   228,   144,   156,   138,
     148,    59,   141,    61,   138,   148,   208,   148,    26,   302,
      50,   304,   129,    13,    54,     4,   138,    20,    58,   141,
      20,   223,   142,   140,    13,   138,   228,   259,   145,   146,
     180,   145,    35,   183,   154,   146,   156,    26,   240,   271,
     272,   243,   156,   156,     0,   156,   248,   243,     0,   142,
     138,   144,   138,   146,    54,   148,   258,   259,   151,   152,
     153,   293,   150,   145,   104,   145,   138,     5,    71,   271,
     272,   273,   138,   223,   138,    78,    14,   279,   150,   311,
     139,   156,   140,   285,   150,   144,   150,   204,   290,   140,
     240,   293,    13,   243,   138,   426,   328,   428,   248,   301,
     138,    22,   142,    24,   162,   411,   150,   165,    29,   311,
      31,   138,   150,   138,   420,    24,    37,   175,    39,   177,
      29,    42,    31,   273,    45,   150,   328,   140,   139,   279,
      39,   171,   334,   335,   138,   140,   338,   339,   149,   256,
     257,   148,   139,   260,   346,   347,   150,   138,   138,   266,
     382,   138,   149,   355,   212,   143,   139,   139,   138,   150,
     150,   139,   146,   150,    81,   223,   149,   149,    85,   139,
     138,   149,   156,   139,   505,   233,   139,   235,    95,   149,
     382,   139,    99,   149,   334,   335,   149,   146,   338,   339,
     139,   149,     6,     7,     8,   397,   139,   156,   400,    13,
     149,   144,   434,   320,   321,   407,   408,    29,   266,   411,
      18,    19,   129,   415,    36,   143,   138,   138,   420,   138,
     141,   279,   150,   140,   426,   148,   428,   147,   145,   146,
     148,   289,   434,   138,   138,   140,   140,   295,   138,   441,
     145,   145,   359,    13,    14,   145,   147,   397,   306,   451,
     400,   453,   139,   485,   312,   138,   138,   407,   408,   317,
     350,   351,   145,     6,     7,     8,   143,   139,   147,   145,
     187,   146,   146,    52,    13,   392,   149,   148,   140,    36,
     145,    11,   148,   485,   486,   202,   203,   204,    14,   491,
     145,   441,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   451,   145,   453,   145,   422,   143,   147,   139,   138,
     368,   139,    13,   147,   138,   145,    14,   140,    32,   236,
     145,     0,   239,   138,    13,   442,   146,   138,   147,   139,
     148,   138,    46,   139,   138,   252,   486,   138,   147,   256,
     257,   491,   145,   260,    23,   145,    25,   145,    27,   266,
     267,   145,    31,   141,    33,    34,    35,   141,    37,   145,
     147,   138,    41,   145,    43,    19,   260,     9,    46,    48,
      49,    50,    51,   240,   171,   177,   355,   353,   436,   304,
     244,   480,   289,   441,   442,   302,   428,   304,   301,   426,
     258,    13,   442,   305,   311,   212,   328,    78,   485,   491,
      22,   311,    24,   320,   321,    -1,    -1,    29,    -1,    31,
      13,   328,   329,   330,    -1,    37,   474,    39,    -1,    -1,
      42,    -1,    -1,    45,    27,    28,    -1,    30,   142,    -1,
      -1,    -1,   146,    -1,   148,    38,    -1,   151,   152,   153,
      43,    -1,   359,    -1,    47,    -1,    -1,    -1,    -1,    52,
      -1,    -1,    55,   370,   371,    -1,    -1,    -1,    23,   138,
      25,    -1,    27,    -1,    -1,    -1,    31,    -1,    33,    34,
      35,   388,    37,    -1,    -1,   392,    41,    -1,    43,    -1,
      -1,    -1,    -1,    48,    49,    50,    -1,    52,    -1,    -1,
      -1,    -1,    -1,   410,    -1,    -1,   413,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   422,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   430,    -1,    -1,   138,    -1,    -1,   141,
     437,    -1,    23,    -1,    25,   442,    27,    -1,    -1,    -1,
      31,    -1,    33,    34,    35,    -1,    37,    -1,    -1,    -1,
      41,    -1,    43,    -1,    -1,    -1,    -1,    48,    49,    50,
     467,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   484,    -1,    -1,
      -1,   488,    -1,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   498,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   506,
     507,    -1,    -1,    -1,    -1,    -1,    -1,   514,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   526,
      -1,    -1,   529,    -1,    -1,    -1,    -1,    -1,   535,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   138
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,     0,    23,    25,    27,    31,    33,    34,    35,    37,
      41,    43,    48,    49,    50,    51,   138,   158,   159,   160,
     162,   163,   164,   166,   172,   175,   176,   178,   200,   202,
     203,   204,   210,   256,   267,   273,    13,   268,   269,   170,
     268,   268,   271,   178,   201,   173,   174,   268,    21,    22,
     148,   179,   180,   181,   182,   183,   270,    11,   182,    34,
     179,   271,   268,   167,   268,   178,    18,    19,     0,     0,
     161,   162,   270,   163,    52,   163,   270,   274,   275,   276,
     277,   156,   177,   268,   268,   142,    15,   145,   257,   270,
     138,   173,   268,   268,   268,    47,   170,   184,   185,   253,
     268,    54,   186,   253,    20,   186,   205,   179,   145,   211,
     270,   165,   170,   140,   168,   138,   138,   271,   276,    27,
      28,    30,    38,    43,    47,    52,    55,   269,   138,   145,
       6,     7,     8,     9,    10,    11,    12,    14,    32,    46,
     142,   144,   146,   148,   151,   152,   153,   171,   224,   236,
     237,   238,   246,   247,   248,   249,   252,   254,   255,   268,
     269,   271,   272,   268,   272,   140,   258,   138,   146,   146,
     269,   139,   149,   269,   154,   272,   186,   271,   212,   272,
     140,   213,   138,   139,   169,   170,   138,   148,   236,   236,
     250,   251,   270,   253,   239,   240,   246,   268,   236,   236,
     143,    14,   142,   155,   148,   241,   242,   270,    23,    44,
     138,   141,   259,   260,   261,   262,   263,   264,   270,   147,
     147,   185,   268,   148,   187,   188,   270,   187,   139,    22,
      24,    29,    31,    37,    39,    42,    45,   138,   141,   170,
     214,   215,   216,   217,   218,   219,   228,   170,   139,   141,
     266,   269,   272,   278,   138,   143,   139,   147,   139,   149,
     142,   235,   236,   269,   236,   243,   146,   265,   272,   141,
     260,    27,    43,   144,   170,   189,   190,   191,   270,   146,
     192,   140,   145,   206,   272,   145,    29,    36,    52,    24,
     148,   229,   230,   231,   270,    37,   229,   269,   269,   141,
     215,   148,   170,   220,   221,   268,   271,    24,    29,   219,
     170,   139,   149,   269,   236,   236,   240,   272,   171,   143,
     150,   139,   149,   236,   244,   245,   270,   269,   148,   272,
     272,   190,    13,   149,   139,   144,   196,   270,    26,   144,
     170,   193,   194,   195,   270,   138,   140,   145,   197,    21,
      28,    30,   138,   207,   208,   140,   209,   268,    36,   145,
      11,   230,   232,   233,   234,   246,   268,    14,   272,   229,
     145,   145,   239,   222,   223,   224,   269,   222,   242,   241,
     269,   278,   145,   270,   282,   241,   143,    14,   155,   236,
     236,   147,   139,   138,   266,   269,   269,   139,   196,     4,
     144,   191,   190,   170,   194,    13,   147,   139,   144,   196,
      40,   198,   199,   253,   268,   145,   209,   209,   141,   208,
     198,   146,   145,   236,   138,    14,   139,   149,   145,   138,
     145,   241,   235,   235,   149,   138,   140,   145,   272,   283,
     284,   140,   146,   269,   236,   149,   138,   138,   190,   196,
      13,   139,   196,   144,   195,   194,   235,   141,   199,   269,
     138,   268,   141,   147,   236,   138,   138,   145,   233,   234,
     268,   235,   138,   138,   272,    36,    38,    50,   138,   225,
     226,   227,   270,   235,   148,   139,    24,   138,   170,   270,
     279,   280,   281,   244,   196,   194,   196,   138,   145,   146,
     138,   138,   235,   138,   241,   145,   145,   145,   141,   227,
     138,   269,   285,   284,   170,   269,   141,   281,   147,   196,
     235,   147,   138,   246,   235,   235,   139,   149,   269,   145,
     138,   138,   138,   138,   269,   145,   235,   235,   138,   138
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   157,   158,   158,   159,   159,   159,   160,   161,   161,
     162,   162,   163,   163,   163,   163,   163,   163,   163,   163,
     163,   163,   163,   163,   163,   163,   164,   165,   165,   166,
     167,   168,   169,   169,   170,   170,   171,   171,   172,   172,
     173,   174,   175,   176,   177,   178,   179,   179,   179,   180,
     181,   182,   182,   183,   184,   184,   185,   185,   186,   186,
     187,   187,   188,   189,   189,   189,   189,   189,   190,   190,
     191,   192,   193,   193,   193,   193,   193,   194,   194,   195,
     195,   196,   196,   197,   197,   197,   198,   198,   199,   199,
     200,   201,   202,   202,   203,   204,   205,   205,   206,   206,
     207,   207,   208,   208,   208,   208,   209,   210,   211,   211,
     212,   212,   213,   213,   214,   214,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   216,   216,   217,   217,
     218,   218,   219,   219,   219,   220,   220,   221,   221,   221,
     222,   222,   222,   223,   223,   224,   225,   225,   226,   226,
     227,   227,   227,   227,   228,   229,   229,   229,   229,   230,
     231,   231,   232,   232,   233,   233,   234,   234,   235,   235,
     236,   236,   237,   237,   237,   237,   237,   237,   237,   237,
     237,   237,   237,   237,   237,   238,   238,   239,   239,   240,
     240,   240,   241,   241,   242,   243,   243,   244,   244,   245,
     245,   246,   246,   246,   247,   247,   248,   248,   249,   250,
     250,   251,   251,   252,   252,   253,   253,   254,   254,   255,
     255,   256,   257,   257,   258,   258,   259,   259,   260,   260,
     260,   260,   261,   261,   262,   263,   264,   265,   266,   266,
     267,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   269,   270,   271,   272,   272,   273,   274,   274,   275,
     275,   276,   276,   277,   266,   266,   278,   279,   279,   280,
     280,   281,   281,   281,   282,   282,   283,   283,   284,   285,
     285
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     3,     1,     1,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     3,     4,
       1,     3,     1,     3,     1,     4,     1,     1,     3,     4,
       1,     1,     6,     4,     1,     2,     1,     1,     6,     4,
       4,     1,     1,     3,     1,     3,     2,     2,     1,     1,
       1,     1,     3,     1,     2,     3,     4,     5,     1,     3,
       2,     3,     1,     2,     3,     4,     5,     1,     3,     2,
       3,     1,     2,     1,     3,     3,     1,     2,     3,     5,
       2,     1,     2,     3,     2,     6,     1,     2,     2,     3,
       1,     2,     1,     2,     2,     5,     3,     4,     1,     2,
       1,     3,     3,     2,     1,     2,     1,     2,     5,     6,
       2,     2,     3,     1,     6,     1,     5,     5,     1,     2,
       1,     2,     1,     1,     1,     2,     2,     5,     2,     2,
       4,     4,     2,     1,     1,     1,     1,     1,     1,     2,
       4,     4,     4,     1,     4,     3,     5,     4,     6,     3,
       1,     3,     1,     3,     3,     1,     1,     1,     1,     3,
       1,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     1,     1,     8,     5,     1,     3,     1,
       4,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       1,     1,     3,     1,     1,     1,     3,     3,     2,     6,
       5,     4,     1,     2,     2,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     5,     5,     4,     4,     1,     3,
       2,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     1,     0,     2,     1,     1,     3,     1,     1,     1,
       2,     1,     1,     9,     1,     3,     2,     1,     1,     1,
       2,     5,     6,     1,     1,     2,     1,     3,     4,     1,
       3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = SCHEMA_EMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == SCHEMA_EMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, ctx, root, errors, sb, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use SCHEMA_error or SCHEMA_UNDEF. */
#define YYERRCODE SCHEMA_UNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if SCHEMA_DEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined SCHEMA_LTYPE_IS_TRIVIAL && SCHEMA_LTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, ctx, root, errors, sb); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (ctx);
  YY_USE (root);
  YY_USE (errors);
  YY_USE (sb);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, ctx, root, errors, sb);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
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
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), ctx, root, errors, sb);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, ctx, root, errors, sb); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !SCHEMA_DEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !SCHEMA_DEBUG */


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
  YYLTYPE *yylloc;
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
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (ctx);
  YY_USE (root);
  YY_USE (errors);
  YY_USE (sb);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_YYEOF: /* "end of source"  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_UNRECOGNIZED: /* UNRECOGNIZED  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_ELLIPSIS: /* ELLIPSIS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_INCREMENT: /* INCREMENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_DECIMAL: /* DECIMAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_OCTAL: /* OCTAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_HEX: /* HEX  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_FLOAT_: /* FLOAT_  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_EXP_FLOAT: /* EXP_FLOAT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_STRING: /* STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_ESCAPED_STRING: /* ESCAPED_STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_IDENTIFIER_1_0: /* IDENTIFIER_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PHYSICAL_IDENTIFIER_1_0: /* PHYSICAL_IDENTIFIER_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_VERSION: /* VERSION  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_UNTERM_STRING: /* UNTERM_STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_UNTERM_ESCAPED_STRING: /* UNTERM_ESCAPED_STRING  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_VERS_1_0: /* VERS_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_VERS_2_0: /* VERS_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW___no_header: /* KW___no_header  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW___row_length: /* KW___row_length  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW___untyped: /* KW___untyped  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_alias: /* KW_alias  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_column: /* KW_column  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_const: /* KW_const  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_control: /* KW_control  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_database: /* KW_database  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_decode: /* KW_decode  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_default: /* KW_default  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_encode: /* KW_encode  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_extern: /* KW_extern  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_false: /* KW_false  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_fmtdef: /* KW_fmtdef  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_function: /* KW_function  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_include: /* KW_include  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_limit: /* KW_limit  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_physical: /* KW_physical  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_read: /* KW_read  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_readonly: /* KW_readonly  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_return: /* KW_return  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_schema: /* KW_schema  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_static: /* KW_static  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_table: /* KW_table  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_template: /* KW_template  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_trigger: /* KW_trigger  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_true: /* KW_true  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_type: /* KW_type  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_typedef: /* KW_typedef  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_typeset: /* KW_typeset  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_validate: /* KW_validate  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_version: /* KW_version  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_view: /* KW_view  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_virtual: /* KW_virtual  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_void: /* KW_void  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_KW_write: /* KW_write  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_ASTLIST: /* PT_ASTLIST  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PARSE: /* PT_PARSE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_SOURCE: /* PT_SOURCE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VERSION_1_0: /* PT_VERSION_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VERSION_2: /* PT_VERSION_2  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_SCHEMA_1_0: /* PT_SCHEMA_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_SCHEMA_2_0: /* PT_SCHEMA_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_INCLUDE: /* PT_INCLUDE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TYPEDEF: /* PT_TYPEDEF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FQN: /* PT_FQN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_IDENT: /* PT_IDENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSIDENT: /* PT_PHYSIDENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_UINT: /* PT_UINT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TYPESET: /* PT_TYPESET  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TYPESETDEF: /* PT_TYPESETDEF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FORMAT: /* PT_FORMAT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_CONST: /* PT_CONST  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_ALIAS: /* PT_ALIAS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_EXTERN: /* PT_EXTERN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FUNCTION: /* PT_FUNCTION  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_UNTYPED: /* PT_UNTYPED  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_ROWLENGTH: /* PT_ROWLENGTH  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FUNCDECL: /* PT_FUNCDECL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_EMPTY: /* PT_EMPTY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_SCHEMASIG: /* PT_SCHEMASIG  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_SCHEMAFORMAL: /* PT_SCHEMAFORMAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_RETURNTYPE: /* PT_RETURNTYPE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FACTSIG: /* PT_FACTSIG  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FUNCSIG: /* PT_FUNCSIG  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FUNCPARAMS: /* PT_FUNCPARAMS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FORMALPARAM: /* PT_FORMALPARAM  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_ELLIPSIS: /* PT_ELLIPSIS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FUNCPROLOGUE: /* PT_FUNCPROLOGUE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_RETURN: /* PT_RETURN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PRODSTMT: /* PT_PRODSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PRODTRIGGER: /* PT_PRODTRIGGER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_SCHEMA: /* PT_SCHEMA  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VALIDATE: /* PT_VALIDATE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSICAL: /* PT_PHYSICAL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSPROLOGUE: /* PT_PHYSPROLOGUE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSSTMT: /* PT_PHYSSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSBODYSTMT: /* PT_PHYSBODYSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TABLE: /* PT_TABLE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TABLEPARENTS: /* PT_TABLEPARENTS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TABLEBODY: /* PT_TABLEBODY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FUNCEXPR: /* PT_FUNCEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_FACTPARMS: /* PT_FACTPARMS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLUMN: /* PT_COLUMN  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLUMNEXPR: /* PT_COLUMNEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLDECL: /* PT_COLDECL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TYPEDCOL: /* PT_TYPEDCOL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLSTMT: /* PT_COLSTMT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_DFLTVIEW: /* PT_DFLTVIEW  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSMBR: /* PT_PHYSMBR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSCOL: /* PT_PHYSCOL  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSCOLDEF: /* PT_PHYSCOLDEF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLSCHEMAPARMS: /* PT_COLSCHEMAPARMS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLSCHEMAPARAM: /* PT_COLSCHEMAPARAM  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_COLUNTYPED: /* PT_COLUNTYPED  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_DATABASE: /* PT_DATABASE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TYPEEXPR: /* PT_TYPEEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_DBBODY: /* PT_DBBODY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_DBDAD: /* PT_DBDAD  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_DBMEMBER: /* PT_DBMEMBER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TBLMEMBER: /* PT_TBLMEMBER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_NOHEADER: /* PT_NOHEADER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_CASTEXPR: /* PT_CASTEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_CONSTVECT: /* PT_CONSTVECT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_NEGATE: /* PT_NEGATE  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_UNARYPLUS: /* PT_UNARYPLUS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VERSNAME: /* PT_VERSNAME  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_ARRAY: /* PT_ARRAY  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_PHYSENCREF: /* PT_PHYSENCREF  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_TYPEDCOLEXPR: /* PT_TYPEDCOLEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VIEW: /* PT_VIEW  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VIEWPARAM: /* PT_VIEWPARAM  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VIEWPARENTS: /* PT_VIEWPARENTS  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VIEWPARENT: /* PT_VIEWPARENT  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_MEMBEREXPR: /* PT_MEMBEREXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_JOINEXPR: /* PT_JOINEXPR  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_ALIASMEMBER: /* PT_ALIASMEMBER  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_PT_VIEWSPEC: /* PT_VIEWSPEC  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_138_: /* ';'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_139_: /* ','  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_140_: /* '{'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_141_: /* '}'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_142_: /* '['  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_143_: /* ']'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_144_: /* '*'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_145_: /* '='  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_146_: /* '('  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_147_: /* ')'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_148_: /* '<'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_149_: /* '>'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_150_: /* '|'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_151_: /* '@'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_152_: /* '-'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_153_: /* '+'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_154_: /* '/'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_155_: /* '.'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_156_: /* ':'  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_parse: /* parse  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_source: /* source  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_version_1_0: /* version_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_1_0_opt: /* schema_1_0_opt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_1_0: /* schema_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_1_0_decl: /* schema_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typedef_1_0_decl: /* typedef_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typedef_1_0_new_name_list: /* typedef_1_0_new_name_list  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typeset_1_0_decl: /* typeset_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typeset_1_0_new_name: /* typeset_1_0_new_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typeset_1_0_def: /* typeset_1_0_def  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typespec_1_0_list: /* typespec_1_0_list  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typespec_1_0: /* typespec_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_dim_1_0: /* dim_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_format_1_0_decl: /* format_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_format_1_0_new_name: /* format_1_0_new_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_format_1_0_name: /* format_1_0_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_const_1_0_decl: /* const_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_alias_1_0_decl: /* alias_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_alias_1_0_new_name: /* alias_1_0_new_name  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_function_1_0_decl: /* function_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_decl: /* func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_untyped_func_1_0_decl: /* untyped_func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_row_length_func_1_0_decl: /* row_length_func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_func_1_0_schema_sig: /* opt_func_1_0_schema_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_schema_sig: /* func_1_0_schema_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_schema_formals: /* func_1_0_schema_formals  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_schema_formal: /* func_1_0_schema_formal  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_return_type: /* func_1_0_return_type  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_func_1_0_fact_sig: /* opt_func_1_0_fact_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_fact_sig: /* func_1_0_fact_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_fact_signature: /* func_1_0_fact_signature  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_fact_params: /* func_1_0_fact_params  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_fact_param_1_0: /* fact_param_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_param_sig: /* func_1_0_param_sig  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_param_signature: /* func_1_0_param_signature  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_formal_params: /* func_1_0_formal_params  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_formal_param_1_0: /* formal_param_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_vararg_formals: /* func_1_0_vararg_formals  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_prologue: /* func_1_0_prologue  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_script_1_0_stmt_seq: /* script_1_0_stmt_seq  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_script_1_0_stmt: /* script_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_extern_1_0_decl: /* extern_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_ext_func_1_0_decl: /* ext_func_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_script_1_0_decl: /* script_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_validate_1_0_decl: /* validate_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_physical_1_0_decl: /* physical_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_1_0_return_type: /* phys_1_0_return_type  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_1_0_prologue: /* phys_1_0_prologue  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_1_0_body: /* phys_1_0_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_1_0_body_stmt: /* phys_1_0_body_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_1_0_stmt: /* phys_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_table_1_0_decl: /* table_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_tbl_1_0_parents: /* opt_tbl_1_0_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_tbl_1_0_parents: /* tbl_1_0_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_tbl_1_0_body: /* tbl_1_0_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_tbl_1_0_stmt_seq: /* tbl_1_0_stmt_seq  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_tbl_1_0_stmt: /* tbl_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_production_1_0_stmt: /* production_1_0_stmt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_1_0_modifiers: /* col_1_0_modifiers  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_1_0_modifier_seq: /* col_1_0_modifier_seq  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_1_0_modifier: /* col_1_0_modifier  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_1_0_decl: /* col_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_enc_ref: /* phys_enc_ref  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_typed_column_decl_1_0: /* typed_column_decl_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_ident: /* col_ident  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_ident: /* phys_ident  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_column_body_1_0: /* opt_column_body_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_column_body_1_0: /* column_body_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_column_stmt_1_0: /* column_stmt_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_default_view_1_0_decl: /* default_view_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_physmbr_1_0_decl: /* physmbr_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_phys_coldef_1_0: /* phys_coldef_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_col_schema_parms_1_0: /* opt_col_schema_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_schema_parms_1_0: /* col_schema_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_schema_parm_1_0: /* col_schema_parm_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_col_schema_value_1_0: /* col_schema_value_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_cond_expr_1_0: /* cond_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_expression_1_0: /* expression_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_primary_expr_1_0: /* primary_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_expr_1_0: /* func_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_parms_1_0: /* schema_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_parm_1_0: /* schema_parm_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_factory_parms_1_0: /* opt_factory_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_factory_parms_1_0: /* factory_parms_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_factory_parms: /* factory_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_func_1_0_parms: /* opt_func_1_0_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_func_1_0_parms: /* func_1_0_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_uint_expr_1_0: /* uint_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_float_expr_1_0: /* float_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_string_expr_1_0: /* string_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_const_vect_expr_1_0: /* const_vect_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_const_vect_exprlist_1_0: /* opt_const_vect_exprlist_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_const_vect_exprlist_1_0: /* const_vect_exprlist_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_bool_expr_1_0: /* bool_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_type_expr_1_0: /* type_expr_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_member_expr_2_0: /* member_expr_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_join_expr_2_0: /* join_expr_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_database_1_0_decl: /* database_1_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_database_dad_1_0: /* opt_database_dad_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_database_body_1_0: /* database_body_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_database_members_1_0: /* database_members_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_database_member_1_0: /* database_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_template_1_0: /* opt_template_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_db_member_1_0: /* db_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_table_member_1_0: /* table_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_alias_member_1_0: /* alias_member_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_spec: /* view_spec  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_parms: /* view_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_include_directive: /* include_directive  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_fqn_1_0: /* fqn_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_ident_1_0: /* ident_1_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_empty: /* empty  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_fqn_vers: /* fqn_vers  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_fqn_opt_vers: /* fqn_opt_vers  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_version_2_0: /* version_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_2_0_opt: /* schema_2_0_opt  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_2_0: /* schema_2_0  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_schema_2_0_decl: /* schema_2_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_2_0_decl: /* view_2_0_decl  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_parm: /* view_parm  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_view_body: /* opt_view_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_body: /* view_body  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_member: /* view_member  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_opt_view_parents: /* opt_view_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_parents: /* view_parents  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_parent: /* view_parent  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

    case YYSYMBOL_view_parent_parms: /* view_parent_parms  */
            {
    if ( (*yyvaluep) . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( (*yyvaluep) . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( (*yyvaluep) . leading_ws );
    }
}
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (ctx_t ctx, ParseTree** root, ErrorReport * errors, struct SchemaScanBlock* sb)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined SCHEMA_LTYPE_IS_TRIVIAL && SCHEMA_LTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

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

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = SCHEMA_EMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
  if (yychar == SCHEMA_EMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc, sb);
    }

  if (yychar <= END_SOURCE)
    {
      yychar = END_SOURCE;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == SCHEMA_error)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = SCHEMA_UNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
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
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = SCHEMA_EMPTY;
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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* parse: "end of source"  */
                                { *root = MakeTree ( ctx, PT_PARSE, T ( ctx, yyvsp[0] ) );              yyval . subtree = 0; yyval . leading_ws = 0;  }
    break;

  case 3: /* parse: source "end of source"  */
                                { *root = MakeTree ( ctx, PT_PARSE, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) );    yyval . subtree = 0; yyval . leading_ws = 0; }
    break;

  case 4: /* source: schema_1_0  */
                                    { yyval . subtree = MakeTree ( ctx, PT_SOURCE, P ( yyvsp[0] ) ); }
    break;

  case 5: /* source: version_1_0 schema_1_0_opt  */
                                    { yyval . subtree = MakeTree ( ctx, PT_SOURCE, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 6: /* source: version_2_0 schema_2_0_opt  */
                                    { yyval . subtree = MakeTree ( ctx, PT_SOURCE, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 7: /* version_1_0: KW_version VERS_1_0 ';'  */
                                { yyval . subtree = MakeTree ( ctx, PT_VERSION_1_0, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 8: /* schema_1_0_opt: schema_1_0  */
                    { yyval . subtree = P ( yyvsp[0] ); }
    break;

  case 9: /* schema_1_0_opt: empty  */
                    { yyval = yyvsp[0]; }
    break;

  case 10: /* schema_1_0: schema_1_0_decl  */
                                   { yyval . subtree = MakeTree ( ctx, PT_SCHEMA_1_0, P ( yyvsp[0] ) ); }
    break;

  case 11: /* schema_1_0: schema_1_0 schema_1_0_decl  */
                                   { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ) , P ( yyvsp[0] ) ); }
    break;

  case 12: /* schema_1_0_decl: typedef_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 13: /* schema_1_0_decl: typeset_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 14: /* schema_1_0_decl: format_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 15: /* schema_1_0_decl: const_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 16: /* schema_1_0_decl: alias_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 17: /* schema_1_0_decl: function_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 18: /* schema_1_0_decl: extern_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 19: /* schema_1_0_decl: script_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 20: /* schema_1_0_decl: validate_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 21: /* schema_1_0_decl: physical_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 22: /* schema_1_0_decl: table_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 23: /* schema_1_0_decl: database_1_0_decl  */
                            { yyval = yyvsp[0]; }
    break;

  case 24: /* schema_1_0_decl: include_directive  */
                            { yyval = yyvsp[0]; }
    break;

  case 25: /* schema_1_0_decl: ';'  */
                            { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 26: /* typedef_1_0_decl: KW_typedef fqn_1_0 typedef_1_0_new_name_list ';'  */
                            { yyval . subtree = MakeTree ( ctx, PT_TYPEDEF, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 27: /* typedef_1_0_new_name_list: typespec_1_0  */
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 28: /* typedef_1_0_new_name_list: typedef_1_0_new_name_list ',' typespec_1_0  */
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 29: /* typeset_1_0_decl: KW_typeset typeset_1_0_new_name typeset_1_0_def ';'  */
                            { yyval . subtree = MakeTree ( ctx, PT_TYPESET, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 30: /* typeset_1_0_new_name: fqn_1_0  */
                            { yyval = yyvsp[0]; }
    break;

  case 31: /* typeset_1_0_def: '{' typespec_1_0_list '}'  */
            { yyval . subtree = MakeTree ( ctx, PT_TYPESETDEF, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 32: /* typespec_1_0_list: typespec_1_0  */
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 33: /* typespec_1_0_list: typespec_1_0_list ',' typespec_1_0  */
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 34: /* typespec_1_0: fqn_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 35: /* typespec_1_0: fqn_1_0 '[' dim_1_0 ']'  */
                                { yyval . subtree = MakeTree ( ctx, PT_ARRAY, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 36: /* dim_1_0: expression_1_0  */
                        { yyval = yyvsp[0]; }
    break;

  case 37: /* dim_1_0: '*'  */
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 38: /* format_1_0_decl: KW_fmtdef format_1_0_new_name ';'  */
                                    { yyval . subtree = MakeTree ( ctx, PT_FORMAT, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 39: /* format_1_0_decl: KW_fmtdef format_1_0_name format_1_0_new_name ';'  */
                                    { yyval . subtree = MakeTree ( ctx, PT_FORMAT, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 40: /* format_1_0_new_name: fqn_1_0  */
                                    { yyval = yyvsp[0]; }
    break;

  case 41: /* format_1_0_name: fqn_1_0  */
                                    { yyval = yyvsp[0]; }
    break;

  case 42: /* const_1_0_decl: KW_const typespec_1_0 fqn_1_0 '=' expression_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_CONST, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 43: /* alias_1_0_decl: KW_alias fqn_1_0 alias_1_0_new_name ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_ALIAS, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 44: /* alias_1_0_new_name: fqn_1_0  */
                                    { yyval = yyvsp[0]; }
    break;

  case 45: /* function_1_0_decl: KW_function func_1_0_decl  */
                                     { yyval . subtree = MakeTree ( ctx, PT_FUNCTION, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 46: /* func_1_0_decl: untyped_func_1_0_decl  */
                                     { yyval = yyvsp[0]; }
    break;

  case 47: /* func_1_0_decl: row_length_func_1_0_decl  */
                                     { yyval = yyvsp[0]; }
    break;

  case 48: /* func_1_0_decl: opt_func_1_0_schema_sig func_1_0_return_type fqn_opt_vers opt_func_1_0_fact_sig func_1_0_param_sig func_1_0_prologue  */
            { yyval . subtree = MakeTree ( ctx, PT_FUNCDECL, P ( yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 49: /* untyped_func_1_0_decl: KW___untyped fqn_1_0 '(' ')'  */
            { yyval . subtree = MakeTree ( ctx, PT_UNTYPED, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 50: /* row_length_func_1_0_decl: KW___row_length fqn_1_0 '(' ')'  */
            { yyval . subtree = MakeTree ( ctx, PT_ROWLENGTH, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 51: /* opt_func_1_0_schema_sig: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 52: /* opt_func_1_0_schema_sig: func_1_0_schema_sig  */
                                { yyval = yyvsp[0]; }
    break;

  case 53: /* func_1_0_schema_sig: '<' func_1_0_schema_formals '>'  */
                                            { yyval . subtree = MakeTree ( ctx, PT_SCHEMASIG, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 54: /* func_1_0_schema_formals: func_1_0_schema_formal  */
                                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 55: /* func_1_0_schema_formals: func_1_0_schema_formals ',' func_1_0_schema_formal  */
                                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 56: /* func_1_0_schema_formal: KW_type ident_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_SCHEMAFORMAL, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 57: /* func_1_0_schema_formal: type_expr_1_0 ident_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_SCHEMAFORMAL, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 58: /* func_1_0_return_type: KW_void  */
                        { yyval . subtree = MakeTree ( ctx, PT_RETURNTYPE, T ( ctx, yyvsp[0] ) ); }
    break;

  case 59: /* func_1_0_return_type: type_expr_1_0  */
                        { yyval . subtree = MakeTree ( ctx, PT_RETURNTYPE, P ( yyvsp[0] ) ); }
    break;

  case 60: /* opt_func_1_0_fact_sig: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 61: /* opt_func_1_0_fact_sig: func_1_0_fact_sig  */
                                { yyval = yyvsp[0]; }
    break;

  case 62: /* func_1_0_fact_sig: '<' func_1_0_fact_signature '>'  */
                                       { yyval . subtree = MakeTree ( ctx, PT_FACTSIG, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 63: /* func_1_0_fact_signature: empty  */
                            { yyval = yyvsp[0]; }
    break;

  case 64: /* func_1_0_fact_signature: func_1_0_fact_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 65: /* func_1_0_fact_signature: '*' func_1_0_fact_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 66: /* func_1_0_fact_signature: func_1_0_fact_params '*' func_1_0_fact_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 67: /* func_1_0_fact_signature: func_1_0_fact_params ',' '*' func_1_0_fact_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 68: /* func_1_0_fact_params: fact_param_1_0  */
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 69: /* func_1_0_fact_params: func_1_0_fact_params ',' fact_param_1_0  */
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 70: /* fact_param_1_0: typespec_1_0 IDENTIFIER_1_0  */
                                        { yyval . subtree = MakeTree ( ctx, PT_FORMALPARAM, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 71: /* func_1_0_param_sig: '(' func_1_0_param_signature ')'  */
                                        { yyval . subtree = MakeTree ( ctx, PT_FUNCSIG, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 72: /* func_1_0_param_signature: empty  */
                            { yyval = yyvsp[0]; }
    break;

  case 73: /* func_1_0_param_signature: func_1_0_formal_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 74: /* func_1_0_param_signature: '*' func_1_0_formal_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 75: /* func_1_0_param_signature: func_1_0_formal_params '*' func_1_0_formal_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 76: /* func_1_0_param_signature: func_1_0_formal_params ',' '*' func_1_0_formal_params func_1_0_vararg_formals  */
                            { yyval . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 77: /* func_1_0_formal_params: formal_param_1_0  */
                                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 78: /* func_1_0_formal_params: func_1_0_formal_params ',' formal_param_1_0  */
                                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 79: /* formal_param_1_0: typespec_1_0 IDENTIFIER_1_0  */
                                        { yyval . subtree = MakeTree ( ctx, PT_FORMALPARAM, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 80: /* formal_param_1_0: KW_control typespec_1_0 IDENTIFIER_1_0  */
                                        { yyval . subtree = MakeTree ( ctx, PT_FORMALPARAM, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 81: /* func_1_0_vararg_formals: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 82: /* func_1_0_vararg_formals: ',' ELLIPSIS  */
                                { yyval . subtree = MakeTree ( ctx, PT_ELLIPSIS, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 83: /* func_1_0_prologue: ';'  */
                                { yyval . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, yyvsp[0] ) ); }
    break;

  case 84: /* func_1_0_prologue: '=' fqn_1_0 ';'  */
                                { yyval . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 85: /* func_1_0_prologue: '{' script_1_0_stmt_seq '}'  */
                                { yyval . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 86: /* script_1_0_stmt_seq: script_1_0_stmt  */
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 87: /* script_1_0_stmt_seq: script_1_0_stmt_seq script_1_0_stmt  */
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 88: /* script_1_0_stmt: KW_return cond_expr_1_0 ';'  */
                                    { yyval . subtree = MakeTree ( ctx, PT_RETURN, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 89: /* script_1_0_stmt: type_expr_1_0 ident_1_0 '=' cond_expr_1_0 ';'  */
                                     { yyval . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 90: /* extern_1_0_decl: KW_extern ext_func_1_0_decl  */
            { yyval . subtree = MakeTree ( ctx, PT_EXTERN, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 91: /* ext_func_1_0_decl: function_1_0_decl  */
                                    { yyval = yyvsp[0]; }
    break;

  case 92: /* script_1_0_decl: KW_schema func_1_0_decl  */
                            { yyval . subtree = MakeTree ( ctx, PT_SCHEMA, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 93: /* script_1_0_decl: KW_schema KW_function func_1_0_decl  */
                            { yyval . subtree = MakeTree ( ctx, PT_SCHEMA, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 94: /* validate_1_0_decl: KW_validate function_1_0_decl  */
                            { yyval . subtree = MakeTree ( ctx, PT_VALIDATE, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 95: /* physical_1_0_decl: KW_physical opt_func_1_0_schema_sig phys_1_0_return_type fqn_vers opt_func_1_0_fact_sig phys_1_0_prologue  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSICAL, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 96: /* phys_1_0_return_type: func_1_0_return_type  */
                                            { yyval = yyvsp[0]; }
    break;

  case 97: /* phys_1_0_return_type: KW___no_header func_1_0_return_type  */
                                            { yyval . subtree = MakeTree ( ctx, PT_NOHEADER, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 98: /* phys_1_0_prologue: '=' phys_1_0_stmt  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSPROLOGUE, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 99: /* phys_1_0_prologue: '{' phys_1_0_body '}'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSPROLOGUE, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 100: /* phys_1_0_body: phys_1_0_body_stmt  */
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 101: /* phys_1_0_body: phys_1_0_body phys_1_0_body_stmt  */
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 102: /* phys_1_0_body_stmt: ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 103: /* phys_1_0_body_stmt: KW_decode phys_1_0_stmt  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 104: /* phys_1_0_body_stmt: KW_encode phys_1_0_stmt  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 105: /* phys_1_0_body_stmt: KW___row_length '=' fqn_1_0 '(' ')'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 106: /* phys_1_0_stmt: '{' script_1_0_stmt_seq '}'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSSTMT, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 107: /* table_1_0_decl: KW_table fqn_vers opt_tbl_1_0_parents tbl_1_0_body  */
            { yyval . subtree = MakeTree ( ctx, PT_TABLE, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 108: /* opt_tbl_1_0_parents: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 109: /* opt_tbl_1_0_parents: '=' tbl_1_0_parents  */
                                { yyval . subtree = MakeTree ( ctx, PT_TABLEPARENTS, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 110: /* tbl_1_0_parents: fqn_opt_vers  */
                                          { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 111: /* tbl_1_0_parents: tbl_1_0_parents ',' fqn_opt_vers  */
                                          { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 112: /* tbl_1_0_body: '{' tbl_1_0_stmt_seq '}'  */
            { yyval . subtree = MakeTree ( ctx, PT_TABLEBODY, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 113: /* tbl_1_0_body: '{' '}'  */
            { yyval . subtree = MakeTree ( ctx, PT_TABLEBODY, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 114: /* tbl_1_0_stmt_seq: tbl_1_0_stmt  */
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 115: /* tbl_1_0_stmt_seq: tbl_1_0_stmt_seq tbl_1_0_stmt  */
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 116: /* tbl_1_0_stmt: production_1_0_stmt  */
                                                { yyval = yyvsp[0]; }
    break;

  case 117: /* tbl_1_0_stmt: col_1_0_modifiers col_1_0_decl  */
                                                { yyval . subtree = MakeTree ( ctx, PT_COLUMN, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 118: /* tbl_1_0_stmt: KW_column KW_limit '=' expression_1_0 ';'  */
                        { yyval . subtree = MakeTree ( ctx, PT_COLUMNEXPR, T ( ctx, yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 119: /* tbl_1_0_stmt: KW_column KW_default KW_limit '=' expression_1_0 ';'  */
                        { yyval . subtree = MakeTree ( ctx, PT_COLUMNEXPR, T ( ctx, yyvsp[-5] ), T ( ctx, yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 120: /* tbl_1_0_stmt: KW_static physmbr_1_0_decl  */
                                                { yyval . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 121: /* tbl_1_0_stmt: KW_physical physmbr_1_0_decl  */
                                                { yyval . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 122: /* tbl_1_0_stmt: KW_static KW_physical physmbr_1_0_decl  */
                                                { yyval . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 123: /* tbl_1_0_stmt: default_view_1_0_decl  */
                                                { yyval = yyvsp[0]; }
    break;

  case 124: /* tbl_1_0_stmt: KW___untyped '=' fqn_1_0 '(' ')' ';'  */
                        { yyval . subtree = MakeTree ( ctx, PT_COLUNTYPED, T ( ctx, yyvsp[-5] ), T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 125: /* tbl_1_0_stmt: ';'  */
                                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 126: /* production_1_0_stmt: typespec_1_0 ident_1_0 '=' cond_expr_1_0 ';'  */
                                     { yyval . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 127: /* production_1_0_stmt: KW_trigger ident_1_0 '=' cond_expr_1_0 ';'  */
                                     { yyval . subtree = MakeTree ( ctx, PT_PRODTRIGGER, T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 128: /* col_1_0_modifiers: KW_column  */
                                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 129: /* col_1_0_modifiers: col_1_0_modifier_seq KW_column  */
                                        { yyval = yyvsp[-1]; AddToList ( ctx, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 130: /* col_1_0_modifier_seq: col_1_0_modifier  */
                                                { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 131: /* col_1_0_modifier_seq: col_1_0_modifier_seq col_1_0_modifier  */
                                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 132: /* col_1_0_modifier: KW_default  */
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 133: /* col_1_0_modifier: KW_extern  */
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 134: /* col_1_0_modifier: KW_readonly  */
                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 135: /* col_1_0_decl: typespec_1_0 typed_column_decl_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_COLDECL, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 136: /* col_1_0_decl: phys_enc_ref typed_column_decl_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_COLDECL, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 137: /* phys_enc_ref: '<' schema_parms_1_0 '>' fqn_opt_vers opt_factory_parms_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSENCREF, T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 138: /* phys_enc_ref: fqn_vers opt_factory_parms_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSENCREF, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 139: /* phys_enc_ref: fqn_1_0 factory_parms_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSENCREF, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 140: /* typed_column_decl_1_0: col_ident '{' opt_column_body_1_0 '}'  */
            { yyval . subtree = MakeTree ( ctx, PT_TYPEDCOL, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 141: /* typed_column_decl_1_0: col_ident '=' cond_expr_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_TYPEDCOLEXPR, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 142: /* typed_column_decl_1_0: col_ident ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_TYPEDCOL, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 143: /* col_ident: ident_1_0  */
                                    { yyval = yyvsp[0]; }
    break;

  case 144: /* col_ident: phys_ident  */
                                    { yyval = yyvsp[0]; }
    break;

  case 145: /* phys_ident: PHYSICAL_IDENTIFIER_1_0  */
                                    { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 146: /* opt_column_body_1_0: empty  */
                        { yyval = yyvsp[0]; }
    break;

  case 147: /* opt_column_body_1_0: column_body_1_0  */
                        { yyval = yyvsp[0]; }
    break;

  case 148: /* column_body_1_0: column_stmt_1_0  */
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 149: /* column_body_1_0: column_body_1_0 column_stmt_1_0  */
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 150: /* column_stmt_1_0: KW_read '=' cond_expr_1_0 ';'  */
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 151: /* column_stmt_1_0: KW_validate '=' cond_expr_1_0 ';'  */
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 152: /* column_stmt_1_0: KW_limit '=' uint_expr_1_0 ';'  */
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 153: /* column_stmt_1_0: ';'  */
                                        { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 154: /* default_view_1_0_decl: KW_default KW_view STRING ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_DFLTVIEW, T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 155: /* physmbr_1_0_decl: phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 156: /* physmbr_1_0_decl: phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 157: /* physmbr_1_0_decl: KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 158: /* physmbr_1_0_decl: KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSMBR, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 159: /* phys_coldef_1_0: opt_col_schema_parms_1_0 fqn_opt_vers opt_factory_parms_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_PHYSCOLDEF, P ( yyvsp[-2] ), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 160: /* opt_col_schema_parms_1_0: empty  */
                                        { yyval = yyvsp[0]; }
    break;

  case 161: /* opt_col_schema_parms_1_0: '<' col_schema_parms_1_0 '>'  */
                                        { yyval . subtree = MakeTree ( ctx, PT_COLSCHEMAPARMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 162: /* col_schema_parms_1_0: col_schema_parm_1_0  */
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 163: /* col_schema_parms_1_0: col_schema_parms_1_0 ',' col_schema_parm_1_0  */
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 164: /* col_schema_parm_1_0: fqn_1_0 '=' col_schema_value_1_0  */
                                            { yyval . subtree = MakeTree ( ctx, PT_COLSCHEMAPARAM, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 165: /* col_schema_parm_1_0: col_schema_value_1_0  */
                                            { yyval = yyvsp[0]; }
    break;

  case 166: /* col_schema_value_1_0: fqn_1_0  */
                                            { yyval = yyvsp[0]; }
    break;

  case 167: /* col_schema_value_1_0: uint_expr_1_0  */
                                            { yyval = yyvsp[0]; }
    break;

  case 168: /* cond_expr_1_0: expression_1_0  */
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 169: /* cond_expr_1_0: cond_expr_1_0 '|' expression_1_0  */
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 170: /* expression_1_0: primary_expr_1_0  */
                                            { yyval = yyvsp[0]; }
    break;

  case 171: /* expression_1_0: '(' type_expr_1_0 ')' expression_1_0  */
                                            { yyval . subtree = MakeTree ( ctx, PT_CASTEXPR, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 172: /* primary_expr_1_0: fqn_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 173: /* primary_expr_1_0: phys_ident  */
                                { yyval = yyvsp[0]; }
    break;

  case 174: /* primary_expr_1_0: '@'  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 175: /* primary_expr_1_0: func_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 176: /* primary_expr_1_0: uint_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 177: /* primary_expr_1_0: float_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 178: /* primary_expr_1_0: string_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 179: /* primary_expr_1_0: const_vect_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 180: /* primary_expr_1_0: bool_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 181: /* primary_expr_1_0: '-' expression_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_NEGATE, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 182: /* primary_expr_1_0: '+' expression_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_UNARYPLUS, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 183: /* primary_expr_1_0: member_expr_2_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 184: /* primary_expr_1_0: join_expr_2_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 185: /* func_expr_1_0: '<' schema_parms_1_0 '>' fqn_opt_vers opt_factory_parms_1_0 '(' opt_func_1_0_parms ')'  */
             { yyval . subtree = MakeTree ( ctx, PT_FUNCEXPR, T ( ctx, yyvsp[-7] ), P ( yyvsp[-6] ), T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 186: /* func_expr_1_0: fqn_opt_vers opt_factory_parms_1_0 '(' opt_func_1_0_parms ')'  */
             { yyval . subtree = MakeTree ( ctx, PT_FUNCEXPR, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 187: /* schema_parms_1_0: schema_parm_1_0  */
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 188: /* schema_parms_1_0: schema_parms_1_0 ',' schema_parm_1_0  */
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 189: /* schema_parm_1_0: fqn_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 190: /* schema_parm_1_0: fqn_1_0 '[' dim_1_0 ']'  */
                                { yyval . subtree = MakeTree ( ctx, PT_ARRAY, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 191: /* schema_parm_1_0: uint_expr_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 192: /* opt_factory_parms_1_0: empty  */
                        { yyval = yyvsp[0]; }
    break;

  case 193: /* opt_factory_parms_1_0: factory_parms_1_0  */
                        { yyval = yyvsp[0]; }
    break;

  case 194: /* factory_parms_1_0: '<' factory_parms '>'  */
                            { yyval . subtree = MakeTree ( ctx, PT_FACTPARMS, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 195: /* factory_parms: expression_1_0  */
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 196: /* factory_parms: factory_parms ',' expression_1_0  */
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 197: /* opt_func_1_0_parms: empty  */
                                            { yyval = yyvsp[0]; }
    break;

  case 198: /* opt_func_1_0_parms: func_1_0_parms  */
                                            { yyval = yyvsp[0]; }
    break;

  case 199: /* func_1_0_parms: expression_1_0  */
                                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 200: /* func_1_0_parms: func_1_0_parms ',' expression_1_0  */
                                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 201: /* uint_expr_1_0: DECIMAL  */
                                            { yyval . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 202: /* uint_expr_1_0: HEX  */
                                            { yyval . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 203: /* uint_expr_1_0: OCTAL  */
                                            { yyval . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 204: /* float_expr_1_0: FLOAT_  */
                                 { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 205: /* float_expr_1_0: EXP_FLOAT  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 206: /* string_expr_1_0: STRING  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 207: /* string_expr_1_0: ESCAPED_STRING  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 208: /* const_vect_expr_1_0: '[' opt_const_vect_exprlist_1_0 ']'  */
                                               { yyval . subtree = MakeTree ( ctx, PT_CONSTVECT, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 209: /* opt_const_vect_exprlist_1_0: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 210: /* opt_const_vect_exprlist_1_0: const_vect_exprlist_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 211: /* const_vect_exprlist_1_0: expression_1_0  */
                                                    { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 212: /* const_vect_exprlist_1_0: const_vect_exprlist_1_0 ',' expression_1_0  */
                                                    { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 213: /* bool_expr_1_0: KW_true  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 214: /* bool_expr_1_0: KW_false  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 215: /* type_expr_1_0: typespec_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 216: /* type_expr_1_0: fqn_1_0 '/' fqn_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_TYPEEXPR, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1]), P ( yyvsp[0] ) ); }
    break;

  case 217: /* member_expr_2_0: ident_1_0 '.' ident_1_0  */
                                        { yyval . subtree = MakeTree ( ctx, PT_MEMBEREXPR, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 218: /* member_expr_2_0: ident_1_0 PHYSICAL_IDENTIFIER_1_0  */
                                        { yyval . subtree = MakeTree ( ctx, PT_MEMBEREXPR, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 219: /* join_expr_2_0: ident_1_0 '[' cond_expr_1_0 ']' '.' ident_1_0  */
        { yyval . subtree = MakeTree ( ctx, PT_JOINEXPR, P ( yyvsp[-5] ), T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P (yyvsp[0] ) ); }
    break;

  case 220: /* join_expr_2_0: ident_1_0 '[' cond_expr_1_0 ']' PHYSICAL_IDENTIFIER_1_0  */
        { yyval . subtree = MakeTree ( ctx, PT_JOINEXPR, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 221: /* database_1_0_decl: KW_database fqn_vers opt_database_dad_1_0 database_body_1_0  */
            { yyval . subtree = MakeTree ( ctx, PT_DATABASE, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2]), P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 222: /* opt_database_dad_1_0: empty  */
                                    { yyval = yyvsp[0]; }
    break;

  case 223: /* opt_database_dad_1_0: '=' fqn_opt_vers  */
                                    { yyval . subtree = MakeTree ( ctx, PT_DBDAD, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 224: /* database_body_1_0: '{' '}'  */
                                    { yyval . subtree = MakeTree ( ctx, PT_DBBODY, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 225: /* database_body_1_0: '{' database_members_1_0 '}'  */
                                    { yyval . subtree = MakeTree ( ctx, PT_DBBODY, T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 226: /* database_members_1_0: database_member_1_0  */
                                                { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 227: /* database_members_1_0: database_members_1_0 database_member_1_0  */
                                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 228: /* database_member_1_0: db_member_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 229: /* database_member_1_0: table_member_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 230: /* database_member_1_0: alias_member_1_0  */
                                { yyval = yyvsp[0]; }
    break;

  case 231: /* database_member_1_0: ';'  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 232: /* opt_template_1_0: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 233: /* opt_template_1_0: KW_template  */
                                { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 234: /* db_member_1_0: opt_template_1_0 KW_database fqn_opt_vers ident_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_DBMEMBER, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 235: /* table_member_1_0: opt_template_1_0 KW_table fqn_opt_vers ident_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_TBLMEMBER, P ( yyvsp[-4] ), T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 236: /* alias_member_1_0: KW_alias view_spec ident_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_ALIASMEMBER, T ( ctx, yyvsp[-3] ), P ( yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 237: /* view_spec: fqn_opt_vers '<' view_parms '>'  */
        { yyval . subtree = MakeTree ( ctx, PT_VIEWSPEC, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 238: /* view_parms: ident_1_0  */
                               { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 239: /* view_parms: view_parms ',' ident_1_0  */
                               { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 240: /* include_directive: KW_include STRING  */
                            { yyval . subtree = MakeTree ( ctx, PT_INCLUDE, T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 241: /* fqn_1_0: ident_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_FQN, P ( yyvsp[0] ) ); }
    break;

  case 242: /* fqn_1_0: fqn_1_0 ':' ident_1_0  */
                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 243: /* fqn_1_0: fqn_1_0 ':' KW_database  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 244: /* fqn_1_0: fqn_1_0 ':' KW_decode  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 245: /* fqn_1_0: fqn_1_0 ':' KW_encode  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 246: /* fqn_1_0: fqn_1_0 ':' KW_read  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 247: /* fqn_1_0: fqn_1_0 ':' KW_table  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 248: /* fqn_1_0: fqn_1_0 ':' KW_type  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 249: /* fqn_1_0: fqn_1_0 ':' KW_view  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 250: /* fqn_1_0: fqn_1_0 ':' KW_write  */
                                { yyvsp[0] . type = IDENTIFIER_1_0; yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ) ); }
    break;

  case 251: /* ident_1_0: IDENTIFIER_1_0  */
                        { yyval . subtree = MakeTree ( ctx, PT_IDENT, T ( ctx, yyvsp[0] ) ); }
    break;

  case 252: /* empty: %empty  */
                { yyval . subtree = MakeTree ( ctx, PT_EMPTY ); }
    break;

  case 253: /* fqn_vers: fqn_1_0 VERSION  */
                            { yyval . subtree = MakeTree ( ctx, PT_VERSNAME, P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 254: /* fqn_opt_vers: fqn_1_0  */
                     { yyval = yyvsp[0]; }
    break;

  case 255: /* fqn_opt_vers: fqn_vers  */
                     { yyval = yyvsp[0]; }
    break;

  case 256: /* version_2_0: KW_version VERS_2_0 ';'  */
                                   { yyval . subtree = MakeTree ( ctx, PT_VERSION_2, T ( ctx, yyvsp[-2] ), T ( ctx, yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 257: /* schema_2_0_opt: schema_2_0  */
                    { yyval . subtree = P ( yyvsp[0] ); }
    break;

  case 258: /* schema_2_0_opt: empty  */
                    { yyval = yyvsp[0]; }
    break;

  case 259: /* schema_2_0: schema_2_0_decl  */
                                   { yyval . subtree = MakeTree ( ctx, PT_SCHEMA_2_0, P ( yyvsp[0] ) ); }
    break;

  case 260: /* schema_2_0: schema_2_0 schema_2_0_decl  */
                                   { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ) , P ( yyvsp[0] ) ); }
    break;

  case 261: /* schema_2_0_decl: schema_1_0_decl  */
                        { yyval = yyvsp[0]; }
    break;

  case 262: /* schema_2_0_decl: view_2_0_decl  */
                        { yyval = yyvsp[0]; }
    break;

  case 263: /* view_2_0_decl: KW_view fqn_vers '<' view_parms '>' opt_view_parents '{' opt_view_body '}'  */
        { yyval . subtree = MakeTree ( ctx, PT_VIEW, T ( ctx, yyvsp[-8] ), P ( yyvsp[-7] ), T ( ctx, yyvsp[-6] ), P ( yyvsp[-5] ), T ( ctx, yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 264: /* view_parms: view_parm  */
                                { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 265: /* view_parms: view_parms ',' view_parm  */
                                { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 266: /* view_parm: fqn_opt_vers ident_1_0  */
                                { yyval . subtree = MakeTree ( ctx, PT_VIEWPARAM, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 267: /* opt_view_body: empty  */
                { yyval = yyvsp[0]; }
    break;

  case 268: /* opt_view_body: view_body  */
                { yyval = yyvsp[0]; }
    break;

  case 269: /* view_body: view_member  */
                            { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 270: /* view_body: view_body view_member  */
                            { yyval . subtree = AddToList ( ctx, P ( yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 271: /* view_member: typespec_1_0 ident_1_0 '=' cond_expr_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 272: /* view_member: KW_column typespec_1_0 ident_1_0 '=' cond_expr_1_0 ';'  */
            { yyval . subtree = MakeTree ( ctx, PT_COLUMN, T ( ctx, yyvsp[-5] ), P ( yyvsp[-4] ), P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 273: /* view_member: ';'  */
            { yyval . subtree = T ( ctx, yyvsp[0] ); }
    break;

  case 274: /* opt_view_parents: empty  */
                                { yyval = yyvsp[0]; }
    break;

  case 275: /* opt_view_parents: '=' view_parents  */
                                { yyval . subtree = MakeTree ( ctx, PT_VIEWPARENTS, T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 276: /* view_parents: view_parent  */
                                   { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 277: /* view_parents: view_parents ',' view_parent  */
                                   { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
    break;

  case 278: /* view_parent: fqn_opt_vers '<' view_parent_parms '>'  */
        { yyval . subtree = MakeTree ( ctx, PT_VIEWPARENT, P ( yyvsp[-3] ), T ( ctx, yyvsp[-2] ), P ( yyvsp[-1] ), T ( ctx, yyvsp[0] ) ); }
    break;

  case 279: /* view_parent_parms: ident_1_0  */
                                        { yyval . subtree = MakeList ( ctx, yyvsp[0] ); }
    break;

  case 280: /* view_parent_parms: view_parent_parms ',' ident_1_0  */
                                        { yyval . subtree = AddToList ( ctx, P ( yyvsp[-2] ), T ( ctx, yyvsp[-1] ), P ( yyvsp[0] ) ); }
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
  *++yylsp = yyloc;

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
  yytoken = yychar == SCHEMA_EMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
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
        yyerror (&yylloc, ctx, root, errors, sb, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
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
                      yytoken, &yylval, &yylloc, ctx, root, errors, sb);
          yychar = SCHEMA_EMPTY;
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, ctx, root, errors, sb);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

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
  yyerror (&yylloc, ctx, root, errors, sb, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != SCHEMA_EMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, ctx, root, errors, sb);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, ctx, root, errors, sb);
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

