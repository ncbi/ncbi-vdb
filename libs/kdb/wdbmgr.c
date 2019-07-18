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

#define TRACK_REFERENCES 0

#include <kdb/extern.h>
#include "libkdb.vers.h"
#include "dbmgr-priv.h"
#include "wkdb-priv.h"
#include <kfs/impl.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h> /* VFSManagerOpenDirectoryReadDecryptUnreliable */
#include <vfs/resolver.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>

#include <klib/symbol.h>
#include <klib/checksum.h>
#include <klib/rc.h>

#include <sysalloc.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * KDBManager
 *  handle to library
 */


/* MakeUpdate
 *  create library handle
 */
LIB_EXPORT rc_t CC KDBManagerMakeUpdate ( KDBManager **mgrp, KDirectory *wd )
{
    if ( wd == NULL || ! wd -> read_only )
        return KDBManagerMake ( mgrp, wd, "make-update", NULL );

    if ( mgrp != NULL )
        * mgrp = NULL;

    return RC ( rcDB, rcMgr, rcConstructing, rcDirectory, rcReadonly );
}

LIB_EXPORT rc_t CC KDBManagerMakeUpdateWithVFSManager (
    KDBManager ** mgrp, KDirectory * wd, struct VFSManager * vfs )
{
    if ( wd == NULL || ! wd -> read_only )
        return KDBManagerMake ( mgrp, wd, "make-update", vfs );

    if ( mgrp != NULL )
        * mgrp = NULL;

    return RC ( rcDB, rcMgr, rcConstructing, rcDirectory, rcReadonly );
}


/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "path" [ IN ] - NUL terminated path
 *
 * TBD: Better reasons for non local paths
 */
static
rc_t KDBManagerWritableInt ( const KDirectory * dir, const char * path )
{
    rc_t rc;

    int type = KDBPathType ( /*NULL,*/ dir, NULL, path ) & ~ kptAlias;
    switch ( type )
    {
    case kptDatabase:
    case kptTable:
    case kptPrereleaseTbl:
    case kptColumn:
        rc = KDBWritable ( dir, path );
        break;

    case kptIndex:
    case kptMetadata:
        /* a wrong database type */
        rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcIncorrect );
        break;
#if 0
        /*TBD - eventually we need to check for an archive here */
    case kptFile: 
        /* check if this is an archive .tar or .sra and return rcReadonly if it is */
#endif
    case kptNotFound:
        rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcNotFound );
        break;
    case kptBadPath:
        rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcInvalid );
        break;
    default:
        rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcIncorrect );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVWritable ( const KDBManager *self, const char * path, va_list args )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcDB, rcMgr, rcAccessing, rcSelf, rcNull );
    else
    {
        char dbpath [ 4096 ];

        /* get full path to object */
        rc = KDirectoryVResolvePath ( self -> wd, true, dbpath, sizeof dbpath, path, args );
        if ( rc == 0 )
            rc = KDBManagerWritableInt ( self -> wd, dbpath );
    }
    return rc;
}

LIB_EXPORT rc_t CC KDBManagerWritable ( const KDBManager *self, const char * path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDBManagerVWritable ( self, path, args );

    va_end ( args );

    return rc;
}

/* Lock
 *  apply lock
 *
 *  if object is already locked, the operation is idempotent
 *  and returns an rc state of rcLocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC KDBManagerVLock ( KDBManager *self, const char *path, va_list args )
{
    rc_t rc;
    char full [ 4096 ];

    if ( self == NULL )
        rc =  RC ( rcDB, rcMgr, rcLocking, rcSelf, rcNull );
    else
    {
        /* TBD:
           Add code to validate that this path is not in a container
           object's control.  That is the last facet in the path
           is the name of the object.  To be valid the facet before
           must not be "col", "tbl", or "db" as in those cases
           the KDBManager should not be called for this object but 
           its containing object
        */

        /* get full path to object */
        rc = KDirectoryVResolvePath ( self -> wd, true,
            full, sizeof full, path, args );
        if ( rc == 0 )
        {
            /* if the path is not writable because it is already locked
             * the return from this will contain "rcLocked" and the 
             * caller will be forced to check for zero or RCState(rcLocked)
             *
             * a side effect is that if the call does not return 0 it will
             * be a wrong type to lock with the DBManager
             */
            rc = KDBManagerWritableInt ( self -> wd, full );
            if ( rc == 0 )
            {
                rc = KDBManagerCheckOpen ( self, full );
                if ( rc == 0 )
                    rc = KDBLockDir ( self -> wd, full );
            }
        }

        /* return dbmgr context */
        if ( rc != 0 )
            rc = ResetRCContext ( rc, rcDB, rcMgr, rcLocking );
    }

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerLock ( KDBManager *self, const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDBManagerVLock ( self, path, args );

    va_end ( args );

    return rc;
}

/* Unlock
 *  remove lock
 *
 *  if object is already unlocked, the operation is idempotent
 *  and returns an rc state of rcUnlocked
 *
 *  "path" [ IN ] - NUL terminated path
 */
