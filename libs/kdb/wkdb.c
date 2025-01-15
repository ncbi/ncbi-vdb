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

#include <kdb/extern.h>
#include <kdb/kdb-priv.h>

#include "wkdb.h"
#include "kdb-cmn.h"
#include "kdbfmt.h"
#include "dbmgr.h"

#include <vfs/manager.h>
#include <vfs/path.h>
#include <vfs/manager-priv.h>

#include <kfs/directory.h>
#include <kfs/file.h>

#include <klib/rc.h>

#include <krypto/wgaencrypt.h>
#include <krypto/encfile.h>
#include <krypto/key.h>

#include <stdio.h>

#ifndef SUPPORT_VFS_URI
#define SUPPORT_VFS_URI 0
#endif

#ifndef SUPPORT_WKDB_TAR
#define SUPPORT_WKDB_TAR 0
#endif

/*--------------------------------------------------------------------------
 * (W)KDB utility
 */

const char *KDBGetNamespaceString ( int namespace )
{
    static const char * ns_array [] = { NULL, "db", "tbl", "col", "idx", "md" };

    switch (namespace)
    {
    default:
        return ns_array [0];
    case kptDatabase:
        return ns_array [1];
    case kptTable:
        return ns_array [2];
    case kptColumn:
        return ns_array [3];
    case kptIndex:
        return ns_array [4];
    case kptMetadata:
        return ns_array [5];
    }
}

#if SUPPORT_VFS_URI
#else
/* return configured password as ASCIZ
 * opertates on vfs/kfs/kfg objects, not kdb objects */
static
rc_t KDBOpenFileGetPassword (char * pw, size_t pwz)
{
    VFSManager * mgr;
    rc_t rc;

    assert (pw);
    assert (pwz);

    pw[0] = '\0';

    rc = VFSManagerMake (&mgr);
    if (rc)
        ;                      /* failure to make VFS manager: pass along rc */
    else
    {
        size_t pwfz;
        char pwf [4096 + 1];

        rc = VFSManagerGetConfigPWFile (mgr, pwf, sizeof (pwf) - 1, &pwfz);
        if (rc)
            /* failure to get password file path: tweak rc */
            rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcNotFound);

        else
        {
            VPath * pwp;

            pwf [pwfz] = '\0'; /* force to ASCIZ */

#if 0
            rc = VPathMakeSysPath (&pwp, pwf);
#else
            rc = VFSManagerMakePath (mgr, &pwp, "%s", pwf);
#endif

            if (rc)
                ;       /* failure to construct a path from the string */

            else
            {
                const KFile * pwf;

                rc = VFSManagerOpenFileRead (mgr, &pwf, pwp);
                if (rc)
                    /* failure to open password file */
                    rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcNotOpen);

                else
                {
                    size_t z;
                    char pwb [4098]; /* arbitrarily using 4096 as maximum
                                        allowed length */

                    /* at this point we are only getting the password from a
                     * file but in the future if we can get it from a pipe of
                     * some sort we can't count on the ReadAll to really know
                     * if we hit end of file and not just a pause in the
                     * streaming.  VFS/KFS 2 will have to fix this somehow
                     */

                    rc = KFileReadAll (pwf, 0, pwb, sizeof pwb, &z);
                    if (rc)
                        ;       /* failure to read password file: pass along rc */
                    else
                    {
                        /* trim off EOL if present */
                        char * pc;

                        pwb[z] = '\0';   /* force ASCIZ */

                        pc = string_chr (pwb, z, '\r');
                        if (pc)
                        {
                            *pc = '\0';
                            z = 1 + pc - pwb;
                        }
                        pc = string_chr (pwb, z, '\n');
                        if (pc)
                        {
                            *pc = '\0';
                            z = 1 + pc - pwb;
                        }
                        if (z == 0)
                            rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcTooShort);

                        else if (pwz < z) /* pwz came in as 4096 */
                            rc = RC (rcDB, rcMgr, rcOpening, rcEncryptionKey, rcTooLong);

                        else
                        {
                            memmove (pw, pwb, z+1);
                        }
                    }
                    KFileRelease (pwf);
                }
                VPathRelease (pwp);
            }
        }
        VFSManagerRelease (mgr);
    }
    return rc;
}


