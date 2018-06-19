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

#ifndef _h_schema_parse_
#define _h_schema_parse_

#ifndef _h_schema_priv_
#include "schema-priv.h"
#endif

#ifndef _h_schema_tok_
#include "schema-tok.h"
#endif

#ifndef _h_klib_symtab_
#include <klib/symtab.h>
#endif

#ifndef _h_klib_debug_
#include <klib/debug.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * defines
 */
#define PARSE_DEBUG( msg ) \
    DBGMSG ( DBG_VDB, DBG_FLAG ( DBG_VDB_PARSE ), msg )


/*--------------------------------------------------------------------------
 * forwards
 */
struct SVector;
struct STable;
struct SDatabase;
struct SFunction;
struct SConstExpr;
struct SFormParmlist;
struct SExpression;
struct STypeExpr;

/*--------------------------------------------------------------------------
 * extension of token ids
 */
enum
{
    /* a symbolic constant */
    eConstant = eNumSymtabIDs,

    /* format type */
    eFormat,

    /* dynamic data type */
    eDatatype,

    /* typeset */
    eTypeset,

    /* factory */
    eFactory,

    /* external C function */
    eFunction,
    eUntypedFunc,
    eRowLengthFunc,

    /* script function */
    eScriptFunc,

    /* schema type and param
       a schema type is a templatized type, basically
       a name without a completed definition

       a schema param is a parameterized constant that
       can be used in defining type vector dimensions */
    eSchemaType,
    eSchemaParam,

    /* factory param and name
       a factory param is a templatized constant, but
       cannot be used to define type dimensions */
    eFactParam,

    /* function param */
    eFuncParam,

    /* intermediate production */
    eProduction,

    /* physical column */
    ePhysical,

    /* physical column member */
    ePhysMember,

    /* external column */
    eColumn,

    /* forward declaration */
    eForward,

    /* purely virtual production */
    eVirtual,

    /* table */
    eTable,

    /* table member */
    eTblMember,

    /* database */
    eDatabase,

    /* database member */
    eDBMember,

    /* view */
    eView,

    /* keywords */
    eFirstKeyword,

    kw_alias = eFirstKeyword,
    kw_column,
    kw_const,
    kw_control,
    kw_database,
    kw_decode,
    kw_default,
    kw_encode,
    kw_extern,
    kw_false,
    kw_fmtdef,
    kw_function,
    kw_include,
    kw_index,
    kw_limit,
    kw_physical,
    kw_read,
    kw_readonly,
    kw_return,
    kw_schema,
    kw_static,
    kw_table,
    kw_template,
    kw_trigger,
    kw_true,
    kw_type,
    kw_typedef,
    kw_typeset,
    kw_validate,
    kw_version,
    kw_view,
    kw_virtual,
    kw_void,
    kw_writeonly,

    /* special keywords */
    kw___untyped,
    kw___row_length,
    kw___no_header

};

/* SchemaEnv
 *  states the schema language version
 *  gives switches as to features, errors, warnings, etc.
 */
typedef struct SchemaEnv SchemaEnv;
struct SchemaEnv
{
    /* maj.min.rel format,
       only maj.min supported */
    uint32_t version;

    /* V0 COMPATIBILITY SWITCHES */
    uint32_t schema_param_types_absent : 1;
    uint32_t mixed_fact_param_list : 1;
    uint32_t script_function_called_schema : 1;

    /* V1 SWITCHES */
    uint32_t default_view_decl : 1;
    uint32_t has_view_keyword : 1;
};

/* SCHEMA_LANG_VERSION
 *  version numbers of schema language
 */
#define CUR_SCHEMA_LANG_VERSION 0x01000000
#define EXT_SCHEMA_LANG_VERSION 0x01000000

/* Init
 *  initialize to current version
 */
void SchemaEnvInit ( SchemaEnv *env, uint32_t version );

/*--------------------------------------------------------------------------
 * schema-parse.c
 */

/*
 * schema             = [ <schema-version> ';' ] [ <schema-decl-list> ]
 *
 * schema-decl-list   = <schema-decl> [ <schema-decl-list> ]
 *
 * schema-decl        = <type-definition> ';'
 *                    | <typeset-definition> ';'
 *                    | <format-definition> ';'
 *                    | <function-decl> ';'
 *                    | <schema-decl>
 *                    | <table-decl>
 *
 * schema-version     = 'version' <maj-min>
 */
rc_t schema ( KTokenSource *src, VSchema *self );

/*
 * init_symtab
 *  initializes "tbl"
 *  places schema and parents into scope
 *  must be balanced by KSymTableWhack
 */
rc_t init_symtab ( KSymTable *tbl, const VSchema *self );

/* next_token
 *  gets next token
 *  looks up any identifier in symbol table
 *  converts token id to found symbol type
 * next_shallow_token
 *  only searches current scope, and optionally intrinsic scope
 */
