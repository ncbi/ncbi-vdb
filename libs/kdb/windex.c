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

#include "windex.h"
#include "wdbmgr.h"
#include "database-cmn.h"
#include "wtable.h"
#include "wkdb.h"
#include "kdbfmt.h"

#include <kdb/kdb-priv.h>
#include <kdb/index.h>

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

static rc_t CC KWIndexWhack ( KWIndex *self );
static bool CC KWIndexLocked ( const KWIndex *self );
static rc_t CC KWIndexVersion ( const KWIndex *self, uint32_t *version );
static rc_t CC KWIndexType ( const KWIndex *self, KIdxType *type );
static rc_t CC KWRIndexConsistencyCheck ( const KWIndex *self, uint32_t level,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes );
static rc_t CC KWIndexFindText ( const KWIndex *self, const char *key, int64_t *start_id, uint64_t *id_count,
    int ( CC * custom_cmp ) ( const void *item, struct PBSTNode const *n, void *data ),
    void *data );
static rc_t CC KWIndexFindAllText ( const KWIndex *self, const char *key,
    rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data );
static rc_t CC KWIndexProjectText ( const KWIndex *self,
    int64_t id, int64_t *start_id, uint64_t *id_count,
    char *key, size_t kmax, size_t *actsize );
static rc_t CC KWIndexProjectAllText ( const KWIndex *self, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data );
static rc_t CC KWIndexFindU64( const KWIndex* self, uint64_t offset, uint64_t* key,
    uint64_t* key_size, int64_t* id, uint64_t* id_qty );
static rc_t CC KWIndexFindAllU64( const KWIndex* self, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data);
static void CC KWIndexSetMaxRowId ( const KWIndex *cself, int64_t max_row_id );

static KIndex_vt KWIndex_vt =
{
    KWIndexWhack,
    KIndexBaseAddRef,
    KIndexBaseRelease,
    KWIndexLocked,
    KWIndexVersion,
    KWIndexType,
    KWRIndexConsistencyCheck,
    KWIndexFindText,
    KWIndexFindAllText,
    KWIndexProjectText,
    KWIndexProjectAllText,
    KWIndexFindU64,
    KWIndexFindAllU64,
    KWIndexSetMaxRowId
};

#define CAST() assert( bself->vt == &KWIndex_vt ); KWIndex * self = (KWIndex *)bself

/* Whack
 */
static
rc_t CC
KWIndexWhack ( KWIndex *self )
{
    rc_t rc, rc2 = 0;
    KDBManager *mgr = self -> mgr;
    KSymbol * symb;

    rc = KIndexCommit( & self -> dad );

    /* release owner */
    if ( self -> db != NULL )
    {
        rc2 = KDatabaseSever ( self -> db );
        self -> db = NULL;
    }
    else if ( self -> tbl != NULL )
    {
        rc2 = KTableSever ( & self -> tbl -> dad );
        self -> tbl = NULL;
    }
    if ( rc == 0 )
        rc = rc2;

    /* remove from mgr */
    symb = KDBManagerOpenObjectFind (mgr, self->path);
    if (symb != NULL)
    {
        rc2 = KDBManagerOpenObjectDelete (mgr, symb);
        if (rc2 == 0)
        {
            /* release manager
               should never fail */
            rc2 = KDBManagerSever ( mgr );
            if ( rc2 != 0 )
                rc2 = KDBManagerOpenObjectAdd (mgr, symb);
            else
            {
                self -> mgr = NULL;

                rc2 = SILENT_RC ( rcDB, rcIndex, rcDestroying, rcIndex, rcBadVersion );

                /* complete */
                switch ( self -> type )
                {
                case kitText:
                case kitText | kitProj:
                    switch ( self -> vers )
                    {
                    case 1:
                        KWTrieIndexWhack_v1 ( & self -> u . txt1 );
                        rc2 = 0;
                        break;
                    case 2:
                    case 3:
                    case 4:
                        KWTrieIndexWhack_v2 ( & self -> u . txt2 );
                        rc2 = 0;
                        break;
                    }
                    break;

                case kitU64:
                    switch ( self -> vers )
                    {
                    case 3:
                    case 4:
                        rc2 = KWU64IndexWhack_v3 ( & self -> u . u64_3 );
                        break;
                    }
                    break;
                }
            }
        }

        if ( rc == 0 )
            rc = rc2;
    }

    rc2 = KDirectoryRelease ( self -> dir );
    if ( rc == 0 )
        rc = rc2;

    rc2 = KIndexBaseWhack( self );
    if ( rc == 0 )
        rc = rc2;

    return rc;
}


