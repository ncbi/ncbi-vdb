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

#include <kdb/manager.h>

#include "dbmgr.h"
#include "../vfs/path-priv.h" /* VPathSetDirectory */

#include <kfs/directory.h>
#include <kfs/sra.h>

#include <vfs/manager.h>
#include <vfs/manager-priv.h>
#include <vfs/path-priv.h> /* VPathGetDirectory */

#include <klib/text.h>
#include <klib/rc.h>

#include <ctype.h>
#include <stdio.h>

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

        rc = VPathGetDirectory(vpath, &ldir);
        if (rc != 0 || ldir == NULL)
          /*
           * We've got to decide if the path coming in is a full or relative
           * path and if relative make it relative to dir or possibly its
           * a srapath accession
           *
           */
          rc = VFSManagerMakeDirectoryRelativeVPath (vmgr,
            &vpath, dir, path, vpath );
        if ( rc == 0 )
        {
            if (ldir == NULL)
            {
                rc = VFSManagerOpenDirectoryReadDirectoryRelativeDecrypt ( vmgr,
                    dir, &ldir, vpath );
                if (rc == 0)
                    VPathSetDirectory((VPath*)aVpath, ldir);
            }

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
            if ( aVpath != vpath )
                VPathRelease ( vpath );
        }
    }

    return rc;
}

rc_t KDBManagerOpenPathTypeRead ( const KDBManager * mgr, const KDirectory * dir, const char * path,
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
