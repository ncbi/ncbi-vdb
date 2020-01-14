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

#include "kdb-priv.h"
#include "kdbfmt-priv.h"
#include "dbmgr-priv.h"

#include <krypto/key.h>
#include <krypto/encfile.h>
#include <krypto/wgaencrypt.h>

#include <vfs/manager.h>
#include <vfs/path.h>
#include <vfs/resolver.h>
#include <vfs/manager-priv.h>
#include <sra/srapath.h>

#include <kfs/kfs-priv.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/arc.h>
#include <kfs/tar.h>
#include <kfs/sra.h>
#include <kfs/kfs-priv.h>
#include <klib/container.h>
#include <klib/text.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <va_copy.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <os-native.h>
#include <assert.h>
#include <errno.h>

#ifndef SUPPORT_VFS_URI
#define SUPPORT_VFS_URI 0
#endif

#ifndef SUPPORT_KDB_TAR
#define SUPPORT_KDB_TAR 0
#endif

/*--------------------------------------------------------------------------
 * KDB utility
 */

/* KDBHdrValidate
 *  validates that a header sports a supported byte order
 *  and that the version is within range
 */
/* this is included to make kdb.c vs. wkdb.c file comparisons easier in [x]emacs.*/
#if NOT_USED_IN_READ_ONLY_SIDE
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
#endif

/* KDBPathType
 *  checks type of path
 */
enum ScanBits
{
    scan_db     = ( 1 <<  0 ),
    scan_tbl    = ( 1 <<  1 ),
    scan_idx    = ( 1 <<  2 ),
    scan_col    = ( 1 <<  3 ),
    scan_idxN   = ( 1 <<  4 ),
    scan_data   = ( 1 <<  5 ),
    scan_dataN  = ( 1 <<  6 ),
    scan_md     = ( 1 <<  7 ),
    scan_cur    = ( 1 <<  8 ),
    scan_rNNN   = ( 1 <<  9 ),
    scan_lock   = ( 1 << 10 ),
    scan_odir   = ( 1 << 11 ),
    scan_ofile  = ( 1 << 12 ),
    scan_meta   = ( 1 << 13 ),
    scan_skey   = ( 1 << 14 ),
    scan_sealed = ( 1 << 15 ),
    scan_zombie = ( 1 << 16 )
};

static
rc_t CC scan_dbdir ( const KDirectory *dir, uint32_t type, const char *name, void *data )
{
    uint32_t *bits = data;

    type &= kptAlias - 1;

    if ( type == kptDir )
    {
        switch ( name [ 0 ] )
        {
        case 'c':
            if ( strcmp ( name, "col" ) == 0 )
            { * bits |= scan_col; return 0; }
            break;
        case 'm':
            if ( strcmp ( name, "md" ) == 0 )
            { * bits |= scan_md; return 0; }
            break;
        case 't':
            if ( strcmp ( name, "tbl" ) == 0 )
            { * bits |= scan_tbl; return 0; }
            break;
        case 'i':
            if ( strcmp ( name, "idx" ) == 0 )
            { * bits |= scan_idx; return 0; }
            break;
        case 'd':
            if ( strcmp ( name, "db" ) == 0 )
            { * bits |= scan_db; return 0; }
            break;
        }

        * bits |= scan_odir;
    }
    else if ( type == kptFile )
    {
        switch ( name [ 0 ] )
        {
        case 'l':
            if ( strcmp ( name, "lock" ) == 0 )
            { * bits |= scan_lock; return 0; }
            break;
        case 'i':
            if ( memcmp ( name, "idx", 3 ) == 0 )
            {
                if ( isdigit ( name [ 3 ] ) )
                { * bits |= scan_idxN; return 0; }
            }
            break;
        case 'd':
            if ( memcmp ( name, "data", 4 ) == 0 )
            {
                if ( name [ 4 ] == 0 )
                { * bits |= scan_data; return 0; }
                if ( isdigit ( name [ 4 ] ) )
                { * bits |= scan_dataN; return 0; }
            }
        case 'c':
            if ( strcmp ( name, "cur" ) == 0 )
            { * bits |= scan_cur; return 0; }
            break;
        case 'r':
            if ( isdigit ( name [ 1 ] ) && isdigit ( name [ 2 ] ) &&
                 isdigit ( name [ 3 ] ) && name [ 4 ] == 0 )
            { * bits |= scan_rNNN; return 0; }
            break;
        case 'm':
            if ( strcmp ( name, "meta" ) == 0 )
            { * bits |= scan_meta; return 0; }
            break;
        case 's':
            if ( strcmp ( name, "skey" ) == 0 )
            { * bits |= scan_skey; return 0; }
            if ( strcmp ( name, "sealed" ) == 0 )
            { * bits |= scan_sealed; return 0; }
            break;
        }

        * bits |= scan_ofile;
    }
    else if (type == kptZombieFile )
    {
        * bits |= scan_zombie;
    }
    
    return 0;
}


