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

#include <vdb/extern.h>

#include "schema-priv.h"
#include "schema-expr.h"
#include "schema-parse.h"
#include "schema-dump.h"

#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/symbol.h>
#include <klib/symtab.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * SchemaEnv
 */

/* Init - PRIVATE
 *  initialize to current version
 */
static
void SchemaEnvInitFeatures ( SchemaEnv *env )
{
    assert ( env != NULL );

    switch ( env -> version >> 24 )
    {
    case 0:
        env -> schema_param_types_absent = 1;
        env -> mixed_fact_param_list = 1;
        env -> script_function_called_schema = 1;
        break;
    case 1:
        switch ( ( uint8_t ) ( env -> version >> 16 ) )
        {
        case 1:
            env -> default_view_decl = 1;
            env -> has_view_keyword = 1;
            break;
        }
    }
}

void SchemaEnvInit ( SchemaEnv *env, uint32_t version )
{
    assert ( env != NULL );
    memset ( env, 0, sizeof * env );

    /* set version */
    env -> version = version;
    SchemaEnvInitFeatures ( env );
}


/*--------------------------------------------------------------------------
 * SConstant
 */
#if SLVL >= 2

/* Whack
 */
void CC SConstantWhack ( void *item, void *ignore )
{
    SConstant *self = item;
    SExpressionWhack ( self -> expr );
    free ( self );
}

/* Mark
 */
void CC SConstantClearMark ( void *item, void *ignore )
{
    SConstant *self = item;
    self -> marked = false;
}

void SConstantMark ( const SConstant *cself )
{
    SConstant *self = ( SConstant* ) cself;
    if ( self != NULL && ! self -> marked )
        self -> marked = true;
}

/* Dump
 *  dump "const", dump object
 */
rc_t SConstantDump ( const SConstant *self, struct SDumper *d )
{
    return FQNDump ( self != NULL ? self -> name : NULL, d );
}

bool CC SConstantDefDump ( void *item, void *data )
{
    SDumper *b = data;
    const SConstant *self = ( const void* ) item;

    if ( SDumperMarkedMode ( b ) && ! self -> marked )
        return false;

    if ( SDumperMode ( b ) == sdmCompact )
    {
        return SDumperPrint ( b, "const %T %N=%E;",
            & self -> td, self -> name, self -> expr );
    }

    return SDumperPrint ( b, "const %T %N = %E;\n",
        & self -> td, self -> name, self -> expr );
}

#endif


/*--------------------------------------------------------------------------
 * VSchema
 */


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
static
int CC SColumnSortByID ( const void **item, const void **n, void *ignore )
{
    const SColumn *a = * item;
    const SColumn *b = * n;
    return VCtxIdCmp ( & a -> cid, & b -> cid );
}

static
void CC SColumnLastDefault ( void *item, void *data )
{
    const SColumn *self = ( const SColumn* ) item;
    const SColumn **colp = ( const SColumn** ) data;

    if ( self -> dflt || * colp == NULL )
        * colp = self;
}

typedef struct column_best_fit_data column_best_fit_data;
struct column_best_fit_data
{
    const SColumn *scol;
    const VSchema *schema;
    VTypedecl td;
    uint32_t distance;
};

static
void CC SColumnBestFit ( void *item, void *data )
{
    const SColumn *scol = ( const void * ) item;
    if ( scol -> td . type_id != 0 )
    {
        uint32_t distance;
        column_best_fit_data *pb = data;
        if ( VTypedeclToTypedecl ( & scol -> td, pb -> schema, & pb -> td, NULL, & distance ) )
        {
            if ( pb -> scol == NULL || pb -> distance > distance )
            {
                pb -> scol = scol;
                pb -> distance = distance;
            }
        }
    }
}

