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

#include <klib/symtab.h>
#include <klib/rc.h>
#include <klib/symbol.h>

#include "schema-priv.h"
#include "schema-parse.h"

/* Cmp
 * Sort
 */
int64_t
SViewOverridesCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SViewOverrides *b = n;

    return ( int64_t ) * a - ( int64_t ) b -> ctx;
}

static
int64_t
SViewOverridesSort ( const void *item, const void *n )
{
    const SViewOverrides *a = item;
    const SViewOverrides *b = n;

    return ( int64_t ) a -> ctx - ( int64_t ) b -> ctx;
}

/* Whack
 */
static
void
SViewOverridesWhack ( void *item, void *ignore )
{
    SViewOverrides *self = item;
    VectorWhack ( & self -> by_parent, NULL, NULL );
    free ( self );
}

/* Make
 */
rc_t
SViewOverridesMake ( Vector *parents, const SView *dad, const Vector *overrides )
{
    rc_t rc;
    SViewOverrides *to;

    /* first question is whether parent exists */
    if ( VectorFind ( parents, & dad -> id, NULL, SViewOverridesCmp ) != NULL )
        return SILENT_RC ( rcVDB, rcSchema, rcParsing, rcTable, rcExists );

    /* create a new override object */
    to = malloc ( sizeof * to );
    if ( to == NULL )
        return RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted );

    /* shallow clone */
    rc = VectorCopy ( overrides, & to -> by_parent );
    if ( rc != 0 )
    {
        free ( to );
        return rc;
    }
    to -> dad = dad;
    to -> ctx = dad -> id;

    /* insert into parent override vector */
    rc = VectorInsert ( parents, to, NULL, SViewOverridesSort );
    if ( rc != 0 )
    {
        SViewOverridesWhack ( to, NULL );
        return rc;
    }

    return 0;
}

/* SView
*/
void
SViewWhack ( void * item, void *ignore )
{
    SView *self = item;

    /* whack components */
    VectorWhack ( & self -> params, NULL, NULL );
    VectorWhack ( & self -> parents, SViewInstanceWhack, NULL );

    VectorWhack ( & self -> col, SColumnWhack, NULL );
    VectorWhack ( & self -> cname, SNameOverloadWhack, NULL );

    VectorWhack ( & self -> prod, SProductionWhack, NULL );
    VectorWhack ( & self -> vprods, NULL, NULL );
    VectorWhack ( & self -> syms, ( void ( CC * ) ( void*, void* ) ) KSymbolWhack, NULL );

    VectorWhack ( & self -> overrides, SViewOverridesWhack, NULL );

    BSTreeWhack ( & self -> scope, KSymbolWhack, NULL );

    free ( self );
}

/* Cmp
 * Sort
 */
