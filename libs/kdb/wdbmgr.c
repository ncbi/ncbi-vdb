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

#include <kdb/index.h>
#include <kdb/kdb-priv.h>

#include "libkdb.vers.h"
#include "wdbmgr.h"
#include "wkdb.h"
#include "kdb-cmn.h"
#include "wtable.h"
#include "windex.h"
#include "wdatabase.h"
#include "colfmt.h"
#include "wcolumn.h"
#include "wmeta.h"

#include "../vfs/path-priv.h" /* VPathSetDirectory */

#include <kfs/impl.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h> /* VFSManagerOpenDirectoryReadDecryptUnreliable */
#include <vfs/resolver.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>

#include <klib/symbol.h>
#include <klib/checksum.h>
#include <klib/rc.h>
#include <klib/printf.h>

#include <sysalloc.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/*--------------------------------------------------------------------------
 * KDBManager
 *  handle to library
 */

static rc_t CC KDBWManagerVWritable ( const KDBManager *self, const char * path, va_list args );
static rc_t CC KDBWManagerRunPeriodicTasks ( const KDBManager *self );
static int CC KDBWManagerPathTypeVP( const KDBManager * self, const VPath * path );
static int CC KDBWManagerVPathType ( const KDBManager * self, const char *path, va_list args );
static int CC KDBWManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args );
static rc_t CC KDBWManagerVOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, va_list args, const VPath *vpath );
static rc_t CC KDBWManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args );
static rc_t CC KDBWManagerOpenTableReadVPath(const KDBManager *self, const KTable **tbl, const struct VPath *path );
static rc_t CC KDBWManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args );
static rc_t CC KDBWManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath );
static rc_t CC KDBWManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache );

static KDBManager_vt KDBWManager_vt =
{
    KDBManagerWhack,
    KDBManagerBaseAddRef,
    KDBManagerBaseRelease,
    KDBManagerCommonVersion,
    KDBManagerCommonVExists,
    KDBWManagerVWritable,
    KDBWManagerRunPeriodicTasks,
    KDBWManagerPathTypeVP,
    KDBWManagerVPathType,
    KDBWManagerVPathTypeUnreliable,
    KDBWManagerVOpenDBRead,
    KDBWManagerVOpenTableRead,
    KDBWManagerOpenTableReadVPath,
    KDBWManagerVOpenColumnRead,
    KDBWManagerVPathOpenLocalDBRead,
    KDBWManagerVPathOpenRemoteDBRead,
    KDBCmnManagerGetVFSManager
};

/* MakeUpdate
 *  create library handle
 */
LIB_EXPORT rc_t CC KDBManagerMakeUpdate ( KDBManager **mgrp, KDirectory *wd )
{
    if ( wd == NULL || ! wd -> read_only )
        return KDBManagerMake ( mgrp, wd, "make-update", NULL, & KDBWManager_vt );

    if ( mgrp != NULL )
        * mgrp = NULL;

    return RC ( rcDB, rcMgr, rcConstructing, rcDirectory, rcReadonly );
}

LIB_EXPORT rc_t CC KDBManagerMakeUpdateWithVFSManager (
    KDBManager ** mgrp, KDirectory * wd, struct VFSManager * vfs )
{
    if ( wd == NULL || ! wd -> read_only )
        return KDBManagerMake ( mgrp, wd, "make-update", vfs, & KDBWManager_vt );

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
        rc = KDBWWritable ( dir, path );
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

static
rc_t CC
KDBWManagerVWritable ( const KDBManager *self, const char * path, va_list args )
{
    char dbpath [ 4096 ];

    /* get full path to object */
    rc_t rc = KDirectoryVResolvePath ( self -> wd, true, dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
        rc = KDBManagerWritableInt ( self -> wd, dbpath );
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
static
rc_t CC
KDBWManagerRunPeriodicTasks ( const KDBManager *self )
{
    return 0;
}

/* PathType
 * check the path type of an object/directory path.
 * this is an extension of the KDirectoryPathType and will return
 * the KDirectory values if a path type is not specifically a
 * kdb object
 */
static
int CC
KDBManagerPathTypeVPImpl( const KDBManager * self, const VPath * path, bool reliable )
{
    int path_type = kptBadPath;
    if ( self != NULL && path != NULL )
    {
        VPath * rpath = NULL;
        const KDirectory * dir = NULL;
        rc_t rc = VPathGetDirectory(path, &dir);
        if (rc != 0 || dir == NULL) {
          /*
           * resolve the possible relative path or accession into
           * a final path we can open directly
           */
          rc = VFSManagerResolvePath( self->vfsmgr, vfsmgr_rflag_kdb_acc,
              path, &rpath );
          if ( rc == 0 ) {
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
          }
        }
        if ( rc == 0 ) {
                path_type = KDBPathTypeDir( dir, kptDir, NULL, "." );
                VPathSetDirectory((VPath*)path, dir);
                KDirectoryRelease( dir );
        }
        /*
         * If we couldn't open the path as a directory we 'might'
         * have a KDB idx but we will only try that for a limited
         * set of uri schemes.
         */
        else {
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
    return path_type;
}

static
int CC
KDBWManagerPathTypeVP( const KDBManager * self, const VPath * path )
{
    return KDBManagerPathTypeVPImpl ( self, path, true );
}

static
int CC
KDBManagerVPathTypeImpl ( const KDBManager * self, const char *path, va_list args, bool reliable )
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

static
int CC
KDBWManagerVPathType ( const KDBManager * self, const char *path, va_list args )
{
    return KDBManagerVPathTypeImpl ( self, path, args, true );
}

static
int CC
KDBWManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args )
{
    return KDBManagerVPathTypeImpl ( self, path, args, false );
}

rc_t KDBManagerInsertDatabase ( KDBManager * self, KDatabase * db )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &db->sym);
    if ( rc == 0 )
        db -> mgr = KDBManagerAttach ( self );
    return rc;
}

/* OpenDBRead
 * VOpenDBRead
 *  open a database for read
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KDBManagerVOpenDBReadInt ( const KDBManager *cself, const KDatabase **dbp,
    KDirectory *wd,
    const char *path, va_list args, bool *cached, bool try_srapath,
    const VPath *vpath )
{
    char key_path[ 4096 ] = "";
    char short_path[ 4096 ] = "";
    size_t z;
    rc_t rc = 0;
    KSymbol *sym = NULL;
    if (vpath == NULL) {
      rc = string_vprintf(short_path, sizeof short_path, &z, path, args);
      if ( rc == 0 )
        rc = KDirectoryResolvePath ( wd, true, key_path, sizeof key_path,
            short_path );
      if ( rc == 0 )
      {
        /* if already open */
        sym = KDBManagerOpenObjectFind( cself, key_path );
        if ( sym != NULL )
        {
            const KDatabase *cdb;
            rc_t obj;

            if ( cached != NULL )
                * cached = true;

            switch (sym->type)
            {
            case kptDatabase:
                cdb = (KDatabase*)sym->u.obj;
                /* if open for update, refuse */
                if ( cdb -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KDatabaseAddRef ( cdb );
                    if ( rc == 0 )
                        * dbp = cdb;
                    return rc;
                }
                obj = rcDatabase;
                break;

            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            rc = RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
        }
      }
    }
    if ( sym == NULL ) {
            const KDirectory *dir;

            if ( cached != NULL )
                * cached = false;

            /* open the directory if its a database */
            rc = KDBManagerOpenPathTypeRead ( cself, wd, short_path, &dir, kptDatabase, NULL,
                try_srapath, vpath );
            if ( rc == 0 )
            {
                KDatabase *db;

                rc = KWDatabaseMake ( &db, dir, key_path, NULL, true );
                if ( rc == 0 )
                {
                    KDBManager *self = ( KDBManager* ) cself;

                    rc = KDBManagerInsertDatabase ( self, db );
                    if ( rc == 0 )
                    {
                        * dbp = db;
                        return 0;
                    }
                    free (db);
                }
                KDirectoryRelease (dir);
            }
    }
    return rc;
}