const void *resolve_object ( const KSymTable *tbl,
    const VSchema *self, VTypedecl *tdp, const SNameOverload **namep,
     uint32_t *type, const char *expr, const char *ctx, bool dflt )
{
    rc_t rc;
    const void *obj;
    
    VTypedecl td;
    bool has_type = false;

    KToken t;
    KTokenText tt;
    KTokenSource src;

    SchemaEnv env;
    SchemaEnvInit ( & env, EXT_SCHEMA_LANG_VERSION );

    KTokenTextInitCString ( & tt, expr, ctx );
    KTokenSourceInit ( & src, & tt );
    next_token ( tbl, & src, & t );

    /* initialize return values */
    obj = NULL;

    /* ready to go - can accept a typecast */
    if ( t . id == eLeftParen )
    {
        has_type = true;
        next_token ( tbl, & src, & t );
        rc = typedecl ( tbl, & src, & t, & env, self, & td );
        if ( rc == 0 )
            rc = expect ( tbl, & src, & t, eRightParen, ")", true );
        if ( tdp != NULL )
            * tdp = td;
    }

    /* can also accept a physical name */
    else 
    {
        if ( t . id == ePeriod )
            physical_name ( tbl, & src, & t, & env );
        if ( tdp != NULL )
            memset ( tdp, 0, sizeof * tdp );
        rc = 0;
    }

    /* we recognize a column name or
       a fully-qualified name optionally followed by
       a version to fully-qualify obj */
    if ( rc == 0 )
        rc = next_fqn ( tbl, & src, & t, & env );
    if ( rc == 0 )
    {
        uint32_t vers;
        KToken st = t;
        bool has_vers = false;
        bool needs_name = true;
        const SNameOverload *name = NULL;

        next_token ( tbl, & src, & t );
        switch ( st . id )
        {
        case eConstant:
        case eFormat:
        case eDatatype:
        case eTypeset:
        case ePhysMember:
        case eTblMember:
        case eDBMember:
            needs_name = false;
            obj = st . sym -> u . obj;
            break;
        case eFunction:
        case eScriptFunc:
        case ePhysical:
        case eTable:
        case eDatabase:
            if ( t . id == eHash )
            {
                next_token ( tbl, & src, & t );
                rc = maj_min_rel ( tbl, & src, & t, & env, self, & vers, true );
                if ( rc != 0 )
                    break;
                has_vers = true;
            }
        case eUntypedFunc:
        case eColumn:
            name = st . sym -> u . obj;
            break;
        }

        /* check if we found anything at all */
        if ( obj != NULL || name != NULL )
        {
            /* accept but don't require semi-colon */
            if ( t . id == eSemiColon )
                next_token ( tbl, & src, & t );

            /* expect end of input or else we didn't recognize */
            if ( t . id != eEndOfInput )
            {
                obj = NULL;
                name = NULL;
            }
        }

        /* if we have something */
        if ( obj != NULL || name != NULL )
        {
            * type = st . id;

            /* resolve by version */
            if ( has_vers )
            {
                assert ( name != NULL );
                switch ( st . id )
                {
                case eFunction:
                case eScriptFunc:
                    obj = VectorFind ( & name -> items, & vers, NULL, SFunctionCmp );
                    break;
                case ePhysical:
                    obj = VectorFind ( & name -> items, & vers, NULL, SPhysicalCmp );
                    break;
                case eTable:
                    obj = VectorFind ( & name -> items, & vers, NULL, STableCmp );
                    break;
                case eDatabase:
                    obj = VectorFind ( & name -> items, & vers, NULL, SDatabaseCmp );
                    break;
                }

                /* since a version was requested,
                   don't allow a partial find */
                if ( obj == NULL )
                    name = NULL;
            }

            /* resolve by type or default */
            else if ( needs_name )
            {
                assert ( name != NULL );
                switch ( st . id )
                {
                case eColumn:
                    if ( ! has_type )
                    {
                        Vector items;
                        rc = VectorCopy ( & name -> items, & items );
                        if ( rc != 0 )
                            break;
                        VectorReorder ( & items, SColumnSortByID, NULL );
                        VectorForEach ( & items, false, SColumnLastDefault, (void*)& obj );
                        VectorWhack ( & items, NULL, NULL );
                    }
                    else
                    {
                        column_best_fit_data pb;
                        pb . scol = NULL;
                        pb . schema = self;
                        pb . td = td;
                        pb . distance = -1;

                        VectorForEach ( & name -> items, false, SColumnBestFit, & pb );
                        obj = pb . scol;
                        if ( obj == NULL )
                        {
                            /* if named column is incomplete, allow match */
                            memset ( & td, 0, sizeof td );
                            obj = VectorFind ( & name -> items, & td, NULL, SColumnCmp );

                            /* if column could not be matched by type, forget it */
                            if ( obj == NULL )
                                name = NULL;
                        }
                    }
                    break;

                default:
                    if ( dflt )
                        obj = VectorLast ( & name -> items );
                }
            }

            * namep = name;
        }
    }

    return obj;
}

