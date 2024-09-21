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

#include "wmeta.h"
#include "wmetadatanode.h"

#include "wdbmgr.h"
#include "wdatabase.h"
#include "wtable.h"
#include "wcolumn.h"

#include <kfs/file.h>
#include <kfs/mmap.h>

#include <klib/debug.h>
#include <klib/rc.h>

#include <strtol.h>
#include <stdio.h>
#include <byteswap.h>

#define KMETADATAVERS 2

/*--------------------------------------------------------------------------
 * KWMetadata
 *  a versioned, hierarchical structure
 */

static rc_t CC KWMetadataWhack ( KMetadata *self );
static rc_t CC KWMetadataAddRef ( const KMetadata *cself );
static rc_t CC KWMetadataRelease ( const KMetadata *cself );
static rc_t CC KWMetadataVersion ( const KMetadata *self, uint32_t *version );
static rc_t CC KWMetadataByteOrder ( const KMetadata *self, bool *reversed );
static rc_t CC KWMetadataRevision ( const KMetadata *self, uint32_t *revision );
static rc_t CC KWMetadataMaxRevision ( const KMetadata *self, uint32_t *revision );
static rc_t CC KWMetadataOpenRevision ( const KMetadata *self, const KMetadata **metap, uint32_t revision );
static rc_t CC KWMetadataGetSequence ( const KMetadata *self, const char *seq, int64_t *val );
static rc_t CC KWMetadataVOpenNodeRead ( const KMetadata *self, const KMDataNode **node, const char *path, va_list args );

static KMetadata_vt KWMetadata_vt =
{
    KWMetadataWhack,
    KWMetadataAddRef,
    KWMetadataRelease,
    KWMetadataVersion,
    KWMetadataByteOrder,
    KWMetadataRevision,
    KWMetadataMaxRevision,
    KWMetadataOpenRevision,
    KWMetadataGetSequence,
    KWMetadataVOpenNodeRead
};

#define CAST() assert( bself->vt == &KWMetadata_vt ); KWMetadata * self = (KWMetadata *)bself

/* OpenNodeUpdate
 * VOpenNodeUpdate
 *  opens a metadata node
 *
 *  "node" [ OUT ] - return parameter for indicated metadata node
 *
 *  "path" [ IN, NULL OKAY ] - optional path for specifying named
 *  node within metadata hierarchy. when NULL, empty, ".", or "/",
 *  return root node in "node". path separator is "/".
 */
LIB_EXPORT rc_t CC KMetadataOpenNodeUpdate ( KMetadata *self,
    KMDataNode **node, const char *path, ... )
{
    rc_t rc;
    va_list args;

    va_start ( args, path );
    rc = KMetadataVOpenNodeUpdate ( self, node, path, args );
    va_end ( args );

    return rc;
}

LIB_EXPORT rc_t CC KMetadataVOpenNodeUpdate ( KMetadata *bself,
    KMDataNode **node, const char *path, va_list args )
{
    CAST();

    rc_t rc;

    if ( node == NULL )
        rc = RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    else
    {
        if ( self == NULL )
            rc = RC ( rcDB, rcMetadata, rcOpening, rcSelf, rcNull );
        else if ( self -> read_only )
            rc = RC ( rcDB, rcMetadata, rcOpening, rcNode, rcReadonly );
        else {
            rc = KMDataNodeVOpenNodeUpdate ( & self -> root -> dad, node, path, args );
            DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
                        ("KMetadataVOpenNodeUpdate(%s) = %d\n", path, rc));
            return rc;
        }

        * node = NULL;
    }

    return rc;
}

/* Flush
 */
typedef struct KMDFlushData KMDFlushData;
struct KMDFlushData
{
    uint64_t pos;
    KFile *f;
    KMD5File *fmd5;
    uint8_t *buffer;
    size_t bsize;
    size_t marker;
    rc_t rc;
};

