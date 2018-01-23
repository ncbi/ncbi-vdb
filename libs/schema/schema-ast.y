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

    #include "ASTBuilder.hpp"
    using namespace ncbi::SchemaParser;

    #include "schema-ast-tokens.h"

    #define AST_lex NextToken
    static int NextToken ( YYSTYPE * p_token, ParseTreeScanner & p_sb )
    {
        return p_sb . NextToken ( p_token -> tok );
    }

    void AST_error ( void * p_parser, ASTBuilder & p_builder, ParseTreeScanner & p_sb, const char * p_msg )
    {
        p_builder . ReportInternalError ( p_msg, p_sb . GetSourceFileName () );
    }

%}

%name-prefix "AST_"
%parse-param { AST*& p_ast }
%parse-param { ASTBuilder& p_builder }
%param { ParseTreeScanner& p_sb }

 /*%define api.value.type { const Token* }*/

%union {
  const Token*  tok;
  AST*          node;
  AST_FQN*      fqn;
  AST_Expr*     expr;
}

%define parse.error verbose

%define api.pure full

 /* !!! Keep token declarations in synch with schema-grammar.y */

%token END_SOURCE 0 "end of source"

%token UNRECOGNIZED

%token ELLIPSIS
%token INCREMENT

%token DECIMAL
%token OCTAL
%token HEX
%token FLOAT_
%token EXP_FLOAT
%token STRING
%token ESCAPED_STRING

%token IDENTIFIER_1_0
%token PHYSICAL_IDENTIFIER_1_0
%token VERSION

/* unterminated strings are dealt with by flex */
%token UNTERM_STRING
%token UNTERM_ESCAPED_STRING

/* version numbers recognized under special flex state */
%token VERS_1_0
%token VERS_2_0

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

    /* Parse tree nodes */
%token PT_ASTLIST
%token PT_PARSE
%token PT_SOURCE
%token PT_VERSION_1_0
%token PT_VERSION_2
%token PT_SCHEMA_1_0
%token PT_SCHEMA_2_0
%token PT_INCLUDE
%token PT_TYPEDEF
%token PT_FQN
%token PT_IDENT
%token PT_PHYSIDENT
%token PT_UINT
%token PT_TYPESET
%token PT_TYPESETDEF
%token PT_FORMAT
%token PT_CONST
%token PT_ALIAS
%token PT_EXTERN
%token PT_FUNCTION
%token PT_UNTYPED
%token PT_ROWLENGTH
%token PT_FUNCDECL
%token PT_EMPTY
%token PT_SCHEMASIG
%token PT_SCHEMAFORMAL
%token PT_RETURNTYPE
%token PT_FACTSIG
%token PT_FUNCSIG
%token PT_FUNCPARAMS
%token PT_FORMALPARAM
%token PT_ELLIPSIS
%token PT_FUNCPROLOGUE
%token PT_RETURN
%token PT_PRODSTMT
%token PT_PRODTRIGGER
%token PT_SCHEMA
%token PT_VALIDATE
%token PT_PHYSICAL
%token PT_PHYSPROLOGUE
%token PT_PHYSSTMT
%token PT_PHYSBODYSTMT
%token PT_TABLE
%token PT_TABLEPARENTS
%token PT_TABLEBODY
%token PT_FUNCEXPR
%token PT_FACTPARMS
%token PT_COLUMN
%token PT_COLUMNEXPR
%token PT_COLDECL
%token PT_TYPEDCOL
%token PT_COLSTMT
%token PT_DFLTVIEW
%token PT_PHYSMBR
%token PT_PHYSCOL
%token PT_PHYSCOLDEF
%token PT_COLSCHEMAPARMS
%token PT_COLSCHEMAPARAM
%token PT_COLUNTYPED
%token PT_DATABASE
%token PT_TYPEEXPR
%token PT_DBBODY
%token PT_DBDAD
%token PT_DBMEMBER
%token PT_TBLMEMBER
%token PT_NOHEADER
%token PT_CASTEXPR
%token PT_CONSTVECT
%token PT_NEGATE
%token PT_UNARYPLUS
%token PT_VERSNAME
%token PT_ARRAY
%token PT_PHYSENCREF
%token PT_TYPEDCOLEXPR
%token PT_VIEW
%token PT_VIEWPARAM
%token PT_VIEWPARENTS
%token PT_VIEWPARENT
%token PT_MEMBEREXPR
%token PT_JOINEXPR

 /* !!! Keep token declarations above in synch with schema-grammar.y */

