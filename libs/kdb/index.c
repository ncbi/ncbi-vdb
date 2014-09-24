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

#define KONST const
#include "index-priv.h"
#include "dbmgr-priv.h"
#include "database-priv.h"
#include "table-priv.h"
#include "kdb-priv.h"
#undef KONST

#include <kdb/index.h>
#include <kfs/file.h>
#include <kfs/mmap.h>
#include <klib/refcount.h>
#include <klib/rc.h>
#include <os-native.h>
#include <sysalloc.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <byteswap.h>
#include <assert.h>

/*--------------------------------------------------------------------------
 * KIndex
 *  an object capable of mapping an object to integer oid
 */
struct KIndex
{
    const KDBManager *mgr;
    const KDatabase *db;
    const KTable *tbl;
    KRefcount refcount;
    uint32_t vers;
    union
    {
        KTrieIndex_v1 txt1;
        KTrieIndex_v2 txt234;
        KU64Index_v3  u64_3;
    } u;
    bool converted_from_v1;
    uint8_t type;
    char path [ 1 ];
};


/* Whack
 */
static
rc_t KIndexWhack ( KIndex *self )
{
    rc_t rc = 0;

    KRefcountWhack ( & self -> refcount, "KIndex" );

    /* release owner */
    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc == 0 )
            self -> db = NULL;
    }
    else if ( self -> tbl != NULL )
    {
        rc = KTableSever ( self -> tbl );
        if ( rc == 0 )
            self -> tbl = NULL;
    }

    /* remove from mgr */
    if ( rc == 0 )
        rc = KDBManagerSever ( self -> mgr );
    if ( rc == 0 )
    {
        self -> mgr = NULL;

        /* complete */
        rc = SILENT_RC ( rcDB, rcIndex, rcDestroying, rcIndex, rcBadVersion );

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
                KTrieIndexWhack_v2 ( & self -> u . txt234 );
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

        if ( rc == 0 )
        {
            free ( self );
            return 0;
        }
    }

    KRefcountInit ( & self -> refcount, 1, "KIndex", "whack", "kidx" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
LIB_EXPORT rc_t CC KIndexAddRef ( const KIndex *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountAdd ( & self -> refcount, "KIndex" ) )
        {
        case krefLimit:
            return RC ( rcDB, rcIndex, rcAttaching, rcRange, rcExcessive );
        }
    }
    return 0;
}

LIB_EXPORT rc_t CC KIndexRelease ( const KIndex *self )
{
    if ( self != NULL )
    {
        switch ( KRefcountDrop ( & self -> refcount, "KIndex" ) )
        {
        case krefWhack:
            return KIndexWhack ( ( KIndex* ) self );
        case krefNegative:
            return RC ( rcDB, rcIndex, rcReleasing, rcRange, rcExcessive );
        }
    }
    return 0;
}


/* Make
 */
static
rc_t KIndexMake ( KIndex **idxp, const char *path )
{
    rc_t rc;

    if ( idxp == NULL )
        rc = RC ( rcDB, rcIndex, rcCreating, rcParam, rcNull );
    else
    {
        if ( path == NULL )
            rc = RC ( rcDB, rcIndex, rcCreating, rcPath, rcNull );
        else if ( path [ 0 ] == 0 )
            rc = RC ( rcDB, rcIndex, rcCreating, rcPath, rcEmpty );
        else
        {
            KIndex* idx = malloc ( sizeof *idx + strlen ( path ) );
            if ( idx == NULL )
                rc = RC ( rcDB, rcIndex, rcConstructing, rcMemory, rcExhausted );
            else
            {
                memset ( idx, 0, sizeof * idx );
                KRefcountInit ( & idx -> refcount, 1, "KIndex", "make", path );
                strcpy ( idx -> path, path );
                * idxp = idx;
                return 0;
            }
        }
    }

    return rc;
}

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
                    switch ( self -> type )
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
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion );
                }
            }
        }
    }
    return rc;
}