/* Attach
 */
static
rc_t KIndexAttach ( KWIndex *self, const KMMap *mm, bool *byteswap )
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
                    self -> type = (uint8_t) fh -> index_type;
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
rc_t KIndexMake ( KWIndex **idxp, KDirectory *dir, const char *path )
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
                size_t fullpath_size = strlen(fullpath);
                KWIndex* idx = malloc ( sizeof *idx + fullpath_size);
                if ( idx == NULL )
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
                else
                {
                    memset ( idx, 0, sizeof * idx );
                    rc = KDirectoryAddRef ( dir );
                    if ( rc == 0 )
                    {
                        idx -> dad . vt = & KWIndex_vt;
                        KRefcountInit ( & idx -> dad . refcount, 1, "KWIndex", "make", fullpath );

                        idx -> dir = dir;
                        string_copy ( idx -> path, fullpath_size + 1, fullpath, fullpath_size );

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

rc_t KWIndexMakeRead ( KWIndex **idxp, const KDirectory *dir, const char *path )
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
                KWIndex *idx = * idxp;
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
                        rc = KWTrieIndexOpen_v1 ( & idx -> u . txt1, mm );
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
                                rc = KWTrieIndexOpen_v2 ( & idx -> u . txt2, mm, byteswap );
                                if( rc == 0 && idx -> u.txt2.pt.ord2node != NULL ) {
                                    idx -> type |= kitProj;
                                }
                                idx -> vers = 3;
                                break;

                            case kitU64:
                                rc = KWU64IndexOpen_v3(&idx->u.u64_3, mm, byteswap);
                                break;
                        }
                        break;
#endif
                    }
                }

                if ( rc != 0 )
                    KWIndexWhack ( idx );
            }

            KMMapRelease ( mm );
        }

        KFileRelease ( f );
    }
    return rc;
}

rc_t KWIndexMakeUpdate ( KWIndex **idxp, KDirectory *dir, const char *path )
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
                KWIndex *idx = * idxp;
                rc = KIndexAttach ( idx, mm, & byteswap );
                if ( rc == 0 )
                {
                    switch ( idx -> vers )
                    {
                    case 1:
                        /* open using v1 code only if KDBINDEXVERS is 1
                           if 2 or later, open as a v2 index */
#if KDBINDEXVERS == 1
                        rc = KWTrieIndexOpen_v1 ( & idx -> u . txt1, mm, byteswap );
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
                                rc = KWTrieIndexOpen_v2 ( & idx -> u . txt2, mm, byteswap );
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
                                            rc = KWTrieIndexAttach_v2 ( & idx -> u . txt2,
                                                          idx -> type != ( uint8_t ) kitText );
                                            if ( rc == 0 )
                                                idx -> dirty = true;
                                        }
                                    }
                                }
                                break;

                            case kitU64:
                                rc = KWU64IndexOpen_v3(&idx->u.u64_3, mm, byteswap);
                                break;
                        }
                        break;