/* not KDB specific - just uses vfs/krypto/kfs objects */
static
rc_t KDBOpenFileAsDirectory (const KDirectory * dir,
                             const char * path,
                             const KDirectory ** pdir,
                             uint32_t rcobj)
{
    const KFile * file;
    const KFile * f;
    const KDirectory * ldir;
    bool encrypted = false;

    rc_t rc;

    *pdir = NULL;

    rc = KDirectoryOpenFileRead (dir, &file, "%s", path);
    if (rc == 0)
    {
        rc = KFileRandomAccess(file);
        if (rc)
            rc = RC (rcDB, rcMgr, rcOpening, rcobj, rcUnsupported);
        else
        {
            size_t tz;
            char tbuff [4096];
            char pbuff [4096 + 1];

            rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
            if (rc == 0)
            {
                if (KFileIsEnc (tbuff, tz) == 0)
                {
                    encrypted = true;

                    rc = KDBOpenFileGetPassword (pbuff, sizeof (pbuff) - 1);
                    if (rc == 0)
                    {
                        KKey key;

                        rc = KKeyInitRead (&key, kkeyAES128, pbuff, string_size (pbuff));
                        if (rc == 0)
                        {
                            rc = KEncFileMakeRead (&f, file, &key);
                            if (rc == 0)
                            {
                                /* KEncFileMakeRead adds a reference */
                                KFileRelease (file);
                                file = f;
                                rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
                            }
                        }
                    }
                }
                else if (KFileIsWGAEnc (tbuff, tz) == 0)
                {
                    encrypted = true;

                    rc = KDBOpenFileGetPassword (pbuff, sizeof (pbuff) - 1);
                    if (rc == 0)
                    {
                        rc = KFileMakeWGAEncRead (&f, file, pbuff, string_size (pbuff));
                        if (rc == 0)
                        {
                            /* KFileMakeWGAEncRead adds a reference */
                            KFileRelease (file);
                            file = f;
                            rc = KFileReadAll (file, 0, tbuff, sizeof tbuff, &tz);
                        }
                    }
                }
                /* else not a handled encryption or unencrypted: we can't distinguish too much */

                if (rc == 0)
                {
                    if (KFileIsSRA (tbuff, tz) == 0)
                    {
                        rc = KDirectoryOpenSraArchiveReadUnbounded_silent_preopened (dir,
                                                                                     &ldir,
                                                                                     false,
                                                                                     file,
                                                                                     "%s",
                                                                                     path);
                    }
                    else
                    {
#if SUPPORT_WKDB_TAR
                        rc = KDirectoryOpenTarArchiveRead_silent_preopened (dir, &ldir, false,
                                                                            file, "%s", path);
                        if (rc == 0)
                            KFileRelease (file);
#else
                        /* will be reset immediately below */
                        rc = -1;
#endif
                    }

                    /* not an archive type we handle or a bad archive */
                    if (rc)
                    {
                        if (encrypted)
                            rc = RC ( rcDB, rcMgr, rcOpening, rcEncryptionKey, rcIncorrect );
                        else
                            rc = RC ( rcDB, rcMgr, rcOpening, rcPath, rcIncorrect );
                    }
                    else
                    {
                        /*
                         * release our ownership of the KFile that but archive will
                         * keep theirs
                         */
                        KFileRelease (file);
                        *pdir = ldir;
                        return 0;
                    }
                }
            }
        }
        KFileRelease (file);
    }
    return rc;
}
#endif