%start parse

%type <node> source schema_1 schema_decls schema_decl schema_2 typedef new_type_names
%type <node> typeset typeset_spec typespec dim fmtdef const alias function func_decl
%type <node> schema_sig_opt return_type prologue formals_list
%type <node> schema_formals schema_formal type_expr formals formal
%type <node> script_stmts script_stmt extern_function script script_decl validate
%type <node> script_prologue physical phys_prologue phys_body phys_body_stmt
%type <node> phys_return_type table parents_opt tbl_body tbl_stmts tbl_stmt
%type <node> tbl_parents production column_decl col_modifiers_opt col_modifiers
%type <node> col_modifier col_decl col_ident col_body col_stmt typed_col
%type <node> factory_parms factory_parms_opt schema_parm schema_parms arrayspec
%type <node> phys_enc_ref col_body_opt database dbdad_opt dbbody db_members
%type <node> db_member template_opt include func_parms_opt expr_list schema_parts_opt
%type <node> physmbr_decl col_schema_parms_opt col_schema_parms
%type <node> col_schema_value col_schema_parm phys_coldef factory_parms_list
%type <node> vararg param_sig param_signature fact_sig
%type <node> view view_parms view_parm view_body_opt view_body view_member
%type <node> view_parents_opt view_parents view_parent view_parent_parms

%type <fqn> fqn qualnames fqn_opt_vers ident fqn_vers

%type <expr> expr cond_expr cond_chain uint_expr func_expr float_expr string_expr const_vect_expr
%type <expr> bool_expr negate_expr cast_expr member_expr join_expr

%type <tok> END_SOURCE version_1 PT_VERSION_1_0 PT_VERSION_2 PT_SCHEMA_1_0 FLOAT_ version_2
%type <tok> PT_TYPEDEF PT_IDENT IDENTIFIER_1_0 DECIMAL PT_ASTLIST PT_ARRAY PT_TYPESET
%type <tok> PT_FORMAT PT_CONST PT_UINT PT_ALIAS PT_EMPTY PT_ELLIPSIS KW_return
%type <tok> VERSION PT_UNTYPED PT_ROWLENGTH PT_FUNCDECL PT_FUNCPARAMS PT_FORMALPARAM
%type <tok> PT_VALIDATE PT_PHYSICAL PT_PHYSSTMT PT_TABLE PT_COLUMN PT_COLUMNEXPR
%type <tok> KW_default KW_extern KW_readonly PHYSICAL_IDENTIFIER_1_0 HEX OCTAL PT_COLSTMT
%type <tok> KW_read KW_validate KW_limit PT_SCHEMAFORMAL PT_PRODSTMT PT_PRODTRIGGER
%type <tok> PT_NOHEADER KW_decode KW_encode KW___row_length PT_COLDECL PT_TYPEDCOL PT_TYPEEXPR
%type <tok> PT_PHYSENCREF KW_column PT_TYPEDCOLEXPR PT_DATABASE PT_DBBODY
%type <tok> KW_template KW_database PT_DBMEMBER PT_TBLMEMBER KW_include STRING
%type <tok> PT_FUNCEXPR PT_COLSCHEMAPARMS KW_static PT_PHYSMBR PT_PHYSCOLDEF PT_COLSCHEMAPARAM
%type <tok> KW_physical PT_COLUNTYPED EXP_FLOAT ESCAPED_STRING PT_CONSTVECT KW_true KW_false
%type <tok> PT_NEGATE PT_CASTEXPR '@' KW_control PT_SCHEMA_2_0
%type <tok> PT_VIEW PT_VIEWPARAM PT_VIEWPARENTS PT_MEMBEREXPR PT_VIEWPARENT PT_JOINEXPR

%%

parse
    : PT_PARSE '(' END_SOURCE ')'                   { p_ast = new AST ( $3 ); }
    | PT_PARSE '(' source END_SOURCE ')'            { p_ast = $3; }
    ;

source
    : PT_SOURCE '(' schema_1 ')'                    { $$ = $3; }
    | PT_SOURCE '(' version_1 schema_1 ')'          { $$ = $4; $$ -> AddNode ( $3 ); }
    | PT_SOURCE '(' version_2 schema_2 ')'          { $$ = $4; $$ -> AddNode ( $3 ); }
    ;

