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

#define TRACK_REFERENCES 0
/* should match dbmgr-cmn.c */

#include "libvdb.vers.h"

#define KONST const
#include "dbmgr-priv.h"
#undef KONST

#include "schema-priv.h"
#include "linker-priv.h"

#include <vdb/manager.h>
#include <vdb/schema.h>
#include <kdb/kdb-priv.h> /* KDBManagerMakeReadWithVFSManager */
#include <kdb/manager.h>
#include <kfs/directory.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * VDBManager
 *  opaque handle to library
 */


/* MakeRead
 * MakeReadWithVFSManager
 *  create library handle for specific use
 *  NB - only one of the functions will be implemented
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 */
LIB_EXPORT rc_t CC VDBManagerMakeRead ( const VDBManager **mgrp, const KDirectory *wd )
{
    return VDBManagerMakeReadWithVFSManager(mgrp, wd, NULL);
}

LIB_EXPORT rc_t CC VDBManagerMakeReadWithVFSManager ( const VDBManager ** mgrp,
    const KDirectory *wd, struct VFSManager *vmgr )
{
    rc_t rc;

    if ( mgrp == NULL )
        rc = RC ( rcVDB, rcMgr, rcConstructing, rcParam, rcNull );
    else
    {
        VDBManager *mgr = malloc ( sizeof * mgr );
        if ( mgr == NULL )
            rc = RC ( rcVDB, rcMgr, rcConstructing, rcMemory, rcExhausted );
        else
        {
            rc = KDBManagerMakeReadWithVFSManager ( & mgr -> kmgr, wd, vmgr );
            if ( rc == 0 )
            {
                rc = VSchemaMakeIntrinsic ( & mgr -> schema );
                if ( rc == 0 )
                {
                    rc = VLinkerMakeIntrinsic ( & mgr -> linker );
                    if ( rc == 0 )
                    {
                        rc = VDBManagerConfigPaths ( mgr, false );
                        if ( rc == 0 )
                        {
                            mgr -> user = NULL;
                            mgr -> user_whack = NULL;
                            KRefcountInit ( & mgr -> refcount, 1, "VDBManager", "make-read", "vmgr" );
                            * mgrp = mgr;
                            return 0;
                        }

                        VLinkerRelease ( mgr -> linker );
                    }

                    VSchemaRelease ( mgr -> schema );
                }

                KDBManagerRelease ( mgr -> kmgr );
            }

            free ( mgr );
        }

        * mgrp = NULL;
    }
    return rc;
}


/* MakeRsrc
 *  common make, regardless of library
 */
LIB_EXPORT rc_t CC VDBManagerMakeRsrc ( VDBManager ** mgr, struct VFSManager * vfs )
{
    return VDBManagerMakeReadWithVFSManager ( ( const VDBManager** ) mgr, NULL, vfs );
}


/* Version
 *  returns the library version
 */
LIB_EXPORT rc_t CC VDBManagerVersion ( const VDBManager *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcVDB, rcMgr, rcAccessing, rcParam, rcNull );
    if ( self == NULL )
        return RC ( rcVDB, rcMgr, rcAccessing, rcSelf, rcNull );

    * version = LIBVDB_VERS;
    return 0;
}


/* RunPeriodicTasks
 *  executes periodic tasks, such as cache flushing
 */
LIB_EXPORT rc_t CC VDBManagerRunPeriodicTasks ( const VDBManager *self )
{
    if ( self != NULL )
        return KDBManagerRunPeriodicTasks ( self -> kmgr );

    return RC ( rcVDB, rcMgr, rcExecuting, rcSelf, rcNull );
}
