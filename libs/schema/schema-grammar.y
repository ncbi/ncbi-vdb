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
        extern enum yytokentype SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb );
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

%}

%name-prefix "Schema_"
%parse-param { ctx_t ctx }
%parse-param { ParseTree** root }
%parse-param { ErrorReport * errors }
%param { struct SchemaScanBlock* sb }

%define api.value.type {SchemaToken}

%define parse.error verbose
%locations

%define api.pure full
%destructor {
    if ( $$ . subtree != NULL )
    {
        ParseTree :: Destroy ( ( ParseTree * ) ( $$ . subtree ) );
    }
    else
    {   /* terminal; release associated whitespace */
        free ( $$ . leading_ws );
    }
} <>

 /* !!! Keep token declarations in synch with schema-ast.y */

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

 /* !!! Keep token declarations above in synch with schema-ast.y */


%start parse

%%

parse
    : END_SOURCE                { *root = MakeTree ( ctx, PT_PARSE, T ( ctx, $1 ) );              $$ . subtree = 0; $$ . leading_ws = 0;  }
    | source END_SOURCE         { *root = MakeTree ( ctx, PT_PARSE, P ( $1 ), T ( ctx, $2 ) );    $$ . subtree = 0; $$ . leading_ws = 0; }
    ;

source
    : schema_1_0                    { $$ . subtree = MakeTree ( ctx, PT_SOURCE, P ( $1 ) ); }
    | version_1_0 schema_1_0_opt    { $$ . subtree = MakeTree ( ctx, PT_SOURCE, P ( $1 ), P ( $2 ) ); }
    | version_2_0 schema_2_0        { $$ . subtree = MakeTree ( ctx, PT_SOURCE, P ( $1 ), P ( $2 ) ); }
    ;

/* schema-1.0
 */
version_1_0
    : KW_version VERS_1_0 ';'   { $$ . subtree = MakeTree ( ctx, PT_VERSION_1_0, T ( ctx, $1 ), T ( ctx, $2 ), T ( ctx, $3 ) ); }
    ;

schema_1_0_opt
    : schema_1_0    { $$ . subtree = P ( $1 ); }
    | empty         { $$ = $1; }
    ;

schema_1_0
    : schema_1_0_decl              { $$ . subtree = MakeTree ( ctx, PT_SCHEMA_1_0, P ( $1 ) ); }
    | schema_1_0 schema_1_0_decl   { $$ . subtree = AddToList ( ctx, P ( $1 ) , P ( $2 ) ); }
    ;

schema_1_0_decl
    : typedef_1_0_decl      { $$ = $1; }
    | typeset_1_0_decl      { $$ = $1; }
    | format_1_0_decl       { $$ = $1; }
    | const_1_0_decl        { $$ = $1; }
    | alias_1_0_decl        { $$ = $1; }
    | function_1_0_decl     { $$ = $1; }
    | extern_1_0_decl       { $$ = $1; }
    | script_1_0_decl       { $$ = $1; }
    | validate_1_0_decl     { $$ = $1; }
    | physical_1_0_decl     { $$ = $1; }
    | table_1_0_decl        { $$ = $1; }
    | database_1_0_decl     { $$ = $1; }
    | include_directive     { $$ = $1; }
    | ';'                   { $$ . subtree = T ( ctx, $1 ); }  /* for lots of reasons, we tolerate stray semicolons */
    ;

/* typedef-1.0
 */