rc_t KDBWManagerVOpenDBReadInt_noargs ( const KDBManager *cself,
    const KDatabase **dbp, KDirectory *wd,
    const char *path, bool *cached, int try_srapath, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, try_srapath );
    rc = KDBManagerVOpenDBReadInt ( cself, dbp, wd, path, args, cached, (bool)try_srapath, NULL );
    va_end ( args );

    return rc;
}

static rc_t CC KDBWManagerVOpenDBRead ( const KDBManager *self, const KDatabase **db, const char *path, va_list args, const VPath *vpath )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    return KDBManagerVOpenDBReadInt ( self, db, self -> wd, path, args, NULL,
        true, vpath );
}

static
rc_t KDBManagerMakeDBUpdate ( KDBManager *self,
    KDatabase **dbp, KDirectory *wd, const char *path, KMD5SumFmt *md5 )
{
    rc_t rc;
    KDirectory *dir;

    rc = KDirectoryOpenDirUpdate ( wd, & dir, false, "%s", path );
    if ( rc == 0 )
    {
        KDatabase *db;

        /* Make the base object */
        rc = KWDatabaseMake ( & db, dir, path, md5, false );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertDatabase ( self, db );

            if ( rc == 0 )
            {
                * dbp = db;
                return 0;   /* successful exit */
            }

            rc = RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcBusy );

            KDatabaseRelease ( db );
        }
        KDirectoryRelease ( dir );
    }
    return rc;
}

/* OpenDBUpdate
 * VOpenDBUpdate
 *  open a database for read/write
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KDBManagerVOpenDBUpdateInt ( KDBManager *self,
    KDatabase **db, KDirectory *wd, const char *path, va_list args )
{
    char dbpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KFile *f;
        KMD5SumFmt * md5 = NULL;

        /* if already open, refuse */
        sym = KDBManagerOpenObjectFind (self, dbpath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }
        /* only open existing dbs */
        switch (KDBPathType ( /*self,*/ wd, NULL, dbpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a database
	     * or a non related file */
	    if ( KDBManagerOpenPathTypeRead ( self, wd, dbpath, NULL, kptDatabase, NULL, false,
            NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcOpening, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        case kptDatabase:
        case kptDatabase | kptAlias:
            break;
        }

        /* test now for locked directory */
        rc = KDBWWritable (wd, dbpath);
        switch (GetRCState (rc))
        {
        default:
#if 0
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcNoPerm ); /* TBD: better state? */
#endif

        case rcLocked:
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcLocked );

        case rcReadonly:
            return RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcReadonly );


        case 0:
            rc = 0;
            break;
        }

        rc = KDirectoryOpenFileWrite ( wd, &f, true, "%s/md5", dbpath );
        if ( rc == 0 )
        {
            rc = KMD5SumFmtMakeUpdate ( & md5, f );
            if ( rc != 0 )
                KFileRelease ( f );
        }
        else if ( GetRCState ( rc ) == rcNotFound )
            rc = 0;

        if ( rc == 0 )
            rc = KDBManagerMakeDBUpdate ( self, db, wd, dbpath, md5 );

        KMD5SumFmtRelease ( md5 );
    }

    return rc;
}

rc_t KDBManagerVOpenDBUpdateInt_noargs ( KDBManager *self,
    KDatabase **db, KDirectory *wd, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBUpdateInt ( self, db, wd, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenDBUpdate ( KDBManager *self,
    KDatabase **db, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBUpdate ( self, db, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenDBUpdate ( KDBManager *self,
    KDatabase **db, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenDBUpdateInt ( self, db, self -> wd, path, args );
}


/* CreateDB
 * VCreateDB
 *  create a new or open an existing database
 *
 *  "db" [ OUT ] - return parameter for newly opened database
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to database
 */
static
rc_t KDBManagerVCreateDBInt ( KDBManager *self,
    KDatabase **db, KDirectory *wd, KCreateMode cmode,
    const char *path, va_list args )
{
    char dbpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
    {
        /* we won't try accession resolution here */
        int type = KDBPathType ( /*NULL,*/ wd, NULL, dbpath );
        switch ( type )
        {
        case kptNotFound:
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );

        case kptDatabase:
        case kptDatabase | kptAlias:
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ((cmode & kcmValueMask) == kcmCreate)
                return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcExists );
            if (KDBManagerOpenObjectBusy (self, dbpath))
                return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcBusy );
            /* test now for locked directory */
            rc = KDBWWritable ( wd, dbpath );
            if (rc)
            {
                switch (GetRCState(rc))
                {
                default:
                    return rc;
                case rcLocked:
                    return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcLocked );
                case rcReadonly:
                    return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcReadonly );
                case rcNotFound:
                    /* not found is good but probably unreachable */
                    break;
                case 0:
                    rc = 0;
                    break;
                }
            }
            /* second good path */
            break;

        case kptTable:
        case kptTable | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcTable, rcExists);

        case kptColumn:
        case kptColumn | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcColumn, rcExists);

        case kptIndex:
        case kptIndex | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcIndex, rcExists);

        case kptMetadata:
        case kptMetadata | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcMetadata, rcExists);

        case kptFile:
        case kptFile | kptAlias:
            /* if we find a file, vary the failure if it is an archive that is a database
             * or a non related file */
            if ( KDBManagerOpenPathTypeRead ( self, wd, dbpath, NULL, kptDatabase, NULL, false,
                                      NULL ) == 0 )
                return RC ( rcDB, rcMgr, rcCreating, rcDirectory, rcUnauthorized );
            /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        /* [re]create directory */
        rc = KDirectoryCreateDir ( wd, 0775, cmode, "%s", dbpath );
        if (rc == 0)
            /* create tbl subdirectory as required for db */
            rc = KDirectoryCreateDir ( wd, 0775, kcmOpen, "%s/tbl", dbpath );

        if ( rc == 0 )
        {
            KMD5SumFmt *md5 = NULL;

            if ( ( cmode & kcmMD5 ) != 0 )
            {
                KFile * f;

                /* if needed create the md5 digest file */
                rc = KDirectoryCreateFile ( wd, &f, true, 0664, kcmOpen, "%s/md5", dbpath );
                if ( rc == 0 )
                {
                    /* create a formatter around file
                       formatter will own "f" afterward */
                    rc = KMD5SumFmtMakeUpdate ( & md5, f );

                    /* if failed to create formatter, release "f" */
                    if ( rc != 0 )
                        KFileRelease ( f );
                }
            }

            if ( rc == 0 )
                rc = KDBManagerMakeDBUpdate ( self, db, wd, dbpath, md5 );
            /* TODO: for now set cmode here, externally to KDatabaseMake
                maybe it's needed to refactor the whole chain from exported
                Open/Create DB functions to the KDatabaseMake
            */
            if ( rc == 0 )
                KDatabaseSetCmode ( *db, cmode );

            KMD5SumFmtRelease ( md5 );
        }
    }
    return rc;
}

