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

/* leaving out all of the bison things ... */

%token END_SOURCE

%token VERS_1_0 /* recognized under special flex state */

%token KW_version KW_typedef KW_typeset

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
    : KW_version REAL ';'                                       /* and check for valid version number */
    ;

schema_2_x
    : '$'                                                       /* TBD */
    ;

/* schema-1.0
 */
schema_1_0
    : %empty
    | schema_1_0_decl_seq
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

/* typedef-1.0
 */
typedef_1_0_decl:
    : KW_typedef typename_1_0 typedef_1_0_new_name_list ';'
    ;

typedef_1_0_new_name_list
    : typedef_1_0_new_name
    | typedef_1_0_new_name_list ',' typedef_1_0_new_name
    ;

typedef_1_0_new_name
    : fqn_1_0                                                   /* simple fqn, apparently ( but not necessarily ) scalar */
    | fqn_1_0 dim_1_0                                           /* vector fqn with dimension */
    ;

typename_1_0
    : fqn_1_0                                                   /* must be from 'typedef' or schema template */
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
    : KW_fmtdef format_1_0__new_name ';'
    | KW_fmtdef format_1_0__name format_1_0__new_name ';'       /* creates a derived format */
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
    : KW_const typedecl_1_0 const_1_0_new_name '=' const_expr ';'
    ;

typedecl_1_0                                                    /* don't know why this was called typedecl, but it was... */
    : typename_1_0
    | typename_1_0 dim_1_0
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
    : untyped_func_1_0_decl ';'                                 /* cannot be inline or a validation function */
    | row_length_func_1_0_decl ';'                              /* cannot be inline or a validation function */
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
    | fqn_1_0 '#' maj_min_1_0
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
    | uint_type_expr_1_0 ident_1_0
    ;

func_1_0_return_type
    : vardim_type_expr_1_0
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
    : func_1_0_mandatory_formals opt_func_1_0_optional_formals func_1_0_vararg_formals
    | func_1_0_optional_formals func_1_0_vararg_formals
    ;

func_1_0_mandatory_formals
    : func_1_0_formal_params
    ;

opt_func_1_0_optional_formals
    : %empty
    | func_1_0_optional_formals
    | ',' func_1_0_optional_formals   /* allow because of lack of clarity - does '*' function as ',' or ... ? */
    ;

func_1_0_optional_formals
    : '*' func_1_0_formal_params
    ;

func_1_0_vararg_formals
    : %empty
    | ',' ELLIPSIS
    ;

func_1_0_prologue
    : ';'                                                       /* this is a simple external function declaration    */
    | '=' fqn_1_0 ';'                                           /* rename the function declaration with fqn          */
    | '{' func_1_0_body '}'                                     /* this is a "script" function - cannot be extern!   */
    | '{' func_1_0_body '}' ';'                                 /* allow stray semi-colon at end for C++ programmers */
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

script_1_0_decl
    : KW_schema func_1_0_decl                                   /* MUST have a function body      */
    | KW_schema KW_function func_1_0_decl                       /* "function" keyword is optional */
    ;

validate_1_0_decl
    : KW_validate function_1_0_decl                             /* has exactly 2 parameters and is not inline */
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
    : fqn_1_0 '#' maj_min_1_0
    ;

phys_1_0_prologue
    : '=' phys_1_0_stmt                                         /* shorthand for decode-only rules */
    | '{' phys_1_0_body '}'
    | '{' phys_1_0_body '}' ';'
    ;

phys_1_0_body
    : phys_1_0_body_stmt
    | phys_1_0_body phys_1_0_body_stmt
    ;

phys_1_0_body_stmt
    : ';'
    | KW_decode phys_1_0_stmt
    | KW_encode phys_1_0_stmt
    | KW___row_length rowlen_func_expr_1_0
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
    : fqn_1_0 '#' maj_min_rel_1_0
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
    ;

tbl_1_0_body
    : '{' tbl_1_0_stmt_seq '}'
    | 

tbl_1_0_stmt_seq
    : tbl_1_0_stmt
    | tbl_1_0_stmt_seq tbl_1_0_stmt
    ;

tbl_1_0_stmt
    : production_1_0_stmt
    | column_1_0_decl
    | col_1_0_modifier_seq column_1_0_decl
    | KW_static physmbr_1_0_decl
    | KW_physical physmbr_1_0_decl
    | KW___untyped untyped_tbl_expr_1_0
    ;

col_1_0_modifier_seq
    : col_1_0_modifier
    | col_1_0_modifier_seq col_1_0_modifier
    ;

col_1_0_modifier
    : KW_default
    | KW_virtual                                                /* this is a noop - not sure it was ever used */
    | KW_extern
    | KW_readonly
    ;

column_1_0_decl
    : KW_column col_1_0_decl
    | KW_column col_1_0_limit col_1_0_decl
    ;

col_1_0_limit
    : KW_limit '=' const_expr_1_0 ';'
    | KW_default KW_limit '=' const_expr_1_0 ';'
    ;

col_1_0_decl
    : HERE
    ;

/* other stuff
 */
fqn_1_0
    : ident_1_0
    | fqn_1_0 ':' ident_1_0
    ;

ident_1_0
    : IDENTIFIER_1_0                                            /* this is just a C identifier */
    ;

dim_1_0
    : '[' const_expr_1_0 ']'                                    /* expects unsigned integer constant expression */
    ;
