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

#include <kdb/database.h>
#include <kdb/kdb-priv.h> /* KDBManagerMakeReadWithVFSManager */

#include "libkdb.vers.h"

#include <vfs/manager.h>
#include <vfs/manager-priv.h> /* VFSManagerOpenDirectoryReadDecryptUnreliable */
#include <vfs/resolver.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>
#include <kfs/directory.h>

#define KONST const
#include "dbmgr-priv.h"
#include "kdb-priv.h"
#include "rdatabase.h"
#include "kdbfmt-priv.h"
#include <klib/checksum.h>
#include <klib/rc.h>
#undef KONST

#include <klib/text.h>

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/*--------------------------------------------------------------------------
 * KDBManager
 *  handle to library
 */

static rc_t CC KDBRManagerVWritable ( const KDBManager *self, const char * path, va_list args );
static rc_t CC KDBRManagerRunPeriodicTasks ( const KDBManager *self );
static int CC KDBRManagerPathTypeVP ( const KDBManager * self, const VPath * path );
static int CC KDBRManagerVPathType ( const KDBManager * self, const char *path, va_list args );
static int CC KDBRManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args );

static KDBManager_vt KDBRManager_vt =
{
    KDBManagerWhack,
    KDBManagerBaseAddRef,
    KDBManagerBaseRelease,
    KDBManagerCommonVersion,
    KDBManagerCommonVExists,
    KDBRManagerVWritable,
    KDBRManagerRunPeriodicTasks,
    KDBRManagerPathTypeVP,
    KDBRManagerVPathType,
    KDBRManagerVPathTypeUnreliable,
};

/* MakeRead
 * MakeReadWithVFSManager
 *  create library handle for specific use
 *  NB - only one of the functions will be implemented
 *
 *  "wd" [ IN, NULL OKAY ] - optional working directory for
 *  accessing the file system. mgr will attach its own reference.
 */
LIB_EXPORT rc_t CC KDBManagerMakeRead ( const KDBManager **mgrp, const KDirectory *wd )
{
    return KDBManagerMakeReadWithVFSManager(mgrp, wd, NULL);
}

LIB_EXPORT rc_t CC KDBManagerMakeReadWithVFSManager ( const KDBManager **mgrp,
    const KDirectory *wd, struct VFSManager *vmanager )
{
    return KDBManagerMake ( ( KDBManager** ) mgrp, wd, "make-read", vmanager, & KDBRManager_vt );
}

/*
 * Resolve using manager, possibly against this directory, using or not uri
 * accession resolution of those dangerous formatted path thingies
 *



 * 1. If naked accession or uri accession resolve to local,


 * self                         = a kdbmanager
 * disable_accession_resolution = turn off VResolver usage for accessions
 *                                read versus create/upate
 * resolved+path                = a vpath created based on text path
 * fmt                          = our scary interface that is 'sprintf'ish
 * args                         = goes with the fmt
 *
 * NOTE: as usual a path with a '%' becomes broken at unsuspected times
 */
rc_t KDBManagerResolveVPathInt (const KDBManager * self,
                                bool disable_accession_resolution,
                                VPath ** resolved_path,
                                const VPath * path)
{
    uint32_t flags;

    assert (self != NULL);
    assert (resolved_path != NULL);
    assert (path != NULL);

    flags = disable_accession_resolution
        ? vfsmgr_rflag_no_acc
        : vfsmgr_rflag_kdb_acc;

    return VFSManagerResolvePath (self->vfsmgr,
                                  flags,
                                  path, resolved_path);
}


rc_t KDBManagerVResolveVPath (const KDBManager * self,
                                bool disable_accession_resolution,
                                VPath ** resolved_path,
                                const VPath * path)
{
    return KDBManagerResolveVPathInt (self, disable_accession_resolution,
                                      resolved_path, path);
}


rc_t KDBManagerVResolvePath (const KDBManager * self,
                             bool disable_accession_resolution,
                             VPath ** resolved_path,
                             const char * fmt, va_list args)
{
    VPath * p;
    rc_t rc;

    if (resolved_path == NULL)
        return RC (rcDB, rcMgr, rcResolving, rcParam, rcNull);

    *resolved_path = NULL;

    if (self == NULL)
        return RC (rcDB, rcMgr, rcResolving, rcSelf, rcNull);

    if ((fmt == NULL) || (fmt[0] == '\0'))
        return RC (rcDB, rcMgr, rcResolving, rcParam, rcNull);

    rc = VFSManagerVMakePath ( self -> vfsmgr, &p, fmt, args);
    if (rc == 0)
    {
        rc = KDBManagerVResolveVPath (self, disable_accession_resolution,
                                      resolved_path, p);
        VPathRelease (p);
    }
    return rc;
}