static
rc_t CC KMDWriteFunc ( void *param, const void *buffer, size_t size, size_t *num_writ )
{
    KMDFlushData *pb = param;
    size_t total, to_write;

    for ( total = 0; total < size; total += to_write )
    {
        to_write = size - total;
        if ( pb -> marker + to_write > pb -> bsize )
            to_write = pb -> bsize - pb -> marker;

        if ( to_write > 0 )
        {
            memmove ( pb -> buffer + pb -> marker,
                ( const uint8_t* ) buffer + total, to_write );
            pb -> marker += to_write;
        }

        if ( pb -> marker == pb -> bsize )
        {
            size_t num_flushed;
            pb -> rc = KFileWrite ( pb -> f, pb -> pos,
                pb -> buffer, pb -> bsize, & num_flushed );
            if ( pb -> rc != 0 )
            {
                * num_writ = 0;
                return pb -> rc;
            }

            if ( num_flushed == 0 )
            {
                * num_writ = total + to_write;
                return pb -> rc = RC ( rcDB, rcMetadata, rcPersisting, rcTransfer, rcIncomplete );
            }

            pb -> marker -= num_flushed;
            pb -> pos += num_flushed;

            if ( pb -> marker != 0 )
                memmove ( pb -> buffer, pb -> buffer + num_flushed, pb -> marker );
        }
    }

    * num_writ = total;
    return 0;
}

static
rc_t CC KWMAttrNodeAuxFunc ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    const KWMAttrNode *n = node;
    size_t nsize = strlen ( n -> name );

    if ( write != NULL )
        return ( * write ) ( write_param, n -> name, nsize + n -> vsize + 1, num_writ );

    * num_writ = nsize + n -> vsize + 1;
    return 0;
}

static
rc_t CC KWMDataNodeAuxFunc ( void *param, const void *node, size_t *num_writ,
    PTWriteFunc write, void *write_param )
{
    rc_t rc;
    const KWMDataNode *n = node;
    size_t nsize = strlen ( n -> name );
    size_t auxsize = 0;

    /* first write node name */
    if ( write != NULL )
    {
        uint8_t bits = ( uint8_t ) ( nsize - 1 ) << 2;
        if ( n -> attr . root != NULL )
            bits |= 1;
        if ( n -> child . root != NULL )
            bits |= 2;
        rc = ( * write ) ( write_param, & bits, 1, num_writ );
        if ( rc == 0 )
            rc = ( * write ) ( write_param, n -> name, nsize, num_writ );
        if ( rc != 0 )
            return rc;
    }

    /* if there are any attributes */
    if ( n -> attr . root != NULL )
    {
        rc = BSTreePersist ( & n -> attr, num_writ,
            write, write_param, KWMAttrNodeAuxFunc, NULL );
        if ( rc != 0 )
            return rc;
        auxsize += * num_writ;
    }

    /* if there are any children */
    if ( n -> child . root != NULL )
    {
        rc = BSTreePersist ( & n -> child, num_writ,
            write, write_param, KWMDataNodeAuxFunc, NULL );
        if ( rc != 0 )
            return rc;
        auxsize += * num_writ;
    }

    /* finally write value */
    if ( write == NULL )
    {
        * num_writ = nsize + 1 + auxsize + n -> vsize;
        return 0;
    }

    rc = ( * write ) ( write_param, n -> value, n -> vsize, num_writ );
    * num_writ += nsize + 1 + auxsize;
    return rc;
}

static
rc_t KWMetadataFlush ( KWMetadata *self )
{
    rc_t rc;
    KMDFlushData pb;
    memset ( & pb, 0, sizeof pb );

    /* allocate buffer */
    pb . buffer = malloc ( pb . bsize = 32 * 1024 );
    if ( pb . buffer == NULL )
        rc = RC ( rcDB, rcMetadata, rcPersisting, rcMemory, rcExhausted );
    else
    {
        /* open output file */
        rc = KWColumnFileCreate ( & pb . f, & pb . fmd5, self -> dir, self -> md5,
                                 kcmInit | kcmParents, false, "md/cur.tmp" );
        /* ZZZZ do we need a "KMD5FileReset ( pb -> fmd5 )" ? I don't think so */
        if ( rc == 0 )
        {
            /* write header */
            KDBHdr *hdr = ( KDBHdr* ) pb . buffer;
            hdr -> endian = eByteOrderTag;
            hdr -> version = KMETADATAVERS;
            pb . marker = sizeof * hdr;

            /* persist root node */
            rc = BSTreePersist ( & self -> root -> child, NULL,
                KMDWriteFunc, & pb, KWMDataNodeAuxFunc, NULL );
            if ( rc == 0 && pb . marker != 0 )
            {
                size_t num_flushed;
                rc = KFileWrite ( pb . f, pb . pos,
                                  pb . buffer, pb . marker, & num_flushed );
                if ( rc == 0 && num_flushed != pb . marker )
                    rc = RC ( rcDB, rcMetadata, rcPersisting, rcTransfer, rcIncomplete );
            }
            pb . rc = KFileRelease ( pb . f );
            if ( pb . rc  ==  0 )
                pb . fmd5 = NULL;
            if ( rc == 0 && ( rc = pb . rc ) == 0 )
            {
                /* rename file */
                rc = KDirectoryRename ( self -> dir, true, "md/cur.tmp", "md/cur" );

                if ( self->md5 != NULL && rc == 0 )
                    rc = KMD5SumFmtRename ( self -> md5, "md/cur.tmp", "md/cur" );

                if ( rc == 0 )
                    self -> dirty = false;
            }
        }

        free ( pb . buffer );
    }
    return rc;
}

