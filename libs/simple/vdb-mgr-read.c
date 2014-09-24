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

#include <simple/extern.h>
#include "sra-exception.h"
#include "vdb-mgr.h"

#include <vdb/manager.h>
#include <atomic.h>

#include <assert.h>


/*--------------------------------------------------------------------------
 * VDBMgr
 *  basically a front to VDBManager, but
 *  may also include certain caches for supporting Java
 */

/* singleton... we all love singletons */
static void *volatile vdb = NULL;

/* ForRead
 *  makes VDBMgr for read-only library
 */
const VDBMgr *VDBMgrMakeRead ( SRAException *x )
{
    const VDBManager *mgr = vdb;

    assert ( x != NULL );

    /* this guy will go from NULL to non-NULL once,
       and be reused from then on */
    if ( mgr == NULL )
    {
        const VDBManager *new_mgr;
        rc_t rc = VDBManagerMakeRead ( & new_mgr, NULL );
        if ( rc != 0 )
        {
            SRAExceptionError ( x, 0, rc, "failed to create VDB manager" );
            return NULL;
        }

        /* set the singleton */
        mgr = atomic_test_and_set_ptr ( & vdb, ( void* ) new_mgr, NULL );
        if ( mgr != NULL )
        {
            assert ( vdb == mgr );
            assert ( new_mgr != mgr );
            VDBManagerRelease ( new_mgr );
        }
        else
        {
            mgr = new_mgr;
        }
    }

    return mgr;
}

/* Whack
 *  closes down anything that was started up
 */
void VDBMgrWhack ( void )
{
    const VDBMgr *mgr = atomic_test_and_set_ptr ( & vdb, NULL, vdb );
    if ( mgr != NULL )
        VDBManagerRelease ( mgr );
}
