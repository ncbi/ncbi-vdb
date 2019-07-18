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

typedef struct KVTableCacheCleanup KVTableCacheCleanup;
#define KTASK_IMPL KVTableCacheCleanup

#include <sysalloc.h>

#include <kfc/extern.h>
#include <kfc/vtbl-impl.h>
#include <kfc/except.h>
#include <kfc/ctx.h>
#include <kfc/xc.h>

#include <kproc/impl.h>
#include <atomic.h>


/* KVTableCacheCleanup
 */
struct KVTableCacheCleanup
{
    /* THIS WILL BREAK WHEN TASK GETS CONVERTED
       SHOULD BE EXPLICITLY KTask_v1
       NB - don't use KTask_v2, because that will
       have a vtable being cleaned up */
    KTask dad;

    /* all vtable caches are stored in a singly-linked list */
    atomic_ptr_t list;
};

/* Whack
 *  
 */
static
rc_t CC KVTableCacheCleanupWhack ( KVTableCacheCleanup * self )
{
    KTaskDestroy ( & self -> dad, "KVTableCacheCleanup" );
    free ( self );
    return 0;
}

/* Execute
 *  perform deferred operation
 */
static
rc_t CC KVTableCacheCleanupExecute ( KVTableCacheCleanup * self )
{
    KHierCache * test, * read = self -> list . ptr;
    if ( read != NULL ) do
        read = atomic_test_and_set_ptr ( & self -> list, NULL, test = read );
    while ( read != test );

    while ( read != NULL )
    {
        test = ( void* ) read -> next;
        /* may insert a debug statement here */
        free ( read );
        read = test;
    }

    return 0;
}

static
KTask_vt_v1 KVTableCacheCleanup_vt =
{
    1, 0,
    KVTableCacheCleanupWhack,
    KVTableCacheCleanupExecute
};

static
void KVTableAddHierCacheToCleanupTask ( KHierCache * cache, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcRuntime, rcQueue, rcInserting );

    KHierCache * test, * read;
    KVTableCacheCleanup * local_task;

    static atomic_ptr_t cleanup_task;
    if ( cleanup_task . ptr == NULL )
    {
        rc_t rc;
        local_task = malloc ( sizeof * local_task );
        if ( local_task == NULL )
        {
            SYSTEM_ERROR ( xcNoMemory, "allocating %zu bytes", sizeof * local_task );
            return;
        }

        rc = KTaskInit ( & local_task -> dad,
            ( const KTask_vt* ) & KVTableCacheCleanup_vt, "KVTableCacheCleanup", "" );
        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcUnexpected, "%R", rc );
            ( ( KCtx* ) ctx ) -> rc = rc;
            free ( local_task );
            return;
        }

        cache -> next = NULL;
        local_task -> list . ptr = cache;
        if ( atomic_test_and_set_ptr ( & cleanup_task, local_task, NULL ) == NULL )
            return;

        KVTableCacheCleanupWhack ( local_task );
    }

    local_task = cleanup_task . ptr;
    read = local_task -> list . ptr;
    do
    {
        cache -> next = test = read;
        read = atomic_test_and_set_ptr ( & local_task -> list, cache, test );
    }
    while ( read != test );
}


/* Depth
 */
static
uint32_t KVTableDepth ( KVTable * self /*, ctx_t ctx */ )
{
    uint32_t idx;

    assert ( self -> itf != NULL );
    idx = self -> itf -> idx;

    if ( idx == 0 )
    {
        KItfTok * itf = ( KItfTok * ) self -> itf;
        if ( self -> parent != NULL )
            idx = KVTableDepth ( ( KVTable* ) self -> parent /*, ctx */ ) + 1;
        else
            idx = 1;
        itf -> idx = idx;
    }

    return idx;
}


/* Resolve
 *  walks a vtable's hierarchy
 *  builds a linear array cache of casting pointers
 *  dynamically configures KItfTok objects to cache their runtime offset
 */
void KVTableResolve ( const KVTable * cself, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcRuntime, rcTable, rcResolving );

    KVTable * self = ( KVTable* ) cself;
    if ( self != NULL && self -> cache == NULL )
    {
        uint32_t idx = KVTableDepth ( self /*, ctx */ );

        KHierCache * cache;
        size_t bytes = sizeof * cache - sizeof cache -> parent + idx * sizeof cache -> parent [ 0 ];

        cache = malloc ( bytes );
        if ( cache == NULL )
            SYSTEM_ERROR ( xcNoMemory, "allocating %zu bytes", bytes );
        else
        {
            cache -> next = NULL;
            cache -> length = idx;

            TRY ( KVTableAddHierCacheToCleanupTask ( cache, ctx ) )
            {
                self -> cache = cache;

                do
                {
                    assert ( idx != 0 );
                    cache -> parent [ -- idx ] = self;
                    self = ( KVTable* ) self -> parent;
                }
                while ( self != NULL );
            }
        }
    }
}