rc_t KDBManagerVCreateDBInt_noargs ( KDBManager *self,
    KDatabase **db, KDirectory *wd, KCreateMode cmode,
    const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateDBInt ( self, db, wd, cmode, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVCreateDB ( KDBManager *self,
    KDatabase **db, KCreateMode cmode, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );

    return KDBManagerVCreateDBInt ( self, db, self -> wd, cmode, path, args );
}


LIB_EXPORT rc_t CC KDBManagerCreateDB ( KDBManager *self,
    KDatabase **db, KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateDB ( self, db, cmode, path, args );
    va_end ( args );

    return rc;
}

rc_t KDBManagerInsertTable ( KDBManager * self, KWTable * tbl )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &tbl->sym);
    if ( rc == 0 )
        tbl -> mgr = KDBManagerAttach ( self );
    return rc;
}

/* OpenTableRead
 * VOpenTableRead
 *  open a table for read
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
static
rc_t KDBManagerVOpenTableReadInt ( const KDBManager *cself,
    const KWTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, va_list args, const struct VPath *vpath )
{
    const String * str = NULL;
    char key_path[ 4096 ];
    char short_path[ 4096 ];
    size_t z;
    rc_t rc = 0;

    if (vpath != NULL) {
        assert(path == NULL);
        rc = VPathMakeString(vpath, &str);
        if (rc == 0)
            path = str->addr;
    }

    rc = string_vprintf(short_path, sizeof short_path, &z, path, args);

    if (str != NULL) {
        StringWhack(str);
        str = NULL;
        path = NULL;
    }

    if ( rc == 0 )
		rc = KDirectoryResolvePath ( wd, true, key_path, sizeof key_path, short_path );
    if ( rc == 0 )
    {
        KSymbol *sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind( cself, key_path );
        if (sym != NULL)
        {
            const KWTable * ctbl;
            rc_t obj;

            switch (sym->type)
            {
            case kptTable:
                ctbl = (KWTable*)sym->u.obj;
                /* if open for update, refuse */
                if ( ctbl -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KTableAddRef ( & ctbl -> dad );
                    if ( rc == 0 )
                        * tblp = ctbl;
                    return rc;
                }
                obj = rcTable;
                break;

            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            rc = RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
        }
        else
        {
			KWTable * tbl;
			const KDirectory *dir;
			bool prerelease = false;

			rc = KDBManagerOpenPathTypeRead ( cself, wd, short_path, &dir, kptTable, NULL, try_srapath, vpath );
			if ( rc != 0 )
			{
				prerelease = true;
				rc = KDBManagerOpenPathTypeRead ( cself, wd, short_path, &dir, kptPrereleaseTbl, NULL, try_srapath, vpath );
			}
			if ( rc == 0 )
			{
				rc = KWTableMake ( & tbl, dir, key_path, NULL, true );
				if ( rc == 0 )
				{
					KDBManager * self = (KDBManager *)cself;

					rc = KDBManagerInsertTable( self, tbl );

					if (rc == 0)
					{
						tbl -> prerelease = prerelease;
						* tblp = tbl;
						return 0;
					}
					free ( tbl );
				}
				KDirectoryRelease ( dir );
			}
        }
    }
    return rc;
}

rc_t KDBWManagerVOpenTableReadInt_noargs ( const KDBManager *cself,
    const KWTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, const struct VPath *vpath, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, vpath );
    rc = KDBManagerVOpenTableReadInt ( cself, tblp, wd, try_srapath, path, args, vpath );
    va_end ( args );

    return rc;
}

static rc_t CC KDBWManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    return KDBManagerVOpenTableReadInt ( self, (const KWTable **)tbl, self -> wd, true, path, args, NULL );
}

static rc_t CC KDBWManagerOpenTableReadVPath(const KDBManager *self, const KTable **tbl, const struct VPath *path)
{
    if (tbl == NULL)
        return RC(rcDB, rcMgr, rcOpening, rcParam, rcNull);

    *tbl = NULL;

    return KDBWManagerVOpenTableReadInt_noargs(self, (const KWTable **)tbl, self->wd, true, NULL, path);
}

rc_t KDBManagerInsertColumn ( KDBManager * self, KWColumn * col )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &col->sym);
    if ( rc == 0 )
        col -> mgr = KDBManagerAttach ( self );
    return rc;
}