#define next_token vdb_next_token
#define next_shallow_token vdb_next_shallow_token
KToken *next_token ( const KSymTable *tbl, KTokenSource *src, KToken *t );
KToken *next_shallow_token ( const KSymTable *tbl,
    KTokenSource *src, KToken *t, bool plus_intrinsic );

/* expect
 *  performs a comparison of t->id against id, and takes one of 3 actions:
 *  1) when ids match, advance to next token and return 0
 *  2) when ids don't match but the expected token is not required,
 *     issue a KTokenExpected warning using expected text and return 0
 *  3) when ids don't match and the expected token is required,
 *     issue a KTokenExpected error using expected text and return its rc_t
 */
rc_t expect ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    int id, const char *expected, bool required );

/*
 * maj-min            = <uint-expr> [ '.' <uint-expr> ]
 * maj-min-rel        = <uint-expr> [ '.' <uint-expr> [ '.' <uint-expr> ] ]
 */
rc_t maj_min_rel ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, uint32_t *version, bool accept_release );

rc_t next_uint ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, uint32_t *val );

/* resolve_object
 *  generic object find
 *
 *  "td" [ OUT, NULL OKAY ] - returns cast type expression
 *  if given or "any" if not
 *
 *  "name" [ OUT ] - returns list of overloaded objects if found
 *
 *  "type" [ OUT ] - returns object type id, e.g.:
 *    eDatatype, eTypeset, eFormat, eFunction, ePhysical, eTable, ...
 *
 *  "expr" [ IN ] - NUL terminated name expression identifying object
 *
 *  "ctx" [ IN ] - NUL terminated context string for evaluation,
 *  substitutes for filename in logging reports
 *
 *  "dflt" [ IN ] - if true, resolve default value
 *
 *  returns principal object identified. if NULL but "name" is not
 *  NULL, then the object was only partially identified.
 */
const void *resolve_object ( const KSymTable *tbl,
    const VSchema *self, VTypedecl *td, const SNameOverload **name,
    uint32_t *type, const char *expr, const char *ctx, bool dflt );


/*--------------------------------------------------------------------------
 * schema-type.c
 */


/*
 * fqn                = ID [ <nested-name> ]
 * nested-name        = ':' NAME [ <nested-name> ]
 */
rc_t next_fqn ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env );
rc_t create_fqn ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t id, const void *obj );


/*
 * dim                = '[' <uint-expr> ']'
 */
rc_t dim ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, uint32_t *dim, bool required );

/*
 * typename           = <fqn>
 * typedecl           = <typename> [ <dim> ]
 */
rc_t type_name ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t *id );
rc_t typedecl ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VTypedecl *td );

/*
 * typeset            = <fqn>
 * typespec           = <typedecl>
 *                    | <typeset> [ <dim> ]
 */
rc_t typeset ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t *id );
rc_t typespec ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VTypedecl *td );

/*
 * fmtname            = <fqn>
 * fmtdecl            = <fmtname> [ '/' <typedecl> ]
 *                    | <typedecl>
 * fmtspec            = <fmtname> [ '/' <typedecl> ]
 *                    | <typespec>
 */
rc_t fmtname ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, uint32_t *id );
rc_t fmtdecl ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VFormatdecl *fd );
rc_t fmtspec ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, VFormatdecl *fd );

/*
 * type-definition    = 'typedef' <typename> <typedef-list>
 * typedef-list       = <typedef-decl> [ ',' <typedef-list> ]
 * typedef-decl       = <fqn> [ <dim> ]
 */
rc_t type_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );

/*
 * typeset-definition = 'typeset' <typeset> '{' <typespec-list> '}'
 * typespec-list      = <typespec> [ ',' <typespec-list> ]
 */
rc_t typeset_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );

/*
 * format-definition  = 'fmtdef' [ <fmtname> ] <fqn>
 */
rc_t format_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );


/*--------------------------------------------------------------------------
 * schema-func.c
 */

/*
 * function-decl      = [ 'extern' ] 'function' <ext-function-decl> ';'
 *                    | 'validate' 'function' <validate-function-decl> ';'
 *                    | 'schema' [ 'function' ] <sch-func-decl>
 *                    | 'function' <ext-func-decl> ';'
 *                    | 'function' <sch-func-decl>
 */
rc_t function_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );
rc_t extfunc_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );
rc_t valfunc_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );
rc_t script_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );

/*
 * schema-signature   = <schema-formals>
 * schema-formals     = <schema-formal> [ ',' <schema-formals> ]
 * schema-formal      = <schema-typedecl> ID
 * schema-parmname    = ID
 */
rc_t schema_signature ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, struct SFunction *sig );

/*
 * fact-signature     = <fact-formals> [ '*' <fact-formals> ] [ ',' '...' ]
 *                    | '*' <fact-formals> [ ',' '...' ]
 *                    | '...'
 */
rc_t fact_signature ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, struct SFormParmlist *sig );