/* Find
 *  generic object find
 *
 *  "name" [ OUT ] - returns list of overloaded names if found
 *
 *  "type" [ OUT ] - returns object type id, e.g.:
 *    eDatatype, eTypeset, eFormat, eFunction, ePhysical, eTable, ...
 *
 *  "expr" [ IN ] - name expression identifying object
 *
 *  returns principal object identified. if NULL but "name" is not
 *  NULL, then the object was only partially identified.
 */
const void *VSchemaFind ( const VSchema *self,
    const SNameOverload **name, uint32_t *type,
    const char *expr, const char *ctx, bool dflt )
{
    rc_t rc;
    KSymTable tbl;

    /* initialize to not-found */
    const void *obj = NULL;
    * name = NULL;
    * type = 0;

    /* build a symbol table for schema */
    rc = init_symtab ( & tbl, self );
    if ( rc == 0 )
    {
        obj = resolve_object ( & tbl, self, NULL, name, type, expr, ctx, dflt );
        KSymTableWhack ( & tbl );
    }

    return obj;
}


/* next_token
 *  gets next token
 *  looks up any identifier in symbol table
 *  converts token id to found symbol type
 */
KToken *next_token ( const KSymTable *tbl, KTokenSource *src, KToken *t )
{
    if ( KTokenizerNext ( kDefaultTokenizer, src, t ) -> id == eIdent )
    {
        KSymbol *sym = KSymTableFind ( tbl, & t -> str );
        t -> sym = sym;
        if ( sym != NULL )
            t -> id = sym -> type;
    }

    return t;
}

KToken *next_shallow_token ( const KSymTable *tbl, KTokenSource *src, KToken *t, bool plus_intrinsic )
{
    if ( KTokenizerNext ( kDefaultTokenizer, src, t ) -> id == eIdent || t -> id == eName )
    {
        KSymbol *sym = KSymTableFindShallow ( tbl, & t -> str );
        if ( sym == NULL && plus_intrinsic )
            sym = KSymTableFindIntrinsic ( tbl, & t -> str );
        t -> sym = sym;
        if ( sym != NULL )
            t -> id = sym -> type;
    }

    return t;
}


/* expect
 *  performs a comparison of t->id against id, and takes one of 3 actions:
 *  1) when ids match, advance to next token and return 0
 *  2) when ids don't match but the expected token is not required,
 *     issue a KTokenExpected warning using expected text and return 0
 *  3) when ids don't match and the expected token is required,
 *     issue a KTokenExpected error using expected text and return its rc_t
 */
rc_t expect ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    int id, const char *expected, bool required )
{
    if ( t -> id == id )
        next_token ( tbl, src, t );
    else if ( ! required )
        KTokenExpected ( t, klogWarn, expected );
    else
        return KTokenExpected ( t, klogErr, expected );
    return 0;
}


/*
 * maj-min            = <uint-expr> [ '.' <uint-expr> ]
 * maj-min-rel        = <uint-expr> [ '.' <uint-expr> [ '.' <uint-expr> ] ]
 */

#if SLVL >= 1
rc_t maj_min_rel ( const KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, const VSchema *self, uint32_t *version, bool accept_release )
{
    rc_t rc;

    if ( ! accept_release && t -> id == eMajMinRel )
        return KTokenExpected ( t, klogErr, "maj.min version" );

    rc = KTokenToVersion ( t, version );
    if ( rc != 0 )
        KTokenExpected ( t, klogErr, "version spec" );

    next_token ( tbl, src, t );
    return rc;
}
#endif


/*
 * const-definition   = 'const' <typedecl> <fqn> '=' <const-expr>
 */