/* Whack
 */
static
rc_t CC
KWMetadataWhack ( KMetadata *bself )
{
    CAST();

    rc_t rc = 0;
    KSymbol * symb;
    KDBManager *mgr = self -> mgr;
    assert ( mgr != NULL );

    if ( self -> dirty )
    {
        /* if this was a version 1 file,
           first freeze it */
        if ( self -> vers == 1 )
        {
            rc = KMetadataFreeze ( bself );
            if ( rc != 0 )
                return rc;
        }

        /* flush it */
        rc = KWMetadataFlush ( self );
        if ( rc != 0 )
            return rc;
        self -> dirty = false;
    }

    if ( self -> db != NULL )
    {
        rc = KDatabaseSever ( self -> db );
        if ( rc != 0 )
            return rc;
        self -> db = NULL;
    }
    else if ( self -> tbl != NULL )
    {
        rc = KTableSever ( & self -> tbl -> dad );
        if ( rc != 0 )
            return rc;
        self -> tbl = NULL;
    }
    else if ( self -> col != NULL )
    {
        rc = KColumnSever ( self -> col );
        if ( rc != 0 )
            return rc;
        self -> col = NULL;
    }

    if ( self -> md5 != NULL )
    {
	rc = KMD5SumFmtRelease ( self -> md5 );
	if ( rc != 0 )
	    return rc;
	self -> md5 = NULL;
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
                KDBManagerOpenObjectAdd (mgr, symb);
            else
            {
                /* complete */
                KDirectoryRelease ( self -> dir );
                KMDataNodeRelease ( & self -> root -> dad );
                return KMetadataBaseWhack( bself );
            }
        }
    }

    KRefcountInit ( & self -> dad . refcount, 1, "KMetadata", "whack", "kmeta" );
    return rc;
}


/* AddRef
 * Release
 *  all objects are reference counted
 *  NULL references are ignored
 */
static
rc_t CC
KWMetadataAddRef ( const KMetadata *bself )
{
    CAST();

    rc_t rc = KMetadataBaseAddRef( bself );
    if ( rc == 0 )
    {
        ++ self -> opencount;
    }
    return rc;
}

static
rc_t CC
KWMetadataRelease ( const KMetadata *bself )
{
    CAST();

    if ( self != NULL )
    {
        -- self -> opencount;
    }
    return KMetadataBaseRelease( bself );
}

/* Make
 */

