    /*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

%{
    #define YYDEBUG 1

    #include <stdio.h>

    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    void Schema_error ( YYLTYPE *llocp, struct SchemaScanBlock* sb, const char* msg )
    {
    /*TODO: call back into the C++ parser */
    /*    sb->report_error ( sb, msg );*/
        printf("Line %i pos %i: %s\n", llocp -> first_line, llocp -> first_column, msg);
    }

%}

%name-prefix "Schema_"
%param { struct SchemaScanBlock* sb }

%define parse.error verbose
%locations

%define api.pure full

%token END_SOURCE 0 "end of source"

%token UNRECOGNIZED

%token ELLIPSIS
%token INCREMENT

%token DECIMAL
%token OCTAL
%token HEX
%token FLOAT
%token EXP_FLOAT
%token STRING
%token ESCAPED_STRING

%token IDENTIFIER_1_0
%token PHYSICAL_IDENTIFIER_1_0
%token VERSION

/* unterminated strings are dealt with by flex */
%token UNTERM_STRING
%token UNTERM_ESCAPED_STRING

%token VERS_1_0 /* recognized under special flex state */

%token KW___no_header
%token KW___row_length
%token KW___untyped
%token KW_alias
%token KW_column
%token KW_const
%token KW_control
%token KW_database
%token KW_decode
%token KW_default
%token KW_encode
%token KW_extern
%token KW_false
%token KW_fmtdef
%token KW_function
%token KW_include
%token KW_limit
%token KW_physical
%token KW_read
%token KW_readonly
%token KW_return
%token KW_schema
%token KW_static
%token KW_table
%token KW_template
%token KW_trigger
%token KW_true
%token KW_type
%token KW_typedef
%token KW_typeset
%token KW_validate
%token KW_version
%token KW_view
%token KW_virtual
%token KW_void
%token KW_write

%start parse

%%

parse
    : END_SOURCE
    | source END_SOURCE
    ;

source
    : schema_1_0
    | version_1_0 schema_1_0
    | version_2_x schema_2_x
    ;

version_1_0
    : KW_version VERS_1_0 ';'
    ;

version_2_x
    : KW_version FLOAT ';'                                       /* and check for valid version number */
    ;

schema_2_x
    : '$'                                                       /* TBD */
    ;

/* schema-1.0
 */
schema_1_0
    : schema_1_0_decl_seq
    ;

schema_1_0_decl_seq
    : schema_1_0_decl
    | schema_1_0_decl_seq schema_1_0_decl
    ;

schema_1_0_decl
    : typedef_1_0_decl
    | typeset_1_0_decl
    | format_1_0_decl
    | const_1_0_decl
    | alias_1_0_decl
    | extern_1_0_decl
    | function_1_0_decl
    | script_1_0_decl
    | validate_1_0_decl
    | physical_1_0_decl
    | table_1_0_decl
    | database_1_0_decl
    | include_directive
    | ';'                                                       /* for lots of reasons, we tolerate stray semicolons */
    ;

script_1_0_decl
    : KW_schema func_1_0_decl                                   /* MUST have a function body      */
    | KW_schema KW_function func_1_0_decl                       /* "function" keyword is optional */
    ;

validate_1_0_decl
    : KW_validate function_1_0_decl                             /* has exactly 2 parameters and is not inline */
    ;

include_directive
    : KW_include STRING
    ;

/* typedef-1.0
 */
typedef_1_0_decl
    : KW_typedef
        fqn_1_0 /* must be from 'typedef' or schema template */
        typedef_1_0_new_name_list
        ';'
    ;

typedef_1_0_new_name_list
    : typedef_1_0_new_name
    | typedef_1_0_new_name_list ',' typedef_1_0_new_name
    ;

typedef_1_0_new_name
    : fqn_1_0                                                   /* simple fqn, apparently ( but not necessarily ) scalar */
    | fqn_1_0 dim_1_0                                           /* vector fqn with dimension */
    ;

/* typeset-1.0
 */
typeset_1_0_decl
    : KW_typeset typeset_1_0_new_name typeset_1_0_def ';'
    ;

typeset_1_0_new_name
    : fqn_1_0                                                   /* we allow duplicate redefinition here... */
    ;

typeset_1_0_def
    : '{' typespec_1_0_list '}'                                 /* ...obviously, the set contents must match */
    ;

/* typespec-1.0
 */
typespec_1_0_list
    : typespec_1_0
    | typespec_1_0_list ',' typespec_1_0
    ;

typespec_1_0
    : typespec_1_0_name
    | typespec_1_0_name dim_1_0
    ;

typespec_1_0_name
    : fqn_1_0                                                   /* must be a typedef, schema type, or typeset */
    ;


/* format-1.0
 */