static rc_t KDBOpenPathTypeReadInt ( const KDBManager * mgr, const KDirectory * dir, const char * path,
        const KDirectory ** pdir, int * type,
        int pathtype, uint32_t rcobj, bool try_srapath, const VPath * aVpath )
{
    VFSManager * vmgr = mgr->vfsmgr;
    const KDirectory * ldir = NULL;
    rc_t rc = 0;

    /* object relative opens can be done using KFS - we hacked in VFS after all */
    if (! try_srapath)
    {
        rc = KDirectoryOpenDirUpdate ((KDirectory*)dir, (KDirectory**)pdir, false, "%s", path);
        if ((rc) && (GetRCState(rc) != rcNotFound))
            rc = KDirectoryOpenDirRead (dir, pdir, false, "%s", path);
    }
    else
    {
        VPath * vpath = ( VPath * ) aVpath;

        /*
         * We've got to decide if the path coming in is a full or relative
         * path and if relative make it relative to dir or possibly its a srapath
         * accession
         *
         */
        rc = VPathMakeDirectoryRelativeVPath ( &vpath, dir, path, vpath);
        if ( rc == 0 )
        {
            rc = VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt ( vmgr, dir, &ldir, vpath );

            if ( rc == 0 )
            {
                *type = (~kptAlias) & KDBPathType ( ldir, NULL, "." );

                /* just a directory, not a kdb type */
                if ( *type == kptDir )
                    rc = RC (rcDB, rcMgr, rcOpening, rcPath, rcIncorrect);

                else if ( *type != pathtype )
                {
                    KDirectoryRelease( ldir );
                    rc = RC ( rcDB, rcMgr, rcOpening, rcobj, rcIncorrect );
                }
                else
                {
                    if ( pdir != NULL )
                        *pdir = ldir;
                    else
                        KDirectoryRelease( ldir );
                }
            }
            if ( aVpath == NULL )
                VPathRelease ( vpath );
        }
    }
    return rc;
}

rc_t KWDBManagerOpenPathTypeRead ( const KDBManager * mgr, const KDirectory * dir, const char * path,
    const KDirectory ** pdir, int pathtype, int * ppathtype, bool try_srapath,
    const VPath * vpath )
{
    const KDirectory *ldir;
    rc_t rc = 0;
    uint32_t rcobj;
    int type = kptNotFound; /* bogus? */

/*     KOutMsg ("%s: %s\n", __func__, path); */

    if ( pdir != NULL )
        *pdir = NULL;
    if ( ppathtype != NULL )
        *ppathtype = type;

    switch (pathtype & ~ kptAlias) /* tune the error message based on path type */
    {
        /* we'll hit this if we don't track defines in kdb/manager.h */
    default:
        rc = RC (rcDB, rcMgr, rcOpening, rcType, rcInvalid);
        return rc;

    case kptTable:
    case kptPrereleaseTbl:
        rcobj = rcTable;
        break;

    case kptColumn:
        rcobj = rcColumn;
        break;

    case kptDatabase:
    case kptDatabase | kptAlias:
        rcobj = rcDatabase;
        break;
    }

    rc = KDBOpenPathTypeReadInt( mgr, dir, path, &ldir, &type, pathtype, rcobj, try_srapath, vpath );

    if (rc == 0)
    {
        if ( ppathtype != NULL )
            *ppathtype = type;

        if (pdir != NULL)
            *pdir = ldir;
        else
            KDirectoryRelease (ldir);
    }

    return rc;
}


/* Writable
 *  examines a directory structure for any reason it can't be opened for update
 *  NOTE: will not currently check for archive
 */