int KDBPathTypeDir (const KDirectory * dir, int type, bool * pHasZombies, const char * path)
{
    const char * leaf, * parent;
    uint32_t bits;
    rc_t rc;

    bits = 0;

    assert ((type == kptDir) || (type == (kptDir|kptAlias)));

    rc = KDirectoryVisit ( dir, false, scan_dbdir, & bits, "%s", path );
    if ( rc == 0 ) do
    {
        if ( ( bits & scan_zombie ) != 0 ) {
            bits &= ~scan_zombie;
            if (pHasZombies)
                *pHasZombies = true;
        }
        /* look for a column */
        if ( ( bits & scan_idxN ) != 0 &&
             ( bits & ( scan_data | scan_dataN ) ) != 0 )
        {
            if ( ( bits & ( scan_db | scan_tbl | scan_idx | scan_col ) ) == 0 )
                type += kptColumn - kptDir;
            break;
        }

        /* look for a table */
        if ( ( bits & scan_col ) != 0 )
        {
            /* can't have sub-tables or a db */
            if ( ( bits & ( scan_db | scan_tbl ) ) == 0 )
            {
                /* look for an old-structure table */
                if ( ( bits & ( scan_meta | scan_md ) ) == scan_meta ||
                     ( bits & ( scan_skey | scan_idx ) ) == scan_skey )
                    type += kptPrereleaseTbl - kptDir;
                else
                    type += kptTable - kptDir;
            }
            break;
        }

        /* look for metadata */
        if ( ( bits & ( scan_cur | scan_rNNN ) ) != 0 )
        {
            if ( ( bits & ( scan_db | scan_tbl | scan_idx | scan_col ) ) == 0 )
                type += kptMetadata - kptDir;
            break;
        }

        /* look for a database */
        if ( ( bits & scan_tbl ) != 0 )
        {
            if ( ( bits & scan_col ) == 0 )
                type += kptDatabase - kptDir;
            break;
        }

        /* look for a structured column */
        if ( ( bits & scan_odir ) != 0 )
        {
            leaf = strrchr ( path, '/' );
            if ( leaf != NULL )
            {
                parent = string_rchr ( path, leaf - path, '/' );
                if ( parent ++ == NULL )
                    parent = path;
                if ( memcmp ( parent, "col/", 4 ) != 0 )
                    break;

                bits = 0;
                if ( KDirectoryVisit ( dir, 1, scan_dbdir, & bits, "%s", path ) == 0 )
                {
                    if ( ( bits & scan_idxN ) != 0 &&
                         ( bits & ( scan_data | scan_dataN ) ) != 0 )
                    {
                        if ( ( bits & ( scan_db | scan_tbl | scan_idx | scan_col ) ) == 0 )
                            type += kptColumn - kptDir;
                        break;
                    }
                }
            }
        }
    } while (0);

    return type;
}