/* CreateColumn
 * VCreateColumn
 *  create a new or open an existing column
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "checksum" [ IN ] - checksum mode
 *
 *  "pgsize" [ IN ] - size of internal column pages
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVCreateColumnInt ( KDBManager *self,
    KWColumn **colp, KDirectory *wd, KCreateMode cmode,
    KChecksum checksum, size_t pgsize, const char *path, va_list args )
{
    char colpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        colpath, sizeof colpath, path, args );
    if ( rc == 0 )
    {
        KDirectory *dir;

        switch ( KDBPathType ( /*NULL,*/ wd, NULL, colpath ) )
        {
        case kptNotFound:
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );
        case kptColumn:
        case kptColumn | kptAlias:
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ( ( cmode & kcmValueMask ) == kcmCreate )
                return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcExists );
            if (KDBManagerOpenObjectBusy (self, colpath))
                return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcBusy );
            /* test now for locked directory */
            rc = KDBWWritable (wd, colpath);
            if (rc)
            {
                switch (GetRCState(rc))
                {
                default:
                    return rc;
                case rcLocked:
                    return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcLocked );
                case rcReadonly:
                    return RC ( rcDB, rcMgr, rcCreating, rcColumn, rcReadonly );
                case rcNotFound:
                    /* not found is good but probably unreachable */
                    break;
                case 0:
                    rc = 0;
                    break;
                }
            }
            /* second good path */
            break;

        case kptDatabase:
        case kptDatabase | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcDatabase, rcExists);

        case kptTable:
        case kptTable | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcTable, rcExists);

        case kptIndex:
        case kptIndex | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcIndex, rcExists);

        case kptMetadata:
        case kptMetadata | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcMetadata, rcExists);

	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a column
	     * or a non related file */
	    if ( KDBManagerOpenPathTypeRead ( self, wd, colpath, NULL, kptColumn, NULL, false,
            NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcCreating, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        /* [re]create directory */
        rc = KDirectoryCreateDir ( wd, 0775, cmode, "%s", colpath );
        if ( rc != 0 )
            return rc;

        /* create column as a directory */
        rc = KDirectoryOpenDirUpdate ( wd, & dir, false, "%s", colpath );
        if ( rc == 0 )
        {
            KWColumn *col = NULL;
            KMD5SumFmt *md5 = NULL;

            /* create an MD5 digest file for column */
            if ( KCOL_CURRENT_VERSION >= 3 && ( cmode & kcmMD5 ) != 0 )
            {
                KFile * f;

                /* create or open the md5 digest file */
                rc = KDirectoryCreateFile ( wd, &f, true, 0664, kcmOpen, "%s/md5", colpath );
                if ( rc == 0 )
                {
                    /* create a formatter around file
                       formatter will own "f" afterward */
                    rc = KMD5SumFmtMakeUpdate ( & md5, f );

                    /* if failed to create formatter, release "f" */
                    if ( rc != 0 )
                        KFileRelease ( f );
                }

            }

            /* create column - will attach several references to "md5" */
            if ( rc == 0 )
                rc = KWColumnCreate ( & col, dir, cmode, checksum, pgsize, colpath, md5 );

            /* release our reference to "md5" if NULL then no problem */
            if (md5)
                KMD5SumFmtRelease ( md5 );

            if ( rc == 0 )
            {
                rc = KDBManagerInsertColumn ( self, col );
                if (rc == 0 )
                {
                    * colp = col;
                    return 0;
                }

                KColumnRelease ( & col -> dad );
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;
}

rc_t KDBManagerVCreateColumnInt_noargs ( KDBManager *self,
    KWColumn **colp, KDirectory *wd, KCreateMode cmode,
    KChecksum checksum, size_t pgsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateColumnInt ( self, colp, wd, cmode, checksum, pgsize, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVCreateColumn ( KDBManager *self, KColumn **col,
    KCreateMode cmode, KChecksum checksum, size_t pgsize,
    const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcParam, rcNull );

    * col = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );

    return KDBManagerVCreateColumnInt
        ( self, (KWColumn **)col, self -> wd, cmode, checksum, pgsize, path, args );
}

LIB_EXPORT rc_t CC KDBManagerCreateColumn ( KDBManager *self, KColumn **col,
    KCreateMode cmode, KChecksum checksum, size_t pgsize, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateColumn ( self, col, cmode, checksum, pgsize, path, args );
    va_end ( args );

    return rc;
}

/* OpenColumnRead
 * VOpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path_fmt" + "args" (formatted string with varargs)  [ IN ] - NUL terminated
 *  string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVOpenColumnReadInt2 ( const KDBManager *cself,
    const KWColumn **colp, const KDirectory *wd,
    const char *path_fmt, va_list args, bool *cached, bool try_srapath, va_list args2 )
{
    char colpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        colpath, sizeof colpath, path_fmt, args );
    if ( rc == 0 )
    {
        KSymbol *sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind (cself, colpath);
        if (sym != NULL)
        {
            const KWColumn *ccol;
            rc_t obj;

            if(cached != NULL ) *cached = true;

            switch (sym->type)
            {
            case kptColumn:
                ccol = (const KWColumn*)sym->u.obj;
                /* if open for update, refuse */
                if ( ccol -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KColumnAddRef ( & ccol -> dad );
                    if ( rc == 0 )
                        * colp = ccol;
                    return rc;
                }
                obj = rcColumn;
                break;
            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            rc = RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
        }
        else
        {
            const KDirectory *dir;
            char path[4096];
            int size = 0;

            if ( cached != NULL )
                *cached = false;

            /* TODO: check if colpath is what we want to pass to KDBManagerOpenPathTypeRead
             * in this case we don't need to vprintf to 'path'
            */
            /* VDB-4386: cannot treat va_list as a pointer! */
            if ( path_fmt != NULL )
                size = /*( args == NULL ) ? snprintf  ( path, sizeof path, "%s", path_fmt ) :*/
                    vsnprintf ( path, sizeof path, path_fmt, args2 );
            if ( size < 0 || ( size_t ) size >=  sizeof path )
                rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );

            if (rc == 0)
                rc = KDBManagerOpenPathTypeRead ( cself, wd, path, &dir, kptColumn, NULL,
                    try_srapath, NULL );

            if ( rc == 0 )
            {
                KWColumn *col;

                rc = KWColumnMakeRead ( (KWColumn **)& col, dir, colpath, NULL );

                if ( rc == 0 )
                {
                    rc = KDBManagerInsertColumn ( (KDBManager*)cself, col );
                    if ( rc == 0 )
                    {
                        * colp = col;
                        return 0;
                    }

                    KColumnRelease ( & col -> dad );
                }

                KDirectoryRelease ( dir );
            }
        }
    }
    return rc;
}

