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
#include <kfc/ctx.h>
#include <kfc/except.h>
#include <kfc/rc.h>
#include <kfc/xc.h>

#include <kfc/tstate.h>
#include <kproc/procmgr.h>
#include <kproc/thread.h>
#include <kfg/config.h>
#include <kns/manager.h>
#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <kdb/manager.h>
#include <vdb/manager.h>
#include <vdb/vdb-priv.h>

#include "rsrc-priv.h"

#include <string.h>
#include <assert.h>


/* Init
 *  initialize a local block from another
 */
LIB_EXPORT void CC KRsrcInit ( KRsrc * rsrc, ctx_t ctx )
{
    FUNC_ENTRY ( ctx, rcRuntime, rcMgr, rcAttaching );

    if ( rsrc == NULL )
        INTERNAL_ERROR ( xcParamNull, "bad KRsrc block" );
    else
    {
        rc_t rc;
        const KRsrc * src = ctx -> rsrc;
        memset ( rsrc, 0, sizeof * rsrc );

        /* attach new reference to each manager */
        rc = KProcMgrAddRef ( rsrc -> proc = src -> proc );
        if ( rc == 0 )
            rsrc -> thread = KProcMgrMakeThreadState ( rsrc -> proc );
#if 0
        if ( rc == 0 )
        {
            rsrc -> mem = KMemMgrDuplicate ( src -> mem, ctx );
            rc = ctx -> rc;
        }
#endif
        if ( rc == 0 )
            rc = KConfigAddRef ( rsrc -> cfg = src -> cfg );
        if ( rc == 0 )
            rc = KNSManagerAddRef ( rsrc -> kns = src -> kns );
        if ( rc == 0 )
            rc = VFSManagerAddRef ( rsrc -> vfs = src -> vfs );
        if ( rc == 0 )
            rc = KDBManagerAddRef ( rsrc -> kdb = src -> kdb );
        if ( rc == 0 )
            rc = VDBManagerAddRef ( rsrc -> vdb = src -> vdb );

        if ( rc != 0 )
        {
            INTERNAL_ERROR ( xcRefcountOutOfBounds, "exceeded references to a KRsrc manager" );
            KRsrcWhack ( rsrc, ctx );
        }
    }
}


/* Whack
 *  release references
 */
LIB_EXPORT void CC KRsrcWhack ( KRsrc * self, ctx_t ctx )
{
    if ( self != NULL )
    {
        VDBManagerRelease ( self -> vdb );
        KDBManagerRelease ( self -> kdb );
        VFSManagerRelease ( self -> vfs );
        KNSManagerRelease ( self -> kns );
        KConfigRelease ( self -> cfg );
#if 0
        /* remember to deal with case of ctx == NULL,
           which is allowed for VDB.2 in this context */
        KMemMgrRelease ( self -> mem, ctx );
#endif
        KThreadStateWhack ( self -> thread );
        KProcMgrRelease ( self -> proc );

        memset ( self, 0, sizeof * self );
    }
}


/* rsrc_init
 *  initialize a resource block with common managers
 *  lazy generation from thread entry
 *  not called when used under KMain
 */
void rsrc_init ( KRsrc * rsrc, ctx_t ctx, bool full )
{
    if ( rsrc == NULL )
        INTERNAL_ERROR ( xcParamNull, "Bad KRsrc block" );
    else
    {
        rc_t rc;
#if 0
        memset ( rsrc, 0, sizeof * rsrc );
#endif

        /* initialize the proc mgr */
        rc = KProcMgrInit ();
        if ( rc == 0 && rsrc -> proc == NULL )
            rc = KProcMgrMakeSingleton ( & rsrc -> proc );

        /* create KMemMgr */

        if ( full )
        {
            /* create current thread */
            if ( rc == 0 )
                rsrc -> thread = KProcMgrMakeThreadState ( rsrc -> proc );

            /* read in KConfig */
            if ( rc == 0 )
                rc = KConfigMake ( & rsrc -> cfg, NULL );
#if 0
            /* not making the KNS manager, because the
               VFS manager makes it, and we can get his */
            if ( rc == 0 )
                rc = KNSManagerMake ( & rsrc -> kns );
#endif
            /* create VFS manager, linked to our KConfig */
            if ( rc == 0 )
                rc = VFSManagerMakeFromKfg ( & rsrc -> vfs, rsrc -> cfg );
#if 1
            /* extract the VFS manager's KNS manager */
            if ( rc == 0 )
                rc = VFSManagerGetKNSMgr ( rsrc -> vfs, & rsrc -> kns );
#endif
            /* create a VDB manager, which creates a KDB manager
               call special library agnostic function, link to vfs */
            if ( rc == 0 )
                rc = VDBManagerMakeRsrc ( & rsrc -> vdb, rsrc -> vfs );
            /* obtain the read version of KDB manager */
            if ( rc == 0 )
                rc = VDBManagerOpenKDBManagerRead ( rsrc -> vdb, ( const KDBManager** ) & rsrc -> kdb );
        }

        if ( rc != 0 )
            INTERNAL_ERROR ( xcUnexpected, "failed to create a KRsrc manager ( rc = %R )", rc );
    }
}