rc_t KDBWWritable ( const KDirectory *dir, const char *path )
{
    uint32_t access;
    rc_t rc;

    /* protect us from bad parameters */
    if (dir == NULL)
        return RC (rcDB, rcPath, rcAccessing, rcDirectory, rcNull);
    if (path == NULL)
        return RC (rcDB, rcPath, rcAccessing, rcPath, rcNull);

    /* we have to be able to check the access if it is to be writable */
    rc = KDirectoryAccess ( dir, & access, "%s", path );
    if ( rc == 0 )
    {
        int kpt;

        /* we could use KDBPathType with some modifications */
        kpt = KDirectoryPathType (dir, "%s", path) & ~kptAlias;
        switch (kpt)
        {
        case kptDir:
            /* if there is a lock (or deprecated sealed) file in this directory */
            switch ( KDirectoryPathType ( dir, "%s/lock", path ) )
            {
            case kptFile:
            case kptFile | kptAlias:
                rc = RC (rcDB, rcPath, rcAccessing, rcLock, rcLocked );
                break;
            case kptNotFound:
                /* much simpler handling for the sealed file */
                switch ( KDirectoryPathType ( dir, "%s/sealed", path ) )
                {
                case kptFile:
                case kptFile | kptAlias:
                    rc = RC (rcDB, rcPath, rcAccessing, rcLock, rcLocked );
                    break;
                case kptNotFound:
                    if ( ( access & 0222 ) == 0 )
                        rc = RC (rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
                    /* else rc is still 0 from VAccess */
                }
                break;
            case kptBadPath:
                /* likely to be a non-directory or something */
                rc = RC (rcDB, rcPath, rcAccessing, rcPath, rcInvalid);
                break;
            default:
                /* an illegal type of object named "lock" is in this directory
                 * which will block the ability to lock it
                 */
                rc = RC (rcDB, rcPath, rcAccessing, rcPath, rcUnexpected);
            }
            break;
        case kptFile:
            /* if there is a lock (or deprecated sealed) file in this directory */
            switch ( KDirectoryPathType ( dir, "%s.lock", path ) )
            {
            case kptFile:
            case kptFile | kptAlias:
                rc = RC ( rcDB, rcPath, rcAccessing, rcLock, rcLocked );
                break;
            case kptNotFound:
                break;
            case kptBadPath:
                /* unlikely */
                rc = RC (rcDB, rcPath, rcAccessing, rcPath, rcInvalid);
                break;
            default:
                /* an illegal type of object named "lock" is in this directory
                 * which will block the ability to lock it
                 */
                rc = RC (rcDB, rcPath, rcAccessing, rcPath, rcUnexpected);
            }
            break;
        case kptBadPath:
            /* likely to be a non-driectory or something */
            rc = RC ( rcDB, rcPath, rcAccessing, rcPath, rcInvalid);
            break;
        default:
            /* an illegal type of object named "lock" is in this directory
             * which will block the ability to lock it
             */
            rc = RC (rcDB, rcPath, rcAccessing, rcPath, rcUnexpected);
        }
    }
    return rc;
}


bool KDBIsLocked ( const KDirectory *dir, const char *path )
{

    rc_t rc = KDBWWritable (dir, path);
    if (GetRCState (rc) == rcLocked)
        return true;
    return false;
}


/* Lock
 *  performs directory locking
 */
rc_t KDBLockDir ( KDirectory *dir, const char *path )
{
    KFile *f;
    rc_t rc;

    if (dir == NULL)
        return RC (rcDB, rcLock, rcLocking, rcDirectory, rcNull);
    if (path == NULL)
        return RC (rcDB, rcLock, rcLocking, rcPath, rcNull);

    rc = KDirectoryCreateFile ( dir, & f, 0, 0444, kcmOpen, "%s/lock", path );
    if ( rc == 0 )
    {
        KFileRelease ( f );
        rc = KDirectorySetAccess ( dir, 1, 0, 0222, "%s", path );
    }
    return rc;
}

rc_t KDBLockFile ( KDirectory *dir, const char *path )
{
    KFile *f;
    rc_t rc;

    if (dir == NULL)
        return RC (rcDB, rcLock, rcLocking, rcDirectory, rcNull);
    if (path == NULL)
        return RC (rcDB, rcLock, rcLocking, rcPath, rcNull);

    rc = KDirectoryCreateFile ( dir, & f, false, 0444, kcmOpen, "%s.lock", path );
    if ( rc == 0 )
    {
        KFileRelease ( f );
        rc = KDirectorySetAccess ( dir, 0, 0, 0222, "%s", path );
    }
    return rc;
}

/* Unlock
 *  performs directory unlocking
 */
static
rc_t CC KDBUnlockVisitor ( KDirectory *dir, uint32_t type, const char *name, void *data )
{
    switch ( type )
    {
    case kptFile:
    case kptFile | kptAlias:

        /* don't ever unlock frozen metadata revisions */
        if ( name [ 0 ] == 'r' )
        {
            char *end;
            strtoul ( name + 1, & end, 10 );
            if ( end [ 0 ] == 0 )
                return 0;
        }

        return KDirectorySetAccess ( dir, false, 0220, 0222, "%s", name );

    case kptDir:
    case kptDir | kptAlias:
        if ( KDirectoryPathType ( dir, "%s/lock", name ) == kptNotFound )
        {
            rc_t rc = KDirectorySetAccess ( dir, 0, 0220, 0222, "%s", name );
            if ( rc == 0 )
            {
                rc = KDirectoryVisitUpdate ( dir, false,
                                              KDBUnlockVisitor, NULL, "%s", name );
            }
            return rc;
        }
        break;
    }

    return 0;
}

rc_t KDBUnlockDir ( KDirectory *dir, const char *path )
{
    /* change directory access bits */
    rc_t rc;

    if (dir == NULL)
        return RC (rcDB, rcLock, rcLocking, rcDirectory, rcNull);
    if (path == NULL)
        return RC (rcDB, rcLock, rcLocking, rcPath, rcNull);

    rc = KDirectorySetAccess ( dir, false, 0220, 0222, "%s", path );
    if ( rc == 0 )
    {
        /* remove lock file */
        rc = KDirectoryRemove ( dir, false, "%s/lock", path );
        if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
            rc = KDirectoryRemove ( dir, false, "%s/sealed", path );

        /* recursively unlock everything underneath
           that is not itself explicitly locked */
        if ( rc == 0 || GetRCState ( rc ) == rcNotFound )
            rc = KDirectoryVisitUpdate ( dir, false, KDBUnlockVisitor, NULL, "%s", path );
    }
    return rc;

}

rc_t KDBUnlockFile ( KDirectory *dir, const char *path )
{
    rc_t rc;

    if (dir == NULL)
        return RC (rcDB, rcLock, rcLocking, rcDirectory, rcNull);
    if (path == NULL)
        return RC (rcDB, rcLock, rcLocking, rcPath, rcNull);

    rc = KDirectoryRemove ( dir, true, "%s.lock", path );
    if ( rc == 0 )
    {
        rc = KDirectorySetAccess ( dir, false, 0220, 0222, "%s", path );
    }
    return rc;
}

/* VDrop
 */
static
rc_t KDBDropInt ( KDirectory * dir, const KDBManager * mgr,
                  const char * path )
{
    if (KDBManagerOpenObjectBusy (mgr, path))
        return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcBusy );

    return KDirectoryRemove ( dir, 1, "%s", path );
}

