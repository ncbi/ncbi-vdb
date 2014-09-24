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

#include <kfc/ctx.h>
#include <kfc/rsrc.h>
#include <kfc/rsrc-global.h>
#include <kfc/except.h>
#include <kfc/rc.h>
#include <kfc/xc.h>
#include <kproc/procmgr.h>
#include <kproc/task.h>
#include <kproc/impl.h>
#include <klib/container.h>

#include <sysalloc.h>

#include <atomic32.h>

#include "rsrc-priv.h"
#include "sysctx-priv.h"

#include <WINDOWS.H>
#include <string.h>
#include <assert.h>

/* thread-local storage */
typedef struct TLS TLS;
struct TLS
{
    DLNode n;
    KRsrc rsrc;
    KCtx ctx;
    DWORD threadId;
};

static DWORD key;
static DLList s_tls;
static uint32_t s_tls_count;
static CRITICAL_SECTION crit;
static atomic32_t key_once, crit_once;


/* whack
 *  whack thread-local storage
 */
static
void CC whack_tls ( DLNode * n, void * ignore )
{
    TLS * tls = ( TLS * ) n;

    /* the "parent" ctx */
    ctx_t ctx = & tls -> ctx;
    FUNC_ENTRY ( ctx, rcRuntime, rcThread, rcDestroying );

    /* whack the resource managers */
    KRsrcWhack ( & tls -> rsrc, ctx );

    /* free the memory */
    free ( tls );

    /* reduce the count */
    -- s_tls_count;
}

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
TLS * win_thread_getspecific ( DWORD tlsKey )
{
    return TlsGetValue ( tlsKey );
}

static
bool CC eliminate_dups ( DLNode * n, void * data )
{
    const TLS * tls = ( const void * ) data;
    if ( ( ( const TLS * ) n ) -> threadId != tls -> threadId )
        return false;

    DLListUnlink ( & s_tls, n );
    whack_tls ( n, NULL );
    return true;
}

static
void CC garbage_collect_tls ( DLNode * n, void * ignore )
{
    TLS * tls = ( TLS* ) n;
    HANDLE t = OpenThread ( THREAD_QUERY_INFORMATION, FALSE, tls -> threadId );
    if ( t != NULL )
        CloseHandle ( t );
    else
    {
        DLListUnlink ( & s_tls, & tls -> n );
        whack_tls ( & tls -> n, NULL );
    }
}

static
void win_thread_setspecific ( DWORD tlsKey, TLS * tls )
{
    /* record the thread id */
    tls -> threadId = GetCurrentThreadId ();

    /* need to record these for cleanup function */
    enter_critical_section ();

    /* quickly scan for reused thread id */
    DLListDoUntil ( & s_tls, false, eliminate_dups, tls );

    /* push the entry */
    DLListPushTail ( & s_tls, & tls -> n );

    /* try to garbage collect for threads
       that have already exited */
    if ( ++ s_tls_count >= 64 )
        DLListForEach ( & s_tls, false, garbage_collect_tls, NULL );

    /* done with the list */
    leave_critical_section ();

    /* store it on the thread */
    TlsSetValue ( tlsKey, tls );
}


/* make_tls
 *  create thread-local storage
 */
static
TLS * make_tls ( const KFuncLoc * func_loc )
{
    ctx_t ctx;

    /* create the storage */
    TLS * tls = calloc ( 1, sizeof * tls );
    if ( tls == NULL )
        exit ( -1 );

    /* grab its context */
    ctx = & tls -> ctx;

    /* recover process-global resources */
    TRY ( KRsrcGlobalInit ( & tls -> ctx, func_loc, true ) )
    {
        /* attach references */
        TRY ( KRsrcInit ( & tls -> rsrc, ctx ) )
        {
            /* reset context */
            tls -> ctx . rsrc = & tls -> rsrc;

            /* set on thread */
            win_thread_setspecific ( key, tls );
            assert ( ! FAILED () );
        }
    }

    if ( FAILED () )
    {
        free ( tls );
        exit ( -1 );
    }

    return tls;
}

static
rc_t CC TLSCleanupTaskWhack ( KTask * self )
{
    KTaskDestroy ( self, "TLSCleanupTask" );
    free ( self );
    return 0;
}

static
rc_t CC TLSCleanupTaskExecute ( KTask * self )
{
    DLListWhack ( & s_tls, whack_tls, NULL );
    return 0;
}

static KTask_vt_v1 TLSCleanupTask_vt =
{
    1, 0,
    TLSCleanupTaskWhack,
    TLSCleanupTaskExecute
};

static
void install_cleanup_task ( void )
{
    rc_t rc = KProcMgrInit ();
    if ( rc == 0 )
    {
        KProcMgr * mgr;
        rc = KProcMgrMakeSingleton ( & mgr );
        if ( rc == 0 )
        {
            /* create task to install into procmgr */
            KTask * task = malloc ( sizeof * task );
            if ( task != NULL )
            {
                rc = KTaskInit ( task,
                    ( const KTask_vt * ) & TLSCleanupTask_vt,
                    "TLSCleanupTask", "" );
                if ( rc != 0 )
                    free ( task );
                else
                {
                    KTaskTicket ticket;
                    rc = KProcMgrAddCleanupTask ( mgr, & ticket, task );
                    if ( rc != 0 )
                        KTaskRelease ( task );
                }
            }

            KProcMgrRelease ( mgr );
        }
    }
}

/* make_key
 *  initialize the thread-local storage key
 */
static
void make_key ( void )
{
    /* install cleanup task -
       Windows doesn't appear to support
       libraries that create TLS but
       don't own the thread main */
    install_cleanup_task ();
                        
    /* create key into thread-local storage */
    key = TlsAlloc ();
}


/* get_tls_ctx
 *  reads thread-local storage
 */
static
const KCtx * get_tls_ctx ( const KFuncLoc * func_loc )
{
    TLS * tls;

    /* ensure the key is there */
    win_thread_once ( & key_once, make_key );

    /* retrieve the existing tls */
    tls = win_thread_getspecific ( key );

    /* create a new one if missing */
    if ( tls == NULL )
    {
        tls = make_tls ( func_loc );
        assert ( tls != NULL );
    }

    return & tls -> ctx;
}


/* ctx_recover
 *  queries thread for previously stored KRsrc block
 *  creates a new one if necessary
 */
ctx_t ctx_recover ( KCtx * new_ctx, const KFuncLoc * func_loc )
{
    DECLARE_FUNC_LOC ( rcRuntime, rcMgr, rcOpening );

    if ( new_ctx != NULL )
    {
        const KCtx * ctx = get_tls_ctx ( & s_func_loc );

        /* clear new_ctx and initialize special members */
        RESET_CTX(new_ctx, ctx, func_loc);
    }

    return new_ctx;
}
