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

#include <kfc/extern.h>
#include <kfc/rsrc.h>
#include <kfc/rsrc-global.h>
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/rc.h>
#include <kfc/xc.h>
#include <kproc/procmgr.h>
#include <atomic32.h>

#include "rsrc-priv.h"

#include <WINDOWS.H>
#include <string.h>
#include <assert.h>


static KRsrc s_rsrc;
static CRITICAL_SECTION crit;
static atomic32_t s_initialized, crit_once;

static
void win_thread_once ( atomic32_t * once, void ( * trigger ) ( void ) )
{
    switch ( atomic32_test_and_set ( once, 1, 0 ) )
    {
    case 0:
        ( * trigger ) ();
        atomic32_set ( once, 2 );
        break;
    case 1:
        while ( atomic32_read ( once ) == 1 )
            Sleep ( 1 );
        break;
    case 2:
        break;
    }
}

static
void init_crit ( void )
{
    InitializeCriticalSection ( & crit );
}

static
void enter_critical_section ( void )
{
    /* Windows didn't come up with the concept of
       multi-thread-safe critical sections until Vista */
    win_thread_once ( & crit_once, init_crit );

    EnterCriticalSection ( & crit );
}

static
void leave_critical_section ( void )
{
    LeaveCriticalSection ( & crit );
}

static
void atexit_task ( void )
{
    if ( atomic32_read ( & s_initialized ) == 2 )
    {
        KCtx local_ctx;
        DECLARE_FUNC_LOC ( rcRuntime, rcMgr, rcDestroying );
        memset ( & local_ctx, 0, sizeof local_ctx );
        local_ctx . rsrc = & s_rsrc;
        local_ctx . loc = & s_func_loc;

        KRsrcGlobalWhack ( & local_ctx );
    }
}

/* Global
 *  retrieve process-global singleton KRsrc block
 *  creates and initializes block on initial request
 *  caches pointer for subsequent requests
 */
void KRsrcGlobalInit ( KCtx * ctx, const KFuncLoc * loc, bool full )
{
    int initialized;

    assert ( ctx != NULL );
    assert ( loc != NULL );

    /* initialize caller's top-level ctx */
    memset ( ctx, 0, sizeof * ctx );
    ctx -> rsrc = & s_rsrc;
    ctx -> loc = loc;

    /* singleton initialization */
    initialized = atomic32_read ( & s_initialized );
    if ( 0 <= initialized && initialized < 2 )
    {
        /* acquire lock */
        enter_critical_section ();
    
        initialized = atomic32_read ( & s_initialized );
        if ( 0 <= initialized && initialized < 2 )
        {
            KCtx local_ctx;
            DECLARE_FUNC_LOC ( rcRuntime, rcMgr, rcConstructing );

            /* link a new local context */
            ctx_init ( & local_ctx, ( ctx_t* ) & ctx, & s_func_loc );

            /* common initializer- errors are
               propagated to and handled by caller */
            rsrc_init ( & s_rsrc, ctx, full );

            /* mark the level of initialization */
            atomic32_set ( & s_initialized, full ? 2 : 1 );

            if ( full )
            {
                /* register global cleanup */
                int status = atexit ( atexit_task );
                if ( status != 0 )
                    SYSTEM_ERROR ( xcUnexpected, "atexit failed: %!", status );
            }
        }

        /* release lock */
        leave_critical_section ();
    }
}


/* Whack
 *  the global KRsrc may be referenced by local KRsrc
 */
void KRsrcGlobalWhack ( ctx_t ctx )
{
    if ( atomic32_read ( & s_initialized ) > 0 )
    {
        /* acquire lock, but ignore status */
        enter_critical_section ();
        if ( atomic32_read ( & s_initialized ) > 0 )
        {
            /* early whack of KProcMgr */
            KProcMgrRelease ( s_rsrc . proc );
            s_rsrc . proc = NULL;

            /* run destructor tasks while we still have other mgrs */
            KProcMgrWhack ();

            /* tear it down */
            KRsrcWhack ( & s_rsrc, ctx );

            /* mark as torn down */
            atomic32_set ( & s_initialized, -1 );
        }

        /* release lock */
        leave_critical_section ();
    }
}