rc_t KDBMgrVDrop ( KDirectory * dir, const KDBManager * mgr, uint32_t obj_type,
                   const char * path, va_list args )
{
    rc_t rc;
    char full_path [ 4096 ];

    rc = KDirectoryVResolvePath ( dir, true, full_path, sizeof (full_path),
                                  path, args );
    if (rc == 0)
    {
        uint32_t pt = KDBPathType ( /*mgr,*/ dir, NULL, full_path );
        switch ( pt )
        {
        case kptNotFound:
            return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcInvalid );

        case kptDatabase | kptAlias:
        case kptTable | kptAlias:
        case kptIndex | kptAlias:
        case kptColumn | kptAlias:
        case kptDatabase:
        case kptTable:
        case kptIndex:
        case kptColumn:
            /* this really needs a new rcWrongType or similar RCState */
            if ((obj_type != kptAny) && (obj_type != (pt & ~kptAlias)))
                return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcIncorrect );
            break;

        case kptFile | kptAlias:
        case kptFile:
	    /* can we get here?  Will we have needed to open for update to get here? */
/* 	    rc = KDBManagerOpenPathTypeRead ( dir, path, NULL, type, NULL ); */
/* 	    if ( rc == 0 ) */
/*                 return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcReadonly ); */
            /* fall through */
        default:
            return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcIncorrect );
        }
        if (rc == 0)
        {
            rc = KDBDropInt ( dir, mgr, full_path );
        }
    }
    return rc;
}


