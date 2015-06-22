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

#include "windex-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "wtable-priv.h"
#include "wkdb-priv.h"
#include "kdbfmt-priv.h"

#include <kdb/kdb-priv.h>
#include <kdb/index.h>
#include <klib/symbol.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KIndex
 *  an object capable of mapping an object to integer oid
 */

struct KIndex
{
    BSTNode n;

    KDBManager *mgr;
    KDatabase *db;
    KTable *tbl;
    KDirectory *dir;

    KRefcount refcount;
    uint32_t opencount;
    uint32_t vers;
    union
    {
        KTrieIndex_v1 txt1;
        KTrieIndex_v2 txt2;
        KU64Index_v3  u64_3;
    } u;
    bool converted_from_v1;
    uint8_t type;
    uint8_t read_only;
    uint8_t dirty;
    bool use_md5;

    KSymbol sym;

    char path [ 1 ];
};


/* Whack
 */
static
rc_t KIndexWhack ( KIndex *self )
{
    rc_t rc;
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;

    KRefcountWhack ( & self -> refcount, "KIndex" );

    rc = KIndexCommit( self );
    if ( rc == 0 )
    {
        /* release owner */
        if ( self -> db != NULL )
        {
            rc = KDatabaseSever ( self -> db );
            if ( rc != 0 )
                return rc;
            self -> db = NULL;
        }
        else if ( self -> tbl != NULL )
        {
            rc = KTableSever ( self -> tbl );
            if ( rc != 0 )
                return rc;
            self -> tbl = NULL;
        }
    }

    /* remove from mgr */
    symb = KDBManagerOpenObjectFind (mgr, self->path);
    if (symb != NULL)
    {
        rc = KDBManagerOpenObjectDelete (mgr, symb);
        if (rc == 0)
        {
            /* release manager
               should never fail */
            rc = KDBManagerSever ( mgr );
            if ( rc != 0 )
                rc = KDBManagerOpenObjectAdd (mgr, symb);
            else
            {
                self -> mgr = NULL;

                rc = SILENT_RC ( rcDB, rcIndex, rcDestroying, rcIndex, rcBadVersion );

                /* complete */
                switch ( self -> type )
                {
                case kitText:
                case kitText | kitProj:
                    switch ( self -> vers )
                    {
                    case 1:
                        KTrieIndexWhack_v1 ( & self -> u . txt1 );
                        rc = 0;
                        break;
                    case 2:
                    case 3:
                    case 4:
                        KTrieIndexWhack_v2 ( & self -> u . txt2 );
                        rc = 0;
                        break;
                    }
                    break;

                case kitU64:
                    switch ( self -> vers )
                    {
                    case 3:
                    case 4:
                        rc = KU64IndexWhack_v3 ( & self -> u . u64_3 );
                        break;
                    }
                    break;
                }
            }
        }
    }
    if ( rc == 0 )
    {
        KDirectoryRelease ( self -> dir );
        free ( self );
    }
    else
        KRefcountInit ( & self -> refcount, 1, "KIndex", "whack", "kidx" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KIndexAddRef ( const KIndex *cself )
{
    KIndex *self = ( KIndex* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KIndex" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcIndex, rcAttaching, rcRange, rcExcessive );
        }
        ++ self -> opencount;
    }
    return 0;
}

LIB_EXPORT rc_t CC KIndexRelease ( const KIndex *cself )
{
    KIndex *self = ( KIndex* ) cself;
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KIndex" ) )
        {
        case krefWhack:
            return KIndexWhack ( ( KIndex* ) self );
        case krefLimit:
            return RC ( rcDB, rcIndex, rcReleasing, rcRange, rcExcessive );
        }
        -- self -> opencount;
    }
    return 0;
}


/* Attach
 */