typedef_1_0_decl
    :   KW_typedef
        fqn_1_0 /* must be from 'typedef' or schema template */
        typedef_1_0_new_name_list
        ';'
                            { $$ . subtree = MakeTree ( ctx, PT_TYPEDEF, T ( ctx, $1 ), P ( $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    ;

typedef_1_0_new_name_list
    : typespec_1_0                                  { $$ . subtree = MakeList ( ctx, $1 ); }
    | typedef_1_0_new_name_list ',' typespec_1_0    { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

/* typeset-1.0
 */
typeset_1_0_decl
    : KW_typeset typeset_1_0_new_name typeset_1_0_def ';'
                            { $$ . subtree = MakeTree ( ctx, PT_TYPESET, T ( ctx, $1 ), P ( $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    ;

typeset_1_0_new_name
    : fqn_1_0               { $$ = $1; }                /* we allow duplicate redefinition here... */
    ;

typeset_1_0_def
    : '{' typespec_1_0_list '}'                                 /* ...obviously, the set contents must match */
            { $$ . subtree = MakeTree ( ctx, PT_TYPESETDEF, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

/* typespec-1.0
 */
typespec_1_0_list
    : typespec_1_0                          { $$ . subtree = MakeList ( ctx, $1 ); }
    | typespec_1_0_list ',' typespec_1_0    { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

typespec_1_0
    : fqn_1_0                   { $$ = $1; }
    | fqn_1_0 '[' dim_1_0 ']'   { $$ . subtree = MakeTree ( ctx, PT_ARRAY, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    ;

dim_1_0
    : expression_1_0    { $$ = $1; }        /* expects unsigned integer constant expression */
    | '*'               { $$ . subtree = T ( ctx, $1 ); }
    ;

/* format-1.0
 */
format_1_0_decl
    : KW_fmtdef format_1_0_new_name ';'
                                    { $$ . subtree = MakeTree ( ctx, PT_FORMAT, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    | KW_fmtdef format_1_0_name format_1_0_new_name ';'       /* creates a derived format */
                                    { $$ . subtree = MakeTree ( ctx, PT_FORMAT, T ( ctx, $1 ), P ( $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    ;

format_1_0_new_name
    : fqn_1_0                       { $$ = $1; }   /* allow for redefinition - current code seems to be wrong */
    ;

format_1_0_name
    : fqn_1_0                       { $$ = $1; }   /* must name a format */
    ;


/* const-1.0
 */
const_1_0_decl
    : KW_const typespec_1_0 fqn_1_0 '=' expression_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_CONST, T ( ctx, $1 ), P ( $2 ), P ( $3 ), T ( ctx, $4 ), P ( $5 ), T ( ctx, $6 ) ); }
    ;

/* alias-1.0
 */
alias_1_0_decl
    : KW_alias fqn_1_0 alias_1_0_new_name ';'
            { $$ . subtree = MakeTree ( ctx, PT_ALIAS, T ( ctx, $1 ), P ( $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    ;

alias_1_0_new_name
    : fqn_1_0                       { $$ = $1; }    /* for some reason, does not allow duplicate redefinition */
    ;

/* function-1.0
 */
function_1_0_decl
    : KW_function func_1_0_decl      { $$ . subtree = MakeTree ( ctx, PT_FUNCTION, T ( ctx, $1 ), P ( $2 ) ); }
    ;

func_1_0_decl
    : untyped_func_1_0_decl          { $$ = $1; }   /* cannot be inline or a validation function */
    | row_length_func_1_0_decl       { $$ = $1; }   /* cannot be inline or a validation function */
    | opt_func_1_0_schema_sig
      func_1_0_return_type
      fqn_opt_vers
      opt_func_1_0_fact_sig
      func_1_0_param_sig
      func_1_0_prologue
            { $$ . subtree = MakeTree ( ctx, PT_FUNCDECL, P ( $1 ), P ( $2 ), P ( $3 ), P ( $4 ), P ( $5 ), P ( $6 ) ); }
    ;

untyped_func_1_0_decl
    : KW___untyped fqn_1_0 '(' ')'
            { $$ . subtree = MakeTree ( ctx, PT_UNTYPED, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), T ( ctx, $4 ) ); }
    ;

row_length_func_1_0_decl
    : KW___row_length fqn_1_0 '(' ')'
            { $$ . subtree = MakeTree ( ctx, PT_ROWLENGTH, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), T ( ctx, $4 ) ); }
    ;

opt_func_1_0_schema_sig
    : empty                     { $$ = $1; }
    | func_1_0_schema_sig       { $$ = $1; }
    ;

func_1_0_schema_sig
    : '<' func_1_0_schema_formals '>'       { $$ . subtree = MakeTree ( ctx, PT_SCHEMASIG, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

func_1_0_schema_formals
    : func_1_0_schema_formal                                { $$ . subtree = MakeList ( ctx, $1 ); }
    | func_1_0_schema_formals ',' func_1_0_schema_formal    { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

func_1_0_schema_formal
    : KW_type ident_1_0         { $$ . subtree = MakeTree ( ctx, PT_SCHEMAFORMAL, T ( ctx, $1 ), P ( $2 ) ); }
    | type_expr_1_0 ident_1_0   { $$ . subtree = MakeTree ( ctx, PT_SCHEMAFORMAL, P ( $1 ), P ( $2 ) ); } /* type-expr must be uint */
    ;

func_1_0_return_type
    : KW_void           { $$ . subtree = MakeTree ( ctx, PT_RETURNTYPE, T ( ctx, $1 ) ); }
    | type_expr_1_0     { $$ . subtree = MakeTree ( ctx, PT_RETURNTYPE, P ( $1 ) ); }
    ;

opt_func_1_0_fact_sig
    : empty                     { $$ = $1; }
    | func_1_0_fact_sig         { $$ = $1; }
    ;

func_1_0_fact_sig
    : '<' func_1_0_fact_signature '>'  { $$ . subtree = MakeTree ( ctx, PT_FACTSIG, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

func_1_0_fact_signature
    : empty                 { $$ = $1; }
    | func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( $1 ), P ( $2 ) ); }
    | '*' func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, T ( ctx, $1 ), P ( $2 ), P ( $3 ) ); }
    | func_1_0_fact_params '*' func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( $1 ), T ( ctx, $2 ), P ( $3 ), P ( $4 ) ); }
    | func_1_0_fact_params ',' '*' func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( $1 ), T ( ctx, $2 ), T ( ctx, $3 ), P ( $4 ), P ( $5 ) ); }
    ;

func_1_0_fact_params
    : fact_param_1_0                                { $$ . subtree = MakeList ( ctx, $1 ); }
    | func_1_0_fact_params ',' fact_param_1_0       { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

fact_param_1_0
    : typespec_1_0 IDENTIFIER_1_0       { $$ . subtree = MakeTree ( ctx, PT_FORMALPARAM, P ( $1 ), T ( ctx, $2 ) ); }
    ;

func_1_0_param_sig
    : '(' func_1_0_param_signature ')'  { $$ . subtree = MakeTree ( ctx, PT_FUNCSIG, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

func_1_0_param_signature
    : empty                 { $$ = $1; }
    | func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( $1 ), P ( $2 ) ); }
    | '*' func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, T ( ctx, $1 ), P ( $2 ), P ( $3 ) ); }
    | func_1_0_formal_params '*' func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( $1 ), T ( ctx, $2 ), P ( $3 ), P ( $4 ) ); }
    | func_1_0_formal_params ',' '*' func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( ctx, PT_FUNCPARAMS, P ( $1 ), T ( ctx, $2 ), T ( ctx, $3 ), P ( $4 ), P ( $5 ) ); }
    ;

func_1_0_formal_params
    : formal_param_1_0                                  { $$ . subtree = MakeList ( ctx, $1 ); }
    | func_1_0_formal_params ',' formal_param_1_0       { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

formal_param_1_0
    : typespec_1_0 IDENTIFIER_1_0       { $$ . subtree = MakeTree ( ctx, PT_FORMALPARAM, P ( $1 ), T ( ctx, $2 ) ); }
    | KW_control typespec_1_0 IDENTIFIER_1_0
                                        { $$ . subtree = MakeTree ( ctx, PT_FORMALPARAM, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

func_1_0_vararg_formals
    : empty                     { $$ = $1; }
    | ',' ELLIPSIS              { $$ . subtree = MakeTree ( ctx, PT_ELLIPSIS, T ( ctx, $1 ), T ( ctx, $2 ) ); }
    ;

func_1_0_prologue
    : ';'                                                       /* this is a simple external function declaration    */
                                { $$ . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, $1 ) ); }
    | '=' fqn_1_0 ';'                                          /* rename the function declaration with fqn          */
                                { $$ . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    | '{' script_1_0_stmt_seq '}'                              /* this is a "script" function - cannot be extern!   */
                                { $$ . subtree = MakeTree ( ctx, PT_FUNCPROLOGUE, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

script_1_0_stmt_seq
    : script_1_0_stmt                       { $$ . subtree = MakeList ( ctx, $1 ); }
    | script_1_0_stmt_seq script_1_0_stmt   { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

script_1_0_stmt
    : KW_return cond_expr_1_0 ';'   { $$ . subtree = MakeTree ( ctx, PT_RETURN, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    | type_expr_1_0 ident_1_0 '=' cond_expr_1_0 ';'
                                     { $$ . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    ;

/* extern-1.0
 */
extern_1_0_decl
    : KW_extern ext_func_1_0_decl                               /* supposed to be an extern C function */
            { $$ . subtree = MakeTree ( ctx, PT_EXTERN, T ( ctx, $1 ), P ( $2 ) ); }
    ;

ext_func_1_0_decl
    : function_1_0_decl             { $$ = $1; }    /* there are restrictions and potentially additions */
    ;

/* script
 */

script_1_0_decl
    : KW_schema func_1_0_decl                                   /* MUST have a function body      */
                            { $$ . subtree = MakeTree ( ctx, PT_SCHEMA, T ( ctx, $1 ), P ( $2 ) ); }
    | KW_schema KW_function func_1_0_decl                       /* "function" keyword is optional */
                            { $$ . subtree = MakeTree ( ctx, PT_SCHEMA, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

/* validate
 */

validate_1_0_decl
    : KW_validate function_1_0_decl                             /* has exactly 2 parameters and is not inline */
                            { $$ . subtree = MakeTree ( ctx, PT_VALIDATE, T ( ctx, $1 ), P ( $2 ) ); }
    ;

/* physical encoding
 */
physical_1_0_decl
    :   KW_physical
        opt_func_1_0_schema_sig
        phys_1_0_return_type
        fqn_vers
        opt_func_1_0_fact_sig
        phys_1_0_prologue
            { $$ . subtree = MakeTree ( ctx, PT_PHYSICAL, T ( ctx, $1 ), P ( $2 ), P ( $3 ), P ( $4 ), P ( $5 ), P ( $6 ) ); }
    ;

phys_1_0_return_type
    : func_1_0_return_type                  { $$ = $1; }
    | KW___no_header func_1_0_return_type   { $$ . subtree = MakeTree ( ctx, PT_NOHEADER, T ( ctx, $1 ), P ( $2 ) ); }  /* not supported with schema signature */
    ;

phys_1_0_prologue
    : '=' phys_1_0_stmt                                         /* shorthand for decode-only rules */
            { $$ . subtree = MakeTree ( ctx, PT_PHYSPROLOGUE, T ( ctx, $1 ), P ( $2 ) ); }
    | '{' phys_1_0_body '}'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSPROLOGUE, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

phys_1_0_body
    : phys_1_0_body_stmt                    { $$ . subtree = MakeList ( ctx, $1 ); }
    | phys_1_0_body phys_1_0_body_stmt      { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

phys_1_0_body_stmt
    : ';'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, $1 ) ); }
    | KW_decode phys_1_0_stmt
            { $$ . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, $1 ), P ( $2 ) ); }
    | KW_encode phys_1_0_stmt
            { $$ . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, $1 ), P ( $2 ) ); }
    | KW___row_length '=' fqn_1_0 '(' ')'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSBODYSTMT, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ), T ( ctx, $5 ) ); }
    ;

phys_1_0_stmt
    : '{' script_1_0_stmt_seq '}'                              /* with caveat that magic parameter "@" is defined */
            { $$ . subtree = MakeTree ( ctx, PT_PHYSSTMT, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

/* table
 */
table_1_0_decl
    :   KW_table
        fqn_vers
        opt_tbl_1_0_parents
        tbl_1_0_body
            { $$ . subtree = MakeTree ( ctx, PT_TABLE, T ( ctx, $1 ), P ( $2 ), P ( $3 ), P ( $4 ) ); }
    ;

opt_tbl_1_0_parents
    : empty                     { $$ = $1; }
    | '=' tbl_1_0_parents       { $$ . subtree = MakeTree ( ctx, PT_TABLEPARENTS, T ( ctx, $1 ), P ( $2 ) ); }
    ;

tbl_1_0_parents
    : fqn_opt_vers                        { $$ . subtree = MakeList ( ctx, $1 ); }
    | tbl_1_0_parents ',' fqn_opt_vers    { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

tbl_1_0_body
    : '{' tbl_1_0_stmt_seq '}'
            { $$ . subtree = MakeTree ( ctx, PT_TABLEBODY, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    | '{' '}'
            { $$ . subtree = MakeTree ( ctx, PT_TABLEBODY, T ( ctx, $1 ), T ( ctx, $2 ) ); }
    ;

tbl_1_0_stmt_seq
    : tbl_1_0_stmt                      { $$ . subtree = MakeList ( ctx, $1 ); }
    | tbl_1_0_stmt_seq tbl_1_0_stmt     { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

tbl_1_0_stmt
    : production_1_0_stmt                       { $$ = $1; }
    | col_1_0_modifiers col_1_0_decl            { $$ . subtree = MakeTree ( ctx, PT_COLUMN, P ( $1 ), P ( $2 ) ); }
    | KW_column KW_limit '=' expression_1_0 ';'
                        { $$ . subtree = MakeTree ( ctx, PT_COLUMNEXPR, T ( ctx, $1 ), T ( ctx, $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    | KW_column KW_default KW_limit '=' expression_1_0 ';'
                        { $$ . subtree = MakeTree ( ctx, PT_COLUMNEXPR, T ( ctx, $1 ), T ( ctx, $2 ), T ( ctx, $3 ), T ( ctx, $4 ), P ( $5 ), T ( ctx, $6 ) ); }
    | KW_static physmbr_1_0_decl                { $$ . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, $1 ), P ( $2 ) ); }
    | KW_physical physmbr_1_0_decl              { $$ . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, $1 ), P ( $2 ) ); }
    | KW_static KW_physical physmbr_1_0_decl    { $$ . subtree = MakeTree ( ctx, PT_PHYSCOL, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    | default_view_1_0_decl                     { $$ = $1; }
    | KW___untyped '=' fqn_1_0 '(' ')' ';'
                        { $$ . subtree = MakeTree ( ctx, PT_COLUNTYPED, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ), T ( ctx, $5 ), T ( ctx, $6 ) ); }
    | ';'                                       { $$ . subtree = T ( ctx, $1 ); }
    ;

production_1_0_stmt
    : typespec_1_0 ident_1_0 '=' cond_expr_1_0 ';'    /* cannot have format */
                                     { $$ . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    | KW_trigger ident_1_0 '=' cond_expr_1_0 ';'
                                     { $$ . subtree = MakeTree ( ctx, PT_PRODTRIGGER, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    ;

col_1_0_modifiers
    : KW_column                         { $$ . subtree = T ( ctx, $1 ); }
    | col_1_0_modifier_seq KW_column    { $$ = $1; AddToList ( ctx, P ( $1 ), T ( ctx, $2 ) ); }
    ;

col_1_0_modifier_seq
    : col_1_0_modifier                          { $$ . subtree = MakeList ( ctx, $1 ); }
    | col_1_0_modifier_seq col_1_0_modifier     { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

col_1_0_modifier
    : KW_default        { $$ . subtree = T ( ctx, $1 ); }
    | KW_extern         { $$ . subtree = T ( ctx, $1 ); }
    | KW_readonly       { $$ . subtree = T ( ctx, $1 ); }
    ;

col_1_0_decl
    : typespec_1_0 typed_column_decl_1_0
            { $$ . subtree = MakeTree ( ctx, PT_COLDECL, P ( $1 ), P ( $2 ) ); }
    | phys_enc_ref typed_column_decl_1_0
            { $$ . subtree = MakeTree ( ctx, PT_COLDECL, P ( $1 ), P ( $2 ) ); }
    ;

phys_enc_ref
    : '<' schema_parms_1_0 '>' fqn_opt_vers opt_factory_parms_1_0
            { $$ . subtree = MakeTree ( ctx, PT_PHYSENCREF, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), P ( $5 ) ); }
    | fqn_vers opt_factory_parms_1_0
            { $$ . subtree = MakeTree ( ctx, PT_PHYSENCREF, P ( $1 ), P ( $2 ) ); }
    | fqn_1_0 factory_parms_1_0
            { $$ . subtree = MakeTree ( ctx, PT_PHYSENCREF, P ( $1 ), P ( $2 ) ); }
    ;

typed_column_decl_1_0
    : col_ident '{' opt_column_body_1_0 '}'
            { $$ . subtree = MakeTree ( ctx, PT_TYPEDCOL, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    | col_ident '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_TYPEDCOLEXPR, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    | col_ident ';'
            { $$ . subtree = MakeTree ( ctx, PT_TYPEDCOL, P ( $1 ), T ( ctx, $2 ) ); }
    ;

col_ident
    : ident_1_0                     { $$ = $1; }
    | phys_ident                    { $$ = $1; }
    ;

phys_ident
    : PHYSICAL_IDENTIFIER_1_0       { $$ . subtree = T ( ctx, $1 ); }     /* starts with a '.' */
    ;

opt_column_body_1_0
    : empty             { $$ = $1; }
    | column_body_1_0   { $$ = $1; }
    ;

column_body_1_0
    : column_stmt_1_0                   { $$ . subtree = MakeList ( ctx, $1 ); }
    | column_body_1_0 column_stmt_1_0   { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

column_stmt_1_0
    : KW_read '=' cond_expr_1_0 ';'     { $$ . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    | KW_validate '=' cond_expr_1_0 ';' { $$ . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    | KW_limit '=' uint_expr_1_0 ';'    { $$ . subtree = MakeTree ( ctx, PT_COLSTMT, T ( ctx, $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    | ';'                               { $$ . subtree = T ( ctx, $1 ); }
    ;

default_view_1_0_decl
    :   KW_default KW_view STRING ';'
            { $$ . subtree = MakeTree ( ctx, PT_DFLTVIEW, T ( ctx, $1 ), T ( ctx, $2 ), T ( ctx, $3 ), T ( ctx, $4 ) ); }
    ;

physmbr_1_0_decl
    : phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSMBR, P ( $1 ), T ( ctx, $2 ), T ( ctx, $3 ) ); }
    | phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSMBR, P ( $1 ), T ( ctx, $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    | KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSMBR, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), T ( ctx, $4 ) ); }
    | KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_PHYSMBR, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), T ( ctx, $4 ), P ( $5 ), T ( ctx, $6 ) ); }
    ;

phys_coldef_1_0
    : opt_col_schema_parms_1_0 fqn_opt_vers opt_factory_parms_1_0
            { $$ . subtree = MakeTree ( ctx, PT_PHYSCOLDEF, P ( $1 ), P ( $2 ), P ( $3 ) ); }
    ;

opt_col_schema_parms_1_0
    : empty                             { $$ = $1; }
    | '<' col_schema_parms_1_0 '>'      { $$ . subtree = MakeTree ( ctx, PT_COLSCHEMAPARMS, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

col_schema_parms_1_0
    : col_schema_parm_1_0                           { $$ . subtree = MakeList ( ctx, $1 ); }
    | col_schema_parms_1_0 ',' col_schema_parm_1_0  { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

col_schema_parm_1_0
    : fqn_1_0 '=' col_schema_value_1_0      { $$ . subtree = MakeTree ( ctx, PT_COLSCHEMAPARAM, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    | col_schema_value_1_0                  { $$ = $1; }
    ;

col_schema_value_1_0
    : fqn_1_0                               { $$ = $1; }
    | uint_expr_1_0                         { $$ = $1; }
    ;

/* expression
 */

cond_expr_1_0
    : expression_1_0                        { $$ . subtree = MakeList ( ctx, $1 ); }
    | cond_expr_1_0 '|' expression_1_0      { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

expression_1_0
    : primary_expr_1_0                      { $$ = $1; }
    | '(' type_expr_1_0 ')' expression_1_0  { $$ . subtree = MakeTree ( ctx, PT_CASTEXPR, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ) ); }
    ;

primary_expr_1_0
    : fqn_1_0                   { $$ = $1; }
    | phys_ident                { $$ = $1; }
    | '@'                       { $$ . subtree = T ( ctx, $1 ); }
    | func_expr_1_0             { $$ = $1; }
    | uint_expr_1_0             { $$ = $1; }
    | float_expr_1_0            { $$ = $1; }
    | string_expr_1_0           { $$ = $1; }
    | const_vect_expr_1_0       { $$ = $1; }
    | bool_expr_1_0             { $$ = $1; }
    | '-' expression_1_0        { $$ . subtree = MakeTree ( ctx, PT_NEGATE, T ( ctx, $1 ), P ( $2 ) ); }
    | '+' expression_1_0        { $$ . subtree = MakeTree ( ctx, PT_UNARYPLUS, T ( ctx, $1 ), P ( $2 ) ); }
    | member_expr_2_0           { $$ = $1; }
    | join_expr_2_0             { $$ = $1; }
    ;

func_expr_1_0
    :   '<' schema_parms_1_0 '>'
        fqn_opt_vers
        opt_factory_parms_1_0
        '('
        opt_func_1_0_parms
        ')'
             { $$ . subtree = MakeTree ( ctx, PT_FUNCEXPR, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), P ( $5 ), T ( ctx, $6 ), P ( $7 ), T ( ctx, $8 ) ); }
    |   fqn_opt_vers
        opt_factory_parms_1_0
        '('
        opt_func_1_0_parms
        ')'
             { $$ . subtree = MakeTree ( ctx, PT_FUNCEXPR, P ( $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    ;

schema_parms_1_0
    : schema_parm_1_0                       { $$ . subtree = MakeList ( ctx, $1 ); }
    | schema_parms_1_0 ',' schema_parm_1_0  { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

schema_parm_1_0
    : fqn_1_0                   { $$ = $1; }
    | fqn_1_0 '[' dim_1_0 ']'   { $$ . subtree = MakeTree ( ctx, PT_ARRAY, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    | uint_expr_1_0             { $$ = $1; }
    ;

opt_factory_parms_1_0
    : empty             { $$ = $1; }
    | factory_parms_1_0 { $$ = $1; }
    ;

factory_parms_1_0
    : '<' factory_parms '>' { $$ . subtree = MakeTree ( ctx, PT_FACTPARMS, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

factory_parms
    : expression_1_0                    { $$ . subtree = MakeList ( ctx, $1 ); }
    | factory_parms ',' expression_1_0  { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

opt_func_1_0_parms
    : empty                                 { $$ = $1; }
    | func_1_0_parms                        { $$ = $1; }
    ;

func_1_0_parms
    : expression_1_0                        { $$ . subtree = MakeList ( ctx, $1 ); }
    | func_1_0_parms ',' expression_1_0     { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

uint_expr_1_0
    : DECIMAL                               { $$ . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, $1 ) ); }
    | HEX                                   { $$ . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, $1 ) ); }
    | OCTAL                                 { $$ . subtree = MakeTree ( ctx, PT_UINT, T ( ctx, $1 ) ); }
    ;

float_expr_1_0
    : FLOAT_                     { $$ . subtree = T ( ctx, $1 ); }
    | EXP_FLOAT                 { $$ . subtree = T ( ctx, $1 ); }
    ;

string_expr_1_0
    : STRING                    { $$ . subtree = T ( ctx, $1 ); }
    | ESCAPED_STRING            { $$ . subtree = T ( ctx, $1 ); }
    ;

const_vect_expr_1_0
    : '[' opt_const_vect_exprlist_1_0 ']'      { $$ . subtree = MakeTree ( ctx, PT_CONSTVECT, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

opt_const_vect_exprlist_1_0
    : empty                     { $$ = $1; }
    | const_vect_exprlist_1_0   { $$ = $1; }
    ;

const_vect_exprlist_1_0
    : expression_1_0                                { $$ . subtree = MakeList ( ctx, $1 ); }
    | const_vect_exprlist_1_0 ',' expression_1_0    { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

bool_expr_1_0
    : KW_true                   { $$ . subtree = T ( ctx, $1 ); }
    | KW_false                  { $$ . subtree = T ( ctx, $1 ); }
    ;

type_expr_1_0
    : typespec_1_0              { $$ = $1; } /* datatype, typeset, schematype */
    | fqn_1_0 '/' fqn_1_0       { $$ . subtree = MakeTree ( ctx, PT_TYPEEXPR, P ( $1 ), T ( ctx, $2), P ( $3 ) ); } /* format /  type */
    ;

member_expr_2_0
    : ident_1_0 '.' ident_1_0           { $$ . subtree = MakeTree ( ctx, PT_MEMBEREXPR, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    | ident_1_0 PHYSICAL_IDENTIFIER_1_0 { $$ . subtree = MakeTree ( ctx, PT_MEMBEREXPR, P ( $1 ), T ( ctx, $2 ) ); }
    ;

join_expr_2_0
    : ident_1_0 '[' cond_expr_1_0 ']' '.' ident_1_0
        { $$ . subtree = MakeTree ( ctx, PT_JOINEXPR, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ), T ( ctx, $5 ), P ($6 ) ); }
    | ident_1_0 '[' cond_expr_1_0 ']' PHYSICAL_IDENTIFIER_1_0
        { $$ . subtree = MakeTree ( ctx, PT_JOINEXPR, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ), T ( ctx, $5 ) ); }
    ;

 /* database */

database_1_0_decl
    :   KW_database
        fqn_vers
        opt_database_dad_1_0
        database_body_1_0
            { $$ . subtree = MakeTree ( ctx, PT_DATABASE, T ( ctx, $1 ), P ( $2), P ( $3 ), P ( $4 ) ); }
    ;

opt_database_dad_1_0
    : empty                         { $$ = $1; }
    | '=' fqn_opt_vers              { $$ . subtree = MakeTree ( ctx, PT_DBDAD, T ( ctx, $1 ), P ( $2 ) ); }
    ;

database_body_1_0
    : '{' '}'                       { $$ . subtree = MakeTree ( ctx, PT_DBBODY, T ( ctx, $1 ), T ( ctx, $2 ) ); }
    | '{' database_members_1_0 '}'  { $$ . subtree = MakeTree ( ctx, PT_DBBODY, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ) ); }
    ;

database_members_1_0
    : database_member_1_0                       { $$ . subtree = MakeList ( ctx, $1 ); }
    | database_members_1_0 database_member_1_0  { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

database_member_1_0
    : db_member_1_0             { $$ = $1; }
    | table_member_1_0          { $$ = $1; }
    | ';'                       { $$ . subtree = T ( ctx, $1 ); }
    ;

opt_template_1_0
    : empty                     { $$ = $1; }
    | KW_template               { $$ . subtree = T ( ctx, $1 ); }
    ;

db_member_1_0
    : opt_template_1_0 KW_database fqn_opt_vers ident_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_DBMEMBER, P ( $1 ), T ( ctx, $2 ), P ( $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    ;

table_member_1_0
    : opt_template_1_0 KW_table fqn_opt_vers ident_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_TBLMEMBER, P ( $1 ), T ( ctx, $2 ), P ( $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    ;

/* include
 */

include_directive
    : KW_include STRING     { $$ . subtree = MakeTree ( ctx, PT_INCLUDE, T ( ctx, $1 ), T ( ctx, $2 ) ); }
    ;



/* other stuff
 */
fqn_1_0
    : ident_1_0                 { $$ . subtree = MakeTree ( ctx, PT_FQN, P ( $1 ) ); }
    | fqn_1_0 ':' ident_1_0     { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    /* a hack to handle keywords used as namespace identifiers in existing 1.0 schemas */
    | fqn_1_0 ':' KW_database   { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_decode     { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_encode     { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_read       { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_table      { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_type       { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_view       { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    | fqn_1_0 ':' KW_write      { $3 . type = IDENTIFIER_1_0; $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), MakeTree ( ctx, PT_IDENT, T ( ctx, $3 ) ) ); }
    ;

ident_1_0
    : IDENTIFIER_1_0    { $$ . subtree = MakeTree ( ctx, PT_IDENT, T ( ctx, $1 ) ); }     /* this is just a C identifier */
    ;

empty
    : %empty    { $$ . subtree = MakeTree ( ctx, PT_EMPTY ); }
    ;

fqn_vers
    :   fqn_1_0 VERSION     { $$ . subtree = MakeTree ( ctx, PT_VERSNAME, P ( $1 ), T ( ctx, $2 ) ); }
    ;

fqn_opt_vers
    :   fqn_1_0      { $$ = $1; }
    |   fqn_vers     { $$ = $1; }
    ;

/*************************** V 2.0 ******************************/

version_2_0
    : KW_version VERS_2_0 ';'      { $$ . subtree = MakeTree ( ctx, PT_VERSION_2, T ( ctx, $1 ), T ( ctx, $2 ), T ( ctx, $3 ) ); }
    ;

schema_2_0
    : schema_2_0_decl              { $$ . subtree = MakeTree ( ctx, PT_SCHEMA_2_0, P ( $1 ) ); }
    | schema_2_0 schema_2_0_decl   { $$ . subtree = AddToList ( ctx, P ( $1 ) , P ( $2 ) ); }
    ;

schema_2_0_decl
    : schema_1_0_decl   { $$ = $1; }
    | view_2_0_decl     { $$ = $1; }
    ;

view_2_0_decl
    : KW_view fqn_vers '<' view_parms '>' opt_view_parents '{' opt_view_body '}'
        { $$ . subtree = MakeTree ( ctx, PT_VIEW, T ( ctx, $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ), P ( $6 ), T ( ctx, $7 ), P ( $8 ), T ( ctx, $9 ) ); }
    ;

view_parms
    : view_parm                 { $$ . subtree = MakeList ( ctx, $1 ); }
    | view_parms ',' view_parm  { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

view_parm
    :  fqn_opt_vers ident_1_0   { $$ . subtree = MakeTree ( ctx, PT_VIEWPARAM, P ( $1 ), P ( $2 ) ); }
    ;

opt_view_body
    : empty     { $$ = $1; }
    | view_body { $$ = $1; }
    ;

view_body
    : view_member           { $$ . subtree = MakeList ( ctx, $1 ); }
    | view_body view_member { $$ . subtree = AddToList ( ctx, P ( $1 ), P ( $2 ) ); }
    ;

view_member
    : typespec_1_0 ident_1_0 '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_PRODSTMT, P ( $1 ), P ( $2 ), T ( ctx, $3 ), P ( $4 ), T ( ctx, $5 ) ); }
    | KW_column typespec_1_0 ident_1_0 '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( ctx, PT_COLUMN, T ( ctx, $1 ), P ( $2 ), P ( $3 ), T ( ctx, $4 ), P ( $5 ), T ( ctx, $6 ) ); }
    | ';'   { $$ . subtree = T ( ctx, $1 ); }
    ;

opt_view_parents
    : empty                     { $$ = $1; }
    | '=' view_parents          { $$ . subtree = MakeTree ( ctx, PT_VIEWPARENTS, T ( ctx, $1 ), P ( $2 ) ); }
    ;

view_parents
    : view_parent                  { $$ . subtree = MakeList ( ctx, $1 ); }
    | view_parents ',' view_parent { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;

view_parent
    : fqn_opt_vers '<' view_parent_parms'>'
        { $$ . subtree = MakeTree ( ctx, PT_VIEWPARENT, P ( $1 ), T ( ctx, $2 ), P ( $3 ), T ( ctx, $4 ) ); }
    ;

view_parent_parms
    : ident_1_0                         { $$ . subtree = MakeList ( ctx, $1 ); }
    | view_parent_parms ',' ident_1_0   { $$ . subtree = AddToList ( ctx, P ( $1 ), T ( ctx, $2 ), P ( $3 ) ); }
    ;