rc_t KDBVDrop ( KDirectory *dir, const KDBManager * mgr,
    uint32_t type, const char *name, va_list args )
{
    rc_t rc;
    char path [ 4096 ];

    int ns_size;
    const char *ns;

    switch ( type )
    {
    case kptDatabase:
        ns = "db"; ns_size = 2; break;
    case kptTable:
        ns = "tbl"; ns_size = 3; break;
    case kptIndex:
        ns = "idx"; ns_size = 3; break;
    case kptColumn:
        ns = "col"; ns_size = 3; break;
    default:
        return RC ( rcDB, rcDirectory, rcRemoving, rcType, rcIncorrect );
    }

    rc = KDBVMakeSubPath ( dir,
        path, sizeof path, ns, ns_size, name, args );
    if ( rc == 0 )
    {
        uint32_t pt = KDBPathType ( /*mgr,*/ dir, NULL, path );
        switch ( pt )
        {
        case kptNotFound:
            return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcInvalid );

        case kptDatabase | kptAlias:
        case kptTable | kptAlias:
        case kptIndex | kptAlias:
        case kptColumn | kptAlias:
            pt -= kptAlias;

        case kptDatabase:
        case kptTable:
        case kptIndex:
        case kptColumn:
            if ( pt == type )
                break;

        case kptFile | kptAlias:
        case kptFile:
	    /* can we get here?  Will we have needed to open for update to get here? */
	    rc = KDBManagerOpenPathTypeRead ( mgr, dir, path, NULL, type, NULL, false, NULL );
	    if ( rc == 0 )
                return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcReadonly );
            /* fall through */
        default:
            return RC ( rcDB, rcDirectory, rcRemoving, rcPath, rcIncorrect );
        }

        rc = KDirectoryResolvePath ( dir, true, path, sizeof path, "%s", path );
        if ( rc == 0 )
        {
            rc = KDBWWritable ( dir, "." );
            switch (GetRCState(rc))
            {
            default:
                rc = RC ( rcDB, rcDirectory, rcRemoving, rcTable, rcUnexpected );
                break;
            case rcLocked:
                rc = RC ( rcDB, rcDirectory, rcRemoving, rcTable, rcLocked );
                break;
            case rcReadonly:
                rc = RC ( rcDB, rcDirectory, rcRemoving, rcTable, rcReadonly );
                break;
            case 0:
                rc = KDBDropInt ( dir, mgr, path );
                break;
            }
        }
    }

    return rc;
}

/* Rename
 */