format_1_0_decl
    : KW_fmtdef format_1_0_new_name ';'
    | KW_fmtdef format_1_0_name format_1_0_new_name ';'       /* creates a derived format */
    ;

format_1_0_new_name
    : fqn_1_0                                                   /* allow for redefinition - current code seems to be wrong */
    ;

format_1_0_name
    : fqn_1_0                                                   /* must name a format */
    ;


/* const-1.0
 */
const_1_0_decl
    : KW_const fqn_1_0 const_1_0_new_name '=' expression_1_0 ';'
    | KW_const fqn_1_0 dim_1_0 const_1_0_new_name '=' expression_1_0 ';'
    ;

const_1_0_new_name
    : fqn_1_0                                                   /* also supposed to allow for duplicate redefinition */
    ;


/* alias-1.0
 */
alias_1_0_decl
    : KW_alias fqn_1_0 alias_1_0_new_name ';'
    ;

alias_1_0_new_name
    : fqn_1_0                                                   /* for some reason, does not allow duplicate redefinition */
    ;


/* extern-1.0
 */
extern_1_0_decl
    : KW_extern ext_func_1_0_decl                               /* supposed to be an extern C function */
    ;

ext_func_1_0_decl
    : function_1_0_decl                                         /* there are restrictions and potentially additions */
    ;


/* function-1.0
 */
function_1_0_decl
    : KW_function func_1_0_decl
    ;

func_1_0_decl
    : untyped_func_1_0_decl                                 /* cannot be inline or a validation function */
    | row_length_func_1_0_decl                              /* cannot be inline or a validation function */
    | opt_func_1_0_schema_sig
      func_1_0_return_type func_1_0_new_name
      opt_func_1_0_fact_sig
      func_1_0_param_sig
      func_1_0_prologue
    ;

untyped_func_1_0_decl
    : KW___untyped func_1_0_new_name '(' ')'
    ;

row_length_func_1_0_decl
    : KW___row_length func_1_0_new_name '(' ')'
    ;

func_1_0_new_name
    : fqn_1_0
    | fqn_1_0 VERSION
    ;

opt_func_1_0_schema_sig
    : %empty
    | func_1_0_schema_sig
    ;

func_1_0_schema_sig
    : '<' func_1_0_schema_formals '>'
    ;

func_1_0_schema_formals
    : func_1_0_schema_formal
    | func_1_0_schema_formals ',' func_1_0_schema_formal
    ;

func_1_0_schema_formal
    : KW_type ident_1_0
    | type_expr_1_0 ident_1_0   /* type-expr must be uint */
    ;

func_1_0_return_type
    : KW_void
    | type_expr_1_0
    ;

opt_func_1_0_fact_sig
    : %empty
    | func_1_0_fact_sig
    ;

func_1_0_fact_sig
    : '<' func_1_0_param_signature '>'
    ;

func_1_0_param_sig
    : '(' func_1_0_param_signature ')'
    ;

func_1_0_param_signature
    : %empty
    | func_1_0_formal_params func_1_0_vararg_formals
    | '*' func_1_0_formal_params func_1_0_vararg_formals
    | func_1_0_formal_params '*' func_1_0_formal_params func_1_0_vararg_formals
    | func_1_0_formal_params ',' '*' func_1_0_formal_params func_1_0_vararg_formals
    ;

func_1_0_formal_params
    : formal_param_1_0
    | func_1_0_formal_params ',' formal_param_1_0
    ;

formal_param_1_0
    : opt_control_1_0 typespec_1_0 IDENTIFIER_1_0
    ;

opt_control_1_0
    : %empty
    | KW_control
    ;

func_1_0_vararg_formals
    : %empty
    | ',' ELLIPSIS
    ;

func_1_0_prologue
    : ';'                                                       /* this is a simple external function declaration    */
    | '=' fqn_1_0';'                                            /* rename the function declaration with fqn          */
    | '{' func_1_0_body '}'                                     /* this is a "script" function - cannot be extern!   */
    ;

func_1_0_body
    : script_1_0_stmt_seq
    ;

script_1_0_stmt_seq
    : script_1_0_stmt
    | script_1_0_stmt_seq script_1_0_stmt
    ;

script_1_0_stmt
    : KW_return cond_expr_1_0 ';'
    | production_1_0_stmt ';'
    ;

production_1_0_stmt
    : type_expr_1_0 IDENTIFIER_1_0 assign_expr_1_0      /* cannot have format */
    | KW_trigger    IDENTIFIER_1_0 assign_expr_1_0
    ;

/* physical encoding
 */
physical_1_0_decl
    : KW_physical phys_1_0_decl
    ;

phys_1_0_decl
    : opt_func_1_0_schema_sig
      phys_1_0_return_type phys_1_0_new_name
      opt_func_1_0_fact_sig
      phys_1_0_prologue
    ;