static
rc_t KDBManagerVOpenColumnReadInt ( const KDBManager *cself,
    const KWColumn **colp, const KDirectory *wd,
    const char *path_fmt, va_list args, bool *cached, bool try_srapath )
{
    rc_t rc;
    va_list args2;

    /* VDB-4386: cannot treat va_list as a pointer! */
/*    if ( args == NULL )
        return KDBManagerVOpenColumnReadInt2 ( cself, colp, wd, path_fmt, args, cached, try_srapath, NULL );*/

    va_copy ( args2, args );
    rc = KDBManagerVOpenColumnReadInt2 ( cself, colp, wd, path_fmt, args, cached, try_srapath, args2 );
    va_end ( args2 );

    return rc;
}

rc_t KDBWManagerVOpenColumnReadInt_noargs ( const KDBManager *cself,
    const KWColumn **colp, const KDirectory *wd,
    const char *path_fmt, bool *cached, int try_srapath, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, try_srapath );
    rc = KDBManagerVOpenColumnReadInt ( cself, colp, wd, path_fmt, args, cached, (bool)try_srapath );
    va_end ( args );

    return rc;
}

static
rc_t CC
KDBWManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * col = NULL;

    return KDBManagerVOpenColumnReadInt ( self, (const KWColumn **)col, self -> wd, path, args , NULL, true);
}

/* OpenColumnUpdate
 * VOpenColumnUpdate
 *  open a column for read/write
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path_fmt" formatted with "args" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVOpenColumnUpdateInt ( KDBManager *self,
    KWColumn **colp, KDirectory *wd, bool try_srapath,
    const char *path_fmt, va_list args )
{
    char colpath [ 4096 ];
    rc_t rc = 0;
    int z = 0;

/*    rc = KDirectoryVResolvePath ( wd, 1,
        colpath, sizeof colpath, path_fmt, args ); */
/* VDB-4386: cannot treat va_list as a pointer! */
    if ( path_fmt != NULL )
        z = /*(args == NULL) ? snprintf  ( colpath, sizeof colpath, "%s", path_fmt) :*/
            vsnprintf ( colpath, sizeof colpath, path_fmt, args );
    if (z < 0 || z >= (int) sizeof colpath)
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    if ( rc == 0 )
    {
        KSymbol * sym;
/*         KFile *f; */
/*         KMD5SumFmt * md5 = NULL; */
        KDirectory *dir;

        /* if already open, refuse */
        sym = KDBManagerOpenObjectFind (self, colpath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }
        /* only open existing dbs */
        switch (KDBPathType ( /*NULL,*/ wd, NULL, colpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
	case kptFile:
	case kptFile | kptAlias:
	    /* if we find a file, vary the failure if it is an archive that is a table
	     * or a non related file
	     * this should be changed to a readonly as it is not possible not
	     * disallowed.  rcReadonly not rcUnauthorized
	     */
	    if ( KDBManagerOpenPathTypeRead ( self, wd, colpath, NULL, kptColumn, NULL,
            try_srapath, NULL ) == 0 )
		return RC ( rcDB, rcMgr, rcOpening, rcDirectory, rcUnauthorized );
	    /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        case kptColumn:
        case kptColumn | kptAlias:
            break;
        }

        /* test now for locked directory */
        rc = KDBWWritable (wd, colpath);
        switch (GetRCState(rc))
        {
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcUnexpected );
        case rcLocked:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcLocked );
        case rcReadonly:
            return RC ( rcDB, rcMgr, rcOpening, rcColumn, rcReadonly );
        case 0:
            rc = 0;
            break;
        }


        rc = KDirectoryOpenDirUpdate ( wd, & dir, 0, "%s", colpath );
        if ( rc == 0 )
        {
            KWColumn *col;
            KMD5SumFmt *md5 = NULL;
            /* open existing md5 digest file */
            KFile * f;
            rc = KDirectoryOpenFileWrite_v1 ( dir, & f, true, "md5" );
            if ( rc == 0 )
            {
                rc = KMD5SumFmtMakeUpdate ( &md5, f );
                if ( rc != 0 )
                    KFileRelease ( f );
            }
            else if ( GetRCState ( rc ) == rcNotFound )
                rc = 0;

            /* make column - will attach several references to "md5" */
            if ( rc == 0 )
                rc = KWColumnMakeUpdate ( & col, dir, colpath, md5 );

            /* release our reference to "md5" */
            KMD5SumFmtRelease ( md5 );

            if ( rc == 0 )
            {
                rc = KDBManagerInsertColumn ( self, col );
                if ( rc == 0 )
                {
                    * colp = col;
                    return 0;
                }

                KColumnRelease ( & col -> dad );
            }
            KDirectoryRelease ( dir );
        }
    }
    return rc;
}