static
rc_t
KMetadataPopulate ( KMetadata *bself, const KDirectory *dir, const char *path, bool read_only )
{
    CAST();

    const KFile *f;
    rc_t rc = KDirectoryOpenFileRead ( dir, & f, "%s", path );
    if ( rc == 0 )
    {
        const KMMap *mm;
        rc = KMMapMakeRead ( & mm, f );
        if ( rc == 0 )
        {
            size_t size;
            const void *addr = NULL;
            rc = KMMapSize ( mm, & size );
            if ( rc == 0 )
                rc = KMMapAddrRead ( mm, & addr );

            if ( rc == 0 )
            {
                union
                {
                    KDBHdr v1;
                    KDBHdr v2;
                } hdrs;

                const KDBHdr *hdr = ( const KDBHdr* ) addr;
                const void *pbstree_src = hdr + 1;

                rc = KDBHdrValidate ( hdr, size, 1, KMETADATAVERS );
                if ( self -> read_only && GetRCState ( rc ) == rcIncorrect && GetRCObject ( rc ) == rcByteOrder )
                {
                    hdrs . v1 . endian = bswap_32 ( hdr -> endian );
                    hdrs . v1 . version = bswap_32 ( hdr -> version );
                    rc = KDBHdrValidate ( & hdrs . v1, size, 1, KMETADATAVERS );
                    if ( rc == 0 )
                    {
                        self -> byteswap = true;
                        switch ( hdrs . v1 . version )
                        {
                        case 1:
                            hdr = & hdrs . v1;
                            break;
                        case 2:
                            hdr = & hdrs . v2;
                            break;
                        }
                    }
                }
                if ( rc == 0 )
                {
                    PBSTree *bst;
                    rc = PBSTreeMake ( & bst, pbstree_src, size - sizeof * hdr, self -> byteswap );
                    if ( rc != 0 )
                        rc = RC ( rcDB, rcMetadata, rcConstructing, rcData, rcCorrupt );
                    else
                    {
                        KWMDataNodeInflateData pb;

                        pb . meta = self;
                        pb . par = self -> root;
                        pb . bst = & self -> root -> child;
                        pb . node_size_limit = read_only ? NODE_SIZE_LIMIT : 0;
                        pb . node_child_limit = read_only ? NODE_CHILD_LIMIT : 0;
                        pb . rc = 0;
                        pb . byteswap = self -> byteswap;

                        if ( hdr -> version == 1 )
                            PBSTreeDoUntil ( bst, false, KWMDataNodeInflate_v1, & pb );
                        else
                            PBSTreeDoUntil ( bst, false, KWMDataNodeInflate, & pb );
                        rc = pb . rc;

                        self -> vers = hdr -> version;

                        PBSTreeWhack ( bst );
                    }
                }
            }

            KMMapRelease ( mm );
        }

        KFileRelease ( f );
    }
    return rc;
}

rc_t
KWMetadataMake ( KWMetadata **metap, KDirectory *dir, const char *path, uint32_t rev, bool populate, bool read_only )
{
    rc_t rc = 0;
    KWMetadata *meta = malloc ( sizeof * meta + strlen ( path ) );
    if ( meta == NULL )
        rc = RC ( rcDB, rcMetadata, rcConstructing, rcMemory, rcExhausted );
    else
    {
        memset ( meta, 0, sizeof * meta );
        meta -> dad . vt = & KWMetadata_vt;
        if ( KWMDataNodeMakeRoot( & meta -> root, meta ) == 0 )
        {
            meta -> dir = dir;
            KRefcountInit ( & meta -> dad . refcount, 1, "KWMetadata", "make-update", path );
            meta -> opencount = 1;
            meta -> rev = rev;
            meta -> read_only = read_only;

            strcpy ( meta -> path, path );

            meta->sym.u.obj = meta;
            StringInitCString (&meta->sym.name, meta->path);
            meta->sym.type = kptMetadata;

            if ( ! populate )
            {
                meta -> vers = KMETADATAVERS;
                KDirectoryAddRef ( dir );
                * metap = meta;
                return 0;
            }

            rc = KMetadataPopulate ( & meta -> dad, dir, path, read_only );
            if ( rc == 0 )
            {
                KDirectoryAddRef ( dir );
                * metap = meta;
                return 0;
            }

            free ( meta -> root );
        }

        free ( meta );
    }
    * metap = NULL;
    return rc;
}

/* Version
 *  returns the metadata format version
 */