#if SLVL >= 2
static
rc_t const_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    rc_t rc;
    SConstant *cnst = malloc ( sizeof * cnst );
    if ( cnst == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    rc = typedecl ( tbl, src, t, env, self, & cnst -> td );
    if ( rc == 0 )
        rc = create_fqn ( tbl, src, t, env, eConstant, cnst );
    if ( rc == 0 )
    {
        cnst -> name = t -> sym;
        rc = expect ( tbl, src, next_token ( tbl, src, t ), eAssign, "=", true );
        if ( rc == 0 )
            rc = const_expr ( tbl, src, t, env, self, & cnst -> expr );
    }
    else if ( GetRCState ( rc ) == rcExists )
    {
        cnst -> name = t -> sym;
        rc = expect ( tbl, src, next_token ( tbl, src, t ), eAssign, "=", true );
        if ( rc == 0 )
            rc = const_expr ( tbl, src, t, env, self, & cnst -> expr );
        if ( rc == 0 )
        {
            /* TBD - compare expressions for equivalence */
            SConstantWhack ( cnst, NULL );
            return expect ( tbl, src, t, eSemiColon, ";", true );
        }
    }

    if ( rc != 0 )
    {
        free ( cnst );
        return rc;
    }

    rc = VectorAppend ( & self -> cnst, & cnst -> id, cnst );
    if ( rc != 0 )
    {
        SConstantWhack ( cnst, NULL );
        return KTokenRCExplain ( t, klogInt, rc );
    }

    return expect ( tbl, src, t, eSemiColon, ";", true );
}
#endif


/*
 * alias-definition   = 'alias' SYMBOL <fqn>
 */
static
rc_t alias_definition ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    const KSymbol *sym;

    /* read symbol name */
    rc_t rc = next_fqn ( tbl, src, t, env );
    if ( rc != 0 )
        return KTokenFailure ( t, klogErr, rc, "fully qualified name" );

    /* remember symbol */
    sym = t -> sym;

    /* create new name */
    rc = create_fqn ( tbl, src, next_token ( tbl, src, t ),
        env, sym -> type, sym -> u . obj );
    if ( rc != 0 )
    {
        if ( GetRCState ( rc ) != rcExists )
            return KTokenFailure ( t, klogErr, rc, "fully qualified name" );

        /* TBD - compare alias */
        return expect ( tbl, src, next_token ( tbl, src, t ), eSemiColon, ";", true );
    }

    /* record alias */
    rc = VectorAppend ( & self -> alias, NULL, t -> sym );
    if ( rc != 0 )
        return KTokenRCExplain ( t, klogErr, rc );

    return expect ( tbl, src, next_token ( tbl, src, t ), eSemiColon, ";", true );
}


/*
 * extern-decl        = 'extern' 'function' <ext-function-decl> ';'
 */
static
rc_t extern_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    switch ( t -> id )
    {
#if SLVL >= 3
    case kw_function:
        return extfunc_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
    default:
        break;
    }

    return KTokenExpected ( t, klogErr, "function" );
}


/*
 * validate-decl      = 'validate' 'function' <validate-function-decl> ';'
 */
static
rc_t validate_declaration ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    switch ( t -> id )
    {
#if SLVL >= 3
    case kw_function:
        return valfunc_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
    default:
        break;
    }

    return KTokenExpected ( t, klogErr, "function" );
}


/*
 * include-stmt       = 'include' STRING ';'
 */
static
rc_t include_stmt ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    rc_t rc;

    if ( t -> id != eString )
        return KTokenExpected ( t, klogErr, "file path" );

    rc = VSchemaParseFile ( self, "%.*s", ( int ) ( t -> str . size - 2 ), t -> str . addr + 1 );
    if ( rc != 0 )
        return KTokenRCExplain ( t, klogErr, rc );

    return expect ( tbl, src, next_token ( tbl, src, t ), eSemiColon, ";", true );
}


/*
 * schema-decl        = <type-definition>
 *                    | <typeset-definition>
 *                    | <format-definition>
 *                    | <const-definition>
 *                    | <alias-definition>
 *                    | <extern-decl>
 *                    | <script-decl>
 *                    | <function-decl>
 *                    | <physical-decl>
 *                    | <table-decl>
 *                    | <database-decl>
 *                    | <include-stmt>
 *                    | ';'
 */