rc_t KDBManagerVOpenColumnUpdateInt_noargs ( KDBManager *self,
    KWColumn **colp, KDirectory *wd, bool try_srapath,
    const char *path_fmt, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path_fmt );
    rc = KDBManagerVOpenColumnUpdateInt ( self, colp, wd, try_srapath, path_fmt, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenColumnUpdate ( KDBManager *self,
    KColumn **col, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenColumnUpdate ( self, col, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenColumnUpdate ( KDBManager *self,
    KColumn **col, const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * col = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenColumnUpdateInt
        ( self, (KWColumn **)col, self -> wd, true, path, args );
}

static
rc_t CC
KDBWManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath )
{
    if ( p_db == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    if ( vpath == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );

    {
        /* vpath has already been resolved and is known to be a local path.
           open it if it is a database; avoid an additional round of resolution */
        const KDirectory *dir;
        rc_t rc = VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt ( self -> vfsmgr, self -> wd, &dir, vpath );
        if ( rc == 0 )
        {
            if ( ( (~kptAlias) & KDBPathType ( dir, NULL, "." ) ) != kptDatabase )
            {
                rc = RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcIncorrect );
            }
            else
            {
                KDatabase *db;

                rc = KWDatabaseMakeVPath ( & db, dir, vpath, NULL, true );
                if ( rc == 0 )
                {
                    rc = KDBManagerInsertDatabase ( ( KDBManager* ) self, db );
                    if ( rc == 0 )
                    {
                        * p_db = db;
                        return 0;
                    }
                    free (db);
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
}

static
rc_t CC
KDBWManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )
{
    if ( p_db == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    if ( remote == NULL )
        return RC ( rcDB, rcDatabase, rcAccessing, rcParam, rcNull );
    /* cache == NULL is OK */

    {
        /*  vpath has already been resolved and is known to be a remote URL.
            Open it if it is a database; use the provided cache; avoid an additional round of resolution */
        const KDirectory *dir;
        rc_t rc = VFSManagerOpenDirectoryReadDecryptRemote( self -> vfsmgr, &dir, remote, cache );
        if ( rc == 0 )
        {
            if ( ( (~kptAlias) & KDBPathType ( dir, NULL, "." ) ) != kptDatabase )
            {
                rc = RC ( rcDB, rcMgr, rcOpening, rcDatabase, rcIncorrect );
            }
            else
            {
                KDatabase *db;

                /* allocate a new guy */
                rc = KWDatabaseMakeVPath ( & db, dir, remote, NULL, true );
                if ( rc == 0 )
                {
                    rc = KDBManagerInsertDatabase ( ( KDBManager* ) self, db );
                    if ( rc == 0 )
                    {
                        * p_db = db;
                        return 0;
                    }
                    free (db);
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
}

static
rc_t KDBManagerMakeTableUpdate ( KDBManager *self,
    KWTable **tblp, KDirectory *wd, const char *path, bool convert, KMD5SumFmt * md5 )
{
    rc_t rc;
    KDirectory *dir;

    rc = KDirectoryOpenDirUpdate ( wd, & dir, false, "%s", path );
    if ( rc == 0 )
    {
        KWTable * tbl;

        if ( convert )
        {
            /* the old table layout:
               <tbl>/
               col/...
               meta
               sealed
               skey
            */
            rc = KDirectorySetAccess_v1 ( dir, true, 0220, 0222, "." );
            if ( rc == 0 )
                rc = KDirectoryCreateDir_v1 ( dir, 0775, kcmCreate, "md" );
            if ( rc == 0 )
            {
                rc = KDirectoryRename ( dir, false, "meta", "md/r001" );
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;
                else if ( rc == 0 )
                {
                    rc = KDirectoryCreateAlias ( dir, 0777, kcmCreate, "md/r001", "md/cur" );
                    if( rc == 0 ) {
                        KFile* md5_file = NULL;
                        if( (rc = KDirectoryOpenFileWrite(dir, &md5_file, true, "md5")) == 0 ) {
                            KMD5SumFmt* fmd5 = NULL;
                            if( (rc = KMD5SumFmtMakeUpdate(&fmd5, md5_file)) == 0 ) {
                                if( (rc = KMD5SumFmtRename(fmd5, "meta", "md/r001")) == 0 ) {
                                    rc = KMD5SumFmtFlush(fmd5);
                                }
                                KMD5SumFmtRelease(fmd5);
                            } else {
                                KFileRelease(md5_file);
                            }
                        }
                    }
                }
            }
            if ( rc == 0 )
                rc = KDirectoryCreateDir_v1 ( dir, 0775, kcmCreate, "idx" );
            if ( rc == 0 )
            {
                rc = KDirectoryRename ( dir, false, "skey", "idx/skey" );
                if ( GetRCState ( rc ) == rcNotFound )
                    rc = 0;
                else if ( rc == 0 )
                {
                    KDirectoryRename ( dir, false, "skey.md5", "idx/skey.md5" );
                }
            }
            if ( rc == 0 )
            {
                if ( KDirectoryPathType_v1 ( dir, "sealed" ) == kptFile )
                {
                    rc = KDBLockDir ( dir, "." );
                }
            }

            if ( rc != 0 )
                return rc;
        }

        /* Make the base object */
        rc = KWTableMake ( & tbl, dir, path, md5, false );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertTable ( self, tbl );

            if ( rc == 0 )
            {
                * tblp = tbl;
                return 0;
            }

            rc = RC ( rcDB, rcMgr, rcOpening, rcTable, rcBusy );

            free ( tbl );
        }

        KDirectoryRelease ( dir );
    }

    return rc;
}

/* OpenTableUpdate
 * VOpenTableUpdate
 *  open a table for read/write
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving name of table
 */
static
rc_t KDBManagerVOpenTableUpdateInt ( KDBManager *self,
    KWTable **tbl, KDirectory *wd, const char *path, va_list args )
{
    char tblpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        tblpath, sizeof tblpath, path, args );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KFile *f;
        KMD5SumFmt * md5 = NULL;
        bool convert = false;

        /* if already open, refuse */
        sym = KDBManagerOpenObjectFind (self, tblpath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }
        /* only open existing tbls */
        switch ( KDBPathType ( /*NULL,*/ wd, NULL, tblpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcNotFound );

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );

        case kptFile:
        case kptFile | kptAlias:
            /* if we find a file, vary the failure if it is an archive that is a table
             * or a non related file */
            if (( KDBManagerOpenPathTypeRead ( self, wd, tblpath, NULL, kptTable, NULL,
                                        false, NULL ) == 0 ) ||
                ( KDBManagerOpenPathTypeRead ( self, wd, tblpath, NULL, kptPrereleaseTbl, NULL,
                                        false, NULL ) == 0 ) )
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcUnauthorized );
            /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        case kptTable:
        case kptTable | kptAlias:
            break;

        case kptPrereleaseTbl:
        case kptPrereleaseTbl | kptAlias:
            convert = true;
            break;
        }

        /* test now for locked directory */
        rc = KDBWWritable ( wd, tblpath );
        switch (GetRCState(rc))
        {
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcNoPerm );

        case rcLocked:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcLocked );

        case rcReadonly:
            return RC ( rcDB, rcMgr, rcOpening, rcTable, rcReadonly );

        case 0:
            rc = 0;
            break;
        }

        rc = KDirectoryOpenFileWrite ( wd, &f, true, "%s/md5", tblpath );
        if ( rc == 0 )
        {
            rc = KMD5SumFmtMakeUpdate ( & md5, f );
            if ( rc != 0 )
                KFileRelease ( f );
        }
        else if ( GetRCState ( rc ) == rcNotFound )
            rc = 0;
#if 0
        else if ( GetRCState ( rc ) == rcUnauthorized )
            return RC ( rcDB, rcMgr, rcCreating, rcTable, rUnauthorized );
#endif

        if ( rc == 0 )
            rc = KDBManagerMakeTableUpdate ( self, tbl, wd, tblpath, convert, md5 );

        KMD5SumFmtRelease ( md5 );
    }

    return rc;
}

rc_t KDBManagerVOpenTableUpdateInt_noargs ( KDBManager *self,
    KWTable **tbl, KDirectory *wd, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenTableUpdateInt ( self, tbl, wd, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerOpenTableUpdate ( KDBManager *self,
    KTable **tbl, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenTableUpdate ( self, tbl, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenTableUpdate ( KDBManager *self,
    KTable **tbl, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenTableUpdateInt ( self, (KWTable**)tbl, self -> wd, path, args );
}

/* CreateTable
 * VCreateTable
 *  create a new or open an existing table
 *
 *  "tbl" [ OUT ] - return parameter for newly opened table
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "path" [ IN ] - NUL terminated string in
 *  wd-native character set giving path to table
 */
static
rc_t KDBManagerVCreateTableInt ( KDBManager *self,
    KWTable **tbl, KDirectory *wd, KCreateMode cmode, const char *path, va_list args )
{
    char tblpath [ 4096 ];
    rc_t rc = KDirectoryVResolvePath ( wd, true,
        tblpath, sizeof tblpath, path, args );
    if ( rc == 0 )
    {
        bool convert = false;
        bool not_found = false;

        switch ( KDBPathType ( /*NULL,*/ wd, NULL, tblpath ) )
        {
        case kptNotFound:
            not_found = true;
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );

        case kptTable:
        case kptTable | kptAlias:
            break;

        case kptPrereleaseTbl:
        case kptPrereleaseTbl | kptAlias:
            convert = true;
            break;

        case kptDatabase:
        case kptDatabase | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcDatabase, rcExists);

        case kptColumn:
        case kptColumn | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcColumn, rcExists);

        case kptIndex:
        case kptIndex | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcIndex, rcExists);

        case kptMetadata:
        case kptMetadata | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcMetadata, rcExists);

        case kptFile:
        case kptFile | kptAlias:
            /* if we find a file, vary the failure if it is an archive that is a table
             * or a non related file */
            if (( KDBManagerOpenPathTypeRead ( self, wd, tblpath, NULL, kptTable, NULL,
                                        false, NULL ) == 0 ) ||
                ( KDBManagerOpenPathTypeRead ( self, wd, tblpath, NULL, kptPrereleaseTbl, NULL,
                                        false, NULL ) == 0 ) )
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcReadonly );
            /* fall through */
        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        if (not_found == false)
        {
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ((cmode & kcmValueMask) == kcmCreate)
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcExists );
            if (KDBManagerOpenObjectBusy (self, tblpath))
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcBusy );
            /* test now for locked directory */
            rc = KDBWWritable ( wd, tblpath );
            switch (GetRCState(rc))
            {
            default:
                return rc;
            case rcLocked:
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcLocked );
            case rcReadonly:
                return RC ( rcDB, rcMgr, rcCreating, rcTable, rcReadonly );
            case rcNotFound:
                /* not found is good but probably unreachable */
                break;
            case 0:
                break;
            }
        }

        /* [re]create directory */
        rc = KDirectoryCreateDir ( wd, 0775, cmode, "%s", tblpath );
        if ( rc == 0 )
        {
            KMD5SumFmt *md5 = NULL;

            /* create column sub-directory */
            rc = KDirectoryCreateDir ( wd, 0775, kcmOpen, "%s/col", tblpath );
            if ( rc == 0 && ( cmode & kcmMD5 ) != 0 )
            {
                KFile * f;

                /* create the md5 digest file */
                rc = KDirectoryCreateFile ( wd, &f, true, 0664, kcmOpen, "%s/md5", tblpath );
                if ( rc == 0 )
                {
                    rc = KMD5SumFmtMakeUpdate ( & md5, f );
                    if ( rc != 0 )
                        KFileRelease ( f );
                }
            }

            if ( rc == 0 )
                rc = KDBManagerMakeTableUpdate ( self, tbl, wd, tblpath, convert, md5 );

            KMD5SumFmtRelease ( md5 );
        }
    }
    return rc;
}

