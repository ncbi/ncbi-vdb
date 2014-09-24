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

#include <atomic32.h>

#include "rsrc-priv.h"
#include "sysctx-priv.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>

/* thread-local storage */
typedef struct TLS TLS;
struct TLS
{
    KRsrc rsrc;
    KCtx ctx;
};

static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;


/* whack
 *  whack thread-local storage
 */
static
void whack_tls ( void * data )
{
    /* the thread-local storage */
    TLS * tls = data;

    /* the "parent" ctx */
    ctx_t ctx = & tls -> ctx;
    FUNC_ENTRY ( ctx, rcRuntime, rcThread, rcDestroying );

    /* whack the resource managers */
    KRsrcWhack ( & tls -> rsrc, ctx );

    /* free the memory */
    free ( tls );
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
        exit ( ENOMEM );

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
            pthread_setspecific ( key, tls );
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


/* make_key
 *  initialize the thread-local storage key
 */
static
void make_key ( void )
{
    pthread_key_create ( & key, whack_tls );
}


/* get_tls_ctx
 *  reads thread-local storage
 */
static
const KCtx * get_tls_ctx ( const KFuncLoc * func_loc )
{
    TLS * tls;

    if ( KProcMgrOnMainThread () )
    {
        static KCtx s_ctx;
        static bool been_here;
        ctx_t ctx = & s_ctx;

        if ( been_here )
            return & s_ctx;

        TRY ( KRsrcGlobalInit ( & s_ctx, func_loc, true ) )
        {
            /* done */
            been_here = true;
            return & s_ctx;
        }

        /* terrible failure */
        exit ( -1 );
    }

    /* ensure the key is there */
    pthread_once ( & key_once, make_key );

    /* retrieve the existing tls */
    tls = pthread_getspecific ( key );

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