int KDBPathType ( const KDirectory *dir, bool *pHasZombies, const char *path )
{
    const char *leaf, *parent;


    rc_t rc;
    int type = KDirectoryPathType ( dir, "%s", path );
    
    if (pHasZombies)
        *pHasZombies = false;

    switch ( type )
    {
    case kptDir:
    case kptDir | kptAlias:
        type = KDBPathTypeDir (dir, type, pHasZombies, path);
        break;

    case kptFile:
    case kptFile | kptAlias:
    {
        /* if we hit a file first try it as an archive */
        const KDirectory * ldir;

        rc = KDirectoryOpenSraArchiveRead_silent ( dir, &ldir, false, "%s", path );
#if SUPPORT_KDB_TAR
        if ( rc != 0 )
            rc = KDirectoryOpenTarArchiveRead_silent ( dir, &ldir, false, "%s", path );
#endif
        /* it was an archive so recur */
        if ( rc == 0 )
        {
            /* recheck this newly opened directory for KDB/KFS type */
            int type2;

            type2 = KDBPathType ( ldir, NULL, "." );
            if ((type2 != kptDir) || (type != (kptDir|kptAlias)))
                type = type2;

            KDirectoryRelease (ldir);
        }
        /* it was not an archive so see if it it's an idx file */
        else
        {
            leaf = strrchr ( path, '/' );
            if ( leaf != NULL )
            {
                parent = string_rchr ( path, leaf - path, '/' );
                if ( parent ++ == NULL )
                    parent = path;
                if ( memcmp ( parent, "idx/", 4 ) == 0 )
                    type += kptIndex - kptFile;
            }
        }
        break;
    }
    }
    return type;
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
#if SUPPORT_KDB_TAR
                        rc = KDirectoryOpenTarArchiveRead_silent_preopened (dir, &ldir, false,
                                                                            file, "%s", path);
#else
                        /* will be reset immediately below */
                        rc = -1;
#endif
                    }

                    /* not an archive type we handle or a bad archive */
                    if ( rc != 0 )
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
                                     int pathtype, uint32_t rcobj, bool try_srapath,
                                     const VPath * aVpath )
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
        rc = VPathMakeDirectoryRelativeVPath ( &vpath, dir, path, vpath );
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

rc_t KDBOpenPathTypeRead ( const KDBManager * mgr, const KDirectory * dir, const char * path, 
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

    rc = KDBOpenPathTypeReadInt( mgr, dir, path, &ldir, &type, pathtype, rcobj,
        try_srapath, vpath );

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
 *  examines a directory structure for any "lock" files
 */
rc_t KDBWritable ( const KDirectory *dir, const char *path )
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
        /* if there is a lock (or deprecated sealed) file in this directory */
        switch ( KDirectoryPathType ( dir, "%s/lock", path ) )
        {
        case kptFile:
        case kptFile | kptAlias:
            rc = RC ( rcDB, rcPath, rcAccessing, rcLock, rcLocked );
            break;
        case kptNotFound:
            /* much simpler handling for the sealed file */
            switch ( KDirectoryPathType ( dir, "%s/sealed", path ) )
            {
            case kptFile:
            case kptFile | kptAlias:
                rc = RC ( rcDB, rcPath, rcAccessing, rcLock, rcLocked );
                break;
            case kptNotFound:
                /* check if there are no write permissions */
                if ( ( access & 0222 ) == 0 )
                    rc = RC ( rcDB, rcPath, rcAccessing, rcPath, rcReadonly );
                /* else rc is still 0 from VAccess */
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
            rc = RC ( rcDB, rcPath, rcAccessing, rcPath, rcIncorrect );
        }
    }
    return rc;
}


bool KDBIsLocked ( const KDirectory *dir, const char *path )
{
    return ( KDBWritable (dir, path) != 0 );
}


/* GetObjModDate
 *  extract mod date from a path
 */
rc_t KDBGetObjModDate ( const KDirectory *dir, KTime_t *mtime )
{
    /* HACK ALERT - there needs to be a proper way to record modification times */
    
    /* this only tells the last time the table was locked,
       which may be close to the last time it was modified */
    rc_t rc = KDirectoryDate ( dir, mtime, "lock" );
    if ( rc == 0 )
        return 0;

    if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = KDirectoryDate ( dir, mtime, "sealed" );
        if ( rc == 0 )
            return 0;
    }

    /* get directory timestamp */
    rc = KDirectoryDate ( dir, mtime, "." );
    if ( rc == 0 )
        return 0;

    * mtime = 0;
    return rc;
}