rc_t KDBManagerVCreateTableInt_noargs ( KDBManager *self,
    KWTable **tbl, KDirectory *wd, KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateTableInt ( self, tbl, wd, cmode, path, args );
    va_end ( args );

    return rc;
}


LIB_EXPORT rc_t CC KDBManagerCreateTable ( KDBManager *self,
    KTable **tbl, KCreateMode cmode, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVCreateTable ( self, tbl, cmode, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVCreateTable ( KDBManager *self,
    KTable **tbl, KCreateMode cmode, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcParam, rcNull );

    * tbl = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcCreating, rcSelf, rcNull );

    return KDBManagerVCreateTableInt ( self, (KWTable**)tbl, self -> wd, cmode, path, args );
}

/* OpenMetadataUpdate
 *  open metadata for read/write
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
rc_t
KDBManagerOpenMetadataUpdateInt ( KDBManager *self, KWMetadata **metap, KDirectory *wd, KMD5SumFmt * md5 )
{
/* WAK
 * NEEDS MD5 UPDATE???
 */
    char metapath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
        metapath, sizeof metapath, "md/cur" );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KWMetadata *meta;
        bool populate = true;

        switch ( KDirectoryPathType ( wd, "%s", metapath ) )
        {
        case kptNotFound:
            populate = false;
            break;
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        }

        /* if already open */
        sym = KDBManagerOpenObjectFind (self, metapath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }

        rc = KWMetadataMake ( & meta, wd, metapath, 0, populate, false );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertMetadata (self, meta);
            if (rc == 0)
            {
                if ( md5 != NULL )
                {
                    meta -> md5 = md5;
                    rc = KMD5SumFmtAddRef ( md5 );
                }

                if ( rc == 0 )
                {
                    * metap = meta;
                    return 0;
                }
            }

            KMetadataRelease ( & meta -> dad );
        }
    }

    return rc;
}

rc_t KDBManagerInsertMetadata ( KDBManager * self, KWMetadata * meta )
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &meta->sym);
    if ( rc == 0 )
        meta -> mgr = KDBManagerAttach ( self );
    return rc;
}

/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
rc_t
KDBWManagerOpenMetadataReadInt ( KDBManager *self, const KWMetadata **metap, const KDirectory *wd, uint32_t rev, bool prerelease,bool *cached )
{
    char metapath [ 4096 ];
    rc_t rc = ( prerelease == 1 ) ?
        KDirectoryResolvePath_v1 ( wd, true, metapath, sizeof metapath, "meta" ):
        ( ( rev == 0 ) ?
          KDirectoryResolvePath_v1 ( wd, true, metapath, sizeof metapath, "md/cur" ):
          KDirectoryResolvePath ( wd, true, metapath, sizeof metapath, "md/r%.3u", rev ) );
    if(cached != NULL ) *cached = false;
    if ( rc == 0 )
    {
        KWMetadata * meta;
        KSymbol * sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind (self, metapath);
        if (sym != NULL)
        {
            const KWMetadata * cmeta;
            rc_t obj;

	    if(cached != NULL ) *cached = true;
            switch (sym->type)
            {
            case kptMetadata:
                cmeta = (KWMetadata*)sym->u.obj;
                /* if open for update, refuse */
                if ( cmeta -> read_only )
                {
                    /* attach a new reference and we're gone */
                    rc = KMetadataAddRef ( & cmeta -> dad );
                    if ( rc == 0 )
                        * metap = cmeta;
                    return rc;
                }
                obj = rcMetadata;
                break;

            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            }
            return  RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
	}


        switch ( KDirectoryPathType ( wd, "%s", metapath ) )
        {
        case kptNotFound:
            rc = RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcNotFound );
            break;
        case kptBadPath:
            rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
            break;
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
            break;
        }

        if ( rc == 0 )
        {
            rc = KWMetadataMake ( & meta, ( KDirectory* ) wd, metapath, rev, true, true );

            if ( rc == 0 )
            {
                rc = KDBManagerInsertMetadata (self, meta );
                if ( rc == 0 )
                {
                    * metap = meta;
                    return 0;
                }

                KMetadataRelease ( & meta -> dad );
            }

/*             rc = RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcExists ); */
        }
    }

    return rc;
}