version_1
    : PT_VERSION_1_0 '(' KW_version VERS_1_0 ';' ')'    { $$ = $1; }
    ;

version_2
    : PT_VERSION_2 '(' KW_version VERS_2_0 ';' ')'     { $$ = $1; }
    ;

schema_1
    : PT_SCHEMA_1_0 '(' schema_decls ')'            { $$ = new AST ( $1, $3 ); }
    ;

 schema_2
    : PT_SCHEMA_2_0 '(' schema_decls ')'            { $$ = new AST ( $1, $3 ); }
    ;


schema_decls
    : schema_decl                                   { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | schema_decls schema_decl                      { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

/* declarations */

schema_decl
    : typedef           { $$ = $1; }
    | typeset           { $$ = $1; }
    | fmtdef            { $$ = $1; }
    | const             { $$ = $1; }
    | alias             { $$ = $1; }
    | function          { $$ = $1; }
    | extern_function   { $$ = $1; }
    | script            { $$ = $1; }
    | validate          { $$ = $1; }
    | physical          { $$ = $1; }
    | table             { $$ = $1; }
    | database          { $$ = $1; }
    | include           { $$ = $1; }
    | view              { $$ = $1; }
    | ';'               { $$ = new AST (); }
    ;

typedef
    : PT_TYPEDEF '(' KW_typedef fqn PT_ASTLIST '(' new_type_names ')' ';' ')'
                                            { $$ = p_builder . TypeDef ( $1, $4, $7 ); }
    ;

new_type_names
    : typespec                         { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | new_type_names ',' typespec      { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

typespec
    : fqn       { $$ = $1; }
    | arrayspec { $$ = $1; }
    ;

arrayspec
    : PT_ARRAY '(' fqn '[' dim ']' ')'  { $$ = new AST ( $1, $3, $5 ); }
    ;

dim
    : expr   { $$ = $1; }
    | '*'    { $$ = new AST ( PT_EMPTY ); }
    ;

typeset
    :  PT_TYPESET '(' KW_typeset fqn PT_TYPESETDEF '(' '{' PT_ASTLIST '(' typeset_spec ')' '}' ')' ';' ')'
                { $$ = p_builder . TypeSet ( $1, $4, $10 ); }
    ;

typeset_spec
    : typespec                      { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | typeset_spec ',' typespec     { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

fmtdef
    : PT_FORMAT '(' KW_fmtdef fqn ';' ')'       { $$ = p_builder . FmtDef ( $1, $4, 0 ); }
    | PT_FORMAT '(' KW_fmtdef fqn fqn ';' ')'   { $$ = p_builder . FmtDef ( $1, $5, $4 ); } /* note the flipped order */
    ;

const
    : PT_CONST '(' KW_const typespec fqn '=' expr ';' ')'        { $$ = p_builder . ConstDef ( $1, $4, $5, $7 ); }
    ;

alias
    : PT_ALIAS '(' KW_alias fqn fqn ';' ')'  { $$ = p_builder . AliasDef ( $1, $4, $5 ); }
    ;

function
    : PT_FUNCTION '(' KW_function func_decl ')' { $$ = $4; }
    ;

func_decl
    : PT_UNTYPED '(' KW___untyped fqn '(' ')' ')'      { $$ = p_builder . UntypedFunctionDecl ( $1, $4 ); }
    | PT_ROWLENGTH '(' KW___row_length fqn '(' ')' ')' { $$ = p_builder . RowlenFunctionDecl ( $1, $4 ); }
    | PT_FUNCDECL '(' schema_sig_opt return_type fqn_opt_vers fact_sig param_sig prologue ')'
                { $$ = p_builder . FunctionDecl ( $1, false, $3, $4, $5, $6, $7, $8 ); }
    ;

schema_sig_opt
    : PT_EMPTY                                                          { $$ = new AST ( $1 ); }
    | PT_SCHEMASIG '(' '<' PT_ASTLIST '(' schema_formals ')' '>' ')'    { $$ = $6; }
    ;

schema_formals
    : schema_formal                     { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | schema_formals ',' schema_formal  { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

schema_formal
    : PT_SCHEMAFORMAL '(' KW_type ident ')'     { $$ = new AST ( $1, $4 ); }
    | PT_SCHEMAFORMAL '(' type_expr ident ')'   { $$ = new AST ( $1, $3, $4 ); }
    ;

return_type
    : PT_RETURNTYPE '(' KW_void ')'     { $$ = new AST (); }
    | PT_RETURNTYPE '(' type_expr ')'   { $$ = $3; }
    ;

fact_sig
    : PT_EMPTY                                   { $$ = new AST ( $1 ); }
    | PT_FACTSIG '(' '<' param_signature '>' ')' { $$ = $4; }
    ;

param_sig
    :  PT_FUNCSIG '(' '(' param_signature ')' ')' { $$ = $4; }
    ;

param_signature
    : PT_EMPTY                                                          { $$ = new AST ( $1 ); }
    | PT_FUNCPARAMS '(' formals_list vararg ')'                         { $$ = new AST ( $1, $3, new AST (), $4 ); }
    | PT_FUNCPARAMS '(' '*' formals_list vararg ')'                     { $$ = new AST ( $1, new AST (), $4, $5 ); }
    | PT_FUNCPARAMS '(' formals_list '*' formals_list vararg ')'        { $$ = new AST ( $1, $3, $5, $6 ); }
    | PT_FUNCPARAMS '(' formals_list ',' '*' formals_list vararg ')'    { $$ = new AST ( $1, $3, $6, $7 ); }
    ;

formals_list
    : PT_ASTLIST '(' formals ')'    { $$ = $3; }
    ;

formals
    : formal                { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | formals ',' formal    { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

formal
    : PT_FORMALPARAM '(' typespec IDENTIFIER_1_0 ')'            { $$ = new AST ( $1, $3, new AST ( $4 ), new AST () ); }
    | PT_FORMALPARAM '(' KW_control typespec IDENTIFIER_1_0 ')' { $$ = new AST ( $1, $4, new AST ( $5 ), new AST ( $3 ) ); }
    ;

vararg
    : PT_EMPTY                          { $$ = new AST ( $1 ); }
    | PT_ELLIPSIS '(' ',' ELLIPSIS ')'  { $$ = new AST ( $1 ); }
    ;

prologue
    : PT_FUNCPROLOGUE '(' ';' ')'           { $$ = new AST ( PT_EMPTY ); }
    | PT_FUNCPROLOGUE '(' '=' fqn ';' ')'   { $$ = $4; }
    | script_prologue
    ;

script_prologue
    : PT_FUNCPROLOGUE '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')'   { $$ = $6; }
    ;

script_stmts
    : script_stmt               { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | script_stmts script_stmt  { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

script_stmt
    : PT_RETURN '(' KW_return cond_expr ';' ')'     { $$ = new AST ( $3, $4 ); }
    | production                                    { $$ = $1; }
    ;

production
    : PT_PRODSTMT '(' type_expr ident '=' cond_expr ';' ')'        { $$ = new AST ( $1, $3, $4, $6 ); }
    | PT_PRODTRIGGER '(' KW_trigger ident '=' cond_expr ';' ')'    { $$ = new AST ( $1, $4, $6 ); }
    ;

extern_function
    : PT_EXTERN '(' KW_extern function ')'    { $$ = $4; }

script
    : PT_SCHEMA '(' KW_schema script_decl ')'             { $$ = $4; }
    | PT_SCHEMA '(' KW_schema KW_function script_decl ')' { $$ = $5; }
    ;

script_decl
    : PT_FUNCDECL '(' schema_sig_opt return_type fqn_opt_vers fact_sig param_sig script_prologue ')'
                { $$ = p_builder . FunctionDecl ( $1, true, $3, $4, $5, $6, $7, $8 ); }
    ;

validate
    : PT_VALIDATE '(' KW_validate
        PT_FUNCTION '(' KW_function
            PT_FUNCDECL '(' schema_sig_opt return_type fqn_opt_vers fact_sig param_sig prologue ')'
        ')'
      ')'
        { $$ = p_builder . FunctionDecl ( $1, false, $9, $10, $11, $12, $13, $14 ); }
    ;

physical
    : PT_PHYSICAL '(' KW_physical schema_sig_opt phys_return_type fqn_vers fact_sig phys_prologue ')'
            { $$ = p_builder . PhysicalDecl ( $1, $4, $5, $6, $7, $8 ); }
    ;

phys_return_type
    : return_type                                       { $$ = $1; }
    | PT_NOHEADER '(' KW___no_header return_type ')'    { $$ = new AST ( $1, $4 ); }

phys_prologue
    : PT_PHYSPROLOGUE '(' '=' PT_PHYSSTMT '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')' ')'
                { $$ = new AST ( $4, $9 ); }
    | PT_PHYSPROLOGUE '(' '{' PT_ASTLIST '(' phys_body ')' '}' ')'
                { $$ = $6; }
    ;

phys_body
    : phys_body_stmt            { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | phys_body phys_body_stmt  { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

phys_body_stmt
    : PT_PHYSBODYSTMT '(' ';' ')'
        { $$ = new AST ( PT_EMPTY ); }
    | PT_PHYSBODYSTMT '(' KW_decode PT_PHYSSTMT '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')' ')'
        { $$ = new AST ( $3, $9 ) ; }
    | PT_PHYSBODYSTMT '(' KW_encode PT_PHYSSTMT '(' '{' PT_ASTLIST '(' script_stmts ')' '}' ')' ')'
        { $$ = new AST ( $3, $9 ); }
    | PT_PHYSBODYSTMT '(' KW___row_length '=' fqn '(' ')' ')'
        { $$ = new AST ( $3, $5 ); }
    ;

/* tables */

table
    : PT_TABLE '(' KW_table fqn_vers parents_opt PT_TABLEBODY '(' '{' tbl_body '}' ')' ')'
                { $$ = p_builder . TableDef ( $1, $4, $5, $9 ); }
    ;

parents_opt
    : PT_EMPTY                                                      { $$ = new AST ( $1 ); }
    | PT_TABLEPARENTS '(' '=' PT_ASTLIST '(' tbl_parents ')' ')'    { $$ = $6; }
    ;

tbl_parents
    : fqn_opt_vers                  { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | tbl_parents ',' fqn_opt_vers  { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

tbl_body
    : %empty                        { $$ = new AST ( PT_EMPTY ); }
    | PT_ASTLIST '(' tbl_stmts ')'  { $$ = $3; }
    ;

tbl_stmts
    : tbl_stmt              { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | tbl_stmts tbl_stmt    { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

tbl_stmt
    : production                                                        { $$ = $1; }
    | column_decl                                                       { $$ = $1; }
    | PT_COLUMNEXPR '(' KW_column KW_limit '=' expr ';' ')'             { $$ = new AST ( $1, $6 ); }
    | PT_COLUMNEXPR '(' KW_column KW_default KW_limit '=' expr ';' ')'  { $$ = new AST ( $1, $7 ); }
    | PT_PHYSCOL '(' KW_static physmbr_decl ')'                         { $$ = new AST ( $3, $4 ); }
    | PT_PHYSCOL '(' KW_physical physmbr_decl ')'                       { $$ = new AST ( $3, $4 ); }
    | PT_PHYSCOL '(' KW_static KW_physical physmbr_decl ')'             { $$ = new AST ( $3, $5 ); }
    | PT_COLUNTYPED '(' KW___untyped '=' fqn '(' ')' ';' ')'            { $$ = new AST ( $1, $5 ); }
    | ';'                                                               { $$ = new AST (); }
;

column_decl
    : PT_COLUMN '(' col_modifiers_opt col_decl ')' { $$ = new AST ( $1, $3, $4 ); }
    ;

col_modifiers_opt
    : KW_column                                     { $$ = new AST ( $1 ); }
    | PT_ASTLIST '(' col_modifiers KW_column ')'    { $$ = $3; }
    ;

col_modifiers
    : col_modifier                  { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | col_modifiers col_modifier    { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

col_modifier
    : KW_default    { $$ = new AST ( $1 ); }
    | KW_extern     { $$ = new AST ( $1 ); }
    | KW_readonly   { $$ = new AST ( $1 ); }
    ;

col_decl
    : PT_COLDECL '(' typespec typed_col ')'     { $$ = new AST ( $1, $3, $4 ); }
    | PT_COLDECL '(' phys_enc_ref typed_col ')' { $$ = new AST ( $1, $3, $4 ); }
    ;

phys_enc_ref
    : PT_PHYSENCREF '(' '<' PT_ASTLIST '(' schema_parms ')' '>' fqn_opt_vers factory_parms_opt ')'
                                                        { $$ = new AST ( $1, $6, $9, $10 ); }
    | PT_PHYSENCREF '(' fqn_vers factory_parms_opt ')'  { $$ = new AST ( $1, $3, $4 ); }
    | PT_PHYSENCREF '(' fqn factory_parms_list ')'      { $$ = new AST ( $1, $3, $4 ); }
    ;

typed_col
    : PT_TYPEDCOL '(' col_ident '{' col_body_opt '}' ')'    {  $$ = new AST ( $1, $3, $5 ); }
    | PT_TYPEDCOLEXPR '(' col_ident '=' cond_expr ';' ')'   {  $$ = new AST ( $1, $3, $5 ); }
    | PT_TYPEDCOL '(' col_ident ';' ')'                     {  $$ = new AST ( $1, $3 ); }
    ;

col_ident
    : ident                     { $$ = $1; }
    | PHYSICAL_IDENTIFIER_1_0   { $$ = new AST ( $1 ); }
    ;

col_body_opt
    : PT_EMPTY                      { $$ = new AST (); }
    | PT_ASTLIST '(' col_body ')'   { $$ = $3; }
    ;

col_body
    : col_stmt          { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | col_body col_stmt { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

col_stmt
    : ';'                                               { $$ = new AST (); }
    | PT_COLSTMT '(' KW_read '=' cond_expr ';' ')'      { $$ = new AST ( $1, new AST ( $3 ), $5 ); }
    | PT_COLSTMT '(' KW_validate '=' cond_expr ';' ')'  { $$ = new AST ( $1, new AST ( $3 ), $5 ); }
    | PT_COLSTMT '(' KW_limit '=' uint_expr ';' ')'     { $$ = new AST ( $1, new AST ( $3 ), $5 ); }
    ;

physmbr_decl
    : PT_PHYSMBR '(' phys_coldef PHYSICAL_IDENTIFIER_1_0 ';' ')'
                                                { $$ = new AST ( $1, $3, new AST ( $4 ) ); }
    | PT_PHYSMBR '(' KW_column phys_coldef PHYSICAL_IDENTIFIER_1_0 ';' ')'
                                                { $$ = new AST ( $1, $4, new AST ( $5 ) ); }
    | PT_PHYSMBR '(' phys_coldef PHYSICAL_IDENTIFIER_1_0 '=' cond_expr ';' ')'
                                                { $$ = new AST ( $1, $3, new AST ( $4 ), $6 ); }
    | PT_PHYSMBR '(' KW_column phys_coldef PHYSICAL_IDENTIFIER_1_0 '=' cond_expr ';' ')'
                                                { $$ = new AST ( $1, $4, new AST ( $5 ), $7 ); }
    ;

phys_coldef
    : PT_PHYSCOLDEF '(' col_schema_parms_opt fqn_opt_vers factory_parms_opt ')'    { $$ = new AST ( $1, $3, $4, $5 ); }
    ;

col_schema_parms_opt
    : PT_EMPTY                                                              { $$ = new AST (); }
    | PT_COLSCHEMAPARMS '(' '<' PT_ASTLIST '(' col_schema_parms ')' '>' ')' { $$ = $6; }
    ;

col_schema_parms
    : col_schema_parm                   { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | col_schema_parms col_schema_parm  { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

col_schema_parm
    : PT_COLSCHEMAPARAM '(' fqn '=' col_schema_value ')'    { $$ = new AST ( $1, $3, $5 ); }
    | col_schema_value                                      { $$ = $1; }
    ;

col_schema_value
    : fqn       { $$ = $1; }
    | uint_expr { $$ = $1; }
    ;

/* database */
database
    : PT_DATABASE '(' KW_database fqn_vers dbdad_opt dbbody ')' { $$ = p_builder . DatabaseDef ( $1, $4, $5, $6 ); }
    ;

dbdad_opt
    : PT_EMPTY                             { $$ = new AST ( $1 ); }
    | PT_DBDAD '(' '=' fqn_opt_vers ')'    { $$ = $4; }

dbbody
    : PT_DBBODY '(' '{' PT_ASTLIST '(' db_members ')' '}' ')'   { $$ = $6; }
    | PT_DBBODY '(' '{' '}' ')'                                 { $$ = new AST ( $1 ); }
    ;

db_members
    : db_member               { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | ';'                       { $$ = new AST (); }
    | db_members db_member    { $$ = $1; $$ -> AddNode ( $2 ); }
    | db_members ';'            { $$ = $1; }
    ;

db_member
    : PT_DBMEMBER '(' template_opt KW_database fqn_opt_vers ident ';' ')'
        { $$ = new AST ( $1, $3, $5, $6 ); }
    | PT_TBLMEMBER '(' template_opt KW_table fqn_opt_vers ident ';' ')'
        { $$ = new AST ( $1, $3, $5, $6 ); }
    ;

template_opt
    : PT_EMPTY      { $$ = new AST ( $1 ); }
    | KW_template   { $$ = new AST ( $1 ); }
    ;

/* include */

include
    : PT_INCLUDE '(' KW_include STRING ')' { $$ = p_builder . Include ( $3, $4 ); }
    ;

/* expressions */

cond_expr
    : PT_ASTLIST '(' cond_chain ')' { $$ = $3; }
    ;

cond_chain
    : expr                   { $$ = new AST_Expr ( $1 ); }
    | cond_chain '|' expr    { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

expr
    : fqn                           { $$ = new AST_Expr ( $1 ); }
    | PHYSICAL_IDENTIFIER_1_0       { $$ = new AST_Expr ( $1 ); }
    | '@'                           { $$ = new AST_Expr ( $1 ); }
    | func_expr                     { $$ = $1; }
    | uint_expr                     { $$ = $1; }
    | float_expr                    { $$ = $1; }
    | string_expr                   { $$ = $1; }
    | const_vect_expr               { $$ = $1; }
    | bool_expr                     { $$ = $1; }
    | negate_expr                   { $$ = $1; }
    | PT_UNARYPLUS '(' '+' expr ')' { $$ = $4; }
    | cast_expr                     { $$ = $1; }
    | member_expr                   { $$ = $1; }
    | join_expr                     { $$ = $1; }
    ;

func_expr
    : PT_FUNCEXPR '(' schema_parts_opt fqn_opt_vers factory_parms_opt '(' func_parms_opt ')' ')'
        { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $3 ); $$ -> AddNode ( $4 ); $$ -> AddNode ( $5 ); $$ -> AddNode ( $7 ); }
    ;

schema_parts_opt
    : %empty                                    { $$ = new AST ( PT_EMPTY ); }
    | '<' PT_ASTLIST '(' schema_parms ')' '>'   { $$ = $4; }

schema_parms
    : schema_parm                   { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | schema_parms ',' schema_parm  { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

schema_parm
    : fqn           { $$ = $1; }
    | arrayspec     { $$ = $1; }
    | uint_expr     { $$ = $1; }
    ;

factory_parms_opt
    : PT_EMPTY              { $$ = new AST ( $1 ); }
    | factory_parms_list    { $$ = $1; }
    ;

factory_parms_list
    : PT_FACTPARMS '(' '<' PT_ASTLIST '(' factory_parms ')' '>' ')'     { $$ = $6; }
    ;

factory_parms
    : expr                      { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | factory_parms ',' expr    { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

func_parms_opt
    : PT_EMPTY                      { $$ = new AST ( $1 ); }
    | PT_ASTLIST '(' expr_list ')'  { $$ = $3; }
    ;

expr_list
    : expr                  { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | expr_list ',' expr    { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

uint_expr
    : PT_UINT '(' DECIMAL ')'   { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $3 ); }
    | PT_UINT '(' HEX ')'       { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $3 ); }
    | PT_UINT '(' OCTAL ')'     { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $3 ); }
    ;

float_expr
    : FLOAT_     { $$ = new AST_Expr ( $1 ); }
    | EXP_FLOAT { $$ = new AST_Expr ( $1 ); }
    ;

string_expr
    : STRING            { $$ = new AST_Expr ( $1 ); }
    | ESCAPED_STRING    { $$ = new AST_Expr ( $1 ); }
    ;

const_vect_expr
    : PT_CONSTVECT '(' '[' PT_ASTLIST '(' expr_list ')' ']' ')'     { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $6 ); }
    ;

bool_expr
    : KW_true   { $$ = new AST_Expr ( $1 ); }
    | KW_false  { $$ = new AST_Expr ( $1 ); }
    ;

negate_expr
    : PT_NEGATE '(' '-' expr ')' { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $4 ); }

cast_expr
    : PT_CASTEXPR '(' '(' type_expr ')' expr  ')' { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $4 ); $$ -> AddNode ( $6 ); }
    ;

type_expr
    : typespec                          { $$ = $1; }
    | PT_TYPEEXPR '(' fqn '/' fqn ')'   { $$ = new AST ( $1, $3, $5 ); }
    ;

member_expr
    : PT_MEMBEREXPR '(' ident '.' ident ')'                 { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $3 ); $$ -> AddNode ( $5 ); }
    | PT_MEMBEREXPR '(' ident PHYSICAL_IDENTIFIER_1_0 ')'
        {   /* remove leading '.'*/
            $$ = new AST_Expr ( $1 );
            $$ -> AddNode ( $3 );
            AST * ident = new AST ( PT_IDENT );
            Token t ( IDENTIFIER_1_0, $4 -> GetValue() + 1, $$ -> GetLocation() );
            ident -> AddNode ( & t );
            $$ -> AddNode ( ident );
        }
    ;

join_expr
    : PT_JOINEXPR '(' ident '[' cond_expr ']' '.' ident ')'
        { $$ = new AST_Expr ( $1 ); $$ -> AddNode ( $3 ); $$ -> AddNode ( $5 ); $$ -> AddNode ( $8 ); }
    ;

/* commonly used productions */

fqn
    : PT_FQN '(' qualnames ')'  { $$ = $3; }
    ;

qualnames
    : PT_IDENT '(' IDENTIFIER_1_0 ')'                   { $$ = new AST_FQN ( $1 ); $$ -> AddNode ( $3 ); }
    | qualnames ':' PT_IDENT  '(' IDENTIFIER_1_0 ')'    { $$ = $1; $$ -> AddNode ( $5 ); }
    ;

ident
    : PT_IDENT '(' IDENTIFIER_1_0 ')'   { $$ = new AST_FQN ( $1 ); $$ -> AddNode ( $3 ); }
    ;

fqn_opt_vers
    : fqn       { $$ = $1; }
    | fqn_vers  { $$ = $1; }
    ;

fqn_vers
    : PT_VERSNAME '(' fqn VERSION ')'   { $$ = $3; $$ -> SetVersion ( $4 -> GetValue () ); }
    ;

/* view */
view
    : PT_VIEW '(' KW_view fqn_vers '<' PT_ASTLIST '(' view_parms ')' '>'
                        view_parents_opt '{' view_body_opt '}' ')'
        { $$ = p_builder . ViewDef ( $1, $4, $8, $11, $13 ); }
    ;

view_parms
    : view_parm                 { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | view_parms ',' view_parm  { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

view_parm
    : PT_VIEWPARAM '(' fqn_opt_vers ident ')'   { $$ = new AST ( $1, $3, $4 ); }
    ;

view_body_opt
    : PT_EMPTY                      { $$ = new AST ( $1 ); }
    | PT_ASTLIST '(' view_body ')'  { $$ = $3; }
    ;

view_body
    : view_member           { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | view_body view_member { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

view_member
    : PT_PRODSTMT '(' typespec ident '=' cond_expr ';' ')'          { $$ = new AST ( $1, $3, $4, $6 ); }
    | PT_COLUMN '(' KW_column typespec ident '=' cond_expr ';' ')'  { $$ = new AST ( $1, $4, $5, $7 ); }
    | ';'                                                           { $$ = new AST ( PT_EMPTY ); }
    ;

view_parents_opt
    : PT_EMPTY                                                      { $$ = new AST ( $1 ); }
    | PT_VIEWPARENTS '(' '=' PT_ASTLIST '(' view_parents ')' ')'    { $$ = new AST ( $1, $6 ); }
    ;

view_parents
    : view_parent                   { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | view_parents ',' view_parent  { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

view_parent
    : PT_VIEWPARENT '(' fqn_opt_vers '<' PT_ASTLIST '(' view_parent_parms ')' '>' ')'
        { $$ = new AST ( $1, $3, $7 ); }
    ;

view_parent_parms
    : ident                         { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | view_parent_parms ',' ident   { $$ = $1; $$ -> AddNode ( $3 ); }
    ;