/* GetPathModDate
 *  extract mod date from a path
 */
rc_t KDBVGetPathModDate ( const KDirectory *dir,
    KTime_t *mtime, const char *path, va_list args )
{
    rc_t rc;
    uint32_t ptype;
    const KDirectory *obj_dir;

    va_list cpy;
    va_copy ( cpy, args );
    ptype = KDirectoryVPathType ( dir, path, cpy );
    va_end ( cpy );

    switch ( ptype )
    {
    case kptDir:
    case kptDir | kptAlias:
        break;

    default:
        return KDirectoryVDate ( dir, mtime, path, args );
    }

    * mtime = 0;
    rc = KDirectoryVOpenDirRead ( dir, & obj_dir, true, path, args );
    if ( rc == 0 )
    {
        rc = KDBGetObjModDate ( obj_dir, mtime );
        KDirectoryRelease ( obj_dir );
    }

    return rc;
}


/* KDBVMakeSubPath
 *  adds a namespace to path spec
 */
rc_t KDBVMakeSubPath ( struct KDirectory const *dir,
    char *subpath, size_t subpath_max, const char *ns,
    uint32_t ns_size, const char *path, va_list args )
{
    rc_t rc;

    if ( ns_size > 0 )
    {
        subpath += ns_size + 1;
        subpath_max -= ns_size + 1;
    }

#if CRUFTY_USE_OF_RESOLVE_PATH
    /* because this call only builds a path instead of resolving anything
     * is is okay that we are using the wrong directory */
    rc = KDirectoryVResolvePath ( dir, false,
        subpath, subpath_max, path, args );
#else
    {
        int sz = vsnprintf ( subpath, subpath_max, path, args );
        if ( sz < 0 || ( size_t ) sz >= subpath_max )
            rc = RC ( rcDB, rcDirectory, rcResolving, rcBuffer, rcInsufficient );
        else if ( sz == 0 )
            rc = RC ( rcDB, rcDirectory, rcResolving, rcPath, rcEmpty );
        else
        {
            rc = 0;
        }
    }
#endif
    switch ( GetRCState ( rc ) )
    {
    case 0:
        assert ( subpath [ 0 ] != 0 );
        if ( subpath [ 0 ] == '.' || subpath [ 1 ] == '/' )
            return RC ( rcDB, rcDirectory, rcResolving, rcPath, rcInvalid );
        break;
    case rcInsufficient:
        return RC ( rcDB, rcDirectory, rcResolving, rcPath, rcExcessive );
    default:
        return rc;
    }

    if ( ns_size != 0 )
    {
        subpath -= ns_size + 1;
        memmove ( subpath, ns, ns_size );
        subpath [ ns_size ] = '/';
    }
    return rc;
}

/* KDBMakeSubPath
 *  adds a namespace to path spec
 */
rc_t KDBMakeSubPath ( struct KDirectory const *dir,
    char *subpath, size_t subpath_max, const char *ns,
    uint32_t ns_size, const char *path, ... )
{
    rc_t rc = 0;
    va_list args;
    va_start(args, path);
    rc = KDBVMakeSubPath(dir, subpath, subpath_max, ns, ns_size, path, args);
    va_end(args);
    return rc;
}

/* this is included to make kdb.c vs. wkdb.c file comparisons easier in [x]emacs.*/
#if NOT_USED_IN_READ_ONLY_SIDE
/* VDrop
 */
static
rc_t KDBDropInt ( KDirectory * dir, const KDBManager * mgr,
                  const char * path )
{
}

rc_t KDBMgrVDrop ( KDirectory * dir, const KDBManager * mgr, uint32_t obj_type,
                   const char * path, va_list args )
{
}
#endif

/* KDBIsPathUri
 * A hack to get some of VFS into KDB that is too tightly bound to KFS
 */

bool KDBIsPathUri (const char * path)
{
    const char * pc;
    size_t z;

    z = string_size (path);

    if (NULL != (pc = string_chr (path, z, ':')))
        return true;

    if (NULL != (pc = string_chr (path, z, '?')))
        return true;

    if (NULL != (pc = string_chr (path, z, '#')))
        return true;

    return false;
}
