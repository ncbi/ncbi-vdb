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
#include <kdb/table.h>
#include <kdb/kdb-priv.h>

#include "libkdb.vers.h"

#include <vfs/manager.h>
#include <vfs/manager-priv.h> /* VFSManagerOpenDirectoryReadDecryptUnreliable */
#include <vfs/resolver.h>
#include <vfs/path.h>
#include <vfs/path-priv.h>

#include <kfs/directory.h>

#define KONST const
#include "dbmgr.h"
#include "rkdb.h"
#include "kdb-cmn.h"
#include "rdatabase.h"
#include "kdbfmt.h"
#include "rtable.h"
#include "rcolumn.h"
#include "rindex.h"
#include "rdbmgr.h"
#include "rmeta.h"
#undef KONST

#include "../vfs/path-priv.h" /* VPathSetDirectory */

#include <klib/checksum.h>
#include <klib/rc.h>
#include <klib/printf.h>
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
static rc_t CC KDBRManagerVOpenDBRead ( const KDBManager *self,
    const KDatabase **db, const char *path, va_list args, const VPath *vpath );
static rc_t CC KDBRManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args );
static rc_t CC KDBRManagerOpenTableReadVPath ( const KDBManager *self, const KTable **tbl, const struct VPath *path );
static rc_t CC KDBRManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args );
static rc_t CC KDBRManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath );
static rc_t CC KDBRManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache );

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
    KDBRManagerVOpenDBRead,
    KDBRManagerVOpenTableRead,
    KDBRManagerOpenTableReadVPath,
    KDBRManagerVOpenColumnRead,
    KDBRManagerVPathOpenLocalDBRead,
    KDBRManagerVPathOpenRemoteDBRead,
    KDBCmnManagerGetVFSManager
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
    char dbpath [ 4096 ];

    /* get full path to object */
    rc_t rc = KDirectoryVResolvePath ( self -> wd, true, dbpath, sizeof dbpath, path, args );
    if ( rc == 0 )
    {
        int type = KDBPathType ( self -> wd, NULL, dbpath ) & ~ kptAlias;
        switch ( type )
        {
        case kptDatabase:
        case kptTable:
        case kptColumn:
        case kptIndex:
            rc = KDBRWritable ( self -> wd, dbpath );
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
    VPath * rpath = NULL;
    int path_type;
    rc_t rc;

    VFSManagerCheckEnvAndAd(self->vfsmgr, aPath, &path);

    path_type = kptBadPath;
    if ((self != NULL) && (path != NULL))
    {
        const KDirectory * dir = NULL;
        rc = VPathGetDirectory(aPath, &dir);
        if (rc != 0 || dir == NULL) {
          /*
           * resolve the possible relative path or accession into
           * a final path we can open directly
           */
          rc = KDBManagerResolveVPathInt (self, false, &rpath, path);
          if (rc == 0)
          {
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
            }
        }
        if (rc == 0) {
                path_type = KDBPathTypeDir (dir, kptDir, NULL, ".");
                VPathSetDirectory((VPath*)aPath, dir);
                KDirectoryRelease (dir);
        }
        /*
         * If we couldn't open the path as a directory we 'might'
         * have a KDB idx but we will only try that for a limited
         * set of uri schemes.
         */
         else {
#if 1
                if ( VPathIsFSCompatible ( rpath ) )
                {
                    char buffer [ 4096 ];
                    rc = VPathReadPath ( rpath, buffer, sizeof buffer, NULL );
                    if ( rc == 0 )
                        path_type = KDBPathType ( self -> wd, NULL, buffer );
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

/// @brief Get Path Contents, from VPath
static rc_t KDBRManagerPathContentsVP_1(const KDBManager *self, KDBContents const **result, int levelOfDetail, const VPath *vpath, KPathType type, const char *path, va_list args)
{
    KDirectory const *dir = NULL;
    rc_t rc = VFSManagerOpenDirectoryReadDecryptUnreliable(self->vfsmgr, &dir, vpath);
    if (rc == 0) {
        rc = KDBVGetPathContents(result, levelOfDetail, dir, type, path, args);
        KDirectoryRelease(dir);
    }
    return rc;
}

/// @brief copy va_list, make a VPath from copy, check env. and AD, and resolve it.
static VPath *makeResolvedVPath(const KDBManager *self, const char *path, va_list args, rc_t *prc)
{
    VPath *result = NULL;

    va_list copy;
    va_copy(copy, args);
    *prc = VFSManagerVMakePath(self->vfsmgr, &result, path, copy);
    va_end(copy);

    if (result != NULL) {
        VPath const *checked = NULL;
        if (VFSManagerCheckEnvAndAd(self->vfsmgr, result, &checked) && checked != NULL) {
            VPathRelease(result);
            result = (VPath *)checked;
        }
    }
    if (result != NULL) {
        VPath *resolved = NULL;
        *prc = KDBManagerResolveVPathInt(self, false, &resolved, result);
        if (resolved) {
            VPathRelease(result);
            result = resolved;
        }
    }
    return result;
}

static KPathType VGetPathType(const KDBManager *self, const char *path, va_list args)
{
    KPathType result = 0;
    va_list copy;
    va_copy(copy, args);
    result = KDirectoryVPathType(self->wd, path, copy);
    va_end(copy);
    return result;
}

/// @brief Get Path Contents, from va_list, unchecked
static rc_t KDBRManagerVPathContents_1(const KDBManager *self, KDBContents const **result, int levelOfDetail, const char *path, va_list args)
{
    rc_t rc = 0;
    VPath const *const vp = makeResolvedVPath(self, path, args, &rc); ///< NB. makes its own copy of args

    if (vp != NULL) {
        rc = KDBRManagerPathContentsVP_1(self, result, levelOfDetail, vp, VGetPathType(self, path, args), path, args);
        VPathRelease(vp);
    }
    return rc;
}

/// @brief Get Path Contents, from va_list
rc_t KDBRManagerVPathContents(const KDBManager *self, KDBContents const **result, int levelOfDetail, const char *path, va_list args)
{
    if (self == NULL)
        return RC(rcDB, rcMgr, rcAccessing, rcSelf, rcNull);
    if (path == NULL)
        return RC(rcDB, rcMgr, rcAccessing, rcParam, rcNull);
    return KDBRManagerVPathContents_1(self, result, levelOfDetail, path, args);
}

/// @brief Get Path Contents, from var_args
rc_t KDBRManagerPathContents(const KDBManager *self, KDBContents const **result, int levelOfDetail, const char *path, ...)
{
    rc_t rc = 0;
    va_list ap;
    va_start(ap, path);
    rc = KDBRManagerVPathContents(self, result, levelOfDetail, path, ap);
    va_end(ap);
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
rc_t KDBManagerVOpenDBReadInt ( const KDBManager *self, const KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, va_list args,
                                const VPath *vpath )
{
    bool tryEnvAndAd = true;
    rc_t rc = 0;

    /* MUST use vsnprintf because the documented behavior of "path"
       is that of stdc library's printf, not vdb printf */
    char aDbpath [ 4096 ] = "";
    char * dbpath = aDbpath;

    if ( vpath == NULL )
    {
      /* VDB-4386: cannot treat va_list as a pointer! */
      int z = 0;
      /*( args == NULL ) ?
        snprintf ( dbpath, sizeof dbpath, "%s", path ):*/
      if ( path != NULL )
        z = vsnprintf ( aDbpath, sizeof aDbpath, path, args );
      if ( z < 0 || ( size_t ) z >= sizeof aDbpath )
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    }

    if ( rc == 0 )
    {
        const KDirectory *dir;
        const VPath *path2 = NULL;

        {
            VPath *path = NULL;
            if (vpath == NULL)
                rc = VFSManagerMakePath(self->vfsmgr, &path, "%s", aDbpath);
            if (rc == 0) {
                const String * str = NULL;
                if (tryEnvAndAd)
                    VFSManagerCheckEnvAndAd(self->vfsmgr,
                        path != NULL ? path : vpath, &path2);
                if (path2 != NULL) {
                    rc = VPathMakeString(path2, &str);
                    if (rc == 0) {
                        assert(str);
                        dbpath = calloc(1, str->size + 1);
                        if (dbpath != NULL)
                            string_printf(dbpath, str->size + 1, NULL,
                                "%S", str);
                        StringWhack(str);
                    }
                }
                else {
                    rc = VPathMakeString(path != NULL ? path : vpath, &str);
                    if (rc == 0) {
                        assert(str);
                        dbpath = calloc(1, str->size + 1);
                        if (dbpath != NULL)
                            string_printf(dbpath, str->size + 1, NULL,
                                "%S", str);
                        StringWhack(str);
                    }
                }
                VPathRelease(path);
            }
        }

        /* open the directory if its a database */
        rc = KDBManagerOpenPathTypeRead ( self, wd, dbpath, &dir,
            kptDatabase, NULL, try_srapath, path2 != NULL ? path2 : vpath );

        if (vpath != NULL && path2 != NULL) {
            rc_t r = VPathCopyDirectoryIfEmpty((VPath*)vpath, path2);
            if (rc == 0 && r != 0)
                rc = r;
        }

        {
            rc_t r = VPathRelease(path2);
            if (rc == 0 && r != 0)
                rc = r;
            path2 = NULL;
        }

        if ( rc == 0 )
        {
            const KDatabase *db;

            /* allocate a new guy */
            rc = KRDatabaseMake ( & db, dir, dbpath, self );
            if ( rc == 0 )
            {
                * dbp = db;
                return 0;
            }

            KDirectoryRelease ( dir );
        }
    }
    if (aDbpath != dbpath)
        free(dbpath);

    return rc;
}

rc_t KDBRManagerVOpenDBReadInt_noargs ( const KDBManager *self, const KDatabase **dbp,
                                const KDirectory *wd, bool try_srapath,
                                const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenDBReadInt ( self, dbp, wd, try_srapath, path, args,
        NULL );
    va_end ( args );

    return rc;
}

static
rc_t CC
KDBRManagerVOpenDBRead ( const KDBManager *self, const KDatabase **db,
    const char *path, va_list args, const VPath *vpath )
{
    if ( db == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * db = NULL;

    return KDBManagerVOpenDBReadInt ( self, db, self -> wd, true, path, args,
        vpath );
}

static
rc_t CC
KDBRManagerVPathOpenLocalDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * vpath )
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
            {   /* allocate a new guy */
                const KDatabase *db;
                rc = KRDatabaseMakeVPath ( & db, dir, vpath, self );
                if ( rc == 0 )
                {
                    * p_db = db;
                    return 0;
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
}

static
rc_t CC
KDBRManagerVPathOpenRemoteDBRead ( struct KDBManager const * self, struct KDatabase const ** p_db, struct VPath const * remote, struct VPath const * cache )
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
            {   /* allocate a new guy */
                const KDatabase *db;
                rc = KRDatabaseMakeVPath ( & db, dir, remote, self );
                if ( rc == 0 )
                {
                    * p_db = db;
                    return 0;
                }
            }

            KDirectoryRelease ( dir );
        }
        return rc;
    }
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
rc_t KDBManagerVOpenTableReadInt ( const KDBManager *self,
    const KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *p_path, va_list args, const struct VPath *vpath,
    bool tryEnvAndAd )
{
    rc_t rc = 0;
    char aTblpath[4096] = "";
    char * tblpath = aTblpath;
    int z = 0;
    /*VDB-4386: cannot treat va_list as a pointer!*/
/*    if (args == NULL) {
        if (path != NULL)
            z = snprintf(aTblpath, sizeof aTblpath, "%s", path);
    }
    else*/
    if (p_path != NULL)
        z = vsnprintf ( aTblpath, sizeof aTblpath, p_path, args );
    if ( z < 0 || ( size_t ) z >= sizeof aTblpath )
        rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcExcessive );
    else
    {
        const KRTable *tbl;
        const KDirectory *dir;
        bool prerelease = false;
        const VPath *path2 = NULL;

        {
            VPath *path = NULL;
            if (vpath == NULL)
                rc = VFSManagerMakePath(self->vfsmgr, &path, "%s", aTblpath);
            if (rc == 0) {
                const String * str = NULL;
                if (tryEnvAndAd)
                    VFSManagerCheckEnvAndAd(self->vfsmgr,
                        path != NULL ? path : vpath, &path2);
                if (path2 != NULL) {
                    rc = VPathMakeString(path2, &str);
                    if (rc == 0) {
                        assert(str);
                        tblpath = calloc(1, str->size + 1);
                        if (tblpath != NULL)
                            string_printf(tblpath, str->size + 1, NULL,
                                "%S", str);
                        StringWhack(str);
                    }
                }
                else {
                    rc = VPathMakeString(path != NULL ? path : vpath, &str);
                    if (rc == 0) {
                        assert(str);
                        tblpath = calloc(1, str->size + 1);
                        if (tblpath != NULL)
                            string_printf(tblpath, str->size + 1, NULL,
                                "%S", str);
                        StringWhack(str);
                    }
                }
                VPathRelease(path);
            }
        }

        rc = KDBManagerOpenPathTypeRead(self, wd, tblpath, &dir, kptTable, NULL,
            try_srapath, path2 != NULL ? path2 : vpath);
        if (rc != 0)
        {
            prerelease = true;
            rc = KDBManagerOpenPathTypeRead(self, wd, tblpath, &dir,
                kptPrereleaseTbl, NULL,
                try_srapath, path2 != NULL ? path2 : vpath);
        }

        if (vpath != NULL && path2 != NULL) {
            rc_t r = VPathCopyDirectoryIfEmpty((VPath*)vpath, path2);
            if (rc == 0 && r != 0)
                rc = r;
        }

        {
            rc_t r = VPathRelease(path2);
            if (rc == 0 && r != 0)
                rc = r;
            path2 = NULL;
        }

        if (rc == 0)
        {
          /*String str;
            const char * p = tblpath;
            if (p == NULL) {
                if (path2 != NULL) {
                    rc = VPathGetPath(vpath, &str);
                    if (rc == 0)
                        p = str.addr;
                }
                else if (vpath != NULL) {
                    rc = VPathGetPath(vpath, &str);
                    if (rc == 0)
                        p = str.addr;
                }
            }*/

            rc = KRTableMake ( & tbl, dir, tblpath, self, prerelease );
            if ( rc == 0 )
            {
                * tblp = & tbl -> dad;

                if (aTblpath != tblpath)
                    free(tblpath);

                return 0;
            }
            KDirectoryRelease ( dir );
        }
    }

    if (aTblpath != tblpath)
        free(tblpath);

    return rc;
}

rc_t KDBRManagerVOpenTableReadInt_noargs ( const KDBManager *self,
    const KTable **tblp, const KDirectory *wd, bool try_srapath,
    const char *path, bool tryEnvAndAd, const struct VPath *vpath,
    ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, vpath );
    rc = KDBManagerVOpenTableReadInt ( self, tblp, wd, try_srapath, path, args, vpath, tryEnvAndAd );
    va_end ( args );

    return rc;
}

static
rc_t CC
KDBRManagerVOpenTableRead ( const KDBManager *self, const KTable **tbl, const char *path, va_list args )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    return KDBManagerVOpenTableReadInt ( self, tbl, self -> wd, true, path,
        args, NULL, true );
}

static
rc_t CC
KDBRManagerOpenTableReadVPath ( const KDBManager *self, const KTable **tbl, const struct VPath *path )
{
    if ( tbl == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * tbl = NULL;

    return KDBRManagerVOpenTableReadInt_noargs ( self, tbl, self->wd, true, "",
        true, path );
}



/* OpenColumnRead
 * VOpenColumnRead
 *  open a column for read
 *
 *  "col" [ OUT ] - return parameter for newly opened column
 *
 *  "path" [ IN ] - NUL terminated string in UTF-8 giving path to col
 *  where "." acts as a structure name separator, i.e. struct.member
 */
static
rc_t KDBManagerVOpenColumnReadInt ( const KDBManager *self,
    const KColumn **colp, const KDirectory *wd, bool try_srapath,
    const char *path, va_list args )
{
    char colpath [ 4096 ];
    rc_t rc;
    size_t z;

/*    rc = KDirectoryVResolvePath ( wd, 1,
        colpath, sizeof colpath, path, args ); */
    rc = string_vprintf( colpath, sizeof colpath, &z, path, args );
    if ( rc == 0 )
    {
        KRColumn *col;
        const KDirectory *dir;

        /* open table directory */
        rc = KDBManagerOpenPathTypeRead ( self, wd, colpath, &dir, kptColumn, NULL,
            try_srapath, NULL );
        if ( rc == 0 )
        {
            rc = KRColumnMakeRead ( & col, dir, colpath );
            if ( rc == 0 )
            {
                col -> mgr = KDBManagerAttach ( self );
                * colp = & col -> dad;
                return 0;
            }

            KDirectoryRelease ( dir );
        }
    }

    return rc;
}

rc_t KDBRManagerVOpenColumnReadInt_noargs ( const KDBManager *self,
    const KColumn **colp, const KDirectory *wd, bool try_srapath,
    const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KDBManagerVOpenColumnReadInt ( self, colp, wd, try_srapath, path, args );
    va_end ( args );

    return rc;
}

static
rc_t CC
KDBRManagerVOpenColumnRead ( const KDBManager *self, const KColumn **col, const char *path, va_list args )
{
    if ( col == NULL )
        return RC ( rcDB, rcMgr, rcOpening, rcParam, rcNull );

    * col = NULL;

    return KDBManagerVOpenColumnReadInt
        ( self, col, self -> wd, true, path, args );
}



/* OpenMetadataRead
 *  opens metadata for read
 *
 *  "meta" [ OUT ] - return parameter for metadata
 */
rc_t KDBRManagerOpenMetadataReadInt ( const KDBManager *self, KRMetadata **metap, const KDirectory *wd, uint32_t rev, bool prerelease )
{
    char metapath [ 4096 ];
    rc_t rc = ( prerelease == 1 ) ?
        KDirectoryResolvePath_v1 ( wd, true, metapath, sizeof metapath, "meta" ):
        ( ( rev == 0 ) ?
          KDirectoryResolvePath_v1 ( wd, true, metapath, sizeof metapath, "md/cur" ):
          KDirectoryResolvePath ( wd, true, metapath, sizeof metapath, "md/r%.3u", rev ) );
    if ( rc == 0 )
    {
        KRMetadata *meta;

        switch ( KDirectoryPathType ( wd, "%s", metapath ) )
        {
        case kptNotFound:
            return RC ( rcDB, rcMgr, rcOpening, rcMetadata, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcInvalid );
        case kptFile:
        case kptFile | kptAlias:
            break;
        default:
            return RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
        }

        rc = KRMetadataMakeRead ( & meta, wd, metapath, rev );
        if ( rc == 0 )
        {
            meta -> mgr = KDBManagerAttach ( self );
            * metap = meta;
            return 0;
        }
    }

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
rc_t KDBRManagerOpenIndexReadInt ( const KDBManager *self, KRIndex **idxp, const KDirectory *wd, const char *path )
{
    char idxpath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
                                      idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KRIndex *idx;

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

        rc = KRIndexMakeRead ( & idx, wd, idxpath );
        if ( rc == 0 )
        {
            idx -> mgr = KDBManagerAttach ( self );
            * idxp = idx;
            return 0;
        }
    }

    return rc;
}