rc_t KDBRename ( KDirectory *dir, KDBManager *mgr, uint32_t type, bool force,
    const char *from, const char *to )
{
    rc_t rc;
    char src [ 4096 ];

    int ns_size;
    const char *ns;

    switch ( type )
    {
    case kptDatabase:
        ns = "db"; ns_size = 2; break;
    case kptTable:
        ns = "tbl"; ns_size = 3; break;
    case kptIndex:
        ns = "idx"; ns_size = 3; break;
    case kptColumn:
        ns = "col"; ns_size = 3; break;
    default:
        return RC ( rcDB, rcDirectory, rcRenaming, rcType, rcIncorrect );
    }

    rc = KDBMakeSubPath ( dir,
        src, sizeof src, ns, ns_size, from);
    if ( rc == 0 )
    {
        char dst [ 256 ];
        uint32_t pt = KDBPathType ( /*mgr,*/ dir, NULL, src );
        switch ( pt )
        {
        case kptNotFound:
            return RC ( rcDB, rcDirectory, rcRenaming, rcPath, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcDirectory, rcRenaming, rcPath, rcInvalid );

        case kptDatabase | kptAlias:
        case kptTable | kptAlias:
        case kptIndex | kptAlias:
        case kptColumn | kptAlias:
            pt -= kptAlias;

        case kptDatabase:
        case kptTable:
        case kptIndex:
        case kptColumn:
            if ( pt == type )
                break;

        default:
            return RC ( rcDB, rcDirectory, rcRenaming, rcPath, rcIncorrect );
        }

        rc = KDBMakeSubPath ( dir,
            dst, sizeof dst, ns, ns_size, to );
        if ( rc == 0 )
        {
            if ( KDirectoryPathType ( dir, "%s", dst ) != kptNotFound )
                return RC ( rcDB, rcDirectory, rcRenaming, rcPath, rcIncorrect );

            rc = KDirectoryResolvePath ( dir, 1, src, sizeof src, "%s", src );
            if ( rc == 0 )
            {
                if (KDBManagerOpenObjectBusy (mgr, src))
                    return RC ( rcDB, rcDirectory, rcRenaming, rcPath, rcBusy );
                if (KDBManagerOpenObjectBusy (mgr, dst))
                    return RC ( rcDB, rcDirectory, rcRenaming, rcPath, rcBusy );

                rc = KDBWWritable ( dir, "." );
                switch (GetRCState(rc))
                {
                default:
                    rc = RC ( rcDB, rcDirectory, rcRenaming, rcTable, rcUnexpected );
                    break;
                case rcLocked:
                    rc = RC ( rcDB, rcDirectory, rcRenaming, rcTable, rcLocked );
                    break;
                case rcReadonly:
                    rc = RC ( rcDB, rcDirectory, rcRenaming, rcTable, rcReadonly );
                    break;
                case 0:
                    rc = KDirectoryRename ( dir, force, src, dst );
                    break;
                }
            }
        }
    }

    return rc;
}

/* Alias
 */
rc_t KDBAlias ( KDirectory *dir, uint32_t type,
    const char *targ, const char *alias )
{
    rc_t rc;
    char src [ 256 ];

    int ns_size;
    const char *ns;

    switch ( type )
    {
    case kptDatabase:
        ns = "db"; ns_size = 2; break;
    case kptTable:
        ns = "tbl"; ns_size = 3; break;
    case kptIndex:
        ns = "idx"; ns_size = 3; break;
    case kptColumn:
        ns = "col"; ns_size = 3; break;
    default:
        return RC ( rcDB, rcDirectory, rcAliasing, rcType, rcIncorrect );
    }

    rc = KDBMakeSubPath ( dir,
        src, sizeof src, ns, ns_size, targ );
    if ( rc == 0 )
    {
        char dst [ 256 ];
        uint32_t pt = KDBPathType ( /*NULL,*/ dir, NULL, src );
        switch ( pt )
        {
        case kptNotFound:
            return RC ( rcDB, rcDirectory, rcAliasing, rcPath, rcNotFound );
        case kptBadPath:
            return RC ( rcDB, rcDirectory, rcAliasing, rcPath, rcInvalid );

        case kptDatabase | kptAlias:
        case kptTable | kptAlias:
        case kptIndex | kptAlias:
        case kptColumn | kptAlias:
            pt &= ~ kptAlias;

        case kptDatabase:
        case kptTable:
        case kptIndex:
        case kptColumn:
            if ( pt == type )
                break;

        default:
            return RC ( rcDB, rcDirectory, rcAliasing, rcPath, rcIncorrect );
        }

        rc = KDBMakeSubPath ( dir,
            dst, sizeof dst, ns, ns_size, alias );
        if ( rc == 0 )
        {
            rc = KDirectoryCreateAlias ( dir,
                0775, kcmCreate, src, dst );
        }
    }

    return rc;
}