LIB_EXPORT rc_t CC KDBManagerVUnlock ( KDBManager *self, const char *path, va_list args )
{
    rc_t rc;
    char full [ 4096 ];

    if ( self == NULL )
        rc =  RC ( rcDB, rcMgr, rcUnlocking, rcSelf, rcNull );
    else
    {
        /* get full path to object */
        rc = KDirectoryVResolvePath ( self -> wd, true,
            full, sizeof full, path, args );
        if ( rc == 0 )
        {
            /* TBD:
               Add code to validate that this path is not in a container
               object's control.  That is the last facet in the path
               is the name of the object.  To be valid the facet before
               must not be "col", "tbl", or "db" as in those cases
               the KDBManager should not be called for this object but 
               its containing object
            */

            /* if the path is not writable because it is already locked
             * we attempt to unlock it.
             *
             * if the return is 0 from Writable than it was already 
             * unlocked so we say so and thus again the
             * the return from this will contain "rcLocked" and the 
             * caller will be forced to check for zero or RCState(rcUnlocked)
             *
             * a side effect is that if the call does not return 0 it will
             * be a wrong type to lock with the DBManager
             */
            rc = KDBManagerWritableInt ( self -> wd, full );
            if ( rc == 0 )
                rc = RC ( rcDB, rcMgr, rcUnlocking, rcLock, rcUnlocked );
            else if ( GetRCState ( rc ) == rcLocked )
            {
                rc = KDBManagerCheckOpen ( self, full );
                if ( rc == 0 )
                    rc = KDBUnlockDir ( self -> wd, full );
            }
        }

        if ( rc != 0 )
            rc = ResetRCContext ( rc, rcDB, rcMgr, rcUnlocking );
    }
    return rc;
}

LIB_EXPORT rc_t CC KDBManagerUnlock ( KDBManager *self, const char *path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDBManagerVUnlock ( self, path, args );

    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVDrop ( KDBManager *self, uint32_t obj_type, const char *path, va_list args )
{
    return KDBMgrVDrop ( self -> wd, self, obj_type, path, args );
}

LIB_EXPORT rc_t CC KDBManagerDrop ( KDBManager *self, uint32_t obj_type, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVDrop ( self, obj_type, path, args );
    va_end ( args );

    return rc;
}



/* RunPeriodicTasks
 *  executes periodic tasks, such as cache flushing
 */
LIB_EXPORT rc_t CC KDBManagerRunPeriodicTasks ( const KDBManager *self )
{
    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcExecuting, rcSelf, rcNull );

    return 0;
}

/* PathType
 * check the path type of an object/directory path.
 * this is an extension of the KDirectoryPathType and will return
 * the KDirectory values if a path type is not specifically a
 * kdb object
 */
static int CC KDBManagerPathTypeVPImpl( const KDBManager * self,
    const VPath * path, bool reliable )
{
    int path_type = kptBadPath;
    if ( self != NULL && path != NULL )
    {
        /*
         * resolve the possible relative path or accession into
         * a final path we can open directly
         */
		VPath * rpath;
		rc_t rc = VFSManagerResolvePath( self->vfsmgr, vfsmgr_rflag_kdb_acc, path, &rpath );
        if ( rc == 0 )
        {
            const KDirectory * dir;

            /*
             * Most KDBPathType values are based on 'directories'
             * so try to open the resolved path as a directory
             */
            if ( reliable )
                rc = VFSManagerOpenDirectoryReadDecrypt          ( self->vfsmgr,
                    &dir, rpath );
            else
                rc = VFSManagerOpenDirectoryReadDecryptUnreliable( self->vfsmgr,
                    &dir, rpath );
            if ( rc == 0 )
            {
                path_type = KDBPathTypeDir( dir, kptDir, NULL, "." );
                KDirectoryRelease( dir );
            }
            /*
             * If we couldn't open the path as a directory we 'might'
             * have a KDB idx but we will only try that for a limited
             * set of uri schemes.
             */
            else
            {
                if ( VPathIsFSCompatible( rpath ) )
                {
                    char buffer [ 4096 ];
                    rc = VPathReadPath( rpath, buffer, sizeof buffer, NULL );
                    if ( rc == 0 )
                        path_type = KDBPathType( self -> wd, false, buffer );
                }
            }
            VPathRelease( rpath );
        }
    }
    return path_type;
}

LIB_EXPORT
int CC KDBManagerPathTypeVP( const KDBManager * self, const VPath * path )
{
    return KDBManagerPathTypeVPImpl ( self, path, true );
}

LIB_EXPORT int CC KDBManagerVPathTypeImpl ( const KDBManager * self,
    const char *path, va_list args, bool reliable )
{
    int path_type = kptBadPath;

    if ( self != NULL && path != NULL )
    {
        VPath * vp;
        rc_t rc;

        rc = VFSManagerVMakePath( self -> vfsmgr, &vp, path, args );
        if ( rc == 0 )
        {
            path_type = KDBManagerPathTypeVPImpl( self, vp, reliable );
            VPathRelease( vp );
        }
    }
    return path_type;
}

LIB_EXPORT int CC KDBManagerVPathType ( const KDBManager * self,
    const char *path, va_list args )
{
    return KDBManagerVPathTypeImpl ( self, path, args, true );
}
LIB_EXPORT int CC KDBManagerVPathTypeUnreliable ( const KDBManager * self,
    const char *path, va_list args )
{
    return KDBManagerVPathTypeImpl ( self, path, args, false );
}

LIB_EXPORT int CC KDBManagerPathType ( const KDBManager * self, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );

    rc = KDBManagerVPathType ( self, path, args );

    va_end (args);
    return rc;
}