phys_1_0_return_type
    : func_1_0_return_type
    | KW___no_header func_1_0_return_type                       /* not supported with schema signature */
    ;

phys_1_0_new_name
    : fqn_1_0 VERSION
    ;

phys_1_0_prologue
    : '=' phys_1_0_stmt                                         /* shorthand for decode-only rules */
    | '{' phys_1_0_body '}'
    ;

phys_1_0_body
    : phys_1_0_body_stmt
    | phys_1_0_body phys_1_0_body_stmt
    ;

phys_1_0_body_stmt
    : ';'
    | KW_decode phys_1_0_stmt
    | KW_encode phys_1_0_stmt
    | KW___row_length '=' fqn_1_0 '(' ')'
    ;

phys_1_0_stmt
    : '{' func_1_0_body '}'                                     /* with caveat that magic parameter "@" is defined */
    ;

/* table
 */
table_1_0_decl
    : KW_table new_tbl_1_0_name
      opt_tbl_1_0_parents
      tbl_1_0_body
    ;

new_tbl_1_0_name
    : fqn_1_0 VERSION
    ;

opt_tbl_1_0_parents
    : %empty
    | '=' tbl_1_0_parents
    ;

tbl_1_0_parents
    : tbl_1_0_parent
    | tbl_1_0_parents ',' tbl_1_0_parent
    ;

tbl_1_0_parent
    : fqn_1_0
    | fqn_1_0 VERSION
    ;

tbl_1_0_body
    : '{' tbl_1_0_stmt_seq '}'
    | '{' '}'
    ;

tbl_1_0_stmt_seq
    : tbl_1_0_stmt
    | tbl_1_0_stmt_seq tbl_1_0_stmt
    ;

tbl_1_0_stmt
    : production_1_0_stmt ';'
    | column_1_0_decl
    | col_1_0_modifier_seq column_1_0_decl
    | default_view_1_0_decl ';'
    | KW_static physmbr_1_0_decl ';'
    | KW_physical physmbr_1_0_decl ';'
    | KW_static KW_physical physmbr_1_0_decl ';'
    | KW___untyped untyped_tbl_expr_1_0 ';'
    | ';'
    ;

col_1_0_modifier_seq
    : col_1_0_modifier
    | col_1_0_modifier_seq col_1_0_modifier
    ;

col_1_0_modifier
    : KW_default
    | KW_extern
    | KW_readonly
    ;

column_1_0_decl
    : KW_column col_1_0_decl
    | KW_column KW_default col_1_0_decl
    | KW_column KW_limit '=' expression_1_0 ';'
    | KW_column KW_default KW_limit '=' expression_1_0 ';'
    ;

col_1_0_decl
    : KW_physical '<' schema_parms_1_0 '>' fqn_1_0 opt_version_1_0 opt_factory_parms_1_0 typed_column_decl_1_0
    |             '<' schema_parms_1_0 '>' fqn_1_0 opt_version_1_0 opt_factory_parms_1_0 typed_column_decl_1_0
    | fqn_1_0 VERSION opt_factory_parms_1_0 typed_column_decl_1_0
    | fqn_1_0 '<' factory_parms_1_0 '>' typed_column_decl_1_0
    | typespec_1_0 typed_column_decl_1_0
    ;

typed_column_decl_1_0
    : ident_1_0 '{' column_body_1_0 '}'
    | ident_1_0 '=' cond_expr_1_0 ';'
    | ident_1_0 ';'
    ;

column_body_1_0
    : column_stmt_1_0
    | column_body_1_0 ';' column_stmt_1_0
    ;

column_stmt_1_0
    : KW_read '=' cond_expr_1_0
    | KW_validate '=' cond_expr_1_0
    | KW_limit '=' uint_expr_1_0
    | %empty
    ;

default_view_1_0_decl
    : KW_default KW_view STRING
    ;

physmbr_1_0_decl
    : opt_KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0
    | opt_KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0
    ;

opt_KW_column
    : %empty
    | KW_column
    ;

phys_coldef_1_0
    : opt_col_schema_parms_1_0 fqn_1_0 opt_version_1_0 opt_factory_parms_1_0
    ;

opt_col_schema_parms_1_0
    : %empty
    | '<' col_schema_parms_1_0 '>'
    ;

col_schema_parms_1_0
    : col_schema_parm_1_0
    | col_schema_parms_1_0 ',' col_schema_parm_1_0
    ;

col_schema_parm_1_0
    : fqn_1_0 '=' col_schema_value_1_0
    | col_schema_value_1_0
    ;

col_schema_value_1_0
    : fqn_1_0
    | uint_expr_1_0
    ;

untyped_tbl_expr_1_0
    : '=' fqn_1_0 '(' ')'


