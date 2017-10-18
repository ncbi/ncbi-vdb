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

/* SViewOverrides
*/
typedef struct SViewOverrides SViewOverrides;
struct SViewOverrides
{
    const SView *dad;
    Vector overrides;
    uint32_t ctx;
};

/* Cmp
 * Sort
 */
static
int64_t CC SViewOverridesCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SViewOverrides *b = n;

    return ( int64_t ) * a - ( int64_t ) b -> ctx;
}

static
int64_t CC SViewOverridesSort ( const void *item, const void *n )
{
    const SViewOverrides *a = item;
    const SViewOverrides *b = n;

    return ( int64_t ) a -> ctx - ( int64_t ) b -> ctx;
}

/* Whack
 */
static
void CC SViewOverridesWhack ( void *item, void *ignore )
{
    SViewOverrides *self = item;
    VectorWhack ( & self -> overrides, NULL, NULL );
    free ( self );
}

/* Make
 */
static
rc_t SViewOverridesMake ( Vector *parents, const SView *dad, const Vector *overrides )
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
    rc = VectorCopy ( overrides, & to -> overrides );
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

static
bool CC SViewOverridesClone ( void *item, void *data )
{
    const SViewOverrides *self = ( const void* ) item;
    rc_t rc = SViewOverridesMake ( data, self -> dad, & self -> overrides );
    return ( rc != 0 && GetRCState ( rc ) != rcExists ) ? true : false;
}

/* SView
*/
void CC SViewWhack ( void * item, void *ignore )
{
    SView *self = item;

    /* whack components */
    VectorWhack ( & self -> tables, NULL, NULL );
    VectorWhack ( & self -> views, NULL, NULL );
    VectorWhack ( & self -> parents, NULL, NULL );

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
int64_t CC SViewCmp ( const void *item, const void *n )
{
    const uint32_t *a = item;
    const SView *b = n;

    if ( * a > b -> version )
        return 1;
    return ( int64_t ) ( * a >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

int64_t CC SViewSort ( const void *item, const void *n )
{
    const SView *a = item;
    const SView *b = n;

    return ( int64_t ) ( a -> version >> 24 ) - ( int64_t ) ( b -> version >> 24 );
}

static
bool
SViewHasDad ( void *item, void *data )
{
    return item == data;
}

/*
 * push-view-scope
 * pop-view-scope
 */
void
CC
pop_view_scope ( struct KSymTable * tbl, const SView * view )
{
    uint32_t i, count = VectorLength ( & view -> overrides );
    for ( ++ count, i = 0; i < count; ++ i )
        KSymTablePopScope ( tbl );
}

rc_t
CC
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

static
bool SViewTestForSymCollision ( const KSymbol *sym, void *data )
{
    const KSymTable *tbl = ( const void* ) data;
    const KSymbol *found = KSymTableFindSymbol ( tbl, sym );
    if ( found != NULL && found != sym ) switch ( found -> type )
    {
    case eColumn:
        if ( sym -> type == eColumn )
        {
            /* when colliding columns originated in the same
               view, consider them to be compatible extensions */
            const SNameOverload *found_col, *sym_col;
            sym_col = sym -> u . obj;
            found_col = found -> u . obj;
            assert ( sym_col != NULL && found_col != NULL );
            if ( sym_col -> cid . ctx == found_col -> cid . ctx )
                return SOverloadTestForTypeCollision ( sym_col, found_col );
        }
    case eProduction:
        PLOGMSG ( klogErr, ( klogErr, "duplicate symbol '$(sym)' in parent view hierarchy"
                             , "sym=%S"
                             , & sym -> name
                      ));
        return true;
    }
    return false;
}

static
bool CC SViewTestColCollisions ( void *item, void *data )
{
    const SNameOverload *no = ( const void* ) item;
    return SViewTestForSymCollision ( no -> name, data );
}

static
bool CC SViewTestProdCollisions ( void *item, void *data )
{
    const SProduction *prod = ( const void* ) item;
    return SViewTestForSymCollision ( prod -> name, data );
}

static
bool SViewTestForCollisions ( void *item, void *data )
{
    const SView *self = ( const void* ) item;

    /* test column names */
    if ( VectorDoUntil ( & self -> cname, false, SViewTestColCollisions, data ) )
        return true;

    /* test production names */
    if ( VectorDoUntil ( & self -> prod, false, SViewTestProdCollisions, data ) )
        return true;

    return false;
}


static
bool CC SViewOverridesTestForCollisions ( void *item, void *data )
{
    const SViewOverrides *to = ( const void* ) item;
    return SViewTestForCollisions ( ( void* ) to -> dad, data );
}

static
bool CC SViewCopyColumnNames ( void *item, void *data )
{
    rc_t rc;
    SView *self= data;
    SNameOverload *copy;
    const SNameOverload *orig = ( const void* ) item;
    const KSymbol *sym = ( const KSymbol* )
        BSTreeFind ( & self -> scope, & orig -> name -> name, KSymbolCmp );
    if ( sym == NULL )
    {
        rc = SNameOverloadCopy ( & self -> scope, & copy, orig );
        if ( rc == 0 )
        {
            rc = VectorAppend ( & self -> cname, & copy -> cid . id, copy );
            if ( rc != 0 )
                SNameOverloadWhack ( copy, NULL );
        }
    }
    else
    {
        copy = ( void* ) sym -> u . obj;
        assert ( copy -> cid . ctx == orig -> cid . ctx );
        rc = VectorMerge ( & copy -> items, true, & orig -> items, SColumnSort );
    }

    return ( rc != 0 ) ? true : false;
}

bool CC view_fwd_scan ( BSTNode *n, void *data )
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
void CC column_set_context ( void *item, void *data )
{
    SColumn *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC name_set_context ( void *item, void *data )
{
    SNameOverload *self = item;
    if ( ( int32_t ) self -> cid . ctx < 0 )
        self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC production_set_context ( void *item, void *data )
{
    SProduction *self = item;
    self -> cid . ctx = * ( const uint32_t* ) data;
}

static
void CC symbol_set_context ( void *item, void *data )
{
    KSymbol *self = item;
    self -> u . fwd . ctx = * ( const uint32_t* ) data;
}

void CC view_set_context ( SView *self )
{
    VectorForEach ( & self -> col, false, column_set_context, & self -> id );
    VectorForEach ( & self -> cname, false, name_set_context, & self -> id );
    VectorForEach ( & self -> prod, false, production_set_context, & self -> id );
    VectorForEach ( & self -> vprods, false, symbol_set_context, & self -> id );
}