static
rc_t CC
KWMetadataVersion ( const KMetadata *bself, uint32_t *version )
{
    CAST();

    if ( version == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * version = self -> vers;
    return 0;
}


/* ByteOrder
 *  indicates whether original byte order is reversed
 *  under current architecture.
 *
 *  the byte order of the column is established by
 *  the host architecture when created.
 *
 *  "reversed" [ OUT ] - if true, the original byte
 *  order is reversed with regard to host native byte order.
 */
static
rc_t CC
KWMetadataByteOrder ( const KMetadata *bself, bool *reversed )
{
    CAST();

    if ( reversed == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * reversed = self -> byteswap;
    return 0;
}


/* Revision
 *  returns current revision number
 *  where 0 ( zero ) means tip
 */
static
rc_t CC
KWMetadataRevision ( const KMetadata *bself, uint32_t *revision )
{
    CAST();

    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * revision = self -> rev;
    return 0;
}


/* MaxRevision
 *  returns the maximum revision available
 */
static
rc_t CC
KWMetadataMaxRevision ( const KMetadata *bself, uint32_t *revision )
{
    CAST();

    rc_t rc;
    KNamelist *listing;

    if ( revision == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );

    * revision = 0;

    rc = KDirectoryList ( self -> dir,
        & listing, NULL, NULL, "md" );
    if ( rc == 0 )
    {
        uint32_t rev_max, count;
        rc = KNamelistCount ( listing, & count );
        if ( rc == 0 )
        {
            uint32_t idx;
            for ( rev_max = idx = 0; idx < count; ++ idx )
            {
                const char *name;

                rc = KNamelistGet ( listing, idx, & name );
                if ( rc != 0 )
                    break;

                if ( name [ 0 ] == 'r' )
                {
                    char *end;
                    uint32_t rev = strtou32 ( name + 1, & end, 10 );
                    if ( end [ 0 ] == 0 && rev > rev_max )
                        rev_max = rev;
                }
            }

            * revision = rev_max;
        }

        KNamelistRelease ( listing );
    }
    else if ( GetRCState ( rc ) == rcNotFound )
    {
        rc = 0;
    }

    return rc;
}


/* Commit
 *  ensure any changes are committed to disk
 */
LIB_EXPORT
rc_t CC
KMetadataCommit ( KMetadata *bself )
{
    CAST();

    rc_t rc;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcCommitting, rcSelf, rcNull );

    /* only flush if dirty */
    if ( ! self -> dirty )
        return 0;

    /* if this was a version 1 file,
       first freeze it */
    if ( self -> vers == 1 )
    {
        rc = KMetadataFreeze ( bself );
        if ( rc != 0 )
            return rc;
    }

    /* flush it */
    rc = KWMetadataFlush ( self );
    if ( rc == 0 )
        self -> dirty = false;

    return rc;
}


/* Freeze
 *  freezes current metadata revision
 *  further modification will begin on a copy
 */
LIB_EXPORT
rc_t CC
KMetadataFreeze ( KMetadata *bself )
{
    CAST();

    rc_t rc;
    uint32_t rev_max;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcFreezing, rcSelf, rcNull );

    if ( self -> read_only )
        return RC ( rcDB, rcMetadata, rcFreezing, rcMetadata, rcReadonly );

    switch ( KDirectoryPathType_v1 ( self -> dir, "md/cur" ) )
    {
    case kptFile:
        break;
    case kptFile | kptAlias:
        return 0;
    default:
        return RC ( rcDB, rcMetadata, rcFreezing, rcPath, rcInvalid );
    }

    /* find max revision */
    rc = KMetadataMaxRevision ( bself, & rev_max );
    if ( rc == 0 )
    {
        int len;
        char revision [ 16 ];	/* nice even stack (could have been 8) */

        /* rename tip to new version */
        len = snprintf ( revision, sizeof revision, "md/r%.3u", rev_max + 1 );
        if ( len < 0 || len >= sizeof revision )
            return RC ( rcDB, rcMetadata, rcFreezing, rcPath, rcInvalid );

        rc = KDirectoryRename ( self -> dir, false, "md/cur", revision );
        if ( rc == 0 )
        {
            /* now create link to this guy */
            rc = KDirectoryCreateAlias ( self -> dir,
                0777, kcmCreate, revision, "md/cur" );

            if ( rc == 0 )
            {
                if ( self -> md5 != NULL )
                {
                    rc = KMD5SumFmtRename ( self -> md5, "md/cur", revision );
                    if ( rc != 0 )	/* attempt to recover */
                        KDirectoryRemove ( self -> dir, false, "md/cur" );
                }
            }
            if ( rc != 0 )                /* attempt to recover */
                KDirectoryRename ( self -> dir, false, revision, "md/cur" );
        }
    }

    return rc;
}