/* expression
 */

cond_expr_1_0
    : expression_1_0
    | cond_expr_1_0 '|' expression_1_0
    ;

expression_1_0
    : primary_expr_1_0
    | '(' type_expr_1_0 ')' expression_1_0
    ;

primary_expr_1_0
    : fqn_1_0
    | '@'
    | func_expr_1_0
    | uint_expr_1_0
    | float_expr_1_0
    | string_expr_1_0
    | const_vect_expr_1_0
    | bool_expr_1_0
    | negate_expr_1_0
    | '+' expression_1_0
    ;

assign_expr_1_0
    : '=' cond_expr_1_0
    ;

func_expr_1_0
    :   '<' schema_parms_1_0 '>'
        func_1_0_name
        opt_factory_parms_1_0
        '(' opt_func_1_0_parms ')'
    |   func_1_0_name
        opt_factory_parms_1_0
        '(' opt_func_1_0_parms ')'
    ;

schema_parms_1_0
    : schema_parm_1_0
    | schema_parms_1_0 ',' schema_parm_1_0
    ;

schema_parm_1_0
    : type_expr_1_0
    | uint_expr_1_0
    ;

func_1_0_name
    : fqn_1_0
    | fqn_1_0 VERSION
    ;

opt_factory_parms_1_0
    : %empty
    | '<' factory_parms_1_0 '>'
    ;

factory_parms_1_0
    : factory_parm_1_0
    | factory_parms_1_0 ',' factory_parm_1_0
    ;

factory_parm_1_0
    : expression_1_0
    | IDENTIFIER_1_0 '=' expression_1_0
    ;

opt_func_1_0_parms
    : %empty
    | func_1_0_parms
    ;

func_1_0_parms
    : expression_1_0
    | func_1_0_parms ',' expression_1_0
    ;

uint_expr_1_0
    : DECIMAL
    | HEX
    | OCTAL
    ;

float_expr_1_0
    : FLOAT
    | EXP_FLOAT
    ;

string_expr_1_0
    : STRING
    | ESCAPED_STRING
    ;

const_vect_expr_1_0
    : '[' opt_const_vect_exprlist_1_0 ']';

opt_const_vect_exprlist_1_0
    : %empty
    | const_vect_exprlist_1_0
    ;

const_vect_exprlist_1_0
    : expression_1_0
    | const_vect_exprlist_1_0 ',' expression_1_0
    ;

bool_expr_1_0
    : KW_true
    | KW_false
    ;

negate_expr_1_0
    : '-' fqn_1_0
    | '-' uint_expr_1_0
    | '-' float_expr_1_0
    ;

type_expr_1_0
    : fqn_1_0                   /* datatype, typeset, schematype */
    | fqn_1_0 '/' fqn_1_0       /* format /  ? */
    | fqn_1_0 dim_1_0
    ;

 /* database */

database_1_0_decl
    : KW_database fqn_1_0 VERSION opt_database_dad_1_0 database_body_1_0
    ;

opt_database_dad_1_0
    : %empty
    | '=' fqn_1_0 opt_version_1_0
    ;

database_body_1_0
    : '{' '}'
    | '{' database_members_1_0 '}'
    ;

database_members_1_0
    : database_member_1_0
    | database_members_1_0 database_member_1_0
    ;

database_member_1_0
    : opt_template_1_0 db_member_1_0
    | opt_template_1_0 table_member_1_0
    | ';'
    ;

opt_template_1_0
    : %empty
    | KW_template
    ;

db_member_1_0
    : KW_database fqn_1_0 opt_version_1_0 IDENTIFIER_1_0 ';'
    ;

table_member_1_0
    : KW_table fqn_1_0 opt_version_1_0 IDENTIFIER_1_0 ';'
    ;

opt_version_1_0
    : %empty
    | VERSION
    ;

/* other stuff
 */
fqn_1_0
    : ident_1_0
    | fqn_1_0 ':' ident_1_0
    /* a hack to handle keywords used as namespace identifiers in existing 1.0 schemas */
    | fqn_1_0 ':' KW_database
    | fqn_1_0 ':' KW_decode
    | fqn_1_0 ':' KW_encode
    | fqn_1_0 ':' KW_read
    | fqn_1_0 ':' KW_table
    | fqn_1_0 ':' KW_type
    | fqn_1_0 ':' KW_view
    | fqn_1_0 ':' KW_write
    ;

ident_1_0
    : IDENTIFIER_1_0                                            /* this is just a C identifier */
    | PHYSICAL_IDENTIFIER_1_0                                   /* starts with a '.' */
    ;

dim_1_0
    : '[' expression_1_0 ']'                                    /* expects unsigned integer constant expression */
    | '[' '*'  ']'
    ;