int64_t
SViewCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SView *b = n;

    if ( * a > b -> version )
        return 1;
    return ( int64_t ) ( * a >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

int64_t
SViewSort ( const void *item, const void *n )
{
    const SView *a = item;
    const SView *b = n;

    return ( int64_t ) ( a -> version >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

/*
 * push-view-scope
 * pop-view-scope
 */
void
pop_view_scope ( struct KSymTable * tbl, const SView * view )
{
    uint32_t i, count = VectorLength ( & view -> overrides );
    for ( ++ count, i = 0; i < count; ++ i )
        KSymTablePopScope ( tbl );
}

rc_t
push_view_scope ( struct KSymTable * tbl, const SView * view )
{
    rc_t rc;
    uint32_t i = VectorStart ( & view -> overrides );
    uint32_t count = VectorLength ( & view -> overrides );
    for ( count += i; i < count; ++ i )
    {
        const SViewOverrides *to = ( const void* ) VectorGet ( & view -> overrides, i );
        rc = KSymTablePushScope ( tbl, ( BSTree* ) & to -> dad -> scope );
        if ( rc != 0 )
        {
            for ( count = VectorStart ( & view -> overrides ); i > count; -- i )
                KSymTablePopScope ( tbl );
            return rc;
        }
    }

    rc = KSymTablePushScope ( tbl, ( BSTree* ) & view -> scope );
    if ( rc != 0 )
    {
        for ( i = VectorStart ( & view -> overrides ); i < count; ++ i )
            KSymTablePopScope ( tbl );
    }

    return rc;
}

bool
view_fwd_scan ( BSTNode *n, void *data )
{
    SViewScanData *pb = data;
    KSymbol *sym = ( KSymbol* ) n;
    SView *self = pb -> self;

    /* process forwarded symbols */
    if ( sym -> type == eForward )
    {
        /* this symbol was introduced in THIS view */
        sym -> u . fwd . ctx = self -> id;

        /* add it to the introduced virtual productions and make it virtual */
        pb -> rc = VectorAppend ( & self -> vprods, & sym -> u . fwd . id, sym );
        if ( pb -> rc != 0 )
            return true;
        sym -> type = eVirtual;
    }
    /* symbols other than fwd or virtual are ignored */
    else if ( sym -> type != eVirtual )
    {
        return false;
    }

    /* add symbol to vector to track ownership */
    pb -> rc = VectorAppend ( & self -> syms, NULL, sym );
    if ( pb -> rc != 0 )
        return true;

    /* remove from symbol table */
    BSTreeUnlink ( & self -> scope, & sym -> n );
    return false;
}

static
void column_set_context ( void *item, void *data )
{
    SColumn *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void name_set_context ( void *item, void *data )
{
    SNameOverload *self = item;
    if ( ( int32_t ) self -> cid . ctx < 0 )
        self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void production_set_context ( void *item, void *data )
{
    SProduction *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void symbol_set_context ( void *item, void *data )
{
    KSymbol *self = item;
    self -> u . fwd . ctx = * ( const uint32_t* ) data;
}

void CC view_set_context ( SView *self, uint32_t p_ctxId )
{
    VectorForEach ( & self -> col, false, column_set_context, & p_ctxId );
    VectorForEach ( & self -> cname, false, name_set_context, & p_ctxId );
    VectorForEach ( & self -> prod, false, production_set_context, & p_ctxId );
    VectorForEach ( & self -> vprods, false, symbol_set_context, & p_ctxId );
}

static
bool view_prod_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    const SProduction *prod = ( const SProduction* ) item;

    * rc = eval_expr_syntax ( prod -> fd );
    if ( * rc == 0 )
        * rc = eval_expr_syntax ( prod -> expr );

    return ( * rc != 0 ) ? true : false;
}

static
rc_t view_stmt_syntax ( const SView * view )
{
    rc_t rc = 0;
    VectorDoUntil ( & view -> prod, false, view_prod_syntax, & rc );
    return rc;
}

static
bool view_typed_column_syntax ( void *item, void *data )
{
    rc_t *rc = data;
    const SColumn *col = ( const SColumn* ) item;

    if ( col -> read != NULL )
        * rc = eval_expr_syntax ( col -> read );
    if ( * rc == 0 && col -> validate != NULL )
        * rc = eval_expr_syntax ( col -> validate );
    if ( * rc == 0 && col -> limit != NULL )
        * rc = eval_expr_syntax ( col -> limit );

    return ( * rc != 0 ) ? true : false;
}

static
rc_t view_column_syntax ( const SView *view )
{
    rc_t rc = 0;
    VectorDoUntil ( & view -> col, false, view_typed_column_syntax, & rc );
    return rc;
}

rc_t
view_fix_forward_refs ( const SView * view )
{
    rc_t rc = view_stmt_syntax ( view );
    if ( rc == 0 )
        rc = view_column_syntax ( view );
    return rc;
}

void
CC SViewInstanceWhack ( void *item, void *ignore )
{
    SViewInstance * self = ( SViewInstance * ) item;
    VectorWhack ( & self -> params, NULL, NULL );
    free ( self );
}


/*
 * init-tbl-symtab
 *  initializes "tbl"
 *  places table in scope
 *  must be balanced by KSymTableWhack
 */
static
rc_t
init_view_symtab ( KSymTable * p_tbl, const VSchema * p_schema, const SView * p_view )
{
    rc_t rc = init_symtab ( p_tbl, p_schema );
    if ( rc == 0 )
    {
        rc = push_view_scope ( p_tbl, p_view );
        if ( rc == 0 )
        {
            return 0;
        }

        KSymTableWhack ( p_tbl );
    }

    return rc;
}


/* Find
 *  generic object find within view scope
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
const void *
SViewFind ( const SView *           p_self,
            const VSchema *         p_schema,
            VTypedecl *             p_td,
            const SNameOverload **  p_name,
            uint32_t *              p_type,
            const char *            p_expr,
            const char *            p_ctx,
            bool                    p_dflt )
{
    rc_t rc;
    KSymTable tbl;

    /* initialize to not-found */
    const void *obj = NULL;
    * p_name = NULL;
    * p_type = 0;

    /* build a symbol table for view */
    rc = init_view_symtab ( & tbl, p_schema, p_self );
    if ( rc == 0 )
    {
        obj = resolve_object ( & tbl, p_schema, p_td, p_name, p_type, p_expr, p_ctx, p_dflt );
        KSymTableWhack ( & tbl );
    }

    return obj;
}

/* FindOverride
 *  finds an inherited or introduced overridden symbol
 */
const struct KSymbol *
SViewFindOverride ( const SView * p_self, const VCtxId * p_cid )
{
    const SViewOverrides *to;

    /* it may be on the existing view */
    if ( p_cid -> ctx == p_self -> id )
    {
        return VectorGet ( & p_self -> vprods, p_cid -> id );
    }

    to = ( const void* ) VectorFind ( & p_self -> overrides, & p_cid -> ctx, NULL, SViewOverridesCmp );
    if ( to == NULL )
    {
        return NULL;
    }

    return VectorGet ( & to -> by_parent, p_cid -> id );
}