/* OpenRevision
 *  opens a read-only indexed revision of metadata
 */
static
rc_t CC
KWMetadataOpenRevision ( const KMetadata *bself, const KMetadata **metap, uint32_t revision )
{
    CAST();

    rc_t rc;
    const KWMetadata *meta;
    bool  meta_is_cached;

    if ( metap == NULL )
        return RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );

    * metap = NULL;

    rc = KDBWManagerOpenMetadataReadInt ( self -> mgr,
        & meta, self -> dir, revision, false, &meta_is_cached  );
    if ( rc == 0 )
    {
        if(!meta_is_cached)
	{
	    if ( self -> db != NULL )
                ((KWMetadata*)meta) -> db = KDatabaseAttach ( self -> db );
            else if ( self -> tbl != NULL )
                ((KWMetadata*)meta) -> tbl = (KWTable*) KTableAttach ( & self -> tbl -> dad );
            else if ( self -> col != NULL )
                ((KWMetadata*)meta) -> col = KColumnAttach ( self -> col );
	}

        * metap = & meta -> dad;
    }

    return rc;
}


/* GetSequence
 * SetSequence
 * NextSequence
 *  access a named sequence
 *
 *  "seq" [ IN ] - NUL terminated sequence name
 *
 *  "val" [ OUT ] - return parameter for sequence value
 *  "val" [ IN ] - new sequence value
 */
static
rc_t CC
KWMetadataGetSequence ( const KMetadata *bself, const char *seq, int64_t *val )
{
    CAST();

    rc_t rc;
    const KMDataNode *found;

    if ( val == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcParam, rcNull );
    * val = 0;

    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcAccessing, rcPath, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcAccessing, rcPath, rcInvalid );

    rc = KMDataNodeOpenNodeRead ( & self -> root -> dad, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        rc = KMDataNodeReadB64 ( found, val );
        KMDataNodeRelease ( found );
    }

    return rc;
}

LIB_EXPORT
rc_t CC
KMetadataSetSequence ( KMetadata *bself, const char *seq, int64_t val )
{
    CAST();

    rc_t rc;
    KMDataNode *found;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcSelf, rcNull );
    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcInvalid );

    rc = KMDataNodeOpenNodeUpdate ( & self -> root -> dad, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        rc = KMDataNodeWriteB64 ( found, & val );
        KMDataNodeRelease ( found );
    }

    return rc;
}

LIB_EXPORT
rc_t CC
KMetadataNextSequence ( KMetadata *bself, const char *seq, int64_t *val )
{
    CAST();

    rc_t rc;
    KMDataNode *found;

    if ( val == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcParam, rcNull );
    * val = 0;

    if ( self == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcSelf, rcNull );
    if ( seq == NULL )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcNull );
    if ( seq [ 0 ] == 0 )
        return RC ( rcDB, rcMetadata, rcUpdating, rcPath, rcInvalid );

    rc = KMDataNodeOpenNodeUpdate ( & self -> root -> dad, & found, ".seq/%s", seq );
    if ( rc == 0 )
    {
        rc = KMDataNodeReadB64 ( found, val );
        if ( rc == 0 || GetRCState ( rc ) == rcIncomplete )
        {
            * val += 1;
            rc = KMDataNodeWriteB64 ( found, val );
        }

        KMDataNodeRelease ( found );
    }

    return rc;
}

static
rc_t CC
KWMetadataVOpenNodeRead ( const KMetadata *bself, const KMDataNode **node, const char *path, va_list args )
{
    CAST();

    rc_t rc = 0;

    if ( node == NULL )
        rc = RC ( rcDB, rcMetadata, rcOpening, rcParam, rcNull );
    else if ( self == NULL )
    {
        * node = NULL;
        rc = RC ( rcDB, rcMetadata, rcOpening, rcSelf, rcNull );
    }
    else
        rc = KMDataNodeVOpenNodeRead ( & self -> root -> dad, node, path, args );

    DBGMSG(DBG_KDB, DBG_FLAG(DBG_KDB_KDB),
            ("KMetadataVOpenNodeRead(%s) = %d\n", path, rc));

    return rc;
}