static
rc_t KIndexMakeRead ( KIndex **idxp,
    const KDirectory *dir, const char *path )
{
    const KFile *f;
    rc_t rc = KDirectoryOpenFileRead ( dir, & f, "%s", path );
    if ( rc == 0 )
    {
        const KMMap *mm;
        rc = KMMapMakeRead ( & mm, f );
        if ( rc == 0 )
        {
            rc = KIndexMake ( idxp, path );
            if ( rc == 0 )
            {
                bool byteswap;
                KIndex *idx = * idxp;
                rc = KIndexAttach ( idx, mm, & byteswap );
                if ( rc == 0 )
                {
                    rc = RC ( rcDB, rcIndex, rcConstructing, rcIndex, rcBadVersion );

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
                        idx -> vers = 3;
                    case 3:
                    case 4:
                        switch ( idx -> type )
                        {
                            case kitText:
                            case kitText | kitProj:
                                /* will guess version in open */
                                rc = KTrieIndexOpen_v2 ( & idx -> u . txt234, mm, byteswap );
                                if( rc == 0 && idx -> u . txt234 . pt . ord2node != NULL )
                                    idx -> type |= kitProj;
                                break;

                            case kitU64:
                                rc = KU64IndexOpen_v3 ( & idx -> u . u64_3, mm, byteswap );
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


/* OpenIndexRead
 * VOpenIndexRead
 *  open an index for read
 *
 *  "idx" [ OUT ] - return parameter for newly opened index
 *
 *  "name" [ IN ] - NUL terminated string in UTF-8 giving simple name of idx
 */
static
rc_t KDBManagerOpenIndexReadInt ( const KDBManager *self,
    KIndex **idxp, const KDirectory *wd, const char *path )
{
    char idxpath [ 4096 ];
    rc_t rc = KDirectoryResolvePath ( wd, true,
                                      idxpath, sizeof idxpath, "%s", path );
    if ( rc == 0 )
    {
        KIndex *idx;

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
            idx -> mgr = KDBManagerAttach ( self );
            * idxp = idx;
            return 0;
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

LIB_EXPORT rc_t CC KDatabaseVOpenIndexRead ( const KDatabase *self,
    const KIndex **idxp, const char *name, va_list args )
{
    rc_t rc = 0;
    char path [ 256 ];

    if ( idxp == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcDatabase, rcOpening, rcSelf, rcNull );

    rc = KDBVMakeSubPath ( self -> dir,
        path, sizeof path, "idx", 3, name, args );
    if ( rc == 0 )
    {
        KIndex *idx;
        rc = KDBManagerOpenIndexReadInt ( self -> mgr,
            & idx, self -> dir, path );
        if ( rc == 0 )
        {
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

    if ( idxp == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcParam, rcNull );

    * idxp = NULL;

    if ( self == NULL )
        return RC ( rcDB, rcTable, rcOpening, rcSelf, rcNull );

    if ( self -> prerelease )
    {
        int len;
        if ( args == 0 )
            len = snprintf ( path, sizeof path, "%s", name );
        else
            len = vsnprintf ( path, sizeof path, name, args );
        if ( len < 0 || ( size_t ) len >= sizeof path )
            return RC ( rcDB, rcTable, rcOpening, rcPath, rcExcessive );
        rc = 0;
    }
    else
    {
        rc = KDBVMakeSubPath ( self -> dir,
            path, sizeof path, "idx", 3, name, args );
    }

    if ( rc == 0 )
    {
        KIndex *idx;
        rc = KDBManagerOpenIndexReadInt ( self -> mgr,
            & idx, self -> dir, path );
        if ( rc == 0 )
        {
            idx -> tbl = KTableAttach ( self );
            * idxp = idx;
        }
    }
    return rc;
}


/* Locked
 *  returns non-zero if locked
 */
LIB_EXPORT bool CC KIndexLocked ( const KIndex *self )
{
    rc_t rc;
    const KDirectory *dir;

    if ( self == NULL )
        return false;

    assert ( self -> db != NULL || self -> tbl != NULL );
    dir = ( self -> db != NULL ) ?
        self -> db -> dir : self -> tbl -> dir;

    rc = KDBWritable ( dir, self -> path );
    return GetRCState ( rc ) == rcLocked;
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


/* CheckConsistency
 *  run a consistency check on the open index
 *
 *  "level" [ IN ] - a measure of rigor of the exercise:
 *    0 is the lightest
 *    1 will test all id mappings
 *    2 will perform key->id retrievals
 *    3 will perform id->key retrievals if a projection index exists
 *
 *  "start_id" [ OUT, NULL OKAY ] - returns the first id in index
 *
 *  "id_range" [ OUT, NULL OKAY ] - returns the range of ids from first to last
 *
 *  "num_keys" [ OUT, NULL OKAY ] - returns the number of key entries
 *
 *  "num_rows" [ OUT, NULL OKAY ] - returns the number of key->id mappings
 *
 *  "num_holes" [ OUT, NULL OKAY ] - returns the number of holes in the mapped id range
 */
LIB_EXPORT rc_t CC KIndexConsistencyCheck ( const KIndex *self, uint32_t level,
    int64_t *start_id, uint64_t *id_range, uint64_t *num_keys,
    uint64_t *num_rows, uint64_t *num_holes )
{
    rc_t rc;

    if ( self == NULL )
        rc = RC ( rcDB, rcIndex, rcValidating, rcSelf, rcNull );
    else
    {
        bool key2id, id2key, all_ids;
        switch ( level )
        {
        case 0:
            key2id = id2key = all_ids = false;
            break;
        case 1:
            key2id = id2key = false;
            all_ids = true;
            break;
        case 2:
            key2id = id2key = true;
            all_ids = false;
            break;
        default:
            key2id = id2key = all_ids = true;
        }

        switch ( self -> type )
        {
        case kitText:
            id2key = false;
        case kitText | kitProj:
            switch ( self -> vers )
            {
            case 1:
                rc = KTrieIndexCheckConsistency_v1 ( & self -> u . txt1,
                    start_id, id_range, num_keys, num_rows, num_holes,
                    self, key2id, id2key );
                break;
            case 2:
            case 3:
            case 4:
                rc = KTrieIndexCheckConsistency_v2 ( & self -> u . txt234,
                    start_id, id_range, num_keys, num_rows, num_holes,
                    self, key2id, id2key, all_ids, self -> converted_from_v1 );
                break;
            default:
                return RC ( rcDB, rcIndex, rcValidating, rcIndex, rcBadVersion );
            }
            break;

        default:
            rc = RC ( rcDB, rcIndex, rcValidating, rcFunction, rcUnsupported );
        }
    }

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
    if ( key [ 0 ] == 0 )
        return RC ( rcDB, rcIndex, rcSelecting, rcString, rcEmpty );

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
            rc = KTrieIndexFind_v2 ( & self -> u . txt234, key, start_id, & span, custom_cmp, data, self -> converted_from_v1 );
#else
            rc = KTrieIndexFind_v2 ( & self -> u . txt234, key, start_id, custom_cmp, data, self -> converted_from_v1 );
#endif
            break;
        default:
            return RC ( rcDB, rcIndex, rcSelecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcSelecting, rcNoObj, rcUnknown );
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
    if ( key [ 0 ] == 0 )
        return RC ( rcDB, rcIndex, rcSelecting, rcString, rcEmpty );

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
            rc = KTrieIndexFind_v2 ( & self -> u . txt234, key, & id64, & span, NULL, NULL, self -> converted_from_v1 );
#else
            rc = KTrieIndexFind_v2 ( & self -> u . txt234, key, & id64, NULL, NULL, self -> converted_from_v1 );
#endif
            if ( rc == 0 )
                rc = ( * f ) ( id64, span, data );
            break;
        default:
            return RC ( rcDB, rcIndex, rcSelecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcSelecting, rcNoObj, rcUnknown );
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
        return RC ( rcDB, rcIndex, rcProjecting, rcIndex, rcIncorrect );

    if ( key == NULL )
        return RC ( rcDB, rcIndex, rcProjecting, rcBuffer, rcNull );

    span = 1;

    switch ( self -> type )
    {
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
            rc = KTrieIndexProject_v2 ( & self -> u . txt234, id, start_id, & span, key, kmax, actsize );
#else
            rc = KTrieIndexProject_v2 ( & self -> u . txt234, id, key, kmax, actsize );
            if ( rc == 0 )
                * start_id = id;
#endif
            break;
        default:
            return RC ( rcDB, rcIndex, rcProjecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcProjecting, rcNoObj, rcUnknown );
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

    if ( ( ( KIdxType ) self -> type & kitProj ) == 0 )
        return RC ( rcDB, rcIndex, rcProjecting, rcIndex, rcIncorrect );

    if ( f == NULL )
        return RC ( rcDB, rcIndex, rcProjecting, rcFunction, rcNull );

    span = 1;

    switch ( self -> type )
    {
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
            rc = KTrieIndexProject_v2 ( & self -> u . txt234, id, & start_id, & span, key, sizeof key, NULL );
#else
            rc = KTrieIndexProject_v2 ( & self -> u . txt234, start_id = id, key, sizeof key );
#endif
            if ( rc == 0 )
                rc = ( * f ) ( start_id, span, key, data );
            break;
            
        default:
            return RC ( rcDB, rcIndex, rcProjecting, rcIndex, rcBadVersion );
        }
        break;
    default:
        return RC ( rcDB, rcIndex, rcProjecting, rcNoObj, rcUnknown );
    }
    
    return rc;
}

LIB_EXPORT rc_t CC KIndexFindU64( const KIndex* self, uint64_t offset, uint64_t* key, uint64_t* key_size, int64_t* id, uint64_t* id_qty )
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
        return RC(rcDB, rcIndex, rcSelecting, rcNoObj, rcUnknown);
    }
    return rc;
}

LIB_EXPORT rc_t CC KIndexFindAllU64( const KIndex* self, uint64_t offset,
    rc_t ( CC * f )(uint64_t key, uint64_t key_size, int64_t id, uint64_t id_qty, void* data ), void* data)
{
    rc_t rc = 0;

    if ( self == NULL )
        return RC(rcDB, rcIndex, rcSelecting, rcSelf, rcNull);
    if ( f == NULL )
        return RC(rcDB, rcIndex, rcSelecting, rcFunction, rcNull);

    switch ( self->type )
    {
    case kitU64:
        switch ( self->vers )
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
        return RC(rcDB, rcIndex, rcSelecting, rcNoObj, rcUnknown);
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
            if ( cself -> u . txt234 . pt . maxid < max_row_id )
                ( ( KIndex* ) cself ) -> u . txt234 . pt . maxid = max_row_id;
            break;
        }
        break;
    }
}