static
rc_t schema_decl ( KSymTable *tbl, KTokenSource *src, KToken *t,
    const SchemaEnv *env, VSchema *self )
{
    /* expect a keyword */
    switch ( t -> id )
    {
#if SLVL >= 1
    case kw_typedef:
        return type_definition ( tbl, src, next_token ( tbl, src, t ), env, self );
    case kw_typeset:
        return typeset_definition ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
    case kw_fmtdef:
        return format_definition ( tbl, src, next_token ( tbl, src, t ), env, self );
#if SLVL >= 2
    case kw_const:
        return const_definition ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
    case kw_alias:
        return alias_definition ( tbl, src, next_token ( tbl, src, t ), env, self );

    case kw_extern:
        return extern_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#if SLVL >= 3
    case kw_function:
        return function_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
#if SLVL >= 4
    case kw_schema:
        return script_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
#if SLVL >= 5
    case kw_physical:
        return physical_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
#if SLVL >= 6
    case kw_table:
        return table_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
    case kw_database:
        return database_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );
#endif
    case kw_include:
        return include_stmt ( tbl, src, next_token ( tbl, src, t ), env, self );

    case kw_validate:
        return validate_declaration ( tbl, src, next_token ( tbl, src, t ), env, self );

    case eSemiColon:
        /* TBD - issue warning if desired */
        next_token ( tbl, src, t );
        return 0;
    }

    /* it wasn't the right thing */
    return KTokenExpected ( t, klogErr,
        "include, typedef, typeset, fmtdef, function, schema, database or table" );
}

/*
 * schema-version     = 'version' <maj-min>
 */
#if SLVL >= 1
static
rc_t schema_version ( const KSymTable *tbl, KTokenSource *src, KToken *t, SchemaEnv *env )
{
    rc_t rc = maj_min_rel ( tbl, src, t, env, NULL, & env -> version, false );
    if ( rc == 0 )
        SchemaEnvInitFeatures ( env );
    return rc;
}
#endif


/* examine_schema_contents
 *  run a sanity check on schema contents
 */
#if EXAMINE_SCHEMA_CONTENTS >= 1
static
rc_t examine_schema_contents ( const VSchema *self )
{
    rc_t rc = 0;

    if ( self -> dad != NULL )
    {
        rc = examine_schema_contents ( self -> dad );
        if ( rc != 0 )
            return rc;
    }

    if ( BSTreeDoUntil ( & self -> scope, false, KSymbolExamineContents, & rc ) )
        return rc;

    return 0;
}
#endif

/*
 * schema             = [ <schema-version> ';' ] [ <schema-decl-list> ]
 *
 * schema-decl-list   = <schema-decl> [ <schema-decl-list> ]
 */
rc_t init_symtab ( KSymTable *tbl, const VSchema *self )
{
    rc_t rc;

    if ( self -> dad == NULL )
        return KSymTableInit ( tbl, ( BSTree* ) & self -> scope );

    rc = init_symtab ( tbl, self -> dad );
    if ( rc == 0 )
        rc = KSymTablePushScope ( tbl, ( BSTree* ) & self -> scope );

    return rc;
}

rc_t schema ( KTokenSource *src, VSchema *self )
{
    /* push schema scope onto symbol table */
    KSymTable tbl;
    rc_t rc = init_symtab ( & tbl, self );
    if ( rc == 0 )
    {
        KToken t;
        SchemaEnv env;

        /* prime token and look for version */
        next_token ( & tbl, src, & t );
#if SLVL >= 1
        /* initialize default to v1 */
        if ( t . id != kw_version )
            SchemaEnvInit ( & env, 0x01000000 );
        else
        {
            /* initialize explicit version */
            memset ( & env, 0, sizeof env );
            rc = schema_version ( & tbl, src, next_token ( & tbl, src, & t ), & env );
            if ( rc == 0 )
                rc = expect ( & tbl, src, & t, eSemiColon, ";", true );
        }
#endif
        if ( rc == 0 && t . id != eEndOfInput )
        {
            /* implement optional schema-decl-list */
            while ( rc == 0 && t . id != eEndOfInput )
            {
#if EXAMINE_SCHEMA_CONTENTS >= 1
                rc = examine_schema_contents ( self );
                if ( rc == 0 )
#endif
                rc = schema_decl ( & tbl, src, & t, & env, self );
            }
        }

        KSymTableWhack ( & tbl );
    }

    return rc;
}