rc_t KDBManagerVResolvePathRelativeDir (const KDBManager * self, const KDirectory * dir,
                                        bool disable_accession_resolution,
                                        VPath ** resolved_path,
                                        const char * fmt, va_list args)
{
    VPath * p;
    rc_t rc;

    if (resolved_path == NULL)
        return RC (rcDB, rcMgr, rcResolving, rcParam, rcNull);

    *resolved_path = NULL;

    if (self == NULL)
        return RC (rcDB, rcMgr, rcResolving, rcSelf, rcNull);

    if ((fmt == NULL) || (fmt[0] == '\0'))
        return RC (rcDB, rcMgr, rcResolving, rcParam, rcNull);

    rc = VFSManagerVMakePath ( self -> vfsmgr, &p, fmt, args);
    if (rc == 0)
    {
        rc = KDBManagerVResolveVPath (self, disable_accession_resolution,
                                      resolved_path, p);
        VPathRelease (p);
    }
    return rc;
}


rc_t KDBManagerResolvePathRelativeDir (const KDBManager * self,
                                       const KDirectory * dir,
                                       bool disable_accession_resolution,
                                       VPath ** resolved_path, const char * fmt, ...)
{
    va_list args;
    rc_t rc;

    va_start (args, fmt);

    rc = KDBManagerVResolvePathRelativeDir (self, dir,
                                            disable_accession_resolution,
                                            resolved_path, fmt, args);
    va_end (args);
    return rc;
}



/* KDBHdrValidate
 *  validates that a header sports a supported byte order
 *  and that the version is within range
 */
rc_t KDBHdrValidate ( const KDBHdr *hdr, size_t size,
    uint32_t min_vers, uint32_t max_vers )
{
    assert ( hdr != NULL );

    if ( size < sizeof * hdr )
        return RC ( rcDB, rcHeader, rcValidating, rcData, rcCorrupt );

    if ( hdr -> endian != eByteOrderTag )
    {
        if ( hdr -> endian == eByteOrderReverse )
            return RC ( rcDB, rcHeader, rcValidating, rcByteOrder, rcIncorrect );
        return RC ( rcDB, rcHeader, rcValidating, rcData, rcCorrupt );
    }

    if ( hdr -> version < min_vers || hdr -> version > max_vers )
        return RC ( rcDB, rcHeader, rcValidating, rcHeader, rcBadVersion );

    return 0;
}



/* Writable
 *  returns 0 if object is writable
 *  or a reason why if not
 *
 *  "path" [ IN ] - NUL terminated path
 */
static
rc_t CC
KDBRManagerVWritable ( const KDBManager *self, const char * path, va_list args )
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
        {
            int type = KDBPathType ( self -> wd, NULL, path ) & ~ kptAlias;
            switch ( type )
            {
            case kptDatabase:
            case kptTable:
            case kptColumn:
            case kptIndex:
                rc = KDBWritable ( self -> wd, path );
                break;
            case kptNotFound:
                rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcNotFound );
                break;
            case kptBadPath:
                rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcInvalid );
                break;
            default:
                rc = RC ( rcDB, rcMgr, rcAccessing, rcPath, rcIncorrect );
            }
        }
    }
    return rc;
}

/* RunPeriodicTasks
 *  executes periodic tasks, such as cache flushing
 */
static rc_t CC KDBRManagerRunPeriodicTasks ( const KDBManager *self )
{
    return 0;
}

/* PathType
 *  check the path type of an object/directory path.
 *  this is an extension of the KDirectoryPathType and will return
 *  the KDirectory values if a path type is not specifically a
 *  kdb object
 */