static
rc_t KIndexAttach ( KIndex *self, const KMMap *mm, bool *byteswap )
{
    size_t size;
    rc_t rc = KMMapSize ( mm, & size );
    if ( rc == 0 )
    {
        const void *addr;
        rc = KMMapAddrRead ( mm, & addr );
        if ( rc == 0 )
        {
            union
            {
                KIndexFileHeader_v1 v1;
                KIndexFileHeader_v2 v2;
                KIndexFileHeader_v3 v3;
            } hdrs;

            const KDBHdr *hdr = addr;
            const KIndexFileHeader_v3 *fh = addr;

            * byteswap = false;
            rc = KDBHdrValidate ( hdr, size, 1, KDBINDEXVERS );
            if ( GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
            {
                hdrs . v1 . endian = bswap_32 ( hdr -> endian );
                hdrs . v1 . version = bswap_32 ( hdr -> version );
                rc = KDBHdrValidate ( & hdrs . v1, size, 1, KDBINDEXVERS );
                if ( rc == 0 )
                {
                    * byteswap = true;
                    switch ( hdrs . v1 . version )
                    {
                    case 1:
                        hdr = & hdrs . v1;
                        break;
                    case 2:
                        hdr = & hdrs . v2;
                        break;
                    case 3:
                    case 4:
                        hdrs . v3 . index_type = bswap_32 ( fh -> index_type );
                        hdrs . v3 . reserved1 = bswap_32 ( fh -> reserved1 );
                        hdr = & hdrs . v3 . h;
                        fh = & hdrs . v3;
                        break;
                    }
                }
            }
            if ( rc == 0 )
            {
                self -> vers = hdr -> version;
                switch ( hdr -> version )
                {
                case 1:
#if KDBINDEXVERS != 1
                    self -> converted_from_v1 = true;
#endif
                case 2:
                    self -> type = kitText;
                    break;
                case 3:
                case 4:
                {
                    self -> type = fh -> index_type;
                    switch( self->type )
                    {
                    case kitText:
                    case kitU64:
                        break;
                    default:
                        rc = RC(rcDB, rcIndex, rcConstructing, rcIndex, rcUnrecognized);
                    }
                    break;
                }
                default:
                    rc = RC(rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion);
                }
            }
        }
    }
    return rc;
}

/* Sever
 * there is no KIndexSever()
 */

/* Make
 *  make an initialized structure
 *  NB - does NOT attach reference to dir, but steals it
 */
static
rc_t KIndexMake ( KIndex **idxp, KDirectory *dir, const char *path )
{
    rc_t rc;
    char fullpath[4096];

    if ( idxp == NULL )
        rc = RC ( rcDB, rcIndex, rcCreating, rcParam, rcNull );
    else
    {
        if ( dir == NULL )
            rc = RC ( rcDB, rcIndex, rcCreating, rcDirectory, rcNull );
        else if ( path == NULL )
            rc = RC ( rcDB, rcIndex, rcCreating, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcDB, rcIndex, rcCreating, rcPath, rcEmpty );
        else
        {
            rc = KDirectoryResolvePath(dir, true, fullpath, sizeof(fullpath), "%s", path);
            if (rc == 0)
            {
                KIndex* idx = malloc ( sizeof *idx + strlen ( fullpath ) );
                if ( idx == NULL )
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    memset ( idx, 0, sizeof * idx );
                    rc = KDirectoryAddRef ( dir );
                    if ( rc == 0 )
                    {
                        idx -> dir = dir;
                        KRefcountInit ( & idx -> refcount, 1, "KIndex", "make", fullpath );
                        idx -> opencount = 1;

                        strcpy ( idx -> path, fullpath );

                        idx->sym.u.obj = idx;
                        idx->sym.dad = NULL;   /* not strictly needed */
                        StringInitCString (&idx->sym.name, idx->path);
                        idx->sym.type = kptIndex;

                        * idxp = idx;
                        return 0;
                    }

                    free ( idx );
                }
            }
        }

        * idxp = NULL;
    }

    return rc;
}

static
rc_t KDBManagerInsertIndex ( KDBManager * self, KIndex * idx)
{
    rc_t rc;
    rc = KDBManagerOpenObjectAdd (self, &idx->sym);
    if (rc == 0)
        idx->mgr = KDBManagerAttach (self);
    return rc;
}

static
rc_t KIndexMakeRead ( KIndex **idxp, const KDirectory *dir, const char *path )
{
    const KFile *f;
    rc_t rc = KDirectoryOpenFileRead ( dir, & f, "%s", path );
    if ( rc == 0 )
    {
        const KMMap *mm;
        rc = KMMapMakeRead ( & mm, f );
        if ( rc == 0 )
        {
            rc = KIndexMake ( idxp, ( KDirectory* ) dir, path );
            if ( rc == 0 )
            {
                bool byteswap;
                KIndex *idx = * idxp;
                rc = KIndexAttach ( idx, mm, & byteswap );
                if ( rc == 0 )
                {
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion );
                    idx -> read_only = true;
                    switch ( idx -> vers )
                    {
                    case 1:
                        /* open using v1 code only if KDBINDEXVERS is 1
                           if 2 or later, open as a v2 index */
#if KDBINDEXVERS == 1
                        rc = KTrieIndexOpen_v1 ( & idx -> u . txt1, mm );
                        if ( rc == 0 )
                        {
                            if ( idx -> u . txt1 . pt . id2node != NULL )
                                idx -> type = ( uint8_t ) ( kitText | kitProj );
                            else
                                idx -> type = ( uint8_t ) kitText;
                        }
                        break;
#else
                    case 2:
                    case 3:
                    case 4:
                        switch(idx->type) {
                            case kitText:
                            case kitText | kitProj:
                                /* will guess version in open */
                                rc = KTrieIndexOpen_v2 ( & idx -> u . txt2, mm, byteswap );
                                if( rc == 0 && idx -> u.txt2.pt.ord2node != NULL ) {
                                    idx -> type |= kitProj;
                                }
                                idx -> vers = 3;
                                break;

                            case kitU64:
                                rc = KU64IndexOpen_v3(&idx->u.u64_3, mm, byteswap);
                                break;
                        }
                        break;
#endif
                    }
                }

                if ( rc != 0 )
                    KIndexWhack ( idx );
            }

            KMMapRelease ( mm );
        }

        KFileRelease ( f );
    }
    return rc;
}