#endif
                    default:
                        rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion );
                    }
                }

                if ( rc != 0 )
                    KIndexWhack ( & idx -> dad );
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
LIB_EXPORT rc_t CC KIndexMarkModified ( KIndex *bself )
{
    CAST();

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
            rc = KWTrieIndexAttach_v2 ( & self -> u . txt2,
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

rc_t KWIndexCreate ( KWIndex **idxp, KDirectory *dir, KIdxType type, KCreateMode cmode, const char *path, int ptype )
{
    rc_t rc = 0;
    KWIndex *idx;

    if ( ptype != kptNotFound )
    {
        switch ( cmode & kcmValueMask )
        {
        case kcmOpen:
            rc = KWIndexMakeUpdate ( idxp, dir, path );
            if ( rc == 0 )
            {
                idx = * idxp;
                if ( ( KIdxType ) idx -> type != type )
                {
                    * idxp = NULL;
                    KIndexWhack ( & idx -> dad );
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
            rc = KWTrieIndexOpen_v1 ( & idx -> u . txt1, NULL, false );
#else
            rc = KWTrieIndexOpen_v2 ( & idx -> u . txt2, NULL, false );
#endif
            break;

        case kitU64:
            rc = KWU64IndexOpen_v3 ( & idx->u.u64_3, NULL, false );
            break;

        default:
            rc = RC ( rcDB, rcIndex, rcConstructing, rcType, rcUnsupported );
        }

        if ( rc != 0 )
        {
            * idxp = NULL;
            KIndexWhack ( & idx -> dad );
        }
        else
        {
            idx -> vers = KDBINDEXVERS;
            idx -> type = ( uint8_t ) type;
        }
    }

    return rc;
}

/* Locked
 *  returns non-zero if locked
 */
static
bool CC
KWIndexLocked ( const KWIndex *self )
{
    rc_t rc = KDBWWritable ( self -> dir, "" );
    return GetRCState ( rc ) == rcLocked;
}


/* Version
 *  returns the format version
 */
static
rc_t CC
KWIndexVersion ( const KWIndex *self, uint32_t *version )
{
    if ( version == NULL )
        return RC ( rcDB, rcIndex, rcAccessing, rcParam, rcNull );

    * version = self -> vers;
    return 0;
}


/* Type
 *  returns the type of index
 */
static
rc_t CC
KWIndexType ( const KWIndex *self, KIdxType *type )
{
    if ( type == NULL )
        return RC ( rcDB, rcIndex, rcAccessing, rcParam, rcNull );

    * type = ( KIdxType ) self -> type;
    return 0;
}

/* Commit
 *  ensure any changes are committed to disk
 */
LIB_EXPORT rc_t CC KIndexCommit ( KIndex *bself )
{
    CAST();

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
                rc = KWTrieIndexPersist_v1 ( & self -> u . txt1,
                    proj, self -> dir, self -> path, self -> use_md5 );
                break;
            case 2:
            case 3:
            case 4:
                rc = KWTrieIndexPersist_v2 ( & self -> u . txt2,
                    proj, self -> dir, self -> path, self -> use_md5 );
                break;
            }
            break;

        case kitU64:
            switch(self -> vers) {
            case 3:
            case 4:
                rc = KWU64IndexPersist_v3(&self->u.u64_3, proj, self->dir,
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
LIB_EXPORT rc_t CC KIndexInsertText ( KIndex *bself, bool unique,
    const char *key, int64_t id )
{
    CAST();

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

            rc = KWTrieIndexInsert_v1 ( & self -> u . txt1,
                proj, key, ( uint32_t ) id );
            break;
        case 2:
        case 3:
        case 4:
            rc = KWTrieIndexInsert_v2 ( & self -> u . txt2,
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
LIB_EXPORT rc_t CC KIndexDeleteText ( KIndex *bself, const char *key )
{
    CAST();

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
            rc = KWTrieIndexDelete_v1 ( & self -> u . txt1, proj, key );
            break;
        case 2:
        case 3:
        case 4:
            rc = KWTrieIndexDelete_v2 ( & self -> u . txt2, proj, key );
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
static
rc_t CC
KWIndexFindText ( const KWIndex *self, const char *key, int64_t *start_id, uint64_t *id_count,
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
            rc = KWTrieIndexFind_v1 ( & self -> u . txt1, key, & id32, custom_cmp, data );
            if ( rc == 0 )
                * start_id = id32;
            break;
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KWTrieIndexFind_v2 ( & self -> u . txt2, key, start_id, & span, custom_cmp, data, self -> converted_from_v1 );
#else
            ( void ) ( span = 0 );
            rc = KWTrieIndexFind_v2 ( & self -> u . txt2, key, start_id, custom_cmp, data, self -> converted_from_v1  );
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
static
rc_t CC
KWIndexFindAllText ( const KWIndex *self, const char *key,
    rc_t ( CC * f ) ( int64_t id, uint64_t id_count, void *data ), void *data )
{
    rc_t rc = 0;
    int64_t id64;
    uint32_t id32, span;

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
            rc = KWTrieIndexFind_v1 ( & self -> u . txt1, key, & id32, NULL, NULL );
            if ( rc == 0 )
                rc = ( * f ) ( id32, 1, data );
            break;
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KWTrieIndexFind_v2 ( & self -> u . txt2, key, & id64, & span, NULL, NULL, self -> converted_from_v1 );
#else
            rc = KWTrieIndexFind_v2 ( & self -> u . txt2, key, & id64, NULL, NULL, self -> converted_from_v1 );
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
static
rc_t CC
KWIndexProjectText ( const KWIndex *self,
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

            rc = KWTrieIndexProject_v1 ( & self -> u . txt1,
                    ( uint32_t ) id, key, kmax, actsize );
            if ( rc == 0 )
                * start_id = id;
            break;
        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KWTrieIndexProject_v2 ( & self -> u . txt2, id, start_id, & span, key, kmax, actsize );
#else
            rc = KWTrieIndexProject_v2 ( & self -> u . txt2, id, key, kmax, actsize );
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
static
rc_t CC
KWIndexProjectAllText ( const KWIndex *self, int64_t id,
    rc_t ( CC * f ) ( int64_t start_id, uint64_t id_count, const char *key, void *data ),
    void *data )
{
    rc_t rc = 0;
    char key [ 256 ];

    uint32_t span;
    int64_t start_id;

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

            rc = KWTrieIndexProject_v1 ( & self -> u . txt1,
                ( uint32_t ) id, key, sizeof key, NULL );
            if ( rc == 0 )
                rc = ( * f ) ( id, 1, key, data );
            break;

        case 2:
        case 3:
        case 4:
#if V2FIND_RETURNS_SPAN
            rc = KWTrieIndexProject_v2 ( & self -> u . txt2, id, & start_id, & span, key, sizeof key, NULL );
#else
            rc = KWTrieIndexProject_v2 ( & self -> u . txt2, id, key, sizeof key, NULL );
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

LIB_EXPORT rc_t CC KIndexInsertU64( KIndex *bself, bool unique, uint64_t key,
    uint64_t key_size, int64_t id, uint64_t id_qty )
{
    CAST();

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
            rc = KWU64IndexInsert_v3(&self->u.u64_3, unique, key, key_size, id, id_qty );
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

LIB_EXPORT rc_t CC KIndexDeleteU64( KIndex *bself, uint64_t key )
{
    CAST();

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
            rc = KWU64IndexDelete_v3(&self->u.u64_3, key);
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

static
rc_t CC
KWIndexFindU64( const KWIndex* self, uint64_t offset, uint64_t* key,
    uint64_t* key_size, int64_t* id, uint64_t* id_qty )
{
    rc_t rc = 0;

    if( key == NULL || key_size == NULL || id == NULL || id_qty == NULL ) {
        return RC(rcDB, rcIndex, rcSelecting, rcParam, rcNull);
    }
    *key = *key_size = *id = *id_qty = 0;

    switch( self->type )
    {
    case kitU64:
        switch( self->vers )
        {
        case 3:
        case 4:
            rc = KWU64IndexFind_v3(&self->u.u64_3, offset, key, key_size, id, id_qty);
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

static rc_t CC KWIndexFindAllU64( const KWIndex* self, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data)
{
    rc_t rc = 0;

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
            rc = KWU64IndexFindAll_v3(&self->u.u64_3, offset, f, data);
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
static
void CC
KWIndexSetMaxRowId ( const KWIndex *cself, int64_t max_row_id )
{
    switch ( cself -> type )
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
                ( ( KWIndex* ) cself ) -> u . txt2 . pt . maxid = max_row_id;
            break;
        }
        break;
    }
}

static
rc_t CC
KWRIndexConsistencyCheck ( const KWIndex *self, uint32_t level,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes )
{   // not used on the write side
    return 0;
}