/*--------------------------------------------------------------------------
 * schema-prod.c
 */

/*
 * production-stmt    = <func-fmtdecl> ID <assign-expr>
 *                    | 'trigger' ID <assign-expr>
 */
rc_t production_stmt ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, struct Vector *v, uint32_t ptype );

/*
 * script-body        = '{' <script-stmts> '}'
 */
rc_t script_body ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, struct SFunction *f );

/*
 * physical-decl      = 'physical' [ 'column' ] <typedecl>
 *                       <fqn> '#' <maj-min-rel> <phys-body>
 */
rc_t physical_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );


/*--------------------------------------------------------------------------
 * schema-expr.c
 */

/*
 * dim-expr           = '[' <uint-expr> ']'
 */
rc_t dim_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, struct SExpression const **dim, bool required );


/*
 * cond-expr          = <expression> [ '|' <cond-expr> ]
 */
rc_t cond_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, struct SExpression const **expr );

/*
 * expression         = <param-value>
 *                    | <func-expr>
 *                    | '(' <func-fmtdecl> ')' <expression>
 */
rc_t expression ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, struct SExpression const **expr );

/*
 * type-expr          = <typeset>
 *                    | <fmtdecl>
 *                    | <fmtname> '/' <typeset>
 */
rc_t type_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, struct SExpression const **fd );
rc_t vardim_type_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, struct SExpression const **fd );

/*
 * const-expr         = <constname>
 *                    | CONST-VALUE
 */
rc_t const_expr ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, struct SExpression const **expr );

/*
 * phys-encoding-expr = [ '<' <schema-parms> '>' ]
 *                      <phys-encoding-name> [ '#' <maj-min-rel> ]
 *                      [ '<' <fact-params> '>' ]
 */
rc_t phys_encoding_expr ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self, VTypedecl *td, struct SExpression const **expr );


/*--------------------------------------------------------------------------
 * schema-eval.c
 */

/* eval-const-expr
 *  tries to evaluate a constant expression against type
 *  returns non-zero error code if failed
 */
rc_t eval_const_expr ( const VSchema *self, const VTypedecl *td,
    struct SExpression const *expr, struct SExpression **value, Vector *cx_bind );

/* eval-uint-expr
 *  special const expression evaluator for uint32_t
 */
rc_t eval_uint_expr ( const VSchema *self,
    struct SExpression const *expr, uint32_t *value, Vector *cx_bind );

/* eval-uint64-expr
 *  special const expression evaluator for uint64_t
 */
rc_t eval_uint64_expr ( const VSchema *self,
    struct SExpression const *expr, uint64_t *value, Vector *cx_bind );

/* eval-expr-syntax
 *  examine expression syntax
 *  fixes forward references
 */
rc_t eval_expr_syntax ( struct SExpression const *expr );


/*--------------------------------------------------------------------------
 * schema-tbl.c
 */

/*
 * physical-name      = '.' ID
 */
void physical_name ( const KSymTable *tbl,
    KTokenSource *src, KToken *t, const SchemaEnv *env );

/*
 * push-tbl-scope
 * pop-tbl-scope
 */
rc_t push_tbl_scope ( KSymTable *tbl, struct STable const *table );
void pop_tbl_scope ( KSymTable *tbl, struct STable const *table );

/*
 * init-tbl-symtab
 *  initializes "tbl"
 *  places table in scope
 *  must be balanced by KSymTableWhack
 */
rc_t init_tbl_symtab ( KSymTable *tbl, const VSchema *schema, struct STable const *table );

/*
 * table-decl         = 'table' <fqn> <table-def>
 * table-def          = [ <table-dad> ] <table-body>
 *                    | <table-dad> ';'
 *
 * table-dad          = '=' <table-name>
 *
 * table-body         = '{' [ <table-decl-list> ] '}'
 * table-decl-list    = <tbl-local-decl> ';' [ <table-decl-list> ]
 */
rc_t table_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );


/*--------------------------------------------------------------------------
 * schema-db.c
 */

/*
 * push-db-scope
 * pop-db-scope
 */
rc_t push_db_scope ( KSymTable *tbl, struct SDatabase const *db );
void pop_db_scope ( KSymTable *tbl, struct SDatabase const *db );

/*
 * init-db-symtab
 *  initializes "tbl"
 *  places db in scope
 *  must be balanced by KSymTableWhack
 */
rc_t init_db_symtab ( KSymTable *tbl, const VSchema *schema, struct SDatabase const *db );

/*
 * database-decl      = 'database' <fqn> <database-def>
 * database-def       = [ <database-dad> ] <database-body>
 *                    | <database-dad> ';'
 *
 * database-dad       = '=' <database-name>
 *
 * database-body      = '{' [ <db-decl-list> ] '}'
 * db-decl-list       = <database-decl>
 *                    | <table-decl>
 */
rc_t database_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_schema_int_ */