static
rc_t KIndexMakeUpdate ( KIndex **idxp, KDirectory *dir, const char *path )
{
    const KFile *f;
    rc_t rc = KDirectoryOpenFileRead ( dir, & f, "%s", path );

    if ( rc == 0 )
    {
        const KMMap *mm;
        rc = KMMapMakeRead ( & mm, f );
        if ( rc == 0 )
        {
            rc = KIndexMake ( idxp, dir, path );
            if ( rc == 0 )
            {
                bool byteswap;
                KIndex *idx = * idxp;
                rc = KIndexAttach ( idx, mm, & byteswap );
                if ( rc == 0 )
                {
                    switch ( idx -> vers )
                    {
                    case 1:
                        /* open using v1 code only if KDBINDEXVERS is 1
                           if 2 or later, open as a v2 index */
#if KDBINDEXVERS == 1
                        rc = KTrieIndexOpen_v1 ( & idx -> u . txt1, mm, byteswap );
                        if ( rc == 0 )
                        {
                            if ( idx -> u . txt1 . pt . id2node != NULL )
                                idx -> type = ( uint8_t ) ( kitText | kitProj );
                            else
                                idx -> type = ( uint8_t ) kitText;
                        }
                        break;
#else
                    case 2:
                    case 3:
                    case 4:
                        switch(idx->type) {
                            case kitText:
                            case kitText | kitProj:
                                /* will guess version in open */
                                rc = KTrieIndexOpen_v2 ( & idx -> u . txt2, mm, byteswap );
                                if ( rc == 0 )
                                {
                                    if( idx -> u . txt2 . pt . ord2node != NULL ) {
                                        idx -> type |= kitProj;
                                    }
                                    /* v3 takes over v1 and v2 */
                                    if( idx->vers < 3 ) {
                                        idx -> vers = 3;
                                        /* check for a sparse id space */
                                        if( idx -> u . txt2 . pt . variant != 0 ) {
                                            /* try to load existing guy */
                                            rc = KTrieIndexAttach_v2 ( & idx -> u . txt2,
                                                          idx -> type != ( uint8_t ) kitText );
                                            if ( rc == 0 )
                                                idx -> dirty = true;
                                        }
                                    }
                                }
                                break;

                            case kitU64:
                                rc = KU64IndexOpen_v3(&idx->u.u64_3, mm, byteswap);
                                break;
                        }
                        break;
#endif
                    default:
                        rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion );
                    }
                }

                if ( rc != 0 )
                    KIndexWhack ( idx );
            }

            KMMapRelease ( mm );
        }

        KFileRelease ( f );
    }
    return rc;
}

/* MarkModified
 *  make the index think it has been modified, such that it may be committed
 *  useful when forcing conversion from an older version
 */
#if KDBINDEXVERS > 1
LIB_EXPORT rc_t CC KIndexMarkModified ( KIndex *self )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcDB, rcIndex, rcUpdating, rcSelf, rcNull );
    else if ( self -> read_only )
        rc = RC ( rcDB, rcIndex, rcUpdating, rcIndex, rcReadonly );
    else if ( self -> dirty )
        rc = 0;
    else switch ( self -> type )
    {
    case kitText:
    case kitText | kitProj:
        switch ( self -> vers )
        {
        default:
            rc = RC ( rcDB, rcIndex, rcUpdating, rcIndex, rcBadVersion );
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            rc = KTrieIndexAttach_v2 ( & self -> u . txt2,
                self -> type != ( uint8_t ) kitText );
            if ( rc == 0 )
                self -> dirty = true;
        }
        break;
    default:
        rc = RC ( rcDB, rcIndex, rcUpdating, rcIndex, rcInvalid );
    }

    return rc;
}
#endif

static
rc_t KIndexCreate ( KIndex **idxp, KDirectory *dir,
    KIdxType type, KCreateMode cmode, const char *path, int ptype )
{
    rc_t rc = 0;
    KIndex *idx;

    if ( ptype != kptNotFound )
    {
        switch ( cmode & kcmValueMask )
        {
        case kcmOpen:
            rc = KIndexMakeUpdate ( idxp, dir, path );
            if ( rc == 0 )
            {
                idx = * idxp;
                if ( ( KIdxType ) idx -> type != type )
                {
                    * idxp = NULL;
                    KIndexWhack ( idx );
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcType, rcIncorrect );
                }
            }
            return rc;

        case kcmInit:
            break;

        case kcmCreate:
            return RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcExists );
        }
    }
#if KDBINDEXVERS == 1 || KDBIDXVERS == 2
    else if ( type != kitText && type != ( kitText | kitProj ) )
    {
        * idxp = NULL;
        return RC ( rcDB, rcIndex, rcConstructing, rcType, rcUnsupported );
    }