rc_t KDBManagerInsertIndex ( KDBManager * self, KWIndex * idx)
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &idx->sym);
    if (rc == 0)
        idx->mgr = KDBManagerAttach (self);
    return rc;
}

/* OpenIndexRead
 * VOpenIndexRead
 *  open an index for read
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
rc_t KDBWManagerOpenIndexReadInt ( KDBManager *self,const KWIndex **idxp, const KDirectory *wd, const char *path )
{
    char idxpath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
                                      idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KWIndex *idx;
        KSymbol * sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind (self, idxpath);
        if (sym != NULL)
        {
            const KWIndex * cidx;
            rc_t obj;

            switch (sym->type)
            {
            case kptIndex:
                cidx = (const KWIndex *)sym->u.obj;
#if 0
                /* if open for update, refuse */
                if ( cidx -> read_only )
#endif
#if 0
                if (cidx is coherent)
#endif
                {
                    /* attach a new reference and we're gone */
                    rc = KIndexAddRef ( & cidx -> dad );
                    if ( rc == 0 )
                        * idxp = cidx;
                    return rc;
                }
                obj = rcDatabase;
                break;

            default:
                obj = rcPath;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return  RC (rcDB, rcMgr, rcOpening, obj, rcBusy);
        }

        switch ( KDirectoryPathType ( wd, "%s", idxpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcIndex, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        }

        rc = KWIndexMakeRead ( & idx, wd, idxpath );
        if ( rc == 0 )
        {
            //idx -> read_only = true; already done in KWIndexMakeRead
            rc = KDBManagerInsertIndex (self, idx);
            if ( rc == 0 )
            {
                * idxp = idx;
                return 0;
            }

            KIndexRelease ( & idx -> dad );
        }
    }

    return rc;
}

/* CreateIndex
 * VCreateIndex
 *  create a new or open an existing index
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "cmode" [ IN ] - creation mode
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
rc_t
KDBManagerCreateIndexInt ( KDBManager *self, KWIndex **idxp, KDirectory *wd, KIdxType type, KCreateMode cmode, const char *path, bool use_md5 )
{
    rc_t rc;
    int ptype;
    char idxpath [ 4096 ];

    rc = KDirectoryResolvePath ( wd, true,
                                  idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KWIndex *idx;
        switch ( ptype = KDBPathType ( wd, NULL, idxpath ) )
        {
        case kptNotFound:
            /* first good path */
            break;

        case kptBadPath:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcInvalid );

        case kptIndex:
        case kptIndex | kptAlias:
            /* found so is not good if we want to create new and not
             * clear/init or open old
             */
            if ((cmode & kcmValueMask) == kcmCreate)
                return RC ( rcDB, rcMgr, rcCreating, rcIndex, rcExists );
            if (KDBManagerOpenObjectBusy (self, idxpath))
                return RC ( rcDB, rcMgr, rcCreating, rcIndex, rcBusy );

            /* test now for locked file */
            rc = KDBWWritable ( wd, idxpath );
            if (rc)
            {
                switch (GetRCState(rc))
                {
                default:
                    return rc;
                case rcLocked:
                    return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcLocked );
                case rcReadonly:
                    return RC ( rcDB, rcMgr, rcCreating, rcDatabase, rcReadonly );
                case rcNotFound:
                    /* not found is good but probably unreachable */
                    break;
                case 0:
                    rc = 0;
                    break;
                }
            }
            /* second good path */
            break;

        case kptTable:
        case kptTable | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcTable, rcExists);

        case kptColumn:
        case kptColumn | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcColumn, rcExists);

        case kptDatabase:
        case kptDatabase | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcDatabase, rcExists);

        case kptMetadata:
        case kptMetadata | kptAlias:
            return RC (rcDB, rcMgr, rcCreating, rcMetadata, rcExists);

        default:
            return RC ( rcDB, rcMgr, rcCreating, rcPath, rcIncorrect );
        }

        rc = KWIndexCreate ( & idx, wd, type, cmode, path, ptype );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertIndex (self, idx);
            if ( rc == 0 )
            {
                idx -> use_md5 = use_md5;
                * idxp = idx;
                return 0;
            }

            KIndexRelease ( & idx -> dad );
        }
    }
    return rc;
}

/* OpenIndexUpdate
 * VOpenIndexUpdate
 *  open an index for read/write
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
rc_t KDBManagerOpenIndexUpdate ( KDBManager *self,
    KWIndex **idxp, KDirectory *wd, const char *path )
{
    char idxpath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
                                      idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KWIndex *idx;

        sym =  KDBManagerOpenObjectFind (self, idxpath);
        if (sym != NULL)
        {
            rc_t obj;
            switch (sym->type)
            {
            default:
                obj = rcPath;
                break;
            case kptDatabase:
                obj = rcDatabase;
                break;
            case kptTable:
                obj = rcTable;
                break;
            case kptColumn:
                obj = rcColumn;
                break;
            case kptIndex:
                obj = rcIndex;
                break;
            case kptMetadata:
                obj = rcMetadata;
                break;
            }
            return RC ( rcDB, rcMgr, rcOpening, obj, rcBusy );
        }
        /* only open existing indices
         * this seems wrong but its because the KDBPathType seems wrong
         */
        switch ( KDirectoryPathType ( wd, "%s", idxpath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcIndex, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        }

        rc = KWIndexMakeUpdate ( & idx, wd, path );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertIndex (self, idx);
            if ( rc == 0 )
            {
                * idxp = idx;
                return 0;
            }

            KIndexRelease ( & idx -> dad );
        }
    }
    return rc;
}
