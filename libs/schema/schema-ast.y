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

    #include "AST.hpp"
    using namespace ncbi::SchemaParser;

    #include "schema-ast-tokens.h"

    #define AST_lex NextToken
    static int NextToken ( YYSTYPE* p_token, ParseTreeScanner& p_sb )
    {
        return p_sb . NextToken ( p_token -> tok );
    }

    void AST_error ( void* parser, ASTBuilder& p_builder, ParseTreeScanner&, const char* msg )
    {
        /*TODO: retrieve name/location data from the scanner, add to the report */
        p_builder . ReportError ( "%s", msg );
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

    /* Parse tree nodes */
%token PT_ASTLIST
%token PT_PARSE
%token PT_SOURCE
%token PT_VERSION_1_0
%token PT_VERSION_2
%token PT_SCHEMA_1_0
%token PT_INCLUDE
%token PT_TYPEDEF
%token PT_FQN
%token PT_IDENT
%token PT_DIM
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
%token PT_ASSIGN
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
%token PT_COLDECL
%token PT_TYPEDCOL
%token PT_COLMODIFIER
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
%token PT_DATABASEMEMBER
%token PT_DBMEMBER
%token PT_TABLEMEMBER
%token PT_TBLMEMBER
%token PT_NOHEADER
%token PT_CASTEXPR
%token PT_CONSTVECT
%token PT_NEGATE
%token PT_UNARYPLUS
%token PT_FACTPARMNAMED
%token PT_VERSNAME
%token PT_ARRAY

 /* !!! Keep token declarations above in synch with schema-grammar.y */

%start parse

%type <node> source schema_1 schema_decls schema_decl schema_2 typedef new_type_names new_type_name expr
%type <fqn> fqn qualnames
%type <tok> END_SOURCE version_1 PT_VERSION_1_0 PT_VERSION_2 PT_SCHEMA_1_0 FLOAT version_2 PT_TYPEDEF PT_IDENT IDENTIFIER_1_0 DECIMAL PT_ASTLIST PT_ARRAY

%%

parse
    : PT_PARSE '(' END_SOURCE ')'                   { p_ast = new AST ( $3 ); }
    | PT_PARSE '(' source END_SOURCE ')'            { p_ast = $3; }
    ;

source
    : PT_SOURCE '(' schema_1 ')'                    { $$ = $3; }
    | PT_SOURCE '(' version_1 schema_1 ')'          { $$ = $4; }
    | PT_SOURCE '(' version_2 schema_2 ')'          { $$ = $4; dynamic_cast < AST_Schema* > ( $$ ) -> SetVersion ( $3 -> GetValue () ); }
    ;

version_1
    : PT_VERSION_1_0 '(' KW_version VERS_1_0 ';' ')'    { $$ = $1; }
    ;

version_2
    : PT_VERSION_2 '(' KW_version FLOAT ';' ')'     { $$ = $4; }
    ;

schema_1
    : PT_SCHEMA_1_0 '(' schema_decls ')'            { $$ = new AST_Schema ( $1, $3 ); }
    ;

 schema_2:
    %empty  { $$ = new AST_Schema (); }             /*TBD*/
    ;


schema_decls
    : schema_decl                                   { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | schema_decls schema_decl                      { $$ = $1; $$ -> AddNode ( $2 ); }
    ;

schema_decl
    : typedef   { $$ = $1; }
    /* | TBD */
    | ';'       { $$ = new AST (); }
    ;

typedef
    : PT_TYPEDEF '(' KW_typedef fqn PT_ASTLIST '(' new_type_names ')' ';' ')'            { $$ = new AST_TypeDef ( p_builder, $1, $4, $7 ); /*TODO: resolve $4; declare all in $7 */}
    ;

new_type_names
    : new_type_name                         { $$ = new AST (); $$ -> AddNode ( $1 ); }
    | new_type_names ',' new_type_name      { $$ = $1; $$ -> AddNode ( $3 ); }
    ;

new_type_name
    : fqn                                               { $$ = $1; }
    | PT_ARRAY '(' fqn PT_DIM '(' '[' expr ']' ')' ')'  { $$ = new AST_ArrayDef ( $1, $3, $7 ); }
    /*| PT_ASTLIST '(' fqn PT_DIM '[' '*' ']' ')'       { $$ = new AST_ArrayDef ( $1, $3, 0 ); }*/
    ;

fqn
    : PT_ASTLIST '(' qualnames ')'  { $$ = $3; }
    ;

qualnames
    : PT_IDENT '(' IDENTIFIER_1_0 ')'                               { $$ = new AST_FQN ( $3 ); }
    /*| PT_IDENT '(' PHYSICAL_IDENTIFIER_1_0 ')'                    { $$ = new AST_FQN ( $3 ); }*/
    | qualnames ':' PT_IDENT  '(' IDENTIFIER_1_0 ')'                { $$ = $1; $$ -> AddNode ( $5 ); }
    /*| qualnames ':' PT_IDENT  '(' PHYSICAL_IDENTIFIER_1_0 ')'     { $$ = $1; $$ -> AddNode ( $5 ); }*/
    ;

expr
    : PT_UINT '(' DECIMAL ')'   { $$ = new AST ( $3 ); }
    /*| TBD */
    ;