#endif

    rc = KIndexMake ( idxp, dir, path );
    if ( rc == 0 )
    {
        idx = * idxp;

        switch ( type )
        {
        case kitText:
        case kitText | kitProj:
#if KDBINDEXVERS == 1
            rc = KTrieIndexOpen_v1 ( & idx -> u . txt1, NULL, false );
#else
            rc = KTrieIndexOpen_v2 ( & idx -> u . txt2, NULL, false );
#endif
            break;

        case kitU64:
            rc = KU64IndexOpen_v3 ( & idx->u.u64_3, NULL, false );
            break;

        default:
            rc = RC ( rcDB, rcIndex, rcConstructing, rcType, rcUnsupported );
        }

        if ( rc != 0 )
        {
            * idxp = NULL;
            KIndexWhack ( idx );
        }
        else
        {
            idx -> vers = KDBINDEXVERS;
            idx -> type = ( uint8_t ) type;
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
static
rc_t KDBManagerCreateIndexInt ( KDBManager *self, KIndex **idxp,
    KDirectory *wd, KIdxType type, KCreateMode cmode, const char *path, bool use_md5 )
{
    rc_t rc;
    int ptype;
    char idxpath [ 4096 ];

    rc = KDirectoryResolvePath ( wd, true,
                                  idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KIndex *idx;
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
            rc = KDBWritable ( wd, idxpath );
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

        rc = KIndexCreate ( & idx, wd, type, cmode, path, ptype );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertIndex (self, idx);
            if ( rc == 0 )
            {
                idx -> use_md5 = use_md5;
                * idxp = idx;
                return 0;
            }
            
            KIndexRelease ( idx );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseCreateIndex ( struct KDatabase *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVCreateIndex ( self, idx, type, cmode, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVCreateIndex ( KDatabase *self, KIndex **idxp,
    KIdxType type, KCreateMode cmode, const char *name, va_list args )
{
    rc_t rc = 0;
    KDirectory *dir;

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcCreating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcCreating, rcDatabase, rcReadonly );

    rc = KDirectoryVCreateDir ( self -> dir, 0777, kcmOpen, "idx", NULL );
    if ( rc == 0 )
        rc = KDirectoryVOpenDirUpdate ( self -> dir, & dir, false, "idx", NULL );
    if ( rc == 0 )
    {
        char path [ 256 ];
        rc = KDirectoryVResolvePath ( dir, false, path, sizeof path, name, args );
        if ( rc == 0 )
        {
            rc = KDBManagerCreateIndexInt ( self -> mgr, idxp, dir,
                type, cmode | kcmParents, path, (self -> cmode & kcmMD5) != 0 );
            if ( rc == 0 )
            {
                KIndex *idx = * idxp;
                idx -> db = KDatabaseAttach ( self );
            }
        }

        KDirectoryRelease ( dir );
    }

    return rc;
}

LIB_EXPORT rc_t CC KTableCreateIndex ( struct KTable *self, KIndex **idx,
    KIdxType type, KCreateMode cmode, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KTableVCreateIndex ( self, idx, type, cmode, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVCreateIndex ( KTable *self, KIndex **idxp,
    KIdxType type, KCreateMode cmode, const char *name, va_list args )
{
    rc_t rc = 0;
    KDirectory *dir;

    if ( idxp == NULL )
        return RC ( rcDB, rcTable, rcCreating, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcCreating, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcTable, rcCreating, rcTable, rcReadonly );

    rc = KDirectoryVCreateDir ( self -> dir, 0777, kcmOpen, "idx", NULL );
    if ( rc == 0 )
        rc = KDirectoryVOpenDirUpdate ( self -> dir, & dir, false, "idx", NULL );
    if ( rc == 0 )
    {
        char path [ 256 ];
        rc = KDirectoryVResolvePath ( dir, false, path, sizeof path, name, args );
        if ( rc == 0 )
        {
            rc = KDBManagerCreateIndexInt ( self -> mgr, idxp, dir,
                type, cmode | kcmParents, path, self -> use_md5 );
            if ( rc == 0 )
            {
                KIndex *idx = * idxp;
                idx -> tbl = KTableAttach ( self );
            }
        }

        KDirectoryRelease ( dir );
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
static
rc_t KDBManagerOpenIndexReadInt ( KDBManager *self,
    const KIndex **idxp, const KDirectory *wd, const char *path )
{
    char idxpath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
                                      idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KIndex *idx;
        KSymbol * sym;

        /* if already open */
        sym = KDBManagerOpenObjectFind (self, idxpath);
        if (sym != NULL)
        {
            const KIndex * cidx;
            rc_t obj;
            
            switch (sym->type)
            {
            case kptIndex:
                cidx = (const KIndex *)sym->u.obj;
#if 0
                /* if open for update, refuse */
                if ( cidx -> read_only )
#endif
#if 0
                if (cidx is coherent)
#endif
                {
                    /* attach a new reference and we're gone */
                    rc = KIndexAddRef ( cidx );
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

        rc = KIndexMakeRead ( & idx, wd, idxpath );
        if ( rc == 0 )
        {
            idx -> read_only = true;
            rc = KDBManagerInsertIndex (self, idx);
            if ( rc == 0 )
            {
                * idxp = idx;
                return 0;
            }

            KIndexRelease ( idx );
        }
    }
    
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenIndexRead ( struct KDatabase const *self,
    const KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenIndexRead ( self, idx, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVOpenIndexRead ( const KDatabase *cself,
    const KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( cself == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    rc = KDBVMakeSubPath ( cself -> dir,
        path, sizeof path, "idx", 3, name, args );
    if ( rc == 0 )
    {
        KIndex *idx;
        rc = KDBManagerOpenIndexReadInt ( cself -> mgr, (const KIndex **)& idx,
                                          cself -> dir, path );
        if ( rc == 0 )
        {
            KDatabase *self = ( KDatabase* ) cself;
            idx -> db = KDatabaseAttach ( self );
            * idxp = idx;
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KTableOpenIndexRead ( struct KTable const *self,
    const KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KTableVOpenIndexRead ( self, idx, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVOpenIndexRead ( const KTable *self,
    const KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    const char *ns = "idx";
    uint32_t ns_size = 3;

    if ( idxp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    if ( self -> prerelease )
    {
        ns = "";
        ns_size = 0;
    }

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, ns, ns_size, name, args );
    if ( rc == 0 )
    {
        KIndex *idx;
        rc = KDBManagerOpenIndexReadInt ( self -> mgr, (const KIndex**)& idx,
                                          self -> dir, path );
        if ( rc == 0 )
        {
            if (idx->tbl != self)
                idx -> tbl = KTableAttach ( self );
            * idxp = idx;
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
static
rc_t KDBManagerOpenIndexUpdate ( KDBManager *self,
    KIndex **idxp, KDirectory *wd, const char *path )
{
    char idxpath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
                                      idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KSymbol * sym;
        KIndex *idx;

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

        rc = KIndexMakeUpdate ( & idx, wd, path );
        if ( rc == 0 )
        {
            rc = KDBManagerInsertIndex (self, idx);
            if ( rc == 0 )
            {
                * idxp = idx;
                return 0;
            }
            
            KIndexRelease ( idx );
        }
    }
    return rc;
}

LIB_EXPORT rc_t CC KDatabaseOpenIndexUpdate ( struct KDatabase *self,
    KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KDatabaseVOpenIndexUpdate ( self, idx, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KDatabaseVOpenIndexUpdate ( KDatabase *self,
    KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    KDirectory *dir;

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcDatabase, rcOpening, rcDatabase, rcReadonly );

    rc = KDirectoryVOpenDirUpdate ( self -> dir, & dir, false, "idx", NULL );
    if ( rc == 0 )
    {
        char path [ 256 ];
        rc = KDirectoryVResolvePath ( dir, false, path, sizeof path, name, args );
        if ( rc == 0 )
        {
            rc = KDBManagerOpenIndexUpdate ( self -> mgr, idxp, dir, path );
            if ( rc == 0 )
            {
                KIndex *idx = * idxp;
                idx -> db = KDatabaseAttach ( self );
            }
        }

        KDirectoryRelease ( dir );
    }
    return rc;
}

LIB_EXPORT rc_t CC KTableOpenIndexUpdate ( struct KTable *self,
    KIndex **idx, const char *name, ... )
{
    rc_t rc = 0;
    va_list args;

    va_start ( args, name );
    rc = KTableVOpenIndexUpdate ( self, idx, name, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KTableVOpenIndexUpdate ( KTable *self,
    KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    KDirectory *dir;

    if ( idxp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcTable, rcOpening, rcTable, rcReadonly );

    rc = KDirectoryVOpenDirUpdate ( self -> dir, & dir, false, "idx", NULL );
    if ( rc == 0 )
    {
        char path [ 256 ];
        rc = KDirectoryVResolvePath ( dir, false, path, sizeof path, name, args );
        if ( rc == 0 )
        {
            rc = KDBManagerOpenIndexUpdate ( self -> mgr, idxp, dir, path );
            if ( rc == 0 )
            {
                KIndex *idx = * idxp;
                idx -> tbl = KTableAttach ( self );
            }
        }

        KDirectoryRelease ( dir );
    }
    return rc;
}


/* Locked
 *  returns non-zero if locked
 */
LIB_EXPORT bool CC KIndexLocked ( const KIndex *self )
{
    if ( self != NULL )
    {
        rc_t rc = KDBWritable ( self -> dir, "" );
        if (GetRCState ( rc ) == rcLocked)
            return true;
    }
    return false;
}


/* Version
 *  returns the format version
 */
LIB_EXPORT rc_t CC KIndexVersion ( const KIndex *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcIndex, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * version = 0;
        return RC ( rcDB, rcIndex, rcAccessing, rcSelf, rcNull );
    }

    * version = self -> vers;
    return 0;
}


/* Type
 *  returns the type of index
 */
LIB_EXPORT rc_t CC KIndexType ( const KIndex *self, KIdxType *type )
{
    if ( type == NULL )
        return RC ( rcDB, rcIndex, rcAccessing, rcParam, rcNull );

    if ( self == NULL )
    {
        * type = ( KIdxType ) 0;
        return RC ( rcDB, rcIndex, rcAccessing, rcSelf, rcNull );
    }

    * type = ( KIdxType ) self -> type;
    return 0;
}

/* Commit
 *  ensure any changes are committed to disk
 */
LIB_EXPORT rc_t CC KIndexCommit ( KIndex *self )
{
    rc_t rc = 0;
    bool proj;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcCommitting, rcSelf, rcNull );

    if ( ! self -> dirty )
        return 0;

    rc = RC ( rcDB, rcIndex, rcCommitting, rcType, rcUnsupported );

    proj = ( self -> type & kitProj ) != 0;

    switch ( self -> type )
    {
        case kitText:
        case kitText | kitProj:
            switch ( self -> vers )
            {
            case 1:
                rc = KTrieIndexPersist_v1 ( & self -> u . txt1,
                    proj, self -> dir, self -> path, self -> use_md5 );
                break;
            case 2:
            case 3:
            case 4:
                rc = KTrieIndexPersist_v2 ( & self -> u . txt2,
                    proj, self -> dir, self -> path, self -> use_md5 );
                break;
            }
            break;

        case kitU64:
            switch(self -> vers) {
            case 3:
            case 4:
                rc = KU64IndexPersist_v3(&self->u.u64_3, proj, self->dir,
                                         self->path, self->use_md5);
                break;
            }
            break;
    }

    if ( rc == 0 )
        self -> dirty = false;

    return rc;
}

/* Insert
 *  creates a mapping from key to id
 *  and potentially from id to key if supported
 *
 *  "unique" [ IN ] - if non zero, key must be unique
 *
 *  "key" [ IN ] - NUL terminated string for text
 *
 *  "id" [ IN ] - id
 */
LIB_EXPORT rc_t CC KIndexInsertText ( KIndex *self, bool unique,
    const char *key, int64_t id )
{
    rc_t rc = 0;
    bool proj;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcInserting, rcSelf, rcNull );
    if ( key == NULL )
        return RC ( rcDB, rcIndex, rcInserting, rcString, rcNull );
    if ( key [ 0 ] == 0 )
        return RC ( rcDB, rcIndex, rcInserting, rcString, rcInvalid );
    if ( self -> read_only )
        return RC ( rcDB, rcIndex, rcInserting, rcIndex, rcReadonly );

    proj = false;
    switch ( self -> type )
    {
    case kitText | kitProj:
        proj = true;
    case kitText:
        switch ( self -> vers )
        {
        case 1:
            /* there is a limit on ids in v1 */
            if ( id <= 0 || ( id >> 32 ) != 0 )
                return RC ( rcDB, rcIndex, rcInserting, rcId, rcExcessive );

            rc = KTrieIndexInsert_v1 ( & self -> u . txt1,
                proj, key, ( uint32_t ) id );
            break;
        case 2:
        case 3:
        case 4:
            rc = KTrieIndexInsert_v2 ( & self -> u . txt2,
                proj, key, id );
            break;
        default:
            return RC ( rcDB, rcIndex, rcInserting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcInserting, rcType, rcUnsupported );
    }

    switch ( GetRCState ( rc ) )
    {
    case 0:
        self -> dirty = true;
        break;
    case rcExists:
        if ( ! unique )
            rc = RC ( rcDB, rcIndex, rcInserting, rcConstraint, rcViolated );
    default:
        break;
    }

    return rc;
}


/* Delete
 *  deletes all mappings from key
 */
LIB_EXPORT rc_t CC KIndexDeleteText ( KIndex *self, const char *key )
{
    rc_t rc;
    bool proj;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcRemoving, rcSelf, rcNull );
    if ( key == NULL )
        return RC ( rcDB, rcIndex, rcRemoving, rcString, rcNull );
    if ( key [ 0 ] == 0 )
        return RC ( rcDB, rcIndex, rcRemoving, rcString, rcInvalid );
    if ( self -> read_only )
        return RC ( rcDB, rcIndex, rcRemoving, rcIndex, rcReadonly );

    proj = false;
    switch ( self -> type )
    {
    case kitText | kitProj:
        proj = true;
    case kitText:
        switch ( self -> vers )
        {
        case 1:
            rc = KTrieIndexDelete_v1 ( & self -> u . txt1, proj, key );
            break;
        case 2:
        case 3:
        case 4:
            rc = KTrieIndexDelete_v2 ( & self -> u . txt2, proj, key );
            break;
        default:
            return RC ( rcDB, rcIndex, rcRemoving, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcRemoving, rcType, rcUnsupported );
    }

    if ( rc == 0 )
        self -> dirty = true;

    return rc;
}


/* Find
 *  finds a single mapping from key
 */
LIB_EXPORT rc_t CC KIndexFindText ( const KIndex *self, const char *key, int64_t *start_id, uint64_t *id_count,
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void *data )
{
    rc_t rc = 0;
    uint32_t id32, span;

    if ( id_count != NULL )
        * id_count = 0;

    if ( start_id == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcParam, rcNull );

    * start_id = 0;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcSelf, rcNull );
    if ( key == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNull );
    if ( key[ 0 ] == 0 )
        return RC ( rcDB, rcIndex, rcSelecting, rcString, rcInvalid );

    span = 1;

    switch ( self -> type )
    {
    case kitText:
    case kitText | kitProj:
        switch ( self -> vers )
        {
        case 1:
            rc = KTrieIndexFind_v1 ( & self -> u . txt1, key, & id32, custom_cmp, data );
            if ( rc == 0 )
                * start_id = id32;
            break;
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KTrieIndexFind_v2 ( & self -> u . txt2, key, start_id, & span, custom_cmp, data, self -> converted_from_v1 );
#else
            ( void ) ( span = 0 );
            rc = KTrieIndexFind_v2 ( & self -> u . txt2, key, start_id, custom_cmp, data, self -> converted_from_v1  );
#endif
            break;
        default:
            return RC ( rcDB, rcIndex, rcSelecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcSelecting, rcType, rcUnsupported );
    }

    if ( id_count != NULL )
        * id_count = span;
    
    return rc;
}


/* FindAll
 *  finds all mappings from key
 */
LIB_EXPORT rc_t CC KIndexFindAllText ( const KIndex *self, const char *key,
    rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data )
{
    rc_t rc = 0;
    int64_t id64;
    uint32_t id32, span;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcSelf, rcNull );
    if ( f == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcFunction, rcNull );
    if ( key == NULL )
        return RC ( rcDB, rcIndex, rcSelecting, rcString, rcNull );
    if ( key[ 0 ] == 0 )
        return RC ( rcDB, rcIndex, rcSelecting, rcString, rcInvalid );

    span = 1;

    switch ( self -> type )
    {
    case kitText:
    case kitText | kitProj:
        switch ( self -> vers )
        {
        case 1:
            rc = KTrieIndexFind_v1 ( & self -> u . txt1, key, & id32, NULL, NULL );
            if ( rc == 0 )
                rc = ( * f ) ( id32, 1, data );
            break;
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KTrieIndexFind_v2 ( & self -> u . txt2, key, & id64, & span, NULL, NULL, self -> converted_from_v1 );
#else
            rc = KTrieIndexFind_v2 ( & self -> u . txt2, key, & id64, NULL, NULL, self -> converted_from_v1 );
#endif
            if ( rc == 0 )
                rc = ( * f ) ( id64, span, data );
            break;
        default:
            return RC ( rcDB, rcIndex, rcSelecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcSelecting, rcType, rcUnsupported );
    }
    
    return rc;
}


/* Project
 *  finds key(s) mapping to value/id if supported
 */
LIB_EXPORT rc_t CC KIndexProjectText ( const KIndex *self,
    int64_t id, int64_t *start_id, uint64_t *id_count,
    char *key, size_t kmax, size_t *actsize )
{
    rc_t rc = 0;
    int64_t dummy;
    uint32_t span;

    if ( start_id == NULL )
        start_id = & dummy;

    * start_id = 0;

    if ( id_count != NULL )
        * id_count = 0;

    if ( key == NULL && kmax != 0 )
        return RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcNull );

    if ( kmax != 0 )
        key [ 0 ] = 0;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcProjecting, rcSelf, rcNull );
        
    if ( ( ( KIdxType ) self -> type & kitProj ) == 0 )
        return RC ( rcDB, rcIndex, rcProjecting, rcFunction, rcInvalid );

    span = 1;

    switch ( self -> type )
    {
    case kitText:
    case kitText | kitProj:
        switch ( self -> vers )
        {
        case 1:
            /* there is a limit on ids in v1 */
            if ( id <= 0 || ( id >> 32 ) != 0 )
                return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );

            rc = KTrieIndexProject_v1 ( & self -> u . txt1,
                    ( uint32_t ) id, key, kmax, actsize );
            if ( rc == 0 )
                * start_id = id;
            break;
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KTrieIndexProject_v2 ( & self -> u . txt2, id, start_id, & span, key, kmax, actsize );
#else
            rc = KTrieIndexProject_v2 ( & self -> u . txt2, id, key, kmax, actsize );
            if ( rc == 0 )
                * start_id = id;
#endif
            break;
        default:
            return RC ( rcDB, rcIndex, rcProjecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcProjecting, rcType, rcUnsupported );
    }

    if ( id_count != NULL )
        * id_count = span;
        
    return rc;
}


/* ProjectAll
 *  finds key(s) mapping to value/id if supported
 */
LIB_EXPORT rc_t CC KIndexProjectAllText ( const KIndex *self, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data )
{
    rc_t rc = 0;
    char key [ 256 ];

    uint32_t span;
    int64_t start_id;

    if ( self == NULL )
        return RC ( rcDB, rcIndex, rcProjecting, rcSelf, rcNull );
    if ( f == NULL )
        return RC ( rcDB, rcIndex, rcProjecting, rcFunction, rcNull );
    if ( ( ( KIdxType ) self -> type & kitProj ) == 0 )
        return RC ( rcDB, rcIndex, rcProjecting, rcFunction, rcInvalid );
        
    span = 1;

    switch ( self -> type )
    {
    case kitText:
    case kitText | kitProj:
        switch ( self -> vers )
        {
        case 1:
            /* there is a limit on ids in v1 */
            if ( id <= 0 || ( id >> 32 ) != 0 )
                return RC ( rcDB, rcIndex, rcProjecting, rcId, rcNotFound );

            rc = KTrieIndexProject_v1 ( & self -> u . txt1,
                ( uint32_t ) id, key, sizeof key, NULL );
            if ( rc == 0 )
                rc = ( * f ) ( id, 1, key, data );
            break;
            
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KTrieIndexProject_v2 ( & self -> u . txt2, id, & start_id, & span, key, sizeof key, NULL );
#else
            rc = KTrieIndexProject_v2 ( & self -> u . txt2, id, key, sizeof key, NULL );
#endif
            if ( rc == 0 )
                rc = ( * f ) ( start_id, span, key, data );
            break;
            
        default:
            return RC ( rcDB, rcIndex, rcProjecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcProjecting, rcType, rcUnsupported );
    }
        
    return rc;
}

LIB_EXPORT rc_t CC KIndexInsertU64( KIndex *self, bool unique, uint64_t key, 
    uint64_t key_size, int64_t id, uint64_t id_qty )
{
    rc_t rc = 0;

    if( self == NULL ) {
        return RC(rcDB, rcIndex, rcInserting, rcSelf, rcNull);
    }
    if( self->read_only ) {
        return RC(rcDB, rcIndex, rcInserting, rcIndex, rcReadonly);
    }

    switch( self->type )
    {
    case kitU64:
        switch( self->vers )
        {
        case 3:
        case 4:
            rc = KU64IndexInsert_v3(&self->u.u64_3, unique, key, key_size, id, id_qty );
            break;
        default:
            return RC(rcDB, rcIndex, rcInserting, rcIndex, rcBadVersion);
        }
        break;
        
    default:
        return RC(rcDB, rcIndex, rcInserting, rcType, rcUnsupported);
    }

    switch( GetRCState(rc) ) {
        case 0:
            self->dirty = true;
            break;
        case rcExists:
            if( !unique ) {
                rc = RC(rcDB, rcIndex, rcInserting, rcConstraint, rcViolated);
            }
        default:
            break;
    }
    return rc;
}

LIB_EXPORT rc_t CC KIndexDeleteU64( KIndex *self, uint64_t key )
{
    rc_t rc = 0;

    if( self == NULL ) {
        return RC(rcDB, rcIndex, rcRemoving, rcSelf, rcNull);
    }
    if( self->read_only ) {
        return RC(rcDB, rcIndex, rcRemoving, rcIndex, rcReadonly);
    }

    switch( self->type )
    {
    case kitU64:
        switch( self->vers )
        {
        case 3:
        case 4:
            rc = KU64IndexDelete_v3(&self->u.u64_3, key);
            break;
        default:
            return RC(rcDB, rcIndex, rcRemoving, rcIndex, rcBadVersion);
        }
        break;
    default:
        return RC(rcDB, rcIndex, rcRemoving, rcType, rcUnsupported);
    }

    if( rc == 0 ) {
        self -> dirty = true;
    }
    return rc;
}

LIB_EXPORT rc_t CC KIndexFindU64( const KIndex* self, uint64_t offset, uint64_t* key, 
    uint64_t* key_size, int64_t* id, uint64_t* id_qty )
{
    rc_t rc = 0;

    if( key == NULL || key_size == NULL || id == NULL || id_qty == NULL ) {
        return RC(rcDB, rcIndex, rcSelecting, rcParam, rcNull);
    }
    *key = *key_size = *id = *id_qty = 0;
    if( self == NULL ) {
        return RC(rcDB, rcIndex, rcSelecting, rcSelf, rcNull);
    }

    switch( self->type )
    {
    case kitU64:
        switch( self->vers )
        {
        case 3:
        case 4:
            rc = KU64IndexFind_v3(&self->u.u64_3, offset, key, key_size, id, id_qty);
            break;
        default:
            return RC(rcDB, rcIndex, rcSelecting, rcIndex, rcBadVersion);
        }
        break;
    default:
        return RC(rcDB, rcIndex, rcSelecting, rcType, rcUnsupported);
    }
    return rc;
}

LIB_EXPORT rc_t CC KIndexFindAllU64( const KIndex* self, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data)
{
    rc_t rc = 0;

    if( self == NULL ) {
        return RC(rcDB, rcIndex, rcSelecting, rcSelf, rcNull);
    }
    if( f == NULL ) {
        return RC(rcDB, rcIndex, rcSelecting, rcFunction, rcNull);
    }

    switch( self->type )
    {
    case kitU64:
        switch( self->vers )
        {
        case 3:
        case 4:
            rc = KU64IndexFindAll_v3(&self->u.u64_3, offset, f, data);
            break;
        default:
            return RC(rcDB, rcIndex, rcSelecting, rcIndex, rcBadVersion);
        }
        break;
    default:
        return RC(rcDB, rcIndex, rcSelecting, rcType, rcUnsupported);
    }
    return rc;
}


/* SetMaxId
 *  certain legacy versions of skey were built to know only the starting id
 *  of the NAME_FMT column, but were never given a maximum id. allow them
 *  to be corrected here.
 */
LIB_EXPORT void CC KIndexSetMaxRowId ( const KIndex *cself, int64_t max_row_id )
{
    if ( cself != NULL ) switch ( cself -> type )
    {
    case kitText:
    case kitText | kitProj:
        switch ( cself -> vers )
        {
        case 2:
        case 3:
        case 4:
            /* here we can repair the max row id */
            if ( cself -> u . txt2 . pt . maxid < max_row_id )
                ( ( KIndex* ) cself ) -> u . txt2 . pt . maxid = max_row_id;
            break;
        }
        break;
    }
}