static int CC KDBManagerPathTypeVPImpl ( const KDBManager * self,
    const VPath * aPath, bool reliable )
{
    const VPath * path = aPath;
    VPath * rpath;
    int path_type;
    rc_t rc;

    VFSManagerCheckEnvAndAd(self->vfsmgr, aPath, &path);

    path_type = kptBadPath;
    if ((self != NULL) && (path != NULL))
    {
        /*
         * resolve the possible relative path or accession into
         * a final path we can open directly
         */
        rc = KDBManagerResolveVPathInt (self, false, &rpath, path);
        if (rc == 0)
        {
            const KDirectory * dir;

            /*
             * Most KDBPathType values are based on 'directories'
             * so try to open the resolved path as a directory
             */
            if ( reliable )
                rc = VFSManagerOpenDirectoryReadDecrypt           (self->vfsmgr,
                    &dir, rpath);
            else
                rc = VFSManagerOpenDirectoryReadDecryptUnreliable (self->vfsmgr,
                    &dir, rpath);
            if (rc == 0)
            {
                path_type = KDBPathTypeDir (dir, kptDir, NULL, ".");
                KDirectoryRelease (dir);
            }
            /*
             * If we couldn't open the path as a directory we 'might'
             * have a KDB idx but we will only try that for a limited
             * set of uri schemes.
             */
            else
            {
#if 1
                if ( VPathIsFSCompatible ( rpath ) )
                {
                    char buffer [ 4096 ];
                    rc = VPathReadPath ( rpath, buffer, sizeof buffer, NULL );
                    if ( rc == 0 )
                        path_type = KDBPathType ( self -> wd, false, buffer );
                }
#else
                VPUri_t t;

                rc = VPathGetScheme_t (rpath, &t);
                if (rc == 0)
                {
                    switch (t)
                    {
                    default:
                        break;
                    case vpuri_ncbi_vfs:
                    case vpuri_file:
                    case vpuri_none:
                    {
                        char b [4 * 1024];
                        size_t z;
                        rc = VPathReadPath (path, b, sizeof b, &z);
                        if (rc == 0)
                            path_type = KDBPathType (self->wd, false, b);
                        break;
                    }}
                }
#endif
            }
            VPathRelease (rpath);
        }
    }

    if (aPath != path)
        VPathRelease(path);

    return path_type;
}

static
int CC
KDBRManagerPathTypeVP ( const KDBManager * self, const VPath * path )
{
    return KDBManagerPathTypeVPImpl ( self, path, true );
}

static
int CC
KDBManagerVPathTypeImpl ( const KDBManager * self, const char *path, va_list args, bool reliable )
{
    int path_type = kptBadPath;

    if ((self != NULL) && (path != NULL))
    {
        VPath * vp;
        rc_t rc;

        rc = VFSManagerVMakePath ( self -> vfsmgr, &vp, path, args);
        if (rc == 0)
        {
            path_type = KDBManagerPathTypeVPImpl (self, vp, reliable);
            VPathRelease (vp);
        }
    }
    return path_type;
}

static
int CC
KDBRManagerVPathType ( const KDBManager * self, const char *path, va_list args )
{
    return KDBManagerVPathTypeImpl ( self, path, args, true );
}

static
int CC
KDBRManagerVPathTypeUnreliable ( const KDBManager * self, const char *path, va_list args )
{
    return KDBManagerVPathTypeImpl ( self, path, args, false );
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
rc_t KDBManagerVOpenDBReadInt ( const KDBManager *self, const KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, va_list args )
{
    rc_t rc;

    /* MUST use vsnprintf because the documented behavior of "path"
       is that of stdc library's printf, not vdb printf */
    char dbpath [ 4096 ];
    /* VDB-4386: cannot treat va_list as a pointer! */
    int z = 0;
    /*( args == NULL ) ?
        snprintf ( dbpath, sizeof dbpath, "%s", path ):*/
    if ( path != NULL )
        z = vsnprintf ( dbpath, sizeof dbpath, path, args );
    if ( z < 0 || ( size_t ) z >= sizeof dbpath )
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    else
    {
        const KDirectory *dir;

        /* open the directory if its a database */
        rc = KDBOpenPathTypeRead ( self, wd, dbpath, &dir, kptDatabase, NULL,
            try_srapath, NULL );
        if ( rc == 0 )
        {
            KDatabase *db;

            /* allocate a new guy */
            rc = KDatabaseMake ( & db, dir, dbpath );
            if ( rc == 0 )
            {
                db -> mgr = KDBManagerAttach ( self );
                * dbp = db;
                return 0;
            }

            KDirectoryRelease ( dir );
        }
    }
    return rc;
}

rc_t KDBManagerVOpenDBReadInt_noargs ( const KDBManager *self, const KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBReadInt ( self, dbp, wd, try_srapath, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDBManagerVOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, va_list args )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcSelf, rcNull );

    return KDBManagerVOpenDBReadInt ( self, db, self -> wd, true, path, args );
}
