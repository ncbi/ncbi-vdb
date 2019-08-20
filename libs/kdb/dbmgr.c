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
#include "kdbfmt-priv.h"
#include <klib/checksum.h>
#include <klib/rc.h>
#undef KONST

#include <klib/text.h>

#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <assert.h>



/*--------------------------------------------------------------------------
 * KDBManager
 *  handle to library
 */


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
    return KDBManagerMake ( ( KDBManager** ) mgrp, wd, "make-read", vmanager );
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

LIB_EXPORT rc_t CC KDBManagerWritable ( const KDBManager *self, const char * path, ... )
{
    rc_t rc;

    va_list args;
    va_start ( args, path );

    rc = KDBManagerVWritable ( self, path, args );

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

static void ad(const KDBManager *self, const VPath *aPath, const VPath **path)
{
    String spath;
    const char *slash;
    assert(self);
    if (VPathGetPath(aPath, &spath) != 0)
        return;
    if ((KDirectoryPathType(self->wd, spath.addr) & ~kptAlias) != kptDir)
        return;
    slash = strrchr(spath.addr, '/');
    if (slash)
        ++slash;
    else
        slash = spath.addr;
    if ((KDirectoryPathType(self->wd, "%s/%s.sra", spath.addr, slash)
        & ~kptAlias) != kptFile)
    {
        return;
    }
    VFSManagerMakePath(self->vfsmgr, (VPath **)path,
        "%s/%s.sra", spath.addr, slash);
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

    ad(self, aPath, &path);

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

LIB_EXPORT
int CC KDBManagerPathTypeVP ( const KDBManager * self, const VPath * path )
{
    return KDBManagerPathTypeVPImpl ( self, path, true );
}

static int CC KDBManagerVPathTypeImpl ( const KDBManager * self,
    const char *path, va_list args, bool reliable )
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
    int res;
    va_list args;

    va_start ( args, path );

    res = KDBManagerVPathType ( self, path, args );

    va_end (args);
    return res;
}
